#include <iostream>
#include <string>
#include "stackauto.hpp"

int main()
{
    Stack<int, 20u>         int20Stack;     // 20 int 栈
    Stack<std::string, 40>  stringStack;    // 40 std::string 栈

    int20Stack.push(7);
    std::cout << int20Stack.top() << std::endl;
    auto size1 = int20Stack.size();

    stringStack.push("hello");
    std::cout << stringStack.top() << std::endl;
    auto size2 = stringStack.size();

    if(!std::is_same_v<decltype(size1), decltype(size2)>)
    {
        std::cout << "size types differ" << std::endl;
    }
    return 0;
}