->The functions ``allocate_array()`` and ``allocate_and_construct_array()`` have a major flaw .

The ``allocate()`` function is called and afterwards if not all objects can fit in the memory pool
we return ``nullptr`` . That is no problem as we either do what the API must do or return indicating failure.

__Problem__ : After the first ``allocate()`` call at failure the bump pointer has been moved and capacity 
has been decremented even for a failure 

__Solution__ : Implement a lightweight templated internal API . Let's call it ``can_fit()`` with a single
argument

1) The object count -> Default function parameter set to 1 so we can reuse it in ``allocate_array()`` and in any other
API we may need it in.

We will use the templated argument representing the type to exctract the ``sizeof(Obj)`` and ``alignof(Obj)`` and 
by hand-alignment and through the ``owns()`` API which checks if the pool owns a pointer decide if the ``allocate()`` 
call makes sense . 

__Conclusion__ : This way we won't be losing valuable memory space to failing API calls . 

**Date** : 30/10/2026
**Author** : Binos

__DISCLAIMER TO SELF__ : Test before making dumb assumptions . Somehow I can't reproduce the bug (what?)
I was using a very small pool size (2 bytes) so allocate was returning nullptr , this way because std::align 
was returning nullptr the other functions I mentioned didn't misbehave as I already implemented a check for 
nullptr...I surely have a **brain** problem.

Although I thought it would be dumb , the API design now handles the pool bounds perfectly , by not getting 
capacity in the way we are way more flexible and safe in determining if an object can fit or not

--------------------------------------------------------------------------------------------------------------------

The last test for ``can_fit()`` is failing . I should fix it before adding it to the functions I mentioned above!

I tested the ``owns()`` API a little more and revealed a very strong feature I didn't intent to add . It also detects the 
first and last valid adresses in the pool as valid which is cool . Wrote some more tests for it and everything seems to be 
working . The last suspect as to why ``can_fit()`` are those gnarly ``reinterpet_cast``'s I used and the use of ``uintptr_t`` . Looking at stack overflow people don't really seem to be font of the idea of doing it __this way__

__Conclusion__ : Rewriting may help (also God may help me at this point!)

Before :

```cpp
// Checks if an array allocation is possible
// More in problems.md
template <typename Obj>
[[nodiscard]] inline constexpr auto can_fit(std::size_t count = 1) const noexcept -> bool {
    // First convert the pointer to a uintptr_t
    auto ptr_to_align = reinterpret_cast<uintptr_t>(bump_ptr_) ;
    // Align it in accordance to the alignment of the object
    auto aligned_ptr = (ptr_to_align + (alignof(Obj) - 1)) & ~(alignof(Obj) - 1) ;
    // Check if we are in the pool after alignment
    if(this->owns(reinterpret_cast<const void*>(aligned_ptr)) && this->owns(reinterpret_cast<const void*>(aligned_ptr+count))){
        // If for count we are still inside
        // everything is nice
        return true ;
    }

    return false ;
}
```

After : 

```cpp
        // Checks if an array allocation is possible
        // More in problems.md
        template <typename Obj>
        [[nodiscard]] inline constexpr auto can_fit(std::size_t count = 1) const noexcept -> bool {
            // Acquire the current location of the bump pointer
            // then bit_cast it to a uintptr_t to align it
            auto aligned_ptr = std::bit_cast<uintptr_t>(bump_ptr_) ;
            // Align it in accordance to the alignemt of Obj
            aligned_ptr = (aligned_ptr + (alignof(Obj) - 1)) & ~(alignof(Obj) - 1) ;
            // Check if the aligned ptr and the end pointer of the allocation
            // are owned by the pool
            if(this->owns(std::bit_cast<const void*>(aligned_ptr)) && this->owns(std::bit_cast<const void*>(aligned_ptr+count))){
                return true ;
            }
            return false ;
        }
```

This implementation is wrong too , but I have another idea . Why try to reinvent the universe when we have ``std::align()``?
I can use it to safely get the aligned pointer by passing in the desired data + a virtual insanely big buffer (irrelevant to our goal , only doing it because otherwise we will get a nullptr!)

__Final working version (hopefully)__ :

```cpp
// Checks if an array allocation is possible
        // More in problems.md
        template <typename Obj>
        [[nodiscard]] inline constexpr auto can_fit(std::size_t count = 1) const noexcept -> bool {
            // Make an unavoidable copy of the bump ptr because the call to std::align
            // per our circumstances is GUARANTEED to move it
            auto bump_copy = bump_ptr_ ;
            // Create a virtual buffer so std::align can play without messing with the pool's
            // internal state
            size_t virtual_buffer = static_cast<size_t>(-1) ;
            auto aligned_ptr = std::align(alignof(Obj) , sizeof(Obj)*count , bump_copy , virtual_buffer) ;
            if(this->owns(aligned_ptr) && this->owns(static_cast<Obj*>(aligned_ptr)+count)){
                return true  ;
            }

            return false ;
        }
```

__The goddamn tests are passing hell yeahhhhhhhhhhh__

__Next step__ : Let's integrate the ``can_fit()`` API to the functions that were deemed as not working correctly . 
For it to be considered a success we need to see the 2 tests about the bump changing position after a failed call 
to allocate to both __fail__
