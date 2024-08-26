#include "specialmemtmpl.hpp"

int main()
{
    std::string s = "sname";
    Person p1(s);               // 字符串对象初始化 => (1)
    Person p2("tmp");           // 字符串字面值初始化 => (1)
    Person p3(p1);              // OK => 拷贝构造 (2)
    Person p4(std::move(p1));   // OK => 移动构造 (3)
}