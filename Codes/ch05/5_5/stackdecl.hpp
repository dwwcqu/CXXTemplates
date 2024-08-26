template <typename T>
class Stack
{
private:
    std::deque<T> elems; // 元素
public:
    void push(T const &); // 压入元素
    void pop();           // 弹出元素
    T const &top() const; // 返回顶部元素
    bool empty() const
    { // 返回栈是否为空
        return elems.empty();
    }

    // 为类型为 T2 的元素的栈定义赋值操作符
    template <typename T2>
    Stack &operator=(Stack<T2> const &);
};