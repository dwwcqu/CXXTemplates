# 6. 移动语义和 `enable_if<>`

C++11 引入的一个最显著的特性是**移动语义**。你可以利用它，通过将内部资源从源对象“移动”（或“窃取”）到目标对象来**优化复制和赋值操作**，而不是复制这些内容。**前提是源对象不再需要它的内部值或状态（因为它即将被丢弃）**。移动语义对模板设计产生了显著影响，并且引入了一些**特殊规则**来支持**泛型代码中的移动语义**。本章将介绍这些特性。

## 6.1. 完美转发(Perfect Forwarding)

假设你想编写能够传递传入参数基本属性的泛型代码：
- 可修改的对象应被转发，以便它们仍然可以被修改。
- 常量对象应被转发为只读对象。
- 可移动对象（我们可以“窃取”的对象，因为它们即将失效）应被转发为可移动对象。

如果不使用模板，要实现这些功能，我们必须针对这三种情况分别编写代码。例如，要将 `f()` 的调用转发到相应的函数 `g()`：[move1.cpp](../../Codes/ch06/6_1/move1.cpp)

```cpp
#include <utility>
#include <iostream>

class X
{
};

void g(X &)
{
    std::cout << "g() for variable\n";
}

void g(X const &)
{
    std::cout << "g() for constant\n";
}

void g(X &&)
{
    std::cout << "g() for movable object\n";
}

// 让 f() 转发参数 val 给 g()
void f(X &val)
{
    g(val); // val 是非常量左值 => 调用 g(X&)
}

void f(X const &val)
{
    g(val); // val 是常量左值 => 调用 g(X const&)
}

void f(X &&val)
{
    g(std::move(val)); // val 是非常量左值 => 需要 std::move() 去调用 g(X&&)
}

int main()
{
    X v;             // 创建变量
    X const c;       // 创建常量
    f(v);            // 对于非常量对象，f(X&) => g(X&)
    f(c);            // 对于常量对象，f(X const&) => g(X const&)
    f(X());          // 对于临时对象 f(X&&) => g(X&&)
    f(std::move(v)); // 对于可移动变量 f(X&&) => g(X&&)
}
```

以下展示了 `f()` 的三种不同实现方式，它们将其参数转发到 `g()`：

```cpp
void f (X& val) {
    g(val); // val 是非常量左值 => 调用 g(X&)
}

void f (X const& val) {
    g(val); // val 是常量左值 => 调用 g(X const&)
}

void f (X&& val) {
    g(std::move(val)); // val 是非常量左值 => 需要使用 std::move() 调用 g(X&&)
}
```

注意，用于**可移动对象**的代码（通过**右值引用**）与其他代码有所不同：它需要 `std::move()`，因为根据语言规则，**移动语义不会自动传递[^1]**。虽然第三个 `f()` 中的 `val` 被声明为右值引用，但当它作为表达式使用时，其值类别是非常量左值（参见附录 B），行为类似于第一个 `f()` 中的 `val`。如果没有 `std::move()`，非常量左值的 `g(X&)` 将被调用，而不是 `g(&&)`。

如果我们想在泛型代码中结合所有三种情况，会遇到一个问题：

```cpp
template<typename T>
void f (T val)
{
    g(T);
}
```

这种写法适用于前两种情况，但不适用于传递可移动对象的第三种情况。

为此，C++11 引入了**完美转发参数的特殊规则**。实现这一功能的惯用代码模式如下：

```cpp
template<typename T>
void f (T&& val)
{
    g(std::forward<T>(val)); // 完美转发 val 给 g()
}
```

注意，`std::move()` 没有模板参数，并“触发”传递参数的**移动语义**，而 `std::forward<>()` 根据传递的模板参数“转发”**可能的移动语义**。

