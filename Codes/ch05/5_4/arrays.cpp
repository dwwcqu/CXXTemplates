#include "arrays.hpp"

template <typename T1, typename T2, typename T3>
void foo(int a1[7], int a2[], // 指针
         int (&a3)[42],       // 已知边界数组的引用
         int (&x0)[],         // 未知边界数组的引用
         T1 x1,               // 值退化传递
         T2 &x2, T3 &&x3)     // 引用传递
{
    MyClass<decltype(a1)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(a2)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(a3)>::print(); // 使用 MyClass<T(&)[SZ]>
    MyClass<decltype(x0)>::print(); // 使用 MyClass<T(&)[]>
    MyClass<decltype(x1)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(x2)>::print(); // 使用 MyClass<T(&)[]>
    MyClass<decltype(x3)>::print(); // 使用 MyClass<T(&)[]>
}

int main()
{
    int a[42];
    MyClass<decltype(a)>::print(); // 使用 MyClass<T[SZ]>

    extern int x[];                // 前向声明
    MyClass<decltype(x)>::print(); // 使用 MyClass<T[]>

    foo(a, a, a, x, x, x, x);
}

int x[] = {0, 8, 15};               // 定义前向声明的数组