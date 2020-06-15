#ifndef _HOPSCOTCH_HASH_TABLE_4_
#define _HOPSCOTCH_HASH_TABLE_4_
#include<type_traits>
#include<cstdint>
#include<functional>
#include<exception>
#include<algorithm>
#include<cassert>
#include<utility>
#include<climits>
#include<memory>
#include<new>
#include<list>
#include<string>
#include"../utils/hash.h"
#include<iostream>
#include<hash_tables/base_bucket.h>
#include<map>
#include<utils/statistic.h>
#include<utils/plf_colony.h>

#if defined(__GNUC__) || defined(__clang__)
#    define LIKELY(exp) (__builtin_expect(!!(exp), true))
#else
#    define LIKELY(exp) (exp)
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define UNLIKELY(exp) (__builtin_expect(!!(exp), false))
#else
#    define UNLIKELY(exp) (exp)
#endif


template<bool NeedStoreHash>
struct hop_4_base_bucket
{
    bool hash_equal(std::size_t ) { return false; }
    void set_hash(std::size_t ) { return; }
    std::size_t get_hash() { return {}; }
};

template<>
struct hop_4_base_bucket<true>
{
    bool hash_equal(std::size_t hash) { return hash == hash_; }
    void set_hash(std::size_t hash) { hash_ = hash; }
    std::size_t get_hash() { return hash_; }
    std::size_t hash_;
};

template <class T, unsigned int NB, class H, class EQ>
class incremental_rehash_hopscotch_set_2;

// hash table for incremental rehash hopscotch set (non-trivial data types)

template<
    class T,
    unsigned int NeighborhoodSize = 30, // Neigborhood
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class hopscotch_hash_table_4 : private Hash, private KeyEqual
{
    static_assert(NeighborhoodSize < 31,
                  "Max size of NeighborhoodSize is 30");
    static constexpr std::size_t NB_RESERVED_BITS = 2;
    friend class incremental_rehash_hopscotch_set_2<T, NeighborhoodSize, Hash, KeyEqual>;
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using nb_bitmask_t = uint32_t;
    using index_type = size_type;
    using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    using bucket_hash_t = uint32_t;
    using hash_t = bucket_hash_t;
    struct bucket
    {
        value_type *data;
        bucket_hash_t hash_;
        nb_bitmask_t nb_info = 0; // probe sequnce number
    public:
        bucket() = default;
        bucket(index_type nb_info_, size_t hash, value_type *&data_ptr)
        {
            construct(hash, data_ptr);
            nb_info = nb_info_;
        }
        bucket(const bucket &b) = delete;
        bucket(bucket &&b) noexcept { swap(b); }
        bucket &operator=(const bucket &b) = delete;
        bucket &operator=(bucket &&b) noexcept { swap(b); return *this; }
        ~bucket() = default;
        const T& operator*() const noexcept { return *get_data(); }
        T& operator*() noexcept { return *get_data(); }
        bool hash_equal(bucket_hash_t hash) { return hash == hash_; }
        void set_hash(bucket_hash_t hash) { hash_ = hash; }
        bucket_hash_t get_hash() { return hash_; }
        auto construct(bucket_hash_t hash, value_type *&data_ptr)
        {
            data = data_ptr;
            this->set_hash(hash);
        }
        auto &get_data() noexcept
        { return data; }
        auto &get_data() const noexcept
        { return data; }
        void swap(bucket &b) noexcept 
        {   
            std::swap(data, b.data);
            std::swap(nb_info, b.nb_info);
        }
        void set_overflow(bool has_overflow)
        {
            if(has_overflow)
                nb_info |= 2;
            else 
                nb_info &= ~2;
        }
        void set_empty(bool is_empty)
        {
            if(is_empty)
                nb_info &= ~1;
            else
                nb_info |= 1;
        }
        void clear()
        {
            set_empty(true);
        }
        void deprecate(value_type *&v)
        {
            std::swap(v, get_data());
           // reinterpret_cast<value_type&>(data).~value_type();
            //get_data()->~value_type();
            set_empty(true);
        }
        bool has_overflow() const { return (nb_info & 2) != 0; }
        bool is_empty() { return (nb_info & 1) == 0; }
        bool is_full() { return (~(nb_info >> 2) & 1) == 0; }
        void toggle_nb_presence(std::size_t bit) noexcept {
            //assert(idx <= NeighborhoodSize);
            nb_info = nb_info ^ (1ull << (bit + NB_RESERVED_BITS));
        }
        bool check_neighbor_presence(std::size_t nb_bit) const noexcept {
            //assert(nb_bit <= NeighborhoodSize);
            if(((nb_info >> (nb_bit + NB_RESERVED_BITS)) & 1) == 1)
                return true;
            return false;
        }
        template<typename... Args>
        void set_value_of_empty_bucket(bucket_hash_t hash, value_type *&data_ptr)
        {
            construct(hash, data_ptr);
            set_empty(false);
        }
        nb_bitmask_t only_nb_info()
        {
            return nb_info >> NB_RESERVED_BITS;
        }
        void swap_empty_bucket(bucket &b)
        {
            std::swap(data, b.data);
            b.set_hash(this->get_hash());
            b.set_empty(false);
            set_empty(true);
        }
    };
private:
    bucket *table = nullptr; // table[]
    plf::colony<value_type> &data_storage;
    std::list<value_type> overflow_list;
    size_type elem_cnt = 0;
    size_type capacity = 0;
    unsigned char log2_bucket_cnt = 0;
#ifdef STATISTIC
    mutable size_type max_compared = 0;
    mutable size_type total_compared = 0;
    mutable size_type find_op_cnt = 0;
    mutable std::map<size_t, size_t> compared_map;
#endif
    bucket *allocate_table(size_type n)
    {
        if(n == 0) return nullptr;
        bucket *m = new bucket [n]{};
        return m;
    }
    void deallocate_table()
    {
        delete[] table;
    }
public:
    hopscotch_hash_table_4() = delete; //: hopscotch_hash_table_2(4){}
    explicit hopscotch_hash_table_4(unsigned char log2_bucket_cnt_, plf::colony<value_type> &storage)
        : table(allocate_table(log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) : 0))
        , data_storage(storage)
        , capacity(log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) : 0)
        , log2_bucket_cnt(log2_bucket_cnt_)
    {}
    hopscotch_hash_table_4(const hopscotch_hash_table_4 &) = delete;
    hopscotch_hash_table_4 &operator=(const hopscotch_hash_table_4 &) = delete;
    ~hopscotch_hash_table_4() { destroy(); }
     void create(unsigned log2_bucket_cnt_)
    {
        assert(!table);
        table = allocate_table(log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_): 0);
        // data_storage.reserve(size_type(1) << log2_bucket_cnt_);
        capacity = (log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_): 0);
        log2_bucket_cnt = log2_bucket_cnt_;
    }
    void destroy()
    {
      //  for (size_t i = 0; i < bucket_count() && !empty() ;i++)
      //  {
      //      if(!table[i].is_empty())
      //      {
      //          table[i].clear();
      //          elem_cnt--;
      //      }
      //  }
        deallocate_table();
        table = nullptr;
    }
    void swap(hopscotch_hash_table_4 &t) noexcept
    {
        std::swap(table, t.table);
        std::swap(elem_cnt, t.elem_cnt);
        std::swap(log2_bucket_cnt, t.log2_bucket_cnt);
        //std::swap(data_storage, t.data_storage);
        std::swap(overflow_list, t.overflow_list);
        std::swap(capacity, t.capacity);
    }
