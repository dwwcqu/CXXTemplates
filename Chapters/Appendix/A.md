# Appendix A. 唯一定义规则(One-Definition Rule, ODR)

简称为 ODR 的“唯一定义规则”是构建良好的 C++ 程序结构的基石。ODR 的最常见后果足够简单，容易记住并应用：在所有文件中，非内联函数或对象必须定义一次，而类、内联函数和内联变量在每个翻译单元中最多定义一次，并确保同一实体的所有定义是相同的。

然而，细节往往更为复杂，尤其是在结合模板实例化时，这些细节可能令人望而生畏。本附录旨在为有兴趣的读者提供 ODR 的全面概述，并指出相关问题在正文中详细讨论的部分。

## A.1. 翻译单元(Translation Units)

在实践中，我们通过在文件中编写“代码”来编写 C++ 程序。然而，在 ODR 的上下文中，文件边界并不重要。相反，重要的是翻译单元。基本上，翻译单元是将预处理器应用于编译器所处理的文件后的结果。预处理器会删除未被条件编译指令（如 `#if`、`#ifdef` 等）选中的代码部分、删除注释、插入 `#include` 的文件（**递归处理**）并展开宏。

因此，就 ODR 而言，以下两个文件

```cpp
// header.hpp:
#ifdef DO_DEBUG
#define debug(x) std::cout << x << '\n'
#else
#define debug(x)
#endif
void debugInit();

// myprog.cpp:
#include "header.hpp"
int main() {
    debugInit();
    debug("main()");
}
```

相当于以下单个文件：

```cpp
// myprog.cpp:
void debugInit();
int main() {
    debugInit();
}
```

翻译单元之间的连接是通过在两个翻译单元中具有相应的外部链接声明（例如，全局函数 `debugInit()` 的两个声明）来建立的。

请注意，翻译单元的概念比“预处理后的文件”更为抽象。例如，如果我们将预处理后的文件两次输入到编译器中形成一个程序，它将为程序带来两个不同的翻译单元（尽管这样做没有意义）。

## A.2. 声明和定义

在常见的“程序员对话”中，“**声明(declaration)**”和“**定义(definition)**”这两个术语经常被互换使用。然而，在 ODR 的上下文中，这些词的确切含义很重要[^1]。

声明是一个 C++ 构造，它（通常[^2]）在你的程序中引入或重新引入一个名字。声明也可以是定义，具体取决于它引入的实体及其引入方式：

- 命名空间和命名空间别名：命名空间及其别名的声明总是定义，尽管在这种情况下使用“定义”一词不常见，因为命名空间的成员列表可以在稍后时间“扩展”（与类和枚举类型不同）；
- 类、类模板、函数、函数模板、成员函数和成员函数模板：只有当声明包含与名字相关的括号封闭体时，该声明才是定义。这条规则包括联合体、运算符、成员运算符、静态成员函数、构造函数和析构函数，以及此类实体的模板版本的显式特化；
- 枚举：声明是定义，前提是它包含枚举器的括号封闭列表；
- 局部变量和非静态数据成员：这些实体通常可以视为定义，尽管区分通常不重要。注意，**函数定义中的函数参数声明本身是定义，因为它表示局部变量，但如果函数声明不是定义，则函数参数声明不是定义**；
- 全局变量：如果声明没有直接加上关键字 `extern` 或包含初始化器，则全局变量的声明也是该变量的定义。否则，它不是定义；
- 静态数据成员：只有当**声明出现在其成员所属的类或类模板之外**时，或它在类或类模板中被声明为内联或 `constexpr` 时，声明才是定义；
- 显式和部分特化：如果 `template<>` 或 `template<...>` 之后的声明本身是定义，那么该声明就是定义，除了静态数据成员或静态数据成员模板的显式特化，只有当它包含初始化器时才是定义；

其他声明不是定义。包括使用 `typedef` 或 `using` 的类型别名，`using` 声明，`using` 指令，模板参数声明，显式实例化指令，`static_assert` 声明等。

[^1]: 我们也认为，在讨论 C 和 C++ 时，谨慎处理术语是一个好的习惯。在整本书中，我们都遵循这一原则。
[^2]: 有些构造（如 `static_assert`）不引入任何名称，但在语法上被视为声明。

## A.3. 唯一定义规则的详细说明

如我们在本附录的介绍中所暗示的，ODR 实际上有许多细节。我们按范围组织规则的约束。

### A.3.1. 每个程序一次的约束

以下项目每个程序最多只能有一个定义：

- 非内联函数和非内联成员函数（包括函数模板的完全特化）
- 非内联变量（本质上是命名空间范围或全局范围内声明的变量，且没有静态说明符）
- 非内联静态数据成员

例如，由以下两个翻译单元组成的 C++ 程序是无效的：

```cpp
// 翻译单元 1:
int counter;

// 翻译单元 2:
int counter; // 错误：定义了两次（ODR 违规）
```

此规则不适用于具有内部链接的实体（本质上是在全局范围或命名空间范围内用静态说明符声明的实体），因为即使两个这样的实体具有相同的名称，它们也被视为不同。同样，如果实体在不同的翻译单元中出现，则位于未命名命名空间中的实体也被视为不同；在 C++11 及以后版本中，此类实体默认也具有内部链接，但在 C++11 之前，它们默认具有外部链接。例如，以下两个翻译单元可以组合成一个有效的 C++ 程序：

