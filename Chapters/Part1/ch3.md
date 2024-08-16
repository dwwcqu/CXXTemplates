# 3. 非类型模板参数(Nontype Template Parameters)

在函数和类模板中，模板参数不一定是类型。它们也可以是普通的数值。与使用类型参数的模板类似，您定义的代码会保留某个细节，直到代码被使用。然而，这个开放的细节是一个数值，而不是一个类型。当使用这样的模板时，您必须**显式地指定这个值**，然后生成的代码会被实例化。本章节详细说明了如何在新版本的堆栈类模板中使用这一特性。此外，我们展示了使用非类型函数模板参数的示例，并讨论了这种技术的一些限制。

## 3.1 非类型类模板参数

与前几章节中堆栈的示例实现相反，您还可以通过使用**固定大小的数组**来实现堆栈。这种方法的优点在于可以避免您或标准容器执行的内存管理开销。然而，确定这种堆栈的最佳大小可能会有挑战。您指定的大小越小，堆栈满的可能性就越大。您指定的大小越大，可能会不必要地保留内存。一个好的解决方案是**让堆栈的使用者指定数组的大小**，作为堆栈元素的最大大小。

为此，将大小定义为一个模板参数：[stacknontype.hpp](.././../Codes/ch03/3_1/stacknontype.hpp)
```cpp
#include <array>
#include <cassert>

template<typename T, std::size_t Maxsize>
class Stack {
private:
    std::array<T, Maxsize> elems;   // 元素
    std::size_t numElems;           // 当前元素个数
public:
    Stack();                        // 构造函数
    void push(T const& elem);       // 推入元素
    void pop();                     // 推出元素
    T const& top();                 // 返回栈顶元素
    bool empty() const {            // 返回当前栈是否为空
        return numElems == 0;
    }
    std::size_t size() const {      // 返回当前元素个数
        return numElems;
    }
};

template<typename T, std::size_t Maxsize>
Stack<T, Maxsize>::Stack()
    : numElems(0)
{
    // null
}

template<typename T, std::size_t Maxsize>
void Stack<T, Maxsize>::push(T const& elem)
{
    assert(numElems < Maxsize);
    elems[numElems] = elem;     // 插入元素到最后
    ++numElems;                 // 元素个数加1
}

template<typename T, std::size_t Maxsize>
void Stack<T, Maxsize>::pop()
{
    assert(!elems.empty());
    --numElems;                 // 元素个数减1
}

template<typename T, std::size_t Maxsize>
T const& Stack<T, Maxsize>::top()
{
    assert(!elems.empty());
    return elems[numElems - 1]; // 返回最后一个元素
}
```

新的第二个模板参数 `Maxsize` 时一个 `int` 类型，它指定栈元素的内部数组的大小。此外，`Maxsize` 在 `push` 方法中用来判断栈是否占满。

为了使用这类类模板，你需要指定元素类型和最大的元素个数：[stacknontype.cpp](.././../Codes/ch03/3_1/stacknontype.cpp)
```cpp
#include "stacknontype.hpp"
#include <iostream>
#include <string>
int main()
{
    Stack<int, 20> int20Stack;          // 20 个 int 元素栈
    Stack<int, 40> int40Stack;          // 40 个 int 元素栈
    Stack<std::string, 40> stringStack; // 40 个 string 元素栈
    
    int20Stack.push(7);
    std::cout << int20Stack.top() << '\n';
    int20Stack.pop();
    
    stringStack.push("hello");
    std::cout << stringStack.top() << '\n';
    stringStack.pop();
}
```

请注意，每个模板实例化都是其自己的类型。因此，`int20Stack` 和 `int40Stack` 是两种不同的类型，它们之间没有隐式或显式的类型转换定义。因此，不能将一个用于另一个，也不能将一个赋值给另一个。

同样地，模板参数可以指定默认参数：
```cpp
template<typename T = int, std::size_t Maxsize = 100>
class Stack {
    // implementation
};
```

然而，从良好设计的角度来看，在这个例子中可能不太合适使用默认参数。默认参数应该直观地正确。但是，对于一个通用的堆栈类型来说，既不是 int 类型，也不是最大大小为 100 的值，看起来都不是很直观。因此，最好让程序员明确地指定这两个值，以便在声明时始终记录这两个属性。

## 3.2. 非类型函数模板参数

你也可以为函数模板定义非类型参数。例如，以下函数模板定义了一组可以添加特定值的函数：

```cpp
template<int Val, typename T>
T addValue(T x)
{
    return x + Val;
}
```

