/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cassert>
#include <cstdlib>

#include <Logging/Logger.hpp>

static void arcAssertFailed(const char* fname, int line, const char* expr)
{
    sLogger.failure("Assertion Failed: ({})", expr);
    sLogger.failure("Location: {}({})", fname, line);
}

#define ANALYSIS_ASSUME(EXPR)

//////////////////////////////////////////////////////////////////////////////////////////
// an assert isn't necessarily fatal, but we want to stop anyways
#define WPAssert(EXPR)                                                         \
    do                                                                         \
    {                                                                          \
        if (!(EXPR))                                                           \
        {                                                                      \
            arcAssertFailed(__FILE__, __LINE__, #EXPR);                        \
            ((void(*)())0)();                                                  \
        }                                                                      \
    } while (0);                                                               \
    ANALYSIS_ASSUME(EXPR)

#define WPError(assertion, errmsg)                                             \
    do                                                                         \
    {                                                                          \
        if (!(assertion))                                                      \
        {                                                                      \
            sLogger.failure("{}:{} ERROR:\n  {}",                              \
                __FILE__, __LINE__, errmsg);                                   \
            assert(false);                                                     \
        }                                                                      \
    } while (0)

#define WPWarning(assertion, errmsg)                                           \
    do                                                                         \
    {                                                                          \
        if (!(assertion))                                                      \
        {                                                                      \
            sLogger.warning("{}:{} WARNING:\n  {}",                            \
                __FILE__, __LINE__, errmsg);                                   \
        }                                                                      \
    } while (0)

//////////////////////////////////////////////////////////////////////////////////////////
// this should always halt everything.
// if you ever find yourself wanting to remove the assert(false),
// switch to WPWarning or WPError
#define WPFatal(assertion, errmsg)                                             \
    do                                                                         \
    {                                                                          \
        if (!(assertion))                                                      \
        {                                                                      \
            sLogger.failure("{}:{} FATAL ERROR:\n  {}",                        \
                __FILE__, __LINE__, errmsg);                                   \
            assert(#assertion && 0);                                           \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

#define ASSERT WPAssert
