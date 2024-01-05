#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

struct data {
    int a, b, c;

private:
    int d;
};

void fn()
{
    std::array arr { 1, 2, 3 };
    auto const& [x, y, z] = arr;

    std::vector<int> vec(2, 0);

    // auto [a, b, c] = data();
}