这种类型的函数在函数或操作作为参数时会非常有用。例如，如果你使用 C++ 标准库，你可以将这个函数模板的实例化传递给 `std::transform` 来将一个值添加到集合的每个元素中：

```cpp
std::transform(source.begin(), source.end(),   // 源的起始和结束
               dest.begin(),                   // 目标的起始
               addValue<5, int>);              // 操作
```

最后一个参数实例化了函数模板 `addValue<>()`，以便将 `5` 添加到传递的 `int` 值中。结果是，这个函数会对源集合 `source` 中的每个元素进行调用，并将其转换为目标集合 `dest`。

需要注意的是，你必须为 `addValue<>()` 的模板参数 `T` 指定 `int` 类型。推导只适用于立即调用，而 `std::transform()` 需要一个完整的类型来推导其第四个参数的类型。目前没有支持仅替换/推导部分模板参数并查看合适的参数，之后推导其余参数的机制。

同样，你也可以指定从前一个参数推导模板参数。例如，从传递的非类型参数中派生返回类型：

```cpp
template<auto Val, typename T = decltype(Val)>
T foo();
```

或者，确保传递值类型与传递类型有相同的类型：

```cpp
template<typename T, T Val = T{}>
T bar();
```

## 3.3. 非类型模板参数的限制

请注意，非类型模板参数具有一些限制。通常，它们只能是**常量整数值**（包括**枚举**）、**指向对象/函数/成员的指针**、**对对象或函数的左值引用**，或 `std::nullptr_t`（`nullptr` 的类型）。

**浮点数和类类型对象不允许作为非类型模板参数**：

```cpp
template<double VAT>        // 错误：浮点值不允许作为模板参数
double process(double v)
{
    return v * VAT;
}

template<std::string name> // 错误：类类型对象不允许作为模板参数
class MyClass {
    // ...
};
```

**当传递指针或引用的模板参数时，对象不能是字符串字面值、临时对象或数据成员以及其他子对象**。由于这些限制在 C++17 之前的每个 C++版本中都有所放宽，因此还需遵守额外的约束：

- 在 C++11 中，对象必须具有外部链接性。
- 在 C++14 中，对象必须具有外部或内部链接性。

因此，以下操作是不可能的：

```cpp
template<char const* name>
class MyClass {
    // ...
};

MyClass<"hello"> x; // 错误：字符串字面值 "hello" 不允许
```

然而，针对这些限制，存在一些变通方法（依赖于 C++ 的具体版本）：

```cpp
extern char const s03[] = "hi";
char const s11[] = "hi";

int main()
{
    Message<s03> m03;       // OK，在所有版本都可以
    Message<s11> m11;       // 从 C++11 开始 OK
    static char const s17[] = "hi";
    Message<s17> m17;       // 从 C++17 开始 OK
}
```

在所有三种情况下，都通过 `"hello"` 初始化了一个常量字符数组，并将该对象用作声明为 `char const*` 的模板参数。如果对象具有外部链接性（`s03`），那么这在所有 C++ 版本中都是合法的；在 C++11 和 C++14 中，如果对象具有内部链接性（`s11`），也是合法的；**自 C++17 起，即使对象没有任何链接性，这也是合法的**。

### 3.3.1. 避免无效表达式

非类型模板参数的参数可以是任何编译时表达式。例如：

```cpp
template<int I, bool B>
class C;
...
C<sizeof(int) + 4, sizeof(int)==4> c;
```

但是，需要注意的是，如果在表达式中使用了 `operator>`，**必须将整个表达式括在括号内**，**以确保嵌套的 `>` 符号不会意外结束模板参数列表**：

```cpp
C<42, sizeof(int) > 4> c;   // 错误：第一个 > 结束了模板参数列表
C<42, (sizeof(int) > 4)> c; // 正确
```

## 3.4. 模板参数类型 `auto`

从 C++17 开始，可以定义一个非类型模板参数，以一般化方式接受任何允许的非类型参数类型。使用这种特征，我们就可以可以提供一个更加一般化固定大小的栈类：[stackauto.hpp](.././../Codes/ch03/3_4/stackauto.hpp)

