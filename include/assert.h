#ifndef BASIC_OPENGL_ASSERT_H
#define BASIC_OPENGL_ASSERT_H

#include <cstdio>

#define Assert(expr, assert_msg, ...)                     \
    do                                                    \
    {                                                     \
        if (!(expr))                                      \
        {                                                 \
            char debug_msg[250];                          \
            sprintf(debug_msg, assert_msg, __VA_ARGS__); \
            OutputDebugString(debug_msg);                 \
            __debugbreak();                               \
        }                                                 \
    } while(false)

#endif // BASIC_OPENGL_ASSERT_H
