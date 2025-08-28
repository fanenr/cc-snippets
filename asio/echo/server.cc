#include "co_server.h"

#include <thread>
#include <list>

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
    servers.emplace_back (8080 + i).start ();

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
}
