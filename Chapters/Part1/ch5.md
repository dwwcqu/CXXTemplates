# 5. 模板基础进阶

本章涵盖了一些与模板实际使用相关的进一步基础内容：`typename` 关键字的额外用法，将成员函数和嵌套类定义为模板，模板化模板参数，零初始化，以及使用字符串字面量作为函数模板参数的一些细节。这些方面有时可能会有些棘手，但每个日常编程人员都应该了解这些内容。

## 5.1. `typename` 关键字

关键词 `typename` 是在 C++ 标准化过程中引入的，用于明确模板中标识符是一个类型。考虑以下示例：

```cpp
template<typename T>
class MyClass {
public:
    ...
    void foo() {
        typename T::SubType* ptr;
    }
};
```

在这里，第二个 `typename` 用于明确 `SubType` 是类 `T` 中定义的类型。因此，`ptr` 是一个指向类型 `T::SubType` 的指针。

如果没有 `typename`，`SubType` 会被假定为一个非类型成员（例如静态数据成员或枚举常量）。因此，表达式 `T::SubType* ptr` 会被视为 `T` 类的静态成员 `SubType` 与 `ptr` 的乘积，这在某些 `MyClass<>` 的实例化中可能是有效的代码，不会报错。

一般来说，`typename` 必须在依赖于模板参数的名称是类型时使用。关于这个问题的详细讨论见第 13.3.2 节。

`typename` 的一个应用是用于在泛型代码中声明标准容器的迭代器：

```cpp
#include <iostream>

// 打印 STL 容器中的元素
template<typename T>
void printcoll (T const& coll) {
    typename T::const_iterator pos; // 迭代器用于遍历 coll
    typename T::const_iterator end(coll.end()); // 结束位置
    for (pos = coll.begin(); pos != end; ++pos) {
        std::cout << *pos << ' ';
    }
    std::cout << '\n';
}
```

在这个函数模板中，传递的参数是类型为 `T` 的标准容器。为了遍历容器中的所有元素，使用了容器的迭代器类型，该类型在每个标准容器类中定义为 `const_iterator` 类型：

```cpp
class stlcontainer {
public:
    using iterator = ...; // 用于读写访问的迭代器
    using const_iterator = ...; // 用于只读访问的迭代器
    ...
};
```

因此，要访问模板类型 `T` 的 `const_iterator` 类型，必须在其前面加上 `typename`：

```cpp
typename T::const_iterator pos;
```

有关在 C++17 之前需要使用 `typename` 的更多细节，请参见第 13.3.2 节。需要注意的是，C++20 很可能会在许多常见情况下移除 `typename` 的必要性（详情参见第 17.1 节）。

### 5.2. 零初始化

对于如 `int`、`double` 或指针类型这样的基础类型，没有默认构造函数会为它们提供有用的默认值。相反，任何未初始化的局部变量将具有未定义的值：

```cpp
void foo() {
    int x; // x 的值未定义
    int* ptr; // ptr 指向任意位置（而不是空指针）
}
```

现在，如果你编写模板，并希望模板类型的变量被初始化为默认值，简单的定义对于内置类型不会这样做：

```cpp
template<typename T>
void foo() {
    T x; // 如果 T 是内置类型，则 x 的值未定义
}
```

因此，可以显式调用默认构造函数，为内置类型初始化为零（对于 `bool` 为 `false`，对于指针为 `nullptr`）。因此，即使是内置类型，也可以通过编写以下代码来确保适当的初始化：

```cpp
template<typename T>
void foo() {
    T x{}; // 如果 T 是内置类型，则 x 为零（或 false）
}
```

这种初始化方式称为值初始化，即调用提供的构造函数或零初始化对象。即使构造函数是 `explicit`，这种方法也能正常工作。

在 C++11 之前，确保正确初始化的语法是：

```cpp
T x = T(); // 如果 T 是内置类型，则 x 为零（或 false）
```

在 C++17 之前，这种机制（仍然支持）仅在选择的复制初始化构造函数不是显式时有效。在 C++17 中，强制复制省略消除了这一限制，任何一种语法都可以工作，但**大括号初始化的语法如果没有可用的默认构造函数，可以使用初始化列表构造函数[^1]**。

[^1]: 该类构造函数的参数类型为 `std::initializer_list<X>`。

**为了确保类模板的某个成员（其类型是参数化的）被初始化，可以定义一个使用大括号初始化成员的默认构造函数**：

