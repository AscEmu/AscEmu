/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.h"

GuildLogEntry::GuildLogEntry(uint32_t guildId, uint32_t guid) : mGuildId(guildId), mGuid(guid), mTimestamp(::time(nullptr))
{
}

GuildLogEntry::GuildLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp) : mGuildId(guildId), mGuid(guid), mTimestamp(timestamp)
{
}

uint32_t GuildLogEntry::getGUID() const
{
    return mGuid;
}

uint64_t GuildLogEntry::getTimestamp() const
{
    return mTimestamp;
}
