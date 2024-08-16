#include <array>
#include <cassert>

template<typename T, std::size_t Maxsize>
class Stack {
private:
    std::array<T, Maxsize> elems;   // 元素
    std::size_t numElems;           // 当前元素个数
public:
    Stack();                        // 构造函数
    void push(T const& elem);       // 推入元素
    void pop();                     // 推出元素
    T const& top();                 // 返回栈顶元素
    bool empty() const {            // 返回当前栈是否为空
        return numElems == 0;
    }
    std::size_t size() const {      // 返回当前元素个数
        return numElems;
    }
};

template<typename T, std::size_t Maxsize>
Stack<T, Maxsize>::Stack()
    : numElems(0)
{
    // null
}

template<typename T, std::size_t Maxsize>
void Stack<T, Maxsize>::push(T const& elem)
{
    assert(numElems < Maxsize);
    elems[numElems] = elem;     // 插入元素到最后
    ++numElems;                 // 元素个数加1
}

template<typename T, std::size_t Maxsize>
void Stack<T, Maxsize>::pop()
{
    assert(!elems.empty());
    --numElems;                 // 元素个数减1
}

template<typename T, std::size_t Maxsize>
T const& Stack<T, Maxsize>::top()
{
    assert(!elems.empty());
    return elems[numElems - 1]; // 返回最后一个元素
}