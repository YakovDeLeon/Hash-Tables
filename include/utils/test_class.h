#ifndef _test_class_h_
#define _test_class_h_
#include <iostream>
#include <algorithm>
struct test_class
{
    test_class() { std::cout << "test_class()\n"; }
    test_class(int x, int y) : x(x), y(y) { std::cout << "test_class(int, int)\n"; }
    ~test_class() { std::cout << "~test_class()\n"; }
    test_class(const test_class &c) : x(c.x), y(c.y) { std::cout << "test_class(const test_class &)\n"; }
    test_class(test_class &&c) : x(c.x), y(c.y) { std::cout << "test_class(test_class &&)\n"; }
    test_class &operator=(const test_class &c) {
        if (this != &c)
        {
            x = c.x;
            y = c.y;
            std::cout << "test_class &operator=(const test_class &)\n"; 
        }
        return *this;
    }
    test_class &operator=(test_class &&c) {
        swap(c);
        std::cout << "test_class &operator=(test_class &&)\n";
        return *this;
    }
    void swap(test_class& c) noexcept{
        std::cout << "void swap(test_class &)\n";
        std::swap(x, c.x);
        std::swap(y, c.y);
    }
    friend bool operator==(const test_class &l, const test_class &r);
    friend bool operator!=(const test_class &l, const test_class &r);
    int x = 100;
    int y = 100;
};

template <>
struct std::hash<test_class>
{
    size_t operator()(const test_class & obj) const
    {
        return std::hash<int>()(obj.x);
    }
};

template<>
void std::swap(test_class &l, test_class &r)
{
    l.swap(r);
}

bool operator==(const test_class &l, const test_class &r)
{
    return (l.x == r.x) && (l.y == r.y);
}

bool operator!=(const test_class &l, const test_class &r)
{
    return !(r == l);
}
#endif // !_test_class_h_