不要假设模板参数 `T` 的 `T&&` 与特定类型 `X` 的 `X&&` 行为相同。**不同的规则适用！**然而，从语法上看它们是相同的：
- 对于**特定类型 `X`**，`X&&` 声明参数为**右值引用**。它**只能绑定到可移动对象**（例如**临时对象的纯右值**和通过 `std::move()` 传递的对象的 `x` 值；详情见附录 B）。它始终是可变的，你总是可以“窃取”它的值[^2]；
- 对于**模板参数 `T`**，`T&&` 声明为**转发引用**（也称为**通用引用**）[^3]。它可以绑定到**可变的**、**不可变的**（即 `const`）或**可移动的对象**。在函数定义内部，参数可以是可变的、不可变的，或者引用你可以“窃取”内部值的对象；

[^1]: 移动语义不会自动传递，这一点是有意且重要的。如果不是这样，我们会在第一次使用可移动对象时丢失其值。

[^2]: 像 `X const&&` 这样的类型是有效的，但在实践中没有常见的语义，因为“窃取”可移动对象的内部表示需要修改该对象。不过，它可能用于强制仅传递临时对象或标记为 `std::move()` 的对象，而不必修改它们。

[^3]: “通用引用”这一术语由 Scott Meyers 提出，作为一个常见的术语，可以表示“左值引用”或“右值引用”。因为“通用”过于宽泛，C++17 标准引入了转发引用一词，因为使用这种引用的主要原因是转发对象。然而，请注意，它并不会自动转发。该术语描述的不是它是什么，而是它通常用于做什么。

请注意，**`T` 必须是真正的模板参数名称**。仅仅依赖模板参数是不够的。对于模板参数 `T`，如 `typename T::iterator&&` 这样的声明只是一个右值引用，而不是转发引用。

因此，用于完美转发参数的完整程序如下：[move2.cpp](../../Codes/ch06/6_1/move2.cpp)

```cpp
#include <utility>
#include <iostream>

class X
{
};

void g(X &)
{
    std::cout << "g() for variable\n";
}

void g(X const &)
{
    std::cout << "g() for constant\n";
}

void g(X &&)
{
    std::cout << "g() for movable object\n";
}

// 让 f() 进行 val 参数的完美转发到 g()
template <typename T>
void f(T &&val)
{
    g(std::forward<T>(val)); // 对于任何传递的参数 val，调用正确的 g()
}

int main()
{
    X v;             // 创建变量
    X const c;       // 创建常量
    f(v);            // 对于非常量对象，f(X&) => g(X&)
    f(c);            // 对于常量对象，f(X const&) => g(X const&)
    f(X());          // 对于临时对象 f(X&&) => g(X&&)
    f(std::move(v)); // 对于可移动变量 f(X&&) => g(X&&)
}
```

