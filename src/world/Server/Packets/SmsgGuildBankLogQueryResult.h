/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildLogHolder.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildBankLogQueryResult : public ManagedPacket
    {
    public:
        uint8_t tabId = 0;
        bool hasCashFlow = false; // getLevel() >= 5 && tabId == MAX_GUILD_BANK_TABS
        GuildLogHolder const* bankLog = nullptr;

        SmsgGuildBankLogQueryResult() : SmsgGuildBankLogQueryResult(0, false, nullptr)
        {
        }

        SmsgGuildBankLogQueryResult(uint8_t tabId, bool hasCashFlow, GuildLogHolder const* bankLog) :
            ManagedPacket(SMSG_GUILD_BANK_LOG_QUERY_RESULT, (bankLog ? bankLog->getSize() : 0) * (4 * 4 + 1) + 4 + 1 + 8),
            tabId(tabId), hasCashFlow(hasCashFlow), bankLog(bankLog)
        {
        }

    protected:
        size_t expectedSize() const override { return (bankLog ? bankLog->getSize() : 0) * (4 * 4 + 1) + 4 + 1 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                packet.writeBit(hasCashFlow);
                bankLog->writeLogHolderPacket(packet);
                packet << uint32_t(tabId);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << uint32_t(tabId);
                packet.writeBit(hasCashFlow);

                if (bankLog)
                    bankLog->writeLogHolderPacket(packet);

                if (hasCashFlow)
                    packet << uint64_t(0); // cash flow total

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
