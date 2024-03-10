# 1. 函数模板

本章介绍函数模板。函数模板是参数化的函数，代表一个函数族。

## 1.1. 函数模板初探

函数模板提供了可以针对不同类型调用的功能行为。换句话说，函数模板表示一组函数。其表示形式看起来很像普通函数，
只是函数的一些元素被保留为未确定的：这些元素被参数化。为了说明这一点，让我们看一个简单的例子。

### 1.1.1. 定义模板

下面的函数模板返回两个值的最大值：
[Codes/ch01/1_1_1/max1.hpp](../../Codes/ch01/1_1/max1.hpp)
```c++
template<typename T>
T max(T a, T b)
{
    // 如果 b < a，则返回 a，否则返回 b
    return b < a ? a : b;
}
```
这个模板定义指定了一组函数，这些函数返回作为函数参数 `a` 和 `b` 传递的两个值的最大值。
这些参数的类型被作为模板参数 `T` 留待确定。正如在这个例子中所看到的，
模板参数必须用以下形式的语法进行声明：
```c++
template<commad-separated-list-of-parameters>
```

在我们的例子中，参数列表是 `typename T`。注意 `<` 和 `>` 标记被用作尖括号；我们将它们称为尖括号。
关键字 `typename` 引入了一个类型参数。这是 C++ 程序中最常见的模板参数类型，但还有其他可能的参数类型，我们将在后面讨论（见第3章）。

在这里，类型参数是 `T`。你可以使用任何标识符作为参数名，但使用 `T` 是一种约定。类型参数表示由调用者在调用函数时确定的任意类型。
只要它提供模板使用的操作，你可以使用任何类型（基本类型、类等）。在这个例子中，类型 `T` 必须支持 `operator <`，
因为 `a` 和 `b` 是使用这个运算符进行比较的。也许从 `max()` 的定义中不太明显的是，为了返回，`T` 类型的值也必须是可复制的。

由于历史原因，你还可以使用关键字 `class` 而不是 `typename` 来定义类型参数。关键字 `typename` 相对较晚出现在 C++98 标准的演变中。
在那之前，关键字 `class` 是引入类型参数的唯一方式，这仍然是一种有效的方式。因此，模板 `max()` 可以等效地定义如下：
```c++
template<class T>
T max(T a, T b)
{
    return b < a ? a : b;
}
```

在这个上下文中，在语义上没有区别。因此，即使在这里使用 `class`，任何类型都可以用作模板参数。但是，因为使用 `class` 可能会产生歧义
（不仅类类型可以替代 `T`），在这个上下文中应该更倾向于使用 `typename`。然而，请注意，与类类型声明不同，当声明类型参数时，
不能使用关键字 `struct` 替代 `typename`。

### 1.1.2. 使用模板

下面的程序展示了如何使用 `max()` 函数模板：[Codes/ch01/1_1_1/max1.cpp](../../Codes/ch01/1_1/max1.cpp)
```c++
#include "max1.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    int i = 42;
    std::cout << "max(7, i): " << ::max(7, i) << std::endl;

    double f1 = 3.4;
    double f2 = -6.7;
    std::cout << "max(f1, f2): " << ::max(f1, f2) << std::endl;

    std::string s1 = "mathematics";
    std::string s2 = "math";
    std::cout << "max(s1, s2): " << ::max(s1, s2) << std::endl;

    return 0;
}
```
在程序中，`max()` 被调用了三次：一次用于两个整数，一次用于两个双精度浮点数，以及一次用于两个 `std::string`。
每次都计算了最大值。因此，程序的输出如下：
```bash
max(7, i): 42
max(f1, f2): 3.4
max(s1, s2): mathematics
```
请注意，每次调用 `max()` 模板时都带有 `::` 限定符。这是为了确保我们的 `max()` 模板在全局命名空间中被找到。
在标准库中还有一个 `std::max()` 模板，可能在某些情况下会被调用或导致歧义[^3]。

[^3]: 例如，如果一个参数类型在命名空间 `std` 中定义（比如 `std::string`），根据C++的查找规则，将找到全局的 `max()` 模板和 `std` 命名空间中的 `max()` 模板（参见附录 C）。

模板不会被编译为能处理任何类型的单一实体。相反，对于使用模板的每种类型，都会从模板生成不同的实体[^4]。
因此，`max()` 针对这三种类型中的每一种都会被编译。例如，第一次调用 `max()`：
```c++
int i = 42;
max(7, i);
```
使用具有 `int` 作为模板参数 `T` 的函数模板。因此，它具有调用以下代码的语义：
```c++
int max(int a, int b)
{
    return b < a ? a : b;
}
```

