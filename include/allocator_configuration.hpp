#pragma once

// Macro definitions for different allocator
// options , such as the upstream source or
// if the fallback resource should be enabled

// Upstream resource
//
// Set the default to operator new

#define UPSTREAM_RESOURCE_MALLOC

#if !defined UPSTREAM_SOURCE_MALLOC && !defined UPSTREAM_SOURCE_MMAP
    #define UPSTREAM_SOURCE_NEW
#endif

#if !defined UPSTREAM_SOURCE_NEW && !defined UPSTREAM_SOURCE_MMAP
    #define UPSTREAM_SOURCE_MALLOC
#endif

#if !defined UPSTREAM_RESOURCE_NEW && !defined UPSTREAM_RESOURCE_MALLOC
    #define UPSTREAM_SOURCE_MMAP
#endif

// Fallback resource
//
// Set the default to TRUE

#define FALLBACK_RESOURCE_ENABLE

#ifndef FALLBACK_RESOURCE_ENABLE
    #define FALLFALLBACK_RESOURCE_DISABLE
#endif
