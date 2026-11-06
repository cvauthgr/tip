I need some way to implement the configuration profiles I wanted .

What is a configuration profile ?

- It is a struct containing some bools which define what the allocator and memory pool are allowed to do 

Example : (Allocator profile)

- If the HARDENED allocator profile is passed then the allocator forces the pool and itself to enable 
a bunch of different checks : nullptr checks from the allocate API , bounds checking , ownership checks for 
the ptr through the pool and can_fit() checks 

- If the UNSAFE allocator profile is passed then the allocator disables all the checks and the user is 100% responsible for any checks . By using this profile the compiler optimizes away all the API calls that are resolved
at compile time through ``if constexpr`` branching 

**Needs to be updated**
