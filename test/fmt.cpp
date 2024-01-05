#include <cstddef>
#include <cstdio>
#include <format>
#include <utility>

template <typename... Args>
constexpr auto println(std::format_string<Args...> fmt, Args&&... args) -> void
{
    auto&& str =
        std::format(std::move(fmt), std::forward<Args>(args)...) + '\n';
    for (size_t n = 0; n < str.size();)
        n += std::fwrite(str.data() + n, sizeof(char), str.size(), stdout);
}

int main()
{
    double num = 3.1415;
    std::reference_wrapper<double> ref(num);

    auto str = "hello";
}