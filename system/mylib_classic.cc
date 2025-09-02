#include "mylib_error.h"

namespace sys = boost::system;

namespace mylib
{

inline void
foo (int v, sys::error_code &ec) noexcept
{
  switch (v)
    {
    case 1:
      ec = error::type1_a;
      break;
    case 2:
      ec = error::type1_b;
      break;
    case 3:
      ec = error::type2_a;
      break;
    case 4:
      ec = error::type2_b;
      break;
    default:
      ec = {};
      break;
    }
}

inline void
foo (int v)
{
  sys::error_code ec;
  foo (v, ec);
  if (ec)
    throw sys::system_error (ec);
}

} // namespace mylib

void
test_foo ()
{
  sys::error_code sys_ec;

  mylib::foo (0, sys_ec);
  assert (!sys_ec.failed ());
  assert (sys_ec == sys::error_code ());
  assert (sys_ec == sys::error_condition ());

  mylib::foo (5, sys_ec);
  assert (!sys_ec.failed ());
  assert (sys_ec == sys::error_code ());
  assert (sys_ec == sys::error_condition ());
}

int
main ()
{
  test_foo ();
}
