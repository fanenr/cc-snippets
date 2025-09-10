#include <list>

#include <boost/asio.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

constexpr asio::chrono::seconds default_timeout (5);
constexpr size_t max_receive = 1024;

class session : public std::enable_shared_from_this<session>
{
public:
  using pointer = std::shared_ptr<session>;

  static pointer
  make (tcp::socket sock, asio::chrono::seconds timeout = default_timeout)
  {
    return std::make_shared<session> (std::move (sock), timeout);
  }

  explicit session (tcp::socket sock,
		    asio::chrono::seconds timeout = default_timeout)
      : socket_ (std::move (sock)), timeout_ (timeout)
  {
  }

  void
  start (bool with_timeout = true)
  {
    if (with_timeout)
      start_receive_with_timeout ();
    else
      {
	start_receive_with_watchdog ();
	start_watchdog ();
      }
  }

private:
  void
  start_receive_with_watchdog ()
  {
    auto self = shared_from_this ();
    auto handle_receive{ [this, self] (const sys::error_code &error,
				       size_t bytes) {
      if (error)
	{
	  timer_.cancel ();
	  return;
	}

      buffer_.resize (bytes);
      start_send_with_watchdog ();
    } };

    buffer_.resize (max_receive);
    socket_.async_receive (asio::buffer (buffer_),
			   asio::bind_executor (strand_, handle_receive));

    deadline_ = asio::chrono::steady_clock::now () + timeout_;
  }

  void
  start_send_with_watchdog ()
  {
    auto self = shared_from_this ();
    auto handle_write{ [this, self] (const sys::error_code &error,
				     size_t /*bytes*/) {
      if (error)
	{
	  timer_.cancel ();
	  return;
	}

      start_receive_with_watchdog ();
    } };

    asio::async_write (socket_, asio::buffer (buffer_),
		       asio::bind_executor (strand_, handle_write));

    deadline_ = asio::chrono::steady_clock::now () + timeout_;
  }

  void
  start_watchdog ()
  {
    auto now = asio::chrono::steady_clock::now ();
    if (now >= deadline_)
      {
	socket_.close ();
	return;
      }

    auto self = shared_from_this ();
    auto handle_wait{ [this] (const sys::error_code &error) {
      if (!error)
	start_watchdog ();
    } };

    timer_.expires_at (deadline_);
    timer_.async_wait (asio::bind_executor (strand_, handle_wait));
  }

  void
  start_receive_with_timeout ()
  {
    auto self = shared_from_this ();
    auto handle_receive{ [this, self] (const sys::error_code &error,
				       size_t bytes) {
      timer_.cancel ();
      if (error)
	return;

      buffer_.resize (bytes);
      start_send_with_timeout ();
    } };

    buffer_.resize (max_receive);
    socket_.async_receive (asio::buffer (buffer_),
			   asio::bind_executor (strand_, handle_receive));

    start_timeout ();
  }

  void
  start_send_with_timeout ()
  {
    auto self = shared_from_this ();
    auto handle_write{ [this, self] (const sys::error_code &error,
				     size_t /*bytes*/) {
      timer_.cancel ();
      if (error)
	return;

      start_receive_with_timeout ();
    } };

    asio::async_write (socket_, asio::buffer (buffer_),
		       asio::bind_executor (strand_, handle_write));

    start_timeout ();
  }

  void
  start_timeout ()
  {
    auto self = shared_from_this ();
    auto handle_wait{ [this, self] (const sys::error_code &error) {
      if (!error)
	socket_.close ();
    } };

    timer_.expires_after (timeout_);
    timer_.async_wait (asio::bind_executor (strand_, handle_wait));
  }

private:
  tcp::socket socket_;
  asio::steady_timer timer_{ socket_.get_executor () };
  asio::strand<asio::any_io_executor> strand_{ socket_.get_executor () };

  std::vector<char> buffer_;
  asio::chrono::seconds timeout_;
  asio::steady_timer::time_point deadline_;
};

class server
{
public:
  explicit server (asio::io_context &io, short port)
      : acceptor_ (io, tcp::endpoint (tcp::v4 (), port))
  {
  }

  void
  start ()
  {
    auto handle_accept{ [this] (const sys::error_code &error,
				tcp::socket sock) {
      if (error)
	return;

      auto sess = session::make (std::move (sock));
      sess->start (false);

      start ();
    } };

    acceptor_.async_accept (handle_accept);
  }

private:
  tcp::acceptor acceptor_;
};

int
main ()
{
  try
    {
      asio::io_context io_context;
      asio::signal_set signals (io_context, SIGINT, SIGTERM);
      std::list<server> servers;
      std::vector<std::thread> threads;

      signals.async_wait ([&] (const sys::error_code & /*error*/,
			       int /*signum*/) { io_context.stop (); });

      for (int i = 0; i < 10; i++)
	servers.emplace (servers.end (), io_context, 8080 + i)->start ();

      unsigned int num_threads = std::thread::hardware_concurrency ();
      num_threads = num_threads ? num_threads * 2 : 10;
      for (unsigned int i = 0; i < num_threads; i++)
	threads.emplace_back ([&io_context] () {
	  try
	    {
	      io_context.run ();
	    }
	  catch (const std::exception &e)
	    {
	      std::printf ("exception: %s\n", e.what ());
	    }
	});

      io_context.run ();
      for (auto &thrd : threads)
	thrd.join ();
    }
  catch (const std::exception &e)
    {
      std::printf ("exception: %s\n", e.what ());
    }
}
