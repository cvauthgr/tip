#include "memory_pool.hpp"

#include <cstddef>
#include <print>

struct alignas(32) huge_struct{
    std::size_t a = 0 ;
};

struct alignas(64) bigger_struct{
    char a = ' ' ;
};

struct alignas(128) biggerer_struct{
    char b = ' ' ;
};

struct alignas(256) super {
    int a = 0 ;
    float b = 0.0 ;
    std::string n = { } ;
    double c = 0.0 ;
};

int main(){
    /*memory_pool<2048> pool ;

    auto char_ptr = pool.allocate<char>();
    auto int_ptr = pool.allocate<int>() ;
    auto double_ptr = pool.allocate<double>() ;
    auto ptr32 = pool.allocate<huge_struct>() ;
    auto ptr64 = pool.allocate<bigger_struct>() ;
    auto ptr128 = pool.allocate<biggerer_struct>() ;

    std::print("1 byte -> {}\n",static_cast<void*>(char_ptr)) ;
    std::print("4 byte -> {}\n",static_cast<void*>(int_ptr)) ;
    std::print("8 byte -> {}\n",static_cast<void*>(double_ptr)) ;
    std::print("32 byte -> {}\n",static_cast<void*>(ptr32)) ;
    std::print("64 byte -> {}\n",static_cast<void*>(ptr64)) ;
    std::print("128 byte -> {}\n",static_cast<void*>(ptr128)) ;

    auto super_ptr = pool.allocate_and_construct<super>(2,2.0f,"binos",8.08f);

    std::println("{}\n{}\n{}\n{}",super_ptr->a,super_ptr->b,super_ptr->c,super_ptr->n) ;

    pool.cache_checkpoint() ;

    char_ptr= pool.allocate<char>() ;
    int_ptr = pool.allocate<int>() ;
    double_ptr = pool.allocate<double>() ;
    ptr32 = pool.allocate<huge_struct>() ;
    ptr64 = pool.allocate<bigger_struct>() ;
    ptr128 = pool.allocate<biggerer_struct>() ;

    std::print("1 byte -> {}\n",static_cast<void*>(char_ptr)) ;
    std::print("4 byte -> {}\n",static_cast<void*>(int_ptr)) ;
    std::print("8 byte -> {}\n",static_cast<void*>(double_ptr)) ;
    std::print("32 byte -> {}\n",static_cast<void*>(ptr32)) ;
    std::print("64 byte -> {}\n",static_cast<void*>(ptr64)) ;
    std::print("128 byte -> {}\n",static_cast<void*>(ptr128)) ;

    std::println("Reverting to checkpoint");
    pool.revert() ;

    char_ptr= pool.allocate<char>();
    int_ptr = pool.allocate<int>() ;
    double_ptr = pool.allocate<double>() ;
    ptr32 = pool.allocate<huge_struct>() ;
    ptr64 = pool.allocate<bigger_struct>() ;
    ptr128 = pool.allocate<biggerer_struct>() ;

    std::print("1 byte -> {}\n",static_cast<void*>(char_ptr)) ;
    std::print("4 byte -> {}\n",static_cast<void*>(int_ptr)) ;
    std::print("8 byte -> {}\n",static_cast<void*>(double_ptr)) ;
    std::print("32 byte -> {}\n",static_cast<void*>(ptr32)) ;
    std::print("64 byte -> {}\n",static_cast<void*>(ptr64)) ;
    std::print("128 byte -> {}\n",static_cast<void*>(ptr128)) ;

    pool.reclaim() ;

    char_ptr= pool.allocate<char>();
    int_ptr = pool.allocate<int>() ;
    double_ptr = pool.allocate<double>() ;
    ptr32 = pool.allocate<huge_struct>() ;
    ptr64 = pool.allocate<bigger_struct>() ;
    ptr128 = pool.allocate<biggerer_struct>() ;

    pool.reclaim() ;

    std::println("Remaning size -> {}",pool.remaining_capacity()) ;
    std::println("Bump position -> {}",pool.acquire_bump()) ;

    auto list_of_256ptrs = pool.allocate_array<super>(5) ;

    std::println("Allocated 5 super structs (default constructed) at {}",static_cast<void*>(list_of_256ptrs)) ;

    std::println("Remaning size -> {}",pool.remaining_capacity()) ;
    std::println("Bump position -> {}",pool.acquire_bump()) ;*/

    tip::memory_pool<2> pool ;

    auto ptr_1 = pool.allocate<char>() ;
    auto ptr_2 = pool.allocate<char>() ;

    std::println("ptr_1 -> {}\nptr_2 -> {}",static_cast<void*>(ptr_1),static_cast<void*>(ptr_2)) ;

    std::println("Bump location == ptr_2 + 1 -> {} == {}",pool.acquire_bump(),static_cast<void*>(ptr_2)) ;

    std::println("Which should match : Bump == initial_start_ + size : {} == {}",
                pool.acquire_bump() ,
                static_cast<const void*>(static_cast<const std::byte*>(pool.acquire_internal_pool_start())+2)) ;

    return 0;
}
