#pragma once

#include <type_traits>

/* https://voithos.io/articles/enum-class-bitmasks/ */
/* Define bitwise operators for an enum class, allowing usage as bitmasks. */
#define ADT_ENUM_BITWISE_OPERATORS(ENUM_T)                                                                             \
    inline constexpr ENUM_T operator|(ENUM_T l, ENUM_T r)                                                              \
    {                                                                                                                  \
        return static_cast<ENUM_T>(                                                                                    \
            static_cast<std::underlying_type_t<ENUM_T>>(l) | static_cast<std::underlying_type_t<ENUM_T>>(r)            \
        );                                                                                                             \
    }                                                                                                                  \
    inline constexpr ENUM_T operator&(ENUM_T l, ENUM_T r)                                                              \
    {                                                                                                                  \
        return static_cast<ENUM_T>(                                                                                    \
            static_cast<std::underlying_type_t<ENUM_T>>(l) & static_cast<std::underlying_type_t<ENUM_T>>(r)            \
        );                                                                                                             \
    }                                                                                                                  \
    inline constexpr ENUM_T operator^(ENUM_T l, ENUM_T r)                                                              \
    {                                                                                                                  \
        return static_cast<ENUM_T>(                                                                                    \
            static_cast<std::underlying_type_t<ENUM_T>>(l) ^ static_cast<std::underlying_type_t<ENUM_T>>(r)            \
        );                                                                                                             \
    }                                                                                                                  \
    inline constexpr ENUM_T operator~(ENUM_T E)                                                                        \
    {                                                                                                                  \
        return static_cast<ENUM_T>(~static_cast<std::underlying_type_t<ENUM_T>>(E));                                   \
    }                                                                                                                  \
    inline constexpr ENUM_T& operator|=(ENUM_T& l, ENUM_T r)                                                           \
    {                                                                                                                  \
        return l = static_cast<ENUM_T>(                                                                                \
                   static_cast<std::underlying_type_t<ENUM_T>>(l) | static_cast<std::underlying_type_t<ENUM_T>>(l)     \
               );                                                                                                      \
    }                                                                                                                  \
    inline constexpr ENUM_T& operator&=(ENUM_T& l, ENUM_T r)                                                           \
    {                                                                                                                  \
        return l = static_cast<ENUM_T>(                                                                                \
                   static_cast<std::underlying_type_t<ENUM_T>>(l) & static_cast<std::underlying_type_t<ENUM_T>>(l)     \
               );                                                                                                      \
    }                                                                                                                  \
    inline constexpr ENUM_T& operator^=(ENUM_T& l, ENUM_T r)                                                           \
    {                                                                                                                  \
        return l = static_cast<ENUM_T>(                                                                                \
                   static_cast<std::underlying_type_t<ENUM_T>>(l) ^ static_cast<std::underlying_type_t<ENUM_T>>(l)     \
               );                                                                                                      \
    }
