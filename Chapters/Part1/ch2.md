# 2. 类模板

类似于函数，类也可以使用一个或多个类型进行参数化。容器类是这种特性的典型示例，它们用于管理特定类型的元素。通过使用类模板，您可以在**元素类型**仍然未确定的情况下实现这些容器类。在本章中，我们以**堆栈**作为类模板的示例。

## 2.1. `Stack` 类模版实现

与函数模板一样，我们在头文件中声明和定义类模板 `Stack<>`，如下所示：[Codes/ch02/2_1/stack1.hpp](../../Codes/ch02/2_1/stack1.hpp)
```cpp
#include <vector>
#include <cassert>

template<typename T>
class Stack
{
private:
    std::vector<T> elems;           // elements
public:
    void push(T const& elem);       // push element
    void pop();                     // pop element
    T const& top() const;           // return top element
    bool empty() const              // return whether the stack is empty
    {
        return elems.empty();
    }
};

template<typename T>
void Stack<T>::push(T const& elem)
{
    elems.push_back(elem);          // append copy of passed elem
}

template<typename T>
void Stack<T>::pop()
{
    assert(!elems.empty());
    elems.pop_back();               // remov the last element
}

template<typename T>
T const& Stack<T>::top() const
{
    assert(!elems.empty());
    return elems.back();            // return copy of the last element
}
```
正如您所看到的，该类模板是通过使用 C++ 标准库的类模板 `vector<>` 实现的。因此，我们不必实现内存管理、拷贝构造函数和赋值运算符，因此可以集中精力处理该类模板的接口。

### 2.1.1. 类模板声明

