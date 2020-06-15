
#include<benchmark/ht_benchmark.h>
#include<utils/constants.h>
#include<utils/random_generator.h>
#include<optional>
#include<string>
#include<utils/statistic.h>
#if !_GLIBCXX_USE_CXX11_ABI
//#error "std::string uses COW"
#endif


int main() {
    using namespace ht_benchmark;
    auto serial_val_arr = generate_serial_val_arr<constants::value_type>(constants::max_size);
    auto rand_val_arr = generate_random_val_arr<constants::value_type>(constants::max_size, 
                                                constants::min, constants::max);
    auto shuffled_val_arr = shuffle_vec(rand_val_arr, 
                                        constants::lower_log_2_bucket_cnt,
                                        constants::upper_log_2_bucket_cnt);
    auto no_exist_val_arr = generate_random_no_exist_val_arr<constants::value_type>(rand_val_arr, 
                                            constants::size_no_exist_elements,
                                            constants::min, constants::max);
    
    constexpr int str_len_8 = 15;
    constexpr int str_len_32 = 32;
    auto str_arr_8 = generate_random_string_arr(constants::max_size, str_len_8);
    auto shuffled_str_arr_8 = shuffle_vec(str_arr_8, 
                                        constants::lower_log_2_bucket_cnt,
                                        constants::upper_log_2_bucket_cnt);
    auto no_exist_str_arr_8 = generate_random_no_exist_string_arr(str_arr_8,
                                    constants::size_no_exist_elements, str_len_8);
//auto str_arr_32 = generate_random_string_arr(constants::max_size, str_len_32);
    //auto no_exist_str_arr_32 = generate_random_no_exist_string_arr(str_arr_32, 
    //                                        constants::size_no_exist_elements, str_len_32);
    try
    {
        //insert_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
        //                                   constants::med_log_2_bucket_cnt,
        //                                   serial_val_arr, "Insert serial val lower" ,
        //                                   "Вставка последовательных чисел");
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
                                          rand_val_arr, no_exist_val_arr, "Erase exist random val lower",
                                          "Удаление случайного числа типа uint64_t, присутствующего в хэш-таблице");
        erase_test<std::string>(constants::lower_log_2_bucket_cnt,
                                constants::med_log_2_bucket_cnt,
                                str_arr_8, no_exist_str_arr_8, "Erase exist str 8 lower",
                                "Удаление случайно сгенерированной 15-символьной строки, присутствующей в хэш-таблице");
        find_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                         constants::med_log_2_bucket_cnt,
                                         rand_val_arr, shuffled_val_arr, "Find exist random val lower",
                                         "Поиск случайного числа типа uint64_t, присутствующего в хэш-таблице");
        find_test<std::string>(constants::lower_log_2_bucket_cnt,
                               constants::med_log_2_bucket_cnt,
                               str_arr_8, shuffled_str_arr_8, "Find exist str 8 lower",
                               "Поиск случайно-сгенерированной 15-символьной строки, присутствующей в хэш-таблице");
        find_test<constants::value_type>(constants::lower_log_2_bucket_cnt,
                                         constants::med_log_2_bucket_cnt,
                                         rand_val_arr, no_exist_val_arr, "Find no exist rand val lower",
                                         "Поиск случайного числа типа uint64_t, отсутвующего в хэш-таблице");
        find_test<std::string>(constants::lower_log_2_bucket_cnt,
                               constants::med_log_2_bucket_cnt,
                               str_arr_8, no_exist_str_arr_8, "Find no exist str 8 lower",
                               "Поиск случайно-сгенерированной 15-символьной строки, отсутвующей в хэш-таблице");