```cpp
template<typename T>
class MyClass {
private:
    T x;
public:
    MyClass() : x{} { // 确保即使对于内置类型 x 也被初始化
    }
    ...
};
```

C++11 之前的语法：

```cpp
MyClass() : x() { // 确保即使对于内置类型 x 也被初始化
}
```

也仍然有效。

自 C++11 起，你还可以为非静态成员提供默认初始化，这样以下方式也是可行的：

```cpp
template<typename T>
class MyClass {
private:
    T x{}; // 零初始化 x，除非另有指定
    ...
};
```

然而，注意默认参数不能使用该语法。例如：

```cpp
template<typename T>
void foo(T p{}) { // 错误
    ...
}
```

相反，我们必须写成：

```cpp
template<typename T>
void foo(T p = T{}) { // 可以（在 C++11 之前必须使用 T()）
    ...
}
```

## 5.3. 使用 `this->`

对于**依赖模板参数的基类**来说，在类模板中使用一个名称 `x` 本身并不总是等同于 `this->x`，即使该成员 `x` 是继承而来的。例如：

```cpp
template<typename T>
class Base {
public:
    void bar();
};

template<typename T>
class Derived : Base<T> {
public:
    void foo() {
        bar(); // 调用外部的 bar() 或者报错
    }
};
```

在这个示例中，为了在 `foo()` 中解析符号 `bar`，`Base` 中定义的 `bar()` 从未被考虑。因此，要么会发生错误，要么会调用另一个 `bar()`（比如全局的 `bar()`）。

我们在第 13.4.2 节中详细讨论了这个问题。目前，作为经验法则，我们建议对于**任何在基类中声明的、依赖模板参数的符号**，都使用 `this->` 或 `Base<T>::` 进行**限定**。

### 5.4. 用于原始数组和字符串字面值的模板

在将原始数组或字符串字面量传递给模板时，需要特别注意。首先，如果模板参数声明为引用类型，传递的参数不会发生衰减。也就是说，传递一个 `"hello"` 参数时，其类型为 `char const[6]`。当传递长度不同的原始数组或字符串参数时，这可能会成为一个问题，因为它们的类型不同。只有当参数按值传递时，类型才会衰减，因此字符串字面量会被转换为 `char const*` 类型。这在第 7 章中有详细讨论。

值得注意的是，你还可以提供专门处理原始数组或字符串字面量的模板。例如：

```cpp
template<typename T, int N, int M>
bool less (T(&a)[N], T(&b)[M]) {
    for (int i = 0; i < N && i < M; ++i) {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return N < M;
}
```

在这里，当调用以下代码时：

```cpp
int x[] = {1, 2, 3};
int y[] = {1, 2, 3, 4, 5};
std::cout << less(x, y) << '\n';
```

`less<>` 被实例化为 `T` 为 `int`，`N` 为 3，`M` 为 5。

你还可以将此模板用于字符串字面量：

```cpp
std::cout << less("ab", "abc") << '\n';
```

在这种情况下，`less<>` 被实例化为 `T` 为 `char const`，`N` 为 3，`M` 为 4。

如果你只想为字符串字面量（和其他 `char` 数组）提供一个函数模板，可以这样做：

```cpp
template<int N, int M>
bool less (char const(&a)[N], char const(&b)[M]) {
    for (int i = 0; i < N && i < M; ++i) {
        if (a[i] < b[i]) return true;
        if (b[i] < a[i]) return false;
    }
    return N < M;
}
```

注意，对于未知边界的数组，你可以且有时必须**进行重载或部分特化**。以下程序展示了所有可能的数组重载：[arrays.hpp](../../Codes/ch05/5_4/arrays.hpp)

```cpp
#include <iostream>

template<typename T>
struct MyClass;             // 主要的模板

template<typename T, std::size_t SZ>
struct MyClass<T[SZ]>       // 对已知边界数组的部分特化
{
    static void print()
    {
        std::cout << "print() for T[" << SZ << "]\n";
    }
};

template<typename T, std::size_t SZ>
struct MyClass<T(&)[SZ]>    // 对已知边界数组引用的部分特化
{
    static void print()
    {
        std::cout << "print() for T(&)[" << SZ << "]\n";
    }
};

template<typename T>
struct MyClass<T[]>         // 对未知边界数组的部分特化
{
    static void print()
    {
        std::cout << "print() for T[]\n";
    }
};

template<typename T>
struct MyClass<T(&)[]>         // 对未知边界数组引用的部分特化
{
    static void print()
    {
        std::cout << "print() for T(&)[]\n";
    }
};

template<typename T>
struct MyClass<T*>              // 对指针的部分特化
{
    static void print()
    {
        std::cout << "print() for T*\n";
    }
};
```

