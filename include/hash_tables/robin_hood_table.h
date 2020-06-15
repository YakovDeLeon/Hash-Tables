#ifndef _ROBIN_HOOD_TABLE_
#define _ROBIN_HOOD_TABLE_
#include <limits>
#include<type_traits>
#include<cstdint>
#include<functional>
#include<exception>
#include<algorithm>
#include<cassert>
#include<utility>
#include<memory>
#include<new>
#include<string>
#include<hash_tables/base_bucket.h>
#include<utils//hash.h>
#include<iostream>
#include<map>
#include<utils/statistic.h>

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

template <class T, bool StoreHash, class H, class EQ>
class incremental_rehash_open_set;

template<
    class T,
    bool  StoreHash = true,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class robin_hood_hash_table : private Hash, private KeyEqual
{
    static constexpr bool NeedStoreHash = StoreHash && !(std::is_arithmetic<T>::value);
    friend class incremental_rehash_open_set<T, StoreHash, Hash, KeyEqual>;
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using distance_type = int16_t;
    using index_type = size_type;
    using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    using hash_type = ::hash::hash_type;
    struct bucket : ::hash::base_bucket<NeedStoreHash>
    {
       using base = ::hash::base_bucket<NeedStoreHash>;
       storage data;
       distance_type psl = -1; // probe sequnce number
    public:
        bucket() = default;
        template<class... Args>
        bucket(distance_type psl_, hash_type hash, Args&&... args)
        {
            construct(psl_, hash, std::forward<Args>(args)...);
            
        }
        bucket(const bucket &b) = delete;
        bucket(bucket &&b) noexcept { swap(b); }
        ~bucket() = default;
        const T& operator*() const noexcept { return *get_data(); }
        T& operator*() noexcept { return *get_data(); }
        template<class... Args>
        void construct(distance_type psl_, hash_type hash, Args&&... args)
        {
            ::new(static_cast<void*>(std::addressof(data)))
                value_type(std::forward<Args>(args)...);
            psl = psl_;
            if constexpr(NeedStoreHash)
                this->set_hash(hash);
        }
//const auto *get_data() const 
        //{ return std::launder(reinterpret_cast<const T*>(&data));}
        auto *get_data() noexcept
        { return reinterpret_cast<value_type*>(std::addressof(data)); }
        auto *get_data() const noexcept
        { return reinterpret_cast<const value_type*>(std::addressof(data)); }
        void swap(bucket &b) noexcept 
        { 
            std::swap(*get_data(), *b.get_data());
            std::swap(psl, b.psl);
            if constexpr(NeedStoreHash)
                std::swap(this->hash_, b.hash_);
        }
        void clear()
        {
            get_data()->~value_type();
            psl = -1;
        }
        void deprecate(value_type &v)
        {
            std::swap(v, *get_data());
           // reinterpret_cast<value_type&>(data).~value_type();
            get_data()->~value_type();
            psl = std::numeric_limits<distance_type>::max();
        }
        bool is_empty() { return psl == -1; }
        bool is_deprecated()
        { return psl == std::numeric_limits<distance_type>::max(); }
    };
private:
    bucket *table = nullptr; // table[]
    size_type elem_cnt = 0;
    size_type mask = 0;
    unsigned char log2_bucket_cnt = 0;
#ifdef STATISTIC
    mutable size_type max_compared = 0;
    mutable size_type total_compared = 0;
    mutable size_type find_op_cnt = 0;
    mutable std::map<size_t, size_t> compared_map;
#endif
    static bucket *allocate_table(size_type n)
    {
        return new bucket [n]{};
    }
    void deallocate_table()
    {
        delete[] table;
    }
public:
    robin_hood_hash_table() = default;
    explicit robin_hood_hash_table(unsigned char log2_bucket_cnt_)
        : table(allocate_table(size_type(1) << log2_bucket_cnt_))
        , mask(log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) - 1 : 0)
        , log2_bucket_cnt(log2_bucket_cnt_) 
    {}
    robin_hood_hash_table(const robin_hood_hash_table &) = delete;
    robin_hood_hash_table &operator=(const robin_hood_hash_table &) = delete;
    ~robin_hood_hash_table() { destroy(); }