```cpp
#include <array>
#include <cassert>

template<typename T, auto Maxsize>
class Stack
{
public:
    using size_type = decltype(Maxsize);
private:
    std::array<T, Maxsize> elems;
    size_type numElems;
public:
    Stack();
    void push(T const& elem);
    void pop();
    T const& top() const;
    bool empty() const
    {
        return numElems == 0;
    }
    size_type size() const
    {
        return numElems;
    }
};

template<typename T, auto Maxsize>
Stack<T, Maxsize>::Stack() : numElems(0)
{

}

template<typename T, auto Maxsize>
void Stack<T, Maxsize>::push(T const& elem)
{
    assert(numElems < Maxsize);
    elems[numElems] = elem;
    ++numElems;
}

template<typename T, auto Maxsize>
void Stack<T, Maxsize>::pop()
{
    assert(!elems.empty());
    --numElems;
}

template<typename T, auto Maxsize>
T const& Stack<T, Maxsize>::top() const
{
    assert(!elems.empty());
    return elems[numElems - 1];
}
```

通过定义

```cpp
template<typename T, auto Maxsize>
class Stack {
    ...
};
```

使用占位符类型 `auto`，你可以将 `Maxsize` 定义为尚未指定类型的值。它可以是任何允许作为**非类型模板参数类型**的类型。

在内部，你可以同时使用该值：

```cpp
std::array<T, Maxsize> elems; // 元素
```

以及它的类型：

```cpp
using size_type = decltype(Maxsize);
```

然后，这个类型可以用于 `size()` 成员函数的返回类型，例如：

```cpp
size_type size() const { // 返回当前元素数量
    return numElems;
}
```

自 C++14 以来，你还可以在此处直接使用 `auto` 作为返回类型，以让编译器自动推导返回类型：

```cpp
auto size() const { // 返回当前元素数量
    return numElems;
}
```

通过这个类声明，当使用栈时，元素数量的类型由用于元素数量的类型定义：[stackauto.cpp](.././../Codes/ch03/3_4/stackauto.cpp)

```cpp
#include <iostream>
#include <string>
#include "stackauto.hpp"

int main()
{
    Stack<int, 20u>         int20Stack;     // 20 int 栈
    Stack<std::string, 40>  stringStack;    // 40 std::string 栈

    int20Stack.push(7);
    std::cout << int20Stack.top() << std::endl;
    auto size1 = int20Stack.size();

    stringStack.push("hello");
    std::cout << stringStack.top() << std::endl;
    auto size2 = stringStack.size();

    if(!std::is_same_v<decltype(size1), decltype(size2)>)
    {
        std::cout << "size types differ" << std::endl;
    }
    return 0;
}
```

使用

```cpp
Stack<int, 20u> int20Stack; // 最多存储 20 个 int 的栈
```

由于传递了 `20u`，内部的 `size_type` 是 `unsigned int`。

使用

```cpp
Stack<std::string, 40> stringStack; // 最多存储 40 个字符串的栈
```

由于传递了 `40`，内部的 `size_type` 是 `int`。

对于这两个栈，`size()` 的返回类型会有所不同，因此在

```cpp
auto size1 = int20Stack.size();
...
auto size2 = stringStack.size();
```

之后，`size1` 和 `size2` 的类型是不同的。通过使用标准类型特性 `std::is_same` 和 `decltype`，我们可以进行检查：

```cpp
if (!std::is_same<decltype(size1), decltype(size2)>::value) {
    std::cout << "size types differ" << '\n';
}
```

因此，输出将是：

```
size types differ
```

从 C++17 开始，对于返回值的特性，你还可以使用后缀 `_v` 来省略 `::value`：

```cpp
if (!std::is_same_v<decltype(size1), decltype(size2)>) {
    std::cout << "size types differ" << '\n';
}
```

注意，关于非类型模板参数类型的其他限制仍然有效。尤其是，关于非类型模板参数可能类型的限制仍然适用。例如：

```cpp
Stack<int, 3.14> sd; // 错误：浮点数作为非类型模板参数
```

并且，由于你可以传递字符串作为常量数组（从 C++17 开始，甚至可以是静态的局部声明），因此以下是可能的：[message.cpp](.././../Codes/ch03/3_4/message.cpp)

```cpp
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
```

请注意，甚至可以使用 `template<decltype(auto) N>`，这允许 `N` 被实例化为引用：

```cpp
template<decltype(auto) N>
class C {
    ...
};
int i;
C<(i)> x; // N 是 int&
```

有关详细信息，请参见 15.10.1 节。

## 3.5. 总结

- 模板可以有非类型的模板参数，即模板参数可以是值而非类型。
- 不能使用浮点数或类类型对象作为非类型模板参数的实参。对于指向字符串字面量、临时对象和子对象的指针/引用，有一定限制。
- 使用 `auto` 可以使模板具有通用类型的非类型模板参数。