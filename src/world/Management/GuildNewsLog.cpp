/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.h"
#include "GuildNewsLog.h"
#include "WoWGuid.h"
#include "Server/MainServerDefines.h"
#include "Server/Definitions.h"
#include "Objects/Object.h"


GuildNewsLogEntry::GuildNewsLogEntry(uint32_t guildId, uint32_t guid, GuildNews type, uint32_t playerGuid, uint32_t flags, uint32_t value) :
    GuildLogEntry(guildId, guid), mType(type), mPlayerGuid(playerGuid), mFlags(flags), mValue(value)
{
}

GuildNewsLogEntry::GuildNewsLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, GuildNews type, uint32_t playerGuid, uint32_t flags, uint32_t value) :
    GuildLogEntry(guildId, guid, timestamp), mType(type), mPlayerGuid(playerGuid), mFlags(flags), mValue(value)
{
}

GuildNewsLogEntry::~GuildNewsLogEntry()
{
}

GuildNews GuildNewsLogEntry::getType() const
{
    return mType;
}

uint64_t GuildNewsLogEntry::getPlayerGuid() const
{
    return mPlayerGuid ? MAKE_NEW_GUID(mPlayerGuid, 0, HIGHGUID_TYPE_PLAYER) : 0;
}

uint32_t GuildNewsLogEntry::getValue() const
{
    return mValue;
}

uint32_t GuildNewsLogEntry::getFlags() const
{
    return mFlags;
}

void GuildNewsLogEntry::setSticky(bool isSticky)
{
    if (isSticky)
    {
        mFlags |= 1;
    }
    else
    {
        mFlags &= ~1;
    }
}


void GuildNewsLogEntry::saveGuildLogToDB() const
{
    CharacterDatabase.Execute("DELETE FROM guild_newslog WHERE guildId = %u AND logGuid = %u", mGuildId, getGUID());

    CharacterDatabase.Execute("INSERT INTO guild_newslog VALUES('%u', '%u', '%u', '%u', '%u', '%u', '%llu')",
        mGuildId, getGUID(), (uint32_t)getType(), (uint32_t)getPlayerGuid(), getFlags(), getValue(), getTimestamp());
}

void GuildNewsLogEntry::writeGuildLogPacket(WorldPacket& data, ByteBuffer&) const
{
    data.writeBits(0, 26);
    ObjectGuid guid = getPlayerGuid();

    data.writeBit(guid[7]);
    data.writeBit(guid[0]);
    data.writeBit(guid[6]);
    data.writeBit(guid[5]);
    data.writeBit(guid[4]);
    data.writeBit(guid[3]);
    data.writeBit(guid[1]);
    data.writeBit(guid[2]);

    data.flushBits();

    data.WriteByteSeq(guid[5]);

    data << uint32_t(getFlags());
    data << uint32_t(getValue());
    data << uint32_t(0);

    data.WriteByteSeq(guid[7]);
    data.WriteByteSeq(guid[6]);
    data.WriteByteSeq(guid[2]);
    data.WriteByteSeq(guid[3]);
    data.WriteByteSeq(guid[0]);
    data.WriteByteSeq(guid[4]);
    data.WriteByteSeq(guid[1]);

    data << uint32_t(getGUID());
    data << uint32_t(getType());

    data.appendPackedTime(getTimestamp());
}
