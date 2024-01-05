#include "extern.h"
#include <limits>
#include <format>
#include <cstddef>
#include <iostream>
#include <optional>

using namespace arthur;
namespace tool = ::util;

std::string tool::str_hello()
{
    return "hello";
}

void func() { }

// [[maybe_unused]] int get_int();
[[nodiscard]] std::size_t get_int([[maybe_unused]] std::size_t len);
class [[nodiscard]] A { };
enum class [[nodiscard]] B { a };

int main()
{
    using std::format;
    using namespace tool;
    using limit = std::numeric_limits<float>;

    std::cout << format("{}\n", str_hello());

    std::cout << format("float max: {}, min: {}, lowest: {}\n", limit::max(),
                        limit::min(), limit::lowest());

    auto size = sizeof(std::optional<int>);

    auto resu = get_int(-1);

    std::cout << std::format("-1 to unsigned: {}\n", resu);
}

std::size_t get_int(std::size_t len)
{
    return len;
}

namespace Ab {

namespace BB {

}

} // namespace Ab