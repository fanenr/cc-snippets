#include <boost/asio/signal_set.hpp>
#include <boost/system/detail/error_code.hpp>
#include <csignal>
#include <list>
#include <thread>

#include <boost/asio.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

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
  start ()
  {
    buffer_.resize (1024);
    auto self = shared_from_this ();
    socket_.async_receive (asio::buffer (buffer_),
			   [this, self] (const sys::error_code &ec, size_t n) {
			     handle_receive (ec, n);
			   });
  }

private:
  void
  handle_receive (const sys::error_code &error, size_t bytes)
  {
    if (error)
      return;

    buffer_.resize (bytes);
    auto self = shared_from_this ();
    asio::async_write (socket_, asio::buffer (buffer_),
		       [this, self] (const sys::error_code &ec, size_t n) {
			 handle_write (ec, n);
		       });
  }

  void
  handle_write (const sys::error_code &error, size_t /*bytes*/)
  {
    if (error)
      return;

    start ();
  }

private:
  tcp::socket socket_;
  std::vector<char> buffer_;
};

class server
{
public:
  explicit server (short port)
      : signals_ (io_context_, SIGINT, SIGTERM, SIGPIPE),
	acceptor_ (io_context_, tcp::endpoint (tcp::v4 (), port))
  {
    wait_signals ();
  }

  ~server ()
  {
    stop ();
    wait ();
  }

  void
  start ()
  {
    if (!thread_.joinable ())
      thread_ = std::thread ([this] () {
	start_accept ();
	io_context_.run ();
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

  void
  start_accept ()
  {
    acceptor_.async_accept (
	[this] (const sys::error_code &ec, tcp::socket sock) {
	  handle_accept (ec, std::move (sock));
	});
  }

private:
  void
  wait_signals ()
  {
    signals_.async_wait ([this] (const sys::error_code &ec, int sig) {
      handle_signals (ec, sig);
    });
  }

  void
  handle_signals (const sys::error_code &error, int signum)
  {
    if (error)
      return;

    switch (signum)
      {
      case SIGINT:
      case SIGTERM:
	stop ();
	break;

      default:
	wait_signals ();
	break;
      }
  }

  void
  handle_accept (const sys::error_code &error, tcp::socket sock)
  {
    if (error)
      throw sys::system_error (error);

    auto sess = session::make (std::move (sock));
    sess->start ();

    start_accept ();
  }

private:
  asio::io_context io_context_;
  asio::signal_set signals_;
  tcp::acceptor acceptor_;
  std::thread thread_;
};

std::atomic<bool> stop;

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
