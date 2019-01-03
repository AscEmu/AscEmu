/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldPacket.h"

class GuildLogEntry
{
    public:

        GuildLogEntry(uint32_t guildId, uint32_t guid);

        GuildLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp);

        virtual ~GuildLogEntry() {}

        uint32_t getGUID() const;
        uint64_t getTimestamp() const;

        virtual void saveGuildLogToDB() const = 0;
        virtual void writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const = 0;

    protected:

        uint32_t mGuildId;
        uint32_t mGuid;
        uint64_t mTimestamp;
};

typedef std::list<GuildLogEntry*> GuildLog;
