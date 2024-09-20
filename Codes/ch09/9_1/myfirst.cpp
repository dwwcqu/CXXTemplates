#include <iostream>
#include <typeinfo>
#include "myfirst.hpp"
// 模板的实现/定义
template<typename T>
void printTypeof(T const& x) {
    std::cout << typeid(x).name() << '\n';
}