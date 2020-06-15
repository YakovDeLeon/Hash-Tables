#ifndef _CHAINING_TABLE_
#define _CHAINING_TABLE_
#include<cstddef>
#include<utility>
#include<tuple>
#include<limits>
#include<memory>
#include<iterator>
#include<cassert>
#include<utils/hash.h>
#include<map>
#include<utils/statistic.h>

//////////////////////////////////////////////////////////////////////////////
template<bool NeedStoreHash>
struct base_node
{
    bool hash_equal(std::size_t ) const { return false; }
    void set_hash(std::size_t ) { return; }
    std::size_t get_hash() const { return {}; }
};

template<>
struct base_node<true>
{
    bool hash_equal(std::size_t hash) const { return hash == hash_; }
    void set_hash(std::size_t hash) { hash_ = hash; }
    std::size_t get_hash() const { return hash_; }
    std::size_t hash_;
};

template<
    class Key,
    bool StoreHash = true,
    class Hash = std::hash<Key>,
    class KeyEqual = std::equal_to<Key>
>
class chaining_set : private Hash, private KeyEqual
{
public:
    using value_type = Key;
    using size_type  = std::size_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
private:
    //static constexpr bool NeedStoreHash = StoreHash;
    static constexpr bool NeedStoreHash = StoreHash && !(std::is_arithmetic<Key>::value);
    struct node : base_node<NeedStoreHash>
    {
        using base = base_node<NeedStoreHash>;
        node *next;
        value_type data;
        template<class... Args>
        node(node *n, Args&&... args)
            : next(n), data(std::forward<Args>(args)...) {}
    };
    struct list
    {
        node *head; // = nullptr; // intialized by new list[n]{}

        list() = default;
        list(const list &) = delete;
        list &operator=(const list &) = delete;
        ~list()
        {
            for(node *p = head; p;)
            {
                node *q = p->next;
                delete p;
                p = q;
            }
        }
        bool empty() const { return !head; }
    };
    class hash_table
    {
        size_type elem_cnt = 0;
        list *buckets = nullptr; // list[]
        static list *allocate_buckets(size_type n) { return new list[n]{}; }
        void deallocate_buckets() { delete [] buckets; }
    public:
        hash_table() = default;
        explicit hash_table(unsigned log2_bucket_cnt)
            : buckets(allocate_buckets(size_type(1) << log2_bucket_cnt)) {}
        hash_table(const hash_table &) = delete;
        hash_table &operator=(const hash_table &) = delete;
        ~hash_table() { if(buckets) deallocate_buckets(); }

        explicit operator bool() const { return buckets; }
        size_type size() const { return elem_cnt; }
        // Using max load factor 0.75
        bool full(unsigned log2_bucket_cnt) const
        {
            return 4 * elem_cnt >= 3 * (size_type(1) << log2_bucket_cnt);
        }

        list &bucket(size_type i) { return buckets[i]; }
        const list &bucket(size_type i) const { return buckets[i]; }
        list *buckets_ptr() { return buckets; }

        template<class... Args>
        value_type &add(list &bucket, size_t hash, Args&&... args)
        {
            auto p = new node(bucket.head, std::forward<Args>(args)...);
            p->set_hash(hash);
            bucket.head = p;
            elem_cnt++;
            return p->data;
        }
        value_type &add_node(list &bucket, node *p)
        {
            p->next = bucket.head;
            bucket.head = p;
            elem_cnt++;
            return p->data;
        }
        node *extract(const Key &key, size_type hash, size_type i, const KeyEqual &eq)
        {
            for(node **p = &bucket(i).head; *p; p = &(*p)->next)
            {
                if constexpr (NeedStoreHash)
                {
                    if((*p)->hash_equal(hash) && eq(key, (*p)->data))
                    {
                        node *q = *p;
                        *p = (*p)->next;
                        elem_cnt--;
                        return q;
                    }
                }
                else
                {
                    if(eq(key, (*p)->data))
                    {
                        node *q = *p;
                        *p = (*p)->next;
                        elem_cnt--;
                        return q;
                    }
                }
            }
            return nullptr;
        }
        node *extract(list &bucket)
        {
            node *p = bucket.head;
            bucket.head = bucket.head->next;
            elem_cnt--;
            return p;
        }
        bool erase(const Key &key, size_type hash, list &bucket, const KeyEqual &eq)
        {
            for(node **p = &bucket.head; *p; p = &(*p)->next)
                if constexpr (NeedStoreHash)
                {
                    if((*p)->hash_equal(hash) && eq(key, (*p)->data))
                    {
                        node *q = *p;
                        *p = (*p)->next;
                        elem_cnt--;
                        delete q;
                        return true;
                    }
                }
                else
                {
                    if(eq(key, (*p)->data))
                    {
                        node *q = *p;
                        *p = (*p)->next;
                        elem_cnt--;
                        delete q;
                        return true;
                    }

                }
            return false;
        }
        template<class Pred>
        size_type erase_if(unsigned log2_bucket_cnt, Pred pred)
        {
            size_type res = 0;
            list *bucket = buckets;
            for(auto c = size_type(1) << log2_bucket_cnt; c--; bucket++)
                for(node **p = &bucket->head; *p;)
                    if(pred(const_cast<const value_type &>((*p)->data)))
                    {
                        node *q = *p;
                        *p = (*p)->next;
                        elem_cnt--;
                        delete q;
                        res++;
                    }
                    else p = &(*p)->next;
            return res;
        }

