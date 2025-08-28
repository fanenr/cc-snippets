#include <boost/asio/any_io_executor.hpp>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

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

  static pointer
  make (asio::io_context &io)
  {
    return std::make_shared<connection> (io);
  }

  explicit connection (asio::io_context &io)
      : socket_ (io, tcp::v4 ()), timer_ (io)
  {
  }

  void
  start (short port, asio::any_io_executor ex)
  {
    auto self = shared_from_this ();
    auto go = [this, port, self] (asio::yield_context yield) {
      sys::error_code ec;

      // connect
      socket_.async_connect (tcp::endpoint (tcp::v4 (), port), yield[ec]);
      completed++;
      if (ec)
	{
	  failed++;
	  return;
	}
      connected++;

      // echo
      for (;;)
	{
	  // send
	  send_buffer_ = "Hello world!";
	  recv_buffer_.resize (send_buffer_.size ());
	  asio::async_write (socket_, asio::buffer (send_buffer_), yield[ec]);
	  if (ec)
	    {
	      connected--;
	      failed++;
	      break;
	    }

	  // recv
	  asio::async_read (socket_, asio::buffer (recv_buffer_), yield[ec]);
	  if (ec)
	    {
	      connected--;
	      failed++;
	      break;
	    }

	  // check
	  if (recv_buffer_ == send_buffer_)
	    echoes++;

	  // delay
	  timer_.expires_after (asio::chrono::seconds (1));
	  timer_.async_wait (yield[ec]);
	}
    };

    asio::spawn (ex, go, asio::detached);
  }

private:
  tcp::socket socket_;
  std::string send_buffer_;
  std::string recv_buffer_;
  asio::steady_timer timer_;
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
  std::vector<std::thread> echo_thrds;
  std::vector<asio::io_context> ctxs (10);

  for (int i = 0; i < connections; i++)
    {
      auto &io = ctxs[i % ctxs.size ()];
      asio::post (io, [port = 8080 + i % 10, &io] () {
	connection::make (io)->start (port, io.get_executor ());
      });
    }

  for (auto &io : ctxs)
    echo_thrds.emplace_back ([&io] () { io.run (); });

  std::thread monitor_thrd (monitor);

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));

  for (auto &io : ctxs)
    io.stop ();
  for (auto &thrd : echo_thrds)
    thrd.join ();
  monitor_thrd.join ();
}
