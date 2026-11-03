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
    tip::allocator<tip::default_policy, 2048> allocator ;

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
