/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "WorldRunnable.h"
#include <fmt/format.h>
#include "Logging/Log.hpp"
#include "Database/Database.h"

#include <cstdint>
#include <string>
#include <atomic>
#include <memory>

#include "Threading/ThreadPool.hpp"

class SessionLog;
class Database;

class SERVER_DECL Master
{
public:
    static Master& getInstance();

    Master(Master&&) = delete;
    Master(Master const&) = delete;
    Master& operator=(Master&&) = delete;
    Master& operator=(Master const&) = delete;

    Database& getWorldDatabase() const { return *databaseWorld; }
    Database& getCharacterDatabase() const { return *databaseCharacter; }

    SessionLog& getAnticheatLog() const { return *anticheatLog; }
    SessionLog& getGmCommandLog() const { return *gmCommandLog; }
    SessionLog& getPlayerLog() const { return *playerLog; }

    uint32_t getShutdownTimer() const { return m_ShutdownTimer; }
    bool isShutdownActive() const { return m_ShutdownEvent; }
    bool isRestartActive() const { return m_restartEvent; }

    bool run(int argc, char** argv);

    void openCheatLogFiles();
    void shutdownThreadPools(bool listenerSockCreate);
    void triggerShutdown(uint32_t timerMs, bool restart);
    void cancelShutdown();

    template <typename... Args>
    static void libLog(fmt::format_string<Args...> format, Args&&... args)
    {
        fmt::println(format, std::forward<Args>(args)...);
    }

    AscEmu::Threading::AEThreadPool& getThreadPool()
    {
        return *m_threadPool;
    }

    const AscEmu::Threading::AEThreadPool& getThreadPool() const
    {
        return *m_threadPool;
    }

private:
    Master() = default;
    ~Master();

    static void onSignal(int s);
    static void unhookSignals();
    static void hookSignals();

    bool checkDBVersion();
    bool startDB();
    void stopDB();
    void updatePeriodicStats(uint32_t currentLoop) const;
    void updateServerTime() const;
    bool processShutdownSequence(long long diff, uint32_t& nextPrintout, uint32_t& nextSend);

    bool m_ShutdownEvent{false};
    uint32_t m_ShutdownTimer{0};
    bool m_restartEvent{false};

    std::atomic<bool> stopEvent{false};
    std::atomic<bool> serverShutdown{false};

    std::unique_ptr<WorldRunnable> worldRunnable;

    std::unique_ptr<Database> databaseWorld;
    std::unique_ptr<Database> databaseCharacter;

    std::unique_ptr<SessionLog> gmCommandLog;
    std::unique_ptr<SessionLog> anticheatLog;
    std::unique_ptr<SessionLog> playerLog;

    std::unique_ptr<AscEmu::Threading::AEThreadPool> m_threadPool;
};

inline Master& sMaster()
{
    return Master::getInstance();
}

#define DLLLogDetail(msg, ...) sMaster().libLog(msg, ##__VA_ARGS__)
