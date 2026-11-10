#include <cstddef>
#include <gtest/gtest.h>
#include <string>

#include "memory_pool.hpp"
#include "policies.hpp"

TEST(memory_pool , bump_not_null_on_constructor){
    tip::memory_pool<tip::default_policy , 2048> pool ;
    EXPECT_NE(pool.acquire_bump() , nullptr);
}

TEST(memory_pool , allocate_fundamental_data_type){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    auto int_ptr = pool.claim<int>() ;

    EXPECT_NE(int_ptr , nullptr);
}

TEST(memory_pool , space_reclamation){
    constexpr std::size_t capacity = 2048 ;

    tip::memory_pool<tip::default_policy , capacity> pool ;

    // Allocate lots of objects
    struct alignas(256) box{
        char b = 'b' ;
    };

    // Allocate on the pool
    [[maybe_unused]] auto ptr = pool.claim_array<box>(5) ;

    // Check that capacity changed
    EXPECT_NE(pool.remaining_capacity() , capacity) ;

    // free the pool
    pool.reclaim() ;

    // Assert that everything is okay
    EXPECT_EQ(capacity , pool.remaining_capacity()) ;
}

// The .owns() API should be bulletproof as
// I will rely on it mostly for the next parts
// of the pool "architecture"

TEST(memory_pool , owns_valid_allocation_pointer){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    // First try an int which under
    // all circumstances will fit
    auto in_pool = pool.claim<int>() ;
    EXPECT_TRUE(pool.owns(in_pool)) ;

    // Now for the final boss
    // This will surely NOT be in the
    // memory pool under no circumstances
    in_pool += 10'000 ;
    EXPECT_FALSE(pool.owns(in_pool)) ;

    // Now it should be in the pool
    // once again
    in_pool -= 10'000 ;
    EXPECT_TRUE(pool.owns(in_pool)) ;
}

TEST(memory_pool , owns_off_by_one){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    // Off by one errors should be
    // handled through an API that can
    // detect them
    //
    // -Me
    auto in_pool = pool.claim<char>() ;
    // Sanity check
    EXPECT_TRUE(pool.owns(in_pool));
    // Off by one now . As a char has an
    // alignment of alignof(char) = 1
    // any memory adress is valid so the bump
    // after the .allocate call will point at
    // the start of the pool , so we safely
    // assume that in_pool -= 1 is guaranteed
    // to be out of the pool
    in_pool -= 1 ;
    EXPECT_FALSE(pool.owns(in_pool));

    // WARNING : For any similar tests with
    // other types or UDT we would need to subtract
    // the alignment size too (not exactly but you know
    // what I am saying) -To future self
}

TEST(memory_pool , owns_small_pool_size_fail){
    tip::memory_pool<tip::default_policy , 2> pool ;

    // Valid as char is aligned to any memory adress
    auto first_allocation = pool.claim<char>() ;
    // Valid for the same reason
    auto second_allocation = pool.claim<char>() ;

    // Both should be valid
    EXPECT_TRUE(pool.owns(first_allocation)) ;
    EXPECT_TRUE(pool.owns(second_allocation)) ;

    // Moving the ptr of the 2nd allocation
    // forward should land us at the end of the pool
    EXPECT_TRUE(pool.owns(second_allocation+1)) ;

    // Moving it once more should land us outside
    EXPECT_FALSE(pool.owns(second_allocation+2)) ;

    // Same thing for moving backwards once at the
    // ptr of the 1st allocation
    EXPECT_FALSE(pool.owns(first_allocation-1)) ;
}

TEST(memory_pool , on_construction_bump_equal_to_start){
    tip::memory_pool<tip::default_policy , 2048> pool ;
    // Test if the bump_ptr_
    // returned is detected as
    // the pool must own it
    EXPECT_EQ(pool.acquire_bump() , pool.acquire_internal_pool_start()) ;
}

TEST(memory_pool , pool_owns_bump){
    tip::memory_pool<tip::default_policy , 2048> pool ;
    EXPECT_TRUE(pool.owns(pool.acquire_bump())) ;
}

TEST(memory_pool , pool_owns_start_ptr){
    tip::memory_pool<tip::default_policy , 2048> pool ;
    EXPECT_TRUE(pool.owns(pool.acquire_internal_pool_start())) ;
}

