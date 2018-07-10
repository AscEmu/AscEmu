/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Guild/GuildDefinitions.h"
#include "GuildLog.h"

#include "WorldPacket.h"

class GuildNewsLogEntry : public GuildLogEntry
{
    public:

        GuildNewsLogEntry(uint32_t guildId, uint32_t guid, GuildNews type, uint32_t playerGuid, uint32_t flags, uint32_t value);

        GuildNewsLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, GuildNews type, uint32_t playerGuid, uint32_t flags, uint32_t value);

        ~GuildNewsLogEntry();

        GuildNews getType() const;
        uint64_t getPlayerGuid() const;
        uint32_t getValue() const;
        uint32_t getFlags() const;
        void setSticky(bool isSticky);

        void saveGuildLogToDB() const;
        void writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const;

    private:

        GuildNews mType;
        uint32_t mPlayerGuid;
        uint32_t mFlags;
        uint32_t mValue;
};

