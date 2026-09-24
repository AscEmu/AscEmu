/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>
#include <memory>

namespace AscEmu::Threading
{
    class AEThreadPool;
}

class Database;

namespace AscEmu::Battlenet
{
    extern std::unique_ptr<Database> sBNetLogonSQL;

    class Master
    {
    public:
        static Master& getInstance();

        Master(Master&&) = delete;
        Master(const Master&) = delete;
        Master& operator=(Master&&) = delete;
        Master& operator=(const Master&) = delete;

        void run();
        void stop();

    private:
        Master() = default;
        ~Master() = default;

        bool loadConfiguration();
        void hookSignals();
        void unhookSignals();
        static void onSignal(int signal);

        std::atomic<bool> m_running{true};
        std::unique_ptr<AscEmu::Threading::AEThreadPool> m_threadPool;
    };
}
