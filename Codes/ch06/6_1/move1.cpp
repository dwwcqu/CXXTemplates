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

// 让 f() 转发参数 val 给 g()
void f(X &val)
{
    g(val); // val 是非常量左值 => 调用 g(X&)
}

void f(X const &val)
{
    g(val); // val 是常量左值 => 调用 g(X const&)
}

void f(X &&val)
{
    g(std::move(val)); // val 是非常量左值 => 需要 std::move() 去调用 g(X&&)
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