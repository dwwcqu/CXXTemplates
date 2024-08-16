#include <iostream>

void print() 
{
}

template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    std::cout << firstArg << '\n'; // 打印第一个参数
    print(args...); // 对剩余的参数递归调用 print()
}