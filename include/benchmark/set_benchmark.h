#ifndef _SET_BENCHMARK_
#define _SET_BENCHMARK_
#include<numeric>
#include<algorithm>
#include<functional>
#include<chrono>
#include<vector>
#include<unordered_set>
#include<tsl/robin_set.h>
#include<tsl/hopscotch_set.h>
#include<set/incremental_rehash_open_set.h>
#include<set/incremental_rehash_chaining_set.h>
#include<set/incremental_rehash_hopscotch_set.h>
#include<set/incremental_rehash_hopscotch_set_2.h>
#include<utils/constants.h>
#include<utils/plot.h>
#include<utils/processor_time.h>
#include<benchmark/ht_benchmark.h>
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Val>
auto rehash_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const std::vector<Val> &insert_arr, float load_factor = 0.75)
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        for(size_t k = 0; k < ResearchCount; k++)
        {
            HashTable table(i);
            if constexpr(std::is_same<HashTable, std::unordered_set<Val>>::value)
            {
                table.reserve(1 << i);
                table.max_load_factor(1000.0);
            }
            if constexpr(std::is_same<HashTable, tsl::robin_set<Val, std::hash<Val>,
                    std::equal_to<Val>, std::allocator<Val>, true>>::value)
            {
                table.max_load_factor(0.75);
                table.min_load_factor(1000.0);
                table.reserve(1 << (i-1));
            }
            if constexpr(std::is_same<HashTable, tsl::hopscotch_set<Val, std::hash<Val>,
                    std::equal_to<Val>, std::allocator<Val>, 30, true>>::value)
            {
                table.max_load_factor(0.75);
                table.reserve(1 << (i-1));
            }

            size_t rehash_size = (1 << i) * load_factor;
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            cycles = 0;
            dur_ms = 0;
            for(size_t j = 0; j < rehash_size; j++)
                    table.insert(insert_arr[j]);
            //    std::cout << "table size: " << table.size() << " bucket count: " << table.bucket_count() << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            auto start_time = rdtsc();
            if constexpr(std::is_same<HashTable, std::unordered_set<Val>>::value)
            {
                    table.rehash(1 << (i+1));
            }
            else
            {
                //assert(((double)table.size() / (double)table.bucket_count()) > 0.7);
                table.insert(insert_arr[rehash_size]);
                __asm__ __volatile__("");
                //assert(table.size() / table.bucket_count() < 0.45);
            }
            auto end_time = rdtsc();
            auto end = std::chrono::high_resolution_clock::now();
            
            //std::cout << "table size: " << table.size() << " bucket count: " << table.bucket_count() << std::endl;
            for(size_t j = 0; j < rehash_size; j++)
            {
                auto h = table.find(insert_arr[j]);
                assert(*h == insert_arr[j]);
                __asm__ __volatile__("");
            }
            dur_ms += (end - start).count();
            cycles += end_time - start_time;
            if constexpr(std::is_same<HashTable, incremental_rehash_hopscotch_set_2<Val>>::value)
            {
                auto start = std::chrono::high_resolution_clock::now();
                auto start_time = rdtsc();
                plf::colony<Val> c (1 << i);
                auto end = std::chrono::high_resolution_clock::now();
                auto end_time = rdtsc();
                dur_ms += (end - start).count();
                cycles += end_time - start_time;
                __asm__ __volatile__("");
            }
            map_ts[table.size()] += dur_ms;
            map_cycles[table.size()] += cycles;
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
    for(size_t k = 0; k < arr_bucket_count.size(); k++)
        std::cout << "Size " << arr_bucket_count[k] << " cycles: " << arr_cycles[k] << std::endl;
    return std::pair{ arr_bucket_count, arr_cycles };
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class Val>
void rehash_test (std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
                 const std::vector<Val> &insert_vec,
                 const std::string &plot_file_name,
                 const std::string &test_name)
{
    using namespace constants;
    print_test(test_name + "\n");
    double point_size = 100;
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере", "Время в циклах процессора для выполнения одной операции", test_name);
    
    auto [elements, ts] = rehash_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    plot_scatter(elements, ts, point_size, "red");
    //std::tie(elements, ts) = rehash_val_test<tsl::robin_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, true>,
     //                           research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    plot_scatter(elements, ts, point_size, "aqua");
//    std::tie(elements, ts) = rehash_val_test<tsl::hopscotch_set<Val, std::hash<Val>, std::equal_to<Val>, std::allocator<Val>, 30, true>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    plot_scatter(elements, ts, point_size, "black");
    //std::tie(elements, ts) = rehash_val_test<incremental_rehash_open_set<Val>,
    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    //plot_scatter(elements, ts, point_size, "limegreen");
    std::tie(elements, ts) = rehash_val_test<incremental_rehash_hopscotch_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    plot_scatter(elements, ts, point_size, "black");
    std::tie(elements, ts) = rehash_val_test<incremental_rehash_hopscotch_set_2<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    plot_scatter(elements, ts, point_size, "blue");
//std::tie(elements, ts) = rehash_val_test<chaining_set<Val, false>,
    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    //plot_graph(elements, ts, "chaining_set (without store hash)", "orange");
    std::tie(elements, ts) = rehash_val_test<incremental_rehash_chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
    plot_scatter(elements, ts, point_size, "orange");
//    //std::tie(elements, ts) = rehash_val_test<linear_probing_hash_table<Val>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    //plot_graph(elements, ts, "linear_probing_set", "darkviolet");
//    //std::tie(elements, ts) = rehash_val_test<linear_probing_hash_table<Val, true>,
//    //                           research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    //plot_graph(elements, ts, "linear_probing_set with deleted cells", "darkmagenta");
//    std::tie(elements, ts) = rehash_val_test<hopscotch_hash_table<Val, 16>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    plot_graph(elements, ts, "hopscotch set 16", "blue");
//    //std::tie(elements, ts) = rehash_val_test<hopscotch_hash_table<Val, 32>,
//    //                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    //plot_graph(elements, ts, "hopscotch_hash_set 32");
//    std::tie(elements, ts) = rehash_val_test<hopscotch_hash_table<Val, 30>,
//                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt, insert_vec);
//    plot_graph(elements, ts, "hopscotch set 30", "black");
//
    save_plot("results/set_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Val>
auto insert_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const std::vector<Val> &insert_arr, const std::vector<Val> &insert_arr_2,
             std::initializer_list<float> load_factors = {0.375, 0.52, 0.6, 0.73})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for(size_t k = 0; k < ResearchCount; k++)
    {
    HashTable table(lower_log2_bucket_cnt);
    size_t last_inserted_index = 0;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        if constexpr(std::is_same<HashTable, std::unordered_set<Val>>::value == true)
        {
            table.rehash(1 << i);
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

        size_t max_size = (1 << i);
        for(auto load_factor : load_factors)
        {
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            size_t inserted_elem_cnt = max_size * load_factor;
            size_t inserted_elem_cnt_2 = load_factor == 0.375 ?
                                                max_size / 16 :
                                                insert_arr_2.size();
            for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
                table.insert(insert_arr[j]);
            last_inserted_index = inserted_elem_cnt;
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
            map_ts[inserted_elem_cnt] += dur_ms / inserted_elem_cnt_2;
            map_cycles[inserted_elem_cnt] += cycles / inserted_elem_cnt_2;
            //std::cout << "size: " << inserted_elem_cnt << " cycles: " <<  map_cycles[inserted_elem_cnt] << std::endl;
        }
        size_t inserted_elem_cnt = max_size * 0.76;
        for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
            table.insert(insert_arr[j]);
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
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                            insert_vec, insert_vec_2);
    plot_graph(elements, ts, "unordered_set", "red");
    if constexpr(std::is_arithmetic<Val>::value)
    {
        std::tie(elements, ts) = insert_val_test<incremental_rehash_hopscotch_set<Val , 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");
    }
    else
    {
        std::tie(elements, ts) = insert_val_test<incremental_rehash_hopscotch_set_2<Val , 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");
    }
    std::tie(elements, ts) = insert_val_test<incremental_rehash_chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2);
    plot_graph(elements, ts, "incremental chaining set", "gold");
    std::tie(elements, ts) = ht_benchmark::insert_val_test<chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2, {0.375, 0.52, 0.6, 0.73});
    plot_graph(elements, ts, "chaining_set", "orange");
    if constexpr(std::is_arithmetic<Val>::value)
    {
        std::tie(elements, ts) = ht_benchmark::insert_val_test<hopscotch_hash_table_2<Val , 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2, {0.375, 0.52, 0.6, 0.73});
        plot_graph(elements, ts, "hopscotch_set", "black");
    }
    else
    {
        std::tie(elements, ts) = ht_benchmark::insert_val_test<hopscotch_hash_table_3<Val , 30>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, insert_vec_2,{0.375, 0.52, 0.6, 0.73});
        plot_graph(elements, ts, "hopscotch_set", "black");
    }
    save_plot("results/set_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Val>
auto erase_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const std::vector<Val> &insert_arr, const std::vector<Val> &erase_arr,
             std::initializer_list<float> load_factors = {0.375, 0.52, 0.6, 0.73})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    for(size_t i = 0; i < ResearchCount; i++)
    {
    HashTable table(lower_log2_bucket_cnt);
    size_t last_inserted_index = 0;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        if constexpr(std::is_same<HashTable, std::unordered_set<Val>>::value == true)
        {
            table.rehash(1 << i);
            table.max_load_factor(1000.0);
        }
        if constexpr(std::is_same<HashTable, tsl::robin_set<Val, std::hash<Val>,
                std::equal_to<Val>, std::allocator<Val>, true>>::value)
        {
            table.max_load_factor(1000.0);
            table.min_load_factor(1000.0);
            table.reserve(1 << (i-1));
        }
        if constexpr(std::is_same<HashTable, tsl::hopscotch_set<Val, std::hash<Val>,
                std::equal_to<Val>, std::allocator<Val>, 30, true>>::value)
        {
            table.max_load_factor(1000.0);
            table.reserve(1 << (i-1));
        }

        size_t max_size = (1 << i);
        for(auto load_factor : load_factors)
        {
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            size_t inserted_elem_cnt = max_size * load_factor;
            size_t erased_elem_cnt =  load_factor == 0.375 ?
                                             max_size / 16 :
                                          erase_arr.size();
            for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
                table.insert(insert_arr[j]);
            last_inserted_index = inserted_elem_cnt;
            //std::cout << "set  size: " << table.size() << std::endl;
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
                //if(erased_count && table.size() != size) table.insert(erase_arr[j]);
                dur_ms += (end - start).count();
                cycles += end_time - start_time;
            }
            map_ts[inserted_elem_cnt] += dur_ms / erased_elem_cnt;
            map_cycles[inserted_elem_cnt] += cycles / erased_elem_cnt;
        }
        size_t inserted_elem_cnt = max_size * 0.76;
        for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
            table.insert(insert_arr[j]);
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
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере",
                 "Время в циклах процессора для выполнения одной операции", test_name);
    
    auto [elements, ts] = erase_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                            insert_vec, erase_vec);
    plot_graph(elements, ts, "unordered_set", "red");
    if constexpr(std::is_arithmetic<Val>::value)
    {
        std::tie(elements, ts) = erase_val_test<incremental_rehash_hopscotch_set<Val , 30>,
                                    research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                     insert_vec, erase_vec);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");
    }
    else
    {
        std::tie(elements, ts) = erase_val_test<incremental_rehash_hopscotch_set_2<Val, 30>,
                                    research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                    insert_vec, erase_vec);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");

    }
    std::tie(elements, ts) = erase_val_test<incremental_rehash_chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, erase_vec);
    plot_graph(elements, ts, "incremental chaining set", "gold");
    std::tie(elements, ts) = ht_benchmark::erase_val_test<chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                 insert_vec, erase_vec, {0.375, 0.52, 0.6, 0.73});
    plot_graph(elements, ts, "chaining_set", "orange");
    if constexpr(std::is_arithmetic<Val>::value)
    {
        std::tie(elements, ts) = ht_benchmark::erase_val_test<hopscotch_hash_table<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, erase_vec, {0.375, 0.52, 0.6, 0.73});
        plot_graph(elements, ts, "hopscotch_set", "black");
    }
    else
    {
        std::tie(elements, ts) = ht_benchmark::erase_val_test<hopscotch_hash_table_3<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, erase_vec, {0.375, 0.52, 0.6, 0.73});
        plot_graph(elements, ts, "hopscotch_set", "black");

    }
       // std::tie(elements, ts) = ht_benchmark::erase_val_test<std::unordered_set<Val>,
       //                         research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
       //                                         insert_vec, erase_vec, {0.375, 0.52, 0.6, 0.73});
    //plot_graph(elements, ts, "un set", "aqua");
