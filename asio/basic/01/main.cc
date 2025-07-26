#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;

int
main ()
{
  asio::io_context ctx;
  asio::steady_timer timer (ctx, asio::chrono::seconds (5));
  timer.wait ();
  std::cout << "Hello, world!" << std::endl;
}
