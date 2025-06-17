// C++ includes
#include <functional>
#include <map>
#include <memory>
#include <tuple>

#include <list>

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



template <typename Ret, typename... Args>
auto memoizeN(std::function<Ret(Args...)> f, size_t max_cache_size) -> std::function<Ret(Args...)>
{
    auto cache = std::make_shared<std::map<std::tuple<Args...>, Ret>>();
    std::list<std::tuple<Args...>> usage_order; // List to track the order of usage for LRU cache management

    return [=](Args... args) mutable -> Ret
    {
        std::tuple<Args...> t(args...); // Create a tuple key from the arguments

        // Check cache size and evict the least recently used item if necessary before adding
        if (cache->size() >= max_cache_size) {
            if (!usage_order.empty()) {
                std::tuple<Args...> lru_t = usage_order.back(); // Get the least recently used key
                usage_order.pop_back();                           // Remove from usage order
                cache->erase(lru_t);                             // Remove from cache
            }
        }

        // Check if result is already in cache
        if (cache->find(t) == cache->end()) { // t not found; compute the result
            (*cache)[t] = f(args...); // Cache the result
            usage_order.push_front(t); // Track the usage order
        }
        
        return (*cache)[t]; // Return the cached result
    };
}



} // namespace ThermoFun

