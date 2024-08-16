#include <iostream>

template<typename T>
void print(T arg)
{
    std::cout << arg << '\n'; // 打印传递的参数
}

template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    print(firstArg); // 调用 print() 处理第一个参数
    print(args...); // 调用 print() 处理剩余的参数
}