[^4]:一个实体适应所有（one-entity-fits-all）的替代方案是可以想象的，但在实践中没有被使用（在运行时效率上会更低）。所有的语言规则都是基于这样一个原则，即为不同的模板参数生成不同的实体。

将模板参数替换为具体类型的过程称为**实例化(instantiation)**。它会产生模板的一个实例[^5]。

[^5]: 术语“实例”和“实例化”在面向对象编程中使用的上下文中有不同的含义，即指类的具体对象。然而，因为这本书是关于模板的，除非另有说明，我们使用这个术语来表示对模板的“使用”。

请注意，仅仅使用一个函数模板就可以触发这样一个实例化的过程。程序员无需单独请求实例化。

同样，对 `max()` 的其他调用将为 `double` 和 `std::string` 实例化 `max` 模板，就好像它们被单独声明和实现一样：
```c++
double max (double, double);
std::string max (std::string, std::string);
```

还要注意，`void` 也是一个有效的模板参数，只要生成的代码是有效的。例如：
```c++
template<typename T>
T foo(T*)
{}

void* vp = nullptr;
foo(vp);                // void foo(void*)
```

### 1.1.3. 两阶段的转换

如果尝试为一个不支持所有操作的类型实例化模板，将导致编译时错误。例如
```c++
std::complex<float> c1, c2; // 没有提供 operator< 运算符
::max(c1, c2);              // 编译时报错
```

因此，模板在两个阶段进行“编译”：
1. 在定义时，不进行实例化，仅检查模板代码本身的正确性，忽略模板参数。这包括：
   - 发现语法错误，如缺少分号；
   - 发现使用未知名称（类型名称、函数名称等）的错误，这些名称不依赖于模板参数；
   - 检查不依赖于模板参数的静态断言；
2. 在实例化时，再次检查模板代码，以确保所有代码都是有效的。也就是说，现在特别仔细检查所有依赖于模板参数的部分。
例如：
```c++
template<typename T>
void foo(T t)
{
    undeclared();   // 如果 undeclared() 未知，在编译第一阶段编译失败
    undeclared(t);  // 如果 undeclared(T) 未知，在编译第二阶段编译失败
    static_assert(sizeof(int) > 10, "int too small");   // 如果 sizeof(int) <= 10，编译失败
    static_assert(sizeof(T) > 10, "T too small");       // 如果实例化类型 T 的大小 <= 10，实例化失败
}
```

