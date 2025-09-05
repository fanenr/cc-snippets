#include <boost/system.hpp>

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

boost::system::result<void, my::error>
foo (bool v)
{
  if (v)
    return {};
  return my::error{ "bad" };
}

// namespace sys = boost::system;

int
main ()
{
  try
    {
      auto res = foo (false);
      res.value ();
      // not *res: if res has no value, *res is UB
    }
  catch (const std::exception &e)
    {
      printf ("exception: %s\n", e.what ());
    }
}
