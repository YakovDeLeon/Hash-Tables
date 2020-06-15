#ifndef _CONSTANTS_
#define _CONSTANTS_
#include <cstdint>
#include <limits>

namespace constants
{
using value_type = uint64_t;
constexpr bool need_extra_test = false;

constexpr std::size_t lower_log_2_bucket_cnt = 9;
constexpr std::size_t med_log_2_bucket_cnt = 21;
constexpr std::size_t upper_log_2_bucket_cnt = need_extra_test ?
                                               24 : med_log_2_bucket_cnt;
constexpr std::size_t max_size = need_extra_test ? 
                    (1U << upper_log_2_bucket_cnt) : (1U << med_log_2_bucket_cnt);
constexpr std::size_t size_no_exist_elements = 1024;
constexpr std::size_t min = std::numeric_limits<value_type>::min();
constexpr std::size_t max = std::numeric_limits<value_type>::max();

constexpr std::size_t research_count = 10;
constexpr std::size_t ts_chunks_count = max_size / 1024;

constexpr std::size_t plot_width = 1280;
constexpr std::size_t plot_height = 720;

};


#endif
