#include "../2_1/stack1.hpp"

// 为指针而实现的 Stack<> 的部分特例化

template <typename T>
class Stack<T*>
{
private:
    // 元素
    std::vector<T*> elems;
public:
    // 栈顶插入指针
    void push(T*);
    // 推出栈顶指针
    T* pop();
    // 返回栈顶元素
    T* top() const
    // 返回栈是否为空
    bool empty() const
    {
        return elems.empty();
    }
};

template <typename T>
void Stack<T*>::push(T* elem)
{
    elems.push_back(elem);
}

template <typename T>
T* Stack<T*>::pop()
{
    assert(!elems.empty());
    T* p = elems.back();
    elems.pop_back();
    return p;
}

template <typename T>
T* Stack<T*>::top() const
{
    assert(!elems.empty());
    return elems.back();
}