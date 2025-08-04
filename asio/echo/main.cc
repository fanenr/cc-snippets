#include "server.h"

#include <thread>

std::atomic<bool> stop;

int
main ()
{
  signal (SIGPIPE, SIG_IGN);
  signal (SIGINT, [] (int signum) {
    if (signum == SIGINT)
      stop = true;
  });

  server srv (8080, 10);
  srv.start ();

  for (; !stop;)
    std::this_thread::sleep_for (std::chrono::milliseconds (100));
  srv.stop ();
}
