#include <utility>      // std::invoke()
#include <functional>   // std::forward()
#include <type_traits>  // std::is_same<> and invoke_result<>

template<typename Callable, typename... Args>
decltype(auto) call(Callable &&op, Args&&... args)
{
    if constexpr(std::is_same_v<std::invoke_result_t<Callable, Args...>, 
        void>)
    {
        // 返回类型为 void
        std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...);
        ...
        return;
    }
    else
    {
        // 返回类型不为 void
        decltype(auto) ret{std::invoke(std::forward<Callable>(op), std::forward<Args>(args)...)};
        ...
        return ret;
    }
}