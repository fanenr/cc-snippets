#ifndef CO_SERVER_H
#define CO_SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

class connection : public std::enable_shared_from_this<connection>
{
public:
  using pointer = std::shared_ptr<connection>;

  static pointer
  make (asio::io_context &io)
  {
    return std::make_shared<connection> (io);
  }

  explicit connection (asio::io_context &io) : socket_ (io) {}

  void
  start (asio::any_io_executor ex)
  {
    auto go = [this, self = shared_from_this ()] (asio::yield_context yield) {
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
    };

    asio::spawn (ex, go, asio::detached);
  }

private:
  tcp::socket socket_;
  std::vector<char> buffer_;
  friend class server;
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
	  auto conn = connection::make (io_context_);

	  acceptor_.async_accept (conn->socket_, yield);

	  conn->start (yield.get_executor ());
	}
    };

    if (!thread_.joinable ())
      thread_ = std::thread ([this, go] () {
	asio::spawn (io_context_, go, asio::detached);
	io_context_.run ();
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

#endif // CO_SERVER_H
