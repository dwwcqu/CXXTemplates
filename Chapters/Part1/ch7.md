# 7. 通过值还是引用？

自从 C++ 诞生以来，它就提供了按值传递和按引用传递两种方式，但选择哪种方式并不总是容易的：通常，对于非简单对象，按引用传递成本较低，但也更复杂。C++11 引入了移动语义，这意味着我们现在有不同的方式来**按引用传递参数**[1]：

1. `X const&`（常量左值引用）：参数引用传递的对象，但无法修改它。
2. `X&`（非常量左值引用）：参数引用传递的对象，可以修改它。
3. `X&&`（右值引用）：参数引用传递的对象，并支持移动语义，这意味着可以修改或“窃取”其值。

对于已知的具体类型，如何声明参数已经相当复杂。而在模板中，类型未知，因此决定使用哪种传递机制就更加困难了。

尽管如此，我们在[第 1.6.1 节](./ch1.md/#161-按值传递还是按引用传递)中建议，除非有充分理由（如以下情况），否则应在函数模板中按值传递参数：

- 无法复制[2]；
- 参数用于返回数据；
- 模板只是将参数转发到其他地方，同时保留原始参数的所有属性；
- 有显著的性能提升；

本章讨论了在模板中声明参数的不同方法，并阐述了推荐使用按值传递的一般理由，同时也提供了不这样做的理由。本章还探讨了在处理字符串字面量和其他原始数组时遇到的棘手问题。阅读本章时，熟悉值类别（左值、右值、纯右值、过期值等）的术语会有所帮助，这些术语在附录 B 中进行了说明。


## 7.1. 按值传递

当按值传递参数时，每个参数原则上都必须被复制。因此，每个参数都成为传递对象的副本。对于类，副本通常由复制构造函数初始化。

调用复制构造函数可能会很昂贵。然而，即使按值传递参数，也有多种方法可以避免昂贵的复制操作。事实上，编译器可能会优化掉复制操作，即使对于复杂的对象，使用移动语义也可以使复制变得廉价。

例如，来看一个简单的函数模板，它的参数是按值传递的：

```cpp
template<typename T>
void printV(T arg) {
    ...
}
```

当为整数调用此函数模板时，生成的代码为：

```cpp
void printV(int arg) {
    ...
}
```

参数 `arg` 成为传递参数的副本，无论是对象、字面值还是函数返回的值。

如果我们定义一个 `std::string` 并为其调用此函数模板：

```cpp
std::string s = "hi";
printV(s);
```

模板参数 `T` 实例化为 `std::string`，因此我们得到：

```cpp
void printV(std::string arg) {
    ...
}
```

再次说明，当传递字符串时，`arg` 成为 `s` 的副本。这次副本是通过字符串类的复制构造函数创建的，这可能是一个昂贵的操作，因为原则上此复制操作会创建一个完整或“深拷贝”，即副本在内部分配自己的内存以保存值[3]。

然而，并不总是会调用复制构造函数。考虑以下情况：

```cpp
std::string returnString();
std::string s = "hi";
printV(s); // 复制构造函数
printV(std::string("hi")); // 通常优化掉了复制（否则使用移动构造函数）
printV(returnString()); // 通常优化掉了复制（否则使用移动构造函数）
printV(std::move(s)); // 移动构造函数
```

在第一个调用中，我们传递了一个左值（lvalue），这意味着使用了复制构造函数。然而，在第二个和第三个调用中，当直接为 prvalue（临时对象，或由其他函数返回的值；详见附录 B）调用函数模板时，编译器通常会优化参数传递，使得根本不会调用复制构造函数。**注意，从 C++17 开始，此优化是强制要求的。在 C++17 之前，如果编译器没有优化掉复制操作，至少必须尝试使用移动语义，这通常使复制操作变得廉价。**在最后一个调用中，我们传递了一个 xvalue（通过 `std::move()` 转换的现有非常量对象），通过这种方式，我们强制调用移动构造函数，表示我们不再需要 `s` 的值。

因此，当参数按值传递时，调用 `printV()` 实现通常只在传递左值时代价较高（即我们之前创建并通常还会继续使用的对象，因为我们没有使用 `std::move()` 来传递它）。不幸的是，这是一个相当常见的情况。一个原因是我们通常会早些创建对象，然后在稍后（经过一些修改后）传递给其他函数。

**值传递时的类型衰退**

我们需要提到值传递的另一个特性：当通过值传递参数时，类型会发生衰退。这意味着原生数组会转换为指针，并且像 `const` 和 `volatile` 这样的限定符会被移除（这与使用值作为对象的初始化器时的行为类似，像使用 `auto` 声明的对象）[4]。例如：

```cpp
template<typename T>
void printV (T arg) {
...
}
```

例如：

```cpp
std::string const c = "hi";
printV(c);      // c 类型衰退，所以 arg 的类型为 std::string
printV("hi");   // 衰退为指针，因此 arg 的类型为 char const*
int arr[4];
printV(arr);    // 衰退为指针，因此 arg 的类型为 char const*
```

因此，当传递字符串字面量 `"hi"` 时，它的类型 `char const[3]` 会衰退为 `char const*`，因此 `T` 的推导类型是 `char const*`。模板实例化如下：

```cpp
void printV (char const* arg) {
...
}
```

这种行为源自 C 语言，并且有其优缺点。通常，它简化了对传递字符串字面量的处理，但缺点是在 `printV()` 内部，我们**无法区分传递的是指向单个元素的指针还是原生数组**。对此，我们将在第 7.4 节讨论如何处理字符串字面量和其他原生数组。

## 7.2 引用传递

现在让我们讨论引用传递的不同方式。在所有情况下，都不会创建副本（因为参数只是引用了传递的对象）。此外，**传递的参数不会发生类型衰退**。不过，有时传递可能会失败，而在可以传递的情况下，参数的类型可能会引发一些问题。

### 7.2.1 通过常量引用传递

为了避免不必要的复制，我们可以通过常量引用来传递非临时对象。例如：

```cpp
template<typename T>
void printR(T const& arg) {
    ...
}
```

通过这种声明，传递对象时不会创建副本，无论这个副本的创建开销如何：

```cpp
std::string returnString();
std::string s = "hi";
printR(s);                  // 不会复制
printR(std::string("hi"));  // 不会复制
printR(returnString());     // 不会复制
printR(std::move(s));       // 不会复制
```

甚至 `int` 也会通过引用传递，这有点反常，但影响不大：

```cpp
int i = 42;
printR(i);  // 传递引用而不是复制 i
```

这个调用会实例化 `printR()` 为：

```cpp
void printR(int const& arg) {
    ...
}
```

底层上，**引用传递通过传递参数的地址实现**。地址编码紧凑，因此在调用者和被调用者之间传递地址本身是高效的。然而，传递地址可能会给编译器带来不确定性：被调用者会对这个地址做什么？理论上，被调用者可以通过该地址改变所有“可达”的值。这意味着，编译器必须假设它缓存的所有值（通常存储在寄存器中）在调用后都无效。重新加载这些值可能代价高昂。你可能会想，既然我们通过常量引用传递，编译器难道不能推断出不会发生任何修改吗？**不幸的是，情况并非如此，因为调用者可以通过自己的非常量引用修改被引用的对象[5]**。

幸运的是，内联（inlining）可以减轻这个问题：如果编译器可以将调用展开为内联代码，它可以同时分析调用者和被调用者的代码，并且在很多情况下“看到”地址只用于传递底层值。模板函数通常很短，因此是内联展开的候选对象。然而，如果模板封装了更复杂的算法，内联的可能性就较低。

**引用传递不会衰退**

当通过引用传递参数时，参数不会发生衰退。这意味着原生数组不会转换为指针，像 `const` 和 `volatile` 这样的限定符也不会被移除。不过，由于调用参数被声明为 `T const&`，模板参数 `T` 本身不会被推导为 `const`。例如：

```cpp
template<typename T>
void printR(T const& arg) {
    ...
}
```

```cpp
std::string const c = "hi";
printR(c);      // T 被推导为 std::string，arg 的类型是 std::string const&
printR("hi");   // T 被推导为 char[3]，arg 的类型是 char const(&)[3]
int arr[4];
printR(arr);    // T 被推导为 int[4]，arg 的类型是 int const(&)[4]
```

因此，在 `printR()` 中声明为 `T` 类型的局部对象不是常量。

### 7.2.2 通过非常量引用传递

当你希望通过传递参数返回值时（即使用 out 或 inout 参数），你必须使用非常量引用（除非你更喜欢通过指针传递）。同样，这意味着传递参数时不会创建副本，被调用函数模板的参数直接访问传递的对象。

例如：

```cpp
template<typename T>
void outR(T& arg) {
    ...
}
```

注意，当传递临时对象（prvalue）或通过 `std::move()` 传递的对象（xvalue）时，通常是不允许的：

```cpp
std::string returnString();
std::string s = "hi";
outR(s);                    // OK：T 被推导为 std::string，arg 是 std::string&
outR(std::string("hi"));    // 错误：不允许传递临时值
outR(returnString());       // 错误：不允许传递临时值
outR(std::move(s));         // 错误：不允许传递 xvalue
```

你可以传递非常量类型的原生数组，它们同样不会衰退：

```cpp
int arr[4];
outR(arr);  // OK：T 被推导为 int[4]，arg 是 int(&)[4]
```

因此，你可以修改数组元素，并处理数组的大小。例如：

```cpp
template<typename T>
void outR(T& arg) {
    if (std::is_array<T>::value) {
        std::cout << "got array of " << std::extent<T>::value << " elems\n";
    }
    ...
}
```

不过，模板在这里有点棘手。如果你传递一个常量参数，推导结果可能会使 `arg` 成为常量引用的声明，这意味着在期望 lvalue 的地方突然允许传递 rvalue：

```cpp
std::string const c = "hi";
outR(c);                    // OK：T 被推导为 std::string const
outR(returnConstString());  // OK：如果 returnConstString() 返回常量字符串也是如此
outR(std::move(c));         // OK：T 被推导为 std::string const
outR("hi");                 // OK：T 被推导为 char const[3]
```

当然，在这种情况下，任何试图修改传递的参数的行为都是错误的。如果你想禁止将常量对象传递给非常量引用，可以采取以下措施：

1. 使用静态断言来触发编译时错误：

```cpp
template<typename T>
void outR(T& arg) {
    static_assert(!std::is_const<T>::value,
    "out parameter of foo<T>(T&) is const");
    ...
}
```

2. 通过使用 `std::enable_if<>` 禁用此情况：

```cpp
template<typename T, typename = std::enable_if_t<!std::is_const<T>::value>>
void outR(T& arg) {
    ...
}
```

3. 或者，使用概念（concepts），一旦它们被支持：

```cpp
template<typename T>
requires !std::is_const_v<T>
void outR(T& arg) {
    ...
}
```

### 7.2.3 使用转发引用传递

使用引用传递的一个原因是为了实现完美转发（详见[第 6.1 节](./ch6.md/#61-完美转发perfect-forwarding)）。但要记住，当使用转发引用时（定义为**模板参数的右值引用**），会有一些特殊规则。考虑以下示例：

```cpp
template<typename T>
void passR(T&& arg) {  // arg 被声明为转发引用
    ...
}
```

你可以将任何类型的对象传递给转发引用，并且与引用传递一样，不会创建副本：

```cpp
std::string s = "hi";
passR(s);                   // OK: T 被推导为 std::string&（arg 的类型也是如此）
passR(std::string("hi"));   // OK: T 被推导为 std::string，arg 的类型是 std::string&&
passR(returnString());      // OK: T 被推导为 std::string，arg 的类型是 std::string&&
passR(std::move(s));        // OK: T 被推导为 std::string，arg 的类型是 std::string&&
int arr[4];
passR(arr);                 // OK: T 被推导为 int(&)[4]（arg 的类型也是如此）
```

然而，类型推导的特殊规则可能会带来一些意外：

```cpp
std::string const c = "hi";
passR(c);       // OK: T 被推导为 std::string const&
passR("hi");    // OK: T 被推导为 char const(&)[3]（arg 的类型也是如此）
int arr[4];
passR(arr);     // OK: T 被推导为 int(&)[4]（arg 的类型也是如此）
```

在每种情况下，`passR()` 中的参数 `arg` 的类型都能够“识别”我们传递的是右值（用于移动语义）还是常量/非常量左值。这是唯一能够传递参数并区分这三种情况的方法。

这使得转发引用看起来几乎是完美的参数声明方式。但请注意，天下没有免费的午餐。

例如，这是模板参数 `T` 唯一可以隐式成为引用类型的情况。结果是，使用 `T` 来声明局部对象时，如果没有初始化，可能会产生错误：

```cpp
template<typename T>
void passR(T&& arg) {   // arg 是转发引用
    T x;                // 对于传递的左值，x 是引用，需要初始化
    ...
}
```

```cpp
foo(42);  // OK: T 被推导为 int
int i;
foo(i);  // 错误: T 被推导为 int&，使得在 passR() 中声明 x 无效
```

有关如何处理这种情况的更多细节，参见第 15.6.2 节。

## 7.3 使用 `std::ref()` 和 `std::cref()`

自 C++11 以来，调用者可以决定在函数模板参数中是通过值传递还是通过引用传递。如果模板声明为按值传递参数，调用者可以使用 `std::cref()` 和 `std::ref()`（声明在头文件 `<functional>` 中）来按引用传递参数。例如：

```cpp
template<typename T>
void printT(T arg) {
    ...
}

std::string s = "hello";
printT(s);                  // 通过值传递 s
printT(std::cref(s));       // 按引用传递 s
```

需要注意的是，`std::cref()` 并不会改变模板中参数的处理方式。它实际上使用了一种技巧：将传递的参数 `s` 包装为一个类引用对象。事实上，它创建了一个类型为 `std::reference_wrapper<>` 的对象，该对象引用了原始参数，并按值传递此对象。包装对象支持的主要操作是隐式类型转换回原始类型，从而返回原始对象[6]。因此，只要对传递的对象有有效的操作符，你就可以使用 `reference_wrapper`。例如：[cref.cpp](../../Codes/ch07/7_4/cref.cpp)

```cpp
#include <functional>
#include <string>
#include <iostream>

void printString(std::string const& s)
{
    std::cout << s << '\n';
}

template<typename T>
void printT(T arg)
{
    printString(arg);
}

int main()
{
    std::string s = "hello";
    printT(s);                  // 输出一个值传递
    printT(std::cref(s));       // 打印一个按 “引用传递” 的值
}
```

最后一次调用通过值传递了一个类型为 `std::reference_wrapper<string const>` 的对象给参数 `arg`，随后这个对象被传递并转换回其底层类型 `std::string`。

注意，**编译器必须知道需要进行隐式转换回原始类型**。因此，`std::ref()` 和 `std::cref()` 通常在**将对象通过泛型代码传递时效果良好**。例如，直接输出传递的泛型类型 `T` 的对象会失败，因为没有为 `std::reference_wrapper<>` 定义输出运算符：

```cpp
template<typename T>
void printV(T arg) {
    std::cout << arg << '\n';
}

std::string s = "hello";
printV(s);                  // OK
printV(std::cref(s));       // 错误：没有为 reference wrapper 定义 << 运算符
```

此外，以下代码也会失败，因为你不能将一个 `reference_wrapper` 与 `char const*` 或 `std::string` 进行比较：

```cpp
template<typename T1, typename T2>
bool isless(T1 arg1, T2 arg2) {
    return arg1 < arg2;
}

std::string s = "hello";
if (isless(std::cref(s), "world")) ...                  // 错误
if (isless(std::cref(s), std::string("world"))) ...     // 错误
```

即使将 `arg1` 和 `arg2` 设为相同的类型 `T` 也无济于事：

```cpp
template<typename T>
bool isless(T arg1, T arg2) {
    return arg1 < arg2;
}
```

因为编译器在推导 `arg1` 和 `arg2` 的 `T` 类型时会出现冲突。

因此，类 `std::reference_wrapper<>` 的作用是允许将引用作为“一等对象”使用，从而可以复制引用并按值传递给函数模板。你还可以在类中使用它，例如在容器中保存对对象的引用。但最终，你始终需要将其转换回原始类型。

## 7.4 处理字符串字面量和原始数组

到目前为止，我们已经看到在模板参数中使用字符串字面量和原始数组的不同效果：
- 按值传递会发生衰退（decay），即它们会变为指向元素类型的指针。
- 任何形式的按引用传递都不会发生衰退，参数仍然是引用，仍然指向数组。

两者各有利弊。数组衰退为指针时，你会失去区分处理指针和传递数组的能力。而在处理可能传递字符串字面量的参数时，不衰退可能会带来问题，因为不同长度的字符串字面量具有不同的类型。例如：

```cpp
template<typename T>
void foo (T const& arg1, T const& arg2)
{
    ...
}
foo("hi", "guy"); // ERROR
```

这里，`foo("hi", "guy")` 无法编译，因为 `"hi"` 的类型是 `char const[3]`，而 `"guy"` 的类型是 `char const[4]`，但模板要求它们具有相同的类型 `T`。只有当字符串字面量的长度相同时，这段代码才能编译。因此，**强烈建议在测试中使用不同长度的字符串字面量**。

通过按值传递参数，可以使这个调用变得可行：

```cpp
template<typename T>
void foo (T arg1, T arg2)
{
    ...
}
foo("hi", "guy"); // compiles, but ...
```

但这并不意味着所有问题都解决了。更糟糕的是，编译时问题可能变为运行时问题。考虑下面的代码，我们使用 `operator==` 比较传递的参数：

```cpp
template<typename T>
void foo (T arg1, T arg2)
{
    if (arg1 == arg2) { // OOPS: compares addresses of passed arrays
        ...
    }
}
foo("hi", "guy"); // compiles, but ...
```

如上所示，你必须知道应将传递的字符指针解释为字符串。但这可能是必然的，因为模板也要处理从已经衰退的字符串字面量中传递的参数（例如，通过按值传递或赋值给使用 `auto` 声明的对象）。

尽管如此，在许多情况下，衰退是有帮助的，特别是在检查两个对象是否具有相同类型或是否可以转换为相同类型时。一个典型的使用场景是完美转发（perfect forwarding）。但是，如果你想使用完美转发，你必须将参数声明为转发引用。在这些情况下，你可以使用 `std::decay<>` 这个类型特性显式衰退参数。有关具体示例，请参见 7.6 节中的 `std::make_pair()` 故事。

需要注意的是，其他类型特性有时也会隐式衰退，例如 `std::common_type<>`，它会返回两个参数类型的公共类型（见 1.3.3 节和 D.5 节）。

### 7.4.1 字符串字面量和原始数组的特殊实现

你可能需要根据传递的是指针还是数组来区分实现。当然，这要求传递的数组还没有衰退。

为了区分这些情况，你必须检测是否传递了数组。基本上有两种选择：
- 你可以声明模板参数，使它们仅对数组有效：
  
```cpp
template<typename T, std::size_t L1, std::size_t L2>
void foo(T (&arg1)[L1], T (&arg2)[L2])
{
    T* pa = arg1; // decay arg1
    T* pb = arg2; // decay arg2
    if (compareArrays(pa, L1, pb, L2)) {
        ...
    }
}
```

这里，`arg1` 和 `arg2` 必须是相同元素类型 `T` 的原始数组，但大小不同。不过，注意你可能需要多种实现来支持不同形式的原始数组（见[第 5.4 节](./ch5.md/#54-用于原始数组和字符串字面值的模板)）。

- 你可以使用类型特性来检测是否传递了数组（或指针）：

```cpp
template<typename T,
         typename = std::enable_if_t<std::is_array_v<T>>>
void foo (T&& arg1, T&& arg2)
{
    ...
}
```

由于这些特殊处理，通常处理数组的最佳方法是使用不同的函数名。当然，最好的方法是确保模板的调用者使用 `std::vector` 或 `std::array`。但只要字符串字面量是原始数组，我们就必须考虑到它们。

## 7.5 处理返回值

对于返回值，你也可以决定是按值返回还是按引用返回。然而，按引用返回可能会带来潜在的问题，因为你可能引用了某个超出你控制范围的对象。

有几个常见的编程实践场景下会按引用返回：
- 返回容器或字符串的元素（例如通过 `operator[]` 或 `front()`）
- 授予类成员的写权限；
- 返回对象以支持链式调用（例如，流的 `operator<<` 和 `operator>>` 以及类对象的 `operator=`）；

此外，通常会通过返回 `const` 引用来授予成员的读权限。

注意，如果使用不当，所有这些情况都可能导致问题。例如：

```cpp
std::string* s = new std::string("whatever");
auto& c = (*s)[0];
delete s;
std::cout << c; // 运行时错误
```

这里，我们获得了字符串元素的引用，但在使用该引用时，底层的字符串已经不存在了（即，我们创建了一个悬空引用），这会导致未定义行为。这个例子可能显得牵强（有经验的程序员可能很快就会发现问题），但问题容易变得不明显。例如：

```cpp
auto s = std::make_shared<std::string>("whatever");
auto& c = (*s)[0];
s.reset();
std::cout << c; // 运行时错误
```

因此，我们应确保函数模板通过值返回结果。然而，正如本章讨论的那样，使用模板参数 `T` 并不能保证它不是引用类型，因为有时 `T` 可能会隐式地被推导为引用类型：

```cpp
template<typename T>
T retR(T&& p) // p 是转发引用
{
    return T{...}; // OOPS: 当传递左值时，按引用返回
}
```

即使 `T` 是从按值调用推导出来的模板参数，当显式指定模板参数为引用时，它可能会变为引用类型：

```cpp
template<typename T>
T retV(T p) // 注意: T 可能成为引用
{
    return T{...}; // OOPS: 如果 T 是引用类型，按引用返回
}
int x;
retV<int&>(x); // retT() 实例化时，T 被推导为 int&
```

为了安全起见，你有两个选择：
- 使用类型特性 `std::remove_reference<>`（见 D.4 节）将类型 `T` 转换为非引用类型：

```cpp
template<typename T>
typename std::remove_reference<T>::type retV(T p)
{
    return T{...}; // 始终按值返回
}
```

其他类型特性，如 `std::decay<>`（见 D.4 节），在此也很有用，因为它们也会**隐式移除引用**。

- 让编译器通过声明返回类型为 `auto` 来推导返回类型（自 C++14 起；见 1.3.2 节），因为 `auto` 始终会衰退：

```cpp
template<typename T>
auto retV(T p)      // 编译器推导出的按值返回类型
{
    return T{...};  // 始终按值返回
}
```

## 7.6 推荐的模板参数声明

正如我们在前几节中学到的那样，声明依赖于模板参数的参数有多种方式：
- **声明按值传递参数**：这种方式简单，能使字符串字面量和原始数组衰退（decay），但对于较大的对象来说性能不是最优。调用者仍然可以使用 `std::cref()` 和 `std::ref()` 来按引用传递参数，但必须谨慎，确保这样做是有效的。

- **声明按引用传递参数**：
  这种方式通常为较大的对象提供更好的性能，尤其是在以下情况下：
  - 将现有对象（左值）传递给左值引用，
  - 将临时对象（纯右值）或标记为可移动的对象（亡值）传递给右值引用，
  - 或将两者传递给转发引用（forwarding references）。

由于在这些情况下参数不会衰退，传递字符串字面量和其他原始数组时需要特别注意。对于转发引用，也要注意模板参数会被推导为引用类型。

### 7.6.1 通用建议
根据这些选择，我们对函数模板的推荐如下：

1. 默认情况下，声明参数为按值传递。这种方式简单，通常即使对于字符串字面量也有效。对于小的参数以及临时对象或可移动对象，性能很好。调用者在传递现有的大对象（左值）时，有时可以使用 `std::ref()` 和 `std::cref()` 来避免昂贵的复制。

2. 如果有充分的理由，可以采取其他方式：
   - 如果需要输出参数或输入/输出参数，且需要返回新对象或允许调用者修改参数，则将参数声明为非 `const` 引用（除非你更喜欢通过指针传递）。然而，可以考虑禁止意外地接受 `const` 对象，详见 [7.2.2 节](./ch7.md/#722-通过非常量引用传递)。
   - 如果模板需要转发参数，使用完美转发。也就是说，声明参数为转发引用，并在适当的地方使用 `std::forward<>()`。考虑使用 `std::decay<>` 或 `std::common_type<>` 来统一字符串字面量和原始数组的不同类型。
   - 如果性能是关键，并且预计复制参数会很昂贵，使用 `const` 引用。当然，如果你无论如何都需要本地副本，这不适用。

3. 如果你有更好的做法，不必遵循这些建议。然而，不要凭直觉对性能做假设，即使是专家也会犯错。相反，应进行性能测试。

### 7.6.2 不要过度泛化
注意，在实践中，函数模板通常不会适用于任意类型的参数。实际上，通常有一些约束。例如，你可能知道只会传递某种类型的向量。在这种情况下，不要声明过于泛化的函数，因为正如前面讨论的那样，可能会出现意想不到的副作用。相反，可以使用如下声明：

```cpp
template<typename T>
void printVector (std::vector<T> const& v)
{
    ...
}
```

在 `printVector()` 中对参数 `v` 的声明可以确保传递的 `T` 不会成为引用类型，因为**向量不能使用引用作为元素类型**。同时，显而易见的是，按值传递向量几乎总是代价高昂的，因为 `std::vector<>` 的复制构造函数会复制元素。因此，将向量参数声明为按值传递几乎永远没有用。如果我们仅仅将参数 `v` 声明为类型 `T`，那么在按值传递和按引用传递之间做出决策就不那么明确了。

### 7.6.3 `std::make_pair()` 示例
`std::make_pair<>()` 是一个很好的例子，说明了如何选择参数传递机制的陷阱。它是 C++ 标准库中的一个便捷函数模板，用于通过类型推导创建 `std::pair<>` 对象。它的声明在 C++ 标准的不同版本中发生了变化：

- 在第一个 C++ 标准 C++98 中，`make_pair<>()` 在 `namespace std` 中声明为按引用传递，以避免不必要的复制：

```cpp
template<typename T1, typename T2>
pair<T1,T2> make_pair (T1 const& a, T2 const& b)
{
    return pair<T1,T2>(a,b);
}
```

然而，这在使用不同大小的字符串字面量或原始数组的 `pair` 时几乎立即导致了重大问题。

- 于是，在 C++03 中，函数定义被更改为按值传递：

```cpp
template<typename T1, typename T2>
pair<T1,T2> make_pair (T1 a, T2 b)
{
    return pair<T1,T2>(a,b);
}
```

在该问题的解决方案中，解释道：“这似乎比其他两个建议对标准的更改要小得多，并且任何效率方面的担忧都被该解决方案的优势所抵消。”

- 然而，随着 C++11 的到来，`make_pair()` 必须支持移动语义，因此参数必须变为转发引用。为此，定义大致如下更改：

```cpp
template<typename T1, typename T2>
constexpr pair<typename decay<T1>::type, typename decay<T2>::type>
make_pair (T1&& a, T2&& b)
{
    return pair<typename decay<T1>::type,
                typename decay<T2>::type>(forward<T1>(a),
                                          forward<T2>(b));
}
```

完整的实现甚至更复杂：为了支持 `std::ref()` 和 `std::cref()`，函数还将 `std::reference_wrapper` 的实例解包为真实的引用。

C++ 标准库现在在许多地方完美转发传递的参数，通常结合使用 `std::decay<>`。

## 7.7 总结
- 在测试模板时，使用不同长度的字符串字面量；
- 按值传递的模板参数会衰退，而按引用传递的不会衰退；
- 类型特性 `std::decay<>` 可以使按引用传递的参数衰退；
- 在函数模板声明按值传递参数时，`std::cref()` 和 `std::ref()` 可以在需要时按引用传递参数；
- 按值传递模板参数很简单，但可能不会带来最佳性能；
- 除非有充分的理由，否则在函数模板中按值传递参数；
- 通常应确保返回值按值传递（这可能意味着模板参数不能直接作为返回类型）；
- 在性能至关重要时，务必进行性能测试。不要依赖直觉；直觉可能是错误的；


[1]: 常量右值引用（`X const&&`）也是可能的，但目前它没有确立的语义意义。

[2]: 请注意，从 C++17 开始，即使没有可用的复制或移动构造函数，也可以通过值传递临时实体（右值）。因此，从 C++17 开始，额外的限制是左值无法复制。

[3]: 注：`string` 类的实现本身可能会有一些优化以减少拷贝的开销。其中一个是小字符串优化（SSO），它在对象内部直接使用一些内存来保存值，避免分配新内存，只要值不太长。另一个是写时拷贝优化（copy-on-write），只要源和拷贝都没有被修改，拷贝会使用与源相同的内存。然而，写时拷贝优化在多线程代码中有显著的缺点，因此自 C++11 起，标准字符串不允许使用此优化。

[4]: 术语“衰退”（decay）来源于 C 语言，并且也适用于函数到函数指针的类型转换（参见第 11.1.1 节）。

[5]: 使用 `const_cast` 是另一种更为显式的方法，用于修改被引用的常量对象。

[6]: 你也可以在 `reference_wrapper` 上调用 `get()`，并将其用作函数对象。