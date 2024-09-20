# 9. 模板使用实践

模板代码与普通代码有些不同。在某些方面，模板介于宏和普通（非模板）声明之间。虽然这种描述可能过于简化，但它不仅影响我们使用模板编写算法和数据结构的方式，也影响到我们在日常编写和分析涉及模板的程序时的逻辑处理方式。

在本章中，我们讨论了一些与之相关的实际问题，但不一定深入探讨其背后的技术细节。很多这些细节会在第 14 章中进行探索。为了简化讨论，我们假设我们的 C++ 编译系统由传统的编译器和链接器组成（不属于这一类的C++系统很少见）。

## 9.1 包含模型(Inclusion Model)

有几种方式可以组织模板源码。本节介绍最常见的方式：包含模型。

### 9.1.1 链接器错误

大多数 C 和 C++ 程序员通常将非模板代码组织如下：
- 类和其他类型完全放在头文件中。通常，这是一个带有`.hpp`（或.H、.h、.hh、.hxx）扩展名的文件；
- 对于全局（非内联）变量和（非内联）函数，仅在头文件中放置声明，而定义则放在一个作为独立翻译单元编译的文件中。这样的 CPP 文件通常具有`.cpp`（或.C、.c、.cc、.cxx）扩展名；

这种做法通常很好：它使所需的类型定义在整个程序中都可以轻松获取，并避免了链接器的变量和函数重复定义错误。

考虑到这些惯例，初学者在使用模板时常遇到的一个常见错误，可以通过以下错误的程序来说明。像往常一样，我们在头文件中声明模板：[myfirst.hpp](../../Codes/ch09/9_1/myfirst.hpp)

```cpp
#ifndef MYFIRST_HPP
#define MYFIRST_HPP
// 模板声明
template<typename T>
void printTypeof(T const&);
#endif // MYFIRST_HPP
```

`printTypeof()`是一个简单的辅助函数声明，用于打印一些类型信息。函数的实现被放置在 CPP 文件中：[myfirst.cpp](../../Codes/ch09/9_1/myfirst.cpp)

```cpp
#include <iostream>
#include <typeinfo>
#include "myfirst.hpp"
// 模板的实现/定义
template<typename T>
void printTypeof(T const& x) {
    std::cout << typeid(x).name() << '\n';
}
```

该示例使用 `typeid` 运算符打印描述传递给它的表达式类型的字符串。它返回的是表达式的静态类型 `std::type_info` 的左值，该类型提供了一个成员函数`name()`，该函数显示某些表达式的类型。C++ 标准并没有明确规定 `name()` 必须返回有意义的内容，但在优良的 C++ 实现中，您应能得到对传递给 `typeid` 的表达式类型的良好描述[1]。

最后，我们在另一个 CPP 文件中使用该模板，模板声明通过 `#include` 引入：[myfirstmain.cpp](../../Codes/ch09/9_1/myfirstmain.cpp)

```cpp
#include "myfirst.hpp"
// 使用模板
int main() 
{
    double ice = 3.0;
    printTypeof(ice); // 对double类型调用函数模板
}
```

C++ 编译器很可能会接受此程序，但链接器可能会报告错误，暗示没有 `printTypeof()` 函数的定义。

此错误的原因是函数模板 `printTypeof()` 的定义未被实例化。要**实例化模板**，**编译器必须知道应实例化哪个定义以及应为哪些模板参数进行实例化**。不幸的是，在前面的示例中，这两个信息分别位于不同的文件中，且分别被单独编译。因此，当编译器看到调用`printTypeof()`时，尽管它没有看到相应的定义去实例化该函数模板，它还是会假设定义会在其他地方提供，并创建一个引用（供链接器解析）。然而，当编译器处理 `myfirst.cpp` 文件时，它**此时没有任何迹象表明，必须为特定的参数，实例化包含的模板定义**。

### 9.1.2 头文件中的模板

