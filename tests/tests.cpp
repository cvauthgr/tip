#include <cstddef>
#include <gtest/gtest.h>
#include <string>

#include "memory_pool.hpp"

TEST(memory_pool , bump_not_null_on_constructor){
    memory_pool<2048> pool ;
    EXPECT_NE(pool.acquire_bump() , nullptr);
}

TEST(memory_pool , allocate_fundamental_data_type){
    memory_pool<2048> pool ;

    auto int_ptr = pool.allocate<int>() ;

    EXPECT_NE(int_ptr , nullptr);
}

TEST(memory_pool , checkpoint_after_one_allocation){
    memory_pool<2048> pool ;

    // Allocate once first
    struct alignas(512) box {
        char a = 'a' ;
    };

    [[maybe_unused]] auto huge_allocation_ptr = pool.allocate<box>() ;
    // Acquire the bump ptr location and cache a checkpoint
    auto initial_bump = pool.acquire_bump() ;
    auto capacity_at_checkpoint = pool.remaining_capacity() ;
    pool.cache_checkpoint() ;

    // Allocate again
    huge_allocation_ptr = pool.allocate<box>() ;
    // Revert
    pool.revert() ;

    EXPECT_EQ(initial_bump , pool.acquire_bump()) ;
    // Also check for a capacity mismatch
    EXPECT_EQ(capacity_at_checkpoint , pool.remaining_capacity()) ;
}

TEST(memory_pool , checkpoint_after_array_allocation){
    memory_pool<2048> pool ;

    // Allocate an array first
    struct alignas(64) box {
        char a = 'a' ;
    };

    [[maybe_unused]] auto huge_allocation_ptr = pool.allocate_array<box>(10) ;
    // Acquire the bump ptr location and cache a checkpoint
    auto initial_bump = pool.acquire_bump() ;
    auto capacity_at_checkpoint = pool.remaining_capacity() ;
    pool.cache_checkpoint() ;

    // Allocate an array again
    huge_allocation_ptr = pool.allocate_array<box>(10) ;
    // Revert
    pool.revert() ;

    EXPECT_EQ(initial_bump , pool.acquire_bump()) ;
    // Also check for a capacity mismatch
    EXPECT_EQ(capacity_at_checkpoint , pool.remaining_capacity()) ;
}

TEST(memory_pool , space_reclamation){
    constexpr std::size_t capacity = 2048 ;

    memory_pool<capacity> pool ;

    // Allocate lots of objects
    struct alignas(256) box{
        char b = 'b' ;
    };

    // Allocate on the pool
    [[maybe_unused]] auto ptr = pool.allocate_array<box>(5) ;

    // Check that capacity changed
    EXPECT_NE(pool.remaining_capacity() , capacity) ;

    // Free the pool
    pool.reclaim() ;

    // Assert that everything is okay
    EXPECT_EQ(capacity , pool.remaining_capacity()) ;
}

// The .owns() API should be bulletproof as
// I will rely on it mostly for the next parts
// of the pool "architecture"

TEST(memory_pool , owns_valid_allocation_pointer){
    memory_pool<2048> pool ;

    // First try an int which under
    // all circumstances will fit
    auto in_pool = pool.allocate<int>() ;
    EXPECT_EQ(pool.owns(in_pool) , true) ;

    // Now for the final boss
    // This will surely NOT be in the
    // memory pool under no circumstances
    in_pool += 10'000 ;
    EXPECT_EQ(pool.owns(in_pool), false) ;

    // Now it should be in the pool
    // once again
    in_pool -= 10'000 ;
    EXPECT_EQ(pool.owns(in_pool), true) ;
}

TEST(memory_pool , owns_off_by_one){
    memory_pool<2048> pool ;

    // Off by one errors should be
    // handled through an API that can
    // detect them
    //
    // -Me
    auto in_pool = pool.allocate<char>() ;
    // Sanity check
    EXPECT_EQ(pool.owns(in_pool) , true);
    // Off by one now . As a char has an
    // alignment of alignof(char) = 1
    // any memory adress is valid so the bump
    // after the .allocate call will point at
    // the start of the pool , so we safely
    // assume that in_pool -= 1 is guaranteed
    // to be out of the pool
    in_pool -= 1 ;
    EXPECT_EQ(pool.owns(in_pool) , false);

    // WARNING : For any similar tests with
    // other types or UDT we would need to subtract
    // the alignment size too (not exactly but you know
    // what I am saying) -To future self
}

TEST(memory_pool , on_construction_bump_equal_to_start){
    memory_pool<2048> pool ;
    // Test if the bump_ptr_
    // returned is detected as
    // the pool must own it
    EXPECT_EQ(pool.acquire_bump() , pool.acquire_internal_pool_start()) ;
}

TEST(memory_pool , pool_owns_bump){
    memory_pool<2048> pool ;
    EXPECT_EQ(pool.owns(pool.acquire_bump()), true) ;
}

