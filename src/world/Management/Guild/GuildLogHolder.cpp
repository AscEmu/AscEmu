/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "GuildLog.hpp"
#include "GuildLogHolder.hpp"
#include "Macros/GuildMacros.hpp"
#include "Management/Guild/GuildDefinitions.hpp"

GuildLogHolder::GuildLogHolder(uint32_t guildId, uint32_t maxRecords) : mGuildId(guildId), mMaxRecords(maxRecords), mNextGUID(uint32_t(UNK_EVENT_LOG_GUID))
{
}

GuildLogHolder::~GuildLogHolder() = default;

uint8_t GuildLogHolder::getSize() const
{
    return uint8_t(mLog.size());
}

bool GuildLogHolder::canInsert() const
{
    return mLog.size() < mMaxRecords;
}

void GuildLogHolder::loadEvent(std::unique_ptr<GuildLogEntry> entry)
{
    if (mNextGUID == uint32_t(UNK_EVENT_LOG_GUID))
    {
        mNextGUID = entry->getGUID();
    }

    mLog.push_front(std::move(entry));
}

void GuildLogHolder::addEvent(std::unique_ptr<GuildLogEntry> entry)
{
    if (mMaxRecords > 0 && mLog.size() >= mMaxRecords)
    {
        mLog.pop_front();
    }

    entry->saveGuildLogToDB();

    mLog.push_back(std::move(entry));
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
    data << uint8_t(mLog.size());
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
