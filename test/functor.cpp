#include <cstdint>
#include <functional>
#include <iostream>

struct T {
    int a;
    int func()
    {
        return a + 1;
    }
};

using func = std::function<int(T*)>;

int main() { }