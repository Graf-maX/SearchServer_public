#pragma once

#include <cstdlib>
#include <limits>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <vector>
#include <type_traits>
#include <utility>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");
    class Access;

    explicit ConcurrentMap(const size_t bucket_count) : maps_(bucket_count) {}
    Access operator[](const Key& key);
    void erase(const Key& key);
    std::map<Key, Value> BuildOrdinaryMap();

private:
    struct ProtectedMap;

    std::vector<ProtectedMap> maps_;
    ProtectedMap& GetProtectedMap(const Key& key);
};

template <typename Key, typename Value>
class ConcurrentMap<Key, Value>::Access {
public:
    Access(Value& value, std::mutex& lck) : ref_to_value_(value),
                                            lock_mtx_(lck) {}
    ~Access() {
        lock_mtx_.unlock();
    }

    operator Value&();

private:
    Value& ref_to_value_;
    std::mutex& lock_mtx_;
};

template <typename Key, typename Value>
ConcurrentMap<Key, Value>::Access::operator Value&() {
    return ref_to_value_;
}

template <typename Key, typename Value>
struct ConcurrentMap<Key, Value>::ProtectedMap {
    std::map<Key, Value> map_values;
    std::mutex           mutex_map;
};


template <typename Key, typename Value>
typename ConcurrentMap<Key, Value>::Access
ConcurrentMap<Key, Value>::operator[](const Key& key) {
    auto& prot_map = GetProtectedMap(key);
    prot_map.mutex_map.lock();
    return Access(prot_map.map_values[key], prot_map.mutex_map);
}

template <typename Key, typename Value>
void ConcurrentMap<Key, Value>::erase(const Key& key) {
    auto& prot_map = GetProtectedMap(key);
    std::lock_guard<std::mutex> guard(prot_map.mutex_map);
    prot_map.map_values.erase(key);
}

template <typename Key, typename Value>
std::map<Key, Value> ConcurrentMap<Key, Value>::BuildOrdinaryMap() {
    std::map<Key, Value> result_map;
    for (auto& m : maps_) {
        std::lock_guard<std::mutex> guard(m.mutex_map);
        result_map.insert(m.map_values.begin(), m.map_values.end());
    }
    return result_map;
}

template <typename Key, typename Value>
typename ConcurrentMap<Key, Value>::ProtectedMap&
ConcurrentMap<Key, Value>::GetProtectedMap(const Key& key) {
    return maps_[static_cast<uint64_t>(key) % maps_.size()];
}
