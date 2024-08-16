#include <iostream>

template<auto T>
class Message
{
public:
    void print()
    {
        std::cout << T << '\n';
    }
};
char const s2[] = "world!";
int main()
{
    Message<42> msg1;
    msg1.print();

    static char const s1[] = "hello";
    Message<s1> msg2 ;
    msg2.print();

    Message<s2> msg3;
    msg3.print();
    return 0;
}