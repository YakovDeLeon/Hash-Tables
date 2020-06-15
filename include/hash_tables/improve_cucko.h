#ifndef _IMPROVE_CUCKOO_TABLE_
#define _IMPROVE_CUCKOO_TABLE_
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
static constexpr inline int EMPTY_SLOT = -1;
static constexpr inline int MAX_TRIES = 10;
// Добавить размер для храненя числа бакетов


template<
    class T,
    int HASH_BUCKET_ENTRIES = 8,
    class Hash = std::hash<T>,
    class KeyEqual = std::equal_to<T>
>
class improve_cuckoo_hash_table : private Hash, private KeyEqual
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using hash_sig_t = int32_t;
    using index_t = hash_sig_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
private:
    struct bucket
    {
        hash_sig_t sig_current[HASH_BUCKET_ENTRIES] { EMPTY_SLOT };
        index_t key_idx[HASH_BUCKET_ENTRIES];
        hash_sig_t sig_alt[HASH_BUCKET_ENTRIES] { EMPTY_SLOT };
        char flag[HASH_BUCKET_ENTRIES];
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
        void swap_slot(std::size_t slot_idx, index_t &key_idx,
                       hash_sig_t &cur_sig, hash_sig_t &alt_sig) noexcept
        {
            assert(slot_idx < HASH_BUCKET_ENTRIES); 
            std::swap(data.sig_current[slot_idx], sig);
            std::swap(data.key_idx[slot_idx], key_idx);
            std::swap(data.sig_current[slot_idx], sig);
            std::swap(data.key_idx[slot_idx], key_idx);
        }
        void set_slot(std::size_t slot_idx, hash_sig_t sig, index_t key_idx) 
        {
            assert(slot_idx < HASH_BUCKET_ENTRIES); 
            data.sig_current[slot_idx] = sig;
            data.key_idx[slot_idx] = key_idx;
        }
        void clear(std::size_t slot_idx) 
        {
            assert(slot_idx < HASH_BUCKET_ENTRIES);
            sig_current[slot_idx] = EMPTY_SLOT;
        }
        bool is_empty_slot(std::size_t slot_idx)
        { 
            assert(slot_idx < HASH_BUCKET_ENTRIES);
            return sig_current[slot_idx] == EMPTY_SLOT; }
    };
    using meta_table = std::vector<bucket>;
    using data_table = std::vector<value_type>;
    using free_slots_list = std::list<value_type*>;
    data_table d_table;
    meta_table m_table;
    free_slots_list free_slots; 
    size_type elem_cnt = 0;
    unsigned char log2_bucket_cnt = 0;
public:
    improve_cuckoo_hash_table() = default;
    explicit improve_cuckoo_hash_table(unsigned char log2_bucket_cnt)
        : d_table(size_type(1) << log2_bucket_cnt)
        , m_table((size_type(1) << log2_bucket_cnt) / HASH_BUCKET_ENTRIES, {})
        , free_slots(d_table.begin(), d_table.end())
        , log2_bucket_cnt(log2_bucket_cnt)
    {}
    improve_cuckoo_hash_table(const improve_cuckoo_hash_table &) = delete;
    improve_cuckoo_hash_table &operator=(const improve_cuckoo_hash_table &) = delete;
    ~improve_cuckoo_hash_table() { if(tables) dealocate_tables(); }
    
    struct error : public std::logic_error
    {
        explicit error(const char *msg) : std::logic_error(msg) {}
        explicit error(const std::string &msg) : std::logic_error(msg) {}
    };
    struct handle
    {
        value_type *p = nullptr;
        bucket *b;
    public:
        handle() = default;
        handle(value_type &v, bucket &b) : p(&v), b(&b) {}
        explicit handle(bucket &b) : p(&b.data), b(&b) {}

        explicit operator bool() const { return p; }
        value_type &operator*() const { return *p; }
        value_type *operator->() const { return p; }
        value_type &value() { return *p; }
        bucket &bucket_data() { return *b; }
    };
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    size_type size() const { return elem_cnt; }
    size_type capacity() const { return size_type(1) << log2_bucket_cnt; }
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
    }
