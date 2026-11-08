#include <cstddef>
#include <gtest/gtest.h>

#include "allocator.hpp"
#include "policies.hpp"

TEST(allocator_default_policy , valid_behavior_allocate_and_construct){
    tip::allocator<tip::default_policy , 2048> allocator ;

    auto ptr = allocator.allocate<double>() ;
    *ptr = 10.0f ;

    EXPECT_NE(ptr , nullptr) ;
    EXPECT_FALSE(allocator.using_fallback_resource()) ;
    // It will have to use 8 bytes in the best case
    // scenario (pointer fully alligned at start)
    EXPECT_LE(allocator.remaining_capacity() , 2040) ;

    // Now access the element
    EXPECT_DOUBLE_EQ(*ptr, 10.0f) ;
}

TEST(allocator_default_policy , valid_behavior_allocate_and_construct_array){
    tip::allocator<tip::default_policy , 2048> allocator ;

    struct alignas(64) box{
        char a = ' ' ;
    };

    auto ptr = allocator.allocate_array<box>(10) ;

    EXPECT_NE(ptr , nullptr) ;
    EXPECT_FALSE(allocator.using_fallback_resource()) ;
    EXPECT_LE(allocator.remaining_capacity() , (2048-(sizeof(box)*10))) ;

    for(std::size_t i = {0uz} ; i < 10 ; ++i){
        ptr[i].a = 'a' ;
        EXPECT_EQ(ptr[i].a , 'a') ;
    }
}

TEST(allocator_permissive_policy , fallback_to_new_single_object){
    tip::allocator<tip::permissive_policy , 2> allocator ;

    // Ask for a huge allocation , which should give us
    // a dynamically allocated object adress back

    struct alignas(512) huge_object{
        char a = ' ' ;
    };

    std::size_t initial_capacity = allocator.remaining_capacity() ;

    auto ptr = allocator.allocate<huge_object>() ;

    // Set it to the value we want
    ptr->a = 'a' ;

    std::size_t capacity_after_allocation = allocator.remaining_capacity() ;

    EXPECT_NE(ptr , nullptr) ;
    // Also the capacity shouldn't change
    EXPECT_EQ(initial_capacity , capacity_after_allocation) ;
    // We should also be able to access the element
    EXPECT_EQ(ptr->a, 'a') ;
    // The allocator should flag that we are using the fallback resource from now on
    EXPECT_TRUE(allocator.using_fallback_resource()) ;

    // Free the pointer
    delete  ptr ;
}

TEST(allocator_permissive_policy , fallback_to_new_array_of_objects){
    tip::allocator<tip::permissive_policy , 2> allocator ;

    // Ask for a huge allocation , which should give us
    // a dynamically allocated object adress back

    struct alignas(512) huge_object{
        char a = 'a' ;
        int i = 0 ;
        double d = 0.0f ;
    };

    std::size_t initial_capacity = allocator.remaining_capacity() ;

    constexpr std::size_t count = 100 ;

    auto ptr = allocator.allocate_array<huge_object>(count) ;

    for(std::size_t i = {0uz} ; i < count ; ++i){
        ptr[i] = {'a',10,13.3f} ;
    }

    std::size_t capacity_after_allocation = allocator.remaining_capacity() ;

    EXPECT_NE(ptr , nullptr) ;
    // Also the capacity shouldn't change
    EXPECT_EQ(initial_capacity , capacity_after_allocation) ;
    // We should also be able to access the element
    for(std::size_t index = {0uz} ; index < count ; ++index){
        EXPECT_EQ(ptr->a, 'a') ;
        EXPECT_EQ(ptr->i, 10) ;
        EXPECT_EQ(ptr->d, 13.3f) ;
    }
    // The allocator should flag that we are using the fallback resource from now on
    EXPECT_TRUE(allocator.using_fallback_resource()) ;

    // Free the pointer
    delete[]  ptr ;
}

TEST(allocator_default_policy , allocate_pool_out_of_memory){
    tip::allocator<tip::default_policy , 2> allocator ;

    // Try to allocate and construct
    // We won't have enough space so
    // we will receive a nullptr
    // We should not SEGFAULT
    auto ptr = allocator.allocate<double>() ;
    EXPECT_EQ(ptr , nullptr) ;
}

TEST(allocator_default_policy , valid_allocate_and_construct_access){
    tip::allocator<tip::default_policy , 2048> allocator ;

    struct alignas(64) UDT{
        int a = 0 ;
    };

    auto ptr = allocator.allocate<UDT>() ;

    ptr->a = 67 ;

    EXPECT_EQ(ptr->a, 67) ;
}

TEST(memory_pool , allocate_array_valid){
    tip::allocator<tip::default_policy , 2048> allocator ;

    struct alignas(128) UDT{
        int a = 0 ;
        double b = 0.0f ;
   };

    std::size_t count = 10 ;

    auto start = allocator.allocate_array<UDT>(count) ;

    EXPECT_NE(start , nullptr) ;

    for(std::size_t i = {0uz} ; i < count ; ++i){
        start[i].a = 67 ;
        start[i].b = 67.0f ;
        EXPECT_EQ(start[i].a , 67) ;
        EXPECT_DOUBLE_EQ(start[i].b , 67.0f) ;
    }
}
