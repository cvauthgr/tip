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
