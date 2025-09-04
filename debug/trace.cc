#include <iostream>

#include <boost/assert.hpp>
#include <boost/stacktrace.hpp>
#include <boost/throw_exception.hpp>

namespace boost
{

void
assertion_failed_msg (const char *expr, const char *msg, const char *function,
		      const char *file, long line)
{
  if (msg)
    // "file:line: function: Assertion `(expr)&&(msg)' failed."
    std::fprintf (stderr, "%s:%ld: %s: Assertion `(%s)&&(%s)' failed.\n", file,
		  line, function, expr, msg);
  else
    // "file:line: function: Assertion `expr' failed."
    std::fprintf (stderr, "%s:%ld: %s: Assertion `%s' failed.\n", file, line,
		  function, expr);

  std::cerr << "Backtrace:\n" << boost::stacktrace::stacktrace ();
  std::fflush (stdout);
  std::abort ();
}

void
assertion_failed (const char *expr, const char *function, const char *file,
		  long line)
{
  assertion_failed_msg (expr, nullptr, function, file, line);
}

} // namespace boost

void
terminate_handler ()
{
  std::set_terminate (nullptr);

#if defined(BOOST_NO_EXCEPTIONS)
  std::fputs ("terminate called with exceptions disabled.\n", stderr);
#else // defined(BOOST_NO_EXCEPTIONS)
  auto exception_ptr = std::current_exception ();
  if (exception_ptr)
    try
      {
	std::rethrow_exception (exception_ptr);
      }
    catch (const std::exception &e)
      {
	const char *exception_type;
#if defined(BOOST_NO_RTTI)
	exception_type = "unknown (RTTI is disabled)";
#else  // defined(BOOST_NO_RTTI)
	exception_type = typeid (e).name ();
	boost::core::scoped_demangled_name demangled_name (exception_type);
	if (demangled_name.get ())
	  exception_type = demangled_name.get ();
#endif // defined(BOOST_NO_RTTI)
	std::fprintf (stderr,
		      "terminate called after throwing an instance of '%s'\n"
		      "  what(): %s\n",
		      exception_type, e.what ());
      }
    catch (...)
      {
	std::fputs ("terminate called after throwing an unknown exception.\n",
		    stderr);
      }
  else
    std::fputs ("terminate called without an active exception.\n", stderr);
#endif // defined(BOOST_NO_EXCEPTIONS)

  std::cerr << "Backtrace:\n" << boost::stacktrace::stacktrace ();
  std::fflush (stdout);
  std::abort ();
}

void
foo (bool v)
{
  BOOST_ASSERT (v);
}

void
bar (bool v)
{
  if (!v)
    BOOST_THROW_EXCEPTION (std::runtime_error ("v is false"));
}

int
main ()
{
  std::set_terminate (terminate_handler);

  bar (false);
  foo (false);
}
