#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

// namespace sys = boost::system;
namespace asio = boost::asio;

void
handle_spawn (std::exception_ptr e)
{
  if (e)
    std::rethrow_exception (e);
}

void
co1 (asio::yield_context yield)
{
  std::printf ("co1: 1\n");
  asio::post (yield);

  std::printf ("co1: 2\n");
  asio::post (yield);

  std::printf ("co1: 3\n");
  asio::post (yield);
}

void
co2 (asio::yield_context yield)
{
  std::printf ("co2: 1\n");
  asio::post (yield);

  std::printf ("co2: 2\n");
  asio::post (yield);

  std::printf ("co2: 3\n");
  asio::post (yield);
}

void
co_main (asio::yield_context yield)
{
  asio::spawn (yield.get_executor (), co1, handle_spawn);
  asio::spawn (yield.get_executor (), co2, handle_spawn);
}

int
main ()
{
  try
    {
      asio::io_context io;
      asio::spawn (io, co_main, handle_spawn);
      io.run ();
    }
  catch (const std::exception &e)
    {
      std::printf ("Exception: %s\n", e.what ());
    }
}
