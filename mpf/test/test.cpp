#include <iostream>
#include <mini_mpf/type_array.hpp>
#include <mini_mpf/type_umap.hpp>
#include <type_traits>

using namespace mini_mpf;

static void
test_array ()
{
  using arr = type_array<int, double, float &>;

  static_assert (3 == arr::len (), "bad");
  static_assert (arr::cmp<float &, 2> (), "bad");
  static_assert (std::is_same_v<float &, arr::at<2> >, "bad");

  using tuple = std::tuple<int, double, float &>;
  static_assert (std::is_same_v<tuple, arr::forward<std::tuple> >, "bad");

  static_assert (arr::find<float &> () == 2, "bad");
  static_assert (!arr::find_if<float> (), "bad");
}

template <typename T> struct tra_umap
{
  void
  operator() (double n)
  {
    if constexpr (std::is_same_v<T, int>)
      std::cout << "traverse umap with given arguments: int" << n << "\n";
    if constexpr (std::is_same_v<T, float>)
      std::cout << "traverse umap with given arguments: float" << n << "\n";
    if constexpr (std::is_same_v<T, double>)
      std::cout << "traverse arr with given arguments: double" << n << "\n";
  }
};

enum class Num
{
  i32,
  f32,
  f64
};

template <typename T, Num K> struct tra_umap_k
{
  void
  operator() ()
  {
    if constexpr (K == Num::i32)
      std::cout << "traverse umap with key: i32 int"
                << "\n";
    if constexpr (K == Num::f32)
      std::cout << "traverse umap with key: f32 float"
                << "\n";
    if constexpr (K == Num::f64)
      std::cout << "traverse umap with key: f64 double"
                << "\n";
  }
};

static void
test_umap ()
{
  using umap = type_umap<Num, int, float, double>;
  using arr = umap::array;
  using umap2 = type_umap<Num, arr>;

  static_assert (std::is_same_v<float, umap::at<Num::f32> >, "bad");

  static_assert (3 == umap::len (), "bad");

  static_assert (umap::cmp<float, Num::f32> (), "bad");

  using tuple = std::tuple<int, float, double>;
  static_assert (std::is_same_v<tuple, umap::forward<std::tuple> >, "bad");

  static_assert (!umap2::find_if<float &> (), "bad");
  static_assert (umap::find<float> () == Num::f32, "bad");
  static_assert (umap2::find<double> () == Num::f64, "bad");
}

int
main ()
{
  test_array ();
  test_umap ();
}