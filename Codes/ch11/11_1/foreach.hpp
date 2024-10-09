template<typename Iter, typename Callable>
void foreach (Iter current, Iter end, Callable op)
{
    while(current != end)   // 只要没有遍历到最后一个元素
    {
        op(*current);       // 对每一个遍历的元素调用传递的函数对象
        ++current;          // 向下一个元素移动
    }
}