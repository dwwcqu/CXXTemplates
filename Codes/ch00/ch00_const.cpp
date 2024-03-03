#include <iostream>

int main()
{
    int var                          = 10;   // int variable

    const int const_int              = 10;   // const int
    int const int_const              = 100;  // const int

    const int *ptr_const_0           = &var; // pointer to const int
    int const *ptr_const_1           = &var; // pointer to const int

    int *const const_ptr             = &var; // const pointer to int

    int const* const const_ptr_const = &var; // const pointer to const int

    if (typeid(const_int) != typeid(int_const))
    {
        std::cout << "`const int` and `int const` are different types!" << std::endl;
    }

    if (typeid(ptr_const_0) != typeid(ptr_const_1))
    {
        std::cout << "`const int*` and `int const*` are different types!" << std::endl;
    }

    if (typeid(const_ptr) == typeid(ptr_const_0))
    {
        std::cout << "`int* const` and `const int*` are the same type!" << std::endl;
    }

    if (typeid(const_ptr_const) == typeid(ptr_const_1))
    {
        std::cout << "`const int* const` and `const int*` are the same type!" << std::endl;
    }

    if (typeid(const_ptr_const) != typeid(const_ptr))
    {
        std::cout << "`const int* const` and `int* const` are the same type!" << std::endl;
    }

    return 0;
}