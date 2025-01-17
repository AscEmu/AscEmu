/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include <mutex>

class WorldPacket;

extern SERVER_DECL time_t UNIXTIME; // update this every loop to avoid the time() syscall!
extern SERVER_DECL tm g_localTime;

//////////////////////////////////////////////////////////////////////////////////////////
// WorldPacketLog
class WorldPacketLog
{
    FILE* mPacketLogFile;
    bool isLogEnabled;

    std::mutex mPacketLogMutex;

private:
    WorldPacketLog() = default;
    ~WorldPacketLog() = default;

public:
    static WorldPacketLog& getInstance();
    void initialize();
    void finalize();

    WorldPacketLog(WorldPacketLog&&) = delete;
    WorldPacketLog(WorldPacketLog const&) = delete;
    WorldPacketLog& operator=(WorldPacketLog&&) = delete;
    WorldPacketLog& operator=(WorldPacketLog const&) = delete;

    void initWorldPacketLog(bool enableLog);
    void enablePacketLog();
    void disablePacketLog();

    // WorldSocket.cpp
    void logPacket(uint32_t len, uint16_t opcode, const uint8_t* data, uint8_t direction, uint32_t accountid = 0);
};
#define sWorldPacketLog WorldPacketLog::getInstance()

//////////////////////////////////////////////////////////////////////////////////////////
// SessionLog
class WorldSession;

class SERVER_DECL SessionLog
{
    FILE* mSessionLogFile;
    char* mFileName;

public:
    SessionLog(const char* filename, bool open);
    ~SessionLog();

    void openSessionLog();
    bool isSessionLogOpen();
    void closeSessionLog();

    void write(const char* format, ...);

    // WorldSession.cpp
    void writefromsession(WorldSession* session, const char* format, ...);
    void write(WorldSession* session, const char* format, ...);
};
