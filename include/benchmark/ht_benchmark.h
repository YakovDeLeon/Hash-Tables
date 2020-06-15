#ifndef _HT_BENCHMARK_
#define _HT_BENCHMARK_
#include<hash_tables/chaining_set.h>
#include<hash_tables/cuckoo_table.h>
#include<hash_tables/robin_hood_table.h>
#include<tsl/robin_set.h>
#include<tsl/hopscotch_set.h>
#include<hash_tables/linear_probing_table.h>
#include<hash_tables/hopscocth_hash_table.h>
#include<hash_tables/hopscocth_hash_table_3.h>
#include<utils/constants.h>
#include<utils/plot.h>
#include<unordered_set>
#include<map>
#include<utils/processor_time.h>
#include<chrono>
#include<iostream>
#include<iomanip> 
#include<vector>
#include<cassert>

namespace ht_benchmark{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Vector>
auto insert_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const Vector &insert_arr, const Vector &insert_arr_2,
             std::initializer_list<float> load_factors = {0.46, 0.6, 0.75, 0.8})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        for(size_t k = 0; k < ResearchCount; k++)
        {
            HashTable table(i);
            if constexpr(std::is_same<HashTable, std::unordered_set<uint64_t>>::value ||
                    std::is_same<HashTable, std::unordered_set<std::string>>::value)
            {
                table.reserve(1 << i);
                table.max_load_factor(1000.0);
            }
            if constexpr(std::is_same<HashTable, tsl::robin_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, true>>::value ||
                    std::is_same<HashTable, tsl::robin_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.min_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }
            if constexpr(std::is_same<HashTable, tsl::hopscotch_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, 30, true>>::value ||
                    std::is_same<HashTable, tsl::hopscotch_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, 30, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }

            if constexpr(std::is_same<HashTable, linear_probing_hash_table<uint64_t, true>>::value ||
                    std::is_same<HashTable, linear_probing_hash_table<std::string, true>>::value)
            {
                table.randomize_clear(0.05);
            }
            size_t max_size = (1 << i);
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            for(auto load_factor : load_factors)
            {
                cycles = 0;
                dur_ms = 0;
                size_t inserted_elem_cnt = max_size * load_factor;
                size_t inserted_elem_cnt_2 = insert_arr.size() == insert_arr_2.size() ?
                                       inserted_elem_cnt : insert_arr_2.size();
                for(size_t j = 0; j < inserted_elem_cnt; j++)
                    table.insert(insert_arr[j]);
                for(size_t j = 0; j < inserted_elem_cnt_2; j++)
                {
                    auto start = std::chrono::high_resolution_clock::now();
                    auto start_time = rdtsc();
                    table.insert(insert_arr_2[j]);
                    auto end_time = rdtsc();
                    auto end = std::chrono::high_resolution_clock::now();
                    table.erase(insert_arr_2[j]);
                    dur_ms += (end - start).count();
                    cycles += end_time - start_time;
                }
              //  if constexpr(std::is_same<HashTable, tsl::hopscotch_set<uint64_t, std::hash<uint64_t>,
             //       std::equal_to<uint64_t>, std::allocator<uint64_t>, 30, true>>::value ||
             //       std::is_same<HashTable, tsl::hopscotch_set<std::string, std::hash<std::string>,
             //       std::equal_to<std::string>, std::allocator<std::string>, 30, true>>::value )
            //{
            //    std::cout << "Size: " << table.size() << " bucket count: " << table.bucket_count() << std::endl;
            //}
                map_ts[inserted_elem_cnt] += dur_ms / inserted_elem_cnt_2;
                map_cycles[inserted_elem_cnt] += cycles / inserted_elem_cnt_2;
            }
        }
    }
    std::for_each(map_ts.begin(), map_ts.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::for_each(map_cycles.begin(), map_cycles.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::vector<double> arr_bucket_count, arr_cycles;
    std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_bucket_count),
                    [](auto &kv){ return kv.first; }); 
    std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_cycles),
                    [](auto &kv){ return kv.second; }); 
    return std::pair{ arr_bucket_count, arr_cycles };
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
void insert_test (std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
                 const std::vector<Val> &insert_vec,
                 const std::vector<Val> &insert_vec_2,
                 const std::string &plot_file_name,
                 const std::string &test_name)
{
    using namespace constants;
    print_test(test_name + "\n");
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере", "Время в циклах процессора для выполнения одной операции", test_name);
    
    auto [elements, ts] = insert_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
    plot_graph(elements, ts, "unordered_set", "red");
//    std::tie(elements, ts) = insert_val_test<tsl::robin_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    plot_graph(elements, ts, "tsl::robin_set", "aqua");
//    std::tie(elements, ts) = insert_val_test<tsl::hopscotch_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, 30, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    plot_graph(elements, ts, "tsl::hopscotch_set");
    std::tie(elements, ts) = insert_val_test<robin_hood_hash_table<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
    plot_graph(elements, ts, "robin_set", "limegreen");
    std::tie(elements, ts) = insert_val_test<chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
    plot_graph(elements, ts, "chaining_set", "orange");
//    std::tie(elements, ts) = insert_val_test<chaining_set<Val, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    plot_graph(elements, ts, "chaining_set (with store hash)", "aqua");
//    //std::tie(elements, ts) = insert_val_test<linear_probing_hash_table<Val>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    //plot_graph(elements, ts, "linear_probing_set", "darkviolet");
//    //std::tie(elements, ts) = insert_val_test<linear_probing_hash_table<Val, true>,
//    //                           research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    //plot_graph(elements, ts, "linear_probing_set with deleted cells", "darkmagenta");
//     std::tie(elements, ts) = insert_val_test<hopscotch_hash_table<Val, 16>,
//                                  research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//      plot_graph(elements, ts, "hopscotch set 16", "blue");
//    //std::tie(elements, ts) = insert_val_test<hopscotch_hash_table<Val, 32>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
//    //plot_graph(elements, ts, "hopscotch_hash_set 32");
    std::tie(elements, ts) = insert_val_test<hopscotch_hash_table<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
    plot_graph(elements, ts, "hopscotch set 30", "black");
    std::tie(elements, ts) = insert_val_test<hopscotch_hash_table_3<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, insert_vec_2);
    plot_graph(elements, ts, "hopscotch set 30 (with data storage)", "blue");

    save_plot("results/ht_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Vector>
auto find_val_test(size_t lower_log2_bucket_cnt, size_t upper_log2_bucket_cnt,
                    const Vector &insert_arr, const Vector &find_arr,
                    std::initializer_list<float> load_factors = {0.46, 0.6, 0.75, 0.8})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        for(size_t k = 0; k < ResearchCount; k++)
        {
            HashTable table(i);
            if constexpr(std::is_same<HashTable, std::unordered_set<uint64_t>>::value ||
                    std::is_same<HashTable, std::unordered_set<std::string>>::value)
            {
                table.reserve(1 << i);
                table.max_load_factor(1000.0);
            }
            if constexpr(std::is_same<HashTable, tsl::robin_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, true>>::value ||
                    std::is_same<HashTable, tsl::robin_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.min_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }
            if constexpr(std::is_same<HashTable, tsl::hopscotch_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, 30, true>>::value ||
                    std::is_same<HashTable, tsl::hopscotch_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, 30, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }


            if constexpr(std::is_same<HashTable, linear_probing_hash_table<uint64_t, true>>::value ||
                    std::is_same<HashTable, linear_probing_hash_table<std::string, true>>::value)
            {
                table.randomize_clear(0.1);
            }
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            size_t insert_idx = 0;
            size_t find_idx = 0;
            size_t max_size = 1 << i;
            for(auto load_factor : load_factors)
            {
                size_t inserted_elem_cnt = max_size * load_factor;
                size_t find_elem_cnt = insert_arr.size() == find_arr.size() ?
                                       inserted_elem_cnt : find_arr.size() * load_factor;
                for(auto j = insert_idx; j < inserted_elem_cnt; j++)
                {
                    table.insert(insert_arr[j]);
                }
                auto start = std::chrono::high_resolution_clock::now();
                auto start_time = rdtsc();
                for(size_t j = 0; j < find_elem_cnt; j++)
                {
                    table.find(find_arr[j]);
                    __asm__ __volatile__("");
                }
                auto end_time = rdtsc();
                auto end = std::chrono::high_resolution_clock::now();
//                std::cout << "insert idx " << insert_idx << " insert_elem cnt " <<inserted_elem_cnt << " diff " << inserted_elem_cnt -insert_idx <<std::endl;
//                std::cout << "find   idx " << find_idx << " find_elem cnt   " << find_elem_cnt << " diff " << find_elem_cnt -find_idx <<std::endl;
                dur_ms = (end - start).count();
                cycles = end_time - start_time;
                map_ts[inserted_elem_cnt] += dur_ms / (find_elem_cnt);
                map_cycles[inserted_elem_cnt] += cycles / (find_elem_cnt);
                find_idx = find_elem_cnt;
                insert_idx = inserted_elem_cnt;
            }
        }
    }
    std::for_each(map_ts.begin(), map_ts.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::for_each(map_cycles.begin(), map_cycles.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::vector<double> arr_bucket_count, arr_cycles;
    std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_bucket_count),
                    [](auto &kv){ return kv.first; }); 
    std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_cycles),
                    [](auto &kv){ return kv.second; }); 
    return std::pair{ arr_bucket_count, arr_cycles };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
