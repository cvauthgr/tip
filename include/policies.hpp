#pragma once

namespace tip {

// Just a thought for now

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

struct resilient_policy : default_policy {
    // Pool options
    static constexpr bool ownership_checks  = true ;
    static constexpr bool can_fit_checks    = true ;
    static constexpr bool nullptr_checks    = true ;

    // Allocator options
    static constexpr bool size_check_validations = true ;
    static constexpr bool upstream_resource      = true ;
    static constexpr bool fallback_to_new        = true ;
    static constexpr bool easy_crash             = true ;
};

struct unsafe : default_policy {
    // Pool options
    static constexpr bool ownership_checks  = false ;
    static constexpr bool can_fit_checks    = false ;
    static constexpr bool nullptr_checks    = false ;

    // Allocator options
    static constexpr bool size_check_validations = false ;
    static constexpr bool pool_chaining          = false ;
    static constexpr bool fallback_to_new        = false ;
};

struct permissive_policy : default_policy {
    static constexpr bool fallback_to_new = true ;
};

}// End of the tip namespace
