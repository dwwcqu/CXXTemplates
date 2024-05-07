#include <vector>
#include <cassert>

template <typename T, typename Cont = std::vector<T>>
class Stack
{
private:
    Cont elems;
public:
    // 插入元素到栈顶
    void push(T const& elem);
    // 删除栈顶元素
    void pop();
    // 返回栈顶元素
    T const& top() const;
    // 判断是否为空
    bool empty() const
    {
        return elems.empty();
    }
};

template <typename T, typename Cont>
void Stack<T, Cont>::push(T const& elem)
{
    elems.push_back(elem);
}

template <typename T, typename Cont>
void Stack<T, Cont>::pop()
{
    assert(!elems.empty());
    return elems.pop_back();
}

template <typename T, typename Cont>
T const& Stack<T, Cont>::top() const
{
    assert(!elems.empty());
    return elems.back();
}