#include <cstring>
#include <iostream>
class cstring {

private:
    char* ptr;

public:
    cstring()
        : ptr(nullptr)
    {
    }

    cstring(char const* str)
        : ptr(new char[strlen(str)])
    {
        strcpy(ptr, str);
    }

    ~cstring()
    {
        delete[] ptr;
    }

    friend std::istream& operator>>(std::istream& in, cstring& str)
    {

        return in;
    }
};