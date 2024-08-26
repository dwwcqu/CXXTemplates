template <typename T>
template <typename T2>
Stack<T> &Stack<T>::operator=(Stack<T2> const &op2)
{
    Stack<T2> tmp(op2); // 创建被赋值栈的副本
    elems.clear();      // 清除现有元素
    while (!tmp.empty())
    { // 复制所有元素
        elems.push_front(tmp.top());
        tmp.pop();
    }
    return *this;
}