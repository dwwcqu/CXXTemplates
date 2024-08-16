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