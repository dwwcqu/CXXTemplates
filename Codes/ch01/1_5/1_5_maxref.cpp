#include <cstring>

// 返回任意类型两个值的最大值，通过引用调用
template<typename T>
T const& max (T const& a, T const &b)
{
    return b < a ? a : b;
}

// 两个 C-格式 字符串的最大值，通过值传递
char const* max (char const* a, char const* b)
{
    return std::strcmp(b, a) < 0 ? a : b;
}

// 三个任意类型值的最大值，通过引用调用
template<typename T>
T const& max (T const& a, T const& b, T const& c)
{
    return max(max(a, b), c);
}

int main()
{
    auto m1 = ::max(7, 42, 68);     // OK

    char const* s1 = "frederic";
    char const* s2 = "anica";
    char const* s3 = "lucas";
    auto m2 = ::max(s1, s2, s3);    // ERROR
}
