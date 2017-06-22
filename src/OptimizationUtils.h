// C++ includes
#include <functional>
#include <map>
#include <memory>
#include <tuple>

namespace ThermoFun {

template <typename Ret, typename... Args>
auto memoize(std::function<Ret(Args...)> f) -> std::function<Ret(Args...)>
{
    auto cache = std::make_shared<std::map<std::tuple<Args...>, Ret>>();
    return [=](Args... args) mutable -> Ret
    {
        std::tuple<Args...> t(args...);
        if(cache->find(t) == cache->end())
            (*cache)[t] = f(args...);
        return (*cache)[t];
    };
}

template<typename Ret, typename... Args>
auto memoizeLast(std::function<Ret(Args...)> f) -> std::function<Ret(Args...)>
{
    std::tuple<typename std::decay<Args>::type...> cache;
    Ret result = Ret();
    return [=](Args... args) mutable -> Ret
    {
        if(std::tie(args...) == cache)
            return Ret(result);
        cache = std::make_tuple(args...);
        return result = f(args...);
    };
}

template<typename Ret, typename... Args>
auto memoizeLastPtr(std::function<Ret(Args...)> f) -> std::shared_ptr<std::function<Ret(Args...)>>
{
    return std::make_shared<std::function<Ret(Args...)>>(memoizeLast(f));
}

template<typename Ret, typename... Args>
auto dereference(const std::shared_ptr<std::function<Ret(Args...)>>& f) -> std::function<Ret(Args...)>
{
    return [=](Args... args) -> Ret { return (*f)(args...); };
}

} // namespace ThermoFun

