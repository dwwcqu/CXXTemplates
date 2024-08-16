#include "stacknontype.hpp"
#include <iostream>
#include <string>
int main()
{
    Stack<int, 20> int20Stack;          // 20 个 int 元素栈
    Stack<int, 40> int40Stack;          // 40 个 int 元素栈
    Stack<std::string, 40> stringStack; // 40 个 string 元素栈
    
    int20Stack.push(7);
    std::cout << int20Stack.top() << '\n';
    int20Stack.pop();
    
    stringStack.push("hello");
    std::cout << stringStack.top() << '\n';
    stringStack.pop();
}