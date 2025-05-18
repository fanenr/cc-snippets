#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory_resource>
#include <string>
#include <vector>

struct T
{
  int32_t arr[10];

  T () {}

  ~T () { std::cout << "destroied" << std::endl; }
};

int
main ()
{
  std::array<std::byte, 1024 * 100> buf;
  std::pmr::monotonic_buffer_resource pool (buf.data (), buf.size ());

  {
    std::pmr::vector<T> vec (std::pmr::new_delete_resource ());
    for (int i = 0; i < 10; ++i)
      vec.emplace_back ();
    vec.clear ();
  }

  std::cout << "end" << std::endl;
}