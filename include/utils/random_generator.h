#ifndef _RANDOM_GENERATOR_
#define _RANDOM_GENERATOR_
#include <random>
#include <algorithm>
#include <vector>
#include <string>
#include <initializer_list>
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Vector>
inline Vector shuffle_vec(const Vector &v, size_t lower_log2_bucket_cnt, size_t upper_log2_bucket_cnt,
                     std::initializer_list<float> load_factor_list = {0.46, 0.6, 0.75, 0.8})
{
    Vector shuffled_vec = v;
    std::random_device random_device;
    std::mt19937_64 generator(random_device());
    auto first_it = shuffled_vec.begin();
    for(size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        size_t max_size = 1 << i;
        for (auto load_factor : load_factor_list)
        {
            size_t size = max_size * load_factor;
            auto end_it = shuffled_vec.begin() + size;
            std::shuffle(first_it, end_it, generator);
            first_it = end_it;
        }
    }
    return shuffled_vec;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline T random(T nBegin = 1, T nEnd = 2)
{
    std::random_device random_device;
    std::mt19937_64 generator(random_device());
    if constexpr (std::is_floating_point_v<T>)
    {
        std::uniform_real_distribution<T> distribution(nBegin, nEnd);
        return distribution(generator);
    }
    else
    {
        std::uniform_int_distribution<T> distribution(nBegin, nEnd);
        return distribution(generator);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string random_string( size_t len )
{
    auto randstr = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(len, 0);
    std::generate_n( str.begin(), len, randstr );
    return str;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::vector<T> generate_random_val_arr(std::size_t arr_size, T nBegin = 1, T nEnd = 2)
{
    std::vector<T> arr(arr_size);
    for (size_t i = 0; i < arr_size; i++)
        arr[i] = random(nBegin, nEnd);
    return arr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::vector<T> generate_serial_val_arr(std::size_t arr_size)
{
    std::vector<T> arr(arr_size);
    for (size_t i = 0; i < arr_size; i++)
        arr[i] = i;
    return arr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
inline std::vector<T> generate_random_no_exist_val_arr(const std::vector<T> &arr, std::size_t arr_size, T nBegin = 1, T nEnd = 2)
{
    std::vector<T> no_exist_arr(arr_size);
    for (size_t i = 0; i < arr_size; i++)
    {
        T rand_val;
        do
        {
            rand_val = random(nBegin, nEnd);
        }
        while(std::find(arr.begin(), arr.end(), rand_val) != arr.end());
        no_exist_arr[i] = rand_val;
    }

    return no_exist_arr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline std::vector<std::string> generate_random_string_arr(std::size_t arr_size,
                                                             std::size_t strlen)
{
    std::vector<std::string> arr(arr_size);
    for (size_t i = 0; i < arr_size; i++)
        arr[i] = random_string(strlen);
    return arr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline std::vector<std::string> generate_random_no_exist_string_arr(
                                            const std::vector<std::string> &arr,
                                                           std::size_t arr_size,
                                                             std::size_t strlen)
{
    std::vector<std::string> no_exist_arr(arr_size);
    for (size_t i = 0; i < arr_size; i++)
    {
        std::string rand_str;
        do
        {
            rand_str = random_string(strlen);
        }
        while(std::find(arr.begin(), arr.end(), rand_str) != arr.end());
        no_exist_arr[i] = rand_str;
    }
    return no_exist_arr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif
