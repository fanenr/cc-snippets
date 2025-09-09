#include <list>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/context/fixedsize_stack.hpp>
#include <boost/context/pooled_fixedsize_stack.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

constexpr asio::chrono::seconds default_timeout (5);
constexpr size_t max_receive = 1024;

using allocator_t = boost::context::fixedsize_stack;
allocator_t allocator (8 * 1024);

void
handle_spawn (std::exception_ptr e)
{
  if (e)
    std::rethrow_exception (e);
}

class session : public std::enable_shared_from_this<session>
{
public:
  using pointer = std::shared_ptr<session>;

  static pointer
  make (tcp::socket sock)
  {
    return std::make_shared<session> (std::move (sock));
  }

  explicit session (tcp::socket sock,
		    asio::chrono::seconds timeout = default_timeout)
      : socket_ (std::move (sock)), timer_ (socket_.get_executor ()),
	timeout_ (timeout), strand_ (socket_.get_executor ())
  {
  }

  void
  start (bool with_timeout = true)
  {
    if (with_timeout)
      start_with_timeout ();
    else
      start_with_watchdog ();
  }

private:
  void
  start_with_timeout ()
  {
    auto self = shared_from_this ();
    auto echo = [this, self] (asio::yield_context yield) {
      for (;;)
	{
	  size_t n;
	  sys::error_code ec;

	  start_timeout ();
	  buffer_.resize (max_receive);
	  n = socket_.async_receive (asio::buffer (buffer_), yield[ec]);
	  timer_.cancel ();
	  if (ec)
	    break;

	  start_timeout ();
	  buffer_.resize (n);
	  asio::async_write (socket_, asio::buffer (buffer_), yield[ec]);
	  timer_.cancel ();
	  if (ec)
	    break;
	}
    };
    asio::spawn (strand_, std::allocator_arg, allocator, echo, handle_spawn);
  }

  void
  start_timeout ()
  {
    auto self = shared_from_this ();
    auto wait = [this, self] (asio::yield_context yield) {
      timer_.expires_after (timeout_);

      sys::error_code ec;
      timer_.async_wait (yield[ec]);

      if (!ec)
	socket_.cancel ();
    };
    asio::spawn (strand_, std::allocator_arg, allocator, wait, handle_spawn);
  }

  void
  start_with_watchdog ()
  {
    auto self = shared_from_this ();
    auto echo = [this, self] (asio::yield_context yield) {
      for (;;)
	{
	  size_t n;
	  sys::error_code ec;

	  deadline_ = asio::chrono::steady_clock::now () + timeout_;
	  buffer_.resize (max_receive);
	  n = socket_.async_receive (asio::buffer (buffer_), yield[ec]);
	  if (ec)
	    {
	      timer_.cancel ();
	      break;
	    }

	  deadline_ = asio::chrono::steady_clock::now () + timeout_;
	  buffer_.resize (n);
	  asio::async_write (socket_, asio::buffer (buffer_), yield[ec]);
	  if (ec)
	    {
	      timer_.cancel ();
	      break;
	    }
	}
    };
    asio::spawn (strand_, std::allocator_arg, allocator, echo, handle_spawn);

    start_watchdog ();
  }

  void
  start_watchdog ()
  {
    auto self = shared_from_this ();
    auto watch = [this, self] (asio::yield_context yield) {
      for (;;)
	{
	  auto now = asio::chrono::steady_clock::now ();
	  if (now >= deadline_)
	    {
	      socket_.cancel ();
	      break;
	    }

	  sys::error_code ec;
	  timer_.expires_at (deadline_);
	  timer_.async_wait (yield[ec]);
	  if (ec)
	    break;
	}
    };
    asio::spawn (strand_, std::allocator_arg, allocator, watch, handle_spawn);
  }

private:
  tcp::socket socket_;
  asio::steady_timer timer_;
  asio::chrono::seconds timeout_;
  asio::steady_timer::time_point deadline_;
  asio::strand<asio::any_io_executor> strand_;
  std::vector<char> buffer_;
};

class server
{
public:
  explicit server (short port)
      : signals_ (io_context_, SIGINT, SIGTERM, SIGPIPE),
	acceptor_ (io_context_, tcp::endpoint (tcp::v4 (), port))
  {
  }

  ~server ()
  {
    stop ();
    wait ();
  }

  void
  start ()
  {
    auto signal = [this] (asio::yield_context yield) {
      for (;;)
	{
	  sys::error_code ec;
	  int sig = signals_.async_wait (yield[ec]);
	  if (!ec && (sig == SIGINT || sig == SIGTERM))
	    stop ();
	}
    };

    auto listen = [this] (asio::yield_context yield) {
      for (;;)
	{
	  auto sock = acceptor_.async_accept (yield);
	  auto sess = session::make (std::move (sock));
	  sess->start ();
	}
    };

    if (!thread_.joinable ())
      thread_ = std::thread ([this, signal, listen] () {
	try
	  {
	    asio::spawn (io_context_, std::allocator_arg, allocator, signal,
			 handle_spawn);
	    asio::spawn (io_context_, std::allocator_arg, allocator, listen,
			 handle_spawn);
	    io_context_.run ();
	  }
	catch (const std::exception &e)
	  {
	    printf ("Exception: %s\n", e.what ());
	  }
      });
  }

  void
  wait ()
  {
    if (thread_.joinable ())
      thread_.join ();
  }

  void
  stop ()
  {
    if (!io_context_.stopped ())
      io_context_.stop ();
  }

private:
  asio::io_context io_context_;
  asio::signal_set signals_;
  tcp::acceptor acceptor_;
  std::thread thread_;
};

int
main ()
{
  std::list<server> servers;
  for (int i = 0; i < 10; i++)
    {
      servers.emplace_back (8080 + i);
      servers.back ().start ();
    }
  for (auto &srv : servers)
    srv.wait ();
}
