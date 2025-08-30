#include <list>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/context/fixedsize_stack.hpp>
#include <boost/context/pooled_fixedsize_stack.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

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

  explicit session (tcp::socket sock) : socket_ (std::move (sock)) {}

  void
  start (asio::yield_context yield)
  {
    for (;;)
      {
	size_t n;
	sys::error_code ec;

	buffer_.resize (1024);
	n = socket_.async_receive (asio::buffer (buffer_), yield[ec]);
	if (ec)
	  break;

	buffer_.resize (n);
	n = asio::async_write (socket_, asio::buffer (buffer_), yield[ec]);
	if (ec)
	  break;
      }
  }

private:
  tcp::socket socket_;
  std::vector<char> buffer_;
};

class server
{
public:
  explicit server (short port)
      : acceptor_ (io_context_, tcp::endpoint (tcp::v4 (), port))
  {
  }

  ~server () { stop (); }

  void
  start ()
  {
    auto go = [this] (asio::yield_context yield) {
      for (;;)
	{
	  auto sock = acceptor_.async_accept (yield);
	  auto sess = session::make (std::move (sock));
	  auto go
	      = [sess] (asio::yield_context yield) { sess->start (yield); };
	  asio::spawn (io_context_, asio::allocator_arg_t (), allocator, go,
		       handle_spawn);
	}
    };

    if (!thread_.joinable ())
      thread_ = std::thread ([this, go] () {
	try
	  {
	    asio::spawn (io_context_, asio::allocator_arg_t (), allocator, go,
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
  stop ()
  {
    io_context_.stop ();
    if (thread_.joinable ())
      thread_.join ();
  }

private:
  asio::io_context io_context_;
  tcp::acceptor acceptor_;
  std::thread thread_;
};

std::atomic<bool> stop;

int
main ()
{
  signal (SIGPIPE, SIG_IGN);
  signal (SIGINT, [] (int signum) {
    if (signum == SIGINT)
      stop = true;
  });

  std::list<server> servers;
  for (int i = 0; i < 10; i++)
    {
      servers.emplace_back (8080 + i);
      servers.back ().start ();
    }

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}
