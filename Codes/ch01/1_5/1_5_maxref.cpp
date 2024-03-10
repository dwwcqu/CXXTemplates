#include <cstring>

// maximum of two values of any types(call-by-reference)
template<typename T>
T const& max(T const& a, T const& b) 
{
    return b < a ? a : b;
}

// maximum of two pointers
template<typename T>
T* max(T* a, T* b)
{
    return *b < *a ? a : b;
}

// maximum of two C-strings(call-by-value):
char const* max(char const* a, char const* b)
{
    return std::strcmp(b, a) < 0 ? a : b;
}

int main()
{
    int a = 7;
    int b = 42;
    auto m1 = ::max(a, b);      // max() for two values of int type

    std::string s1 = "hey";
    std::string s2 = "you";
    auto m2 = ::max(s1, s2);    // max() for two values of std::string type

    int* p1 = &b;
    int* p2 = &a;
    auto m3 = ::max(p1, p2);   // max() for two pointers

    char const* c1 = "hello";
    char const* c2 = "world";
    auto m4 = ::max(c1, c2);   // max() for two C-strings

    return 0;
}