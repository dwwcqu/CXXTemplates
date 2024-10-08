#include <utility>
#include <string>
#include <iostream>
#include <type_traits>

template <typename T>
using EnableIfString = std::enable_if_t<std::is_convertible_v<T, std::string>>;

class Person
{
private:
    std::string name;

public:
    // 传递初始值的一般构造 (1)
    template <typename STR, typename = EnableIfString<STR>>
    explicit Person(STR &&n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }
    // 拷贝构造 (2)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // 移动构造 (3)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE-CONSTR Person '" << name << "'\n";
    }
};