```cpp
// 翻译单元 1:
static int counter = 2; // 与其他翻译单元无关
namespace {
    void unique() { // 与其他翻译单元无关
    }
}

// 翻译单元 2:
static int counter = 0; // 与其他翻译单元无关
namespace {
    void unique() { // 与其他翻译单元无关
        ++counter;
    }
}

int main() {
    unique();
}
```

此外，如果这些项目在除 `constexpr if` 语句丢弃的分支之外的上下文中被使用，则程序中必须恰好有一个（此功能仅在 C++17 中可用；参见第 14.6 节）。在这种情况下，使用一词具有精确的含义。它表示程序中的某处对实体进行了某种引用，从而导致实体对于直接代码生成是必需的。这个引用可以是对变量值的访问，对函数的调用，或对该实体的地址的引用。这个引用可以是显式的，也可以是隐式的。例如，`new` 表达式可能会创建一个隐式调用关联的 `delete` 运算符，以处理当构造函数抛出异常时需要清理未使用（但已分配）内存的情况。另一个例子是复制构造函数，即使它们最终被优化掉了，也必须定义（除非语言要求它们被优化掉，在 C++17 中这种情况经常发生）。虚函数也被隐式使用（通过启用虚函数调用的内部结构），除非它们是纯虚函数。

还有几种其他类型的隐式使用存在，但我们为了简洁起见省略了它们。

某些引用不构成前述意义上的使用：那些出现在未求值操作数中的引用（例如 `sizeof` 或 `decltype` 运算符的操作数）。`typeid` 运算符的操作数（参见第 9.1.1 节）仅在某些情况下未被求值。具体来说，如果引用出现在 `typeid` 运算符中，则除非 `typeid` 运算符的参数最终指定了多态对象（具有——可能继承的——虚函数的对象），否则该引用不构成前述意义上的使用。例如，考虑以下单文件程序：

```cpp
#include <typeinfo>
class Decider {
#if defined(DYNAMIC)
    virtual ~Decider() {}
#endif
};

extern Decider d;

int main() {
    const char* name = typeid(d).name();
    return (int)sizeof(d);
}
```

如果未定义预处理符号 `DYNAMIC`，则此程序有效。确实，变量 `d` 未定义，但 `sizeof(d)` 中的引用不构成使用，且 `typeid(d)` 中的引用仅当 `d` 是多态类型的对象时才构成使用（因为一般来说，在运行时之前不总是可能确定多态 `typeid` 操作的结果）。

根据 C++ 标准，本节描述的约束不要求 C++ 实现提供诊断。在实践中，它们通常由链接器报告为重复或缺失的定义。

### A.3.2 每个翻译单元的一次性约束

在一个翻译单元中，任何实体都不能被定义多次。因此，以下示例是无效的 C++ 代码：

```cpp
inline void f() {}
inline void f() {} // 错误：重复定义
```

这也是在头文件中使用防护措施（guards）的主要原因之一：

```cpp
// guarddemo.hpp:
#ifndef GUARDDEMO_HPP
#define GUARDDEMO_HPP
...
#endif // GUARDDEMO_HPP
```

这些防护措施确保当一个头文件被多次 `#include` 时，它的内容会被丢弃，从而**避免了类、内联实体、模板等的重复定义**。

ODR（One-Definition Rule）还规定，在某些情况下，某些实体必须被定义。这可能适用于类类型、内联函数和内联变量。在接下来的段落中，我们将详细回顾这些规则。

一个类类型 X（包括结构体和联合体）必须在一个翻译单元中定义，然后才可以在该翻译单元中进行以下几种使用：

- 创建类型为 X 的对象（例如，作为变量声明或通过 `new` 表达式）。**创建可能是间接的，例如，当一个包含 X 类型对象的对象被创建时**；
- 声明一个类型为 X 的数据成员；
- 对类型为 X 的对象应用 `sizeof` 或 `typeid` 操作符；
- 显式或隐式地访问 X 类型的成员；
- 使用任何类型的转换将表达式转换为 X 类型或从 X 类型转换，或通过隐式转换、`static_cast`、`dynamic_cast` 将表达式转换为或从指向 X 的指针或引用（除了 `void*`）；
- 为类型为 X 的对象赋值；
- 定义或调用一个带有 X 类型参数或返回类型的函数。然而，仅仅声明这样的函数不需要定义该类型；

这些规则同样适用于从类模板生成的类型 X，这意味着在需要定义类型 X 的情况下，必须定义相应的模板。这些情况会创建实例化点或 POIs（见第 14.3.2 节）。

**内联函数必须在每个使用它们的翻译单元中定义**（例如，在调用它们或获取它们的地址时）。然而，与类类型不同的是，它们的定义可以在使用点之后出现：

```cpp
inline int notSoFast();
int main()
{
    notSoFast();
}
inline int notSoFast()
{
}
```

虽然这是合法的 C++ 代码，但一些基于旧技术的编译器实际上不会内联一个尚未看到其函数体的函数调用；因此，可能无法达到预期效果。与类模板类似，从参数化函数声明（函数或成员函数模板，或类模板的成员函数）生成的函数的使用会创建一个实例化点。然而，与类模板不同的是，相应的定义可以出现在实例化点之后。

本小节中解释的 ODR 各个方面通常可以被 C++ 编译器轻松验证；因此，C++ 标准要求编译器在违反这些规则时发出某种诊断信息。例外情况是参数化函数的定义缺失，这种情况通常不会被诊断出来。