解决上述问题的常见方法是像处理宏或内联函数一样：**在声明模板的头文件中包含模板的定义**。

也就是说，我们不再提供`myfirst.cpp`文件，而是将`myfirst.hpp`改写为同时包含模板声明和模板定义：

```cpp
#ifndef MYFIRST_HPP
#define MYFIRST_HPP
#include <iostream>
#include <typeinfo>

// 模板的声明

template<typename T>
void printTypeof(T const&);

// 模板声明与定义
template<typename T>
void printTypeof(T const& x) {
    std::cout << typeid(x).name() << '\n';
}
#endif // MYFIRST_HPP
```

这种组织模板的方式称为包含模型。使用这种方式，我们的程序现在应能正确编译、链接和运行。

我们可以在此指出一些观察到的问题。最显著的一点是，这种方法大大增加了包含头文件 `myfirst.hpp` 的成本。在这个示例中，成本不是模板定义本身的大小，而是我们还必须包含模板定义中使用的头文件——在本例中是 `<iostream>` 和 `<typeinfo>`。您会发现，这可能意味着数万行代码，因为像 `<iostream>` 这样的头文件本身也包含了许多模板定义。

这在实践中是一个实际问题，因为它大大增加了编译器编译大型程序所需的时间。因此，我们将探讨一些可能解决该问题的方法，包括预编译头文件（参见第9.3节）和显式模板实例化的使用（参见第14.5节）。

尽管有这个构建时间的问题，我们仍然**建议在可能的情况下使用这种包含模型来组织模板**，直到有更好的机制出现。在撰写本书的 2017 年，正在开发一种这样的机制：模块（参见第17.11节）。模块是一种语言机制，允许程序员更合逻辑地组织代码，使得编译器可以分别编译所有声明，然后高效地选择性导入已处理的声明。

关于包含方法，还有一个更微妙的观察点是：非内联函数模板与内联函数和宏有一个重要区别：它们不会在调用点展开。相反，当它们被实例化时，会创建函数的新副本。由于这是一个自动化的过程，编译器可能会在不同的文件中创建两个副本，某些链接器在发现同一函数的两个不同定义时可能会报错。从理论上讲，这不应是我们关心的问题：这是 C++ 编译系统需要解决的问题。在实践中，大多数情况下事情进展顺利，我们不需要处理这个问题。然而，对于创建自己代码库的大型项目，偶尔会出现问题。第 14 章关于实例化方案的讨论以及对 C++编译系统（编译器）随附文档的仔细研究应有助于解决这些问题。

最后，我们需要指出的是，适用于示例中的普通函数模板的内容，同样适用于类模板的成员函数和静态数据成员，以及成员函数模板。

## 9.2 模板与 `inline`

声明函数为内联函数是提高程序运行时间的常用工具。`inline` 说明符旨在提示编译器，优先在调用点直接替换函数体，而不是使用通常的函数调用机制。

然而，编译器可以忽略这个提示。因此，`inline` 的唯一保证效果是允许函数定义在程序中出现多次（通常是因为它出现在多个地方被包含的头文件中）。像内联函数一样，函数模板也可以在多个翻译单元中定义。这通常通过将定义放在一个由多个 CPP 文件包含的头文件中实现。

但这**并不意味着函数模板默认使用内联替换**。是否以及何时在调用点内联替换函数模板体完全取决于编译器。有趣的是，编译器通常比程序员更擅长判断内联是否能带来性能提升。因此，编译器的内联策略因编译器而异，甚至依赖于特定编译的选项。

尽管如此，借助合适的性能监控工具，程序员可能会比编译器拥有更多信息，因此可能希望覆盖编译器的决策（例如，在为特定平台优化软件时，如手机，或针对特定输入）。有时只能通过编译器特定的属性（如 `noinline` 或 `always_inline`）来实现这一点。

