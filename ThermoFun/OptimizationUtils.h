// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// C++ includes
#include <functional>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <list>

namespace ThermoFun {

namespace detail {

/// A memoized call: the argument values as they stood when f() returned,
/// alongside the value it returned. The argument values are kept so that a
/// cache hit can replay whatever f() wrote through its reference parameters.
template <typename Key, typename Ret>
struct MemoizedCall
{
    Key out_args;
    Ret value;
};

/// As MemoizedCall, plus this entry's position in the usage-order list, so that
/// a cache hit can promote it to most-recently-used in constant time.
template <typename Key, typename Ret>
struct MemoizedCallLRU
{
    Key out_args;
    Ret value;
    typename std::list<Key>::iterator lru_pos;
};

/// Copy the post-call argument values in `stored` back into `args`, for those
/// parameters f() is able to write through (non-const lvalue references);
/// parameters taken by value are left untouched.
///
/// A cache hit returns without running f(), so without this the writes f()
/// would have made are simply lost. ThermoFun's memoized property functions
/// take P as an in-out parameter - a caller passing P == 0 means "at the
/// saturation pressure" and gets the computed Psat back through the reference
/// (ThermoModelsSolvent.cpp, WaterHGK-JNgems.cpp) - so a hit has to deliver
/// that value just as a miss does.
template <typename... Args, std::size_t... I>
void restoreOutArgs(const std::tuple<std::decay_t<Args>...>& stored,
                    std::index_sequence<I...>, Args&... args)
{
    ([&]{
        if constexpr (std::is_lvalue_reference_v<Args> &&
                      !std::is_const_v<std::remove_reference_t<Args>>)
            args = std::get<I>(stored);
    }(), ...);
}

} // namespace detail

template <typename Ret, typename... Args>
auto memoize(std::function<Ret(Args...)> f) -> std::function<Ret(Args...)>
{
    // Key on decayed argument types. Args... may contain references (the
    // double& P of ThermoEngine's property functions), and keying on the
    // reference itself would leave every entry holding a reference into the
    // caller's frame - dangling, and corrupting the map's ordering, as soon as
    // that frame died. The key is built before the call, so it holds the
    // argument values the caller passed in.
    using Key = std::tuple<std::decay_t<Args>...>;
    using Entry = detail::MemoizedCall<Key, Ret>;

    auto cache = std::make_shared<std::map<Key, Entry>>();
    return [=](Args... args) mutable -> Ret
    {
        Key t(args...);
        auto it = cache->find(t);
        if (it == cache->end())
        {
            Ret value = f(args...); // may write through reference arguments
            it = cache->emplace(std::move(t), Entry{Key(args...), std::move(value)}).first;
        }
        else
            detail::restoreOutArgs<Args...>(it->second.out_args,
                                            std::index_sequence_for<Args...>{}, args...);
        return it->second.value;
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
    // Key on the argument values, not on the references - see memoize() above.
    using Key = std::tuple<std::decay_t<Args>...>;
    using Entry = detail::MemoizedCallLRU<Key, Ret>;

    // Both the cache and the usage order are shared, so that copies of the
    // returned std::function go on operating as one cache, and so the iterators
    // the entries hold into usage_order stay valid across those copies.
    auto cache = std::make_shared<std::map<Key, Entry>>();
    auto usage_order = std::make_shared<std::list<Key>>(); // front = most recently used

    return [=](Args... args) mutable -> Ret
    {
        Key t(args...); // Create a key from the argument values

        auto it = cache->find(t);
        if (it != cache->end())
        {
            // Cache hit. Promote to most recently used - without this the list
            // is in insertion order rather than usage order, and eviction is
            // not LRU at all.
            usage_order->splice(usage_order->begin(), *usage_order, it->second.lru_pos);
            // Replay whatever f() wrote through its reference arguments.
            detail::restoreOutArgs<Args...>(it->second.out_args,
                                            std::index_sequence_for<Args...>{}, args...);
            return it->second.value;
        }

        // Cache miss. Make room before inserting - evicting here rather than on
        // every call, so that looking up the least recently used entry is a hit
        // instead of evicting the very entry being asked for.
        while (cache->size() >= max_cache_size && !usage_order->empty())
        {
            cache->erase(usage_order->back());
            usage_order->pop_back();
        }

        Ret value = f(args...); // may write through reference arguments
        Key out_args(args...);
        auto pos = cache->emplace(t, Entry{std::move(out_args), std::move(value),
                                           usage_order->end()}).first;
        usage_order->push_front(std::move(t));
        pos->second.lru_pos = usage_order->begin();

        return pos->second.value;
    };
}



} // namespace ThermoFun
