class Collection {
public:
    template<typename T>    // 类内部成员类模板定义
    class Node {
        ...
    };

    template<typename T>    // 类内部（或者成为隐式 inline）成员函数模板定义
    T *alloc() {
        ...
    }

    template<typename T>    // 一个成员变量模板（自C++14开始支持）
    static T zero = 0;

    template<typename T>    // 一个类内部的成员别名模板
    using NodePtr = Node<T>*; 
};