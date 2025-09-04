#include <boost/assert.hpp>
#include <boost/assert/source_location.hpp>

void
foo (bool v)
{
  BOOST_ASSERT (v);
}

void
bar (bool v)
{
  BOOST_ASSERT_MSG (v, "v is false");
}

int
main ()
{
  bar (false);
  foo (false);

  auto loc = BOOST_CURRENT_LOCATION;
  printf ("loc: %s\n", loc.to_string ().data ());
}
