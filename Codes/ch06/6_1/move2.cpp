#include <utility>
#include <iostream>

class X
{
};

void g(X &)
{
    std::cout << "g() for variable\n";
}

void g(X const &)
{
    std::cout << "g() for constant\n";
}

void g(X &&)
{
    std::cout << "g() for movable object\n";
}

// 让 f() 进行 val 参数的完美转发到 g()
template <typename T>
void f(T &&val)
{
    g(std::forward<T>(val)); // 对于任何传递的参数 val，调用正确的 g()
}

int main()
{
    X v;             // 创建变量
    X const c;       // 创建常量
    f(v);            // 对于非常量对象，f(X&) => g(X&)
    f(c);            // 对于常量对象，f(X const&) => g(X const&)
    f(X());          // 对于临时对象 f(X&&) => g(X&&)
    f(std::move(v)); // 对于可移动变量 f(X&&) => g(X&&)
}