# 4. 可变参数模板(Variadic Templates)

自 C++11 以来，模板可以接受可变数量的模板参数。这一特性允许在需要传递任意数量和类型的参数的情况下使用模板。一个典型的应用是通过一个类或框架传递任意数量和类型的参数。另一个应用是提供通用代码来处理任意数量和类型的参数。

## 4.1 可变参数模板

**模板参数可以被定义为接受无限数量的模板参数。具有这种能力的模板被称为可变参数模板。**

### 4.1.1 通过示例了解可变参数模板

例如，你可以使用以下代码为不同类型的可变数量参数调用 `print()`：[varprint1.hpp](../../Codes/ch04/4_1/varprint1.hpp)

```cpp
#include <iostream>

void print() 
{
}

template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    std::cout << firstArg << '\n'; // 打印第一个参数
    print(args...); // 对剩余的参数递归调用 print()
}
```

如果传递了一个或多个参数，则会使用函数模板。通过单独指定第一个参数，这个模板允许在递归调用 `print()` 处理剩余参数之前，先打印第一个参数。这些名为 `args` 的剩余参数构成了一个函数参数包：

```cpp
void print (T firstArg, Types... args)
```

而这些不同的“类型”由模板参数包指定：

```cpp
template<typename T, typename... Types>
```

**为了结束递归，需要提供一个非模板重载的 `print()`，当参数包为空时调用该重载。**

例如，调用如下代码：

```cpp
std::string s("world");
print(7.5, "hello", s);
```

会输出以下内容：

```
7.5
hello
world
```

原因是这个调用首先展开为：

```cpp
print<double, char const*, std::string>(7.5, "hello", s);
```

其中：

- `firstArg` 的值为 `7.5`，因此类型 `T` 是 `double`。
- `args` 是一个可变参数模板，包含值 `"hello"` 和 `"world"`，分别为类型 `char const*` 和 `std::string`。

在打印 `firstArg` 的值 `7.5` 后，再次调用 `print()` 处理剩余的参数，接着展开为：

```cpp
print<char const*, std::string>("hello", s);
```

其中：

- `firstArg` 的值为 `"hello"`，因此类型 `T` 是 `char const*`。
- `args` 是一个可变参数模板，包含值 `s`，类型为 `std::string`。

在打印 `"hello"` 后，再次调用 `print()` 处理剩余的参数，接着展开为：

```cpp
print<std::string>(s);
```

其中：

- `firstArg` 的值为 `world`，因此类型 `T` 是 `std::string`。
- `args` 是一个空的可变参数模板，没有值。

因此，在打印 `"world"` 后，再次调用 `print()` 没有参数，这会调用非模板的 `print()` 重载，而它什么也不做。

### 4.1.2 重载可变参数模板与非可变参数模板

注意，你也可以通过以下方式实现上面的示例：[varprint2.hpp](../../Codes/ch04/4_1/varprint2.hpp)

```cpp
#include <iostream>

template<typename T>
void print(T arg)
{
    std::cout << arg << '\n'; // 打印传递的参数
}

template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    print(firstArg); // 调用 print() 处理第一个参数
    print(args...); // 调用 print() 处理剩余的参数
}
```

也就是说，如果两个函数模板的唯一区别是尾部的参数包，则优先选择没有尾部参数包的函数模板。更多关于此处适用的重载决策规则，请参见第 C.3.1 节。

### 4.1.3 操作符 `sizeof...`

C++11 还引入了用于可变参数模板的新形式 `sizeof...`，它扩展为参数包包含的元素数量。因此：

```cpp
template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    std::cout << sizeof...(Types) << '\n';  // 打印剩余类型的数量
    std::cout << sizeof...(args) << '\n';   // 打印剩余参数的数量
    ...
}
```

该代码会两次打印传递给 `print()` 函数后剩余的参数数量。可以看到，`sizeof...` 可以用于模板参数包和函数参数包。

这可能让我们认为可以通过不调用 `print()` 来跳过递归的终止函数，前提是没有更多的参数：

```cpp
template<typename T, typename... Types>
void print(T firstArg, Types... args)
{
    std::cout << firstArg << '\n';
    if (sizeof...(args) > 0) { // 错误：当 sizeof...(args) == 0 时出错
        print(args...); // 且没有声明 print() 处理无参数的情况
    }
}
```

