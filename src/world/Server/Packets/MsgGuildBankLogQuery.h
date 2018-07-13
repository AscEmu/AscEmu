/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
        uint8_t slotId;
        std::vector<GuildBankMoneyLog> moneyLog;

        MsgGuildBankLogQuery() : MsgGuildBankLogQuery(0, {})
        {
        }

        MsgGuildBankLogQuery(uint8_t slotId, std::vector<GuildBankMoneyLog> moneyLog) :
            ManagedPacket(MSG_GUILD_BANK_LOG_QUERY, slotId != 6 ? 21 : 17 * moneyLog.size() + 2),
            slotId(slotId),
            moneyLog(moneyLog)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << slotId;
            packet << uint8_t(moneyLog.size() > 25 ? 25 : moneyLog.size());

            for (const auto& log : moneyLog)
            {
                packet << log.action << log.memberGuid << log.entry;

                if (slotId < 6)
                    packet << log.stackCount;

                const uint32_t currentTime = static_cast<uint32_t>(UNIXTIME);
                packet << uint32_t(currentTime - log.timestamp);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> slotId;
            return true;
        }
    };
}}
