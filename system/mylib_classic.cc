#include "mylib_error.h"

namespace mylib
{

inline void
foo (int v, boost::system::error_code &ec) noexcept
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
  boost::system::error_code ec;
  foo (v, ec);
  if (ec)
    BOOST_THROW_EXCEPTION (boost::system::system_error (ec));
}

} // namespace mylib

namespace sys = boost::system;

void
test_foo ()
{
  sys::error_code ec;

  // 0 success: ec = {}, cond = {}
  mylib::foo (0, ec);
  BOOST_ASSERT (!ec && !ec.failed ());
  BOOST_ASSERT (ec == sys::error_code ());
  BOOST_ASSERT (ec == sys::error_condition ());

  // 1 failure: ec = type1_a, cond = type1
  mylib::foo (1, ec);
  BOOST_ASSERT (ec && ec.failed ());
  BOOST_ASSERT (ec == mylib::error::type1_a);
  BOOST_ASSERT (ec == mylib::condition::type1);

  // 3 failure: ec = type2_a, cond = type2
  mylib::foo (3, ec);
  BOOST_ASSERT (ec && ec.failed ());
  BOOST_ASSERT (ec == mylib::error::type2_a);
  BOOST_ASSERT (ec == mylib::condition::type2);

  // 5 success: ec = {}, cond = {}
  mylib::foo (5, ec);
  BOOST_ASSERT (!ec && !ec.failed ());
  BOOST_ASSERT (ec == sys::error_code ());
  BOOST_ASSERT (ec == sys::error_condition ());
}

int
main ()
{
  test_foo ();
}