在这里，类模板 `MyClass<>` 被特化为各种类型：已知和未知边界的数组、对已知和未知边界数组的引用、以及指针。每种情况都是不同的，并且可能在使用数组时出现：[arrays.cpp](../../Codes/ch05/5_4/arrays.cpp)

```cpp
#include "arrays.hpp"

template <typename T1, typename T2, typename T3>
void foo(int a1[7], int a2[], // 指针
         int (&a3)[42],       // 已知边界数组的引用
         int (&x0)[],         // 未知边界数组的引用
         T1 x1,               // 值退化传递
         T2 &x2, T3 &&x3)     // 引用传递
{
    MyClass<decltype(a1)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(a2)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(a3)>::print(); // 使用 MyClass<T(&)[SZ]>
    MyClass<decltype(x0)>::print(); // 使用 MyClass<T(&)[]>
    MyClass<decltype(x1)>::print(); // 使用 MyClass<T*>
    MyClass<decltype(x2)>::print(); // 使用 MyClass<T(&)[]>
    MyClass<decltype(x3)>::print(); // 使用 MyClass<T(&)[]>
}

int main()
{
    int a[42];
    MyClass<decltype(a)>::print(); // 使用 MyClass<T[SZ]>

    extern int x[];                // 前向声明
    MyClass<decltype(x)>::print(); // 使用 MyClass<T[]>

    foo(a, a, a, x, x, x, x);
}

int x[] = {0, 8, 15};               // 定义前向声明的数组
```

需要注意的是，由语言规则声明为数组（无论是否有长度）的调用参数实际上具有指针类型。另外，未知边界数组的模板可以用于不完整类型，例如：

```cpp
extern int i[];
```

当通过引用传递时，它变成了 `int(&)[]`，这也可以用作模板参数[^2]，请参考第 19.3.1 节。

[^2]: 类型 `X (&)[]` 的参数（其中 `X` 是任意类型）在 C++17 中才通过解决核心问题 393 而变得有效。然而，许多编译器在较早版本的语言中就已经接受了这种参数。

## 5.5. 类成员模板

类成员也可以是模板，这对于嵌套类和成员函数都是可能的。这个功能的应用和优势可以通过 `Stack<>` 类模板再次得到展示。通常情况下，只有当两个栈具有相同的类型（这意味着元素类型相同）时，才能将它们互相赋值。然而，即使元素类型之间定义了隐式类型转换，也无法将不同类型元素的栈互相赋值：

```cpp
Stack<int> intStack1, intStack2; // 整型栈
Stack<float> floatStack; // 浮点型栈
...
intStack1 = intStack2; // 可以：栈的类型相同
floatStack = intStack1; // 错误：栈的类型不同
```

默认的赋值操作符要求赋值操作符两侧的类型相同，而如果栈的元素类型不同，这个条件就不满足。

然而，通过将赋值操作符定义为模板，可以使具有适当类型转换的元素的栈之间的赋值成为可能。要做到这一点，你需要像下面这样声明 `Stack<>`：[stackdecl](../../Codes/ch05/5_5/stackdecl.hpp)

```cpp
template <typename T>
class Stack
{
private:
    std::deque<T> elems; // 元素
public:
    void push(T const &); // 压入元素
    void pop();           // 弹出元素
    T const &top() const; // 返回顶部元素
    bool empty() const
    { // 返回栈是否为空
        return elems.empty();
    }

    // 为类型为 T2 的元素的栈定义赋值操作符
    template <typename T2>
    Stack &operator=(Stack<T2> const &);
};
```

在这里，进行了以下两个更改：
1. 添加了一个用于其他类型 `T2` 元素的**栈的赋值操作符**声明。
2. 现在，栈使用 `std::deque<>` 作为内部容器存储元素。这是实现新赋值操作符的一个结果。

新的赋值操作符的实现如下[^3]：[stackassign](../../Codes/ch05/5_5/stackassign.hpp)

