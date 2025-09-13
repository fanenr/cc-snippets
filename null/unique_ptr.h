#ifndef UNIQUE_PTR_H
#define UNIQUE_PTR_H

#include <cstddef>
#include <utility>
#include <type_traits>

template <typename T>
class default_delete
{
public:
  static_assert (sizeof (T) > 0, "Cannot delete an incomplete type.");

  default_delete () noexcept = default;

  template <typename U>
  default_delete (const default_delete<U> &)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
  }

  void
  operator() (T *ptr) const
  {
    delete ptr;
  }
};

template <typename T, typename Deleter = default_delete<T>>
class unique_ptr
{
  template <typename U, typename D>
  friend class unique_ptr;

public:
  using pointer = T *;
  using element_type = T;
  using deleter_type = Deleter;

  unique_ptr (std::nullptr_t = nullptr) : ptr_ (nullptr) {}

  explicit unique_ptr (T *ptr = nullptr, Deleter del = {}) noexcept
      : ptr_ (ptr), del_ (std::move (del))
  {
  }

  ~unique_ptr ()
  {
    if (ptr_)
      del_ (ptr_);
  }

  unique_ptr (const unique_ptr &) = delete;
  unique_ptr &operator= (const unique_ptr &) = delete;

  unique_ptr (unique_ptr &&other) noexcept
      : ptr_ (std::exchange (other.ptr_, nullptr)),
	del_ (std::move (other.del_))
  {
  }

  unique_ptr &
  operator= (unique_ptr &&other) noexcept
  {
    unique_ptr (std::move (other)).swap (*this);
    return *this;
  }

  template <typename T2, typename Deleter2>
  unique_ptr (unique_ptr<T2, Deleter2> &&other) noexcept
      : ptr_ (std::exchange (other.ptr_, nullptr)),
	del_ (std::move (other.del_))
  {
    static_assert (std::is_convertible<T2 *, T *>::value,
		   "T2* must be convertible to T*");
  }

  template <typename T2, typename Deleter2>
  unique_ptr &
  operator= (unique_ptr<T2, Deleter2> &&other) noexcept
  {
    unique_ptr (std::move (other)).swap (*this);
    return *this;
  }

  void
  swap (unique_ptr &other) noexcept
  {
    using std::swap;
    swap (ptr_, other.ptr_);
    swap (del_, other.del_);
  }

  T &
  operator* () const
  {
    return *ptr_;
  }

  T *
  operator->() const noexcept
  {
    return ptr_;
  }

  explicit
  operator bool () const noexcept
  {
    return ptr_;
  }

  T *
  get () const noexcept
  {
    return ptr_;
  }

  void
  reset (T *ptr = nullptr, Deleter del = {}) noexcept
  {
    unique_ptr (ptr, std::move (del)).swap (*this);
  }

  T *
  release () noexcept
  {
    return std::exchange (ptr_, nullptr);
  }

  Deleter &
  get_deleter () noexcept
  {
    return del_;
  }

  const Deleter &
  get_deleter () const noexcept
  {
    return del_;
  }

private:
  T *ptr_;
  Deleter del_;
};

#endif // UNIQUE_PTR_H
