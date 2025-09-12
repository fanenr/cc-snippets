#ifndef SHARED_PTR_H
#define SHARED_PTR_H

#include <atomic>
#include <cstddef>
#include <utility>
#include <type_traits>

template <typename T>
class default_delete
{
public:
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

struct control_block_base
{
  std::atomic<long> use_count;
  std::atomic<long> weak_count;

  control_block_base () : use_count (1), weak_count (1) {}

  long
  dec_use_count ()
  {
    long old = use_count.fetch_sub (1, std::memory_order_acq_rel);
    if (old == 1)
      dispose ();
    return old;
  }

  bool
  try_inc_use_count ()
  {
    long count = use_count.load (std::memory_order_relaxed);
    do
      if (count == 0)
	return false;
    while (!use_count.compare_exchange_weak (count, count + 1,
					     std::memory_order_acq_rel,
					     std::memory_order_relaxed));
    return true;
  }

  long
  dec_weak_count ()
  {
    long old = weak_count.fetch_sub (1, std::memory_order_acq_rel);
    if (old == 1)
      destroy ();
    return old;
  }

  virtual ~control_block_base () = default;
  virtual void dispose () = 0;
  virtual void destroy () = 0;
};

template <typename T, typename Deleter>
struct control_block : public control_block_base
{
  T *ptr;
  Deleter del;

  control_block (T *ptr_, Deleter del_) : ptr (ptr_), del (std::move (del_)) {}

  ~control_block () override = default;

  void
  dispose () override
  {
    del (ptr);
  }

  void
  destroy () override
  {
    delete this;
  }
};

template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr
{
  template <typename U>
  friend class weak_ptr;

  template <typename U>
  friend class shared_ptr;

  // for weak_ptr::lock
  shared_ptr (T *ptr, control_block_base *cb) noexcept : ptr_ (ptr), cb_ (cb)
  {
  }

public:
  using element_type = T;
  using weak_type = weak_ptr<T>;

  template <typename U, typename Deleter = default_delete<U>>
  explicit shared_ptr (U *ptr = nullptr, Deleter del = {})
      : ptr_ (ptr), cb_ (nullptr)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    try
      {
	if (ptr_)
	  cb_ = new control_block<U, Deleter> (ptr_, std::move (del));
      }
    catch (...)
      {
	del (ptr_);
	throw;
      }
  }

  ~shared_ptr ()
  {
    if (cb_ && cb_->dec_use_count () == 1)
      cb_->dec_weak_count ();
  }

  // copy
  template <typename U>
  shared_ptr (const shared_ptr<U> &other) noexcept
      : ptr_ (other.ptr_), cb_ (other.cb_)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    if (cb_)
      cb_->use_count.fetch_add (1, std::memory_order_relaxed);
  }

  template <typename U>
  shared_ptr &
  operator= (const shared_ptr<U> &other) noexcept
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    shared_ptr (other).swap (*this);
    return *this;
  }

  // move
  template <typename U>
  shared_ptr (shared_ptr<U> &&other) noexcept
      : ptr_ (std::exchange (other.ptr_, nullptr)),
	cb_ (std::exchange (other.cb_, nullptr))
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
  }

  template <typename U>
  shared_ptr &
  operator= (shared_ptr<U> &&other) noexcept
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    shared_ptr (std::move (other)).swap (*this);
    return *this;
  }

  // swap
  void
  swap (shared_ptr &other) noexcept
  {
    using std::swap;
    swap (ptr_, other.ptr_);
    swap (cb_, other.cb_);
  }

  // operator *
  T &
  operator* () noexcept
  {
    return *ptr_;
  }

  const T &
  operator* () const noexcept
  {
    return *ptr_;
  }

  // operator ->
  T *
  operator->() noexcept
  {
    return ptr_;
  }

  const T *
  operator->() const noexcept
  {
    return ptr_;
  }

  // get
  T *
  get () noexcept
  {
    return ptr_;
  }

  const T *
  get () const noexcept
  {
    return ptr_;
  }

  // operator bool
  explicit
  operator bool () const noexcept
  {
    return ptr_;
  }

  // use_count
  long
  use_count () const noexcept
  {
    return cb_ ? cb_->use_count.load (std::memory_order_relaxed) : 0;
  }

  // reset
  template <typename U, typename Deleter = default_delete<U>>
  void
  reset (U *ptr = nullptr, Deleter del = {})
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    shared_ptr (ptr, std::move (del)).swap (*this);
  }

private:
  T *ptr_;
  control_block_base *cb_;
};

template <typename T>
class weak_ptr
{
  template <typename U>
  friend class weak_ptr;

public:
  template <typename U>
  weak_ptr (const shared_ptr<U> sp = {}) noexcept
      : ptr_ (sp.ptr_), cb_ (sp.cb_)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    if (cb_)
      cb_->weak_count.fetch_add (1, std::memory_order_relaxed);
  }

  ~weak_ptr ()
  {
    if (cb_)
      cb_->dec_weak_count ();
  }

  // copy
  template <typename U>
  weak_ptr (const weak_ptr<U> &other) noexcept
      : ptr_ (other.ptr_), cb_ (other.cb_)
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    if (cb_)
      cb_->weak_count.fetch_add (1, std::memory_order_relaxed);
  }

  template <typename U>
  weak_ptr &
  operator= (const weak_ptr<U> &other) noexcept
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    weak_ptr (other).swap (*this);
    return *this;
  }

  template <typename U>
  weak_ptr &
  operator= (const shared_ptr<U> &sp) noexcept
  {
    static_assert (std::is_convertible<U *, T *>::value,
		   "U* must be convertible to T*");
    weak_ptr (sp).swap (*this);
    return *this;
  }

  // move
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

  // swap
  void
  swap (weak_ptr &other) noexcept
  {
    using std::swap;
    swap (ptr_, other.ptr_);
    swap (cb_, other.cb_);
  }

  // reset
  void
  reset () noexcept
  {
    weak_ptr ().swap (*this);
  }

  // use_count
  long
  use_count () const noexcept
  {
    return cb_ ? cb_->use_count.load (std::memory_order_relaxed) : 0;
  }

  // expired
  bool
  expired () const noexcept
  {
    return use_count () == 0;
  }

  // lock
  shared_ptr<T>
  lock () noexcept
  {
    if (cb_ && cb_->try_inc_use_count ())
      return shared_ptr (ptr_, cb_);
    return {};
  }

private:
  T *ptr_;
  control_block_base *cb_;
};

#endif // SHARED_PTR_H