```cpp
template <typename T>
template <typename T2>
Stack<T> &Stack<T>::operator=(Stack<T2> const &op2)
{
    Stack<T2> tmp(op2); // 创建被赋值栈的副本
    elems.clear();      // 清除现有元素
    while (!tmp.empty())
    { // 复制所有元素
        elems.push_front(tmp.top());
        tmp.pop();
    }
    return *this;
}
```

首先来看一下定义成员模板的语法。在模板参数 `T` 的模板内，定义了一个模板参数 `T2` 的内嵌模板：

```cpp
template<typename T>
template<typename T2>
...
```

在成员函数内部，你可能期望直接访问被赋值栈 `op2` 的所有必要数据。然而，由于这个栈的类型不同（如果为两个不同的参数类型实例化类模板，你将得到两个不同的类类型），所以你只能使用公共接口。因此，访问元素的唯一方法是调用 `top()`。然而，这样每个元素都必须成为栈顶元素。因此，首先需要创建 `op2` 的副本，以便通过调用 `pop()` 从该副本中获取元素。由于 `top()` 返回的是最后一个压入栈的元素，我们可能更希望使用一个支持在集合的另一端插入元素的容器。出于这个原因，我们使用 `std::deque<>`，它提供 `push_front()` 来将元素放在集合的另一端。

[^3]: 这是一个基本的实现，用于展示模板的特性。需要注意的是，诸如适当的异常处理等问题在此实现中尚未涉及。

为了访问 `op2` 的所有成员，可以声明所有其他栈实例为友元：

```cpp
template<typename T>
class Stack
{
private:
    std::deque<T> elems;        // 元素
public:
    void push(T const&);
    void pop();
    T const& top() const;
    bool empty() const;
    template<typename T2>
    Stack& operator=(Stack<T2> const&);
    template<typename> friend class Stack;
};
```

如你所见，由于模板参数的名称没有被使用，因此可以省略它：

```cpp
template<typename> friend class Stack;
```

现在，可以实现如下的模板赋值操作符：

```cpp
template<typename T>
template<typename T2>
Stack<T>& Stack<T>::operator= (Stack<T2> const& op2) {
    elems.clear(); // 移除现有元素
    elems.insert(elems.begin(), // 插入到起始位置
                 op2.elems.begin(), // 来自 op2 的所有元素
                 op2.elems.end());
    return *this;
}
```

无论你的实现如何，借助这个成员模板，你现在可以将一个 `int` 类型的栈赋值给一个 `float` 类型的栈：

```cpp
Stack<int> intStack; // 整型栈
Stack<float> floatStack; // 浮点型栈
...
floatStack = intStack; // 可以：栈的类型不同，但 int 可转换为 float
```

当然，这种赋值操作不会改变栈及其元素的类型。赋值后，`floatStack` 的元素仍然是 `float` 类型，因此 `top()` 返回的仍然是 `float` 类型。

看起来这个函数会禁用类型检查，使得你可以将任意类型的元素栈赋值给另一个栈，但实际情况并非如此。当（源栈副本的）元素移动到目标栈时，必要的类型检查仍会进行：

```cpp
elems.push_front(tmp.top());
```

例如，如果尝试将一个 `string` 类型的栈赋值给 `float` 类型的栈，编译这行代码时会产生错误信息，指出由 `tmp.top()` 返回的 `string` 无法作为参数传递给 `elems.push_front()`（具体信息取决于编译器，但大致意思就是如此）：

```cpp
Stack<std::string> stringStack; // 字符串栈
Stack<float> floatStack; // 浮点型栈
...
floatStack = stringStack; // 错误：std::string 无法转换为 float
```

此外，你可以修改实现以**参数化内部容器类型**：

```cpp
template<typename T, typename Cont = std::deque<T>>
class Stack {
private:
    Cont elems;             // 元素
public:
    void push(T const&);    // 压入元素
    void pop();             // 弹出元素
    T const& top() const;   // 返回顶部元素
    bool empty() const {    // 返回栈是否为空
        return elems.empty();
    }

    // 为类型为 T2 的元素的栈定义赋值操作符
    template<typename T2, typename Cont2>
    Stack& operator= (Stack<T2,Cont2> const&);

    // 允许任意类型 T2 的 Stack<T2> 访问私有成员
    template<typename, typename> friend class Stack;
};
```

然后模板赋值操作符的实现如下：

```cpp
template<typename T, typename Cont>
template<typename T2, typename Cont2>
Stack<T, Cont>&
Stack<T, Cont>::operator=(Stack<T2, Cont2> const& op2)
{
    elems.clear();
    elems.insert(elems.begin(),
                 op2.elems.begin(),
                 op2.elems.end());
    return *this;
}
```

