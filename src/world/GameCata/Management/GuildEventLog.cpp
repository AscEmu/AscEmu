/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.h"
#include "GuildEventLog.h"
#include "WoWGuid.h"
#include "Server/MainServerDefines.h"


GuildEventLogEntry::GuildEventLogEntry(uint32_t guildId, uint32_t guid, GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2, uint8_t newRank) :
    GuildLogEntry(guildId, guid), mEventType(eventType), mPlayerGuid1(playerGuid1), mPlayerGuid2(playerGuid2), mNewRank(newRank)
{
}

GuildEventLogEntry::GuildEventLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2, uint8_t newRank) :
    GuildLogEntry(guildId, guid, timestamp), mEventType(eventType), mPlayerGuid1(playerGuid1), mPlayerGuid2(playerGuid2), mNewRank(newRank)
{
}

GuildEventLogEntry::~GuildEventLogEntry()
{
}

void GuildEventLogEntry::saveGuildLogToDB() const
{
    CharacterDatabase.Execute("DELETE FROM guild_eventlog WHERE guildId = %u AND logGuid = %u", mGuildId, mGuid);
    CharacterDatabase.Execute("INSERT INTO guild_eventlog VALUES(%u, %u, %u, %u, %u, %u, %llu)",
        mGuildId, mGuid, uint8_t(mEventType), mPlayerGuid1, mPlayerGuid2, (uint32_t)mNewRank, mTimestamp);
}

void GuildEventLogEntry::writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const
{
    ObjectGuid guid1 = MAKE_NEW_GUID(mPlayerGuid1, 0, HIGHGUID_TYPE_PLAYER);
    ObjectGuid guid2 = MAKE_NEW_GUID(mPlayerGuid2, 0, HIGHGUID_TYPE_PLAYER);

    data.writeBit(guid1[2]);
    data.writeBit(guid1[4]);
    data.writeBit(guid2[7]);
    data.writeBit(guid2[6]);
    data.writeBit(guid1[3]);
    data.writeBit(guid2[3]);
    data.writeBit(guid2[5]);
    data.writeBit(guid1[7]);
    data.writeBit(guid1[5]);
    data.writeBit(guid1[0]);
    data.writeBit(guid2[4]);
    data.writeBit(guid2[2]);
    data.writeBit(guid2[0]);
    data.writeBit(guid2[1]);
    data.writeBit(guid1[1]);
    data.writeBit(guid1[6]);

    content.WriteByteSeq(guid2[3]);
    content.WriteByteSeq(guid2[2]);
    content.WriteByteSeq(guid2[5]);

    content << uint8_t(mNewRank);

    content.WriteByteSeq(guid2[4]);
    content.WriteByteSeq(guid1[0]);
    content.WriteByteSeq(guid1[4]);

    content << uint32_t(::time(nullptr) - mTimestamp);

    content.WriteByteSeq(guid1[7]);
    content.WriteByteSeq(guid1[3]);
    content.WriteByteSeq(guid2[0]);
    content.WriteByteSeq(guid2[6]);
    content.WriteByteSeq(guid2[7]);
    content.WriteByteSeq(guid1[5]);

    content << uint8_t(mEventType);

    content.WriteByteSeq(guid2[1]);
    content.WriteByteSeq(guid1[2]);
    content.WriteByteSeq(guid1[6]);
    content.WriteByteSeq(guid1[1]);
}
