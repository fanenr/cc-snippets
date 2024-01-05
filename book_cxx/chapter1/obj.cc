#include <iostream>

struct data {
    int num;

    data(int n) : num { n }
    {
        std::cout << num << " ctor" << std::endl;
    }

    ~data()
    {
        std::cout << num << " dtor" << std::endl;
    }

    data(data const& other) : num { other.num }
    {
        std::cout << num << " ctor2" << std::endl;
    }

    data(data&& other) : num { other.num }
    {
        std::cout << num << " ctor3" << std::endl;
    }
};

void func(data obj)
{
    std::cout << obj.num << std::endl;
}

int main()
{
    func(data { 1 });
    std::cout << "end of main" << std::endl;
}