TEST(memory_pool , pool_owns_start_ptr){
    memory_pool<2048> pool ;
    EXPECT_EQ(pool.owns(pool.acquire_internal_pool_start()), true) ;
}

TEST(memory_pool , single_allocation_pool_out_of_space){
    memory_pool<2> pool ;

    struct alignas(256) UDT{
        char a = 'a' ;
    };

    // This should be returning nullptr
    auto ptr = pool.allocate<UDT>() ;

    EXPECT_EQ(ptr , nullptr) ;
}

TEST(memory_pool , multiple_allocations_pool_out_of_space){
    memory_pool<2> pool ;

    struct alignas(256) UDT{
        char a = 'a' ;
    };

    auto ptr = pool.allocate_array<UDT>(100) ;

    EXPECT_EQ(ptr , nullptr) ;
}

// Tests for allocate and construct

TEST(memory_pool , allocate_and_construct_pool_out_of_memory){
    memory_pool<2> pool ;

    // Try to allocate and construct
    // We won't have enough space so
    // we will receive a nullptr
    // We should not SEGFAULT
    auto ptr = pool.allocate_and_construct<double>(0.0f) ;
    EXPECT_EQ(ptr , nullptr) ;
}

TEST(memory_pool , valid_allocate_and_construct_access){
    memory_pool<2048> pool ;

    struct alignas(64) UDT{
        int a = 0 ;
    };

    auto ptr = pool.allocate_and_construct<UDT>(67) ;

    EXPECT_EQ(ptr->a, 67) ;
}

TEST(memory_pool , allocate_and_construct_array_pool_out_of_memory){
    // For this test a nullptr with
    // no seg fault is expected
    memory_pool<2048> pool ;

    struct alignas(128) UDT{
        int a = 0 ;
        double b = 0.0f ;
   };

    std::size_t count = 10 ;

    auto start = pool.allocate_and_construct_array<UDT>(count , 67 , 67.0f) ;

    EXPECT_NE(start , nullptr) ;

    for(std::size_t i = {0uz} ; i < count ; ++i){
        EXPECT_EQ(start[i].a , 67) ;
        EXPECT_DOUBLE_EQ(start[i].b , 67.0f) ;
    }
}

TEST(memory_pool , bump_moves_after_failed_allocate_array){
    memory_pool<256> pool ;

    // Acquire the current bump location
    auto initial_location = pool.acquire_bump() ;

    // Fail to allocate the array
    // Will return the nullptr but the
    // bump will move either way
    [[maybe_unused]] auto failed_allocate_array_ptr = pool.allocate_array<double>(1'000) ;

    // Acquire the current bump ptr again
    auto location_after_fail = pool.acquire_bump() ;

    EXPECT_NE(initial_location , location_after_fail) ;
}

TEST(memory_pool , bump_moves_after_failed_allocate_and_construct_array){
    memory_pool<256> pool ;

    // Acquire the current bump location
    auto initial_location = pool.acquire_bump() ;

    // Fail to allocate the array
    // Will return the nullptr but the
    // bump will move either way
    [[maybe_unused]] auto failed_allocate_array_ptr = pool.allocate_and_construct_array<double>(1'000 , 0.0f) ;

    // Acquire the current bump ptr again
    auto location_after_fail = pool.acquire_bump() ;

    EXPECT_NE(initial_location , location_after_fail) ;
}

TEST(memory_pool , valid_can_fit){
    memory_pool<2048> pool ;

    // 2048 chars should fit perfectly
    EXPECT_TRUE(pool.can_fit<char>(2048)) ;
}

TEST(memory_pool , invalid_can_fit_off_by_one){
    memory_pool<2048> pool ;

    // 2049 chars should not fit by one
    EXPECT_FALSE(pool.can_fit<char>(2049)) ;
}

TEST(memory_pool , valid_can_fit_normal_request){
    memory_pool<256> pool ;

    EXPECT_TRUE(pool.can_fit<double>(10)) ;
}

TEST(memory_pool , valid_can_fit_after_allocation){
    memory_pool<2048> pool ;

    [[maybe_unused]] auto ptr = pool.allocate_and_construct_array<double>(200, 0.0f) ;

    // Worst case scenario , the ptr had to move 7 bytes forward
    // 2048 - 7 = 2041 - 8*200 = 2041 - 1600 = 441
    // 441 / 8 = 55 doubles can fit (and some change)

    EXPECT_TRUE(pool.can_fit<double>(55)) ;
}

TEST(memory_pool , invalid_can_fit_after_allocation){
    memory_pool<2048> pool ;

    [[maybe_unused]] auto ptr = pool.allocate_and_construct_array<double>(200, 0.0f) ;

    // Worst case scenario , the ptr had to move 7 bytes forward
    // 2048 - 7 = 2041 - 8*200 = 2041 - 1600 = 441
    // 441 / 8 = 55 doubles can fit (and some change)
    // 56 should fail

    EXPECT_FALSE(pool.can_fit<double>(57)) ;
}
