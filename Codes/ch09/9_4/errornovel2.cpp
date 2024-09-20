#include <string>
#include <unordered_set>

class Customer {
private:
    std::string name;
public:
    Customer (std::string const& n) : name(n) {}
    std::string getName() const {
        return name;
    }
};

int main() {
    // 提供我们自己的哈希函数：
    struct MyCustomerHash {
        // 注意：缺少 const 只有在 g++ 和 clang 中会报错：
        std::size_t operator() (Customer const& c) {
            return std::hash<std::string>()(c.getName());
        }
    };

    // 使用它为 Customer 创建哈希表：
    std::unordered_set<Customer, MyCustomerHash> coll;
}