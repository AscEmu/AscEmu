/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

struct GuildBankMoneyLog
{
    uint8_t action;
    uint64_t memberGuid;
    uint32_t entry;
    uint32_t stackCount;
    uint32_t timestamp;
};

namespace AscEmu { namespace Packets
{
    class MsgGuildBankLogQuery : public ManagedPacket
    {
    public:
        uint8_t tabId;
        std::vector<GuildBankMoneyLog> moneyLog;

        MsgGuildBankLogQuery() : MsgGuildBankLogQuery(0, {})
        {
        }

        MsgGuildBankLogQuery(uint8_t tabId, std::vector<GuildBankMoneyLog> moneyLog) :
            ManagedPacket(MSG_GUILD_BANK_LOG_QUERY, 1),
            tabId(tabId),
            moneyLog(moneyLog)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return tabId != 6 ? 21 : 17 * moneyLog.size() + 2;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << tabId;
            packet << uint8_t(moneyLog.size() > 25 ? 25 : moneyLog.size());

            for (const auto& log : moneyLog)
            {
                packet << log.action << log.memberGuid << log.entry;

                if (tabId < 6)
                    packet << log.stackCount;

                const uint32_t currentTime = static_cast<uint32_t>(UNIXTIME);
                packet << uint32_t(currentTime - log.timestamp);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> tabId;
            return true;
        }
    };
}}