记住，**对于类模板，只有被调用的成员函数才会被实例化**。因此，如果你避免将不同类型的元素栈互相赋值，你甚至可以使用 `vector` 作为内部容器：

```cpp
// 使用 vector 作为内部容器的整型栈
Stack<int, std::vector<int>> vStack;
...
vStack.push(42);
vStack.push(7);
std::cout << vStack.top() << '\n';
```

由于模板赋值操作符并不必要，因此不会出现缺少成员函数 `push_front()` 的错误信息，程序可以正常运行。

**成员函数模板的特化**

成员函数模板也可以进行部分或完全特化。例如，对于以下类：

```cpp
class BoolString {
private:
    std::string value;
public:
    BoolString (std::string const& s)
    : value(s) {}

    template<typename T = std::string>
    T get() const { // 获取值（转换为 T 类型）
        return value;
    }
};
```

你可以为成员函数模板提供一个完全特化，如下所示：

```cpp
// BoolString::getValue<>() 针对 bool 类型的完全特化
template<>
inline bool BoolString::get<bool>() const {
    return value == "true" || value == "1" || value == "on";
}
```

注意，你不需要也不能声明这些特化；你只需要定义它们。由于这是一个完全特化，并且它位于头文件中，为了避免当定义被不同翻译单元包含时出现错误，需要将其声明为 `inline`。

你可以像下面这样使用类和完全特化：

```cpp
std::cout << std::boolalpha;
BoolString s1("hello");
std::cout << s1.get() << '\n'; // 输出 hello
std::cout << s1.get<bool>() << '\n'; // 输出 false
BoolString s2("on");
std::cout << s2.get<bool>() << '\n'; // 输出 true
```

**特殊的成员函数模板**

模板成员函数可以用于特殊成员函数允许拷贝或移动对象的情况。类似于上面定义的赋值操作符，它们也可以是构造函数。然而，注意模板构造函数或模板赋值操作符不会替代预定义的构造函数或赋值操作符。成员模板并不被视为允许拷贝或移动对象的特殊成员函数。在这个例子中，对于相同类型的栈的赋值操作，仍然会调用默认的赋值操作符。

这种效果有好有坏：

- 可能会发生模板构造函数或赋值操作符比预定义的拷贝/移动构造函数或赋值操作符更匹配的情况，尽管模板版本只为其他类型的初始化而提供。详细信息请参见第 6.2 节；
- 很难“模板化”一个拷贝/移动构造函数，例如，为了能够限制它的存在。详细信息请参见第 6.4 节；

### 5.5.1. `.template` 构造

在某些情况下，当调用成员模板时，有必要显式指定模板参数。这时，你需要使用 `template` 关键字来确保 `<` 是模板参数列表的开始。请看下面使用标准 `bitset` 类型的示例：

```cpp
template<unsigned long N>
void printBitset(std::bitset<N> const& bs) {
    std::cout << bs.template to_string<char, std::char_traits<char>, std::allocator<char>>();
}
```

在这个例子中，我们为 `bitset` 类型的 `bs` 调用了成员函数模板 `to_string()`，同时显式指定了字符串类型的细节。如果没有额外使用 `.template`，编译器不知道紧跟着的 `<` 是小于号还是模板参数列表的开始。

注意，只有在句点之前的构造依赖于模板参数时，这才是一个问题。在我们的示例中，参数 `bs` 依赖于模板参数 `N`。

`.template` 语法（以及类似的 `->template` 和 `::template`）应**仅在模板内部使用**，并且**仅在它们跟随的内容依赖于模板参数时**使用。更多细节请参见第 13.3.3 节。

### 5.5.2. 泛型 Lambda 和成员模板

需要注意的是，C++14 引入的泛型 lambda 是成员模板的简写。例如，下面这个用于计算任意类型参数求和的简单 `lambda`：

```cpp
[] (auto x, auto y) {
    return x + y;
}
```

是以下类的默认构造对象的简写：

```cpp
class SomeCompilerSpecificName {
public:
    SomeCompilerSpecificName(); // 仅编译器可调用的构造函数

    template<typename T1, typename T2>
    auto operator()(T1 x, T2 y) const {
        return x + y;
    }
};
```

更多细节请参见第 15.10.6 节。

## 5.6. 模板变量

