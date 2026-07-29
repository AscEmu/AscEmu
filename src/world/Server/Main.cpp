/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Server/Master.h"
#include "ServerState.h"
#include "Debugging/CrashHandler.hpp"
#include "Debugging/StackTrace.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <exception>

#ifndef _WIN32
#include <sys/resource.h>
#endif

namespace
{
    void installWorldCrashHandler()
    {
        AscEmu::Debugging::CrashHandlerOptions crashHandlerOptions{};
        crashHandlerOptions.processKind = AscEmu::Debugging::ProcessKind::world;
        AscEmu::Debugging::installCrashHandler(crashHandlerOptions);
    }

    int runWorldCrashTestStack()
    {
        const auto frames = AscEmu::Debugging::StackTrace::capture(0, 32);
        if (frames.empty())
        {
            std::fprintf(stderr, "[world] crash test failed: no frames captured\n");
            return EXIT_FAILURE;
        }

        const std::string formatted = AscEmu::Debugging::StackTrace::format(frames);
        if (formatted.empty())
        {
            std::fprintf(stderr, "[world] crash test failed: formatted stack trace is empty\n");
            return EXIT_FAILURE;
        }

        std::fprintf(stderr, "[world] stack trace smoke test passed\n%s\n", formatted.c_str());
        AscEmu::Debugging::requestCrashDiagnostics("world manual stack smoke test");
        return EXIT_SUCCESS;
    }

    [[noreturn]] void runWorldCrashTestTerminate()
    {
        AscEmu::Debugging::terminateWithDiagnostics("world manual terminate test");
    }

    [[noreturn]] void runWorldCrashTestSegv()
    {
        volatile int* invalidPointer = nullptr;
        *invalidPointer = 42;
        std::abort();
    }

    [[maybe_unused]] int tryRunWorldCrashTests(std::string_view test)
    {
        if (test == "--crash-test-stack")
            return runWorldCrashTestStack();

        if (test == "--crash-test-terminate")
            runWorldCrashTestTerminate();

        if (test == "--crash-test-segv")
            runWorldCrashTestSegv();

        return -1;
    }

#ifndef _WIN32
    int unixMain(int argc, char** argv)
    {
        rlimit rl;
        if (getrlimit(RLIMIT_CORE, &rl) == -1)
            fmt::println("getrlimit failed. This could be problem.");
        else
        {
            rl.rlim_cur = rl.rlim_max;
            if (setrlimit(RLIMIT_CORE, &rl) == -1)
                fmt::println("setrlimit failed. Server may not save core.dump files.");
        }

        // Return directly to allow proper stack unwinding instead of hard exit()
        return sMaster().run(argc, argv) ? 0 : -1;
    }

#else

    int win32Main(int argc, char** argv)
    {
        int exitCode = 1;

        try
        {
            exitCode = sMaster().run(argc, argv) ? 0 : 1;
        }
        catch (const std::exception& e)
        {
            fmt::println("Fatal C++ exception: {}", e.what());
        }
        catch (...)
        {
            fmt::println("Unknown C++ exception in main.");
        }

        return exitCode;
    }
#endif
}

int main(int argc, char** argv)
{
    installWorldCrashHandler();

//#define ASCEMU_ENABLE_CRASH_TESTS 1
#ifdef ASCEMU_ENABLE_CRASH_TESTS
    std::string test = "--crash-test-stack";
    if (const int crashTestResult = tryRunWorldCrashTests(test); crashTestResult >= 0)
    {
        fmt::println("CrashHandler {} ended {}", test, crashTestResult == 0 ? "success" : "failed");
    }
#endif

    // Init this asap to set initTime correctly
    ServerState::instance();

#ifdef _WIN32
    return win32Main(argc, argv);
#else
    return unixMain(argc, argv);
#endif
}
