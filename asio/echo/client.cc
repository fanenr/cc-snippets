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
  start (short port)
  {
    socket_.async_connect (
	tcp::endpoint (tcp::v4 (), port),
	[this, self = shared_from_this ()] (const sys::error_code &ec) {
	  handle_connect (ec);
	});
  }

private:
  tcp::socket socket_;
  std::string send_buffer_;
  std::string recv_buffer_;
  asio::steady_timer timer_;

  void
  start ()
  {
    send_buffer_ = "Hello world!";
    recv_buffer_.resize (send_buffer_.size ());

    asio::async_write (
	socket_, asio::buffer (send_buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_write (ec, n); });
  }

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
  handle_write (const sys::error_code &error, size_t /*bytes*/)
  {
    if (error)
      {
	handle_error ();
	return;
      }

    asio::async_read (
	socket_, asio::buffer (recv_buffer_),
	[this, self = shared_from_this ()] (
	    const sys::error_code &ec, size_t n) { handle_read (ec, n); });
  }

  void
  handle_read (const sys::error_code &error, size_t /*bytes*/)
  {
    if (error)
      {
	handle_error ();
	return;
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
  int rate = (times == 0 ? 0 : total / times);
  printf (fmt, connected.load (), failed.load (), total, rate);
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
      {
	stop = true;
	puts ("");
      }
  });

  connections = std::atoi (argv[1]);
  std::vector<asio::io_context> io_contexts (10);

  std::vector<std::thread> worker_threads;
  for (auto &io_context : io_contexts)
    worker_threads.emplace_back ([&io_context] () {
      auto work = asio::make_work_guard (io_context);
      io_context.run ();
    });

  for (int i = 0; i < connections; i++)
    {
      auto &io_context = io_contexts[i % io_contexts.size ()];
      asio::post (io_context, [port = 8080 + i % 10, &io_context] () {
	connection::make (io_context)->start (port);
      });
    }

  std::thread monitor_thread (monitor);

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

  for (auto &io : io_contexts)
    io.stop ();
  for (auto &thrd : worker_threads)
    thrd.join ();
  monitor_thread.join ();
}
