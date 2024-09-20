# 8. 编译时编程

C++ 一直以来都提供了一些简单的方法来在编译时计算值。模板极大地增加了这方面的可能性，语言的进一步发展也为此提供了更多的工具。

在简单的情况下，你可以决定是否使用某些模板代码，或者在不同的模板代码之间进行选择。但如果**所有必要的输入都在编译时可用，编译器甚至可以在编译时计算控制流的结果**。

事实上，C++ 具有多种支持编译时编程的特性：
- 在 C++98 之前，模板就已经提供了编译时计算的能力，包括使用循环和执行路径选择（不过，有些人认为这是对模板特性的“滥用”，例如因为它需要非直观的语法）；
- 通过部分特化，我们可以根据特定的约束或需求在编译时选择不同的类模板实现；
- 通过 SFINAE 原则，我们可以根据不同的类型或约束在不同的函数模板实现之间进行选择；
- 在 C++11 和 C++14 中，编译时计算得到了更好的支持，`constexpr` 特性提供了“直观”的执行路径选择，并且从 C++14 开始，大多数语句类型（包括 `for` 循环、`switch` 语句等）都可以在编译时使用；
- C++17 引入了“编译时 `if`”语句，以根据编译时条件或约束丢弃语句，它甚至可以在模板之外使用；

本章将介绍这些特性，特别关注模板的作用和背景。

## 8.1 模板元编程

模板是在编译时实例化的（与在运行时处理泛型动态语言不同）。事实证明，C++ 模板的一些特性可以与实例化过程结合，形成一种在 C++ 语言中存在的原始递归“编程语言[^1]”。因此，模板可以用于“计算一个程序”。第 23 章将详细介绍所有内容和特性，这里是一个简单的示例。

以下代码在编译时判断一个给定的数字是否为质数：[isprime.hpp](../../Codes/ch08/8_1/isprime.hpp)

```cpp
template<unsigned p, unsigned d>
struct DoIsPrime
{
    static constexpr bool value = (p % d != 0) && DoIsPrime<p, d - 1>::value;
};

template<unsigned p>
struct DoIsPrime<p, 2>
{
    static constexpr bool value = (p % 2 == 0);
};

template<unsigned p>
struct IsPrime
{
    static constexpr bool value = DoIsPrime<p, p/2>::value;
};

template<>
struct IsPrime<0>
{
    static constexpr bool value = false;
};

template<>
struct IsPrime<1>
{
    static constexpr bool value = false;
};

template<>
struct IsPrime<2>
{
    static constexpr bool value = true;
};

template<>
struct IsPrime<3>
{
    static constexpr bool value = true;
};
```

[^1]: 实际上，Erwin Unruh 是第一个通过展示一个在编译时计算质数的程序发现这一点的人。详见第 23.7 节。

模板 `IsPrime<>` 在其成员 `value` 中返回传递的模板参数 `p` 是否为质数。为此，它实例化了 `DoIsPrime<>`，该模板递归展开为一个表达式，检查从 `p/2` 到 `2` 的每个除数 `d` 是否能整除 `p`。

例如，表达式
```cpp
IsPrime<9>::value
```
展开为
```cpp
DoIsPrime<9,4>::value
```
进而展开为
```cpp
9%4!=0 && DoIsPrime<9,3>::value
```
继续展开为
```cpp
9%4!=0 && 9%3!=0 && DoIsPrime<9,2>::value
```
最终展开为
```cpp
9%4!=0 && 9%3!=0 && 9%2!=0
```
因为 `9%3` 为 0，计算结果为 `false`。

这个实例化链式表明：
- 我们使用 `DoIsPrime<>` 的递归展开来遍历从 `p/2` 到 `2` 的所有除数，判断是否有任何除数能够整除给定的整数（即没有余数）。
- 当除数 `d` 等于 2 时，`DoIsPrime<>` 的部分特化用于终止递归。

请注意，所有这些都是在编译时完成的。也就是说，
```cpp
IsPrime<9>::value
```
在编译时就展开为 `false`。

尽管模板语法显得笨拙，但类似的代码自 C++98（甚至更早）以来就是有效的，并且对一些库非常有用[^2]。

详见第 23 章。

[^2]: 在 C++11 之前，为了避免需要在类外定义静态数据成员，通常会将值成员声明为枚举常量而不是静态数据成员（详细内容见第 23.6 节）。例如：
```cpp
enum { value = (p % d != 0) && DoIsPrime<p, d-1>::value };
```

## 8.2 使用 `constexpr` 进行计算

