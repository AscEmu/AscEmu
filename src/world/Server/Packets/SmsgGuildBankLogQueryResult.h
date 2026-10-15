/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildBankEventLog.hpp"
#include <ctime>
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
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // tab, the entries with player, age, type and the money or the item with count and the other tab;
                // the weekly bonus bit stands behind the entries on 6.x and 12.x, in front of them on 7.x to 11.x
                const bool bonusBitFirst = !m_protocol.isWoD() && !m_protocol.isMidnight();
                const std::time_t now = ::time(nullptr);

                uint32_t count = 0;
                if (bankLog != nullptr)
                {
                    for (const auto& entry : bankLog->getLog())
                    {
                        if (dynamic_cast<GuildBankEventLogEntry const*>(entry.get()) != nullptr)
                            ++count;
                    }
                }

                packet << int32_t(tabId);
                packet << uint32_t(count);
                if (bonusBitFirst)
                {
                    packet.writeBit(false);                             // weekly bonus money
                    packet.flushBits();
                }

                if (bankLog != nullptr)
                {
                    for (const auto& entry : bankLog->getLog())
                    {
                        const auto bankEntry = dynamic_cast<GuildBankEventLogEntry const*>(entry.get());
                        if (bankEntry == nullptr)
                            continue;

                        const bool isMoney = GuildBankEventLogEntry::isMoneyEvent(bankEntry->getEventType());
                        const bool isMove = bankEntry->getEventType() == GB_LOG_MOVE_ITEM || bankEntry->getEventType() == GB_LOG_MOVE_ITEM2;

                        packet << WoWGuid(bankEntry->getPlayerGuid(), 0, HIGHGUID_TYPE_PLAYER).toGuid128(m_protocol.realmId, 0);
                        packet << uint32_t(now - static_cast<std::time_t>(bankEntry->getTimestamp()));
                        packet << int8_t(bankEntry->getEventType());
                        packet.writeBit(isMoney);                       // money
                        packet.writeBit(!isMoney);                      // item
                        packet.writeBit(!isMoney);                      // count
                        packet.writeBit(isMove);                        // other tab
                        packet.flushBits();
                        if (isMoney)
                        {
                            packet << uint64_t(bankEntry->getItemOrMoney());
                        }
                        else
                        {
                            packet << int32_t(bankEntry->getItemOrMoney());
                            packet << int32_t(bankEntry->getItemStackCount());
                        }
                        if (isMove)
                            packet << int8_t(bankEntry->getDestTabId());
                    }
                }

                if (!bonusBitFirst)
                {
                    packet.writeBit(false);                             // weekly bonus money
                    packet.flushBits();
                }
                return true;
            }

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
