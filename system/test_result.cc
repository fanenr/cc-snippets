#include <boost/system.hpp>

namespace sys = boost::system;

namespace my
{

struct error
{
  std::string message;
};

void
throw_exception_from_error (const error &e, const boost::source_location &loc)
{
  auto msg = e.message + " [location: " + loc.to_string () + "]";
  throw std::runtime_error (msg);
}

} // namespace my

sys::result<void, my::error>
foo (bool v)
{
  if (v)
    return {};
  return my::error{ "bad" };
}

int
main ()
{
  try
    {
      auto res = foo (false);
      res.value ();
    }
  catch (const std::exception &e)
    {
      printf ("exception: %s\n", e.what ());
    }
}
