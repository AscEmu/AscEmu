/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdlib>
#include <string_view>

#include "Debugging/CrashHandler.hpp"
#include "Logging/Logger.hpp"

inline void assertFailed(const char* fileName, int line, const char* expression)
{
    sLogger.failure("Assertion failed: ({})", expression);
    sLogger.failure("Location: {}({})", fileName, line);
    AscEmu::Debugging::requestCrashDiagnostics("Assertion diagnostics");
}

#define ANALYSIS_ASSUME(EXPR)

#define WPAssert(EXPR)                                                               \
    do                                                                               \
    {                                                                                \
        if (!(EXPR))                                                                 \
        {                                                                            \
            assertFailed(__FILE__, __LINE__, #EXPR);                              \
            std::abort();                                                            \
        }                                                                            \
    } while (0)                                                                     \
    ANALYSIS_ASSUME(EXPR)

#define WPError(assertion, errmsg)                                                   \
    do                                                                               \
    {                                                                                \
        if (!(assertion))                                                            \
        {                                                                            \
            sLogger.failure("{}:{} ERROR:\n  {}", __FILE__, __LINE__, errmsg);       \
            AscEmu::Debugging::requestCrashDiagnostics("WPError diagnostics");       \
            std::abort();                                                            \
        }                                                                            \
    } while (0)

#define WPWarning(assertion, errmsg)                                                 \
    do                                                                               \
    {                                                                                \
        if (!(assertion))                                                            \
        {                                                                            \
            sLogger.warning("{}:{} WARNING:\n  {}", __FILE__, __LINE__, errmsg);     \
        }                                                                            \
    } while (0)

#define WPFatal(assertion, errmsg)                                                   \
    do                                                                               \
    {                                                                                \
        if (!(assertion))                                                            \
        {                                                                            \
            sLogger.failure("{}:{} FATAL ERROR:\n  {}", __FILE__, __LINE__, errmsg); \
            AscEmu::Debugging::terminateWithDiagnostics("WPFatal diagnostics");      \
        }                                                                            \
    } while (0)

#define ASSERT WPAssert
