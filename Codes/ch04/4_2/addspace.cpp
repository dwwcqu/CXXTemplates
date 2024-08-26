template<typename T>
class AddSpace
{
private:
    T const& ref;       // 参数引用
public:
    AddSpace(T const& r) : ref(r)
    {}
    friend std::ostream& operator<< (std::ostream& os, AddSpace<T> s)
    {
        // 输出传递的参数和一个空格
        return os << s.ref << ' ';
    }
};

template<typename... Args>
void print(Args... args)
{
    ( std::cout << ... << AddSpace(args) ) << '\n';
}