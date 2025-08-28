#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>

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
  start ()
  {
    buffer_.resize (1024);
    socket_.async_receive (
	asio::buffer (buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_receive (ec, n); });
  }

private:
  void
  handle_receive (const sys::error_code &error, size_t bytes)
  {
    if (error)
      return;

    buffer_.resize (bytes);
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
    if (!thread_.joinable ())
      thread_ = std::thread ([this] () {
	start_accept ();
	auto work = asio::make_work_guard (io_context_);
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

  void
  start_accept ()
  {
    auto conn = connection::make (io_context_);
    acceptor_.async_accept (conn->socket_,
			    [this, conn] (const sys::error_code &ec) {
			      handle_accept (ec, conn);
			    });
  }

private:
  void
  handle_accept (const sys::error_code &error, connection::pointer conn)
  {
    if (error)
      throw sys::system_error (error);

    conn->start ();

    start_accept ();
  }

private:
  asio::io_context io_context_;
  tcp::acceptor acceptor_;
  std::thread thread_;
};

#endif // SERVER_H