自 C++14 起，变量也可以通过特定类型进行参数化，这种特性称为**模板变量[^4]**。

例如，以下代码定义了 `pi` 的值，但没有明确指定其类型：

```cpp
template<typename T>
constexpr T pi{3.1415926535897932385};
```

需要注意的是，和所有模板一样，这种声明不能出现在函数内部或块作用域中。

[^4]: 是的，虽然我们有非常相似的术语，但它们代表的是完全不同的概念：**模板变量**是一个模板化的变量（“变量”在这里是名词）；而**可变参数模板**则是针对可变数量模板参数的模板（“可变参数”在这里是形容词）。

要使用模板变量，必须指定其类型。例如，下面的代码使用了在 `pi<>` 声明的作用域中定义的两个不同变量：

```cpp
std::cout << pi<double> << '\n';
std::cout << pi<float> << '\n';
```

你也可以声明在不同翻译单元中使用的模板变量：

```cpp
// header.hpp:
template<typename T> T val{}; // 零初始化值

// translation unit 1:
#include "header.hpp"
int main()
{
    val<long> = 42;
    print();
}

// translation unit 2:
#include "header.hpp"
void print()
{
    std::cout << val<long> << '\n'; // OK: 打印 42
}
```

模板变量还可以有默认模板参数：

```cpp
template<typename T = long double>
constexpr T pi = T{3.1415926535897932385};
```

你可以使用默认类型或其他任何类型：

```cpp
std::cout << pi<> << '\n'; // 输出 long double
std::cout << pi<float> << '\n'; // 输出 float
```

不过需要注意的是，你始终需要指定尖括号。如果仅使用 `pi` 会产生错误：

```cpp
std::cout << pi << '\n'; // 错误
```

模板变量也可以通过非类型参数进行参数化，并且这些参数还可以用于参数化初始化器。例如：

```cpp
#include <iostream>
#include <array>

template<int N>
std::array<int, N> arr{}; // N 个元素的数组，零初始化

template<auto N>
constexpr decltype(N) dval = N; // dval 的类型取决于传入的值

int main()
{
    std::cout << dval<'c'> << '\n'; // N 的值为字符类型的 'c'
    arr<10>[0] = 42; // 设置全局 arr 的第一个元素
    for (std::size_t i = 0; i < arr<10>.size(); ++i) { // 使用 arr 中的值
        std::cout << arr<10>[i] << '\n';
    }
}
```

同样需要注意，即使在不同的翻译单元中初始化和遍历 `arr`，仍然使用的是全局作用域中的相同变量 `std::array<int, 10> arr`。

**数据成员的模板变量**

**模板变量的一个有用应用是定义表示类模板成员的变量**。例如，如果定义了如下的类模板：

```cpp
template<typename T>
class MyClass {
public:
    static constexpr int max = 1000;
};
```

这允许为 `MyClass<>` 的不同特化定义不同的值，然后可以定义：

```cpp
template<typename T>
int myMax = MyClass<T>::max;
```

这样，程序员只需编写：

```cpp
auto i = myMax<std::string>;
```

而不必写：

```cpp
auto i = MyClass<std::string>::max;
```

这意味着，对于像这样的标准类：

```cpp
namespace std {
template<typename T> class numeric_limits {
public:
    ...
    static constexpr bool is_signed = false;
    ...
};
}
```

你可以定义：

```cpp
template<typename T>
constexpr bool isSigned = std::numeric_limits<T>::is_signed;
```

从而可以编写：

```cpp
isSigned<char>
```

而不必写：

```cpp
std::numeric_limits<char>::is_signed
```

**类型特征后缀 `_v`**

自 C++17 起，标准库使用**模板变量技术**为所有产生（布尔）值的类型特征定义了简写。例如，为了写：

```cpp
std::is_const_v<T> // 自 C++17 起
```

而不是：

```cpp
std::is_const<T>::value // 自 C++11 起
```

标准库定义了：

```cpp
namespace std {
template<typename T> constexpr bool is_const_v = is_const<T>::value;
}
```

## 5.7. 模板化模板参数

在某些情况下，允许模板参数本身成为类模板是非常有用的。例如，可以使用模板化模板参数来改进之前提到的 `Stack` 类模板，使其支持使用不同的内部容器类型。

通常，如果应用程序员想要为栈指定一个不同的内部容器类型，他们需要再次指定元素类型。例如：

```cpp
Stack<int, std::vector<int>> vStack; // 使用 vector 作为内部容器的整数栈
```