声明类模板与声明函数模板类似：在声明之前，您必须将一个或多个标识符声明为类型参数。再次说明，通常使用 `T` 作为标识符：
```cpp
template<typename T>
class Stack
{
    ...
};
```
在这里，可以使用 `class` 关键字来替代 `typename`:
```cpp
template <class T>
class Stack
{
    ...
};
```
在类模板内部，`T` 可以像任何其他类型一样用于**声明成员**和**成员函数**。在这个示例中，`T`被用来声明元素的类型为`T`的向量，声明 `push()` 作为一个使用 `T` 作为参数的成员函数，以及声明 `top()` 作为一个返回 `T` 的函数：
```cpp
template<typename T>
class Stack
{
private:
    std::vector<T> elems;       // elements
public:
    void push(T const& elem);   // push element
    void pop();                 // pop element
    T const& top() const;       // return top element
    bool empty() const          // return whether the stack is empty
    {
        return elems.empty();
    }
};
```
这个类的类型是 `Stack<T>`，其中 `T` 是一个模板参数。因此，在声明中，每当你使用这个类的类型时，你都必须使用 `Stack<T>`，除非在模板参数可以被推断的情况下。然而，在类模板内部，使用类名而不跟随模板参数，表示**将模板参数作为其参数的类**（有关详细信息，请参阅[第 13.2.3 节](./ch13.md#1323)）。

例如，如果你必须声明自己的复制构造函数和赋值运算符，通常会像这样：
```cpp
template<typename T>
class Stack
{
    ...
    Stack (Stack const&);           // copy constructor
    Stack& operator=(Stack const&); // assignment operator
    ...
};
```
上面这种声明形式和：
```cpp
template<typename T>
class Stack
{
    ...
    Stack (Stack<T> const&);                // copy constructor
    Stack<T>& operator=(Stack<T> const&);   // assignment operator
    ...
};
```
一样。但通常 `<T>` 表示对**特殊模板参数**的**特殊处理**，因此**最好使用第一种形式**。

然而，在类结构声明以外的话，你需要：
```cpp
template<typename T>
bool operator==(Stack<T> const& lhs, Stack<T> const& rhs);
```
加上 `Stack<T>` 模板类的模板参数 `T`。

请注意，在需要类的名称，而不是类型的地方，只能使用 `Stack`。这在指定构造函数的名称（而不是它们的参数）和析构函数时特别如此。

还要注意，**与非模板类不同，你不能在函数或块作用域内声明或定义类模板**。通常，**模板只能在全局/命名空间范围内或在类声明内部定义**。

### 2.1.2. 成员函数的实现

要定义类模板的成员函数，你必须指定它是一个模板，并且必须使用类模板的完整类型限定。因此，类型为 `Stack<T>` 的成员函数 `push()` 的实现如下所示：
```cpp
template<typename T>
void Stack<T>::push (T const& elem)
{
    elems.push_back(elem);
}
```
在这种情况下，调用元素向量的 `push_back()` 函数，该函数将元素添加到向量的末尾。

请注意，向量的 `pop_back()` 函数移除最后一个元素，但不返回它。这种行为的原因是异常安全性。无法实现一个完全异常安全的 `pop()` 函数来返回被移除的元素（这个话题最初由 Tom Cargill 在[CargillExceptionSafety] 中讨论，并在 [SutterExceptional] 的第 10 项中讨论）。然而，如果忽略这种危险，我们可以实现一个返回刚刚移除的元素的 `pop()` 函数。为此，我们简单地使用 `T` 来声明一个元素类型的局部变量：
```cpp
template<typename T>
T Stack<T>::pop()
{
    assert(!elems.empty());
    T elem = elems.back();      // save the copy of the last element
    elems.pop_back();           // remove the last element
    return elem;                // return copy of saved element
}
```
因为 `back()`（返回最后一个元素）和 `pop_back()`（移除最后一个元素）在向量为空时具有未定义的行为，所以我们决定先检查栈是否为空。如果栈为空，我们使用断言（`assert`），因为在空栈上调用 `pop()` 是一种使用错误。在 `top()` 中也是如此，它返回但不移除顶部元素，在尝试移除不存在的顶部元素时执行相同的操作：
```cpp
template<typename T>
T const& Stack<T>::top() const
{
    assert(!elems.empty());
    return elems.back();            // return copy of the last element
}
```
当然，对于任何成员函数，您也可以将类模板的成员函数实现为类声明内的内联函数。例如：
```cpp
template<typename T>
class Stack
{
    ...
    void push(T const& elem)
    {
        elems.push_back(elem);
    }
};
```

## 2.2. 类模板 `Stack` 的使用

要使用类模板的对象，在 **C++17** 之前，您必须始终明确指定**模板参数**[1]。以下示例显示了如何使用类模板`Stack<>`：[Codes/ch02/2_2/stacktest.cpp](../../Codes/ch02/2_2/stacktest.cpp)
```cpp
#include "../2_1/stack1.hpp"
#include <iostream>
#include <string>

int main()
{
    Stack<int>          intStack;       // stack of ints
    Stack<std::string>  stringStack;    // stack of strings

    // manipulate int stack;
    intStack.push(7);
    std::cout << intStack.top() << std::endl;

    // manipulate string stack
    stringStack.push("hello");
    std::cout << stringStack.top() << std::endl;
    stringStack.pop();

    return 0;
}
```



通过声明类型 `Stack<int>`，在类模板内部使用 `int` 作为类型 `T`。因此，`intStack` 被创建为一个对象，它使用 `int` 的向量作为元素，并且**对于调用的所有成员函数，都会实例化该类型的代码**。类似地，通过声明和使用 `Stack<std::string>`，将创建一个使用字符串向量作为元素的对象，并且对于调用的所有成员函数，都会实例化该类型的代码。

请注意，**只有调用的模板（成员）函数才会被实例化**。**对于类模板，仅当使用时才实例化成员函数**。当然，这节省了时间和空间，并且允许仅部分使用类模板，我们将在[第2.3节](#23-类模板的部分使用)讨论。

在这个示例中，默认构造函数、`push()` 和 `top()` 都被 `int` 和字符串实例化。然而，`pop()` 仅对字符串进行实例化。如果一个类模板有静态成员，那么这些成员也会针对每种被使用的类型进行一次实例化。

实例化的类模板类型可以像任何其他类型一样使用。您可以用 `const` 或 `volatile` 限定符进行限定，或者从中派生数组和引用类型。您还可以将其用作 `typedef` 或 `using` 的类型定义的一部分（有关类型定义的详细信息，请参见[第2.8节](#28)），或者在构建另一个模板类型时将其用作类型参数。例如：
```cpp
void foo(Stack<int> const &s)       // parameter s is int stack
{
    using IntStack = Stack<int>;    // IntStack 为 Stack<int> 的另一个名称
    Stack<int> istack[10];          // istack 是拥有 10 个 int 栈的数组
    IntStack istack2[10];           // istack2 也是拥有 10 个 int 栈的数组
}
```
模板参数可以是任何类型，例如指向浮点数的指针甚至是整数的堆栈：
```cpp
Stack<float*>       floatPtrStack;  // stack of float pointers
Stack<Stack<int>>   intStackStack;  // stack of stack of ints
```
唯一的要求是根据此类型可以调用任何操作。

在 C++11 之前，你必须在两个闭合的模板尖括号之间放置空格：
```cpp
Stack<Stack<int> > intStackStack;   // OK for all c++ versions
```
如果你没有这样做，你会使用运算符 `>>`，这将导致语法错误：
```cpp
Stack<Stack<int>> intStackStack;    // ERROR before c++ 11
```
老版本的行为是为了帮助 C++ 编译器的第一遍对源代码进行**标记化（tokenize）**，而不考虑代码的语义。然而，由于缺少空格是一个典型的错误，需要相应的错误消息，因此代码的语义越来越需要被考虑进去。因此，随着 C++11 的到来，通过“尖括号规避”来在两个封闭的模板尖括号之间放置空格的规则被移除了（有关详细信息，请参见[第 13.3.1 节](./ch13.md#1331)）。

## 2.3. 类模板的部分使用

类模板通常对其实例化的模板参数执行多个操作（包括构造和析构）。这可能会让人误以为这些模板参数必须为类模板的所有成员函数提供所需的所有操作。但事实并非如此：**模板参数只需要提供所有需要的操作，而不是可能需要的所有操作**。

例如，如果类模板 `Stack<>` 提供了一个名为 `printOn()` 的成员函数来打印整个堆栈内容，该函数调用 `operator<<` 来打印每个元素：
```cpp
template<typename T>
class Stack
{
    ...
    void printOn(std::ostream& strm) const
    {
        for ( T const& elem : elems)
        {
            strm << elem << ' ';
        }
    }
};
```
你仍然可以将此类用于没有定义 `operator<<` 运算符的元素：
```cpp
Stack<std::pair<int, int>> ps;          // note: std::pair<> has no operator<< defined
ps.push({4, 5});                        // OK
ps.push({6, 7});                        // OK
std::cout << ps.top().first << '\n';    // OK
std::cout << ps.top().second << '\n';   // OK
```
只有当你为这样的栈调用 `printOn()` 时，代码才会产生错误，因为它**无法实例化**对于这种特定元素类型的 `operator<<` 的调用：
```cpp
ps.printOn(std::cout);                  // ERROR: operator<< not supported for std::pair<>
```

### 2.3.1 Concepts

这引出了一个问题：我们如何知道哪些操作是需要的，以便实例化一个模板？术语 “Concept” 通常用于表示在模板库中重复要求的一组约束。例如，C++ 标准库依赖于诸如随机访问迭代器和默认可构造类型之类的 concepts。

目前（即**C++17**），concepts 更多地可以通过文档（例如代码注释）来表达。这可能成为一个重大问题，因为未遵循约束可能导致可怕的错误消息（参见[第9.4节](./ch9.md#94)）。

多年来，已经有了一些支持定义和验证 concepts 作为一种语言特性的方法和尝试。然而，直到 C++17 为止，尚未标准化这样的方法。

自 C++11 以来，您至少可以通过使用`static_assert`关键字和一些预定义的类型特征来检查一些基本约束。例如：
```cpp
template<typename T>
class C
{
    static_assert(std::is_default_constructible<T>::value, "Class C requires default-constructible elements");
    ...
};
```
如果没有这个断言，即使需要默认构造函数，编译仍然会失败。然而，错误消息可能会包含从实例化的初始原因到检测到错误的实际模板定义的整个模板实例化历史（参见[第9.4节](./ch9.md#94)）。

然而，为了检查更复杂的代码，例如，对象类型 `T` 是否提供了特定的成员函数或者它们是否可以使用`operator<` 进行比较，需要更复杂的代码。有关此类代码的详细示例，请参见[第19.6.3节](../Part2/ch19.md#1963)。

有关 C++ Concepts 的详细讨论，请参阅附录 E。

## 2.4. 友元

为了打印堆栈内容，最好实现堆栈的`operator<<`而不是`printOn()`。然而，通常情况下，`operator<<`必须作为**非成员函数**实现，然后可以内联调用`printOn()`：
```cpp
template<typename T>
class Stack
{
    ...
    void printOn(std::ostream& strm) const
    {
        ...
    }
    friend std::ostream& operator<<(std::ostream& strm, Stack<T> const& s)
    {
        s.printOn(strm);
        return strm;
    }
};
```
注意，这意味着`class Stack<>`的`operator<<`不是函数模板，而是根据需要实例化的**普通**函数[2]。

然而，当尝试声明友元函数并在后面定义它时，情况变得更加复杂。实际上，我们有两个选择：

1. 我们可以隐式声明一个新的函数模板，该模板必须使用不同的模板参数，例如 `U`:
```cpp
template<typename T>
class Stack
{
    ...
    template<typename U>
    friend std::ostream& operator<<(std::ostream&, Stack<U> const&);
};
```
既不能再次使用 `T`，也不能跳过模板参数的声明（内部的 `T` 会隐藏外部的 `T`，或者我们在命名空间范围内声明一个非模板函数）。
2. 我们可以提前声明一个模板化的 `Stack<T>` 输出运算符，这意味着我们首先必须提前声明 `Stack<T>`：
```cpp
template<typename T>
class Stack;
template<typename T>
std::ostream& operator<<(std::ostream&, Stack<T> const&);
```
随后，可以定义该函数为友元函数：
```cpp
template<typename T>
class Stack
{
    ...
    friend std::ostream& operator<< <T>(std::ostream&, Stack<T> const&);
};
```
请注意在“函数名”`operator<<`后面的`<T>`。因此，我们声明了**非成员函数模板**的一个**特化版本**为**友元**。如果没有`<T>`，我们将声明一个新的非模板函数。详细信息请参阅[第12.5.2节](./ch12.md#1252)。

无论如何，您仍然可以使用这个类来处理没有定义`operator<<`的元素。只有调用该栈的`operator<<`会导致错误。
```cpp
Stack<std::pair<int, int>> ps;          // note: std::pair<> has no operator<< defined
ps.push({4, 5});                        // OK
ps.push({6, 7});                        // OK
std::cout << ps.top().first << '\n';    // OK
std::cout << ps.top().second << '\n';   // OK
std::cout << ps << '\n';                // ERROR: operator<< not supported for element type
```

## 2.5. 类模板的特例化

您可以为**特定的模板参数特例化类模板**。与函数模板的重载类似（参见[第1.5节](./ch1.md#15-函数模板重载)），**特例化类模板允许您优化特定类型的实现，或者修复某些类型在类模板的实例化中的不良行为**。然而，如果您特例化了一个类模板，则必须同时特例化所有成员函数。虽然可以特例化类模板的单个成员函数，但一旦这样做了，您就不能再特例化包含特例化成员的整个类模板实例。

要特例化一个类模板，您必须使用带有 `template<>` 前缀并指定类模板特例化的类型的声明。这些类型作为模板参数使用，必须直接跟在类名后面指定：
```cpp
template<>
class Stack<std::string>
{
    ...
};
```
对于这些特例化，任何成员函数的定义都必须定义为**普通**成员函数，其中每个 `T` 的出现都被替换为**特例化的类型**：
```cpp
void Stack<std::string>::push(std::string const& elem)
{
    elems.push_back(elem);      // append copy of passed elem
}
```
以下是针对类型`std::string`的`Stack<>`模板的完整特例化示例：[Codes/ch02/2_5/stack2.hpp](../../Codes/ch02/2_5/stack2.hpp)

```cpp
#include "../2_1/stack1.hpp"
#include <deque>
#include <string>
#include <cassert>

template<>
class Stack<std::string>
{
private:
    std::deque<std::string> elems;          // 元素
public:
    void push(std::string const& elem);     // 插入一个元素
    void pop();                             // 推出一个元素
    std::string const& top() const;         // 返回栈顶元素
    bool empty() const                      // 返回栈是否为空
    {
        return elems.empty();
    }
};

void Stack<std::string>::push(std::string const& elem)
{
    elems.push_back(elem);                  // 将传递的 elem 元素的拷贝进行插入
}

void Stack<std::string>::pop()
{
    assert(!elems.empty());
    elems.pop_back();                       // 移除栈底元素
}

std::string const& Stack<std::string>::top() const
{
    assert(!elems.empty());
    return elems.back();                    // 返回栈顶元素的引用
}
```

在这个例子中，特例化使用引用语义将字符串参数传递给`push()`，对于这种特定类型来说更合理（尽管最好使用转发引用(forwarding reference)，这在第 [6.1](./ch6.md) 节中有讨论）。

另一个区别是使用 `deque` 而不是 `vector` 来管理栈内的元素。虽然在这里没有特别的好处，但它确实表明特例化的实现可能与主模板的实现看起来非常不同。

## 2.6. 部分特例化

类模板可以进行部分特化。您可以针对特定情况提供特殊的实现，但某些模板参数仍必须由用户定义。例如，我们可以为指针定义一个类模板 `Stack<>` 的特殊实现：
[Codes/ch02/2_6/stackpartspec.hpp](../../Codes/ch02/2_6/stackpartspec.hpp)

```cpp
#include "../2_1/stack1.hpp"

// 为指针而实现的 Stack<> 的部分特例化

template <typename T>
class Stack<T*>
{
private:
    // 元素
    std::vector<T*> elems;
public:
    // 栈顶插入指针
    void push(T*);
    // 推出栈顶指针
    T* pop();
    // 返回栈顶元素
    T* top() const
    // 返回栈是否为空
    bool empty() const
    {
        return elems.empty();
    }
};

template <typename T>
void Stack<T*>::push(T* elem)
{
    elems.push_back(elem);
}

template <typename T>
T* Stack<T*>::pop()
{
    assert(!elems.empty());
    T* p = elems.back();
    elems.pop_back();
    return p;
}

template <typename T>
T* Stack<T*>::top() const
{
    assert(!elems.empty());
    return elems.back();
}
```

使用
```cpp
template<typename T>
class Stack<T*>{};
```
我们定义了一个类模板，其依然是针对 `T` 进行参数化，但却是对一个指针进行特例化(`Stack<T*>`)。

需要再次注意的是，特例化可能会提供一个不同的接口。例如，这里的 `pop()` 返回了存储的指针，当该类模板的用户使用 `new` 创建指针元素时，后面就可以调用 `delete` 返回来移除该值：
```cpp
Stack<int*> ptrStack;         // 指针栈
ptrStack.push(new int{42});
std::cout << *ptrStack.top() << '\n';
delete ptrStack.pop();
```

### 2.6.1. 多个模板参数的部分特例化

类模板可能也会在多个模板参数之间的关系上进行特例化，例如下面的类模板：
```cpp
template<typename T1, typename T2>
class MyClass{...};
```
以下的部分特例化是可能的：
```cpp
// 两个模板参数是相同类型的部分特例化
template<typename T>
class MyClass<T, T>
{...};

// 第二个模板参数类型为 int 的部分特例化
template<typename T>
class MyClass<T, int>{...};

// 两个参数类型都为指针类型的部分特例化
template<typename T1, typename T2>
class MyClass<T1*, T2*>{...};
```

下面的例子展示每一个声明使用了哪一个模板：
```cpp
MyClass<int, float> mif;    // MyClass<T1, T2>
MyClass<float, float> mff;  // MyClass<T, T>
MyClass<float, int> mfi;    // MyClass<T, int>
MyClass<int*, float*> mp;   // MyClass<T1*, T2*>
```

如果有超过一个部分特例化相同匹配的话，那么声明就是存在二义性的：
```cpp
MyClass<int, int> m         // ERROR: MyClass<T, T> 和 MyClass<T, int> 都匹配
MyClass<int*, int*> m;      // ERROR: MyClass<T, T> 和 MyClass<T1*, T2*> 都匹配
```

为了解决上面第二个歧义，可以针对指针，使用相同类型提供另一个部分特例化：
```cpp
template<typename T>
class MyClass<T*, T*>{...};
```

关于部分特例化的具体细节，可以参考 [16.4](../Part2/ch16.md) 节。

## 2.7. 类模板默认参数

与函数模板类似，您可以为类模板参数定义默认值。例如，在类模板 `Stack<>` 中，您可以将用于管理元素的容器定义为第二个模板参数，并使用 `std::vector<>` 作为默认值：[Codes/ch02/2_7/stack3.hpp](../../Codes/ch02/2_7/stack3.hpp)

```cpp
#include <vector>
#include <cassert>

template <typename T, typename Cont = std::vector<T>>
class Stack
{
private:
    Cont elems;
public:
    // 插入元素到栈顶
    void push(T const& elem);
    // 删除栈顶元素
    void pop();
    // 返回栈顶元素
    T const& top() const;
    // 判断是否为空
    bool empty() const
    {
        return elems.empty();
    }
};

template <typename T, typename Cont>
void Stack<T, Cont>::push(T const& elem)
{
    elems.push_back(elem);
}

template <typename T, typename Cont>
void Stack<T, Cont>::pop()
{
    assert(!elems.empty());
    return elems.pop_back();
}

template <typename T, typename Cont>
T const& Stack<T, Cont>::top() const
{
    assert(!elems.empty());
    return elems.back();
}
```

需要注意，我们现在有两个模板参数，所以，每个成员函数的定义必须提供两个参数：
```cpp
template <typename T, typename Cont>
void Stack<T, Cont>::push(T const& elem)
{
    elems.push_back(elem);
}
```

你可以像之前一样使用该模板栈，因此，如果你只传递第一个模板参数作为元素类型的话，`std::vector<T>` 就会被用来管理 `T` 类型的元素：
```cpp
template<typename T, typename Cont = std::vector<T>>
class Stack
{
private:
    Cont elems;     // 元素
    ...
};
```

此外，你可以在你的程序中，指定元素的容器：[../../Codes/ch02/2_7/stack3test.cpp](../../Codes/ch02/2_7/stack3test.cpp)
```cpp
#include "stack3.hpp"
#include <iostream>
#include <deque>

int main()
{
    // int 栈
    Stack<int> intStack;

    // 使用 std::deque<> 管理元素的 double 类型栈
    Stack<double, std::deque<double>> dblStack;

    // int 类型栈操作
    intStack.push(7);
    std::cout << intStack.top() << std::endl;
    intStack.pop();

    // double 类型栈操作
    dblStack.push(42.42);
    std::cout << dblStack.top() << std::endl;
    dblStack.pop();
    
    return 0;
}
```

使用
```cpp
Stack<double, std::deque<double>>
```
你可以声明使用 `std::deque<>` 来内部管理栈元素的 `double` 类型栈。

## 2.8. 类型别名

你可以针对完整类型，通过定义一个新的名称，以便更为便捷的使用一个类模板。

### 2.8.1. `typedef` 和别名声明

简单地定义一个现有类型的新名称有两种方法：

1. 使用关键字 `typedef`：
   
```cpp
typedef Stack<int> IntStack; // typedef
void foo(IntStack const& s); // s 是 int 栈
IntStack istack[10]; // istack 是包含 10 个 int 栈的数组
```

我们将这种声明称为 `typedef`[3]，生成的名称称为 `typedef`-名称。

2. 使用关键字 `using`（自 C++11 起）：

```cpp
using IntStack = Stack<int>; // 别名声明
void foo(IntStack const& s); // s 是 int 栈
IntStack istack[10]; // istack 是包含 10 个 int 栈的数组
```

这是由 DosReisMarcusAliasTemplates 引入的别名声明(alias declaration)。

请注意，在这两种情况下，我们都为**现有类型定义**了一个新名称，而**不是新类型**。因此，在 `typedef` 或 `using` 后：
```cpp
typedef Stack<int> IntStack;
```
或者
```cpp
using IntStack = Stack<int>;
```
`IntStack` 和 `Stack<int>` 是同一类型的两种可互换的表示法。

作为定义现有类型的新名称的两种替代方案的通用术语，我们使用术语“类型别名声明(type alias declaration)”。新名称是类型别名。

由于这种声明更易读（始终在 `=` 的左侧具有已定义的类型名称），在本书的其余部分中，我们更喜欢使用别名声明语法来声明类型别名。

### 2.8.2. 别名模板

与 `typedef` 不同，别名声明可以被模板化，以便为一组类型提供便捷的名称。自从 C++11 开始，这也是可用的，称为别名模板（alias template）[4]。

下面的别名模板 `DequeStack`，以元素类型 `T` 作为参数，扩展为使用 `std::deque` 存储其元素的 `Stack`：
```cpp
template<typename T>
using DequeStack = Stack<T, std::deque<T>>;
```

因此，类模板和别名模板都可以用作参数化类型。但是，**别名模板只是给现有类型一个新名称**，仍然可以使用该类型。`DequeStack<int>` 和 `Stack<int, std::deque<int>>` 表示相同的类型。

再次注意，通常情况下，模板只能在**全局**/**命名空间**范围内或**类声明**内声明和定义。

### 2.8.3. 成员类型的别名模板

对于一个类模板的成员类型，使用别名模板特别便于定义该成员类型的快捷方式。例如：
```cpp
struct C {
    typedef ... iterator;
    ...
};
```
或者：
```cpp
struct MyType {
    using iterator = ...;
};
```
那么以下的定义：
```cpp
template<typename T>
using MyTypeIterator = typename MyType<T>::iterator;
```
就可以直接使用：
```cpp
MyTypeIterator<int> pos;
```
而不是以下这种使用方式[5]：
```cpp
typename MyType<T>::iterator pos;
```

### 2.8.4. Type Traits Suffix_t

自 C++14 起，标准库使用这种技术为标准库中所有生成类型的类型特性定义简写。例如，可以写成：
```cpp
std::add_const_t<T>                 // 自 C++14 起
```
而不是：
```cpp
typename std::add_const<T>::type    // 自 C++11 起
```
标准库定义如下：
```cpp
namespace std {
    template<typename T> using add_const_t = typename add_const<T>::type;
}
```

## 2.9. 类模板参数推导

在 C++17 之前，你总是需要为类模板传递所有的模板参数类型（除非它们有默认值）。自 C++17 起，不再强制要求显式指定模板参数。只要构造函数能够推导出所有没有默认值的模板参数，你就可以省略显式定义模板参数。

例如，在之前的所有代码示例中，你可以在不指定模板参数的情况下使用复制构造函数：
```cpp
Stack<int> intStack1;               // int 类型的栈
Stack<int> intStack2 = intStack1;   // 在所有版本中都可以
Stack intStack3 = intStack1;        // 自 C++17 起可以
```

通过提供接收一些初始参数的构造函数，你可以支持栈元素类型的推导。例如，我们可以提供一个可以通过单个元素进行初始化的栈：
```cpp
template<typename T>
class Stack {
private:
    std::vector<T> elems;   // 元素
public:
    Stack() = default;
    Stack(T const& elem)    // 用一个元素初始化栈
    : elems({elem}) {
    }
    ...
};
```

这允许你如下声明一个栈：
```cpp
Stack intStack = 0; // 自 C++17 起可以推导为 Stack<int>
```

通过用整数 0 初始化栈，模板参数 T 被推导为 int，因此实例化了 `Stack<int>`。

注意以下几点：
- 由于定义了带有整型参数的构造函数，你需要请求默认构造函数具有其默认行为，因为只有在未定义其他构造函数时，默认构造函数才可用：
  ```cpp
  Stack() = default;
  ```
- 参数 `elem` 用大括号括起来传递给 `elems`，以使用包含 `elem` 作为唯一参数的初始化列表来初始化向量 `elems`：
  ```cpp
  : elems({elem})
  ```
  没有能够直接接受单个参数作为初始元素的向量构造函数[6]。

请注意，与函数模板不同，类模板参数**不能仅部分推导**（即只能显式指定一些模板参数）。详见第 15.12 节。

### 2.9.1. 使用字符串字面量的类模板参数推导

原则上，你甚至可以使用字符串字面量来初始化栈：
```cpp
Stack stringStack = "bottom"; // 自 C++17 起推导为 Stack<char const[7]>
```

但这会引发很多问题：通常情况下，当按引用传递模板类型 `T` 的参数时，参数不会衰变（decay，即将原始数组类型转换为相应的原始指针类型的机制）。这意味着我们实际上初始化了一个 `Stack<char const[7]>`，并在任何使用 T 的地方使用类型 `char const[7]`。例如，我们可能无法推送不同大小的字符串，因为它们具有不同的类型。详细讨论请参见第 7.4 节。

然而，当按值传递模板类型 T 的参数时，参数会衰变，即将原始数组类型转换为相应的原始指针类型的机制。也就是说，构造函数的调用参数 T 被推导为 `char const*`，因此整个类被推导为 `Stack<char const*>`。

出于这个原因，可能值得将构造函数声明为按值传递参数：
```cpp
template<typename T>
class Stack {
private:
    std::vector<T> elems;
public:
    Stack() = default;
    Stack(T elem) // 按值传递参数
    : elems({elem}) {
    }
    ...
};
```

有了这个，以下初始化可以正常工作：
```cpp
Stack stringStack = "bottom"; // 自C++17起，推导为Stack<char const*>
```

然而，在这种情况下，我们最好将临时的 `elem` 移动到栈中，以避免不必要的复制：
```cpp
template<typename T>
class Stack {
private:
    std::vector<T> elems;   // 元素
public:
    Stack(T elem)           // 通过值初始化栈并传递一个元素
    : elems({std::move(elem)}) {
    }
    ...
};
```

### 2.9.2. 推导指南（Deduction Guides）

与其声明构造函数以值传递参数，还有一种不同的解决方案：因为在容器中处理原始指针是个麻烦的来源，我们应该禁用对容器类**自动推导**原始字符指针。

你可以定义特定的推导指南，以提供额外的或修正现有的类模板参数推导。例如，你可以定义每当传递字符串字面量或 C 字符串时，栈应实例化为 `std::string`：
```cpp
Stack(char const*) -> Stack<std::string>;
```

此指南必须出现在与类定义相同的作用域（命名空间）中。通常，它会紧随类定义之后。我们称箭头后的类型为推导指南的指导类型。

现在，使用以下声明：
```cpp
Stack stringStack{"bottom"}; // 可以：自C++17起，推导为Stack<std::string>
```
推导出的栈是 `Stack<std::string>`。然而，以下仍然无法工作：
```cpp
Stack stringStack = "bottom"; // 推导为Stack<std::string>，但仍无效
```

我们推导出 `std::string`，因此我们实例化了一个 `Stack<std::string>`：
```cpp
class Stack {
    private:
        std::vector<std::string> elems;     // 元素
    public:
        Stack(std::string const& elem)
            : elems({elem}) {

        }
    ...
};
```

然而，根据语言规则，你不能通过传递一个字符串字面量给一个期望 `std::string` 的构造函数来进行**复制初始化**（使用 `=` 初始化）。因此，你必须如下初始化栈：
```cpp
Stack stringStack{"bottom"}; // 推导并有效为 Stack<std::string>
```

请注意，如果有疑问，类模板参数推导会进行复制。在将 `stringStack` 声明为 `Stack<std::string>` 之后，以下初始化声明了相同的类型（因此调用了复制构造函数），而不是通过字符串栈的元素来初始化一个栈：
```cpp
Stack stack2{stringStack};      // 推导为 Stack<std::string>
Stack stack3(stringStack);      // 推导为 Stack<std::string>
Stack stack4 = {stringStack};   // 推导为 Stack<std::string>
```
有关类模板参数推导的更多详细信息，请参见第 15.12 节。

## 2.10. 模板化聚合

聚合类（Aggregate class，没有用户提供的、显式的或继承的构造函数，没有私有或受保护的非静态数据成员，没有虚函数，并且没有虚拟的、私有的或受保护的基类的类/结构体）也可以是模板。例如：
```cpp
template<typename T>
struct ValueWithComment {
    T value;
    std::string comment;
};
```

这个定义了一个聚合，参数化为其持有的值 `val` 的类型。你可以像对待任何其他类模板一样声明对象，并且仍然可以将其用作聚合：

```cpp
ValueWithComment<int> vc;
vc.value = 42;
vc.comment = "initial value";
```

自 C++17 起，你甚至可以为聚合类模板定义推导指引：
```cpp
ValueWithComment(char const*, char const*)
-> ValueWithComment<std::string>;
ValueWithComment vc2 = {"hello", "initial value"};
```

如果没有推导指引，这种初始化将不可能，因为 `ValueWithComment` 没有构造函数可以进行推导。

标准库类 `std::array<>` 也是一个聚合，参数化为元素类型和大小。C++17 标准库还为其定义了推导指引，我们将在第 4.4.4 节中讨论这一点。

## 2.11. 总结

- **类模板**是一个实现时留有一个或多个类型参数未确定的类；
- 要使用类模板，您需要将这些未确定的类型作为模板参数传递。然后类模板将针对这些类型进行实例化（和编译）；
- 对于类模板，仅实例化被调用的成员函数；
- 您可以针对某些类型专门化类模板；
- 您可以对某些类型进行部分专门化类模板；
- 自 C++17 起，类模板参数可以从构造函数中自动推导；
- 您可以定义聚合类模板；
- 如果模板类型的调用参数声明为按值调用，则这些参数会衰减；
- 模板只能在全局/命名空间范围内或类声明内部声明和定义；

[1]: C++17 引入了类模板参数推导，允许跳过模板参数，如果它们可以从构造函数推导出来。这将在[第2.9节](#29)中讨论。

[2]: 这是一个模板后的实体，详细参考 [12.1](../Part2/ch12.md) 节。

[3]: 使用关键字 `typedef` 而不是“类型定义”是有意的。关键字 `typedef` 最初的意图是表示“类型定义”。然而，在 C++ 中，“类型定义”实际上指的是其他内容（例如，类或枚举类型的定义）。相反，`typedef` 应该被视为现有类型的替代名称（“别名”），可以通过 `typedef` 实现这一点。

[4]: 别名模板有时被错误地称为“typedef模板”，因为它们实现了，如果typedef可以作为模板使用时所能实现的相同作用。

[5]: 这里需要使用 `typename`，因为该成员是一个类型。详细信息请参见第 5.1 节。

[6]: 更糟的是，向量构造函数会接受一个整数参数作为初始大小，因此对于初始值为 `5` 的栈，如果使用 `elems(elem)`，向量将得到五个元素的初始大小。