C++11 引入了一个新特性 —— `constexpr`，它极大地简化了各种形式的编译时计算。特别是，给定适当的输入，`constexpr` 函数可以在编译时进行评估。虽然在 C++11 中，`constexpr` 函数有严格的限制（例如，每个 `constexpr` 函数的定义基本上被限制为只有一个 `return` 语句），但这些限制中的大多数在 C++14 中被取消了。当然，成功评估 `constexpr` 函数仍然要求所有计算步骤在编译时都是可能且有效的：目前，这排除了像堆分配或抛出异常等操作。

在 C++11 中，判断一个数是否为质数的示例可以这样实现：

```cpp
constexpr bool doIsPrime(unsigned p, unsigned d)
{
    return d != 2 ? (p % d != 0) && doIsPrime(p, d - 1) : (p % 2 != 0);
}

constexpr bool isPrime(unsigned p)
{
    return p < 4 ? !(p < 2) : doIsPrime(p, p/2);
}
```

由于只能使用一个语句的限制，我们只能使用条件运算符作为选择机制，仍然需要递归来遍历元素。不过，这里的语法是普通的 C++ 函数代码，比依赖模板实例化的第一个版本更易于理解。

在 C++14 中，`constexpr` 函数可以使用一般 C++ 代码中可用的大多数控制结构。因此，不再需要编写繁琐的模板代码或晦涩的单行代码，我们现在可以直接使用一个普通的 `for` 循环：

```cpp
constexpr bool isPrime(unsigned int p)
{
    for(unsigned int d = 2; d <= p / 2; ++d){
        if(p % d == 0)
            return false;
    }
    return p > 1;
}
```

对于 C++11 和 C++14 版本的 `constexpr isPrime()` 实现，我们可以简单地调用

```cpp
isPrime(9)
```

来判断 9 是否为质数。请注意，它可以在编译时进行计算，但不一定必须这样做。在**需要编译时的值上下文**中（例如，数组长度或非类型模板参数），编译器会尝试在编译时评估对 `constexpr` 函数的调用，如果不可能则会发出错误（因为最终必须产生一个常量）。在**其他上下文**中，编译器可能会在**编译时或运行时进行评估[^3]**，如果评估失败，则不会发出错误，而是将调用留作运行时调用。

例如：
```cpp
constexpr bool b1 = isPrime(9); // 在编译时评估
```
将会在编译时计算该值。同样的情况也适用于：

```cpp
const bool b2 = isPrime(9);     // 如果在命名空间作用域中，在编译时评估
```

前提是 `b2` 是**全局或在命名空间中定义**的。在块作用域中，编译器可以决定是在编译时还是在运行时[^4]进行计算。例如：

```cpp
bool fiftySevenIsPrime() {
    return isPrime(57);         // 在编译时或运行时评估
}
```
编译器可以选择是否在编译时评估对 `isPrime` 的调用。

另一方面：
```cpp
int x;
...
std::cout << isPrime(x); // 在运行时评估
```
将在运行时生成代码，以判断 `x` 是否为质数。

[^3]: 在撰写本书（2017年）时，编译器确实会尝试在不严格必要的情况下进行编译时评估。

[^4]: 理论上，即使使用了 `constexpr`，编译器仍可以选择在运行时计算 `b` 的初始值。编译器只需要检查它是否能够在编译时计算该值。

## 8.3 使用部分特化进行执行路径选择

一个有趣的编译时测试应用（如 `isPrime()`）是使用**部分特化**在编译时选择不同的实现。

例如，我们可以根据模板参数是否为素数在不同的实现之间进行选择：

```cpp
// 主辅助模板：
template<int SZ, bool = isPrime(SZ)>
struct Helper;

// 如果 SZ 不是素数的实现：
template<int SZ>
struct Helper<SZ, false>
{
    ...
};

// 如果 SZ 是素数的实现：
template<int SZ>
struct Helper<SZ, true>
{
    ...
};

template<typename T, std::size_t SZ>
long foo(std::array<T, SZ> const& coll)
{
    Helper<SZ> h; // 实现取决于数组大小是否为素数
    ...
}
```

在这里，根据 `std::array<>` 参数的大小是否为质数，我们使用了 `Helper<>` 类的两种不同实现。这种部分特化的应用，广泛适用于根据调用参数的属性在函数模板的不同实现之间进行选择。

在上面的示例中，我们使用了两个部分特化来实现两个可能的选项。我们也可以使用主模板作为一种默认情况的实现，并为其他特殊情况使用部分特化：

```cpp
// 主辅助模板（在没有匹配特化时使用）
template<int SZ, bool = isPrime(SZ)>
struct Helper
{
    ...
};

// 如果 SZ 是素数的特殊实现
template<int SZ>
struct Helper<SZ, true>
{
    ...
};
```

