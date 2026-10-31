// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_RESULTCACHE_H
#define THERMOFUN_ENGINE_RESULTCACHE_H

#include <cstddef>
#include <list>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>

namespace ThermoFun::v2 {

/// What identifies a calculation. The engine clears its caches when the database revision or a setting changes.
struct ResultKey
{
    std::string symbol;
    double T = 0.0;
    double P = 0.0;           ///< as given: 0 means the saturation pressure
    int state = -1;           ///< the state of water that was asked for; -1 for none
    bool gas_correction = false;   ///< a reaction switches on the pressure correction of its gases: another result

    auto operator<(const ResultKey& o) const -> bool
    {
        return std::tie(symbol, T, P, state, gas_correction) < std::tie(o.symbol, o.T, o.P, o.state, o.gas_correction);
    }
};

/// The results of calculations, the least recently used dropped first. A pressure of 0 is written back as the saturation
/// pressure by a calculation, so an entry holds the pressure after the calculation, and a hit writes it back too.
template <class Value>
class ResultCache
{
public:
    ResultCache() = default;
    /// A copy starts empty.
    ResultCache(const ResultCache&) {}
    auto operator=(const ResultCache&) -> ResultCache& { clear(); return *this; }

    /// The result of a calculation, if it is there; P gets the pressure after that calculation.
    auto find(const ResultKey& key, double& P) -> std::optional<Value>
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = entries_.find(key);
        if (it == entries_.end())
            return std::nullopt;
        usage_.splice(usage_.begin(), usage_, it->second.usage);
        P = it->second.P_after;
        return it->second.value;
    }

    /// Keeps a result. limit: the largest number of entries; 0 is no limit.
    void store(const ResultKey& key, const Value& value, double P_after, std::size_t limit)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (entries_.count(key))
            return;
        while (limit > 0 && entries_.size() >= limit)
        {
            entries_.erase(*usage_.back());
            usage_.pop_back();
        }
        const auto it = entries_.emplace(key, Entry{value, P_after, {}}).first;
        usage_.push_front(&it->first);
        it->second.usage = usage_.begin();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
        usage_.clear();
    }

    auto size() const -> std::size_t
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }

private:
    using Usage = std::list<const ResultKey*>;   // most recent first
    struct Entry
    {
        Value value;
        double P_after;
        typename Usage::iterator usage;
    };
    mutable std::mutex mutex_;
    std::map<ResultKey, Entry> entries_;
    Usage usage_;
};

} // namespace ThermoFun::v2

#endif