#ifdef STATISTIC
    auto get_map_cmp() const { return compared_map; }
    void reset_operation_count() { compared_map.clear(); total_compared = 0; find_op_cnt = 0; }
    size_type max_cmp_count() const { return max_compared; }
    double average_cmp_count () const { return (double)total_compared / (double)find_op_cnt;  }
#endif
    struct error : public std::logic_error
    {
        explicit error(const char *msg) : std::logic_error(msg) {}
        explicit error(const std::string &msg) : std::logic_error(msg) {}
    };
    struct handle
    {
        const value_type *p = nullptr;
        bucket *b = nullptr;
    public:
        handle() = default;
        handle(value_type &v, bucket &b) : p(&v), b(&b) {}
        explicit handle(bucket &b) : p(b.get_data()), b(&b) {}
        explicit handle(value_type &v) : p(&v) {}
        explicit handle(value_type *v) : p(v) {}
        explicit handle(const value_type *v) : p(v) {}

        explicit operator bool() const { return p; }
        const value_type &operator*() const { return *p; }
        const value_type *operator->() const { return p; }
        const value_type &value() { return *p; }
        bucket &bucket_data() { return *b; }
    };
    explicit operator bool() const { return table; }
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    size_type size() const { return elem_cnt; }
    size_type bucket_count() const { return capacity; }
    bool empty() const { return elem_cnt == 0; } 
    handle find(const T &v) const
    {
        return find_(v ,hash_function()(v));
    }
    handle find(T &&v) const
    {
        return find_(std::move(v) ,hash_function()(v));
    }
    auto insert(const value_type &v) -> std::pair<handle, bool>
    {
        auto hash = static_cast<hash_t>(hash_function()(v));
        if(auto h = find_(v, hash)) return {h, false};
        return {add_bucket(v, hash), true};
    }
    auto insert(value_type &&v) -> std::pair<handle, bool>
    {
        auto hash = static_cast<hash_t>(hash_function()(v));
        if(auto h = find_(v, hash)) return {h, false};
        return {add_bucket(std::move(v), hash), true};
    }
    template<class... Args>
    auto emplace(Args&&... args) -> std::pair<handle, bool>
    {
        value_type v {std::forward<Args>(args)...};
        auto hash = static_cast<hash_t>(hash_function()(v));
        if(auto h = find_(v, hash)) return {h, false};
        return { add_bucket(std::move(v), hash), true };
    }
    size_type erase(const T& v)
    {
        auto hash = static_cast<hash_t>(hash_function()(v));
        return erase_impl(v, hash);
    }
