#include "../2_1/stack1.hpp"
#include <iostream>
#include <string>

int main()
{
    Stack<int>          intStack;       // stack of ints
    Stack<std::string>  stringStack;    // stack of strings

    // manipulate int stack;
    intStack.push(7);
    std::cout << intStack.top() << std::endl;

    // manipulate string stack
    stringStack.push("hello");
    std::cout << stringStack.top() << std::endl;
    stringStack.pop();

    return 0;
}