然而，这种方法不起作用，因为通常在函数模板中，所有 `if` 语句的分支都会被实例化。代码是否有用是运行时决定的，而调用的实例化是编译时决定的。因此，如果你调用带有一个（最后一个）参数的 `print()` 函数模板，那么即使没有参数，`print(args...)` 语句仍然会被实例化，并且如果没有为无参数的情况提供函数 `print()`，就会出现错误。

但需要注意的是，自 C++17 以来，可以使用编译时 `if`，它可以实现我们预期的功能，只是语法略有不同。这将在第 8.5 节进行讨论。

## 4.2. 折叠表达式

从 C++17 开始，使用二元操作符，在一个参数包上的所有参数上，可以计算其二元操作结果。

例如，下面的函数返回所有穿都参数的求和值：

```cpp
template<typename... T>
auto foldSum(T... s)
{
    return (... + s); // ((s1 + s2) + s3)...
}
```

如果**参数包为空**，则该表达式通常是格式错误的（有一个例外，对于操作符 `&&`，值为 `true`；对于操作符 `||`，值为 `false`；对于逗号操作符，空参数包的值为 `void()`）。表 4.1 列出了可能的折叠表达式。

<div align="center">
表 4.1 折叠表达式(C++17开始支持)
</div>

|折叠表达式|计算|
|:--:|:--:|
|`(... op pack)`|`(((pack1 op pack2) op pack3) ...op packN)`|
|`(pack op ...)`|`(pack1 op( ...(packN-1 op packN)))`|
|`(init op ... op pack)`|`(((init op pack1) op pack2) ...op packN)`|
|`(pack op ... op init)`|`(pack1 op(...(packN op init)))`|

你几乎可以对所有二元运算符使用折叠表达式（详见第 12.4.6 节）。例如，你可以使用折叠表达式通过操作符 `->*` 来遍历二叉树中的路径:[foldtraverse.cpp](../../Codes/ch04/4_2/foldtraverse.cpp)

```cpp
struct Node
{
    int value;
    Node* left;
    Node* right;
    Node(int i = 0) : value(i), left(nullptr), right(nullptr)
    {}
    ...
};

template<typename T, typename... TP>
Node* traverse(T np, TP... paths)
{
    return np->* ... ->* paths;
}

int main()
{
    // 初始化二叉树结构
    Node* root = new Node(0);
    root->left = new Node(1);
    root->left->right = new Node(2);
    ...

    // 遍历二叉树
    Node* node = traverse(root, left, right);

    ...
}
```

这里，`(np ->* ... ->* paths)` 使用了折叠表达式来遍历从 `np` 开始的 `paths` 可变参数元素。

通过使用带初始化器的折叠表达式，我们可以考虑简化上面介绍的可变参数模板来打印所有参数：

```cpp
template<typename... Types>
void print (Types const&... args)
{
    (std::cout << ... << args) << '\n';
}
```

但是，注意在这种情况下，没有空格将所有参数包中的元素分隔开。为了实现这个，你需要一个额外的类模板，以确保任何参数的输出都扩展为带有空格的形式：[addspace.cpp](../../Codes/ch04/4_2/addspace.cpp):

```cpp
template<typename T>
class AddSpace
{
private:
    T const& ref;       // 参数引用
public:
    AddSpace(T const& r) : ref(r)
    {}
    friend std::ostream& operator<< (std::ostream& os, AddSpace<T> s)
    {
        // 输出传递的参数和一个空格
        return os << s.ref << ' ';
    }
};

template<typename... Args>
void print(Args... args)
{
    ( std::cout << ... << AddSpace(args) ) << '\n';
}
```