由于函数模板不支持部分特化，你必须使用其他机制来根据特定约束更改函数的实现。我们的选择包括以下几种方法：
- 使用带有静态函数的类，
- 使用 `std::enable_if`（详见第 6.3 节），
- 使用 SFINAE 特性（将在下一节介绍），或
- 使用 C++17 引入的编译时 `if` 特性（详见第 8.5 节）。

第 20 章讨论了基于约束选择函数实现的技术。

## 8.4 SFINAE(Substitution Failure Is Not An Error)

在 C++ 中，重载函数以处理不同类型的参数是相当常见的。当编译器看到对重载函数的调用时，它必须分别考虑每个候选函数，评估调用的参数，并选择最符合的候选函数（关于此过程的更多细节见附录 C）。

当候选函数集中包含**函数模板**时，编译器首先需要确定该候选函数应使用哪些模板参数，然后将这些参数替换到函数的参数列表和返回类型中，最后评估其匹配程度（与普通函数相同）。然而，替换过程中可能会遇到问题，可能生成无意义的构造。根据语言规则，这种替换失败不会导致错误，而是直接忽略这些候选函数。这个原则被称为 **SFINAE**（"substitution failure is not an error"，即替换失败不是错误）。

需要注意，这里的替换过程不同于按需实例化过程（见第 2.2 节，第 27 页）：替换可能会发生在那些不需要的潜在实例化上（编译器要评估它们是否真的不需要）。替换只发生在函数声明中的构造上（不包括函数体）。

考虑以下示例：

```cpp
// 数组中元素的数量：
template<typename T, unsigned N>
std::size_t len(T(&)[N]) {
    return N;
}

// 拥有 size_type 类型的类型的元素数量：
template<typename T>
typename T::size_type len(T const& t) {
    return t.size();
}
```

这里，我们定义了两个函数模板 `len()`，它们接受一个泛型参数[^5]：

1. 第一个函数模板声明参数为 `T(&)[N]`，这意味着参数必须是 N 个类型为 T 的元素组成的数组；   
2. 第二个函数模板声明参数为 `T`，这对参数没有任何限制，但它返回 `T::size_type` 类型，这要求传递的参数类型具有对应的 `size_type` 成员；

[^5]: 我们没有将这个函数命名为 `size()`，因为我们想避免与 C++ 标准库中的标准函数模板 `std::size()`（自 C++17 起定义）发生命名冲突。

当传递一个原生数组或字符串字面量时，只有针对原生数组的函数模板匹配：

```cpp
int a[10];
std::cout << len(a);        // OK：只有数组的 len() 匹配
std::cout << len("tmp");    // OK：只有数组的 len() 匹配
```

根据它的签名，第二个函数模板在替换 `T` 为 `int[10]` 和 `char const[4]` 时也会匹配，但这些替换会导致返回类型 `T::size_type` 出现潜在错误。因此，对于这些调用，第二个模板会被忽略。

当传递一个 `std::vector<>` 时，只有第二个函数模板匹配：

```cpp
std::vector<int> v;
std::cout << len(v);        // OK：只有带 size_type 的类型的 len() 匹配
```

当传递一个原生指针时，两个模板都不匹配（且不会导致失败）。因此，编译器会报告没有找到匹配的 `len()` 函数：

```cpp
int* p;
std::cout << len(p);        // 错误：未找到匹配的 len() 函数
```

注意，这与传递一个具有 `size_type` 成员，但没有 `size()` 成员函数的类型（例如 `std::allocator<>`）的情况不同：

```cpp
std::allocator<int> x;
std::cout << len(x);        // 错误：找到 len() 函数，但无法调用 size()
```

在这种情况下，编译器找到第二个函数模板作为匹配函数模板。因此，**不是没有找到匹配函数的错误**，而是因为 `std::allocator<int>` 无法调用 `size()` 导致**编译时错误**。此时，第二个函数模板不会被忽略。

当在替换返回类型时忽略候选函数可能导致编译器选择另一个参数匹配度更差的候选函数。例如：[len2.hpp](../../Codes/ch08/8_4/len2.hpp)

```cpp
#include <cstddef>

// 原始数组元素的实现
template<typename T, unsigned N>
std::size_t len(T(&)[N])
{
    return N;
}

// 对于有 size_type 类型的元素的实现
template<typename T>
typename T::size_type len(T const &t)
{
    return t.size();
}

// 任何类型的后备实现
std::size_t len(...)
{
    return 0;
}
```