void find_test (std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
            const std::vector<Val> &insert_vec, const std::vector<Val> &find_vec,
            const std::string &plot_file_name, const std::string &test_name)
{
    using namespace constants;
    print_test(test_name + "\n");
    auto [elements, ts] = find_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                            insert_vec, find_vec);
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере", "Время в циклах процессора для выполнения одной операции", test_name);
    plot_graph(elements, ts, "unordered_set", "red");
//    std::tie(elements, ts) = find_val_test<tsl::robin_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//                                                insert_vec, find_vec);
//   plot_graph(elements, ts, "tsl::robin_set", "aqua");
//    std::tie(elements, ts) = find_val_test<tsl::hopscotch_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, 30, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//                                                insert_vec, find_vec);
//    plot_graph(elements, ts, "tsl::hopscotch_set");
    std::tie(elements, ts) = find_val_test<robin_hood_hash_table<Val, true>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec);
    plot_graph(elements, ts, "robin_set", "limegreen");
//    std::tie(elements, ts) = find_val_test<robin_hood_hash_table<Val, false>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//                                                insert_vec, find_vec);
//   plot_graph(elements, ts, "robin_set", "aqua");
//std::tie(elements, ts) = find_val_test<chaining_set<Val, false>,
    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
    //                                            insert_vec, find_vec);
    //plot_graph(elements, ts, "chaining_set (without store hash)", "orange");
    std::tie(elements, ts) = find_val_test<chaining_set<Val, true>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec);
    plot_graph(elements, ts, "chaining_set", "orange");
