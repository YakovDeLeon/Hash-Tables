#include<hash_tables/cuckoo_table.h>
#include<hash_tables/robin_hood_table.h>
#include<hash_tables/hopscocth_hash_table.h>
#include<hash_tables/hopscocth_hash_table_3.h>
#include<hash_tables/linear_probing_table.h>
#include<hash_tables/chaining_set.h>
#include<tsl/robin_set.h>
#include<utils/test_class.h>
#include<iostream>
#include<vector>
#include<cassert>
#include<utils/random_generator.h>
#include<utils/constants.h>

template<class T>
inline T* allocate_array(size_t arr_size)
{
    return new T [arr_size];
}

template<class T>
inline void deallocate_array(T* arr)
{
    delete[] arr;
}

int main() {
//    size_t log_2_bucket_cnt = 20;
//    size_t max_size = 1 << log_2_bucket_cnt;
//   // auto rand_val_arr = generate_random_val_arr<constants::value_type>(max_size, 
//   //                                             constants::min, constants::max);
//  //  auto shuffled_val_arr = shuffle_vec(rand_val_arr, 
//  //                                      constants::lower_log_2_bucket_cnt,
//  //                                      constants::upper_log_2_bucket_cnt);
//   // auto no_exist_val_arr = generate_random_no_exist_val_arr<constants::value_type>(rand_val_arr, 
//   //                                         constants::size_no_exist_elements,
//   //                                         constants::min, constants::max);
//    
//    constexpr int str_len_8 = 15;
//    auto str_arr_8 = generate_random_string_arr(max_size, str_len_8);
//    //auto shuffled_str_arr_8 = shuffle_vec(str_arr_8, 
//    //                                    constants::lower_log_2_bucket_cnt,
//    //                                    constants::upper_log_2_bucket_cnt);
//    auto no_exist_str_arr_8 = generate_random_no_exist_string_arr(str_arr_8,
//                                    constants::size_no_exist_elements, str_len_8);
//    float load_factor = 0.8;
//    hopscotch_hash_table<std::string, 30> ht_1(10);
//    chaining_set<std::string> ht_2(10);
//    for(size_t i = 0; i < max_size * load_factor; i++)
//    {
//        ht_1.insert(str_arr_8[i]);
//    }
//    for(size_t i = 0; i < max_size * load_factor; i++)
//    {
//        ht_2.insert(str_arr_8[i]);
//    }
//    for(size_t i = 0; i < max_size * load_factor; i++)
//    {
//        ht_1.find(no_exist_str_arr_8[i]);
//        __asm__ __volatile__("");
//    }
//    for(size_t i = 0; i < max_size * load_factor; i++)
//    {
//        ht_2.find(no_exist_str_arr_8[i]);
//        __asm__ __volatile__("");
//    }
//
    static constexpr std::size_t log_2_bucket_cnt = 18;
    static constexpr std::size_t log_2_bucket_cnt_for_user_objects = 4;
    static constexpr std::size_t total_size = 1 << log_2_bucket_cnt;
    static constexpr std::size_t total_size_for_user_objects = 1 << log_2_bucket_cnt_for_user_objects;
    static constexpr std::size_t arr_size = (1 << (log_2_bucket_cnt)) * 0.35;
try
{
    {
        using value_type = int64_t;
        using hash_table = hopscotch_hash_table_3<value_type*>;
        std::cout << "=======================================================\n";
        std::cout << "Tests for base types:\n";
        hash_table table(log_2_bucket_cnt);
        auto ptr1 = std::make_unique<value_type>(1);
        auto ptr2 = std::make_unique<value_type>(1);
        auto ptr3 = std::make_unique<value_type>(1);
        auto ptr4 = std::make_unique<value_type>(1);
        auto ptr5 = std::make_unique<value_type>(1);
        auto ptrs = std::make_unique<std::unique_ptr<value_type>[]>(arr_size);
        for (size_t i = 0; i < arr_size; i++)
        {
            ptrs[i] = std::make_unique<value_type>(1);
        }
        {
            std::cout << "Tests for insert, erase, insert, erase some element in table:\n";
            auto it = table.insert(ptr1.get());
            assert(it.first && it.second == true && *it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size);
            it = table.insert(ptr1.get());
            assert(it.first && it.second == true && *it.first == ptr1.get());
            assert(table.size() == 1);
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size);

            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "=======================================================\n";
            std::cout << "Tests for constructor:\n";
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert new element in empty table:\n";
            auto it = table.insert(ptr1.get());
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert exist element:\n";
            auto it = table.insert(ptr1.get());
            assert(it.second == false);
            assert(it.first);
            assert(*it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert new element in not empty table:\n";
            auto it = table.insert(ptr2.get());
            assert(it.first);
            assert(*it.first == ptr2.get());
            assert(it.second == true);
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert new elements in not empty table:\n";
            auto it = table.insert(ptr3.get());
            assert(it.first && it.second == true && *it.first == ptr3.get());
            it = table.insert(ptr4.get());
            assert(it.first && it.second == true && *it.first == ptr4.get());
            it = table.insert(ptr5.get());
            assert(it.first && it.second == true && *it.first == ptr5.get());
            assert(table.size() == 5);
            assert(table.bucket_count() == total_size);
            assert(1 == table.erase(ptr3.get()));
            assert(1 == table.erase(ptr4.get()));
            assert(1 == table.erase(ptr5.get()));
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for find exist element in not empty table:\n";
            auto it = table.find(ptr2.get());
            assert(it);
            assert(*it == ptr2.get());
            it = table.find(ptr1.get());
            assert(it);
            assert(*it == ptr1.get());
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract exist element in not empty table:\n";
            assert(1 == table.erase(ptr2.get()));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for find no exist element in not empty table:\n";
            auto it = table.find(ptr2.get());
            assert(!it);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract no exist element in not empty table:\n";
            assert(0 == table.erase(ptr2.get()));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract last element in not empty table:\n";
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert some elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                auto it = table.insert(ptrs[i].get());
                assert(it.first && it.second == true && *it.first == ptrs[i].get());
                assert(table.size() == i + 1);
                assert(table.bucket_count() == total_size);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
       {
            std::cout << "Tests for find exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                auto it = table.find(ptrs[i].get());
                assert(it);
                assert(*it == ptrs[i].get());
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for erase some elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                assert(1 == table.erase(ptrs[i].get()));
                assert(table.size() == arr_size - i - 1);
                assert(table.bucket_count() == total_size);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for find no exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                auto it = table.find(ptrs[i].get());
                assert(!it);
                assert(table.size() == 0);
                assert(table.bucket_count() == total_size);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for erase no exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                assert(0 == table.erase(ptrs[i].get()));
                assert(table.size() == 0);
                assert(table.bucket_count() == total_size);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        std::cout << "=======================================================\n";
    }
    {
        using value_type = test_class;
        using hash_table = hopscotch_hash_table_3<value_type>;
        std::cout << "=======================================================\n";
        std::cout << "Tests for user objects:\n";
        hash_table table(log_2_bucket_cnt_for_user_objects);
        test_class obj_1(0,0);
        test_class obj_2(1,1);
        {
            std::cout << "=======================================================\n";
            std::cout << "Tests for constructor:\n";
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert(const type&) new element in empty table:\n";
            auto it = table.insert(obj_1);
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == obj_1);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert(const type&) exist element:\n";
            auto it = table.insert(obj_1);
            assert(it.second == false);
            assert(it.first);
            assert(*it.first == obj_1);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for insert(const type&) new element in not empty table:\n";
            auto it = table.insert(obj_2);
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == obj_2);
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for find(const type&) exist element in not empty table:\n";
            auto it = table.find(obj_2);
            assert(it);
            assert(*it == obj_2);
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract(const type&) exist element in not empty table:\n";
            assert(1 == table.erase(obj_2));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for find(const type&) no exist element in not empty table:\n";
            auto it = table.find(obj_2);
            assert(!it);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract(const type&) no exist element in not empty table:\n";
            assert(0 == table.erase(obj_2));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "Tests for extract(const type&) last element in not empty table:\n";
            assert(1 == table.erase(obj_1));
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        std::cout << "Lvalue tests done!\n";
        std::cout << "=======================================================\n";
        std::cout << "Rvalue tests!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for insert(type&&) new element in empty table:\n";
            auto tmp = obj_1;
            std::cout << "tmp object was constructed!\n";
            auto it = table.insert(std::move(tmp));
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == obj_1);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for insert(type&&) exist element:\n";
            auto tmp = obj_1;
            std::cout << "tmp object was constructed!\n";
            auto it = table.insert(std::move(tmp));
            assert(it.second == false);
            assert(it.first);
            assert(*it.first == obj_1);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for insert(type&&) new element in not empty table:\n";
            auto tmp = obj_2;
            std::cout << "tmp object was constructed!\n";
            auto it = table.insert(std::move(tmp));
            assert(it.first);
            assert(it.second == true);
            assert(*it.first == obj_2);
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for find(type&&) exist element in not empty table:\n";
            auto tmp = obj_2;
            std::cout << "tmp object was constructed!\n";
            auto it = table.find(std::move(tmp));
            assert(it);
            assert(*it == obj_2);
            assert(table.size() == 2);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for extract(type&&) exist element in not empty table:\n";
            auto tmp = obj_2;
            std::cout << "tmp object was constructed!\n";
            assert(1 == table.erase(std::move(tmp)));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for find(const type&&) no exist element in not empty table:\n";
            auto tmp = obj_2;
            std::cout << "tmp object was constructed!\n";
            auto it = table.find(std::move(tmp));
            assert(!it);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for extract(type&&) no exist element in not empty table:\n";
            auto tmp = obj_2;
            std::cout << "tmp object was constructed!\n";
            assert(0 == table.erase(std::move(tmp)));
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for extract(type&&) last element in not empty table:\n";
            auto tmp = obj_1;
            std::cout << "tmp object was constructed!\n";
            assert(1 == table.erase(std::move(tmp)));
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for emplace(type&&):\n";
            test_class tmp {10, 10};
            std::cout << "tmp object was constructed!\n";
            auto it = table.emplace(10, 10);
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == tmp);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "Done!\n";
        std::cout << "=======================================================\n";
        {
            std::cout << "Tests for emplace(type&&) 5 elements:\n";
            for (size_t i = 0; i < 5; i++)
            {
                auto it = table.emplace(i, i);
                assert(it.second == true);
                assert(it.first);
                //assert(*it.first == test_class{i, i});
            }
            assert(table.size() == 6);
            assert(table.bucket_count() == total_size_for_user_objects);
        }
        std::cout << "=======================================================\n";
        std::cout << "Rvalue tests done!\n";
    }
    std::cout << "=======================================================\n";
    std::cout << "All tests done!\n";
    std::cout << "=======================================================\n";
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
}
}
