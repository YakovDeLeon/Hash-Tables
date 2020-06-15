#ifndef _LINEAR_PROBING_TABLE_
#define _LINEAR_PROBING_TABLE_
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
#include"../utils/hash.h"
#include<iostream>
#include<map>
#include<utils/statistic.h>

template<
    class T,
    bool StoreHash = true,
    bool deleted_cells = false,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class linear_probing_hash_table : private Hash, private KeyEqual
{
    static constexpr bool NeedStoreHash = StoreHash && !(std::is_arithmetic<T>::value);
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using distance_type = int64_t;
    using index_type = size_type;
    using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    using hash_type = ::hash::hash_type;
    enum bucket_state : uint8_t
    {
        empty = 0,
        deleted = 1,
        occupied = 2
    };
    struct bucket : ::hash::base_bucket<NeedStoreHash>
    {
        storage data;
        bucket_state state = empty; // probe sequnce number
    public:
        bucket() = default;
        template<class... Args>
        bucket(bucket_state state_, Args&&... args)
        {
            construct(std::forward<Args>(args)...);
            state = state_;
        }
        bucket(const bucket &b) = delete;
        bucket(bucket &&b) noexcept { swap(b); }
        bucket &operator=(const bucket &b) = delete;
        bucket &operator=(bucket &&b) noexcept { swap(b); return *this; }
        ~bucket() = default;
        template<class... Args>
        auto construct(Args&&... args)
        {
            return ::new(static_cast<void*>(std::addressof(data)))
                value_type(std::forward<Args>(args)...);
        }
        auto *get_data() noexcept
        { return reinterpret_cast<value_type*>(std::addressof(data)); }
        auto *get_data() const noexcept
        { return reinterpret_cast<const value_type*>(std::addressof(data)); }
        const value_type &operator*() const noexcept { return *get_data(); }
        value_type &operator*() noexcept { return *get_data(); }
        void swap(bucket &b) noexcept 
        { std::swap(*get_data(), *b.get_data()); std::swap(state, b.state); }
        void clear()
        {
            get_data()->~value_type();
            state = bucket_state::deleted;
        }
        template<class... Args>
        auto set_value_of_empty_bucket(size_t hash, Args&&... args)
        { construct(std::forward<Args>(args)...); set_occupied(); this->set_hash(hash); }
        void set_occupied() { state = bucket_state::occupied; }
        bool is_empty() const { return state == bucket_state::empty; }
        bool is_deleted() const { return state == bucket_state::deleted; }
        bool is_occupied() const { return state == bucket_state::occupied; }
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
    bucket *allocate_table(size_type n)
    {
        bucket *m = new bucket [n]{};
        return m;
    }
    void deallocate_table()
    {
        delete[] table;
    }
public:
    linear_probing_hash_table() = default;
    explicit linear_probing_hash_table(unsigned char log2_bucket_cnt_)
        : table(allocate_table(size_type(1) << log2_bucket_cnt_))
        , mask(log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) - 1 : 0)
        , log2_bucket_cnt(log2_bucket_cnt_) 
    {}
    linear_probing_hash_table(const linear_probing_hash_table &) = delete;
    linear_probing_hash_table &operator=(const linear_probing_hash_table &) = delete;
    ~linear_probing_hash_table() { destroy(); }
    
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
    size_type size() const { return elem_cnt; }
    size_type bucket_count() const { return size_type(1) << log2_bucket_cnt; }
    bool is_full() const  { return (size_type(1) << log2_bucket_cnt) == elem_cnt; }
    bool is_empty() const { return elem_cnt == 0; }
    void create(unsigned log2_bucket_cnt_)
    {
        assert(!table);
        table = allocate_table(size_type(1) << log2_bucket_cnt_);
        log2_bucket_cnt = log2_bucket_cnt_;
        mask = (log2_bucket_cnt_ ? (size_type(1) << log2_bucket_cnt_) - 1 : 0);
    }
    void destroy()
    {
        for (size_t i = 0; !is_empty() ;i++)
        {
            if(!table[i].is_empty())
            {
                table[i].clear();
                elem_cnt--;
            }
        }
        deallocate_table();
        table = nullptr;
    }
    void randomize_clear(float load_factor)
    {
        assert(load_factor < 1);
        std::size_t step = 1 / load_factor;
        for(std::size_t i = 0; i < bucket_count(); i++)
        {
            if(i % step == 0) continue;
            if(table[i].is_occupied()) table[i].clear();
            else table[i].state = bucket_state::deleted;
            if(elem_cnt > 0) elem_cnt--;
        }
    }
    handle find(const T &v) const
    {
        auto hash = hash_function()(v);
        return find_(v, hash, get_index(hash));
    }
    auto insert(const value_type &v) -> std::pair<handle, bool>
    {
        if(is_full()) return {{}, false};
        auto hash = hash_function()(v);
        auto idx = get_index(hash);
        if(auto h = find_(v, hash,  idx)) return {h, false};
        return {add_bucket(v, hash, idx), true};
    }
    auto insert(value_type &&v) -> std::pair<handle, bool>
    {
        if(is_full()) return {{}, false};
        auto hash = hash_function()(v);
        auto idx = get_index(hash);
        if(auto h = find_(v, hash,  idx)) return {h, false};
        return {add_bucket(std::move(v), hash, idx), true};
    }
    template<class... Args>
    auto emplace(Args&&... args) -> std::pair<handle, bool>
    {
        if(is_full()) return {{}, false};
        value_type v {std::forward<Args>(args)...};
        auto hash = hash_function()(v);
        auto idx = get_index(hash);
        if(auto h = find_(v, hash,  idx)) return {h, false};
        return { add_bucket(std::move(v), hash, idx), true };
    }
    size_type erase(const T& v)
    {
        size_t hash = hash_function()(v);
        auto idx = get_index(hash);
        if(auto it = find_(v, hash,  idx))
        {
            erase_(it);
            return 1;
        }
        return 0;
    }
private:
    handle find_(const T &v, hash_type hash, std::size_t idx) const
    {
#ifdef STATISTIC
        find_op_cnt++;
        size_type compared = 1;
#endif
        if(table[idx].is_empty()) return {};
        if constexpr(NeedStoreHash)
        {
            if(!table[idx].is_deleted())
            {
                if(table[idx].hash_equal(hash) && key_eq()(v, *table[idx]))
                {
#ifdef STATISTIC
                    max_compared = std::max(max_compared, compared);
                    compared_map[compared] += 1;
                    total_compared += compared;
#endif
                    return handle{ table[idx] };
                }    
            }
        }
        else
        {
            if(!table[idx].is_deleted())
            {
                if(key_eq()(v, *table[idx]))
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
        for(auto i = (idx + 1) & mask; i != idx && !table[i].is_empty(); ++i &= mask)
        {
#ifdef STATISTIC
            compared++;
#endif
            if constexpr(NeedStoreHash)
            {
                if(!table[i].is_empty() && !table[i].is_deleted())
                {
                    if(table[i].hash_equal(hash) && key_eq()(v, *table[i]))
                    {
#ifdef STATISTIC
                        compared_map[compared] += 1;
                        max_compared = std::max(max_compared, compared);
                        total_compared += compared;
#endif
                        return handle{ table[i] };
                    }
                }
               
            }
            else
            {
                if(!table[i].is_empty() && !table[i].is_deleted())
                {
                    if(key_eq()(v, *table[i]))
                    {
#ifdef STATISTIC
                        compared_map[compared] += 1;
                        max_compared = std::max(max_compared, compared);
                        total_compared += compared;
#endif
                        return handle{ table[i] };
                    }
                }
            }
        }        
#ifdef STATISTIC
        compared_map[compared] += 1;
        max_compared = std::max(max_compared, compared);
        total_compared += compared;
 #endif
        return {};
    }
    template<typename Data>
    handle add_bucket(Data &&val, std::size_t hash, std::size_t idx)
    {
        for(; table[idx].is_occupied(); ++idx &= mask);
        table[idx].set_value_of_empty_bucket(hash, std::forward<Data>(val));
        ++elem_cnt;
        return handle{ table[idx] };
    }

    void erase_(handle h)
    {
        h.bucket_data().clear();
        --elem_cnt;
        return;
    }
    auto get_index(std::size_t hash) const
    {
        return (hash * hash_magic) >>
            (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
};
#endif // !_ROBIN_HOOD_TABLE_
