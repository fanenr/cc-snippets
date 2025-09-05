#ifndef MYLIB_ERROR_H
#define MYLIB_ERROR_H

#include <boost/system.hpp>

namespace mylib
{

// error_code
enum class error
{
  success = 0,
  type1_a,
  type1_b,
  type2_a,
  type2_b,
};

class error_category_impl : public boost::system::error_category
{
public:
  const char *name () const noexcept;
  std::string message (int ev) const;
  const char *message (int ev, char *buffer, std::size_t len) const noexcept;

  boost::system::error_condition
  default_error_condition (int ev) const noexcept;
};

const boost::system::error_category &error_category ();
boost::system::error_code make_error_code (error e);

// error_condition
enum class condition
{
  success = 0,
  type1,
  type2,
};

class condition_category_impl : public boost::system::error_category
{
public:
  const char *name () const noexcept;
  std::string message (int ev) const;
  const char *message (int ev, char *buffer, std::size_t len) const noexcept;
};

const boost::system::error_category &condition_category ();
boost::system::error_condition make_error_condition (condition c);

} // namespace mylib

namespace boost
{
namespace system
{

template <>
struct is_error_code_enum<::mylib::error> : public std::true_type
{
};

template <>
struct is_error_condition_enum<::mylib::condition> : public std::true_type
{
};

} // namespace system
} // namespace boost

namespace std
{

template <>
struct is_error_code_enum<::mylib::error> : public std::true_type
{
};

template <>
struct is_error_condition_enum<::mylib::condition> : public std::true_type
{
};

} // namespace std

namespace mylib
{

inline const char *
error_category_impl::name () const noexcept
{
  return "mylib";
}

inline std::string
error_category_impl::message (int ev) const
{
  char buffer[64];
  return message (ev, buffer, sizeof (buffer));
}

inline const char *
error_category_impl::message (int ev, char *buffer,
			      std::size_t len) const noexcept
{
  (void)buffer, (void)len;
  switch (static_cast<error> (ev))
    {
    case error::success:
      return "success";
    case error::type1_a:
      return "error type1 a";
    case error::type1_b:
      return "error type1 b";
    case error::type2_a:
      return "error type2 a";
    case error::type2_b:
      return "error type2 b";
    default:
      return "unknown error code";
    }
}

inline boost::system::error_condition
error_category_impl::default_error_condition (int ev) const noexcept
{
  switch (static_cast<error> (ev))
    {
    case error::success:
      return {};

    case error::type1_a:
    case error::type1_b:
      return condition::type1;

    case error::type2_a:
    case error::type2_b:
      return condition::type2;

    default:
      return boost::system::error_condition (ev, *this);
    }
}

inline const boost::system::error_category &
error_category ()
{
  static const error_category_impl instance;
  return instance;
}

inline boost::system::error_code
make_error_code (error e)
{
  return boost::system::error_code (static_cast<int> (e), error_category ());
}

inline const char *
condition_category_impl::name () const noexcept
{
  return "mylib";
}

inline std::string
condition_category_impl::message (int ev) const
{
  char buffer[64];
  return message (ev, buffer, sizeof (buffer));
}

inline const char *
condition_category_impl::message (int ev, char *buffer,
				  std::size_t len) const noexcept
{
  (void)buffer, (void)len;
  switch (static_cast<condition> (ev))
    {
    case condition::success:
      return "success";
    case condition::type1:
      return "error type1";
    case condition::type2:
      return "error type2";
    default:
      return "unknown error condition";
    }
}

inline const boost::system::error_category &
condition_category ()
{
  static const condition_category_impl instance;
  return instance;
}

inline boost::system::error_condition
make_error_condition (condition c)
{
  return boost::system::error_condition (static_cast<int> (c),
					 condition_category ());
}

} // namespace mylib

#endif // MYLIB_H