当然，完美转发也可以与可变参数模板一起使用（参见[第 4.3 节](./ch4.md/#43-可变参数模板的应用)）。有关完美转发的详细信息，参见第 15.6.3 节。

## 6.2. 特殊成员函数模板

成员函数模板也可以用作特殊成员函数，包括构造函数。然而，这可能会导致意外的行为。考虑以下示例：[specialmemtmp1.cpp](../../Codes/ch06/6_2/specialmemtmp1.cpp)

```cpp
#include <utility>
#include <string>
#include <iostream>

class Person
{
private:
    std::string name;

public:
    // 传递初始值的构造函数(1)
    explicit Person(std::string const &n) : name(n)
    {
        std::cout << "copying string-CONSTR for '" << name << "'\n";
    }
    // (2)
    explicit Person(std::string &&n) : name(std::move(n))
    {
        std::cout << "moving string-CONSTR for '" << name << "'\n";
    }
    // 拷贝和移动构造(3)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // (4)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE_CONSTR Person '" << name << "'\n";
    }
};

int main()
{
    std::string s = "sname";
    Person p1(s);             // 用字符串对象初始化 => (1)
    Person p2("tmp");         // 用字符串字面值初始化 => (2)
    Person p3(p1);            // 拷贝构造 => (3)
    Person p4(std::move(p1)); // 移动构造 => (4)
}
```

我们有一个包含 `name` 字符串成员的 `Person` 类，并为其提供了初始化构造函数。为了支持移动语义，我们重载了接受 `std::string` 的构造函数：

- 对于调用者仍然需要的字符串对象，我们提供了一个版本，`name` 通过传递的参数的副本进行初始化：

```cpp
Person(std::string const& n) : name(n) 
{
    std::cout << "copying string-CONSTR for '" << name << "'\n";
}
```

- 对于可移动的字符串对象，我们调用 `std::move()` 来“窃取”值：

```cpp
Person(std::string&& n) : name(std::move(n)) {
    std::cout << "moving string-CONSTR for '" << name << "'\n";
}
```

正如预期的那样，第一个构造函数在传递**仍在使用的字符串对象（左值）**时被调用，而第二个构造函数在**传递可移动对象（右值）**时被调用：

```cpp
std::string s = "sname";
Person p1(s);       // 用字符串对象初始化 => 调用 (1)
Person p2("tmp");   // 用字符串字面值初始化 => 调用 (2)
```

除了这些构造函数，示例还为**复制和移动构造函数**提供了具体实现，以查看何时整个 `Person` 对象被复制/移动：

```cpp
Person p3(p1);              // 复制 Person => 调用 (3)
Person p4(std::move(p1));   // 移动 Person => 调用 (4)
```

现在，让我们用一个泛型构造函数替换这两个字符串构造函数，该构造函数将传递的参数完美转发给成员 `name`：[specialmemtmp2.hpp](../../Codes/ch06/6_2/specialmemtmp2.hpp)

```cpp
#include <utility>
#include <string>
#include <iostream>

class Person
{
private:
    std::string name;

public:
    // 传递初始值的一般构造函数(1)
    template<typename STR>
    explicit Person(STR&& n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }
    // 拷贝和移动构造(2)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // (3)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE_CONSTR Person '" << name << "'\n";
    }
};
```

传递字符串的构造行为如预期的那样正常工作：

```cpp
std::string s = "sname";
Person p1(s);               // 用字符串对象初始化 => (1)
Person p2("tmp");           // 用字符串字面值初始化 => (2)
```

注意，在 `p2` 的构造中，并没有创建临时字符串对象：参数 `STR` 被推导为 `char const[4]` 类型。对构造函数的指针参数应用 `std::forward<STR>` 并没有多大效果，因此 `name` 成员是从一个以 `null` 结尾的字符串构造的。

但是，当我们尝试调用复制构造函数时，会出现错误：

```cpp
Person p3(p1);              // 编译时会报错：首先会匹配 (1)
```

而使用可移动对象初始化一个新的 `Person` 对象仍然可以正常工作：

```cpp
Person p4(std::move(p1));   // OK: 移动 Person => 调用 (3)
```

注意，复制常量 `Person` 对象也可以正常工作：

```cpp
Person const p2c("ctmp");   // 用字符串字面值初始化常量对象
Person p3c(p2c);            // OK: 复制常量 Person => 调用 COPY-CONSTR
```

问题在于，根据 C++ 的**重载解析规则**（见第 16.2.4 节），对于**非常量左值** `Person p`，成员模板：

```cpp
template<typename STR>
Person(STR&& n)
```

比（通常预定义的）复制构造函数**更匹配**：

```cpp
Person(Person const& p)
```

`STR` 只是被替换为 `Person&`，而对于**复制构造函数**，则需要转换为 `const`。

你可能会考虑通过提供一个非常量复制构造函数来解决这个问题：

```cpp
Person(Person& p)
```

然而，这只是一个部分解决方案，因为对于派生类的对象，成员模板仍然是更好的匹配。**你真正需要的是在传递的参数是 `Person` 或可以转换为 `Person` 的表达式时禁用成员模板。**这可以通过使用 `std::enable_if<>` 来实现，在下一节中将介绍这一点。

## 6.3. 使用 `enable_if<>` 关闭模板

自 C++11 起，C++ 标准库提供了一个辅助模板 `std::enable_if<>`，用于在某些编译时条件下，**忽略函数模板**。

例如，如果函数模板 `foo<>` 定义如下：

```cpp
template<typename T>
typename std::enable_if<(sizeof(T) > 4)>::type
foo() {
}
```

那么当 `sizeof(T) > 4` 结果为 `false` 时，这个 `foo<>` 的定义会被**忽略**[^4]。如果 `sizeof(T) > 4` 结果为 `true`，那么这个函数模板实例会扩展为：

```cpp
void foo() {
}
```

也就是说，`std::enable_if<>` 是一个**类型特性**，它会对传入的**编译时表达式**（作为第一个模板参数）进行评估，并有以下行为：

- 如果表达式结果为 `true`，则它的类型成员 `type` 会产生一个类型：
  - 如果没有传入第二个模板参数，类型为 `void`；
  - 否则，类型为第二个模板参数的类型；
- 如果表达式结果为 `false`，则成员 `type` 不会被定义。由于模板具有一种名为 `SFINAE`（替换失败不是错误）的特性（将在第 8.4 节中进一步介绍），因此这会导致带有 `enable_if` 表达式的**函数模板被忽略**。

对于所有自 C++14 起产生类型的类型特性，有一个相应的别名模板 `std::enable_if_t<>`，可以省略 `typename` 和 `::type`（详见[第 2.8 节](./ch2.md/#28-类型别名)）。因此，自 C++14 起你可以这样写：

```cpp
template<typename T>
std::enable_if_t<(sizeof(T) > 4)>
foo() {
}
```

如果传递了第二个参数给 `enable_if<>` 或 `enable_if_t<>`：

```cpp
template<typename T>
std::enable_if_t<(sizeof(T) > 4), T>
foo() {
return T();
}
```

如果表达式结果为 `true`，那么 `enable_if` 结构将扩展为第二个参数。因此，如果 `MyType` 是传入或推导为 `T` 的具体类型，并且其大小大于 4，那么效果就是：

```cpp
MyType foo();
```

[^4]: 不要忘记将条件放入括号中，否则条件中的 `>` 会结束模板参数列表。

注意，将 `enable_if` 表达式放在声明中间是相当笨拙的。出于这个原因，使用 `std::enable_if<>` 的常见方式是使用**带有默认值的附加函数模板参数**：

```cpp
template<typename T,
typename = std::enable_if_t<(sizeof(T) > 4)>>
void foo() {
}
```

如果 `sizeof(T) > 4`，那么这将扩展为：

```cpp
template<typename T,
typename = void>
void foo() {
}
```

如果你觉得这样仍然过于笨拙，并且想让需求/约束更明确，可以使用别名模板定义自己的名称：

```cpp
template<typename T>
using EnableIfSizeGreater4 = std::enable_if_t<(sizeof(T) > 4)>;

template<typename T,
typename = EnableIfSizeGreater4<T>>
void foo() {
}
```

有关 `std::enable_if` 如何实现的讨论，详见第 20.3 节。

## 6.4. 使用 `enable_if<>`

我们可以使用 `enable_if<>` 来解决[第 6.2 节](#62-特殊成员函数模板)中引入的构造函数模板问题。

我们要解决的问题是，当传递的参数 `STR` 具有正确的类型（即 `std::string` 或可以转换为 `std::string` 的类型）时，禁用模板构造函数的声明：

```cpp
template<typename STR>
Person(STR&& n);
```

为此，我们使用了另一个标准类型特性 `std::is_convertible<FROM, TO>`。在 C++17 中，对应的声明如下：

```cpp
template<typename STR,
typename = std::enable_if_t<
std::is_convertible_v<STR, std::string>>>
Person(STR&& n);
```

如果类型 `STR` 可以转换为 `std::string` 类型，则整个声明扩展为：

```cpp
template<typename STR,
typename = void>
Person(STR&& n);
```

如果类型 `STR` 不能转换为 `std::string`，则整个函数模板会被忽略[^5]。

同样，我们可以通过使用**别名模板**为这个约束定义自己的名称：

```cpp
template<typename T>
using EnableIfString = std::enable_if_t<
std::is_convertible_v<T, std::string>>;
...
template<typename STR, typename = EnableIfString<STR>>
Person(STR&& n);
```

因此，整个 `Person` 类应如下所示：[specialmemtmpl.hpp](../../Codes/ch06/6_4/specialmemtmpl.hpp)

```cpp
#include <utility>
#include <string>
#include <iostream>
#include <type_traits>

template <typename T>
using EnableIfString = std::enable_if_t<std::is_convertible_v<T, std::string>>;

class Person
{
private:
    std::string name;

public:
    // 传递初始值的一般构造 (1)
    template <typename STR, typename = EnableIfString<STR>>
    explicit Person(STR &&n) : name(std::forward<STR>(n))
    {
        std::cout << "TMPL-CONSTR for '" << name << "'\n";
    }
    // 拷贝构造 (2)
    Person(Person const &p) : name(p.name)
    {
        std::cout << "COPY-CONSTR Person '" << name << "'\n";
    }
    // 移动构造 (3)
    Person(Person &&p) : name(std::move(p.name))
    {
        std::cout << "MOVE-CONSTR Person '" << name << "'\n";
    }
};
```

[^5]: 如果你想知道为什么我们不检查 `STR` 是否“不可转换为 `Person`”，请注意：我们正在定义一个可能允许将字符串转换为 `Person` 的函数。因此，构造函数需要知道它是否启用，这取决于它是否可转换，而这又取决于它是否启用，如此反复。切勿在影响 `enable_if` 所用条件的地方使用 `enable_if`，这是一种逻辑错误，编译器不一定能检测到。

现在，所有调用都按预期进行：[specialmemtmpl.cpp](../../Codes/ch06/6_4/specialmemtmpl.cpp)

```cpp
#include "specialmemtmpl.hpp"

int main()
{
    std::string s = "sname";
    Person p1(s);               // 字符串对象初始化 => (1)
    Person p2("tmp");           // 字符串字面值初始化 => (1)
    Person p3(p1);              // OK => 拷贝构造 (2)
    Person p4(std::move(p1));   // OK => 移动构造 (3)
}
```

请注意，在 C++14 中，我们必须如下声明别名模板，因为 `_v` 版本对于生成值的类型特性未定义：

```cpp
template<typename T>
using EnableIfString = std::enable_if_t<
std::is_convertible<T, std::string>::value>;
```

而在 C++11 中，我们必须如下声明特殊成员模板，因为对于生成类型的类型特性，`_t` 版本未定义：

```cpp
template<typename T>
using EnableIfString
= typename std::enable_if<std::is_convertible<T, std::string>::value>::type;
```

但这些都已隐藏在 `EnableIfString<>` 的定义中。

还要注意，使用 `std::is_convertible<>` 有一个替代方法，因为它要求类型能够隐式转换。通过使用 `std::is_constructible<>`，我们还允许使用显式转换进行初始化。然而，在这种情况下，参数的顺序是相反的：

```cpp
template<typename T>
using EnableIfString = std::enable_if_t<
std::is_constructible_v<std::string, T>>;
```

有关 `std::is_constructible<>` 的详细信息，请参见第 D.3.2 节，有关 `std::is_convertible<>` 的详细信息，请参见第 D.3.3 节。有关在可变参数模板上应用 `enable_if<>` 的详细信息和示例，请参见第 D.6 节。

**关闭特殊成员函数**

请注意，通常情况下我们无法使用 `enable_if<>` 来禁用预定义的复制/移动构造函数和/或赋值运算符。原因是**成员函数模板从不被视为特殊成员函数**，当需要复制构造函数时，它们会被忽略。例如，以下声明：

```cpp
class C {
public:
    template<typename T>
    C(T const&) {
        std::cout << "模板复制构造函数\n";
    }
    ...
};
```

当请求复制一个 `C` 时，仍然会使用预定义的复制构造函数：

```cpp
C x;
C y{x}; // 仍然使用预定义的复制构造函数（而不是成员模板）
```

（实际上无法使用成员模板，因为没有办法指定或推导其模板参数 `T`。）

删除预定义的复制构造函数并不是解决方案，因为这样会导致试图复制 `C` 时产生错误。

然而，有一个巧妙的解决方法：我们可以为 `const volatile` 参数声明一个复制构造函数并将其标记为“已删除”（即使用 `= delete` 定义）。这样可以防止隐式声明其他复制构造函数。在此基础上，我们可以定义一个构造函数模板，该模板将优先于（已删除的）复制构造函数用于非易失性类型：

```cpp
class C {
public:
    ...
    // 将预定义的复制构造函数定义为已删除
    // （使用转换为 volatile 以启用更好的匹配）
    C(C const volatile&) = delete;

    // 实现具有更好匹配的复制构造函数模板：
    template<typename T>
    C(T const&) {
        std::cout << "模板复制构造函数\n";
    }
    ...
};
```

现在，即使对于“普通”复制操作，也会使用模板构造函数：

```cpp
C x;
C y{x}; // 使用成员模板
```

在这样的模板构造函数中，我们可以应用额外的约束条件，例如使用 `enable_if<>`。例如，为了防止当模板参数是整数类型时能够复制类模板 `C<>` 的对象，我们可以实现如下代码：

```cpp
template<typename T>
class C {
public:
    ...
    // 将预定义的复制构造函数定义为已删除
    // （使用转换为 volatile 以启用更好的匹配）
    C(C const volatile&) = delete;

    // 如果 T 不是整数类型，提供具有更好匹配的复制构造函数模板：
    template<typename U,
    typename = std::enable_if_t<!std::is_integral<U>::value>>
    C(C<U> const&) {
        ...
    }
    ...
};
```

## 6.5 使用 `Concepts` 去简化 `enable_if<>` 表达式

即使使用别名模板，`enable_if` 的语法也相当笨拙，因为它使用了一种变通方法：为了达到预期效果，我们添加了一个额外的模板参数，并“滥用”该参数来提供函数模板可用的特定要求。这样的代码难以阅读，并且使得函数模板的其余部分难以理解。

原则上，我们只需要一个**语言特性**，允许我们以一种方式来**表达函数的要求或约束**，**如果这些要求/约束不满足，该函数将被忽略**。

这正是人们期待已久的语言特性“Concepts”的一个应用，它允许我们使用简单的语法为模板制定要求/条件。不幸的是，尽管已经讨论了很长时间，但概念特性仍未成为 C++17 标准的一部分。然而，一些编译器提供了对此特性的实验性支持，而且概念特性很可能会成为 C++17 之后的下一个标准的一部分。

按照提议的使用方式，使用 `Concepts` 特性时，我们只需编写如下代码：

```cpp
template<typename STR>
requires std::is_convertible_v<STR,std::string>
Person(STR&& n) : name(std::forward<STR>(n)) {
...
}
```

我们甚至可以将这一要求定义为一个通用的概念：

```cpp
template<typename T>
concept ConvertibleToString = std::is_convertible_v<T,std::string>;
```

并将这一概念作为一个要求来表达：

```cpp
template<typename STR>
requires ConvertibleToString<STR>
Person(STR&& n) : name(std::forward<STR>(n)) {
...
}
```

这也可以表述为：

```cpp
template<ConvertibleToString STR>
Person(STR&& n) : name(std::forward<STR>(n)) {
...
}
```

有关 C++ 中概念特性的详细讨论，请参见附录 E。

## 6.6. 总结

- 在模板中，你可以通过将参数声明为转发引用（通过使用模板参数名加 `&&` 的形式声明类型）并在转发调用中使用 `std::forward<>()` 来“完美地”转发参数；
- **当使用完美转发的成员函数模板时，它们可能比预定义的特殊成员函数更匹配，以便复制或移动对象**；
- 使用 `std::enable_if<>`，你可以在编译时条件为 `false` 时禁用函数模板（在确定该条件后，该模板将被忽略）；
- 通过使用 `std::enable_if<>`，你**可以避免在构造函数模板或赋值运算符模板在处理单一参数时，比隐式生成的特殊成员函数更匹配时出现的问题**；
- 你可以通过删除 `const volatile` 的预定义特殊成员函数来**将特殊成员函数模板化**（并应用 `enable_if<>`）；
- `Concepts` 特性将允许我们为函数模板的要求，使用更直观的语法；