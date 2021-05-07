/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Log.hpp"
#include "Logging/Logger.hpp"
#include "Util.hpp"
#include "Config/Config.h"

#include <iostream>
#include <cstdarg>
#include <string>
#include "../../src/world/WorldConf.h"

//////////////////////////////////////////////////////////////////////////////////////////
// World functions
SERVER_DECL time_t UNIXTIME;
SERVER_DECL tm g_localTime;

WorldPacketLog& WorldPacketLog::getInstance()
{
    static WorldPacketLog mInstance;
    return mInstance;
}

void WorldPacketLog::initialize()
{
    isLogEnabled = false;
    mPacketLogFile = nullptr;
}

void WorldPacketLog::finalize()
{
    if (mPacketLogFile)
    {
        fclose(mPacketLogFile);
        mPacketLogFile = nullptr;
    }
}

void WorldPacketLog::initWorldPacketLog(bool enableLog)
{
    isLogEnabled = enableLog;

    if (isLogEnabled)
    {
        sLogger.debug("WorldPacketLog : Enabling packetlog output to \"world-packet.log\"");
        enablePacketLog();
    }
    else
    {
        disablePacketLog();
    }
}

void WorldPacketLog::enablePacketLog()
{
    if (mPacketLogFile != nullptr)
    {
        disablePacketLog();
        isLogEnabled = true;
    }
    mPacketLogFile = fopen("world-packet.log", "a");
}

void WorldPacketLog::disablePacketLog()
{
    if (mPacketLogFile != nullptr)
    {
        fflush(mPacketLogFile);
        fclose(mPacketLogFile);
        mPacketLogFile = nullptr;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// SessionLog functions

SessionLog::SessionLog(const char* filename, bool open)
{
#if defined(linux) || defined(__linux)
    mFileName = strdup(filename);
#else
    mFileName = _strdup(filename);
#endif

    mSessionLogFile = nullptr;
    if (open)
    {
        openSessionLog();
    }
}

SessionLog::~SessionLog()
{
    if (mSessionLogFile != nullptr)
    {
        closeSessionLog();
    }

    free(mFileName);
}

void SessionLog::openSessionLog()
{
    mSessionLogFile = fopen(mFileName, "a");
}

bool SessionLog::isSessionLogOpen()
{
    return (mSessionLogFile != nullptr);
}

void SessionLog::closeSessionLog()
{
    if (mSessionLogFile != nullptr)
    {
        fflush(mSessionLogFile);
        fclose(mSessionLogFile);
        mSessionLogFile = nullptr;
    }
}

void SessionLog::write(const char* format, ...)
{
    if (mSessionLogFile != nullptr)
    {
        char out[32768];
        va_list ap;

        va_start(ap, format);

        std::string current_time = "[" + Util::GetCurrentDateTimeString() + "] ";
        sprintf(out, current_time.c_str());

        size_t l = strlen(out);
        vsnprintf(&out[l], 32768 - l, format, ap);
        fprintf(mSessionLogFile, "%s\n", out);
        va_end(ap);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// AscEmuLog format/color functions
namespace AELog
{
#ifndef _WIN32
    const char* GetColorForDebugFlag(LogFlags log_flags)
    {
        switch (log_flags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return CONSOLE_COLOR_BLUE;
            case LF_OPCODE:
                return CONSOLE_COLOR_WHITE;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return CONSOLE_COLOR_PURPLE;
            case LF_SCRIPT_MGR:
            case LF_DB_TABLES:
                return CONSOLE_COLOR_YELLOW;
            default:
                return CONSOLE_COLOR_YELLOW;
}
}
#else
    int GetColorForDebugFlag(LogFlags log_flags)
    {
        switch (log_flags)
        {
            case LF_MAP:
            case LF_MAP_CELL:
            case LF_VMAP:
            case LF_MMAP:
                return CONSOLE_COLOR_BLUE;
            case LF_OPCODE:
                return CONSOLE_COLOR_WHITE;
            case LF_SPELL:
            case LF_AURA:
            case LF_SPELL_EFF:
            case LF_AURA_EFF:
                return CONSOLE_COLOR_PURPLE;
            case LF_SCRIPT_MGR:
            case LF_DB_TABLES:
                return CONSOLE_COLOR_YELLOW;
            default:
                return CONSOLE_COLOR_YELLOW;
        }
    }
#endif
}
