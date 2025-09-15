#ifndef WEAK_PTR_H
#define WEAK_PTR_H

#include "shared_ptr.h"

template <typename T>
class weak_ptr
{
  template <typename U>
  friend class weak_ptr;

  template <typename U>
  friend class shared_ptr;

public:
  using element_type = T;

  weak_ptr (std::nullptr_t = nullptr) : ptr_ (nullptr), cb_ (nullptr) {}

  template <typename U>
  weak_ptr (const shared_ptr<U> &sp) noexcept : ptr_ (sp.ptr_), cb_ (sp.cb_)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    if (cb_)
      cb_->inc_weak_count ();
  }

  ~weak_ptr ()
  {
    if (cb_)
      cb_->dec_weak_count ();
  }

  weak_ptr (const weak_ptr &other) noexcept
      : ptr_ (other.ptr_), cb_ (other.cb_)
  {
    if (cb_)
      cb_->inc_weak_count ();
  }

  weak_ptr &
  operator= (const weak_ptr &other) noexcept
  {
    weak_ptr (other).swap (*this);
    return *this;
  }

  template <typename U>
  weak_ptr (const weak_ptr<U> &other) noexcept
      : ptr_ (other.ptr_), cb_ (other.cb_)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    if (cb_)
      cb_->inc_weak_count ();
  }

  template <typename U>
  weak_ptr &
  operator= (const weak_ptr<U> &other) noexcept
  {
    weak_ptr (other).swap (*this);
    return *this;
  }

  template <typename U>
  weak_ptr &
  operator= (const shared_ptr<U> &sp) noexcept
  {
    weak_ptr (sp).swap (*this);
    return *this;
  }

  weak_ptr (weak_ptr &&other) noexcept
      : ptr_ (std::exchange (other.ptr_, nullptr)),
	cb_ (std::exchange (other.cb_, nullptr))
  {
  }

  weak_ptr &
  operator= (weak_ptr &&other) noexcept
  {
    weak_ptr (std::move (other)).swap (*this);
    return *this;
  }

  template <typename U>
  weak_ptr (weak_ptr<U> &&other) noexcept
      : ptr_ (std::exchange (other.ptr_, nullptr)),
	cb_ (std::exchange (other.cb_, nullptr))
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
  }

  template <typename U>
  weak_ptr &
  operator= (weak_ptr<U> &&other) noexcept
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    weak_ptr (std::move (other)).swap (*this);
    return *this;
  }

  void
  swap (weak_ptr &other) noexcept
  {
    using std::swap;
    swap (ptr_, other.ptr_);
    swap (cb_, other.cb_);
  }

  shared_ptr<T>
  lock () noexcept
  {
    if (cb_ && cb_->lock_use_count ())
      {
	shared_ptr<T> sp;
	sp.ptr_ = ptr_;
	sp.cb_ = cb_;
	return sp;
      }
    return {};
  }

  void
  reset () noexcept
  {
    weak_ptr ().swap (*this);
  }

  bool
  expired () const noexcept
  {
    return use_count () == 0;
  }

  long
  use_count () const noexcept
  {
    return cb_ ? cb_->use_count.load (std::memory_order_relaxed) : 0;
  }

private:
  T *ptr_;
  control_block_base *cb_;
};

#endif // WEAK_PTR_H
