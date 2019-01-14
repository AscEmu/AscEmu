/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.h"
#include "GuildLogHolder.h"
#include "Management/Guild/GuildDefinitions.h"

GuildLogHolder::GuildLogHolder(uint32_t guildId, uint32_t maxRecords) : mGuildId(guildId), mMaxRecords(maxRecords), mNextGUID(uint32_t(UNK_EVENT_LOG_GUID))
{
}

GuildLogHolder::~GuildLogHolder()
{
    for (GuildLog::iterator itr = mLog.begin(); itr != mLog.end(); ++itr)
    {
        delete (*itr);
    }
}

uint8_t GuildLogHolder::getSize() const
{
    return uint8_t(mLog.size());
}

bool GuildLogHolder::canInsert() const
{
    return mLog.size() < mMaxRecords;
}

void GuildLogHolder::loadEvent(GuildLogEntry* entry)
{
    if (mNextGUID == uint32_t(UNK_EVENT_LOG_GUID))
    {
        mNextGUID = entry->getGUID();
    }

    mLog.push_front(entry);
}

void GuildLogHolder::addEvent(GuildLogEntry* entry)
{
    if (mLog.size() >= mMaxRecords)
    {
        GuildLogEntry* oldEntry = mLog.front();
        delete oldEntry;
        mLog.pop_front();
    }

    mLog.push_back(entry);

    entry->saveGuildLogToDB();
}

void GuildLogHolder::writeLogHolderPacket(WorldPacket& data) const
{
#if VERSION_STRING >= Cata
    ByteBuffer buffer;
    data.writeBits(mLog.size(), 23);
    for (GuildLog::const_iterator itr = mLog.begin(); itr != mLog.end(); ++itr)
        (*itr)->writeGuildLogPacket(data, buffer);

    data.flushBits();
    data.append(buffer);
#else
    ByteBuffer buffer;
    data << uint8(mLog.size());
    for (auto itr = mLog.begin(); itr != mLog.end(); ++itr)
        (*itr)->writeGuildLogPacket(data, buffer);
#endif
}

uint32_t GuildLogHolder::getNextGUID()
{
    if (mNextGUID == uint32_t(UNK_EVENT_LOG_GUID))
    {
        mNextGUID = 0;
    }
    else
    {
        mNextGUID = (mNextGUID + 1) % mMaxRecords;
    }

    return mNextGUID;
}

GuildLog* GuildLogHolder::getGuildLog()
{
    return &mLog;
}