值得注意的是，函数模板的完全特化在这方面像普通函数一样：它们的定义只能出现一次，除非它们被定义为内联函数（参见第16.3节）。有关该主题的更广泛、详细的概述，请参阅附录 A。

## 9.3 预编译头文件

即使没有模板，C++ 头文件也可能非常大，因此需要很长时间来编译。模板加剧了这一趋势，程序员等待编译的抱怨，常常促使厂商实施一种通常被称为预编译头文件（PCH）的方案。这个方案在标准之外运作，依赖于厂商特定的选项。尽管我们将如何创建和使用预编译头文件的细节留给各个 C++ 编译系统的文档，但理解其工作原理是有用的。

当编译器翻译文件时，它从文件的开头开始处理直到结束。它处理文件中的每个标记（可能来自 `#include` 文件），并调整其内部状态，如向符号表添加条目以供以后查找。在此过程中，编译器还可能在目标文件中生成代码。

预编译头文件方案利用了代码可以以某种方式组织，使得许多文件以相同的代码行开始这一事实。假设要编译的每个文件都以相同的 N 行代码开始。我们可以编译这些 N 行代码，并在此时保存编译器的完整状态到一个预编译头文件中。然后，对于程序中的每个文件，我们可以重新加载保存的状态并从第 N+1 行开始编译。值得注意的是，重新加载保存的状态比实际编译前 N 行的操作快得多。然而，首次保存状态的成本通常比仅仅编译 N 行更昂贵，增加的成本大约从 20% 到 200% 不等。

有效使用预编译头文件的关键是确保文件尽可能多地以相同的代码行开始。实际上，这意味着文件必须以相同的 `#include` 指令开始，而正如之前提到的，这些指令占用了我们构建时间的一大部分。因此，**注意头文件的包含顺序是非常有利的**。例如，以下两个文件：

```cpp
#include <iostream>
#include <vector>
#include <list>
...
```

和

```cpp
#include <list>
#include <vector>
...
```

会阻碍预编译头文件的使用，因为**源文件中没有共同的初始状态**。

一些程序员认为，`#include` 一些额外的不必要的头文件比错失使用预编译头文件加速文件编译的机会更好。这个决定可以大大简化包含策略的管理。例如，通常比较容易创建一个名为 `std.hpp` 的头文件，包含所有标准头文件[2]：

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <list>
...
```

然后，可以预编译此文件，并且每个使用标准库的程序文件可以简单地以如下方式开始：

```cpp
#include "std.hpp"
...
```

通常这将需要较长时间编译，但如果系统内存充足，预编译头文件方案，可以比没有预编译的情况下处理任何单个标准头文件的速度快得多。标准头文件在这方面特别方便，因为它们很少更改，因此可以只生成一次 `std.hpp` 文件的预编译头文件。否则，预编译头文件通常是项目依赖配置的一部分（例如，由常用的 `make` 工具或集成开发环境 (IDE) 的项目构建工具按需更新）。

管理预编译头文件的一种有效方法是创建预编译头文件的层次结构，从最广泛使用且稳定的头文件（例如我们的 `std.hpp` 头文件）到不经常更改且仍值得预编译的头文件。然而，*如果头文件处于频繁开发中，为其创建预编译头文件可能会耗时多于重用它们所节省的时间*。这个方法的关键概念是：可以重用更稳定层的预编译头文件以提高不太稳定头文件的预编译时间。例如，假设除了预编译我们的 `std.hpp` 头文件外，我们还定义了一个 `core.hpp` 头文件，它包含一些特定于项目的其他功能，但仍然达到了一定的稳定性：

```cpp
#include "std.hpp"
#include "core_data.hpp"
#include "core_algos.hpp"
...
```

因为这个文件以 `#include "std.hpp"` 开始，编译器可以加载相关的预编译头文件，并在下一行继续，而无需重新编译所有标准头文件。当文件完全处理完毕时，可以生成一个新的预编译头文件。应用程序可以使用 `#include "core.hpp"` 来快速访问大量功能，因为编译器可以加载后者的预编译头文件。

