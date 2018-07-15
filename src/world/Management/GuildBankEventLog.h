/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Guild/GuildDefinitions.h"
#include "GuildLog.h"

#include "../world/Server/WorldSession.h"

class GuildBankEventLogEntry : public GuildLogEntry
{
    public:

        GuildBankEventLogEntry(uint32_t guildId, uint32_t guid, GuildBankEventLogTypes eventType, uint8_t tabId, uint32_t playerGuid,
            uint64_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId);

        GuildBankEventLogEntry(uint32_t guildId, uint32_t guid, time_t timestamp, uint8_t tabId, GuildBankEventLogTypes eventType, uint32_t playerGuid,
            uint64_t itemOrMoney, uint16_t itemStackCount, uint8_t destTabId);

        ~GuildBankEventLogEntry();

        static bool isMoneyEvent(GuildBankEventLogTypes eventType)
        {
            switch (eventType)
            {
                case GB_LOG_DEPOSIT_MONEY:
                case GB_LOG_WITHDRAW_MONEY:
                case GB_LOG_REPAIR_MONEY:
                case GB_LOG_CASH_FLOW_DEPOSIT:
                    return true;
                default:
                    return false;
            }
        }

        bool isMoneyEvent() const;

        void saveGuildLogToDB() const;
        void writeGuildLogPacket(WorldPacket& data, ByteBuffer& content) const;

    private:

        GuildBankEventLogTypes mEventType;
        uint8_t mBankTabId;
        uint32_t mPlayerGuid;
        uint64_t mItemOrMoney;
        uint16_t mItemStackCount;
        uint8_t mDestTabId;
};

