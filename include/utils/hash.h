#ifndef _HASH_
#define _HASH_
#include<cstddef>

constexpr inline std::size_t hash_magic{0x9E37'79B9'7F4A'7C15};
inline void hash_append(std::size_t &hash, std::size_t table_id)
{
    hash ^= hash + hash_magic + (table_id << 6) + (table_id >> 2);
}

#endif