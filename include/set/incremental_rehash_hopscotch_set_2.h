#ifndef INCREMENTAL_REHASH_HOPSCOTCH_SET_2_H
#define INCREMENTAL_REHASH_HOPSCOTCH_SET_2_H

#include"utils/hash.h"
#include"utils/plf_colony.h"
#include"hash_tables/hopscocth_hash_table_4.h"
#include<type_traits>

// for non-trivial data types
template<
    class T,
    unsigned int NeighborhoodSize = 30,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class incremental_rehash_hopscotch_set_2 : private Hash, private KeyEqual
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using hash_table = hopscotch_hash_table_4<T, NeighborhoodSize, Hash, KeyEqual>;
    using handle = typename hash_table::handle;
    using const_handle = const handle;
private:
    plf::colony<value_type> data_storage;
    hash_table t1, t2;
    std::size_t t2_next_bucket_idx = 0;
public:
    incremental_rehash_hopscotch_set_2() : 
        data_storage(),
        t1(0, data_storage),
        t2(0, data_storage)
    {}                    ;
    explicit incremental_rehash_hopscotch_set_2(unsigned );
    incremental_rehash_hopscotch_set_2(const incremental_rehash_hopscotch_set_2 &) = delete;
    incremental_rehash_hopscotch_set_2 &operator=(const incremental_rehash_hopscotch_set_2 &) = delete;
    ~incremental_rehash_hopscotch_set_2();
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

template <class T, unsigned int NbSize,  class H, class EQ>
incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::incremental_rehash_hopscotch_set_2(unsigned k) :
    data_storage(k ? (size_type(1) << k) : 0)
    , t1(k, data_storage), t2(0, data_storage)
{
}

template <class T, unsigned int NbSize,  class H, class EQ>
incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::~incremental_rehash_hopscotch_set_2()
{}

template <class T, unsigned int NbSize,  class H, class EQ>
auto incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::find(
        const value_type &val) -> handle
{
    auto hash = hasher()(val);
    //auto idx1 = t1.get_index(hash);
   // auto orig_idx = idx1;
    auto h = t1.find_(val, hash);
    if(!h && t2)
    {
        //auto idx2 = std::max(orig_idx >> 1, t2_next_bucket_idx);
        h = t2.find_(val, hash);
    }
    return h;
}

template <class T, unsigned int NbSize,  class H, class EQ>
auto incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::find(
        const value_type &val) const -> const_handle
{
    auto hash = hasher()(val);
    //auto orig_idx1 = t1.get_index(hash);
    //auto idx1 = orig_idx1;
    auto h = t1.find_(val, hash);
    if(!h && t2)
    {
        //auto idx2 = std::max(orig_idx1 >> 1, t2_next_bucket_idx);
        h = t2.find_(val, hash);
    }
    return h;
}

template <class T, unsigned int NbSize,  class H, class EQ>
auto incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::insert(value_type &&v)
-> std::pair<handle, bool>
{
    if(full()) rehash();
    if(t2) move_element();
    auto hash = hash_function()(v);
    return insert_(std::move(v),hash);

}

template <class T, unsigned int NbSize,  class H, class EQ>
auto incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::insert(const value_type &v)
-> std::pair<handle, bool>
{
    if(full()) rehash();
    if(t2) move_element();
    auto hash = hash_function()(v);
    return insert_(v, hash);
}

template <class T, unsigned int NbSize,  class H, class EQ>
template <class Data>
auto incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::insert_(Data &&v, size_type hash)
-> std::pair<handle, bool>
{
    if (auto h = t1.find_(v, hash)) return {h, false};
    if (t2)
    {
        auto idx = t2.get_index(hash);
        value_type *tmp;
        handle h;
        if(auto h = t2.find_in_table(v, hash, idx))
        {
            t2.deprecate_(tmp, h, idx);
            assert(tmp != nullptr);
            return { t1.add_data_ptr(tmp, hash),
                     false };
        }
        else if(t2.table[idx].has_overflow())
        {
            // TODO: correct this
            if(auto it = t2.find_in_overflow_list(v); it != t2.overflow_list.end())
            {
                value_type tmp{};
                t2.deprecate_in_overflow_list(tmp, it);
                return { t1.add_bucket(std::move(tmp), hash),
                     false };
            }
        }
    }
    return { t1.add_bucket(std::forward<Data>(v), hash), true };
}

template <class T, unsigned int NbSize,  class H, class EQ>
size_t incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::erase(const value_type &v)
{
    auto hash = hash_function()(v);
    auto deleted_elem_cnt = erase_(v, hash);
    if(t2) move_element();
    return deleted_elem_cnt;
}

template <class T, unsigned int NbSize,  class H, class EQ>
size_t incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::erase(value_type &&v)
{
    auto hash = hash_function()(v);
    auto deleted_elem_cnt = erase_(std::move(v), hash);
    if(t2) move_element();
    return deleted_elem_cnt;
}

template <class T, unsigned int NbSize,  class H, class EQ>
template <class Data>
size_t incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::erase_(Data &&v, size_t hash)
{
    if (auto h = t1.erase_impl(v, hash)) return 1;
    if(t2)
    {
        return t2.erase_impl(v, hash);
    }
    return 0;
}

template <class T, unsigned int NbSize,  class H, class EQ>
void incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::rehash()
{
    if(t2 || t1.log2_bucket_cnt == std::numeric_limits<std::size_t>::digits)
        return;
    t2.create(t1.log2_bucket_cnt + 1);
    t1.swap(t2);
}

template <class T, unsigned int NbSize,  class H, class EQ>
void incremental_rehash_hopscotch_set_2<T, NbSize, H, EQ>::move_element()
{
    assert(t2);
    //std::cout << "t2.size " << t2.size() << std::endl;
    //assert(!t2.empty());
    if(t2.empty())
    {
        t2_next_bucket_idx = 0;
        t2.destroy();
        return;
    }
    auto idx = t2_next_bucket_idx;
    for(int c = 2; c--;)
    {
        if(idx < t2.bucket_count())
        {
            handle h;
            for(;idx < t2.bucket_count();){
                h = t2.find_not_empty_bucket(idx);
                if(h) break;
                else idx++;
            }
            if(idx >= t2.bucket_count())
               continue;
            value_type *v;
            auto hash = h.bucket_data().get_hash();
            t2.deprecate_(v, h, idx);
            assert(v != nullptr);
            t1.add_data_ptr(v, hash);
            //assert(t2.size() == table_4_size - 2 + c);
        }
        else if(!t2.empty())
        {
            value_type v;
            t2.deprecate_in_overflow_list(v, t2.overflow_list.begin());
            t1.insert(std::move(v));
            //assert(t2.size() == table_4_size - 2 + c);
        }
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
