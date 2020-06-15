#include<benchmark/set_benchmark.h>
#include<set/incremental_rehash_hopscotch_set.h>
#include<utils/constants.h>
#include<utils/random_generator.h>
#include<optional>
#include<string>
#if !_GLIBCXX_USE_CXX11_ABI
#error "std::string uses COW"
#endif


int main() {
    auto serial_val_arr = generate_serial_val_arr<constants::value_type>(constants::max_size);
    auto rand_val_arr = generate_random_val_arr<constants::value_type>(constants::max_size, 
                                                constants::min, constants::max);
    //auto no_exist_val_arr = generate_random_no_exist_val_arr<constants::value_type>(rand_val_arr, 
    //                                        constants::max_size / 16,
    //                                        constants::min, constants::max);
  auto no_exist_val_arr = generate_random_val_arr<constants::value_type>(
                                            constants::max_size,
                                            constants::min, constants::max);
    
    constexpr int str_len_8 = 8;
    constexpr int str_len_32 = 32;
    auto str_arr_8 = generate_random_string_arr(constants::max_size, str_len_8);
     auto no_exist_str_arr_8 = generate_random_string_arr(
                                    constants::max_size, str_len_8);
//auto no_exist_str_arr_8 = generate_random_no_exist_string_arr(str_arr_8,
//                                    constants::max_size / 16, str_len_8);
    //auto str_arr_32 = generate_random_string_arr(constants::max_size, str_len_32);
    //auto no_exist_str_arr_32 = generate_random_no_exist_string_arr(str_arr_32, 
    //                                        constants::size_no_exist_elements, str_len_32);
    try
    {
        insert_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                           constants::med_log_2_bucket_cnt,
                                           rand_val_arr, no_exist_val_arr, "Insert random val lower" ,
                                           "Вставка случайного числа типа uint64_t");
        insert_test<std::string>(constants::lower_log_2_bucket_cnt,
                                 constants::med_log_2_bucket_cnt,
                                 str_arr_8, no_exist_str_arr_8, "Insert random str 8 lower",
                                 "Вставка случайно-сгенерированной 15-символьной строки");
        erase_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                           constants::med_log_2_bucket_cnt,
                                           rand_val_arr, no_exist_val_arr, "Erase random val lower" ,
                                           "Удаление случайного числа типа uint64_t");
        erase_test<std::string>(constants::lower_log_2_bucket_cnt,
                                 constants::med_log_2_bucket_cnt,
                                 str_arr_8, no_exist_str_arr_8, "Erase random str 8 lower",
                                 "Удаление случайно-сгенерированной 15-символьной строки");
        find_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                           constants::med_log_2_bucket_cnt,
                                           rand_val_arr, rand_val_arr, "Find random exist val lower" ,
                                           "Поиск случайного числа типа uint64_t, присутствующего в хеш-таблице");
        find_test<std::string>(constants::lower_log_2_bucket_cnt,
                                 constants::med_log_2_bucket_cnt,
                                 str_arr_8, str_arr_8, "Find random exist str 8 lower",
                                 "Поиск случайно-сгенерированной 15-символьной строки, присутствующей в хеш-таблице");
        find_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                           constants::med_log_2_bucket_cnt,
                                           rand_val_arr, no_exist_val_arr, "Find random no exist val lower" ,
                                           "Поиск случайного числа типа uint64_t, отсутствующего в хеш-таблице");
        find_test<std::string>(constants::lower_log_2_bucket_cnt,
                                 constants::med_log_2_bucket_cnt,
                                 str_arr_8, no_exist_str_arr_8, "Find random no exist str 8 lower",
                                 "Поиск случайно-сгенерированной 15-символьной строки, отсутствующей в хеш-таблице");
        rehash_test<constants::value_type>(constants::lower_log_2_bucket_cnt, 
                                           constants::med_log_2_bucket_cnt,
                                           rand_val_arr, "rehash val test",
                                           "Выполнение одной операции рехэширования. Хранимые данные в хеш-таблице - числа типа uint64_t");
        rehash_test<std::string>(constants::lower_log_2_bucket_cnt, 
                                           constants::med_log_2_bucket_cnt,
                                           str_arr_8, "rehash str test",
                                           "Выполнение одной операции рехэширования. Хранимые данные в хеш-таблице - 15-символьные строки типа std::string");
}
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