## 9.4 编译错误的解读

普通的编译错误通常相当简洁明了。例如，当编译器提示“类 X 没有成员函数 'fun'”时，通常不难弄清代码中的问题（比如，我们可能把 `run` 错写成了 `fun`）。但对于模板而言情况并非如此。让我们来看一些例子。

### 9.4.1 简单的类型不匹配

考虑下面这个使用了 C++ 标准库的相对简单的例子：[errornovel1.cpp](../../Codes/ch09/9_4/errornovel1.cpp)

```cpp
#include <string>
#include <map>
#include <algorithm>

int main()
{
    std::map<std::string, double> coll;

    // 在 coll 中寻找第一个非空的字符串
    auto pos = std::find_if(coll.begin(), coll.end(),
     [](std::string const& s){
        return s != "";
     });
}
```

这个代码包含一个相当小的错误：在 lambda 表达式中，我们检查了集合中的字符串。然而，`map` 中的元素是键值对，因此我们应该期望的是 `std::pair<std::string const, double>`。

GNU C++ 编译器报告了如下错误：

```bash
1 In file included from /cygdrive/p/gcc/gcc61-include/bits/stl_algobase.h:71:0,
2                  from /cygdrive/p/gcc/gcc61-include/bits/char_traits.h:39,
3                  from /cygdrive/p/gcc/gcc61-include/string:40,
4                  from errornovel1.cpp:1:
5 /cygdrive/p/gcc/gcc61-include/bits/predefined_ops.h: In instantiation of 'bool __gnu_cxx
  ::__ops::_Iter_pred<_Predicate>::operator()(_Iterator) [with _Iterator = std::_Rb_tree_i
  terator<std::pair<const std::__cxx11::basic_string<char>, double> >; _Predicate = main()
  ::<lambda(const string&)>]':
6 /cygdrive/p/gcc/gcc61-include/bits/stl_algo.h:104:42: required from '_InputIterator
  std::__find_if(_InputIterator, _InputIterator, _Predicate, std::input_iterator_tag)
  [with _InputIterator = std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string
  <char>, double> >; _Predicate = __gnu_cxx::__ops::_Iter_pred<main()::<lambda(const
  string&)> >]'
7 /cygdrive/p/gcc/gcc61-include/bits/stl_algo.h:161:23: required from '_Iterator std::__
  find_if(_Iterator, _Iterator, _Predicate) [with _Iterator = std::_Rb_tree_iterator<std::
  pair<const std::__cxx11::basic_string<char>, double> >; _Predicate = __gnu_cxx::__ops::_
  Iter_pred<main()::<lambda(const string&)> >]'
8 /cygdrive/p/gcc/gcc61-include/bits/stl_algo.h:3824:28: required from '_IIter std::find
  _if(_IIter, _IIter, _Predicate) [with _IIter = std::_Rb_tree_iterator<std::pair<const
  std::__cxx11::basic_string<char>, double> >; _Predicate = main()::<lambda(const string&)
  >]'
9 errornovel1.cpp:13:29: required from here
10 /cygdrive/p/gcc/gcc61-include/bits/predefined_ops.h:234:11: error: no match for call to
  '(main()::<lambda(const string&)>) (std::pair<const std::__cxx11::basic_string<char>,
  double>&)'
11 { return bool(_M_pred(*__it)); }
12 ^~~~~~~~~~~~~~~~~~~~
13 /cygdrive/p/gcc/gcc61-include/bits/predefined_ops.h:234:11: note: candidate: bool (*)(
  const string&) {aka bool (*)(const std::__cxx11::basic_string<char>&)} <conversion>
14 /cygdrive/p/gcc/gcc61-include/bits/predefined_ops.h:234:11: note: candidate expects 2
   arguments, 2 provided
15 errornovel1.cpp:11:52: note: candidate: main()::<lambda(const string&)>
16 [] (std::string const& s) {
17 ^
18 errornovel1.cpp:11:52: note: no known conversion for argument 1 from 'std::pair<const
   std::__cxx11::basic_string<char>, double>' to 'const string& {aka const std::__cxx11::
   basic_string<char>&}'
```

