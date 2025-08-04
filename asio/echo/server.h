#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <vector>
#include <boost/asio.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

class connection : public std::enable_shared_from_this<connection>
{
public:
  using pointer = std::shared_ptr<connection>;

  connection (asio::io_context &io_context) : socket_ (io_context) {}

  static pointer
  make (asio::io_context &io_context)
  {
    return std::make_shared<connection> (io_context);
  }

  tcp::socket &
  socket ()
  {
    return socket_;
  }

  void
  start ()
  {
    buffer_.resize (1024);
    socket_.async_receive (
	asio::buffer (buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_receive (ec, n); });
  }

private:
  tcp::socket socket_;
  std::vector<char> buffer_;

  void
  handle_receive (const sys::error_code &error, size_t bytes)
  {
    if (error)
      return;

    buffer_.resize (bytes);

    for (;;)
      {
	char buff[1024];
	sys::error_code ec;
	size_t n = socket_.read_some (asio::buffer (buff), ec);

	if (ec)
	  {
	    if (ec == asio::error::try_again || ec == asio::error::would_block)
	      break;
	    return;
	  }

	buffer_.insert (buffer_.end (), buff, buff + n);
      }

    asio::async_write (
	socket_, asio::buffer (buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_write (ec, n); });
  }

  void
  handle_write (const sys::error_code &error, size_t /*bytes*/)
  {
    if (error)
      return;

    start ();
  }
};

class server
{
public:
  explicit server (short port, int n = 1)
  {
    for (int i = 0; i < n; i++)
      acceptors_.emplace_back (io_context_, tcp::endpoint (tcp::v4 (), port++))
	  .non_blocking (true);
  }

  ~server () { stop (); }

  void
  start ()
  {
    start_accept ();
    if (!thread_.joinable ())
      thread_ = std::thread ([this] () {
	auto work = asio::make_work_guard (io_context_);
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
    io_context_.stop ();
    if (thread_.joinable ())
      thread_.join ();
  }

private:
  std::thread thread_;
  asio::io_context io_context_;
  std::vector<tcp::acceptor> acceptors_;

  void
  start_accept ()
  {
    for (auto &acceptor : acceptors_)
      start_accept (acceptor);
  }

  void
  start_accept (tcp::acceptor &acceptor)
  {
    auto conn = connection::make (io_context_);
    acceptor.async_accept (
	conn->socket (), [this, conn, &acceptor] (const sys::error_code &ec) {
	  handle_accept (ec, conn);
	  start_accept (acceptor);
	});
  }

  void
  handle_accept (const sys::error_code &error, connection::pointer connection)
  {
    if (error)
      throw sys::system_error (error);

    connection->socket ().non_blocking (true);
    connection->start ();
  }
};

#endif // SERVER_H
