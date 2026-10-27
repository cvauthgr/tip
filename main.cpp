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
    memory_pool<2048> pool ;

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

    auto super_ptr = pool.allocate_and_construct<super>(1,2.0f,"binos",8.08f);

    std::println("{}\n{}\n{}\n{}",super_ptr->a,super_ptr->b,super_ptr->c,super_ptr->n) ;

    return 0;
}
