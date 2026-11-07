# Contributing to tip

Before everything please read this : https://en.wikipedia.org/wiki/Region-based_memory_management

Every contribution at this stage of deployment is very much welcome 

**Areas in need of expansion** :

- Benchmarks for most APIs
- Tests for the new additions (soon the pool manager) and also the allocator
- Implementing new policies
- A custom `std::align` with no size checks (maybe?)

## Architecture 

Here I will try to explain the architecture behind **tip** as simply as possibly.

As of now there are two main classes. The **memory_pool** class and the **allocator** class

### The memory_pool class

Allocates a N bytes raw memory region on construction and deletes it on destruction through RAII

The most important APIs are `allocate` and `allocate_array` 

They align the bump pointer according to `alignof(Object)` and the if the object can fit 
decrease the capacity (padding+sizeof(Object)) and return the aligned pointer . The bump pointer
is moved to the end of the region the Object will occupy 

`allocate_array` is the same thing just for many objects

**Important** : POD data only (for now)

### The allocator class 

This class handles a memory pool and exposes the APIs `allocate_and_construct` and `allocate_and_construct_array` which call the `allocate` and `allocate_array` APIs internally 
and construct the Objects at their locations respectively .

# Policies 

Template arguments passed into the classes which dictate at compile time the structure of the member functions that we use 

## What is a Policy really ?

A policy is a struct of boolean values like this :

```cpp
struct default_policy{
    // Pool options
    static constexpr bool ownership_checks  = true ;
    static constexpr bool can_fit_checks    = true ;
    static constexpr bool nullptr_checks    = true ;

    // Allocator options
    static constexpr bool size_check_validations = true  ;
    static constexpr bool upstream_resource      = false ;
    static constexpr bool pool_chaining          = false ;
    static constexpr bool fallback_to_new        = false ;
    static constexpr bool easy_crash             = false ;
};
```

Example : Assume that `nullptr_checks` is `false`

Then inside a (fake) API this is what will happen :

Before compilation :

```cpp
if constrexpr(nullptr_checks){
    if(is_nullptr(ptr)){
        return false ;
    }
}
```

The compiler can determine at compile time if the branch is used and immediately optimise it away . The function may look bloated but through different policy options only a tiny amount remains there at runtime

# Additions

I am currently working on implementing a pool manager which will allow for pool chaining and a logging background thread to allow for easier debugging.
