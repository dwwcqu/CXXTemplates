template<typename T>        // 一个命名空间范围内的类模板
class List {
public:
    List() = default;       // 模板构造函数定义

    template<typename U>    // 一个成员类模版，没有定义
    class Handle;

    template<typename U>    // 一个成员函数模板（构造函数）
    List(List<U> const&);

    template<typename U>    // 一个成员变量模板（自C++14开始支持）
    static U zero;
};

template<typename T>        // 在类外面的成员类模板定义
    template<typename U>
class List<T>::Handle {
    ...
};

template<typename T>        // 在类外面的成员函数模板定义
    template<typename T2>
List<T>::List(List<T2> const& b) {
    ...
};

template<typename T>        // 在类外面的静态数据成员模板定义
    template<typename U>
U List<T>::zero = 0;