但是，通过使用模板化模板参数，我们可以简化这个过程，使得只需指定容器类型，而不需要再次指定其元素类型：

```cpp
Stack<int, std::vector> vStack; // 使用 vector 作为内部容器的整数栈
```

要实现这一点，需要将第二个模板参数指定为模板化模板参数。其声明方式如下[^5]：

```cpp
template<typename T,
template<typename Elem> class Cont = std::deque>
class Stack {
private:
    Cont<T> elems; // 元素
public:
    void push(T const&); // 压入元素
    void pop(); // 弹出元素
    T const& top() const; // 返回栈顶元素
    bool empty() const { // 返回栈是否为空
        return elems.empty();
    }
    // 其他成员函数
};
```

[^5]: 在 C++17 之前，这种版本存在一个问题，我们稍后会解释。然而，这仅影响默认值 `std::deque`。因此，我们可以在讨论如何处理 C++17 之前的情况之前，用这个默认值来说明模板化模板参数的一般特性。

区别在于第二个模板参数被声明为类模板：

```cpp
template<typename Elem> class Cont
```

默认值已从 `std::deque<T>` 更改为 `std::deque`。此参数必须是一个类模板，它会为第一个模板参数传递的类型进行实例化：
```cpp
Cont<T> elems;
```

在这个示例中，使用第一个模板参数来实例化第二个模板参数是特定的。一般来说，你可以在类模板内部用任何类型来实例化模板化模板参数。

像往常一样，你可以使用 `class` 关键字来替代 `typename` 用于模板参数。在 C++11 之前，`Cont` 只能被替换为类模板的名称：

```cpp
template<typename T,
template<class Elem> class Cont = std::deque>
class Stack { // OK
...
};
```

从 C++11 开始，我们也可以用别名模板的名称替换 `Cont`，但直到 C++17 才允许使用 `typename` 关键字代替 `class` 来声明模板化模板参数：

```cpp
template<typename T,
template<typename Elem> typename Cont = std::deque>
class Stack { // C++17 之前出错
...
};
```

这两种变体含义完全相同：使用 `class` 代替 `typename` 并不阻止我们将**别名模板**作为 `Cont` 参数的实参。

由于模板化模板参数的模板参数没有使用，通常省略其名称（除非它提供了有用的文档）：

```cpp
template<typename T,
template<typename> class Cont = std::deque>
class Stack {
...
};
```

**成员函数也必须相应地进行修改**。因此，你需要将**第二个模板参数指定为模板化模板参数**。这同样适用于成员函数的实现。例如，`push()` 成员函数的实现如下：

```cpp
template<typename T, template<typename> class Cont>
void Stack<T,Cont>::push (T const& elem)
{
    elems.push_back(elem); // 添加传递的 elem 的副本
}
```

请注意，虽然模板化模板参数是类或别名模板的占位符，但没有对应的占位符用于函数或变量模板。

**模板化模板参数匹配**

如果你尝试使用新版的 `Stack`，你可能会收到错误信息，说明默认值 `std::deque` 与模板化模板参数 `Cont` 不兼容。**问题在于 C++17 之前，模板化模板参数必须是一个参数完全匹配的模板**，尽管有一些与变参模板相关的例外（见第 12.3.4 节）。模板化模板参数的默认模板参数没有被考虑，因此通过省略具有默认值的参数无法匹配（在 C++17 中，默认参数会被考虑）。

C++17 之前的这个问题在于，标准库的 `std::deque` 模板有多个参数：第二个参数（描述分配器）有一个默认值，但在 C++17 之前，这在将 `std::deque` 匹配到 `Cont` 参数时没有被考虑。

不过有一个解决方法。我们可以重写类声明，使得 `Cont` 参数期望具有两个模板参数的容器：

```cpp
template<typename T,
template<typename Elem,
typename Alloc = std::allocator<Elem>>
class Cont = std::deque>
class Stack {
private:
    Cont<T> elems; // 元素
    ...
};
```

我们可以省略 `Alloc`，因为它没有被使用。

我们最终版本的 `Stack` 模板（包括不同元素类型的栈赋值成员模板）现在如下所示：[stack.hpp](../../Codes/ch05/5_7/stack.hpp)

