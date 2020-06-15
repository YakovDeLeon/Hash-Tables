#ifndef _BASE_BUCKET_
#define _BASE_BUCKET_
#include<new>
#include<cstdint>

namespace hash{

template<bool NeedStoreHash>
struct base_bucket
{
    bool hash_equal(std::size_t ) { return false; }
    void set_hash(std::size_t ) { return; }
    std::size_t get_hash() { return {}; }
};

using hash_type = size_t;

template<>
struct base_bucket< true>
{
    bool hash_equal(hash_type hash) { return hash == hash_; }
    void set_hash(hash_type hash) { hash_ = hash; }
    hash_type get_hash() { return hash_; }
    hash_type hash_;
};
};
#endif
