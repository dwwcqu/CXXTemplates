#include <iostream>

namespace X
{
    template<typename T> void f(T);
} // namespace X

namespace N {
    using namespace X;
    enum E { e1 };
    void f(E) {
        std::cout << "N::f(N::E) called\n";
    }
}

void f(int) {
    std::cout << "::f(int) called\n";
}

int main()
{
    ::f(N::e1); // 限定函数名：没有 ADL
    f(N::e1);   // 普通查找到：::f() 和 ADL 查找到 N::f()，
                // 后面一个查找最为匹配
}
