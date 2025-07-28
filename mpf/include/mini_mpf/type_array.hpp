#ifndef TYPE_ARRAY_H
#define TYPE_ARRAY_H

#include <cstddef>
#include <optional>
#include <type_traits>

namespace mini_mpf
{

template <typename... Types>
class type_array
{
private:
  constexpr static size_t size = sizeof...(Types);

  template <size_t Index, typename... Other>
  struct at_impl;

  template <typename T, typename... Other>
  struct at_impl<0, T, Other...>
  {
    using type = T;
  };

  template <size_t Index, typename T, typename... Other>
  struct at_impl<Index, T, Other...>
  {
    using type = typename at_impl<Index - 1, Other...>::type;
  };

  template <template <typename...> typename T>
  struct forward_impl
  {
    using type = T<Types...>;
  };

public:
  template <size_t Index>
  using at = typename at_impl<Index, Types...>::type;

  template <template <typename...> typename T>
  using forward = typename forward_impl<T>::type;

  constexpr static size_t
  len () noexcept
  {
    return size;
  }

  template <typename T, size_t Pos>
  constexpr static bool
  cmp () noexcept
  {
    return std::is_same_v<at<Pos>, T>;
  }

  template <typename Type, size_t Pos = 0>
  constexpr static size_t
  find () noexcept
  {
    constexpr bool ret = cmp<Type, Pos> ();

    if constexpr (!ret && Pos + 1 >= size)
      static_assert (false, "type not found");

    if constexpr (!ret)
      return find<Type, Pos + 1> ();

    return Pos;
  }

  template <typename Type, size_t Pos = 0>
  constexpr static std::optional<size_t>
  find_if () noexcept
  {
    constexpr bool ret = cmp<Type, Pos> ();

    if constexpr (!ret && Pos + 1 >= size)
      return std::nullopt;

    if constexpr (!ret && Pos + 1 < size)
      return find_if<Type, Pos + 1> ();

    return Pos;
  }
};

};

#endif
