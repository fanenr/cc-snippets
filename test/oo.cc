class Base {
public:
    Base();

    void print(int i) { }

    virtual void print(float j) { }
};

class Derive : public Base {
public:
    Derive() = default;

    void print(float j) { }
};

int main()
{
    Derive d {};
}