这样的错误信息看起来更像是一本小说，而不是诊断报告。对于模板的新手用户来说，这样的信息可能令人不知所措甚至丧失信心。然而，随着练习的增加，这些消息会变得易于处理，且错误的位置至少能相对轻松地找到。

错误信息的第一部分表示在 `predefined_ops.h` 头文件中的函数模板实例化时发生了错误，该头文件通过其他头文件从 `errornovel1.cpp` 中包含。编译器报告了什么被实例化了、使用了什么参数。在这种情况下，问题始于 `errornovel1.cpp` 第 13 行的语句：

```cpp
auto pos = std::find_if(coll.begin(), coll.end(),
    [] (std::string const& s) {
        return s != "";
    });
```

这导致了 `stl_algo.h` 头文件第 115 行上的 `find_if` 模板的实例化，其中代码：

```cpp
_IIter std::find_if(_IIter, _IIter, _Predicate)
```

被实例化为：

```cpp
_IIter = std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, double> >
_Predicate = main()::<lambda(const string&)>
```

编译器报告这些信息是为了让我们了解可能意外的模板实例化情况，以确定导致这些实例化的链条。

然而，在我们的例子中，我们相信各种模板的实例化是必要的，我们只是想知道为什么它不起作用。相关信息位于错误消息的最后部分：“no match for call” 表示某个函数调用未能解析，因为**参数的类型与函数形参的类型不匹配**。具体说明了调用的内容：

```cpp
(main()::<lambda(const string&)>)(std::pair<const std::__cxx11::basic_string<char>, double>&)
```

以及导致该调用的代码：

```cpp
{ return bool(_M_pred(*__it)); }
```

此外，“note: candidate:” 一行指出有一个候选函数期望接收 `const string&`，这个候选函数在 `errornovel1.cpp` 第 11 行定义的 lambda 表达式 `[] (std::string const& s)`，并结合了该候选函数不适用的原因：

```bash
no known conversion for argument 1
from 'std::pair<const std::__cxx11::basic_string<char>, double>'
to 'const string& {aka const std::__cxx11::basic_string<char>&}'
```

这正是我们遇到的问题。

毫无疑问，错误消息可以更好一些。实际的问题应该在实例化历史之前发出，并且可以仅使用 `std::string` 而不是完全展开的模板实例化名称 `std::__cxx11::basic_string<char>`。然而，诊断信息中的所有内容在某些情况下确实有用。因此，其他编译器提供类似信息也就不足为奇了（虽然有些使用了提到的结构化技术）。

例如，Visual C++ 编译器输出的信息可能类似于：