TEST(memory_pool , single_allocation_pool_out_of_space){
    tip::memory_pool<tip::default_policy , 2> pool ;

    struct alignas(256) UDT{
        char a = 'a' ;
    };

    // This should be returning nullptr
    auto ptr = pool.claim<UDT>() ;

    EXPECT_EQ(ptr , nullptr) ;
}

TEST(memory_pool , multiple_allocations_pool_out_of_space){
    tip::memory_pool<tip::default_policy , 2> pool ;

    struct alignas(256) UDT{
        char a = 'a' ;
    };

    auto ptr = pool.claim_array<UDT>(100) ;

    EXPECT_EQ(ptr , nullptr) ;
}

// Used to reproduce the bug in problems.md
TEST(memory_pool , bump_moves_after_failed_allocate_array){
    tip::memory_pool<tip::default_policy , 256> pool ;

    // Acquire the current bump location
    auto initial_location = pool.acquire_bump() ;

    // Fail to allocate the array
    // Will return the nullptr but the
    // bump will move either way
    [[maybe_unused]] auto failed_allocate_array_ptr = pool.claim_array<double>(1'000) ;

    // Acquire the current bump ptr again
    auto location_after_fail = pool.acquire_bump() ;

    EXPECT_EQ(initial_location , location_after_fail) ;
}

TEST(memory_pool , valid_can_fit){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    // 2048 chars should fit perfectly
    EXPECT_TRUE(pool.can_fit<char>(2048)) ;
}

TEST(memory_pool , invalid_can_fit_off_by_one){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    // 2049 chars should not fit by one
    EXPECT_FALSE(pool.can_fit<char>(2049)) ;
}

TEST(memory_pool , valid_can_fit_normal_request){
    tip::memory_pool<tip::default_policy , 256> pool ;

    EXPECT_TRUE(pool.can_fit<double>(10)) ;
}

TEST(memory_pool , impossible_can_fit){
    tip::memory_pool<tip::default_policy , 1024> pool ;

    struct alignas(2048) box{
        char a = ' ' ;
    };

    // Potential for an awful test if the
    // initial_start_ is aligned for alignof(box)
    // so let's cut the pool size by half
    // (2048 prior to 1024 now)
    EXPECT_FALSE(pool.can_fit<box>()) ;
}

TEST(memory_pool , valid_can_fit_after_claim_array){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    const std::size_t count = 200 ;

    [[maybe_unused]] auto ptr = pool.claim_array<double>(count) ;

    // Worst case scenario , the ptr had to move 7 bytes forward
    // 2048 - 7 = 2041 - 8*200 = 2041 - 1600 = 441
    // 441 / 8 = 55 doubles can fit (and some change)

    EXPECT_TRUE(pool.can_fit<double>(55)) ;
}

TEST(memory_pool , invalid_can_fit_char_after_claim_array){
    tip::memory_pool<tip::default_policy , 2048> pool ;

    [[maybe_unused]] auto ptr = pool.claim_array<char>(2048) ;

    EXPECT_FALSE(pool.can_fit<char>(1)) ;
}

TEST(memory_pool , invalid_can_fit_big_struct_after_claim){
    struct box{
        char data[64] ;
    };

    tip::memory_pool<tip::default_policy , 256> pool ;

    std::size_t count = 3 ;

    [[maybe_unused]] auto initial_construction = pool.claim_array<box>(count) ;

    EXPECT_LE(pool.remaining_capacity() , 256-(3*64)) ;

    EXPECT_FALSE(pool.can_fit<box>(2)) ;
}

// Used to reproduce the bug
TEST(memory_pool , bump_moves_after_failed_claim_array){
    tip::memory_pool<tip::default_policy , 256> pool ;

    // Acquire the current bump location
    auto initial_location = pool.acquire_bump() ;

    std::size_t count = 1'000 ;

    // Fail to allocate the array
    // Will return the nullptr but the
    // bump will move either way
    [[maybe_unused]] auto failed_allocate_array_ptr = pool.claim_array<double>(count) ;

    // Acquire the current bump ptr again
    auto location_after_fail = pool.acquire_bump() ;

    EXPECT_EQ(initial_location , location_after_fail) ;
}
