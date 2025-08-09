/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "GuildLog.hpp"

class GuildLogHolder
{
public:
    GuildLogHolder(uint32_t guildId, uint32_t maxRecords);

    ~GuildLogHolder();

    uint8_t getSize() const;

    bool canInsert() const;
    void loadEvent(std::unique_ptr<GuildLogEntry> entry);
    void addEvent(std::unique_ptr<GuildLogEntry> entry);
    void writeLogHolderPacket(WorldPacket& data) const;
    uint32_t getNextGUID();
    GuildLog* getGuildLog();

private:
    GuildLog mLog;
    uint32_t mGuildId;
    uint32_t mMaxRecords;
    uint32_t mNextGUID;
};