```bash
1 c:\tools_root\cl\inc\algorithm(166): error C2664: 'bool main::<lambda_b863c1c7cd07048816
  f454330789acb4>::operator ()(const std::string &) const': cannot convert argument 1 from
  'std::pair<const _Kty,_Ty>' to 'const std::string &'
2         with
3         [
4             _Kty=std::string,
5             _Ty=double
6         ]
7 c:\tools_root\cl\inc\algorithm(166): note: Reason: cannot convert from 'std::pair<const
  _Kty,_Ty>' to 'const std::string'
8         with
9         [
10            _Kty=std::string,
11            _Ty=double
12        ]
13 c:\tools_root\cl\inc\algorithm(166): note: No user-defined-conversion operator available
   that can perform this conversion, or the operator cannot be called
14 c:\tools_root\cl\inc\algorithm(177): note: see reference to function template instantiat
   ion '_InIt std::_Find_if_unchecked<std::_Tree_unchecked_iterator<_Mytree>,_Pr>(_InIt,_In
   It,_Pr &)' being compiled
15       with
16       [
17            _InIt=std::_Tree_unchecked_iterator<std::_Tree_val<std::_Tree_simple_types
              <std::pair<const std::string,double>>>>,
18           _Mytree=std::_Tree_val<std::_Tree_simple_types<std::pair<const std::string,
             double>>>,
19           _Pr=main::<lambda_b863c1c7cd07048816f454330789acb4>
20       ]
21 main.cpp(13): note: see reference to function template instantiation '_InIt std::find_if
   <std::_Tree_iterator<std::_Tree_val<std::_Tree_simple_types<std::pair<const _Kty,_Ty>>>>
   ,main::<lambda_b863c1c7cd07048816f454330789acb4>>(_InIt,_InIt,_Pr)' being compiled
22       with
23       [
24           _InIt=std::_Tree_iterator<std::_Tree_val<std::_Tree_simple_types<std::pair<
             const std::string,double>>>>,
25           _Kty=std::string,
26           _Ty=double,
27           _Pr=main::<lambda_b863c1c7cd07048816f454330789acb4>
28       ]
```

在这里，我们再次提供了实例化链式的信息，告诉我们哪些内容以哪些参数在代码中实例化了，我们两次看到无法从 `std::pair<const _Kty, _Ty>` 转换为 `const std::string`，其中：

```bash
cannot convert from 'std::pair<const _Kty,_Ty>' to 'const std::string'
with
[_Kty=std::string, _Ty=double]
```

### 9.4.1 某些编译器上缺少 `const`

不幸的是，泛型代码有时只会在某些编译器上出现问题。考虑下面的例子：[errornovel2.cpp](../../Codes/ch09/9_4/errornovel2.cpp)

```cpp
#include <string>
#include <unordered_set>

class Customer {
private:
    std::string name;
public:
    Customer (std::string const& n) : name(n) {}
    std::string getName() const {
        return name;
    }
};

int main() {
    // 提供我们自己的哈希函数：
    struct MyCustomerHash {
        // 注意：缺少 const 只有在 g++ 和 clang 中会报错：
        std::size_t operator() (Customer const& c) {
            return std::hash<std::string>()(c.getName());
        }
    };

    // 使用它为 Customer 创建哈希表：
    std::unordered_set<Customer, MyCustomerHash> coll;
    ...
}
```

在 Visual Studio 2013 或 2015 上，这段代码按预期编译。但在 g++ 或 clang 上，代码会引发大量错误消息。例如，在 g++ 6.1 上，第一个错误消息如下：

```bash
1 In file included from /cygdrive/p/gcc/gcc61-include/bits/hashtable.h:35:0,
2                  from /cygdrive/p/gcc/gcc61-include/unordered_set:47,
3                  from errornovel2.cpp:2:
4 /cygdrive/p/gcc/gcc61-include/bits/hashtable_policy.h: In instantiation of 'struct std::
  __detail::__is_noexcept_hash<Customer, main()::MyCustomerHash>':
5 /cygdrive/p/gcc/gcc61-include/type_traits:143:12: required from 'struct std::__and_<
  std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::__is_noexcept_hash<Customer,
  main()::MyCustomerHash> >'
6 /cygdrive/p/gcc/gcc61-include/type_traits:154:38: required from 'struct std::__not_<
  std::__and_<std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::__is_noexcept_
  hash<Customer, main()::MyCustomerHash> > >'
7 /cygdrive/p/gcc/gcc61-include/bits/unordered_set.h:95:63: required from 'class std::
  unordered_set<Customer, main()::MyCustomerHash>'
8 errornovel2.cpp:28:47: required from here
9 /cygdrive/p/gcc/gcc61-include/bits/hashtable_policy.h:85:34: error: no match for call to
  '(const main()::MyCustomerHash) (const Customer&)'
10 noexcept(declval<const _Hash&>()(declval<const _Key&>()))>
11          ~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~
12 errornovel2.cpp:22:17: note: candidate: std::size_t main()::MyCustomerHash::operator()(
   const Customer&) <near match>
13       std::size_t operator() (const Customer& c) {
14                   ^~~~~~~~
15 errornovel2.cpp:22:17: note: passing 'const main()::MyCustomerHash*' as 'this' argument
   discards qualifiers
```

