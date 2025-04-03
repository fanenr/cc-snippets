# mini_mpf is a header-only metaprograming framework

1. Prerequisites
```
1. C++17 support
2. CMake(3.20 or later)
```

2. Run test
``` shell
mkdir build && cd build
cmake .. && make run_t
```

3. Demo
``` C++
#include <mini_mpf/type_array.hpp>

int main()
{
    using types = type_array<int, float&, double>;
    std::cout << "size of types: " << sizeof(types) << '\n';

    // get length
    constexpr int len = types::len();
    static_assert(len == 3, "bad");

    // get type
    types::at<2> num = 1.0;
    static_assert(std::is_same_v<decltype(num), double>, "bad");

    // compare
    static_assert(types::cmp<float&, 1>(), "bad");

    // forward types
    float f = 1;
    types::forward<std::tuple> tuple { 1, f, 1.0 };
    using tuple_t = std::tuple<int, float&, double>;
    static_assert(std::is_same_v<decltype(tuple), tuple_t>, "bad");
}
```