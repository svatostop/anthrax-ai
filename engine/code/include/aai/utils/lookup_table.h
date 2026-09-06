#pragma once
#include <string>
// LOOKUP TABLES MACRO
    #define GENERATE_ENUM(element, name) element,
    #define GENERATE_STRING(element, name) case element: return name;
    #define GENERATE_KEY(element, name) if (id == name) return element;

    #define DECLARE_LOOKUP_TABLE(TABLE_NAME, ENUM_NAME)                                                                    \
    enum ENUM_NAME                                                                                                         \
    {                                                                                                                      \
        TABLE_NAME(GENERATE_ENUM)                                                                                          \
    };                                                                                                                     \
                                                                                                                           \
    inline const std::string get_value(ENUM_NAME id)                                                                             \
    {                                                                                                                      \
        switch (id)                                                                                                        \
        {                                                                                                                  \
            TABLE_NAME(GENERATE_STRING)                                                                               \
        default:                                                                                                           \
            return "undef";                                                                                                \
        }                                                                                                                  \
    }                                                                                                                       \
    inline ENUM_NAME get_key(std::string id)                                                                                \
    {                                                                                                                       \
        TABLE_NAME(GENERATE_KEY)                                                                                            \
        return SIZE;                                                                                                        \
    }                                                                                                                       \
    //------------------


