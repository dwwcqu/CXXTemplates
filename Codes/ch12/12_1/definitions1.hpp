template<typename T>        // 一个命名空间范围内的类模板
class Data {
public:
    static constexpr bool copyable = true;
    ....
};

template<typename T>        // 一个命名空间范围内的函数模板
void log(T x) {
    ...
}

template<typename T>        // 一个命名空间范围内的变量模板（从C++14开始支持）
T zero = 0;


template<typename T>        // 一个命名空间范围内的变量模板（从C++14开始支持）
bool dataCopyable = Data<T>::copyable;

template<typename T>        // 一个命名空间范围内的别名模板
using DataList = Data<T*>;