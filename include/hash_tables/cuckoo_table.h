#ifndef _CUCKOO_TABLE_
#define _CUCKOO_TABLE_
#include<type_traits>
#include<functional>
#include<exception>
#include<algorithm>
#include<cassert>
#include<utility>
#include<memory>
#include<new>
#include<string>
#include"../utils/hash.h"


static constexpr inline std::size_t a1 = 17;
static constexpr inline std::size_t b1 = 263;
static constexpr inline std::size_t a2 = 37;
static constexpr inline std::size_t b2 = 31;
static constexpr inline std::size_t p = 2803;

template<
    class T,
    std::size_t d = 2,
    std::size_t max_loop = 100,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class cuckoo_hash_table : private Hash, private KeyEqual
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
private:
    template<typename BucketData>
    struct bucket
    {
        BucketData data;
        bool empty = true;
    public:
        bucket() = default;
        template<class... Args>
        bucket(Args&&... args) : 
            data(std::forward<Args>(args)...), empty(false)
        {}
        bucket(const bucket &b) = delete;
        bucket(bucket &&b) noexcept { swap(b); }
        bucket &operator=(const bucket &b) = delete;
        bucket &operator=(bucket &&b) noexcept { swap(b); return *this; }
        ~bucket() = default;
        explicit operator bool() const { return !empty; }
        BucketData &operator*() const { return data; }
        BucketData *operator->() const { return data; }
        void swap(bucket &b) noexcept { std::swap(data, b.data); std::swap(empty, b.empty); }
        void clear() { empty = true; }
        bool is_empty() { return empty; }
    };
    template<typename BucketData>
    struct bucket<BucketData*>
    {
        BucketData *data = nullptr;
    public:
        bucket() = default;
        template<class... Args>
        bucket(BucketData *val) : 
            data(val)
        {}
        bucket(const bucket &b) = delete;
        bucket(bucket &&b) noexcept { swap(b); }
        bucket &operator=(const bucket &b) = delete;
        bucket &operator=(bucket &&b) noexcept { swap(b); return *this; }
        ~bucket() = default;
        explicit operator bool() const { return data; }
        BucketData &operator*() const { return *data; }
        BucketData *operator->() const { return data; }
        void swap(bucket &b) noexcept { std::swap(data, b.data); }
        void clear() { data = nullptr; }
        bool is_empty() { return !data; }
    };
    bucket<value_type> **tables = nullptr; // table[][]
    size_type elem_cnt = 0;
    unsigned char log2_bucket_cnt = 0;
    // std::vector<bucket> stash;
    bucket<value_type> **allocate_table(size_type n)
    {
        assert(n % d == 0);
        bucket<value_type> **m = new bucket<value_type> *[d];
        // m[0] = static_cast<value_type*>(::operator new(sizeof(value_type) * n);
        m[0] = new bucket<value_type>[n]{};
        for(std::size_t i = 1; i != d; ++i)
            m[i] = m[i - 1] + n / d;
        return m;
    }
    void dealocate_tables()
    {
        delete[] tables[0];
        delete[] tables;
    }
