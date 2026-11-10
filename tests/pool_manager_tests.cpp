#include <cstddef>
#include <gtest/gtest.h>

#include "allocator.hpp"
#include "policies.hpp"

TEST(pool_manager , pool_chaining_on_out_of_space_allocate){
    constexpr std::size_t size = 2 ;
    constexpr std::size_t max_pools_chained = 10 ;

    tip::allocator<tip::monotonic_policy , size, max_pools_chained> allocator ;

    // We can fit 2 chars
    EXPECT_EQ(allocator.remaining_capacity() , 2) ;
    auto char_1 = allocator.allocate<char>() ;
    EXPECT_NE(char_1 , nullptr) ;
    EXPECT_EQ(allocator.remaining_capacity() , 1) ;
    auto char_2 = allocator.allocate<char>() ;
    EXPECT_NE(char_2 , nullptr) ;

    // The pool should have 0 space left
    EXPECT_EQ(allocator.remaining_capacity() , 0) ;

    // Access the characters and asign them
    // This way we can check if they are still
    // valid to access after we move to the second pool
    // (which should be possible to access)

    *char_1 = 'a' ;
    *char_2 = 'b' ;

    // Grab the pool's internal start to see if it changed
    const void* old_internal_start = allocator.expose_internal_pool_start() ;

    // Push over the limit and allow the pool to chain
    auto next_pool_character = allocator.allocate<char>() ;

    // First the pointer should be not null
    // otherwise explode (literally)
    ASSERT_NE(next_pool_character , nullptr) ;
    // Also the capacity should be equal to the
    // size of the initial allocator pool minus
    // the character we just allocated (sizeof(char) = 1)
    EXPECT_EQ(allocator.remaining_capacity(), 1);
    // The internal starts should also be swapped
    // but first make sure the new start is not nullptr
    EXPECT_NE(allocator.expose_internal_pool_start() , nullptr);
    EXPECT_NE(old_internal_start , allocator.expose_internal_pool_start()) ;

    // This is why we must explode
    // Set the char we allocated
    *next_pool_character = 'c' ;

    // Now check if everything is :
    // 1) Accesible
    // 2) Valid as it was set

    EXPECT_EQ(*char_1 , 'a');
    EXPECT_EQ(*char_2 , 'b');
    EXPECT_EQ(*next_pool_character , 'c');

}

TEST(pool_manager , pool_chaining_on_out_of_space_allocate_array){
    constexpr std::size_t size = 128 ;
    constexpr std::size_t max_pools_chained = 10 ;

    tip::allocator<tip::monotonic_policy , size, max_pools_chained> allocator ;

    EXPECT_EQ(allocator.remaining_capacity() , size) ;
    auto char_sequence_1 = allocator.allocate_array<char>(100) ;
    EXPECT_NE(char_sequence_1 , nullptr) ;
    EXPECT_EQ(allocator.remaining_capacity() , 28) ;
    auto char_sequence_2 = allocator.allocate_array<char>(28) ;
    EXPECT_NE(char_sequence_2 , nullptr) ;

    // The pool should have 0 space left
    EXPECT_EQ(allocator.remaining_capacity() , 0) ;

    // Access the characters and asign them
    // This way we can check if they are still
    // valid to access after we move to the second pool
    // (which should be possible to access)

    for(std::size_t i = {0uz} ; i < 100 ; ++i){
        char_sequence_1[i] = 'a' ;
    }

    for(std::size_t i = {0uz} ; i < 28 ; ++i){
        char_sequence_2[i] = 'b' ;
    }

    // Grab the pool's internal start to see if it changed
    const void* old_internal_start = allocator.expose_internal_pool_start() ;

    // Push over the limit and allow the pool to chain
    auto next_pool_character_sequence = allocator.allocate_array<char>(125) ;

    // First the pointer should be not null
    // otherwise explode (literally)
    ASSERT_NE(next_pool_character_sequence , nullptr) ;
    // Also the capacity should be equal to the
    // size of the initial allocator pool minus
    // the character we just allocated (125*sizeof(char) = 125)
    EXPECT_EQ(allocator.remaining_capacity(), 3);
    // The internal starts should also be swapped
    // but first make sure the new start is not nullptr
    EXPECT_NE(allocator.expose_internal_pool_start() , nullptr);
    EXPECT_NE(old_internal_start , allocator.expose_internal_pool_start()) ;

    // This is why we must explode
    // Set the char we allocated
    for(std::size_t i = {0uz} ; i < 125 ; ++i){
            next_pool_character_sequence[i] = 'c' ;
    }


    // Now check if everything is :
    // 1) Accesible
    // 2) Valid as it was set

    for(std::size_t i = {0uz} ; i < 100 ; ++i){
        EXPECT_EQ(char_sequence_1[i] , 'a');
    }

    for(std::size_t i = {0uz} ; i < 28 ; ++i){
        EXPECT_EQ(char_sequence_2[i] , 'b');
    }

    for(std::size_t i = {0uz} ; i < 100 ; ++i){
        EXPECT_EQ(next_pool_character_sequence[i] , 'c');
    }

}

