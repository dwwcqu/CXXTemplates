#include "../2_1/stack1.hpp"
#include <deque>
#include <string>
#include <cassert>

template<>
class Stack<std::string>
{
private:
    std::deque<std::string> elems;          // 元素
public:
    void push(std::string const& elem);     // 插入一个元素
    void pop();                             // 推出一个元素
    std::string const& top() const;         // 返回栈顶元素
    bool empty() const                      // 返回栈是否为空
    {
        return elems.empty();
    }
};

void Stack<std::string>::push(std::string const& elem)
{
    elems.push_back(elem);                  // 将传递的 elem 元素的拷贝进行插入
}

void Stack<std::string>::pop()
{
    assert(!elems.empty());
    elems.pop_back();                       // 移除栈底元素
}

std::string const& Stack<std::string>::top() const
{
    assert(!elems.empty());
    return elems.back();                    // 返回栈顶元素的引用
}