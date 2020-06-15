#ifndef INCREMENTAL_REHASH_OPEN_SET_H
#define INCREMENTAL_REHASH_OPEN_SET_H

#include"utils/hash.h"
#include"hash_tables/robin_hood_table.h"
#include<type_traits>

template<
    class T,
    bool NeedStoreHash = true,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class incremental_rehash_open_set : private Hash, private KeyEqual
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using hash_table = robin_hood_hash_table<T, NeedStoreHash, Hash, KeyEqual>;
    using handle = typename hash_table::handle;
    using const_handle = const handle;
private:
    hash_table t1, t2;
    std::size_t t2_next_bucket_idx = 0;
    unsigned char log2_bucket_cnt;
public:
    incremental_rehash_open_set() = default;
    explicit incremental_rehash_open_set(unsigned );
    incremental_rehash_open_set(const incremental_rehash_open_set &) = delete;
    incremental_rehash_open_set &operator=(const incremental_rehash_open_set &) = delete;
    ~incremental_rehash_open_set();
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    std::size_t hash(const value_type &k) const { return hash_function()(k); }

    bool full() const 
    { return 4 * t1.size() >= 3 * t1.bucket_count(); }
    size_type size() const
    { return t2 ? t1.size() +  t2.size() : t1.size(); }
    size_type bucket_count() const
    { return t1.bucket_count(); }

    handle find(const value_type &);
    handle find(value_type && v)
    { return find(v); }
    const_handle find(const value_type &) const;
    template<class... Args>
    std::pair<handle, bool> emplace(Args&&... args)
    { return insert({ std::forward<Args>(args)... }); }
    std::pair<handle, bool> insert(const value_type &);
    std::pair<handle, bool> insert(value_type &&);
    size_type erase(const value_type & );
    size_type erase(value_type && );
private:
    template <class Data>
    std::pair<handle, bool> insert_(Data && , size_t );
    template <class Data>
    size_type erase_(Data &&, size_t );
    void rehash();
    void move_element();
};

template <class T, bool StoreHash, class H, class EQ>
incremental_rehash_open_set<T, StoreHash, H, EQ>::incremental_rehash_open_set(unsigned k) :
    t1(k), t2()
{
}

template <class T, bool StoreHash, class H, class EQ>
incremental_rehash_open_set<T, StoreHash, H, EQ>::~incremental_rehash_open_set()
{}

template <class T, bool StoreHash, class H, class EQ>
auto incremental_rehash_open_set<T, StoreHash, H, EQ>::find(
        const value_type &val) -> handle
{
    auto hash = hasher()(val);
    auto idx1 = t1.get_index(hash);
    auto orig_idx = idx1;
    auto h = t1.find_(val, hash, idx1);
    if(!h && t2)
    {
        auto idx2 = std::max(orig_idx >> 1, t2_next_bucket_idx);
        h = t2.find_in_deprecated_table(val, hash, idx2);
    }
    return h;
}

template <class T, bool StoreHash, class H, class EQ>
auto incremental_rehash_open_set<T, StoreHash, H, EQ>::find(
        const value_type &val) const -> const_handle
{
    auto hash = hasher()(val);
    auto orig_idx1 = t1.get_index(hash);
    auto idx1 = orig_idx1;
    auto h = t1.find_(val, hash, idx1);
    if(!h && t2)
    {
        auto idx2 = std::max(orig_idx1 >> 1, t2_next_bucket_idx);
        h = t2.find_in_deprecated_table(val, hash, idx2);
    }
    return h;
}

template <class T, bool StoreHash, class H, class EQ>
auto incremental_rehash_open_set<T, StoreHash, H, EQ>::insert(value_type &&v)
-> std::pair<handle, bool>
{
    if(full()) rehash();
    if(t2) move_element();
    auto hash = hash_function()(v);
    return insert_(std::move(v),hash);

}

template <class T, bool StoreHash, class H, class EQ>
auto incremental_rehash_open_set<T, StoreHash, H, EQ>::insert(const value_type &v)
-> std::pair<handle, bool>
{
    if(full()) rehash();
    if(t2) move_element();
    auto hash = hash_function()(v);
    return insert_(v, hash);
}

template <class T, bool StoreHash, class H, class EQ>
template <class Data>
auto incremental_rehash_open_set<T, StoreHash, H, EQ>::insert_(Data &&v, size_type hash)
-> std::pair<handle, bool>
{
    auto idx1 = t1.get_index(hash);
    auto ideal_idx = idx1;
    if (auto h = t1.find_(v, hash, idx1)) return {h, false};
    typename hash_table::distance_type psl = (idx1 - ideal_idx) & t1.mask;
    if (t2)
    {
        auto idx2 = std::max(ideal_idx >> 1, t2_next_bucket_idx);
        if(auto h = t2.find_in_deprecated_table(v, hash, idx2)){
            value_type tmp;
            t2.deprecate_(tmp, h);
            return { t1.add_bucket(std::move(tmp), hash, std::move(idx1), psl),
                     false };
        }
    }
    return { t1.add_bucket(std::forward<Data>(v), hash, std::move(idx1), psl), true };
}

template <class T, bool StoreHash, class H, class EQ>
size_t incremental_rehash_open_set<T, StoreHash, H, EQ>::erase(const value_type &v)
{
    auto hash = hash_function()(v);
    auto deleted_elem_cnt = erase_(v, hash);
    if(t2) move_element();
    return deleted_elem_cnt;
}

template <class T, bool StoreHash, class H, class EQ>
size_t incremental_rehash_open_set<T, StoreHash, H, EQ>::erase(value_type &&v)
{
    auto hash = hash_function()(v);
    auto deleted_elem_cnt = erase_(std::move(v), hash);
    if(t2) move_element();
    return deleted_elem_cnt;
}

template <class T, bool StoreHash, class H, class EQ>
template <class Data>
size_t incremental_rehash_open_set<T, StoreHash, H, EQ>::erase_(Data &&v, size_t hash)
{
    auto idx1 = t1.get_index(hash);
    auto orig_idx = idx1;
    if(auto h = t1.find_(v, hash, idx1)){
        t1.erase_(h, idx1);
        return 1;
    }
    if(t2)
    {
         auto idx2 = std::max(orig_idx >> 1, t2_next_bucket_idx);
         if(auto h = t2.find_in_deprecated_table(v, hash, idx2)){
             value_type tmp;
             t2.deprecate_(tmp, h);
             return 1;
         }
    }
    return 0;
}

template <class T, bool StoreHash, class H, class EQ>
void incremental_rehash_open_set<T, StoreHash, H, EQ>::rehash()
{
    if(t2 || t1.log2_bucket_cnt == std::numeric_limits<std::size_t>::digits)
        return;
    t2.create(t1.log2_bucket_cnt + 1);
    t1.swap(t2);
}

template <class T, bool StoreHash, class H, class EQ>
void incremental_rehash_open_set<T, StoreHash, H, EQ>::move_element()
{
    assert(t2);
    assert(!t2.empty());
    auto idx = t2_next_bucket_idx;
    for(int c = 2; c--;)
    {
        while(t2.table[idx].is_empty() || t2.table[idx].is_deprecated()) idx++;
        value_type v {};
        t2.deprecate_(v, idx);
        t1.insert(std::move(v));
        if(t2.empty())
        {
            t2_next_bucket_idx = 0;
            t2.destroy();
            return;
        }
    }
    t2_next_bucket_idx = idx;
}
#endif
