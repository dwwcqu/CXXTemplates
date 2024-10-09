#include <vector>
#include <iostream>

template<typenmae T, int& SZ>   // SZ 为一个引用
class Arr
{
private:
    std::vector<T> elems;
public:
    Arr() : elems(SZ){}         // 使用 SZ 初始化向量大小
    void print() const
    {
        for(int i = 0; i < SZ, ++i)
        {
            // 循环遍历 SZ 个元素
            std::cout << elems[i] << ' ';
        }
    }
};

int size = 10;

int main()
{
    Arr<int&, size> y;  // 编译时 ERROR

    Arr<int, size> x;   // 初始化里面 10 个元素
    x.print();          // OK
    size += 100;        // 修改了 Arr<> 中的 SZ 大小
    x.print();          // 运行时 ERROR：数组原本只有 10 个元素，修改 SZ 后变成了 120 个，非法访问
}