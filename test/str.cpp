#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

int main()
{

    auto cstr = "hello world";
    std::string str(cstr, strlen(cstr));
    std::string_view strv(cstr, strlen(cstr) - 6);

    std::cout << strv.data();
}