我们还提供了一个通用的 `len()` 函数，它总是匹配，但匹配度最差（匹配省略号 `...`，参见附录 C.2）。

因此，对于原始数组和向量，具体的匹配是最佳匹配。而对于指针，只有回退匹配，这样编译器就不会再抱怨找不到匹配的 `len()` 调用了[^6]。但是对于 `allocator`，第二个和第三个函数模板匹配，第二个模板是最佳匹配。结果还是会出现调用 `size()` 的编译错误：

```cpp
int a[10];
std::cout << len(a);        // OK：数组的 len() 是最佳匹配
std::cout << len("tmp");    // OK：数组的 len() 是最佳匹配
std::vector<int> v;
std::cout << len(v);        // OK：带 size_type 的类型的 len() 是最佳匹配
int* p;
std::cout << len(p);        // OK：只有后备的 len() 匹配
std::allocator<int> x;
std::cout << len(x);        // 错误：第二个 len() 函数最佳匹配，但无法为 x 调用 size()
```

关于 SFINAE 的更多细节，请参见第 15.7 节；关于 SFINAE 的应用，请参见第 19.4 节。

[^6]: 实际上，这样的回退函数通常会提供一个更有用的默认值，抛出异常，或者包含一个静态断言，以生成有用的错误消息。

**SFINAE 和重载解析**

随着时间的推移，SFINAE 原则在模板设计中变得如此重要和普遍，以至于这个缩写已经成为一个动词。我们说“我们用 SFINAE 排除一个函数”时，是指应用 SFINAE 机制，通过修改模板代码，使其在某些约束下生成无效代码，从而确保模板函数在这些约束下被忽略。每当你在 C++ 标准中读到某个函数模板“不参与重载解析，除非...”时，这意味着使用了 SFINAE 来为某些情况“排除”该函数模板。

例如，类 `std::thread` 声明了一个构造函数：
```cpp
namespace std {
class thread {
public:
    ...
    template<typename F, typename... Args>
    explicit thread(F&& f, Args&&... args);
    ...
};
}
```
并附有如下说明：
备注：如果 `decay_t<F>` 与 `std::thread` 类型相同，则此构造函数不参与重载解析。

这意味着，如果使用 `std::thread` 作为第一个且唯一的参数调用此模板构造函数，则该模板构造函数会被忽略。原因是，否则像这样的成员模板有时可能会比任何预定义的复制或移动构造函数更匹配（详细内容见第 6.2 节和第 16.2.4 节）。通过在调用线程时 SFINAE 排除该构造函数模板，我们确保在用线程构造另一个线程时，总是使用预定义的复制或移动构造函数[^7]。

按个案应用这种技术可能会很繁琐。幸运的是，标准库提供了更容易**禁用模板的工具**。最著名的此类功能是 `std::enable_if<>`，它在第 6.3 节中引入。它允许我们通过用包含禁用条件的结构替换类型，来禁用一个模板。

因此，`std::thread` 的实际声明通常如下：

```cpp
namespace std {
class thread {
public:
    ...
    template<typename F, typename... Args,
    typename = std::enable_if_t<!std::is_same_v<std::decay_t<F>,
    thread>>>
    explicit thread(F&& f, Args&&... args);
    ...
};
}
```

有关如何使用部分特化和 SFINAE 实现 `std::enable_if<>` 的详细信息，请参见第 20.3 节。

[^7]: 由于 `thread` 类的复制构造函数被删除，这也确保了复制操作被禁止。

翻译如下：

### 8.4.1 使用 `decltype` 的表达式 SFINAE

找出并准确地表达出用于 SFINAE，排除某些条件下的函数模板并不总是容易的。

例如，假设我们想确保函数模板 `len()` 在处理具有 `size_type` 成员，没有 `size()` 成员函数的类型时被忽略。在没有任何形式的对 `size()` 成员函数的要求声明时，函数模板会被选中，最终导致实例化时发生错误：

```cpp
template<typename T>
typename T::size_type len (T const& t) {
    return t.size();
}
std::allocator<int> x;
std::cout << len(x) << '\n'; // 错误：len() 被选中，但 x 没有 size()
```

有一个通用的模式或习惯用法来处理这种情况：
- 使用尾随返回类型语法，指定返回类型（在函数声明中使用 `auto`，在返回类型前使用 `->`）；
- 使用 `decltype` 和逗号运算符定义返回类型；
- 在逗号运算符的开始部分，列出所有必须有效的表达式（若逗号运算符被重载，则将表达式转换为 `void`）；
- 在逗号运算符的末尾定义真正的返回类型的对象；

例如：

