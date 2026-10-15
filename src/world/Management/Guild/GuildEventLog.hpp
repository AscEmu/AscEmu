/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Guild/GuildDefinitions.hpp"
#include "GuildLog.hpp"

class GuildEventLogEntry : public GuildLogEntry
{
public:
    GuildEventLogEntry(uint32_t guildId, uint32_t guid, GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2, uint8_t newRank);
    GuildEventLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2, uint8_t newRank);
    ~GuildEventLogEntry();

    void saveGuildLogToDB() const;
    void writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const;

    GuildEventLogTypes getEventType() const { return mEventType; }
    uint32_t getPlayerGuid1() const { return mPlayerGuid1; }
    uint32_t getPlayerGuid2() const { return mPlayerGuid2; }
    uint8_t getNewRank() const { return mNewRank; }

private:
    GuildEventLogTypes mEventType;
    uint32_t mPlayerGuid1;
    uint32_t mPlayerGuid2;
    uint8_t mNewRank;
};

