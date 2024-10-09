#include <iostream>
#include <vector>
#include "foreach.hpp"

// 函数调用
void func(int i)
{
    std::cout << "func() called for: " << i << '\n';
}

// 函数对象类型（可以作为函数使用的对象）
class FuncObj
{
public:
    void operator()(int i) const    // 常量成员函数
    {
        std::cout << "FuncObj::op() called for: " << i << '\n';
    }
};

int main()
{
    std::vector<int> primes = {2, 3, 5,7, 11, 13, 17, 19};
    foreach(primes.begin(), primes.end(),   // 处理范围
        func);                              // 函数本身可调用的（退化为指针）

    foreach(primes.begin(), primes.end(),   // 处理范围
        &func);                             // 函数指针也是可调用的

    foreach(primes.begin(), primes.end(),   // 处理范围
        FuncObj());                         // 函数对象可调用的

    foreach(primes.begin(), primes.end(),   // 处理范围
        [](int i) {                         // lambda 表达式是可调用的
            std::cout << "lambda called for: " << i << '\n';
        });
}