#include <iostream>
#include <memory>
#include <exception>
#include <stdexcept>

class exception {
private:
    int level;

public:
    exception() : level(1)
    {
        std::cout << "created" << std::endl;
    }

    exception(exception const& other) : level(other.level + 1)
    {
        std::cout << "copied" << std::endl;
    }

    exception(exception&& other) : level(other.level + 1)
    {
        std::cout << "moved" << std::endl;
    }

    void show() const
    {
        std::cout << "level: " << level << std::endl;
    }

    virtual ~exception()
    {
        std::cout << "destoried: " << level << std::endl;
    }
};

void func()
{
    throw exception();
}

void indirection()
{
    // func();
    try {
        func();
    } catch (exception const& e) {
        throw e;
    }
}

int main()
{
    try {
        // func();
        indirection();
    } catch (exception const& e) {
        e.show();
    }
}