/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>
#include <string>

namespace AscEmu::Threading
{
    class AEThread;
}

namespace AscEmu::Battlenet
{
    class BNetConsole
    {
    public:
        static BNetConsole& getInstance();

        BNetConsole(BNetConsole&&) = delete;
        BNetConsole(const BNetConsole&) = delete;
        BNetConsole& operator=(BNetConsole&&) = delete;
        BNetConsole& operator=(const BNetConsole&) = delete;

        void run(AscEmu::Threading::AEThread& thread);
        void stop();

    private:
        BNetConsole() = default;
        ~BNetConsole() = default;

        void processCommand(const std::string& input);
        void printHelp() const;

        void createBattleNetAccount(const std::string& args);
        void createGameAccount(const std::string& args);
        void linkGameAccount(const std::string& args);
        void unlinkGameAccount(const std::string& args);
        void listGameAccounts(const std::string& args);

        std::atomic<bool> m_running{true};
    };
}