        void create(unsigned log2_bucket_cnt)
        {
            assert(!buckets);
            buckets = allocate_buckets(size_type(1) << log2_bucket_cnt);
        }
        void destroy()
        {
            assert(elem_cnt == 0);
            deallocate_buckets();
            buckets = nullptr;
            elem_cnt = 0;
        }
        void swap(hash_table &o) noexcept
        {
            std::swap(elem_cnt, o.elem_cnt);
            std::swap(buckets, o.buckets);
        }
    };
    size_type index(std::size_t hash) const
    {
        return hash * hash_magic >>
            (std::numeric_limits<std::size_t>::digits - log2_bucket_cnt);
    }
    hash_table t1;
    unsigned char log2_bucket_cnt;
#ifdef STATISTIC
    mutable size_type max_compared = 0;
    mutable size_type total_compared = 0;
    mutable size_type find_op_cnt = 0;
    mutable std::map<size_t, size_t> compared_map;
#endif
public:
    explicit chaining_set(unsigned );
    chaining_set(const chaining_set &) = delete;
    chaining_set &operator=(const chaining_set &) = delete;
    ~chaining_set();
#ifdef STATISTIC
    auto get_map_cmp() const { return compared_map; }
    void reset_operation_count() { compared_map.clear(); total_compared = 0; find_op_cnt = 0; }
    size_type max_cmp_count() const { return max_compared; }
    double average_cmp_count () const { return (double)total_compared / (double)find_op_cnt;  }
#endif
    struct const_handle
    {
        const value_type *p = nullptr;
        const list *bucket;
    public:
        const_handle() = default;
        const_handle(const value_type &v, const list &b) : p(&v), bucket(&b) {}

        explicit operator bool() const { return p; }
        const value_type &operator*() const { return *p; }
        const value_type *operator->() const { return p; }
    };
    using handle = const_handle;
    const hasher &hash_function() const { return *this; }
    const key_equal &key_eq() const { return *this; }
    std::size_t hash(const Key &key) const { return hash_function()(key); }

    handle find(const Key & , std::size_t );
    const_handle find(const Key & , std::size_t ) const;
    handle find(const Key &key)
    {
        return find(key, hash(key));
    }
    const_handle find(const Key &key) const
    {
        return find(key, hash(key));
    }

    std::pair<handle, bool> insert(const value_type & , std::size_t );
    std::pair<handle, bool> insert(value_type && , std::size_t );
    std::pair<handle, bool> insert(const value_type &v)
    {
        return insert(v, hash(v));
    }
    std::pair<handle, bool> insert(value_type &&v)
    {
        return insert(std::move(v), hash(v));
    }

    void erase(const_handle , size_type );
    size_type erase(const Key & );