private:
    handle find_(const T &v, std::size_t hash) const
    {
        for(std::size_t i = 0; i < d; ++i)
        {
            auto idx = index(hash, i);
            if(tables[i][idx].is_empty()) continue;
            if(key_eq()(v, tables[i][idx].data))
                return handle{ tables[i][idx] };
        }
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
                return handle{ tables[i][idx] };
        }
        return {it, };
    }
    template<typename Data>
    handle add_bucket(Data &&val, std::size_t prime_hash)
    {
        auto tmp = val;
        auto prime_pos = index(prime_hash, j);
        for(size_t i = 0; i < HASH_BUCKET_ENTRIES; ++i)
        {
            if(m_table[prime_pos].is_empty_slot(i))
            {
                auto it = free_slots.pop_front();
                index_t key_idx = it - d_table.begin();
                m_table[prime_pos].swap_slot(i, prime_hash, key_idx);
                d_table.insert(it, std::forward<Data>(val));
                return {it, &m_table[prime_pos] };
            }
        }
        auto alt_hash = alt_hash()(hash);
        auto alt_pos = index()(alt_hash);
        for(size_t i = 0; i < HASH_BUCKET_ENTRIES; ++i)
        {
            if(m_table[alt_pos].is_empty_slot(i))
            {
                auto it = free_slots.pop_front();
                index_t key_idx = it - d_table.begin();
                m_table[alt_pos].swap_slot(i, prime_hash, key_idx);
                d_table.insert(it, std::forward<Data>(val));
                return {it, &m_table[alt_pos] };
            }
        }
        for (size_t k = 0; k < MAX_TRIES; k++)
        {
            for (size_t i = 0; i < HASH_BUCKET_ENTRIES; i++)
            {
                /* code */
            }
            
            auto key_idx = m_table[prime_pos].swap_slot(i, prime_hash, key_idx);
        }
        
        throw error(std::string("Need to rehash: load factor: ")
                + std::to_string(size()) + " / " 
                + std::to_string(capacity()) + "\n");
        // if(stash.size() == 50)
        //     throw error(std::string("Need to rehash: load factor: ")
        //         + std::to_string(size()) + " / " 
        //         + std::to_string(capacity()) + "\n");
        // stash.push_back(b);
        return{};
    }
    static inline int
    make_space_bucket(bucket &bkt, std::size_t prime_hash, unsigned int &nr_pushes)
    {
        int ret;
        auto alt_hash = alt_hash(hash);
        auto alt_idx = index(alt_hash);
        struct rte_hash_bucket *next_bkt[RTE_HASH_BUCKET_ENTRIES];

        /*
        * Push existing item (search for bucket with space in
        * alternative locations) to its alternative location
        */
        unsigned i, j;
        for (i = 0; i < HASH_BUCKET_ENTRIES; i++) {
            /* Search for space in alternative locations */
            for (j = 0; j < HASH_BUCKET_ENTRIES; j++)
                if (m_table[alt_idx].is_empty_slot(j))
                    break;
            if (j != HASH_BUCKET_ENTRIES)
                break;
        }

        /* Alternative location has spare room (end of recursive function) */
        if (i != HASH_BUCKET_ENTRIES) {
            m_table[alt_idx].set_slot(i, alt_hash, j);
            return i;
        }

        /* Pick entry that has not been pushed yet */
        for (i = 0; i < HASH_BUCKET_ENTRIES; i++)
            if (bkt->flag[i] == 0)
                break;

        /* All entries have been pushed, so entry cannot be added */
        if (i == RTE_HASH_BUCKET_ENTRIES || ++(*nr_pushes) > RTE_HASH_MAX_PUSHES)
            return -ENOSPC;

        /* Set flag to indicate that this entry is going to be pushed */
        bkt->flag[i] = 1;

        /* Need room in alternative bucket to insert the pushed entry */
        ret = make_space_bucket(h, next_bkt[i], nr_pushes);
        /*
        * After recursive function.
        * Clear flags and insert the pushed entry
        * in its alternative location if successful,
        * or return error
        */
        bkt->flag[i] = 0;
        if (ret >= 0) {
            next_bkt[i]->sig_alt[ret] = bkt->sig_current[i];
            next_bkt[i]->sig_current[ret] = bkt->sig_alt[i];
            next_bkt[i]->key_idx[ret] = bkt->key_idx[i];
            return i;
        } else
            return ret;

    }
    void erase_(handle h)
    {
        h.bucket_data().clear();
        --elem_cnt;
    }
    size_type alt_hash(std::size_t hash) const
    {
        static const unsigned all_bits_shift = 12;
        static const unsigned alt_bits_xor = 0x5bd1e995;
        uint32_t tag = hash >> all_bits_shift;
        return (hash ^ ((tag + 1) * alt_bits_xor)) >>
            (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
    size_type index(std::size_t hash) const
    {
        return (hash * hash_magic) >>
            (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
};
#endif
