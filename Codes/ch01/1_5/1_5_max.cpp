// maximum of two int values
int max(int a, int b)
{
    return b < a ? a : b;
}

// maximum of two values of any type:
template<typename T>
T max(T a, T b)
{
    return b < a ? a : b;
}

int main()
{
    ::max(7, 42);           // 调用类型为 int 非模板函数
    ::max(7.0, 42.0);       // 调用 max<double> 模板函数，推导类型
    ::max('a', 'b');        // 调用 max<char> 模板函数，推导类型
    ::max<>(7, 42);         // 调用 max<int> 模板函数，推导类型
    ::max<double>(7, 42);   // 调用 max<double> 模板函数，没有推导 
    ::max('a', 42.7);       // 调用类型为 int 的非模板函数
}