注意，表达式 `AddSpace(args)` 使用了类模板参数推导（详见[第 2.9 节](../Part1/ch2.md/#29-类模板参数推导)），其效果相当于 `AddSpace<Args>(args)`，为每个参数创建一个 `AddSpace` 对象，该对象引用传递的参数，并在输出表达式中使用时添加空格。

关于折叠表达式的详细信息，请参阅第 12.4.6 节。

## 4.3. 可变参数模板的应用

可变参数模板在实现泛型库（如 C++ 标准库）时发挥着重要作用。一个典型的应用是**转发任意类型的可变数量参数**。例如，我们在以下情况下使用这一特性：

- 将参数传递给由共享指针拥有的新堆对象的构造函数：

```cpp
// 创建一个指向 complex<float> 的共享指针，并以 4.2 和 7.7 进行初始化：
auto sp = std::make_shared<std::complex<float>>(4.2, 7.7);
```

- 将参数传递给由库启动的线程：

```cpp
std::thread t(foo, 42, "hello"); // 在单独的线程中调用 foo(42, "hello")
```

- 将参数传递给插入到向量中的新元素的构造函数：

```cpp
std::vector<Customer> v;
...
v.emplace("Tim", "Jovi", 1962); // 插入一个以三个参数初始化的 Customer 对象
```

通常，参数会通过移动语义“完美转发”（详见第 6.1 节），因此相应的声明例如如下：

```cpp
namespace std {
  template<typename T, typename... Args>
  shared_ptr<T> make_shared(Args&&... args);

  class thread {
  public:
    template<typename F, typename... Args>
    explicit thread(F&& f, Args&&... args);
    ...
  };

  template<typename T, typename Allocator = allocator<T>>
  class vector {
  public:
    template<typename... Args>
    reference emplace_back(Args&&... args);
    ...
  };
}
```

还要注意，对于**可变参数函数模板参数**，适用与普通参数相同的规则。例如，如果按值传递，参数会被复制并发生退化（例如，数组会变为指针）；而如果按引用传递，参数会引用原始参数且不会退化：

```cpp
// args 是被复制的且类型已退化的副本：
template<typename... Args>
void foo(Args... args);

// args 是对传递对象的未退化的引用：
template<typename... Args>
void bar(Args const&... args);
```

## 4.4. 可变类模板参数及可变表达式

除了上述示例外，参数包还可以出现在其他地方，例如**表达式**、**类模板**、`using` 声明，甚至是**推导指引**。第 12.4.2 节提供了完整列表。

### 4.4.1. 可变表达式

你可以做的不仅仅是转发所有参数，还可以对它们进行计算，这意味着可以对参数包中的所有参数进行计算。例如，以下函数将参数包 `args` 中的每个参数加倍，并将加倍后的参数传递给 `print()`：

```cpp
template<typename... T>
void printDoubled (T const&... args)
{
    print(args + args...);
}
```

例如，如果你调用 `printDoubled(7.5, std::string("hello"), std::complex<float>(4,2));`，该函数将产生以下效果（不包括任何构造函数的副作用）：

```cpp
print(7.5 + 7.5,
      std::string("hello") + std::string("hello"),
      std::complex<float>(4,2) + std::complex<float>(4,2));
```

如果你只想给每个参数加 1，请注意省略号 `...` 不能直接跟在数值字面量之后：

```cpp
template<typename... T>
void addOne (T const&... args)
{
    print(args + 1...);  // 错误：1... 是一个小数点过多的字面量
    print(args + 1 ...); // 正确
    print((args + 1)...); // 正确
}
```

编译时表达式可以以相同方式包含模板参数包。例如，以下函数模板返回所有参数类型是否相同：

```cpp
template<typename T1, typename... TN>
constexpr bool isHomogeneous (T1, TN...)
{
    return (std::is_same<T1, TN>::value && ...); // C++17 以来支持
}
```

这是折叠表达式的一个应用（见[第 4.2 节](#42-折叠表达式)）：对于 `isHomogeneous(43, -1, "hello")`，返回值的表达式展开为：

```cpp
std::is_same<int, int>::value && std::is_same<int, char const*>::value
```

结果为 `false`，而 `isHomogeneous("hello", " ", "world", "!")` 返回 `true`，因为所有传递的参数都被推导为 `char const*`（请注意，由于调用参数是按值传递的，因此参数类型会退化）。

### 4.4.2. 可变索引

作为另一个例子，以下函数使用可变索引列表来访问传递的第一个参数的相应元素：

```cpp
template<typename C, typename... Idx>
void printElems (C const& coll, Idx... idx)
{
    print(coll[idx]...);
}
```

也就是说，当调用 `std::vector<std::string> coll = {"good", "times", "say", "bye"}; printElems(coll, 2, 0, 3);` 时，会调用 `print(coll[2], coll[0], coll[3]);`。

你也可以声明非类型模板参数为参数包。例如：

```cpp
template<std::size_t... Idx, typename C>
void printIdx (C const& coll)
{
    print(coll[Idx]...);
}
```

允许你调用 `std::vector<std::string> coll = {"good", "times", "say", "bye"}; printIdx<2, 0, 3>(coll);`，其效果与前面的示例相同。

### 4.4.3. 可变类模板

可变模板也可以是类模板。一个重要的例子是一个类，其中**任意数量的模板参数指定相应成员的类型**：

```cpp
template<typename... Elements>
class Tuple;

Tuple<int, std::string, char> t; // t可以容纳整数、字符串和字符
```

这将在第 25 章讨论。

另一个例子是能够指定对象可能具有的类型：

```cpp
template<typename... Types>
class Variant;

Variant<int, std::string, char> v; // v 可以容纳整数、字符串或字符
```

这将在第 26 章讨论。

你还可以定义一个类，其类型表示一个索引列表：

```cpp
// 表示任意数量索引的类型：
template<std::size_t...>
struct Indices {
};
```

这可以用来定义一个函数，使用给定索引通过 `get<>()` 在编译时访问 `std::array` 或 `std::tuple` 的元素，并调用 `print()`：

```cpp
template<typename T, std::size_t... Idx>
void printByIdx(T t, Indices<Idx...>)
{
    print(std::get<Idx>(t)...);
}
```

这个模板可以这样使用：

```cpp
std::array<std::string, 5> arr = {"Hello", "my", "new", "!", "World"};
printByIdx(arr, Indices<0, 4, 3>());
```

或者这样使用：

```cpp
auto t = std::make_tuple(12, "monkeys", 2.0);
printByIdx(t, Indices<0, 1, 2>());
```

这是元编程的第一步，将在第 8.1 节和第 23 章中讨论。

### 4.4.4. 可变推导指引

甚至推导指引（见[第 2.9 节](./ch2.md/#29-类模板参数推导)）也可以是可变的。例如，C++ 标准库为 `std::array` 定义了以下推导指引：

```cpp
namespace std {
    template<typename T, typename... U>
    array(T, U...)
    -> array<enable_if_t<(is_same_v<T, U> && ...), T>, (1 + sizeof...(U))>;
}
```

像 `std::array a{42, 45, 77};` 这样的初始化在推导指引中推导出元素类型 `T`，以及后续元素的各种 `U...` 类型。因此，总元素数为 `1 + sizeof...(U)`：

```cpp
std::array<int, 3> a{42, 45, 77};
```

第一个数组参数的 `std::enable_if<>` 表达式是一个折叠表达式（如[第 4.4.1 节](#441-可变表达式)所介绍的 `isHomogeneous()`）扩展为：

```cpp
is_same_v<T, U1> && is_same_v<T, U2> && is_same_v<T, U3> ...
```

如果结果不为 `true`（即并非所有元素类型相同），则推导指引被丢弃，整个推导失败。通过这种方式，标准库确保所有元素必须具有相同类型，推导指引才会成功。

### 4.4.5. 可变基类与 `using`

最后，考虑以下示例：[varusing.cpp](../../Codes/ch04/4_2/varusing.cpp)

```cpp
#include <string>
#include <unordered_set>

class Customer
{
private:
    std::string name;
public:
    Customer(std::string const& n) : name(n){}
    std::string getName() const { return name;}
};

struct CustomerEq
{
    bool operator()(Customer const& c1, Customer const& c2) const
    {
        return c1.getName() == c2.getName();
    }
};

struct CustomerHash
{
    std::size_t operator()(Customer const& c) const
    {
        return std::hash<std::string>()(c.getName());
    }
};

// 定义类：对可变基类的 operator() 进行组合
template<typename... Bases>
struct Overloader : Bases...
{
    using Bases::operator()...;     // 自 C++ 17 开始 OK
};

int main()
{
    using CustomerOP = Overloader<CustomerHash, CustomerEq>;
    std::unordered_set<Customer, CustomerHash, CustomerEq> coll1;
    std::unordered_set<Customer, CustomerOP, CustomerOP> coll2;
}
```

我们首先定义了一个 `Customer` 类和独立的函数对象，用于对 `Customer` 对象进行哈希和比较。使用

```cpp
template<typename... Bases>
struct Overloader : Bases...
{
    using Bases::operator()...; // C++17 以来支持
};
```

我们可以定义一个从可变数量基类派生的类，并引入这些基类中的 `operator()` 声明。使用

```cpp
using CustomerOP = Overloader<CustomerHash, CustomerEq>;
```

我们可以从 `CustomerHash` 和 `CustomerEq` 派生 `CustomerOP`，并在**派生类中启用这两个 `operator()` 的实现**。

有关此技术的另一个应用，请参见第 26.4 节。

### 4.5 总结

- 通过使用参数包，可以为**任意数量**和**任意类型**的**模板参数**定义模板；
- 为了处理这些参数，你需要递归和/或匹配的**非可变函数**；
- `sizeof...` 运算符返回为参数包提供的参数数量；
- 可变模板的一个典型应用是**转发任意数量**和**任意类型**的参数；
- 通过使用**折叠表达式**，可以将**运算符**应用于**参数包的所有参数**；