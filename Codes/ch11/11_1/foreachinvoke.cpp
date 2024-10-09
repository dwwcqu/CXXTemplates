#include <iostream>
#include <vector>
#include <string>
#include "foreachinvoke.hpp"

// 成员函数会被调用的类
class MyClass
{
public:
    void memfunc(int i) const
    {
        std::cout << "MyClass()::memfunc() called for: " << i << '\n';
    }
};

int main()
{
    std::vector<int> primes = {2, 3, 5, 7, 11, 13, 17, 19};
    // 传递 lambda 作为函数体，并传递一个额外参数
    foreach(primes.begin(), primes.end(),       // 作为 lambda 表达式的第二个参数
        [](std::string const prefix, int i){    // lambda
        std::cout << prefix << i << '\n';
    }, "- value: ");                            // lambda 表达式的第一个参数

    // 对 primes 中的每个元素调用 obj.memfunc()
    MyClass obj;
    foreach(primes.begin(), primes.end(),       // 作为参数使用的元素
        &MyClass::memfunc,                      // 被调用的成员函数
        obj);                                   // 调用 memfunc() 的对象
}