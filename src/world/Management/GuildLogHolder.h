/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "GuildLog.h"

class GuildLogHolder
{
    public:

        GuildLogHolder(uint32_t guildId, uint32_t maxRecords);

        ~GuildLogHolder();

        uint8_t getSize() const;

        bool canInsert() const;
        void loadEvent(GuildLogEntry* entry);
        void addEvent(GuildLogEntry* entry);
        void writeLogHolderPacket(WorldPacket& data) const;
        uint32_t getNextGUID();
        GuildLog* getGuildLog();

    private:

        GuildLog mLog;
        uint32_t mGuildId;
        uint32_t mMaxRecords;
        uint32_t mNextGUID;
};
