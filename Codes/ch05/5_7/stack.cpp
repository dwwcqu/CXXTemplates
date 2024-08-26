#include "stack.hpp"
#include <iostream>
#include <vector>

int main()
{
    Stack<int> iStack; // 整数栈
    Stack<float> fStack; // 浮点栈

    // 操作整数栈
    iStack.push(1);
    iStack.push(2);
    std::cout << "iStack.top(): " << iStack.top() << '\n';

    // 操作浮点栈
    fStack.push(3.3);
    std::cout << "fStack.top(): " << fStack.top() << '\n';

    // 赋值不同类型的栈并再次操作
    fStack = iStack;
    fStack.push(4.4);
    std::cout << "fStack.top(): " << fStack.top() << '\n';

    // 使用 vector 作为内部容器的双精度栈
    Stack<double, std::vector> vStack;
    vStack.push(5.5);
    vStack.push(6.6);
    std::cout << "vStack.top(): " << vStack.top() << '\n';
    vStack = fStack;
    std::cout << "vStack: ";
    while (!vStack.empty()) {
        std::cout << vStack.top() << ' ';
        vStack.pop();
    }
    std::cout << '\n';
}