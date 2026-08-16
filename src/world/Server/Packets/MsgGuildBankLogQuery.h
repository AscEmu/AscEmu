/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildLogHolder.hpp"
#include <cstdint>

#include "Utilities/Util.hpp"

struct GuildBankMoneyLog
{
    uint8_t action;
    uint64_t memberGuid;
    uint32_t entry;
    uint32_t stackCount;
    uint32_t timestamp;
};

namespace AscEmu::Packets
{
    class MsgGuildBankLogQuery : public ManagedPacket
    {
    public:
        uint8_t tabId;
        std::vector<GuildBankMoneyLog> moneyLog;

        // NOTE Guild::sendBankLog (pre-Cata) sends this same opcode by
        // delegating directly to GuildLogHolder::writeLogHolderPacket, which already contains the
        // correct per-entry wire format via GuildLogEntry::writeGuildLogPacket.
        GuildLogHolder const* bankLog = nullptr;

        MsgGuildBankLogQuery() : MsgGuildBankLogQuery(0, {})
        {
        }

        MsgGuildBankLogQuery(uint8_t tabId, std::vector<GuildBankMoneyLog> moneyLog) :
            ManagedPacket(MSG_GUILD_BANK_LOG_QUERY, 1),
            tabId(tabId),
            moneyLog(moneyLog)
        {
        }

        MsgGuildBankLogQuery(uint8_t tabId, GuildLogHolder const* bankLog) :
            ManagedPacket(MSG_GUILD_BANK_LOG_QUERY, 1 + (bankLog ? bankLog->getSize() : 0) * (4 * 4 + 1)),
            tabId(tabId),
            bankLog(bankLog)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (bankLog)
                return 1 + bankLog->getSize() * (4 * 4 + 1);

            return tabId != 6 ? 21 : 17 * moneyLog.size() + 2;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                if (bankLog)
                {
                    packet << tabId;
                    bankLog->writeLogHolderPacket(packet);
                    return true;
                }

                packet << tabId;
                packet << uint8_t(moneyLog.size() > 25 ? 25 : moneyLog.size());

                for (const auto& log : moneyLog)
                {
                    packet << log.action << log.memberGuid << log.entry;

                    if (tabId < 6)
                        packet << log.stackCount;

                    const uint32_t currentTime = ::Util::getMSTime();
                    packet << uint32_t(currentTime - log.timestamp);
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> tabId;
                return true;
            }

            return false;
        }
    };
}