    size_type bucket_count() const { return size_type(1) << log2_bucket_cnt; }
    size_type size() const { return t1.size(); }
private:
    handle find_(list &bucket, const Key &key, size_type hash)
    {
#ifdef STATISTIC
        find_op_cnt++;
        size_type compared = 0;
#endif
        for(node *p = bucket.head; p; p = p->next)
        {
#ifdef STATISTIC
            ++compared;
#endif
            if constexpr(NeedStoreHash)
            {
                if(p->hash_equal(hash) && key_eq()(key, p->data))
                {
#ifdef STATISTIC
                    assert(compared != 0);
                    compared_map[compared] += 1;
                    max_compared = std::max(max_compared, compared);
                    total_compared += compared;
#endif
                    return {p->data, bucket};
                }
            }
            else
            {
                if(key_eq()(key, p->data))
                {
#ifdef STATISTIC
                    assert(compared != 0);
                    compared_map[compared] += 1;
                    max_compared = std::max(max_compared, compared);
                    total_compared += compared;
#endif
                    return {p->data, bucket};
                } 
            }
            //__asm__ __volatile__("");
        }
#ifdef STATISTIC
        compared_map[compared] += 1;
        max_compared = std::max(max_compared, compared);
        total_compared += compared;
#endif
        return {};
    }
    const_handle find_(const list &bucket, const Key &key, size_type hash) const
    {
#ifdef STATISTIC
        find_op_cnt++;
        size_type compared = 0;
#endif
        for(node *p = bucket.head; p; p = p->next)
        {
#ifdef STATISTIC
            ++compared;
#endif
            if constexpr(NeedStoreHash)
            {
                if(p->hash_equal(hash) && key_eq()(key, p->data))
                {
#ifdef STATISTIC
                    assert(compared != 0);
                    compared_map[compared] += 1;
                    max_compared = std::max(max_compared, compared);
                    total_compared += compared;
#endif
                    return {p->data, bucket};
                }
            }
            else
            {
                if(key_eq()(key, p->data))
                {
#ifdef STATISTIC
                    assert(compared != 0);
                    compared_map[compared] += 1;
                    max_compared = std::max(max_compared, compared);
                    total_compared += compared;
#endif
                    return {p->data, bucket};
                } 
            }
            //__asm__ __volatile__("");
        }
#ifdef STATISTIC
        compared_map[compared] += 1;
        max_compared = std::max(max_compared, compared);
        total_compared += compared;
#endif
        return {};
    }
    bool full() const { return t1.full(log2_bucket_cnt); }
};
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
chaining_set<K,StoreHash,H,EQ>::chaining_set(unsigned log2)
    : t1(log2), log2_bucket_cnt(log2)
{
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
chaining_set<K,StoreHash,H,EQ>::~chaining_set()
{
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
auto chaining_set<K,StoreHash,H,EQ>::find(
    const K &key, std::size_t hash) -> handle
{
    return find_(t1.bucket(index(hash)), key, hash);
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
auto chaining_set<K,StoreHash,H,EQ>::find(
    const K &key, std::size_t hash) const -> const_handle
{
    return find_(t1.bucket(index(hash)), key, hash);
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
inline auto chaining_set<K,StoreHash,H,EQ>::insert(
    const value_type &v, std::size_t hash) -> std::pair<handle, bool>
{
    auto idx = index(hash);
    list &bucket = t1.bucket(idx);
    if(auto h = find_(bucket, v, hash)) return {h, false};
    auto &el = t1.add(bucket, hash, v);
    return {{el, bucket}, true};
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
inline auto chaining_set<K,StoreHash,H,EQ>::insert(
    value_type &&v, std::size_t hash) -> std::pair<handle, bool>
{
    auto idx = index(hash);
    list &bucket = t1.bucket(idx);
    if(auto h = find_(bucket, v, hash)) return {h, false};
    auto &el = t1.add(bucket, hash, std::move(v));
    return {{el, bucket}, true};
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
void chaining_set<K,StoreHash,H,EQ>::erase(const_handle h, size_type hash)
{
    t1.erase(*h.p, hash, *const_cast<list*>(h.bucket), key_eq());
}
//----------------------------------------------------------------------------
template<class K, bool StoreHash, class H, class EQ>
auto chaining_set<K,StoreHash,H,EQ>::erase(const K &key) -> size_type
{
    size_t hash = hash_function()(key);
    if(t1.erase(key, hash, t1.bucket(index(hash)), key_eq()))
    {
        return 1;
    }
    return 0;
}
//----------------------------------------------------------------------------
#endif
