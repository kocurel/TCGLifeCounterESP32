#ifndef ASSERT_H
#define ASSERT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// CONFIGURATION: Determine Action on Failure
// -----------------------------------------------------------------------------
// NDEBUG is a standard C flag defined automatically by CMake/Compilers
// when you build in "Release" mode.
//
// - If NDEBUG is NOT defined (Development): We ABORT (Crash).
// - If NDEBUG IS defined (Production): We just LOG and continue.
// -----------------------------------------------------------------------------

#ifndef NDEBUG
// Development: Stop everything so we can debug
#define _ASSERT_ACTION() abort()
#define _ASSERT_TAG "[ASSERT FAILED]"
#else
// Production: Log it, but try to keep the game running
#define _ASSERT_ACTION() ((void)0)
#define _ASSERT_TAG "[ASSERT WARNING]"
#endif

// -----------------------------------------------------------------------------
// Internal Helper: Handles the error reporting and action
// -----------------------------------------------------------------------------
#define _ASSERT_FAIL(file, line, expr, fmt, ...)                  \
    do {                                                          \
        fprintf(stderr, "\n%s %s:%d\n", _ASSERT_TAG, file, line); \
        fprintf(stderr, "    Expr: %s\n", expr);                  \
        fprintf(stderr, "    Msg:  " fmt "\n", ##__VA_ARGS__);    \
        _ASSERT_ACTION();                                         \
    } while (0)

// -----------------------------------------------------------------------------
// Numeric Assertions (Int32 based)
// -----------------------------------------------------------------------------

#define ASSERT_EQUALS(expected, actual)                                    \
    do {                                                                   \
        int32_t _e = (int32_t)(expected);                                  \
        int32_t _a = (int32_t)(actual);                                    \
        if (_e != _a) {                                                    \
            _ASSERT_FAIL(__FILE__, __LINE__, #expected " == " #actual,     \
                         "Expected %ld, but got %ld", (long)_e, (long)_a); \
        }                                                                  \
    } while (0)

#define ASSERT_NOT_EQUALS(expected, actual)                              \
    do {                                                                 \
        int32_t _e = (int32_t)(expected);                                \
        int32_t _a = (int32_t)(actual);                                  \
        if (_e == _a) {                                                  \
            _ASSERT_FAIL(__FILE__, __LINE__, #expected " != " #actual,   \
                         "Expected different values, but both were %ld", \
                         (long)_e);                                      \
        }                                                                \
    } while (0)

#define ASSERT_GREATER(x, y)                                          \
    do {                                                              \
        if (!((x) > (y))) {                                           \
            _ASSERT_FAIL(__FILE__, __LINE__, #x " > " #y,             \
                         "Expected %ld > %ld", (long)(x), (long)(y)); \
        }                                                             \
    } while (0)

#define ASSERT_GREATER_EQUAL(x, y)                                     \
    do {                                                               \
        if (!((x) >= (y))) {                                           \
            _ASSERT_FAIL(__FILE__, __LINE__, #x " >= " #y,             \
                         "Expected %ld >= %ld", (long)(x), (long)(y)); \
        }                                                              \
    } while (0)

#define ASSERT_LESSER(x, y)                                           \
    do {                                                              \
        if (!((x) < (y))) {                                           \
            _ASSERT_FAIL(__FILE__, __LINE__, #x " < " #y,             \
                         "Expected %ld < %ld", (long)(x), (long)(y)); \
        }                                                             \
    } while (0)

#define ASSERT_LESSER_EQUAL(x, y)                                      \
    do {                                                               \
        if (!((x) <= (y))) {                                           \
            _ASSERT_FAIL(__FILE__, __LINE__, #x " <= " #y,             \
                         "Expected %ld <= %ld", (long)(x), (long)(y)); \
        }                                                              \
    } while (0)

// -----------------------------------------------------------------------------
// Pointer & Boolean Assertions
// -----------------------------------------------------------------------------

#define ASSERT_NOT_NULL(ptr)                                  \
    do {                                                      \
        if ((ptr) == NULL) {                                  \
            _ASSERT_FAIL(__FILE__, __LINE__, #ptr " != NULL", \
                         "Pointer was NULL");                 \
        }                                                     \
    } while (0)

#define ASSERT_NULL(ptr)                                                      \
    do {                                                                      \
        if ((ptr) != NULL) {                                                  \
            _ASSERT_FAIL(__FILE__, __LINE__, #ptr " == NULL",                 \
                         "Pointer was NOT NULL (Address: %p)", (void*)(ptr)); \
        }                                                                     \
    } while (0)

#define ASSERT_TRUE(condition)                           \
    do {                                                 \
        if (!(condition)) {                              \
            _ASSERT_FAIL(__FILE__, __LINE__, #condition, \
                         "Expected true, got false");    \
        }                                                \
    } while (0)

#define ASSERT_FALSE(condition)                          \
    do {                                                 \
        if (condition) {                                 \
            _ASSERT_FAIL(__FILE__, __LINE__, #condition, \
                         "Expected false, got true");    \
        }                                                \
    } while (0)

// -----------------------------------------------------------------------------
// String Assertions
// -----------------------------------------------------------------------------

#define ASSERT_STR_EQUALS(s1, s2)                                  \
    do {                                                           \
        const char* _s1 = (s1);                                    \
        const char* _s2 = (s2);                                    \
        if (_s1 == NULL || _s2 == NULL || strcmp(_s1, _s2) != 0) { \
            _ASSERT_FAIL(__FILE__, __LINE__, #s1 " == " #s2,       \
                         "Expected \"%s\", but got \"%s\"",        \
                         _s1 ? _s1 : "NULL", _s2 ? _s2 : "NULL");  \
        }                                                          \
    } while (0)

#endif  // ASSERT_H