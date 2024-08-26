#include <utility>
#include <string>
#include <iostream>

class Person
{
private:
    std::string name;

public:
    // 传递初始值的构造函数(1)
    explicit Person(std::string const &n) : name(n)
    {
        std::cout << "copying string-CONSTR for '" << name << "'\n";
    }
    // (2)
    explicit Person(std::string &&n) : name(std::move(n))
    {
        std::cout << "moving string-CONSTR for '" << name << "'\n";
    }
    // 拷贝和移动构造(3)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // (4)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE_CONSTR Person '" << name << "'\n";
    }
};

int main()
{
    std::string s = "sname";
    Person p1(s);             // 用字符串对象初始化 => (1)
    Person p2("tmp");         // 用字符串字面值初始化 => (2)
    Person p3(p1);            // 拷贝构造 => (3)
    Person p4(std::move(p1)); // 移动构造 => (4)
}