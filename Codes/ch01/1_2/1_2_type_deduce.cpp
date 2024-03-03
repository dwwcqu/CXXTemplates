#include <iostream>

template<typename T>
T max(T a, T b)
{
    return b < a ? a : b;
}

int main()
{
    int i = 33;
    int const j = 44;

    int& ia = i;
    max(i, i);                          // OK: T 推导为 int

    max(i, j);                          // OK: T 推导为 int

    int arr[4];
    max(&i, arr);                       // OK: T 推导为 int

    // max(4, 7.2);                        // ERROR: T 推导为 int 或者 double

    std::string s = "hello";
    // max("hello", s);                    // ERROR: T 推导为 char const[6] 或 std::string

    max(static_cast<double>(4), 7.2);   // OK

    max<double>(4, 7.2);                // OK

    return 0;
}