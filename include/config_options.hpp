#pragma once

namespace tip {

// Just a thought for now

enum class allocator_options{
    ENABLE_FALLBACK_RESOURCE,
    DISABLE_FALLBACK_RESOURCE,
    ENABLE_USAGE_TRACKING,
    DISABLE_USAGE_TRACKING,
    NULLPTR_CHECKS,
    PROMPT_ON_FAILURE,
    ENABLE_LOGGING,
    DISABLE_LOGGING,
    BOUNDS_CHECKS,
    OWNERSHIP_CHECKS,
    CAN_FIT_CHECKS,
};

enum class allocator_profiles{
    MINIMAL,
    HARDENED,
    UNSAFE,
    ENABLE_ALL,
    DISABLE_ALL,
};

}// End of the tip namespace
