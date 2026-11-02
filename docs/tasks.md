# Major refactoring 

1) The memory pool class is nice but having another class handling a memory pool 
and exposing the ``construct()`` and ``construct_array()`` API's would be better 

2) The pool and allocator class constructors need to accept parameters for the pool
profile (more info soon)

### Things I am planning to add (God forbid a man has plans)

- ``get_high_mark()`` . The allocator upon construction allocates a secondary memory region 
that it internally handles . There is saves upon specific API calls the current usage to return 
if asked . Helps determine the highest usage of the pool in it's lifetime . Overhead is expected 
so a parameter must be passed to the constructor . Something like ``PROFILE::ENABLE_USAGE_TRACKING``
