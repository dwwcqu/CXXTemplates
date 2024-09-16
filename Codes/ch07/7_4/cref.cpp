#include <functional>
#include <string>
#include <iostream>

void printString(std::string const& s)
{
    std::cout << s << '\n';
}

template<typename T>
void printT(T arg)
{
    printString(arg);
}

int main()
{
    std::string s = "hello";
    printT(s);                  // 输出一个值传递
    printT(std::cref(s));       // 打印一个按 “引用传递” 的值
}