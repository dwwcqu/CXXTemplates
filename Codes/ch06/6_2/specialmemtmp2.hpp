#include <utility>
#include <string>
#include <iostream>

class Person
{
private:
    std::string name;

public:
    // 传递初始值的一般构造函数(1)
    template<typename STR>
    explicit Person(STR&& n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }
    // 拷贝和移动构造(2)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // (3)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE_CONSTR Person '" << name << "'\n";
    }
};