名称被检查两次的事实被称为两阶段查找，并在第[14.3.1](../Part2/ch14.md#1431)节详细讨论。

请注意，有些编译器不执行第一阶段的完整检查[^6]。因此，您可能在实例化模板代码一次之前，看不到模板本身的一般性问题。

[^6]: 例如，Visual C++ 编译器在某些版本中（例如 Visual Studio 2013 和 2015）允许未声明的不依赖于模板参数的名称，甚至一些语法错误（如缺少分号）。

两阶段编译在实践中处理模板时导致了一个重要的问题：当以触发实例化的方式使用函数模板时，编译器（在某个时刻）需要看到该模板的定义。这打破了通常对于普通函数的编译和链接区别的规则，即函数的声明足以编译其使用。处理这个问题的方法在[第 9 章](../Part1/ch9.md)中进行了讨论。目前，让我们采取最简单的方法：将每个模板实现在一个头文件中。

## 1.2. 模板参数推导

当我们调用一个函数模板，比如 `max()`，并传入一些参数时，模板参数会根据我们传递的参数确定。如果我们传递两个整数给参数类型 `T`，
C++ 编译器必须推断 `T` 必须是 `int`。

然而，`T` 可能只是类型的**一部分**。例如，如果我们声明 `max()` 使用常量引用：
```c++
template<typename T>
T max(T const& a, T const& b)
{
    return b < a ? a : b;
}
```
并传递 `int`，那么 `T` 会被推导为 `int`, 因为函数参数可以与 `int const&` 进行匹配。

### 1.2.1 在类型推导时的类型转换

请注意，在类型推导期间，自动类型转换是有限制的：

- 在通过**引用声明**调用参数时，即使是很小的转换也不适用于类型推导。使用相同模板参数 `T` 声明的两个参数必须完全匹配。
- 在通过**值声明**调用参数时，仅支持衰减(decay)的很小转换：忽略带有 `const` 或 `volatile` 的修饰符，引用转换为引用的类型，原始数组或函数转换为相应的指针类型。对于使用相同模板参数 `T` 声明的两个参数，衰减后的类型必须匹配。

例如：
```c++
template<typename T>
T max(T a, T b);

int i = 33;
int const c = 42;
max(i, c);          // OK: T is int
max(c, c);          // OK: T is int
int& ir = i;
max(i, ir);         // OK: T is int
int arr[4];
foo(&i, arr);       // OK: T is int*
```

但是下面的类型推导会存在问题：
```c++
max(4, 7.2);        // ERROR: T deduced as int or double
std::string s;
max("hello", s);    // ERROR: T deduced as char const [6] or std::string
```

这里有三种办法去解决这些问题：
1. 转换参数，使其两者都满足参数类型的推导：
```c++
max(static_cast<double>(4), 7.2);   // OK
```
2. 显式地指定参数的类型，以避免编译器进行类型推导：
```c++
max<double>(4, 7.2);                // OK
```
3. 在声明模板时，指定其参数可能会有不同的类型。

### 1.2.2 默认参数的类型推导

需要注意的是，对于默认参数的调用，类型推导不会起作用。例如：
```c++
template<typename T>
void f(T = "")

f(1);               // OK: 推导 T 为 int，所以调用 f<int>(1)
f();                // ERROR: 无法推导 T
```

为了支持这类情况，你需要为模板参数指定一个默认参数(在 [1.4](#14) 节会进行讨论)，例如：
```c++
template<typename T = std::string>
void f(T = "");

foo();          // OK
```

## 1.3. 有多个模板参数的模板

到目前为止，函数模板有两个不同的参数集合：
1. 模板参数，定义在函数模板名前面的方括号 `<>` 中：
```c++
template<typename T>    // T 为模板参数
```
2. 调用参数，声明在函数模板名后面的方括号中 `()` 中：
```c++
T max (T a, T b)        // a, b 为调用参数
```

你可以根据自己的需要定义多个模板参数。例如，你可以定义有两个不同调用类型的模板 `max()`：
```c++
template<typename T1, typename T2>
T1 max(T1 a, T2 b)
{
    return b < a ? a : b;
}

auto m = ::max(4, 7.2);     // OK，使用第一个参数的类型作为返回类型
```

可能希望能够将不同类型的参数传递给 `max()` 模板，但是，正如这个例子所展示的，这会引发一个问题。如果将其中一个参数类型用作返回类型，
另一个参数的参数可能会被转换为此类型，而不管调用者的意图如何。因此，返回类型取决于调用参数的顺序。66.66 和 42 的最大值将是双精度浮点数 66.66，而 42 和 66.66 的最大值将是整数 66。

C++ 提供了不同的方法来解决这个问题：
- 引入第三个模板参数作为返回类型；
- 让编译器找出返回类型；
- 声明返回类型为这两个参数类型的“通用类型”；

接下来将讨论所有这些选项。

### 1.3.1. 返回类型的模板参数

我们之前的讨论表明，**模板参数推导**允许我们使用与调用普通函数相同的语法调用函数模板：我们不必显式指定与模板参数相对应的类型。

然而，我们也提到过，我们可以显式指定要用于模板参数的类型：
```c++
template<typename T>
T max(T a, T b)
::max<double>(4, 7.2);          // 实例化 T 为 double
```
在没有模板和调用参数之间关联，并且无法确定模板参数的情况下，必须在调用时明确指定模板参数。
例如，可以引入第三个模板参数类型来定义函数模板的返回类型：
```c++
template<typename T1, typename T2, typename RT>
RT max (T1 a, T2 b);
```

然而，**模板参数推导不考虑返回类型[^7]**，而 `RT` 不出现在函数调用参数的类型中。因此，无法推导出 `RT`[^8]。

[^7]：推导可以被视为重载决议的一部分——这个过程不是基于选择返回类型的。唯一的例外是转换运算符成员的返回类型。

[^8]：在 C++ 中，从调用者使用调用的上下文中也无法推断出返回类型。

因此，您必须显式指定模板参数列表。例如：
```c++
template<typename T1, typename T2, typename RT>
RT max (T1 a, T2 b);

::max<int, double, double>(4, 7.2);     // OK：但是很繁琐
```
到目前为止，我们已经看到了显式提供所有函数模板参数，或不提供任何模板参数的情况。另一种方法是仅显式指定前几个参数，
并允许推导过程推导出其余参数。通常，您必须指定所有参数类型，直到不能隐式确定的最后一个参数类型。因此，
如果更改我们示例中模板参数的顺序，调用者只需指定返回类型：
```c++
template<typename RT, typename T1, typename T2>
RT max(T1 a, T2 b);

::max<double>(4, 7.2);                  // OK: 返回类型为 double，T1 和 T2 可以推导
```

在这个例子中，对 `max<double>` 的调用明确将 `RT` 设置为 `double`，但参数 `T1` 和 `T2` 从参数中被推导为 `int` 和 `double`。

请注意，这些修改版本的 `max()` 不会带来显著的优势。对于单参数版本，如果两个参数的类型已知，你已经可以指定参数（和返回）类型。
因此，保持简单并使用 `max()` 的单参数版本是个好主意（在接下来的章节中，我们在讨论其他模板问题时也会采用这种方式）。

有关推导过程的详细信息，请参阅第 [15](../Part2/ch15.md) 章。

### 1.3.2. 推导返回类型

如果返回类型取决于模板参数，最简单和最好的方法是让编译器自己找出。从 C++14 开始，这可以通过简单地不声明任何返回类型来实现（你仍然必须将返回类型声明为 `auto`）：[ch01/1_3/maxauto.hpp](../../Codes/ch01/1_3/maxauto.hpp)
```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b)
{
    return b < a ? a : b;
}
```
事实上，使用没有相应*尾随返回类型*（*trailing return type*, 应在末尾使用 `->` 引入）的 `auto` 作为返回类型，表明实际返回类型必须从函数体中的返回语句中推导出来。当然，从函数体中推导返回类型必须是可能的。因此，代码必须可用，并且多个返回语句必须匹配。

在 C++14 之前，要让编译器确定返回类型，只能更多或更少地将函数的实现部分放在其声明中。在 C++11 中，我们可以受益于*尾随返回类型*语法，允许我们使用调用参数。也就是说，我们可以声明返回类型是从 `operator?:` 产生的：
```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b) -> decltype(b < a ? a : b)
{
    return b < a ? : a : b;
}
```
在这里，结果类型由 `operator ?:` 的规则确定，这些规则相当复杂，但通常会产生一种直观的期望结果（例如，如果 `a` 和 `b` 具有不同的算术类型，将为结果找到一个公共的算术类型）。

需要注意：
```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b) -> decltype(b < a ? a : b);
```
只是一个*声明(declaration)*，这样，编译器在编译时使用了对参数 `a` 和 `b` 调用的 `operator ?:` 的规则来找出 `max()` 的返回类型。实现不一定要匹配。事实上，在声明中使用 `true` 作为 `operator ?:` 的条件就足够了：
```c++
template<typename T1, typename T2>
auto max (T1 a, T2 b) -> decltype(true ? a : b);
```
然而，在任何情况下，此定义具有一个显著的缺点：返回类型可能是一个引用类型，因为在某些条件下 `T` 可能是引用。因此，应该返回 `T` 衰减(*decayed*)的类型，具体如下：
```c++
#include <type_traits>

template<typename T1, typename T2>
auto max (T1 a, T2 b) -> typename std::decay<decltype(true?a:b)>::type
{
    return b < a ? a : b;
}
```
在这里，使用了类型特征 `std::decay<>`，它在一个成员类型中返回结果类型。它由标准库在 `<type_trait>` 中定义。由于成员类型是一种类型，因此您必须使用 `typename` 限定符来访问它（参见第 [5.1](ch5.md#51) 节）。

请注意，`auto` 类型的初始化总是*衰减(decay)*的。当返回类型仅为 `auto` 时，返回值也是如此。`auto` 作为返回类型的行为就像下面的代码中的 `a`，由 `i` 的衰减类型 `int` 声明的：
```c++
int i = 42;
int const& ir = i;      // ir 是 i 的引用类型
auto a = ir;            // a 定义为一个类型为 int 的新类型
```

### 1.3.3. 通用类型的返回类型

自从 C++11 以来，C++ 标准库提供了一种选择**更通用类型**的方式。`std::common_type<>::type` 产生作为模板参数传递的两个（或更多）不同类型的“公共类型”。例如：
```c++
#include <type_traits>

template<typename T1, typename T2>
std::common_type<T1, T2>::type max (T1 a, T2 b)
{
    return b < a ? a : b;
}
```
再次强调，`std::common_type` 是一个类型特征（type trait），定义在 `<type_traits>` 中，它产生一个具有结果类型的 `type` 成员的结构体。因此，其核心用法如下：
```cpp
typename std::common_type<T1, T2>::type // 自 C++11 起
```
然而，自 C++14 起，您可以通过在特征名后添加 `_t` 并省略 `typename` 和 `::type`，使得返回类型定义简化为：
```cpp
std::common_type_t<T1, T2> // C++14 起的等效写法
```
`std::common_type<>` 的实现方式使用了一些巧妙的模板编程，这在第 [26.5.2](../Part3/ch26.md#2652) 节中有详细讨论。在内部，它根据 `operator ?:` 的语言规则或特定类型的特化来选择结果类型。因此，`::max(4, 7.2)` 和 `::max(7.2, 4)` 都产生相同的值 `7.2`，其类型为 `double`。请注意，`std::common_type<>` 也会进行衰减。

## 1.4. 默认模板参数

您还可以为模板参数定义默认值。这些值称为**默认模板参数**[^9]，可以与任何类型的模板一起使用。它们甚至可以引用先前的模板参数。例如，如果您想结合使用定义返回类型的方法和具有多个参数类型的能力（如前面的部分所讨论），可以引入一个模板参数 `RT` 用作返回类型，其默认值为两个参数的通用类型。同样，我们有多个选项：
1. 我们可以直接使用 `operator?:`。但是，由于我们必须在声明调用参数 a 和 b 之前应用 `operator?:`，我们只能使用它们的类型：
```c++
#include <type_traits>

template<typename T1, typename T2,
        typename RT = std::decay_t<decltype<true ? T1() : T2()>>>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
再次注意使用 `std::decay_t<>` 以确保不会返回引用[^10]。

[^9]: 在C++11之前，由于在函数模板的开发中存在历史性的错误，只允许在类模板中使用默认的模板参数。

[^10]: 同样地，在 C++11 中，您必须使用`typename std::decay<...>::type`而不是`std::decay_t<...>`。

请注意，这个实现要求我们能够调用传递类型的**默认构造函数**。还有另一种解决方案，使用`std::declval`，然而，这会使声明变得更加复杂。有关示例，请参见第[11.2.3](../Part1/ch11.md#1123)节。

2. 我们可以使用 `std::common_type<>` 来指定返回类型的默认值：
```c++
#include <type_traits>

template<typename T1, typename T2,
        typename RT = std::common_type_t<T1, T2>>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```

作为调用者，在所有情况下，你可以使用默认值作为返回类型：
```c++
auto a = ::max(4, 7.2);
```
或者，你可以在所有其他模板参数类型的后面显示地指定返回类型：
```cpp
auto b = ::max<double, int, long double>(7.2, 4);
```
然而，我们再次面临的问题是，我们必须指定三种类型l来指定返回类型。我们需要能够将返回类型作为第一个模板参数，同时仍然能够从参数类型中推导出它。原则上，即使后面有没有默认参数的参数，也可以为前导函数模板参数设置默认参数：
```c++
template<typename RT = long, typename T1, typename T2>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```
在这里饿模板定义下，我们可以调用：
```cpp
int i;
long l;
...
max(i, l);  // return long: 默认模板返回类型
max<int>(4, 42); // return int: 显示指定
```
然而，这种方法只有在模板参数存在“自然”默认值的情况下才有意义。在这里，我们需要模板参数的默认值依赖于前面的模板参数。原则上，这是可能的，正如我们在第[26.5.1](../Part3/ch26.md#2651)节中讨论的那样，但这种技术依赖于类型特征并使定义变得更加复杂。
因此，出于所有这些原因，最好且最简单的解决方案是让编译器像在第[1.3.2](#132-推导返回类型)节中提出的那样推导返回类型。

## 1.5. 函数模板重载

像普通函数一样，函数模板可以重载。也就是说，你可以有不同的函数定义，它们使用相同的函数名，以便在函数调用中使用该名称时，C++ 编译器必须决定调用哪个候选函数。即使没有模板，这种决策的规则可能变得相当复杂。在本节中，我们讨论涉及模板时的重载。如果您对不涉及模板的基本重载规则不熟悉，请查看附录C，我们在那里提供了关于**重载解析规则**的相当详细的调查。

以下是一个简短的程序，演示了如何重载函数模板：[Codes/ch01/1_5/1_5_max.cpp](../../Codes/ch01/1_5/1_5_max.cpp)
```cpp
// maximum of two int values
int max(int a, int b)
{
    return b < a ? a : b;
}

// maximum of two values of any type:
template<typename T>
T max(T a, T b)
{
    return b < a ? a : b;
}

int main()
{
    ::max(7, 42);           // 调用类型为 int 非模板函数
    ::max(7.0, 42.0);       // 调用 max<double> 模板函数，推导类型
    ::max('a', 'b');        // 调用 max<char> 模板函数，推导类型
    ::max<>(7, 42);         // 调用 max<int> 模板函数，推导类型
    ::max<double>(7, 42);   // 调用 max<double> 模板函数，没有推导 
    ::max('a', 42.7);       // 调用类型为 int 的非模板函数
}
```

正如这个例子所示，一个非模板函数可以与一个具有相同名称并且可以用相同类型实例化的函数模板共存。在其他所有因素相等的情况下，**重载解析过程会优先选择非模板函数而不是从模板生成的函数**。第一次调用就符合这个规则：
```cpp
::max(7, 42); // 两个 int 类型值与非模板函数完美匹配
```

如果模板函数可以生成一个更好的匹配函数的话，那么就会选择模板函数：
```cpp
::max(7.0, 42.0);   // 通过参数推导调用 max<double>
::max('a', 'b');    // 通过参数推导调用 max<char>
```
在这里，模板是一个更好的匹配，因为不需要从 `double` 或 `char` 转换为 `int`（请参阅第 C.2 节的重载解析规则）。

也可以明确指定一个空的模板参数列表。这种语法表示只有模板可以解析调用，但所有模板参数都应从调用参数中推导出：
```cpp
::max<>(7, 42); // 通过参数推导调用 max<int>
```
由于对于推导的模板参数不考虑自动类型转换，但对于普通函数参数考虑，最后的调用使用了非模板函数（显然 `'a'` 和 `42.7` 都被转换为 `int`）：
```cpp
::max('a', 42.7);   // 只有非模板函数才允许 nontrivial 类型转换
```

这里是一种有趣的例子，可以重载模板以能够显式指定返回类型：
```cpp
template<typename T1, typename T2>
auto max(T1 a, T2 b)
{
    return b < a ? a : b;
}

template<typename RT, typename T2, typename T2>
RT max(T1 a, T2 b)
{
    return b < a ? a : b;
}
```

例如，我们此时可以调用 `max()` 函数：
```cpp
auto a = ::max(4, 7.2);                 // 使用第一个模板函数
auto b = ::max<long double>(7.2, 4);    // 使用第二个模板函数
```
然而，当调用：
```cpp
auto c = ::max<int>(4, 7.2);    // ERROR: 两个模板函数都匹配
```
这两个模板函数都匹配，这导致了通常会优先选择其中一个，并导致歧义错误的过载解析。因此，**在重载函数模板时，应确保它们中的任何一个都仅匹配任何调用**。

一个有用的例子是为指针和普通 C 字符串重载最大值模板：[Codes/ch01/1_5/1_5_maxval.cpp](../../Codes/ch01/1_5/1_5_maxval.cpp)
```cpp
#include <cstring>
#include <string>

// maximum of two values of any types
template<typename T>
T max(T a, T b) 
{
    return b < a ? a : b;
}

// maximum of two pointers
template<typename T>
T* max(T* a, T* b)
{
    return *b < *a ? a : b;
}

// maximum of two C-strings:
char const* max(char const* a, char const* b)
{
    return std::strcmp(b, a) < 0 ? a : b;
}

int main()
{
    int a = 7;
    int b = 42;
    auto m1 = ::max(a, b);      // max() for two values of int type

    std::string s1 = "hey";
    std::string s2 = "you";
    auto m2 = ::max(s1, s2);    // max() for two values of std::string type

    int* p1 = &b;
    int* p2 = &a;
    auto m3 = ::max(p1, p2);   // max() for two pointers

    char const* c1 = "hello";
    char const* c2 = "world";
    auto m4 = ::max(c1, c2);   // max() for two C-strings

    return 0;
}
```

请注意，在 `max()` 的所有重载中，我们通过值传递参数。通常，当重载函数模板时，最好只更改必要的部分。您应该限制更改到参数的数量或显式指定模板参数。否则，可能会发生意外的效果。例如，如果您将 `max()` 模板实现为通过引用传递参数并为通过值传递的两个 C 字符串重载它，您无法使用三参数版本来计算三个 C 字符串的最大值：[Codes/ch01/1_5/1_5_maxref.cpp](../../Codes/ch01/1_5/1_5_maxref.cpp)
```cpp

```