接着出现了超过 20 条其他错误消息：

```bash
16 In file included from /cygdrive/p/gcc/gcc61-include/bits/move.h:57:0,
18                  from /cygdrive/p/gcc/gcc61-include/bits/stl_pair.h:59,
19                  from /cygdrive/p/gcc/gcc61-include/bits/stl_algobase.h:64,
20                  from /cygdrive/p/gcc/gcc61-include/bits/char_traits.h:39,
21                  from /cygdrive/p/gcc/gcc61-include/string:40,
22                  from errornovel2.cpp:1:
23 /cygdrive/p/gcc/gcc61-include/type_traits: In instantiation of 'struct std::__not_<std::
   __and_<std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::__is_noexcept_hash<
   Customer, main()::MyCustomerHash> > >':
24 /cygdrive/p/gcc/gcc61-include/bits/unordered_set.h:95:63: required from 'class std::
   unordered_set<Customer, main()::MyCustomerHash>'
25 errornovel2.cpp:28:47: required from here
26 /cygdrive/p/gcc/gcc61-include/type_traits:154:38: error: 'value' is not a member of 'std
   ::__and_<std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::__is_noexcept_hash<
   Customer, main()::MyCustomerHash> >'
27 : public integral_constant<bool, !_Pp::value>
28                                  ^~~~
29 In file included from /cygdrive/p/gcc/gcc61-include/unordered_set:48:0,
30 from errornovel2.cpp:2:
31 /cygdrive/p/gcc/gcc61-include/bits/unordered_set.h: In instantiation of 'class std::
   unordered_set<Customer, main()::MyCustomerHash>':
32 errornovel2.cpp:28:47: required from here
33 /cygdrive/p/gcc/gcc61-include/bits/unordered_set.h:95:63: error: 'value' is not a member
   of 'std::__not_<std::__and_<std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::
   __is_noexcept_hash<Customer, main()::MyCustomerHash> > >'
34 typedef __uset_hashtable<_Value, _Hash, _Pred, _Alloc> _Hashtable;
35                                                        ^~~~~~~~~~
36 /cygdrive/p/gcc/gcc61-include/bits/unordered_set.h:102:45: error: 'value' is not a member
  of 'std::__not_<std::__and_<std::__is_fast_hash<main()::MyCustomerHash>, std::__detail::
  __is_noexcept_hash<Customer, main()::MyCustomerHash> > >'
37          typedef typename _Hashtable::key_type key_type;
38                                                ^~~~~~~~
```

再次说明，错误信息很难阅读（甚至找到每条消息的开始和结束都是件麻烦事）。核心问题在于，`hashtable_policy.h` 头文件中的 `std::unordered_set<>` 模板实例化需要：

```cpp
std::unordered_set<Customer, MyCustomerHash> coll;
```

然而，在实例化过程中，未找到以下调用的匹配项：

```cpp
const main()::MyCustomerHash (const Customer&)
```

在实例化的表达式中：

```cpp
noexcept(declval<const _Hash&>()(declval<const _Key&>()))>
~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~
```

（`declval<const _Hash&>()` 是 `main()::MyCustomerHash` 类型的表达式）。一个可能的“接近匹配”候选函数是：

```cpp
std::size_t main()::MyCustomerHash::operator()(const Customer&)
```

声明为：

```cpp
std::size_t operator() (const Customer& c) {
^~~~~~~~
```

最后的提示指出问题所在：

