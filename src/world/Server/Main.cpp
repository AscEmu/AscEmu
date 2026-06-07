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
#include "Debugging/CrashHandler.h"
#include "ServerState.h"

#include <exception>

#ifndef _WIN32
#include <sys/resource.h>
#endif

namespace
{
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
        // This sets up the global unhandled exception filter
        startCrashHandler();

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
    // Init this asap to set initTime correctly
    ServerState::instance();

#ifdef _WIN32
    return win32Main(argc, argv);
#else
    return unixMain(argc, argv);
#endif
}