TEST(pool_manager , pool_chaining_exhaustion){
    // Execute pool chaining untill the pool
    // manager stops it

    tip::allocator<tip::monotonic_policy, 128, 10> allocator ;

    // We can freely allocate 128*10 = 1280 chars
    // I will save one per pool and initialize it
    // Then I will allocate once more . We should get
    // a nullptr + all the chars should be alive and
    // accessible

    auto c1 = allocator.allocate_array<char>(128) ;
    auto c2 = allocator.allocate_array<char>(128) ;
    auto c3 = allocator.allocate_array<char>(128) ;
    auto c4 = allocator.allocate_array<char>(128) ;
    auto c5 = allocator.allocate_array<char>(128) ;
    auto c6 = allocator.allocate_array<char>(128) ;
    auto c7 = allocator.allocate_array<char>(128) ;
    auto c8 = allocator.allocate_array<char>(128) ;
    auto c9 = allocator.allocate_array<char>(128) ;
    auto c10 = allocator.allocate_array<char>(128) ;

    ASSERT_NE(c1, nullptr) ;
    ASSERT_NE(c2, nullptr) ;
    ASSERT_NE(c3, nullptr) ;
    ASSERT_NE(c4, nullptr) ;
    ASSERT_NE(c5, nullptr) ;
    ASSERT_NE(c6, nullptr) ;
    ASSERT_NE(c7, nullptr) ;
    ASSERT_NE(c8, nullptr) ;
    ASSERT_NE(c9, nullptr) ;
    ASSERT_NE(c10, nullptr) ;

    // Initialize all of them
    for(std::size_t i = {0uz} ; i < 128 ; ++i){
        c1[i] = 'a' ;
        c2[i] = 'a' ;
        c3[i] = 'a' ;
        c4[i] = 'a' ;
        c5[i] = 'a' ;
        c6[i] = 'a' ;
        c7[i] = 'a' ;
        c8[i] = 'a' ;
        c9[i] = 'a' ;
        c10[i] = 'a' ;
    }

    // Try to allocate one more char
    // This should return a nullptr
    auto fail = allocator.allocate<char>() ;
    EXPECT_EQ(fail , nullptr) ;
    // Did we run out of pools ?

    // Check if everyone is alive still
    for(std::size_t i = {0uz} ; i < 128 ; ++i){
        EXPECT_TRUE( c1[i] == 'a') ;
        EXPECT_TRUE( c2[i] == 'a') ;
        EXPECT_TRUE( c3[i] == 'a') ;
        EXPECT_TRUE( c4[i] == 'a') ;
        EXPECT_TRUE( c5[i] == 'a') ;
        EXPECT_TRUE( c6[i] == 'a') ;
        EXPECT_TRUE( c7[i] == 'a') ;
        EXPECT_TRUE( c8[i] == 'a') ;
        EXPECT_TRUE( c9[i] == 'a') ;
        EXPECT_TRUE(c10[i] == 'a') ;
    }
}