```cpp
#include <deque>
#include <cassert>
#include <memory>

template<typename T,
template<typename Elem,
typename = std::allocator<Elem>>
class Cont = std::deque>
class Stack {
private:
    Cont<T> elems; // 元素

public:
    void push(T const&); // 压入元素
    void pop(); // 弹出元素
    T const& top() const; // 返回顶部元素
    bool empty() const { // 返回栈是否为空
        return elems.empty();
    }

    // 赋值不同类型元素的栈
    template<typename T2,
    template<typename Elem2,
    typename = std::allocator<Elem2>
    >class Cont2>
    Stack<T,Cont>& operator= (Stack<T2,Cont2> const&);

    // 访问任何具有类型 T2 元素的 Stack 的私有成员：
    template<typename, template<typename, typename>class>
    friend class Stack;
};

template<typename T, template<typename,typename> class Cont>
void Stack<T,Cont>::push (T const& elem)
{
    elems.push_back(elem); // 添加传递的 elem 的副本
}

template<typename T, template<typename,typename> class Cont>
void Stack<T,Cont>::pop ()
{
    assert(!elems.empty());
    elems.pop_back(); // 移除最后一个元素
}

template<typename T, template<typename,typename> class Cont>
T const& Stack<T,Cont>::top () const
{
    assert(!elems.empty());
    return elems.back(); // 返回最后一个元素的副本
}

template<typename T, template<typename,typename> class Cont>
template<typename T2, template<typename,typename> class Cont2>
Stack<T,Cont>&
Stack<T,Cont>::operator= (Stack<T2,Cont2> const& op2)
{
    elems.clear(); // 移除现有元素
    elems.insert(elems.begin(), // 插入到开始位置
        op2.elems.begin(), // 从 op2 插入所有元素
        op2.elems.end());
    return *this;
}
```

再次提醒，为了访问 `op2` 的所有成员，我们**声明所有其他栈实例为友元**（省略了模板参数的名称）：

```cpp
template<typename, template<typename, typename>class>
friend class Stack;
```

然而，并非所有标准容器模板都可以用作 `Cont` 参数。例如，`std::array` 将无法使用，因为它包含一个用于数组长度的非类型模板参数，在我们的模板模板参数声明中没有匹配项。

以下程序使用了最终版本的所有特性：[stack.cpp](../../Codes/ch05/5_7/stack.cpp)

```cpp
#include "stack.hpp"
#include <iostream>
#include <vector>

int main()
{
    Stack<int> iStack; // 整数栈
    Stack<float> fStack; // 浮点栈

    // 操作整数栈
    iStack.push(1);
    iStack.push(2);
    std::cout << "iStack.top(): " << iStack.top() << '\n';

    // 操作浮点栈
    fStack.push(3.3);
    std::cout << "fStack.top(): " << fStack.top() << '\n';

    // 赋值不同类型的栈并再次操作
    fStack = iStack;
    fStack.push(4.4);
    std::cout << "fStack.top(): " << fStack.top() << '\n';

    // 使用 vector 作为内部容器的双精度栈
    Stack<double, std::vector> vStack;
    vStack.push(5.5);
    vStack.push(6.6);
    std::cout << "vStack.top(): " << vStack.top() << '\n';
    vStack = fStack;
    std::cout << "vStack: ";
    while (!vStack.empty()) {
        std::cout << vStack.top() << ' ';
        vStack.pop();
    }
    std::cout << '\n';
}
```

程序输出如下：
```bash
iStack.top(): 2
fStack.top(): 3.3
fStack.top(): 4.4
vStack.top(): 6.6
vStack: 4.4 2 1
```

## 5.8. 总结

- 要访问依赖于模板参数的类型名称，必须用前缀 `typename` 进行限定；
- 要访问依赖于模板参数的基类成员，必须用 `this->` 或其类名进行限定；
- 嵌套类和成员函数也可以是模板。一个应用是实现具有内部类型转换的通用操作；
- **模板版本的构造函数或赋值运算符不会替代预定义的构造函数或赋值运算符**；
- 通过使用花括号初始化或显式调用默认构造函数，可以确保即使实例化为内置类型时，模板的变量和成员也会以默认值初始化；
- 可以为原始数组提供特定的模板，这也适用于字符串字面量；
- 在传递原始数组或字符串字面量时，参数在参数推导过程中会退化（进行数组到指针的转换），如果且仅如果参数不是引用；
- 可以定义模板变量（自 C++14 起）；
- 你还可以使用**类模板**作为**模板参数**，即**模板化模板参数**；
- 模板化模板参数通常必须与其参数完全匹配；