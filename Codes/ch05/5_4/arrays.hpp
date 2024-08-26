#include <iostream>

template<typename T>
struct MyClass;             // 主要的模板

template<typename T, std::size_t SZ>
struct MyClass<T[SZ]>       // 对已知边界数组的部分特化
{
    static void print()
    {
        std::cout << "print() for T[" << SZ << "]\n";
    }
};

template<typename T, std::size_t SZ>
struct MyClass<T(&)[SZ]>    // 对已知边界数组引用的部分特化
{
    static void print()
    {
        std::cout << "print() for T(&)[" << SZ << "]\n";
    }
};

template<typename T>
struct MyClass<T[]>         // 对未知边界数组的部分特化
{
    static void print()
    {
        std::cout << "print() for T[]\n";
    }
};

template<typename T>
struct MyClass<T(&)[]>      // 对未知边界数组引用的部分特化
{
    static void print()
    {
        std::cout << "print() for T(&)[]\n";
    }
};

template<typename T>
struct MyClass<T*>          // 对指针的部分特化
{
    static void print()
    {
        std::cout << "print() for T*\n";
    }
};