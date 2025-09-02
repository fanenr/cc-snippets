#include "mylib_error.h"

namespace sys = boost::system;

namespace mylib
{

inline sys::result<void>
foo (int v) noexcept
{
  switch (v)
    {
    case 1:
      return error::type1_a;
    case 2:
      return error::type1_b;
    case 3:
      return error::type2_a;
    case 4:
      return error::type2_b;
    default:
      return {};
    }
}

} // namespace mylib

void
test_foo ()
{
  auto res = mylib::foo (3);

  // error_code
  assert (res.has_error ());
  auto ec = res.error ();
  assert (ec == mylib::error::type2_a);
  assert (ec == mylib::condition::type2);

  assert (!res);
  assert (!res.has_value ());

  // exception
  bool thrown = false;
  try
    {
      res.value (); // not *res
    }
  catch (const std::exception &e)
    {
      printf ("exception: %s\n", e.what ());
      thrown = true;
    }
  assert (thrown);
}

int
main ()
{
  test_foo ();
}