//    size_type erase(T&& v)
//    {
//        size_t hash = hash_function()(v);
//        auto hash_idx = get_index(hash);
//        if(auto it = find_in_table(v, hash, hash_idx))
//        {
//            erase_(it, hash_idx);
//            return 1;
//        }
//        if(table[hash_idx].has_overflow()) 
//        {
//            if(auto it = find_in_overflow_list(v); it != overflow_list.end())
//            {
//                erase_in_overflow_list(it, hash_idx);
//                return 1;
//            }
//        }
//        return 0;
//    }
private:
    handle find_(const value_type &v, hash_t hash) const
    {
        if(UNLIKELY(!table)) return handle{};
        auto idx = get_index(hash);
        auto bucket_hash = static_cast<bucket_hash_t>(hash);
        if(auto it = find_in_table(v, bucket_hash, idx))
        {
            return it;
        }
        if(table[idx].has_overflow())
        {
            // TODO: correct this
            auto it = find_in_overflow_list(v);
            if(it != overflow_list.end())
            {
                return handle{ &(*it) };
            }
        }
        return {};
    }
    handle find_in_table(const value_type &v, bucket_hash_t bucket_hash, index_type idx) const
    {
#ifdef STATISTIC
        find_op_cnt++;
        size_type compared = 0;
#endif
        for(auto info = table[idx].only_nb_info(); info != 0; ++idx)
        {
            if(((info & 1) == 1))
            {
#ifdef STATISTIC
                compared++;
#endif
                if(table[idx].hash_equal(bucket_hash) && key_eq()(v, *table[idx])) 
                {
#ifdef STATISTIC
                    compared_map[compared] += 1;
                    max_compared = std::max(max_compared, compared);
                    total_compared += compared;
#endif
                    return handle{ table[idx] };
                }
            }
            info >>= 1;
        }
#ifdef STATISTIC
       compared_map[compared] += 1;
       max_compared = std::max(max_compared, compared);
       total_compared += compared;
#endif
       return {};
    }

    auto find_in_overflow_list(const value_type &v) const
    {
        auto it = std::find_if(overflow_list.begin(), overflow_list.end(),
               [&](const auto &key){ return key_eq()(key, v); });
#ifdef STATISTIC
       size_t compared = std::distance(overflow_list.begin(), it);
       find_op_cnt++;
       compared_map[compared] += 1;
       max_compared = std::max(max_compared, compared);
       total_compared += compared;
#endif
        return it;
    }

    index_type find_empty_bucket(index_type idx) const
    {
        for(; idx < bucket_count(); ++idx)
            if(table[idx].is_empty())
                return idx;
        return bucket_count();
    }
    auto find_not_empty_bucket(index_type idx) const
    {
        for(auto info = table[idx].only_nb_info(); info != 0; ++idx)
        {
            if(((info & 1) == 1))
            {
                return handle{ table[idx] };
            }
            info >>= 1;
        }
       return handle{};
    }

    bool swap_empty_bucket_closer(std::size_t& empty_idx)
    {
        // assert(empty_idx >= NeighborhoodSize);
        std::size_t start_idx = empty_idx - NeighborhoodSize + 1;
        for(auto check_idx = start_idx; check_idx < empty_idx; check_idx++)
        {
            auto nb_info = table[check_idx].only_nb_info();
            auto swap_idx = check_idx;
            while(nb_info != 0 && swap_idx < empty_idx) {
                // If val isn't empty and belongs to home bucket
                if((nb_info & 1) == 1) {
                    table[swap_idx].swap_empty_bucket(table[empty_idx]);

                    table[check_idx].toggle_nb_presence(empty_idx - check_idx);
                    table[check_idx].toggle_nb_presence(swap_idx - check_idx);

                    empty_idx = swap_idx;
                    return true;
                }
                swap_idx++;
                nb_info >>= 1;
            }
        }
        
        return false;
    }

    template<typename Data>
    handle add_bucket(Data &&val, hash_t hash)
    {
        auto orig_idx = get_index(hash);
        auto bucket_hash = static_cast<bucket_hash_t>(hash);
        std::size_t empty_idx = find_empty_bucket(orig_idx);
        if(empty_idx < bucket_count())
        {
            do 
            { // Empty bucket is in range of NeighborhoodSize, use it
                if(empty_idx - orig_idx < NeighborhoodSize) 
                {
                    auto it = data_storage.emplace(std::forward<Data>(val));
                    value_type *ptr = &(*it);
                    table[empty_idx].set_value_of_empty_bucket(bucket_hash, ptr);
                    table[orig_idx].
                        toggle_nb_presence(empty_idx - orig_idx);
                    ++elem_cnt;
                    return handle{ table[empty_idx] };
                }
            }
            // else, try to swap values to get a closer empty bucket
            while(swap_empty_bucket_closer(empty_idx));
        }
        overflow_list.emplace_back(std::forward<Data>(val));
        table[orig_idx].set_overflow(true);
        ++elem_cnt;
        return handle{ overflow_list.back() };
    }

    handle add_data_ptr(value_type *&val, hash_t hash)
    {
        auto orig_idx = get_index(hash);
        auto bucket_hash = static_cast<bucket_hash_t>(hash);
        std::size_t empty_idx = find_empty_bucket(orig_idx);
        if(empty_idx < bucket_count())
        {
            do 
            { // Empty bucket is in range of NeighborhoodSize, use it
                if(empty_idx - orig_idx < NeighborhoodSize) 
                {
                    table[empty_idx].set_value_of_empty_bucket(bucket_hash, val);
                    table[orig_idx].
                        toggle_nb_presence(empty_idx - orig_idx);
                    ++elem_cnt;
                    return handle{ table[empty_idx] };
                }
            }
            // else, try to swap values to get a closer empty bucket
            while(swap_empty_bucket_closer(empty_idx));
        }
        overflow_list.emplace_back(*val);
        table[orig_idx].set_overflow(true);
        ++elem_cnt;
        return handle{ overflow_list.back() };
    }
    void deprecate_(T *&v, handle h, index_type hash_bucket)
    {
        auto erase_bucket = std::distance(table, h.b);
        h.bucket_data().deprecate(v);
        table[hash_bucket].toggle_nb_presence(erase_bucket - hash_bucket);
        --elem_cnt;
        return;
    }
    template<class BiIterator>
    void deprecate_in_overflow_list(T &v, BiIterator h)
    {
        //for(auto i = 0; i < bucket_count(); i++)
        //    std::cout << "table[" << i <<"]: " << *table[i] << std::endl;
        assert(!overflow_list.empty());
        v = std::move(const_cast<value_type&>(*h));
        overflow_list.erase(h);
        --elem_cnt;
        return;
    }

    size_type erase_impl(const T& v, hash_t hash)
    {
        if(UNLIKELY(!table)) return 0;
        auto hash_idx = get_index(hash);
        auto bucket_hash = static_cast<bucket_hash_t>(hash);
        if(auto it = find_in_table(v, bucket_hash, hash_idx))
        {
            erase_(it, hash_idx);
            return 1;
        }
        if(table[hash_idx].has_overflow()) 
        {
            if(auto it = find_in_overflow_list(v); it != overflow_list.end())
            {
                erase_in_overflow_list(it, hash_idx);
                return 1;
            }
        }
        return 0;
    }
    void erase_(handle h, index_type hash_bucket_idx)
    {
        auto erase_bucket_idx = std::distance(table, h.b);
        auto &erase_bucket = h.bucket_data();
        auto it = data_storage.get_iterator_from_pointer(erase_bucket.get_data());
        data_storage.erase(it);
        erase_bucket.clear();
        table[hash_bucket_idx].toggle_nb_presence(erase_bucket_idx - hash_bucket_idx);
        --elem_cnt;
        return;
    }
    template<class BiIterator>
    void erase_in_overflow_list(BiIterator h, index_type idx)
    {
        overflow_list.erase(h);
        --elem_cnt;
        for(const auto &v : overflow_list)
        {
            auto list_idx = get_index(hash_function()(v));
            if(idx == list_idx) return;
        }
        table[idx].set_overflow(false);
        return;
    }
    index_type get_index(hash_t hash) const
    {
        return (hash * hash_magic) >>
               (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
};

#endif

