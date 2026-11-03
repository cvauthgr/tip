#include <cstddef>
#include <gtest/gtest.h>

#include "allocator.hpp"
#include "policies.hpp"

TEST(allocator_default_policy , valid_behavior_allocate_and_construct){
    tip::allocator<tip::default_policy , 2048> allocator ;

    auto ptr = allocator.allocate_and_construct<double>(10.0f) ;

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

    auto ptr = allocator.allocate_and_construct_array<box>(10, 'a') ;

    EXPECT_NE(ptr , nullptr) ;
    EXPECT_FALSE(allocator.using_fallback_resource()) ;
    EXPECT_LE(allocator.remaining_capacity() , (2048-(sizeof(box)*10))) ;

    for(std::size_t i = {0uz} ; i < 10 ; ++i){
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

    auto ptr = allocator.allocate_and_construct<huge_object>('a') ;

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

    auto ptr = allocator.allocate_and_construct_array<huge_object>(count,'a',10,13.3f) ;

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
