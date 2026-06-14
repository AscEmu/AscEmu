/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "DatabaseDefinition.hpp"
#include "Logging/Logger.hpp"
#include "World.h"
#include <atomic>
#include <thread>
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _WIN32

void onCrash(bool terminate)
{
    sLogger.failure("Crash Handler : Advanced crash handler initialized.");

    static std::atomic_flag isHandlingCrash = ATOMIC_FLAG_INIT;
    if (isHandlingCrash.test_and_set())
    {
        // Another thread is already handling the crash, stop this thread.
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::hours(24));
        }
    }

    try
    {
        sLogger.info("sql : Waiting for all database queries to finish...");
        WorldDatabase.endThreads();
        CharacterDatabase.endThreads();
        sLogger.info("sql : All pending database operations cleared.");
        sWorld.saveAllPlayersToDb();
        sLogger.info("sql : Data saved.");
    }
    catch (...)
    {
        sLogger.failure("sql : Threw an exception while attempting to save all data.");
    }

    sLogger.info("Server : Closing.");

    // Terminate Entire Application
    if (terminate)
    {
        TerminateProcess(GetCurrentProcess(), 1);
    }
}

#endif
