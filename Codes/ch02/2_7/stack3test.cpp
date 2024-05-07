#include "stack3.hpp"
#include <iostream>
#include <deque>

int main()
{
    // int 栈
    Stack<int> intStack;

    // 使用 std::deque<> 管理元素的 double 类型栈
    Stack<double, std::deque<double>> dblStack;

    // int 类型栈操作
    intStack.push(7);
    std::cout << intStack.top() << std::endl;
    intStack.pop();

    // double 类型栈操作
    dblStack.push(42.42);
    std::cout << dblStack.top() << std::endl;
    dblStack.pop();
    
    return 0;
}