public:
    cuckoo_hash_table() = default;
    explicit cuckoo_hash_table(unsigned char log2_bucket_cnt)
        : tables(allocate_table(size_type(1) << log2_bucket_cnt))
        , log2_bucket_cnt(log2_bucket_cnt) 
    {}
    cuckoo_hash_table(const cuckoo_hash_table &) = delete;
    cuckoo_hash_table &operator=(const cuckoo_hash_table &) = delete;
    ~cuckoo_hash_table() { if(tables) dealocate_tables(); }
    
    struct error : public std::logic_error
    {
        explicit error(const char *msg) : std::logic_error(msg) {}
        explicit error(const std::string &msg) : std::logic_error(msg) {}
    };
    struct handle
    {
        value_type *p = nullptr;
        bucket<value_type> *b;
    public:
        handle() = default;
        handle(value_type &v, bucket<value_type> &b) : p(&v), b(&b) {}
        explicit handle(bucket<value_type> &b) : p(&b.data), b(&b) {}

        explicit operator bool() const { return p; }
        value_type &operator*() const { return *p; }
        value_type *operator->() const { return p; }
        value_type &value() { return *p; }
        bucket<value_type> &bucket_data() { return *b; }
    };
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    size_type size() const { return elem_cnt; }
    size_type bucket_count() const { return size_type(1) << log2_bucket_cnt; }
    size_type memory_usage(size_t str_len = 0) const 
    {
        if constexpr(std::is_same<value_type, std::string>::value)
        {
            if(str_len > 8)
                return bucket_count() * (sizeof(bucket<value_type>)) + size() * (str_len + 1) + sizeof(elem_cnt) + sizeof(log2_bucket_cnt);
            else
                return bucket_count() * sizeof(bucket<value_type>) + sizeof(elem_cnt) + sizeof(log2_bucket_cnt);
        }
        else
            return bucket_count() * sizeof(bucket<value_type>) + sizeof(elem_cnt) + sizeof(log2_bucket_cnt);
    }
    handle find(const T &v) const
    {
        return find_(v ,hash_function()(v));
    }
    handle find(T &&v) const
    {
        auto hash = hash_function()(v);
        return find_(std::move(v), hash);
    }
    auto insert(const value_type &v) -> std::pair<handle, bool>
    {
        auto hash = hash_function()(v);
        if(auto h = find_(v, hash)) return {h, false};
        return { add_bucket(v, hash), true };
    }
    auto insert(value_type &&v) -> std::pair<handle, bool>
    {
        auto hash = hash_function()(v);
        if(auto h = find_(v, hash)) return {h, false};
        return { add_bucket(std::move(v), hash), true };
    }
    template<class... Args>
    auto emplace(Args&&... args) -> std::pair<handle, bool>
    {
        value_type v {std::forward<Args>(args)...};
        auto hash = hash_function()(v);
        if(auto h = find_(v, hash)) return {h, false};
        return { add_bucket(std::move(v), hash), true };
    }
    // template<typename M>
    // auto insert_or_assign(const T &v) -> std::pair<handle, bool>
    // {
    //     auto hash = hash_function()(v);
    //     if(auto h = find_(v, hash))
    //     {
    //         *h = v;
    //         return {h, false};
    //     }
    //     return {add_bucket(v, hash), true};
    // }
    // auto insert_or_assign(T &&v) -> std::pair<handle, bool>
    // {
    //     auto hash = hash_function()(v);
    //     if(auto h = find_(v, hash))
    //     {
    //         *h = std::move(v);
    //         return {h, false};
    //     }
    //     return {add_bucket(std::move(v), hash), true};
    // }
    size_type erase(const T &v)
    {
        size_t hash = hash_function()(v);
        if(auto it = find_(v, hash))
        {
            erase_(it);
            return 1;
        }
        return 0;
        
    }

    size_type erase(T &&v)
    {
        size_t hash = hash_function()(v);
        if(auto it = find_(std::move(v), hash))
        {
            erase_(it);
            return 1;
        }
        return 0;
        // size_t hash = hash_function()(v);
        // for(std::size_t i = 0; i < d; ++i)
        // {
        //     auto idx = index(hash, i);
        //     if(tables[i][idx].is_empty()) continue;
        //     if(key_eq()(v, tables[i][idx].data))
        //     {
        //         tables[i][idx].clear();
        //         return 1;
        //     }
        // }
        // return 0;
    }
private:
    handle find_(const T &v, std::size_t hash) const
    {
        for(std::size_t i = 0; i < d; ++i)
        {
            auto idx = index(hash, i);
            if(tables[i][idx].is_empty()) continue;
            if(key_eq()(v, tables[i][idx].data))
            {
                return handle{ tables[i][idx] };
            }
        }
        __asm__ __volatile__("");
        return {};
        // if (!stash.empty())
        //     for (auto it = stash.begin(); it != stash.end(); ++it)
        //         if(key_eq()(v, *it))
        //             return handle{ *it };
    }
    handle find_(const T &&v, std::size_t hash) const
    {
        for(std::size_t i = 0; i < d; ++i)
        {
            auto idx = index(hash, i);
            if(tables[i][idx].is_empty()) continue;
            if(key_eq()(v, tables[i][idx].data))
            {
                return handle{ tables[i][idx] };
            }
        }
        __asm__ __volatile__("");
        return {};
    }
    template<typename Data>
    handle add_bucket(Data &&val, std::size_t hash)
    {
        auto tmp = val;
        bucket<value_type> b {std::forward<Data>(val)};
        auto first_pos = index(hash, 0);
        auto pos = first_pos;
        for(std::size_t i = 0; i < max_loop; ++i) {
            for(std::size_t j = 0; j < d; ++j) {
                b.swap(tables[j][pos]);
                if(!b.is_empty())
                {
                    pos = index(hash_function()(b.data), (j + 1) % d);
                    continue;
                }
                ++elem_cnt;
                if(i == 0 || (key_eq()(tables[0][first_pos].data, val)))
                    return handle{ tables[0][first_pos] };
                else 
                {
                    auto second_pos = index(hash, 1);
                    return handle{tables[1][second_pos]};
                }
                return find_(std::move(tmp), hash);
            }
        }
        throw error(std::string("Need to rehash: load factor: ")
                + std::to_string(size()) + " / " 
                + std::to_string(bucket_count()) + "\n");
        // if(stash.size() == 50)
        //     throw error(std::string("Need to rehash: load factor: ")
        //         + std::to_string(size()) + " / " 
        //         + std::to_string(bucket_count()) + "\n");
        // stash.push_back(b);
        return{};
    }
    void erase_(handle h)
    {
        h.bucket_data().clear();
        --elem_cnt;
    }
    size_type index(std::size_t hash, std::size_t table_id) const
    {
        switch (table_id)
        {
        case 0:
        {
            return (hash * hash_magic) >>
                (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt + d - 1);
            // return ((a1 *hash + b1) % p) % bucket_count();
        }
        case 1:
        {
            static const unsigned all_bits_shift = 12;
            static const unsigned alt_bits_xor = 0x5bd1e995;
            uint32_t tag = hash >> all_bits_shift;
            return (hash ^ ((tag + 1) * alt_bits_xor)) >>
                (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt + d - 1);
            // return ((a2 *hash + b2) % p) % bucket_count();
        }
        default:
            return 0;
        }
    }
};
#endif
