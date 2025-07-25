#pragma once

#define ADT_WARN_INIT [[deprecated("warning: should be initialized with (INIT)")]]
#define ADT_WARN_DONT_USE [[deprecated("warning: don't use!")]]
#define ADT_WARN_IMPOSSIBLE_OPERATION [[deprecated("warning: imposibble operation")]]

#if defined __clang__ || __GNUC__
    #define ADT_NO_UB __attribute__((no_sanitize("undefined")))
    #define ADT_LOGS_FILE __FILE_NAME__
    #define ADT_FUNC_SIG __PRETTY_FUNCTION__
#else
    #define ADT_NO_UB
    #define ADT_LOGS_FILE __FILE__
    #define ADT_FUNC_SIG __FUNCSIG__
#endif

#if defined __clang__ || __GNUC__
    #define ADT_ALWAYS_INLINE inline __attribute__((__always_inline__))
#elif defined _WIN32
    #define ADT_ALWAYS_INLINE inline __forceinline
#else
    #define ADT_ALWAYS_INLINE inline
#endif

#if defined _WIN32
    #define ADT_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define ADT_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
