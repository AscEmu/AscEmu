/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
 *
 */

#include <iostream>
#include "Config/Config.h"
#include "Log.hpp"
#include "Util.hpp"
#include <cstdarg>
#include <string>

//createFileSingleton(oLog);
initialiseSingleton(WorldLog);

SERVER_DECL time_t UNIXTIME;
SERVER_DECL tm g_localTime;

void SessionLogWriter::write(const char* format, ...)
{
    if(!m_file)
        return;

    char out[32768];
    va_list ap;

    va_start(ap, format);

    std::string current_time = "[" + Util::GetCurrentDateTimeString() + "] ";
    sprintf(out, current_time.c_str());

    size_t l = strlen(out);
    vsnprintf(&out[l], 32768 - l, format, ap);
    fprintf(m_file, "%s\n", out);
    va_end(ap);
}

WorldLog::WorldLog()
{
    bEnabled = false;
    m_file = NULL;

    if(Config.MainConfig.GetBoolDefault("LogLevel", "World", false))
    {
        LogNotice("WorldLog : Enabling packetlog output to \"world.log\"");
        Enable();
    }
    else
    {
        Disable();
    }
}

void WorldLog::Enable()
{
    if(bEnabled)
        return;

    bEnabled = true;
    if(m_file != NULL)
    {
        Disable();
        bEnabled = true;
    }
    m_file = fopen("world.log", "a");
}

void WorldLog::Disable()
{
    if(!bEnabled)
        return;

    bEnabled = false;
    if(!m_file)
        return;

    fflush(m_file);
    fclose(m_file);
    m_file = NULL;
}

WorldLog::~WorldLog()
{
    if(m_file)
    {
        fclose(m_file);
        m_file = NULL;
    }
}

void SessionLogWriter::Open()
{
    m_file = fopen(m_filename, "a");
}

void SessionLogWriter::Close()
{
    if(!m_file) return;
    fflush(m_file);
    fclose(m_file);
    m_file = NULL;
}

SessionLogWriter::SessionLogWriter(const char* filename, bool open)
{
    m_filename = strdup(filename);
    m_file = NULL;
    if(open)
        Open();
}

SessionLogWriter::~SessionLogWriter()
{
    if(m_file)
        Close();

    free(m_filename);
}
