#include <iostream>
#include <memory>

// void pfree(int* ptr) noexcept
// {
//     delete ptr;
// }

struct Test {
    int a;
    double b;
};

int main()
{
    auto pa = new Test();
    std::unique_ptr<Test> upa(pa);
    delete pa;
    auto pb = new Test();
    // std::unique_ptr<Test> upb(pb);
    // pfree(pa);
}