```cpp
passing 'const main()::MyCustomerHash*' as 'this' argument discards qualifiers
```

你能看出问题吗？这个 `std::unordered_set` 类模板的实现要求哈希对象的函数调用运算符是一个 `const` 成员函数（参见第 11.1.1 节）。如果不是这样，就会在算法的内部引发错误。

其他所有错误消息都是由第一个错误引发的，并且只需在哈希函数运算符中添加 `const` 限定符即可解决问题：

```cpp
std::size_t operator() (const Customer& c) const {
...
}
```

Clang 3.9 在第一个错误消息的末尾给出了稍好的提示，指出哈希函子的 `operator()` 未被标记为 `const`：

```bash
errornovel2.cpp:28:47: note: in instantiation of template class 'std::unordered_set<Customer, MyCustomerHash, std::equal_to<Customer>, std::allocator<Customer> >' requested here
std::unordered_set<Customer, MyCustomerHash> coll;
^
errornovel2.cpp:22:17: note: candidate function not viable: 'this' argument has type 'const MyCustomerHash', but method is not marked const
std::size_t operator() (const Customer& c) {
^
```

注意，clang 提到了像 `std::allocator<Customer>` 这样的默认模板参数，而 gcc 则跳过了这些。

正如你所见，同时使用多个编译器来测试代码通常是有帮助的。这不仅有助于编写更具可移植性的代码，还能在某个编译器给出特别难以理解的错误信息时，从另一个编译器中获得更多的反馈。

## 9.5 后记

在头文件和 CPP 文件中组织源代码，是“一次定义规则”（ODR）各种表现形式的实际结果。对此规则的广泛讨论见附录 A。

包含模型主要是由现有 C++ 编译器实现中的实践，得出来的解决方案。然而，第一个 C++ 实现与此不同：模板定义的包含是隐式的，营造出一种分离的假象（详情见第 14 章关于这种原始模型的描述）。

第一个 C++ 标准（[C++98]）通过导出模板提供了对模板编译分离模型的明确支持。分离模型允许在头文件中声明带有 `export` 标记的模板声明，而相应的定义则放在 CPP 文件中，类似于非模板代码的声明和定义。然而，不同于包含模型，这个模型是一个理论模型，并未基于任何现有的实现，而其实现远比 C++ 标准委员会预期的要复杂。直到 2002 年 5 月，第一个实现才发布，而在此后的几年里，未出现其他实现。为了更好地与现有实践保持一致，C++ 标准委员会在 C++11 中移除了导出模板。对于想了解分离模型细节（及其陷阱）的读者，推荐阅读本书第一版的第 6.3 节和第 10.3 节。

有时人们会设想扩展预编译头文件的概念，以便在一次编译中加载多个头文件。这在原则上可以实现对预编译的更细粒度控制。这里的主要障碍是预处理器：一个头文件中的宏可以完全改变后续头文件的含义。然而，一旦文件被预编译，宏处理已经完成，试图修补其他头文件引入的预处理器效果几乎是不现实的。一个名为模块的新语言特性（见第 17.11 节）预计将在不久的将来添加到 C++ 中，以解决此问题（宏定义无法泄露到模块接口中）。

## 9.6 总结

- 模板的包含模型是组织模板代码的最广泛使用的模型。第 14 章讨论了其他替代方案；
- 只有函数模板的完全特化，需要在类或结构体之外的头文件中定义为 `inline`；
- 为了充分利用预编译头文件，请确保 `#include` 指令的顺序保持一致；
- 使用模板调试代码可能会具有挑战性；


[1]: 在某些实现中，这个字符串可能会被“混淆”（即通过对参数类型和周围作用域的名称进行编码，以将其与其他名称区分开来），但可以使用一个“解混淆器”将其转换为人类可读的文本。

[2]: 理论上，标准头文件实际上不需要对应物理文件。然而，在实际操作中，它们确实对应物理文件，并且这些文件非常大。