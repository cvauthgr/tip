#pragma once

namespace tip {

// Just a thought for now

struct default_policy{
    // Pool options
    static constexpr bool bounds_checks     = true ;
    static constexpr bool ownership_checks  = true ;
    static constexpr bool can_fit_checks    = true ;
    static constexpr bool nullptr_checks    = true ;

    // Allocator options
    static constexpr bool size_check_validations = true  ;
    static constexpr bool fallback_resource      = false ;
    static constexpr bool upstream_resource      = false ;

};

struct hardened_policy : default_policy {
    // Pool options
    static constexpr bool bounds_checks     = true ;
    static constexpr bool ownership_checks  = true ;
    static constexpr bool can_fit_checks    = true ;
    static constexpr bool nullptr_checks    = true ;

    // Allocator options
    static constexpr bool size_check_validations = true ;
    static constexpr bool fallback_resource      = true ;
    static constexpr bool upstream_resource      = true ;
};

struct unsafe : default_policy {
    // Pool options
    static constexpr bool bounds_check      = false ;
    static constexpr bool ownership_checks  = false ;
    static constexpr bool can_fit_checks    = false ;
    static constexpr bool nullptr_checks    = false ;

    // Allocator options
    static constexpr bool size_check_validations = false ;
};

}// End of the tip namespace