save_plot("results/set_benchmark/", plot_file_name);
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template<class HashTable, size_t ResearchCount, class Val>
auto find_val_test(std::size_t lower_log2_bucket_cnt, std::size_t upper_log2_bucket_cnt,
             const std::vector<Val> &insert_arr, const std::vector<Val> &find_arr,
             std::initializer_list<float> load_factors = {0.375, 0.52, 0.6, 0.73})
{
    std::map<double, double> map_cycles;
    std::map<double, double> map_ts;
    HashTable table(lower_log2_bucket_cnt);
    size_t last_inserted_index = 0;
    for (size_t i = lower_log2_bucket_cnt; i < upper_log2_bucket_cnt; i++)
    {
        if constexpr(std::is_same<HashTable, std::unordered_set<Val>>::value == true)
        {
            table.rehash(1 << i);
            table.max_load_factor(1000.0);
        }
        if constexpr(std::is_same<HashTable, tsl::robin_set<Val, std::hash<Val>,
                std::equal_to<Val>, std::allocator<Val>, true>>::value)
        {
            table.max_load_factor(1000.0);
            table.min_load_factor(1000.0);
            table.reserve(1 << (i-1));
        }
        if constexpr(std::is_same<HashTable, tsl::hopscotch_set<Val, std::hash<Val>,
                std::equal_to<Val>, std::allocator<Val>, 30, true>>::value)
        {
            table.max_load_factor(1000.0);
            table.reserve(1 << (i-1));
        }

        size_t max_size = (1 << i);
        for(auto load_factor : load_factors)
        {
            unsigned long long cycles = 0;
            unsigned long long dur_ms = 0;
            size_t inserted_elem_cnt = max_size * load_factor;
            size_t inserted_elem_cnt_2 = inserted_elem_cnt;
            for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
                table.insert(insert_arr[j]);
            last_inserted_index = inserted_elem_cnt;
            auto start = std::chrono::high_resolution_clock::now();
            auto start_time = rdtsc();
            for(size_t j = 0; j < inserted_elem_cnt_2; j++)
            {
                table.find(find_arr[j]);
                __asm__ __volatile__("");
            }
            auto end_time = rdtsc();
            auto end = std::chrono::high_resolution_clock::now();
            for(size_t j = 0; j < inserted_elem_cnt_2; j++)
            {
                if constexpr (std::is_same<HashTable, std::unordered_set<Val>>::value)
                {
                    if(auto h = table.find(find_arr[j]); h != table.end())
                    {

                        table.erase(find_arr[j]);
                        table.insert(find_arr[j]);
                    }
                    else
                    {
                        table.insert(find_arr[j]);
                        table.erase(find_arr[j]);
                    }

                }
                else
                {
                    if(auto h = table.find(find_arr[j]))
                    {

                        table.erase(find_arr[j]);
                        table.insert(find_arr[j]);
                    }
                    else
                    {
                        table.insert(find_arr[j]);
                        table.erase(find_arr[j]);
                    }
                }
            }
            dur_ms += (end - start).count();
            cycles += end_time - start_time;
            map_ts[inserted_elem_cnt] += dur_ms / inserted_elem_cnt_2;
            map_cycles[inserted_elem_cnt] += cycles / inserted_elem_cnt_2;
        }
        size_t inserted_elem_cnt = max_size * 0.76;
        for(size_t j = last_inserted_index; j < inserted_elem_cnt; j++)
            table.insert(insert_arr[j]);
    }
    std::for_each(map_ts.begin(), map_ts.end(), 
            [](auto &pair_val){ pair_val.second /= 1; });
    std::for_each(map_cycles.begin(), map_cycles.end(), 
            [](auto &pair_val){ pair_val.second /= 1; });
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
                 const std::vector<Val> &insert_vec,
                 const std::vector<Val> &find_vec,
                 const std::string &plot_file_name,
                 const std::string &test_name)
{
     using namespace constants;
    print_test(test_name + "\n");
    plot_settings(plot_width, plot_height, "Количество элементов в контейнере",
            "Время в циклах процессора для выполнения одной операции", test_name);
    
    auto [elements, ts] = find_val_test<std::unordered_set<Val>,
                            research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                            insert_vec, find_vec);
    plot_graph(elements, ts, "unordered_set", "red");
    if constexpr (std::is_arithmetic<Val>::value)
    {
        std::tie(elements, ts) = find_val_test<incremental_rehash_hopscotch_set<Val , 30>,
                                    research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                    insert_vec, find_vec);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");
    }
    else
    {
        std::tie(elements, ts) = find_val_test<incremental_rehash_hopscotch_set_2<Val , 30>,
                                    research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                    insert_vec, find_vec);
        plot_graph(elements, ts, "incremental hopscotch_set", "blue");

    }
    std::tie(elements, ts) = find_val_test<incremental_rehash_chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec);
    plot_graph(elements, ts, "incremental chaining set", "gold");
    std::tie(elements, ts) = ht_benchmark::find_val_test<chaining_set<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec, {0.375, 0.52, 0.6, 0.73});
    plot_graph(elements, ts, "chaining_set", "orange");
    std::tie(elements, ts) = ht_benchmark::find_val_test<hopscotch_hash_table_3<Val>,
                                research_count>(lower_log2_bucket_cnt, upper_log2_bucket_cnt,
                                                insert_vec, find_vec, {0.375, 0.52, 0.6, 0.73});
    plot_graph(elements, ts, "hopscotch_set", "black");
   save_plot("results/set_benchmark/", plot_file_name);
}
#endif // !header guard
