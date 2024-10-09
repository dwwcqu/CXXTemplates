template<typename T, T Z = T{}>
class RefMem
{
private:
    T zero;
public:
    RefMem() : zero{Z}{}
};

int null = 0;
int main()
{
    RefMem<int> rm1, rm2;
    rm1 = rm2;                  // OK

    RefMem<int&> rm3;           // ERROR: 非法的默认值
    RefMem<int&, 0> rm4;        // ERROR: 非法的默认值

    extern int null;
    RefMem<int&, null> rm5, rm6;
    rm5 = rm6;                  // ERROR: 对于引用成员，operator= 运算符是被 delete 掉的
}