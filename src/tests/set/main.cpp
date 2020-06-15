#include<set/incremental_rehash_open_set.h>
#include<set/incremental_rehash_hopscotch_set.h>
#include<set/incremental_rehash_hopscotch_set.h>
#include<set/incremental_rehash_hopscotch_set_2.h>
#include<set/incremental_rehash_chaining_set.h>
#include<type_traits>
#include<utils/test_class.h>
#include<iostream>
#include<vector>
#include<cassert>

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
    static constexpr std::size_t log_2_bucket_cnt = 14;
    static constexpr std::size_t log_2_bucket_cnt_for_user_objects = 4;
    static constexpr std::size_t total_size = 1 << log_2_bucket_cnt;
    static constexpr std::size_t total_size_for_user_objects = 1 << log_2_bucket_cnt_for_user_objects;
    static constexpr std::size_t arr_size = (1 << log_2_bucket_cnt) * 0.75;
try
{
    {
        using value_type = int64_t;
        using hash_table = incremental_rehash_hopscotch_set_2<value_type*>;
        std::cout << "incremental_rehash_open_set tests:\n";
        std::cout << "=======================================================\n";
        std::cout << "Tests for base types:\n";
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
            std::cout << "=======================================================\n";
            std::cout << "Tests for default constructor:\n";
            hash_table table;
            assert(table.size() == 0);
            assert(table.bucket_count() == 0);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            std::cout << "=======================================================\n";
            std::cout << "Tests for constructor:\n";
            hash_table table(log_2_bucket_cnt);
            assert(table.size() == 0);
            assert(table.bucket_count() == total_size);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for insert new element in empty table:\n";
            auto it = table.insert(ptr1.get());
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for insert exist element:\n";
            auto it = table.insert(ptr1.get());
            it = table.insert(ptr1.get());
            assert(it.second == false);
            assert(it.first);
            assert(*it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for insert new element in not empty table:\n";
            auto it = table.insert(ptr1.get());
            it = table.insert(ptr2.get());
            assert(it.first);
            assert(*it.first == ptr2.get());
            assert(it.second == true);
            assert(table.size() == 2);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for insert new elements in not empty table:\n";
            auto it = table.insert(ptr1.get());
            it = table.insert(ptr2.get());
            it = table.insert(ptr3.get());
            assert(it.first && it.second == true && *it.first == ptr3.get());
            it = table.insert(ptr4.get());
            assert(it.first && it.second == true && *it.first == ptr4.get());
            it = table.insert(ptr5.get());
            assert(it.first && it.second == true && *it.first == ptr5.get());
            assert(table.size() == 5);
            assert(table.bucket_count() == 8);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for find exist element in not empty table:\n";
            table.insert(ptr1.get());
            table.insert(ptr2.get());
            auto it = table.find(ptr2.get());
            assert(it);
            assert(*it == ptr2.get());
            it = table.find(ptr1.get());
            assert(it);
            assert(*it == ptr1.get());
            assert(table.size() == 2);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for find no exist element in not empty table:\n";
            table.insert(ptr1.get());
            table.insert(ptr2.get());
            auto it = table.find(ptr3.get());
            assert(!it);
            assert(table.size() == 2);
            assert(table.bucket_count() == 2 );
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for find no exist element in empty table:\n";
            auto it = table.find(ptr3.get());
            assert(!it);
            assert(table.size() == 0);
            assert(table.bucket_count() == 0);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for extract exist element in not empty table:\n";
            table.insert(ptr1.get());
            table.insert(ptr2.get());
            assert(1 == table.erase(ptr2.get()));
            assert(table.size() == 1);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for extract no exist element in not empty table:\n";
            table.insert(ptr1.get());
            table.insert(ptr2.get());
            assert(0 == table.erase(ptr3.get()));
            assert(table.size() == 2);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for extract last element in not empty table:\n";
            table.insert(ptr1.get());
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for insert, erase, insert, erase some element in table:\n";
            auto it = table.insert(ptr1.get());
            assert(it.first && it.second == true && *it.first == ptr1.get());
            assert(table.size() == 1);
            assert(table.bucket_count() == 2);
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == 2);
            it = table.insert(ptr1.get());
            assert(it.first && it.second == true && *it.first == ptr1.get());
            assert(table.size() == 1);
            assert(1 == table.erase(ptr1.get()));
            assert(table.size() == 0);
            assert(table.bucket_count() == 2);

            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table(2);
            std::cout << "Tests for insert some elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                //std::cout << "Pointer " << i << " " << (void*)ptrs[i].get();
                auto it = table.insert(ptrs[i].get());
                //std::cout << " bucket count: " << table.bucket_count() << std::endl;
                assert(it.first && it.second == true && *it.first == ptrs[i].get());
                assert(table.size() == i + 1);
            }
            assert(table.size() == arr_size);
            //assert(table.bucket_count() == arr_size * 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for find exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
                table.insert(ptrs[i].get());
            for (size_t i = 0; i < arr_size; i++)
            {
                //std::cout << "Pointer " << i << " " << (void*)ptrs[i].get() << std::endl;
                auto it = table.find(ptrs[i].get());
                assert(it);
                assert(*it == ptrs[i].get());
            }
            // assert(table.bucket_count() == arr_size * 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for erase some elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
                table.insert(ptrs[i].get());
            for (size_t i = 0; i < arr_size; i++)
            {
                //std::cout << "Pointer " << i << " " << (void*)ptrs[i].get() << std::endl;
                assert(1 == table.erase(ptrs[i].get()));
                assert(table.size() == arr_size - i - 1);
                //assert(table.bucket_count() == arr_size * 2);
            }
            // assert(table.bucket_count() == arr_size * 2);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for find no exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                //std::cout << "Pointer " << i << " " << (void*)ptrs[i].get() << std::endl;
                auto it = table.find(ptrs[i].get());
                assert(!it);
                assert(table.size() == 0);
                assert(table.bucket_count() == 0);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        {
            hash_table table;
            std::cout << "Tests for erase no exist elements in table:\n";
            for (size_t i = 0; i < arr_size; i++)
            {
                assert(0 == table.erase(ptrs[i].get()));
                assert(table.size() == 0);
                assert(table.bucket_count() == 0);
            }
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        std::cout << "=======================================================\n";
    }
    {
        using value_type = test_class;
        using hash_table = incremental_rehash_hopscotch_set_2<value_type>;
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
    {
        using value_type = std::string;
        using hash_table = robin_hood_hash_table<value_type>;
        std::cout << "=======================================================\n";
        std::cout << "Tests for user objects:\n";
        hash_table table(log_2_bucket_cnt_for_user_objects);
        value_type obj_1("12345678");
        value_type obj_2("123456789");
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
            //std::aligned_storage<sizeof(std::string), alignof(std::string)> storage;
            //::new(static_cast<void*>(&storage))std::string(obj_1);
            //assert(reinterpret_cast<std::string&>(storage) == obj_1);
            auto it = table.insert(obj_1);
            assert(it.second == true);
            assert(it.first);
            assert(*it.first == obj_1);
            assert(table.size() == 1);
            assert(table.bucket_count() == total_size_for_user_objects);
            std::cout << "Done!\n";
            std::cout << "=======================================================\n";
        }
        //{
        //    std::cout << "Tests for insert(const type&) exist element:\n";
        //    auto it = table.insert(obj_1);
        //    assert(it.second == false);
        //    assert(it.first);
        //    assert(*it.first == obj_1);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for insert(const type&) new element in not empty table:\n";
        //    auto it = table.insert(obj_2);
        //    assert(it.second == true);
        //    assert(it.first);
        //    assert(*it.first == obj_2);
        //    assert(table.size() == 2);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for find(const type&) exist element in not empty table:\n";
        //    auto it = table.find(obj_2);
        //    assert(it);
        //    assert(*it == obj_2);
        //    assert(table.size() == 2);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for extract(const type&) exist element in not empty table:\n";
        //    assert(1 == table.erase(obj_2));
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for find(const type&) no exist element in not empty table:\n";
        //    auto it = table.find(obj_2);
        //    assert(!it);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for extract(const type&) no exist element in not empty table:\n";
        //    assert(0 == table.erase(obj_2));
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //{
        //    std::cout << "Tests for extract(const type&) last element in not empty table:\n";
        //    assert(1 == table.erase(obj_1));
        //    assert(table.size() == 0);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //    std::cout << "Done!\n";
        //    std::cout << "=======================================================\n";
        //}
        //std::cout << "Lvalue tests done!\n";
        //std::cout << "=======================================================\n";
        //std::cout << "Rvalue tests!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for insert(type&&) new element in empty table:\n";
        //    auto tmp = obj_1;
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.insert(std::move(tmp));
        //    assert(it.second == true);
        //    assert(it.first);
        //    assert(*it.first == obj_1);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for insert(type&&) exist element:\n";
        //    auto tmp = obj_1;
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.insert(std::move(tmp));
        //    assert(it.second == false);
        //    assert(it.first);
        //    assert(*it.first == obj_1);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for insert(type&&) new element in not empty table:\n";
        //    auto tmp = obj_2;
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.insert(std::move(tmp));
        //    assert(it.first);
        //    assert(it.second == true);
        //    assert(*it.first == obj_2);
        //    assert(table.size() == 2);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for find(type&&) exist element in not empty table:\n";
        //    auto tmp = obj_2;
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.find(std::move(tmp));
        //    assert(it);
        //    assert(*it == obj_2);
        //    assert(table.size() == 2);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for extract(type&&) exist element in not empty table:\n";
        //    auto tmp = obj_2;
        //    std::cout << "tmp object was constructed!\n";
        //    assert(1 == table.erase(std::move(tmp)));
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for find(const type&&) no exist element in not empty table:\n";
        //    auto tmp = obj_2;
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.find(std::move(tmp));
        //    assert(!it);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for extract(type&&) no exist element in not empty table:\n";
        //    auto tmp = obj_2;
        //    std::cout << "tmp object was constructed!\n";
        //    assert(0 == table.erase(std::move(tmp)));
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for extract(type&&) last element in not empty table:\n";
        //    auto tmp = obj_1;
        //    std::cout << "tmp object was constructed!\n";
        //    assert(1 == table.erase(std::move(tmp)));
        //    assert(table.size() == 0);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for emplace(type&&):\n";
        //    test_class tmp {10, 10};
        //    std::cout << "tmp object was constructed!\n";
        //    auto it = table.emplace(10, 10);
        //    assert(it.second == true);
        //    assert(it.first);
        //    assert(*it.first == tmp);
        //    assert(table.size() == 1);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
        //std::cout << "Done!\n";
        //std::cout << "=======================================================\n";
        //{
        //    std::cout << "Tests for emplace(type&&) 5 elements:\n";
        //    for (size_t i = 0; i < 5; i++)
        //    {
        //        auto it = table.emplace(i, i);
        //        assert(it.second == true);
        //        assert(it.first);
        //        //assert(*it.first == test_class{i, i});
        //    }
        //    assert(table.size() == 6);
        //    assert(table.bucket_count() == total_size_for_user_objects);
        //}
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
