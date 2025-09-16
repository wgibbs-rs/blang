#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #define GCC_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define GCC_UNLIKELY(x) __builtin_expect(!!(x), 0)
    #define GCC_PURE        __attribute__((pure))
    #define GCC_CONST       __attribute__((const))
    #define GCC_NORETURN    __attribute__((noreturn))
    #define GCC_COLD        __attribute__((cold))
    #define GCC_HOT         __attribute__((hot))
#else
    #define GCC_LIKELY(x)   (x)
    #define GCC_UNLIKELY(x) (x)
    #define GCC_PURE
    #define GCC_CONST
    #define GCC_NORETURN
    #define GCC_COLD
    #define GCC_HOT
#endif