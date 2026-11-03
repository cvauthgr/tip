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
    static constexpr bool use_reserve            = true  ;
    static constexpr bool upstream_resource      = false ;
    static constexpr bool pool_chaining          = false ;

};

struct hardened_policy : default_policy {
    // Pool options
    static constexpr bool ownership_checks  = true ;
    static constexpr bool can_fit_checks    = true ;
    static constexpr bool nullptr_checks    = true ;

    // Allocator options
    static constexpr bool size_check_validations = true ;
    static constexpr bool use_reserve            = true ;
    static constexpr bool upstream_resource      = true ;
};

struct unsafe : default_policy {
    // Pool options
    static constexpr bool ownership_checks  = false ;
    static constexpr bool can_fit_checks    = false ;
    static constexpr bool nullptr_checks    = false ;

    // Allocator options
    static constexpr bool size_check_validations = false ;
    static constexpr bool pool_chaining          = true  ;
};

}// End of the tip namespace