//    std::tie(elements, ts) = find_val_test<linear_probing_hash_table<Val>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//                                                insert_vec, find_vec);
//    plot_graph(elements, ts, "linear_probing_set", "darkviolet");
//    //if(upper_log2_bucket_cnt <= constants::med_log_2_bucket_cnt)
//    //{
//    //    std::tie(elements, ts) = find_val_test<linear_probing_hash_table<Val, true>,
//    //                           research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//    //                                            insert_vec, find_vec);
//    //    plot_graph(elements, ts, "linear_probing_set with deleted cells", "darkmagenta");
//    //}
//    
//    std::tie(elements, ts) = find_val_test<hopscotch_hash_table<Val, 16>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//                                                insert_vec, find_vec);
//    plot_graph(elements, ts, "hopscotch set 16", "blue");
//    //std::tie(elements, ts) = find_val_test<hopscotch_hash_table<Val, 32>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
//    //                                            insert_vec, find_vec);
//    //plot_graph(elements, ts, "hopscotch_hash_set 32");
    std::tie(elements, ts) = find_val_test<hopscotch_hash_table<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec);
    plot_graph(elements, ts, "hopscotch set 30", "black");
    
    std::tie(elements, ts) = find_val_test<hopscotch_hash_table_3<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec);
    plot_graph(elements, ts, "hopscotch set 30 (with data storage)", "blue");
    save_plot("results/ht_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef STATISTIC
template<class HashTable, size_t ResearchCount, class Vector>
auto find_val_cmp_test(size_t log2_bucket_cnt,
                    const Vector &insert_arr, const Vector &find_arr,
                    float load_factor)
{
    size_t insert_idx = 0;
    size_t find_idx = 0;
    size_t max_size = 1 << log2_bucket_cnt;
    HashTable table(log2_bucket_cnt);
    size_t inserted_elem_cnt = max_size * load_factor;
    size_t find_elem_cnt = insert_arr.size() == find_arr.size() ?
                           inserted_elem_cnt : find_arr.size() * load_factor;
    for(auto j = insert_idx; j < inserted_elem_cnt; j++)
        table.insert(insert_arr[j]);
    table.reset_operation_count();
    for(size_t j = 0; j < find_elem_cnt; j++)
    {
        table.find(find_arr[j]);
    __asm__ __volatile__("");
    }
    //std::cout << " insert_elem cnt " << inserted_elem_cnt << " average cmp: " << table.average_cmp_count() << " max cmp:" << table.max_cmp_count()<<std::endl;
//    std::cout << "find   idx " << find_idx << " find_elem cnt   " << find_elem_cnt << " diff " << find_elem_cnt -find_idx <<std::endl;
    auto compared_map = table.get_map_cmp();
    find_idx = find_elem_cnt;
    insert_idx = inserted_elem_cnt;
//    for (auto [x,y] : compared_map)
//        std::cout << "Число сравнений: " << x << " Сколько раз было выполнено: " << y <<std::endl;
    std::vector<double> compared_element_count, compared_count;
    std::transform(compared_map.begin(), compared_map.end(), 
                    std::back_inserter(compared_element_count),
                    [](auto &kv){ return kv.first; }); 
    std::transform(compared_map.begin(), compared_map.end(), 
                    std::back_inserter(compared_count),
                    [](auto &kv){ return kv.second; }); 
    return std::pair{ compared_element_count, compared_count };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
void find_cmp_test (std::size_t log2_bucket_cnt, float load_factor,
            const std::vector<Val> &insert_vec, const std::vector<Val> &find_vec,
            const std::string &plot_file_name, const std::string &test_name)
{
    using namespace constants;
    print_test(test_name + "\n");
    double point_size = 50.0;
    plot_settings(plot_width, plot_height, "Число сравнений при коэффициенте загрузки " + std::to_string(load_factor),
            "Сколько раз было выполнено заданное число сравнений", test_name);
    auto[elements, ts] = find_val_cmp_test<robin_hood_hash_table<Val, true>,
                                research_count>(log2_bucket_cnt,
                                                insert_vec, find_vec, load_factor);
    plot_scatter(elements, ts, point_size, "limegreen");
    std::tie(elements, ts) = find_val_cmp_test<chaining_set<Val, true>,
                                research_count>(log2_bucket_cnt,
                                                insert_vec, find_vec, load_factor);
    plot_scatter(elements, ts, point_size * 3, "orange");
    //std::tie(elements, ts) = find_val_cmp_test<linear_probing_hash_table<Val>,
    //                            research_count>(log2_bucket_cnt,
    //                                            insert_vec, find_vec, load_factor);
    //plot_scatter(elements, ts, point_size, "darkviolet");
    std::tie(elements, ts) = find_val_cmp_test<hopscotch_hash_table<Val, 30>,
                                research_count>(log2_bucket_cnt,
                                                insert_vec, find_vec, load_factor);
    plot_scatter(elements, ts, point_size, "black");
    
    save_plot("results/ht_benchmark/", {plot_file_name + std::to_string(static_cast<size_t>(load_factor*100))});
};
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Vector>
auto erase_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const Vector &insert_arr, const Vector &erase_arr,
             std::initializer_list<float> load_factors = {0.46, 0.6, 0.75, 0.8})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        for(size_t k = 0; k < ResearchCount; k++)
        {
            HashTable table(i);
            if constexpr(std::is_same<HashTable, std::unordered_set<uint64_t>>::value ||
                    std::is_same<HashTable, std::unordered_set<std::string>>::value)
            {
                table.reserve(1 << i);
                table.max_load_factor(1000.0);
            }
            if constexpr(std::is_same<HashTable, tsl::robin_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, true>>::value ||
                    std::is_same<HashTable, tsl::robin_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.min_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }
            if constexpr(std::is_same<HashTable, tsl::hopscotch_set<uint64_t, std::hash<uint64_t>,
                    std::equal_to<uint64_t>, std::allocator<uint64_t>, 30, true>>::value ||
                    std::is_same<HashTable, tsl::hopscotch_set<std::string, std::hash<std::string>,
                    std::equal_to<std::string>, std::allocator<std::string>, 30, true>>::value )
            {
                table.max_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }

            if constexpr(std::is_same<HashTable, linear_probing_hash_table<uint64_t, true>>::value ||
                    std::is_same<HashTable, linear_probing_hash_table<std::string, true>>::value)
            {
                table.randomize_clear(0.05);
            }
            size_t max_size = (1 << i);
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            for(auto load_factor : load_factors)
            {
                cycles = 0;
                dur_ms = 0;
                size_t inserted_elem_cnt = max_size * load_factor;
                size_t erased_elem_cnt = insert_arr.size() == erase_arr.size() ?
                                       inserted_elem_cnt : erase_arr.size();
                for(size_t j = 0; j < inserted_elem_cnt; j++)
                    table.insert(insert_arr[j]);
            //std::cout << "ht  size: " << table.size() << std::endl;
            //std::cout << " erased count : " << erased_elem_cnt << std::endl;
                for(size_t j = 0; j < erased_elem_cnt; j++)
                {
                    auto size = table.size();
                    table.insert(erase_arr[j]);
                    auto start = std::chrono::high_resolution_clock::now();
                    auto start_time = rdtsc();
                    auto erased_count = table.erase(erase_arr[j]);
                    auto end_time = rdtsc();
                    auto end = std::chrono::high_resolution_clock::now();
                    dur_ms += (end - start).count();
                    cycles += end_time - start_time;
                    if(erased_count && table.size() != size) table.insert(erase_arr[j]);
                }
                map_ts[inserted_elem_cnt] += dur_ms / erased_elem_cnt;
                map_cycles[inserted_elem_cnt] += cycles / erased_elem_cnt;
            }
        }
    }
    std::for_each(map_ts.begin(), map_ts.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::for_each(map_cycles.begin(), map_cycles.end(), 
            [](auto &pair_val){ pair_val.second /= ResearchCount; });
    std::vector<double> arr_bucket_count, arr_cycles;
    std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_bucket_count),
                    [](auto &kv){ return kv.first; }); 
