#include <string>
#include <unordered_set>

class Customer
{
private:
    std::string name;
public:
    Customer(std::string const& n) : name(n){}
    std::string getName() const { return name;}
};

struct CustomerEq
{
    bool operator()(Customer const& c1, Customer const& c2) const
    {
        return c1.getName() == c2.getName();
    }
};

struct CustomerHash
{
    std::size_t operator()(Customer const& c) const
    {
        return std::hash<std::string>()(c.getName());
    }
};

// 定义类：对可变基类的 operator() 进行组合
template<typename... Bases>
struct Overloader : Bases...
{
    using Bases::operator()...;     // 自 C++ 17 开始 OK
};

int main()
{
    using CustomerOP = Overloader<CustomerHash, CustomerEq>;
    std::unordered_set<Customer, CustomerHash, CustomerEq> coll1;
    std::unordered_set<Customer, CustomerOP, CustomerOP> coll2;
}