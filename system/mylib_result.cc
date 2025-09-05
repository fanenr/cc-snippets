#include "mylib_error.h"

namespace mylib
{

inline boost::system::result<void>
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

// namespace sys = boost::system;

void
test_foo ()
{
  auto res = mylib::foo (3);

  // error_code
  auto ec = res.error ();
  BOOST_ASSERT (res.has_error ());
  BOOST_ASSERT (ec == mylib::error::type2_a);
  BOOST_ASSERT (ec == mylib::condition::type2);

  BOOST_ASSERT (!res);
  BOOST_ASSERT (!res.has_value ());

  // exception
  bool thrown = false;
  try
    {
      res.value ();
      // not *res: if res has no value, *res is UB
    }
  catch (const std::exception &e)
    {
      printf ("exception: %s\n", e.what ());
      thrown = true;
    }
  BOOST_ASSERT (thrown);
}

int
main ()
{
  test_foo ();
}