std::transform(map_cycles.begin(), map_cycles.end(), 
                    std::back_inserter(arr_cycles),
                    [](auto &kv){ return kv.second; }); 
    return std::pair{ arr_bucket_count, arr_cycles };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
void erase_test (std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
                 const std::vector<Val> &insert_vec,
                 const std::vector<Val> &erase_vec,
                 const std::string &plot_file_name,
                 const std::string &test_name)
{
    using namespace constants;
    print_test(test_name + "\n");
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере", "Время в циклах процессора для выполнения одной операции", test_name);
    
    auto [elements, ts] = erase_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    plot_graph(elements, ts, "unordered_set", "red");
//    std::tie(elements, ts) = erase_val_test<tsl::robin_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    plot_graph(elements, ts, "tsl::robin_set", "aqua");
//    std::tie(elements, ts) = erase_val_test<tsl::hopscotch_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, 30, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    plot_graph(elements, ts, "tsl::hopscotch_set");
    std::tie(elements, ts) = erase_val_test<robin_hood_hash_table<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    plot_graph(elements, ts, "robin_set", "limegreen");
    //std::tie(elements, ts) = erase_val_test<chaining_set<Val, false>,
    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    //plot_graph(elements, ts, "chaining_set (without store hash)", "orange");
    std::tie(elements, ts) = erase_val_test<chaining_set<Val, true>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    plot_graph(elements, ts, "chaining_set", "orange");
//    //std::tie(elements, ts) = erase_val_test<linear_probing_hash_table<Val>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    //plot_graph(elements, ts, "linear_probing_set", "darkviolet");
//    //std::tie(elements, ts) = erase_val_test<linear_probing_hash_table<Val, true>,
//    //                           research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    //plot_graph(elements, ts, "linear_probing_set with deleted cells", "darkmagenta");
//    std::tie(elements, ts) = erase_val_test<hopscotch_hash_table<Val, 16>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    plot_graph(elements, ts, "hopscotch set 16", "blue");
//    //std::tie(elements, ts) = erase_val_test<hopscotch_hash_table<Val, 32>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
//    //plot_graph(elements, ts, "hopscotch_hash_set 32");
    std::tie(elements, ts) = erase_val_test<hopscotch_hash_table<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    plot_graph(elements, ts, "hopscotch set 30", "black");
    std::tie(elements, ts) = erase_val_test<hopscotch_hash_table_3<Val, 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec, erase_vec);
    plot_graph(elements, ts, "hopscotch set 30 (with data storage)", "blue");

    save_plot("results/ht_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
};

#endif // !header guard