#ifdef STATISTIC
    void reset_operation_count() { compared_map.clear(); total_compared = 0; find_op_cnt = 0; }
    auto get_map_cmp() const { return compared_map; }
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
        value_type *p = nullptr;
        bucket *b = nullptr;
    public:
        handle() = default;
        handle(value_type &v, bucket &b) : p(&v), b(&b) {}
        explicit handle(bucket &b) : p(b.get_data()), b(&b) {}

        explicit operator bool() const { return p; }
        value_type &operator*() const { return *p; }
        value_type *operator->() const { return p; }
        value_type &value() { return *p; }
        bucket &bucket_data() { return *b; }
    };
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    void create(unsigned log2_bucket_cnt_)
    {
        assert(!table);
        table = allocate_table(size_type(1) << log2_bucket_cnt_);
        log2_bucket_cnt = log2_bucket_cnt_;
        mask = (log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) - 1 : 0);
    }
    void destroy()
    {
        for (size_t i = 0; !empty() ;i++)
        {
            if(!table[i].is_empty() && !table[i].is_deprecated())
            {
                table[i].clear();
                elem_cnt--;
            }
        }
        deallocate_table();
        table = nullptr;
    }
    void swap(robin_hood_hash_table &t) noexcept
    {
        std::swap(table, t.table);
        std::swap(elem_cnt, t.elem_cnt);
        std::swap(mask, t.mask);
        std::swap(log2_bucket_cnt, t.log2_bucket_cnt);
    }
    size_type size() const { return elem_cnt; }
    bool empty() const { return elem_cnt == 0; }
    explicit operator bool() const { return table; }
    size_type bucket_count() const 
    { return log2_bucket_cnt ? size_type(1) << log2_bucket_cnt : 0; }
    handle find(const T &v) const
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        return find_(v, hash, idx);
    }
    handle find(T &&v) const
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        return find_(v, hash, idx);
    }
    auto insert(const value_type &v) -> std::pair<handle, bool>
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        auto ideal_idx = idx;
        if(auto h = find_(v, hash, idx)) return {h, false};
        distance_type psl = (idx - ideal_idx) & mask;
        return {add_bucket(v, hash, std::move(idx), psl), true};
    }
    auto insert(value_type &&v) -> std::pair<handle, bool>
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        auto ideal_idx = idx;
        if(auto h = find_(v, hash, idx)) return {h, false};
        distance_type psl = (idx - ideal_idx) & mask;
        return {add_bucket(std::move(v), hash, std::move(idx), psl), true};
    }
    template<class... Args>
    auto emplace(Args&&... args) -> std::pair<handle, bool>
    {
        value_type v {std::forward<Args>(args)...};
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        auto ideal_idx = idx;
        if(auto h = find_(v, hash, idx)) return {h, false};
        distance_type psl = (idx - ideal_idx) & mask;
        return { add_bucket(std::move(v), hash, std::move(idx), psl), true };
    }

    size_type erase(const T& v)
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        if(auto it = find_(v, hash, idx))
        {
            erase_(it, std::move(idx));
            return 1;
        }
        return 0;
    }
    size_type erase(T&& v)
    {
        auto hash =  hash_function()(v);
        auto idx = get_index(hash);
        if(auto it = find_(v, hash, idx))
        {
            erase_(it, std::move(idx));
            return 1;
        }
        return 0;
    }
private:
    bool less_half_full() const
    {
        return elem_cnt < (size_t(1) << (log2_bucket_cnt - 1));
    }
    handle find_(const T &v, hash_type hash, size_t &idx) const
    {
        if(UNLIKELY(!table)) return handle{};
#ifdef STATISTIC
        find_op_cnt++;
        size_type compared = 0;
#endif
        for(distance_type psl = 0; psl++ <= table[idx].psl; ++idx &= mask)
        {
#ifdef STATISTIC
           compared++;
#endif
           if constexpr(NeedStoreHash)
           {
               if(LIKELY(table[idx].hash_equal(hash) && key_eq()(v, *table[idx])))
               {
#ifdef STATISTIC
                   max_compared = std::max(max_compared, compared);
                   total_compared += compared;
                   compared_map[compared] += 1;
#endif
                   return handle{ table[idx] };
               }
           }
           else
           {
               if(LIKELY(key_eq()(v, *table[idx])))
               {
#ifdef STATISTIC
                   max_compared = std::max(max_compared, compared);
                   total_compared += compared;
                   compared_map[compared] += 1;
#endif
                  return handle{ table[idx] };
               }
           }
        }
        __asm__ __volatile__("");
#ifdef STATISTIC
        max_compared = std::max(max_compared, compared);
        total_compared += compared;
        compared_map[compared] += 1;
#endif
        return handle {};
    }
    template<typename Data>
    handle add_bucket(Data &&val, hash_type hash, size_t idx, distance_type psl)
    {
        bucket b {psl, hash, std::forward<Data>(val)};
        table[idx].swap(b);
        handle h { table[idx] };
        while(!b.is_empty())
        {
            for (; b.psl <= table[idx].psl; (++idx) &= mask)
            {
                b.psl++;
            }
            table[idx].swap(b);
        }
        ++elem_cnt;
        return h;
    }

    void erase_(handle h, size_t idx)
    {
        h.b->clear();
        --elem_cnt;
        // need swap next elements
        ++idx &= mask;
        for (; table[idx].psl > 0 /*&& !table[idx].is_empty()*/; (++idx) &= mask)
        {
            auto psl = table[idx].psl - 1;
            size_t prev_idx = (idx - 1) & mask;
            table[prev_idx].construct(psl, table[idx].get_hash(),
                                             std::move(*table[idx]));
            table[idx].clear();
            //table[idx].psl--;
            //size_t prev_idx = (idx - 1) & mask;
            //table[idx].swap(table[prev_idx]);
        }
        return;
    }
    handle find_in_deprecated_table(const value_type &v, size_t hash, size_t &idx)
    {
        for(distance_type psl = 0; psl <= table[idx].psl; ++idx &= mask)
        {
            if constexpr(NeedStoreHash)
            {
                if(!table[idx].is_deprecated() && table[idx].hash_equal(hash) &&
                        key_eq()(v, *table[idx]))
                    return handle{ table[idx] };

            }
            else
            {
                if(!table[idx].is_deprecated() && key_eq()(v, *table[idx]))
                    return handle{ table[idx] };
            }
        }
        __asm__ __volatile__("");
        return {};
    }
    void deprecate_(value_type &v, size_type idx)
    {
        assert(idx < bucket_count());
        table[idx].deprecate(v);
        --elem_cnt;
    }
    void deprecate_(value_type &v, handle h)
    {
        assert(h);
        auto &b = h.bucket_data();
        b.deprecate(v);
        --elem_cnt;
    }
    size_t get_index(std::size_t hash) const
    {
        return
            (hash * hash_magic) >> 
            (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
};
#endif // !_ROBIN_HOOD_TABLE_
