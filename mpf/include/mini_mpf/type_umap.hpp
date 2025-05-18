#pragma once
#include "type_array.hpp"
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace mini_mpf
{

template <typename Enum, typename... Types> class type_umap
{

public:
  // preserve type info of map and inner
  using self = type_umap<Enum, Types...>;
  using array = type_array<Types...>;
  using key = Enum;

private:
  constexpr static std::size_t length = array::len ();

  template <Enum Key> struct _at
  {
    using type = typename array::template at<static_cast<std::size_t> (Key)>;
  };

  template <template <typename...> typename T> struct _forward
  {
    using type = T<Types...>;
  };

public:
  // at is used to get the type at Pos in a umap
  template <Enum Key> using at = typename _at<Key>::type;

  // len will return the number of types
  constexpr static std::size_t
  len () noexcept
  {
    return length;
  }

  // cmp can be used to compare a given type with the type associated with Key
  template <typename T, Enum Key>
  constexpr static bool
  cmp () noexcept
  {
    return std::is_same_v<at<Key>, T>;
  }

  // forward can forward all types in umap to any suitable templates
  template <template <typename...> typename T>
  using forward = typename _forward<T>::type;

  // find will return Key associated with the given type or assert failed
  template <typename Type, std::size_t Pos = 0>
  constexpr static Enum
  find () noexcept
  {
    constexpr bool ret = cmp<Type, static_cast<Enum> (Pos)> ();

    if constexpr (ret)
      return static_cast<Enum> (Pos);

    if constexpr (!ret && Pos + 1 < length)
      return find<Type, Pos + 1> ();

    if constexpr (Pos == length - 1)
      static_assert (ret, "the given type not exists");
  }

  // find_if will return an optional
  template <typename Type, std::size_t Pos = 0>
  constexpr static std::optional<Enum>
  find_if () noexcept
  {
    constexpr bool ret = cmp<Type, static_cast<Enum> (Pos)> ();

    if constexpr (ret)
      return static_cast<Enum> (Pos);

    if constexpr (!ret && Pos + 1 < length)
      return find_if<Type, Pos + 1> ();

    if constexpr (!ret && Pos == length - 1)
      return std::nullopt;
  }

  // for_each is used to traverse all types in an array and perform some
  // actions Func only receive the current type in array
  template <template <typename> typename Func, std::size_t Pos = 0,
	    typename... Args>
  constexpr static void
  for_each (Args &&...args)
  {
    constexpr Enum key = static_cast<Enum> (Pos);

    if constexpr (Pos < length)
      std::invoke (Func<at<key> > (), std::forward<Args> (args)...);

    if constexpr (Pos + 1 < length)
      for_each<Func, Pos + 1> (std::forward<Args> (args)...);
  }

  // Func receive both the current type and index in array
  template <template <typename, Enum> typename Func, std::size_t Pos = 0,
	    typename... Args>
  constexpr static void
  for_each (Args &&...args)
  {
    constexpr Enum key = static_cast<Enum> (Pos);

    if constexpr (Pos < length)
      std::invoke (Func<at<key>, key> (), std::forward<Args> (args)...);

    if constexpr (Pos + 1 < length)
      for_each<Func, Pos + 1> (std::forward<Args> (args)...);
  }
};

// type_umap also accept a type_array as its types pack
template <typename Enum, typename... Types>
class type_umap<Enum, type_array<Types...> >
{

public:
  // preserve type info of map and inner
  using self = type_umap<Enum, Types...>;
  using array = type_array<Types...>;
  using key = Enum;

private:
  constexpr static std::size_t length = array::len ();

  template <Enum Key> struct _at
  {
    using type = typename array::template at<static_cast<std::size_t> (Key)>;
  };

  template <template <typename...> typename T> struct _forward
  {
    using type = T<Types...>;
  };

public:
  // at is used to get the type at Pos in a umap
  template <Enum Key> using at = typename _at<Key>::type;

  // len will return the number of types
  constexpr static std::size_t
  len () noexcept
  {
    return length;
  }

  // cmp can be used to compare a given type with the type associated with Key
  template <typename T, Enum Key>
  constexpr static bool
  cmp () noexcept
  {
    return std::is_same_v<at<Key>, T>;
  }

  // forward can forward all types in umap to any suitable templates
  template <template <typename...> typename T>
  using forward = typename _forward<T>::type;

  // find will return Key associated with the given type or assert failed
  template <typename Type, std::size_t Pos = 0>
  constexpr static Enum
  find () noexcept
  {
    constexpr bool ret = cmp<Type, static_cast<Enum> (Pos)> ();

    if constexpr (ret)
      return static_cast<Enum> (Pos);

    if constexpr (!ret && Pos + 1 < length)
      return find<Type, Pos + 1> ();

    if constexpr (Pos == length - 1)
      static_assert (ret, "the given type not exists");
  }

  // find_if will return an optional
  template <typename Type, std::size_t Pos = 0>
  constexpr static std::optional<Enum>
  find_if () noexcept
  {
    constexpr bool ret = cmp<Type, static_cast<Enum> (Pos)> ();

    if constexpr (ret)
      return static_cast<Enum> (Pos);

    if constexpr (!ret && Pos + 1 < length)
      return find_if<Type, Pos + 1> ();

    if constexpr (!ret && Pos == length - 1)
      return std::nullopt;
  }

  // for_each is used to traverse all types in an array and perform some
  // actions Func only receive the current type in umap
  template <template <typename> typename Func, std::size_t Pos = 0,
	    typename... Args>
  constexpr static void
  for_each (Args &&...args)
  {
    constexpr Enum key = static_cast<Enum> (Pos);

    if constexpr (Pos < length)
      std::invoke (Func<at<key> > (), std::forward<Args> (args)...);

    if constexpr (Pos + 1 < length)
      for_each<Func, Pos + 1> (std::forward<Args> (args)...);
  }

  // Func receive both the current type and key in umap
  template <template <typename, Enum> typename Func, std::size_t Pos = 0,
	    typename... Args>
  constexpr static void
  for_each (Args &&...args)
  {
    constexpr Enum key = static_cast<Enum> (Pos);

    if constexpr (Pos < length)
      std::invoke (Func<at<key>, key> (), std::forward<Args> (args)...);

    if constexpr (Pos + 1 < length)
      for_each<Func, Pos + 1> (std::forward<Args> (args)...);
  }
};

};