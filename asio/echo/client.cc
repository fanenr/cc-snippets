#include <thread>
#include <boost/asio.hpp>

namespace sys = boost::system;
namespace asio = boost::asio;
using asio::ip::tcp;

int connections;
std::atomic<bool> stop;
std::atomic<int> echoes;
std::atomic<int> failed;
std::atomic<int> connected;
std::atomic<int> completed;

class connection : public std::enable_shared_from_this<connection>
{
public:
  using pointer = std::shared_ptr<connection>;

  connection (asio::io_context &io_context)
      : socket_ (io_context, tcp::v4 ()), timer_ (io_context)
  {
    socket_.non_blocking (true);
  }

  static pointer
  make (asio::io_context &io_context)
  {
    return std::make_shared<connection> (io_context);
  }

  void
  connect (short port)
  {
    socket_.async_connect (
	tcp::endpoint (tcp::v4 (), port),
	[this, self = shared_from_this ()] (const sys::error_code &ec) {
	  handle_connect (ec);
	});
  }

  void
  start ()
  {
    send_buffer_ = "Hello world!";
    recv_buffer_.resize (send_buffer_.size ());

    asio::async_write (
	socket_, asio::buffer (send_buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_send (ec, n); });
  }

private:
  tcp::socket socket_;
  std::string send_buffer_;
  std::string recv_buffer_;
  asio::steady_timer timer_;

  void
  handle_connect (const sys::error_code &error)
  {
    completed++;

    if (error)
      {
	failed++;
	return;
      }

    connected++;
    start ();
  }

  void
  handle_error ()
  {
    failed++;
    connected--;
  }

  void
  handle_send (const sys::error_code &error, size_t /*bytes*/)
  {
    if (error)
      {
	handle_error ();
	return;
      }

    socket_.async_receive (
	asio::buffer (recv_buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_receive (ec, n); });
  }

  void
  handle_receive (const sys::error_code &error, size_t bytes)
  {
    if (error)
      {
	handle_error ();
	return;
      }

    recv_buffer_.resize (bytes);

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

	recv_buffer_.insert (recv_buffer_.end (), buff, buff + n);
      }

    if (recv_buffer_ == send_buffer_)
      echoes++;

    timer_.expires_after (asio::chrono::seconds (1));
    timer_.async_wait ([this, self = shared_from_this ()] (
			   const sys::error_code &ec) { handle_wait (ec); });
  }

  void
  handle_wait (const sys::error_code &error)
  {
    if (error)
      {
	handle_error ();
	return;
      }

    start ();
  }
};

void
monitor ()
{
  auto print_line = [] () {
    for (int i = 0; i < 75; i++)
      putchar ('-');
    puts ("");
  };

  printf ("Target: 127.0.0.1:8080-8089 | Total Connections: %d\n",
	  connections);
  print_line ();

  auto start = std::chrono::steady_clock::now ();
  for (; !stop && completed < connections;
       std::this_thread::sleep_for (std::chrono::milliseconds (100)))
    {
      printf ("\rConnecting: %d / %d", connected.load (), connections);
      fflush (stdout);
    }
  auto end = std::chrono::steady_clock::now ();

  double seconds = std::chrono::duration<double> (end - start).count ();
  printf ("\rEstablished %d connections in %.2f seconds (%d failed).\n",
	  connected.load (), seconds, failed.load ());

  if (stop)
    return;
  print_line ();

  const char *fmt = "Active: %6d | "
		    "Failed: %6d | "
		    "Echoes: %9d | "
		    "Rate: %8d echo/s\n";

  int last = 0, times = 0;
  for (; !stop; times++)
    {
      if (connected.load () == 0)
	{
	  stop = true;
	  break;
	}

      int now = echoes.load ();
      int rate = now - last;
      last = now;

      printf (fmt, connected.load (), failed.load (), now, rate);
      std::this_thread::sleep_for (std::chrono::seconds (1));
    }

  print_line ();
  int total = echoes.load ();
  printf (fmt, connected.load (), failed.load (), total, total / times);
}

int
main (int argc, char **argv)
{
  if (argc != 2)
    {
      printf ("Usage: %s <connections>\n", argv[0]);
      return 1;
    }

  signal (SIGPIPE, SIG_IGN);
  signal (SIGINT, [] (int signum) {
    if (signum == SIGINT)
      stop = true;
  });

  asio::io_context io_context;
  connections = std::atoi (argv[1]);
  for (int i = 0; i < connections; i++)
    {
      auto conn = connection::make (io_context);
      conn->connect (8080 + i % 10);
    }

  std::vector<std::thread> worker_threads;
  for (int i = 0; i < 4; i++)
    worker_threads.emplace_back ([&] () {
      auto work = asio::make_work_guard (io_context);
      io_context.run ();
    });
  std::thread monitor_thread (monitor);

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

  io_context.stop ();
  monitor_thread.join ();
  for (auto &t : worker_threads)
    t.join ();
}