```cpp
template<typename T>
auto len (T const& t) -> decltype( (void)(t.size()), T::size_type() ) {
    return t.size();
}
```

这里的返回类型是：

```cpp
decltype( (void)(t.size()), T::size_type() )
```

`decltype` 的操作数是逗号分隔的表达式列表，因此最后一个表达式 `T::size_type()` 生成所需的返回类型（`decltype` 将其转换为返回类型）。在最后一个逗号之前，我们列出了必须有效的表达式，在这种情况下就是 `t.size()`。将表达式强制转换为 `void` 是为了避免用户为表达式的类型重载逗号运算符的可能性。

请注意，`decltype` 的参数是未求值的操作数，这意味着你可以创建“虚拟对象”而无需调用构造函数，这将在第 11.2.3 节讨论。

## 8.5 编译时 `if`

部分特化、SFINAE 和 `std::enable_if` 允许我们**整体启用或禁用模板**。C++17 还引入了一个编译时 `if` 语句，它允许我们基于**编译时条件启用或禁用特定语句**。使用 `if constexpr(...)` 语法，编译器通过编译时表达式来决定是否应用 `then` 部分或 `else` 部分（如果有）。

作为第一个示例，考虑第 4.1.1 节中引入的变参函数模板 `print()`。它使用递归打印其参数（任意类型）。不需要提供一个单独的函数来结束递归，`constexpr if` 特性允许我们局部决定是否继续递归：

```cpp
template<typename T, typename... Types>
void print (T const& firstArg, Types const&... args) {
    std::cout << firstArg << '\n';
    if constexpr(sizeof...(args) > 0) {
        print(args...); // 只有当 sizeof...(args) > 0 时才生成代码（自 C++17 起）
    }
}
```

在这里，如果 `print()` 只调用一个参数，`args` 变成一个空的参数包，因此 `sizeof...(args)` 变为 0。结果是，递归调用 `print()` 变成了被丢弃的语句，这段代码不会被实例化。因此，不需要存在相应的函数，递归就会终止。

代码未被实例化意味着只执行第一个翻译阶段（定义时），它会检查正确的语法和不依赖于模板参数的名称（见第 1.1.3 节）。例如：

```cpp
template<typename T>
void foo(T t) {
    if constexpr(std::is_integral_v<T>) {
        if (t > 0) {
            foo(t - 1);     // 没问题
        }
    }
    else {
        undeclared(t);      // 如果未声明且未丢弃（即 T 不是整型），则报错
        undeclared();       // 如果未声明，即使被丢弃，依然报错
        static_assert(false, "no integral");                    // 总是断言失败（即使被丢弃）
        static_assert(!std::is_integral_v<T>, "no integral");   // 没问题
    }
}
```

尽管代码写作 `if constexpr`，但这个特性被称为 `constexpr if`，因为它是 `if` 的“`constexpr`”形式（出于历史原因）。

请注意，`if constexpr` 可以在任何函数中使用，不仅限于模板。我们只需要一个**编译时表达式**，该表达式生成布尔值。例如：

```cpp
int main() {
    if constexpr(std::numeric_limits<char>::is_signed) {
        foo(42);        // 没问题
    }
    else {
        undeclared(42); // 如果 undeclared() 未声明则报错
        static_assert(false, "unsigned"); // 总是断言失败（即使被丢弃）
        static_assert(!std::numeric_limits<char>::is_signed, "char is unsigned"); // 没问题
    }
}
```

通过这个特性，我们可以使用第 8.2 节中介绍的 `isPrime()` 编译时函数，当给定大小不是素数时执行额外代码：

```cpp
template<typename T, std::size_t SZ>
void foo (std::array<T,SZ> const& coll) {
    if constexpr(!isPrime(SZ)) {
        ... // 如果传递的数组大小不是素数，则进行特殊的额外处理
    }
    ...
}
```

有关详细信息，请参见第 14.6 节。

## 8.6 总结

- 模板提供了在编译时进行计算的能力（使用递归进行迭代，使用部分特化或 `?:` 运算符进行选择）；
- 使用 `constexpr` 函数，我们可以用“普通函数”替换大多数编译时计算，这些函数可以在编译时上下文中调用；
- 通过部分特化，我们可以根据某些编译时约束，选择不同的类模板实现；
- 只有在需要时才使用模板，函数模板声明中的替换不会导致无效代码。这一原则称为 SFINAE（替换失败不是错误）；
- SFINAE 可用于为某些类型和/或约束提供函数模板；
- 自 C++17 起，编译时 `if` 允许我们根据编译时条件启用或丢弃语句（即使在模板外部）；