#ifdef STATISTIC
        find_cmp_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                         0.46,
                                         rand_val_arr, rand_val_arr, "Statistic of comparisons of find exist random val med cmp",
                                         "Статистика сравнений при поиске случайного числа типа uint64_t, присутствующего в хэш-таблице ");
        find_cmp_test<std::string>(constants::med_log_2_bucket_cnt,
                               0.46,
                               str_arr_8, str_arr_8, "Statistic of comparisons of find exist str 8 med cmp",
                               "Статистика сравнений при поиске случайно-сгенерированной 15-символьной строки, присутствующей в хэш-таблице ");
        find_cmp_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                         0.46,
                                         rand_val_arr, no_exist_val_arr, "Statistic of comparisons of find no exist rand val med cmp",
                                         "Статистика сравнений при поиске случайного числа типа uint64_t, отсутвующего в хэш-таблице ");
        find_cmp_test<std::string>(constants::med_log_2_bucket_cnt,
                               0.46,
                               str_arr_8, no_exist_str_arr_8, "Statistic of comparisons of find no exist str 8 med cmp",
                               "Статистика сравнений при поиске случайно-сгенерированной 15-символьной строки, отсутвующей в хэш-таблице ");
        find_cmp_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                         0.8,
                                         rand_val_arr, shuffled_val_arr, "Statistic of comparisons of find exist random val med cmp",
                                         "Статистика сравнений при поиске случайного числа типа uint64_t, присутствующего в хэш-таблице ");
        find_cmp_test<std::string>(constants::med_log_2_bucket_cnt,
                               0.8,
                               str_arr_8, shuffled_str_arr_8, "Statistic of comparisons of find exist str 8 med cmp",
                               "Статистика сравнений при поиске случайно-сгенерированной 15-символьной строки, присутствующей в хэш-таблице ");
        find_cmp_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                         0.8,
                                         rand_val_arr, no_exist_val_arr, "Statistic of comparisons of find no exist rand val med cmp",
                                         "Статистика сравнений при поиске случайного числа типа uint64_t, отсутвующего в хэш-таблице ");
        find_cmp_test<std::string>(constants::med_log_2_bucket_cnt,
                               0.8,
                               str_arr_8, no_exist_str_arr_8, "Statistic of comparisons of find no exist str 8 med cmp",
                               "Статистика сравнений при поиске случайно-сгенерированной 15-символьной строки, отсутвующей в хэш-таблице ");
#endif
if(constants::need_extra_test)
        {
            //insert_test<constants::value_type>(constants::med_log_2_bucket_cnt,
            //                                   constants::upper_log_2_bucket_cnt,
            //                                   serial_val_arr, "Insert serial val upper",
            //                                   "Вставка последовательных чисел");
            insert_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                               constants::upper_log_2_bucket_cnt,
                                               rand_val_arr, no_exist_val_arr,  "Insert random val upper",
                                               "Вставка случайных чисел");
            insert_test<std::string>(constants::med_log_2_bucket_cnt,
                                     constants::upper_log_2_bucket_cnt,
                                     str_arr_8, no_exist_str_arr_8, "Insert random string upper",
                                    "Вставка случайно-сгенерированных 15-символьных строк");
            find_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                             constants::upper_log_2_bucket_cnt,
                                             rand_val_arr, shuffled_val_arr, "Find exist random val upper",
                                "Поиск случайного числа типа uint64_t, присутствующего в хэш-таблице");
            find_test<std::string>(constants::med_log_2_bucket_cnt,
                                   constants::upper_log_2_bucket_cnt,
                                   str_arr_8, shuffled_str_arr_8, "Find exist str 8 upper",
                                "Поиск случайно-сгенерированной 15-символьной строки, присутствующей в хэш-таблице");
            find_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                             constants::upper_log_2_bucket_cnt,
                                             rand_val_arr, no_exist_val_arr, "Find no exist rand val upper",
                                         "Поиск случайного числа типа uint64_t, отсутствующего в хэш-таблице");
            find_test<std::string>(constants::med_log_2_bucket_cnt,
                                   constants::upper_log_2_bucket_cnt,
                                   str_arr_8, no_exist_str_arr_8, "Find no exist str 8 upper",
                               "Поиск случайно-сгенерированной 15-символьной строки, отсутвующей в хэш-таблице");
            erase_test<constants::value_type>(constants::med_log_2_bucket_cnt,
                                              constants::upper_log_2_bucket_cnt,
                                              rand_val_arr, no_exist_val_arr,
                                              "Erase exist random val upper",
                                              "Удаление случайного числа типа uint64_t, присутствующего в хэш-таблице");
            erase_test<std::string>(constants::med_log_2_bucket_cnt,
                                    constants::upper_log_2_bucket_cnt,
                                    str_arr_8, no_exist_str_arr_8,
                                    "Erase exist str 8 upper",
                                    "Удаление случайно сгенерированной 15-символьной строки, присутствующей в хэш-таблице");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

