/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/GuildMacros.hpp"
#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgGuildPermissionsQueryResults : public ManagedPacket
    {
    public:
        uint32_t rankId = 0;
        uint32_t purchasedTabsSize = 0;
        uint32_t rankRights = 0;
        uint32_t remainingMoney = 0; // Cata only
        uint32_t rankBankMoneyPerDay = 0; // Mop only
        std::vector<std::pair<uint32_t, uint32_t>> tabs; // {rights, remainingSlots}, MAX_GUILD_BANK_TABS entries

        SmsgGuildPermissionsQueryResults() : SmsgGuildPermissionsQueryResults(0, 0, 0, 0, 0, {})
        {
        }

        SmsgGuildPermissionsQueryResults(uint32_t rankId, uint32_t purchasedTabsSize, uint32_t rankRights,
            uint32_t remainingMoney, uint32_t rankBankMoneyPerDay, std::vector<std::pair<uint32_t, uint32_t>> tabs) :
            ManagedPacket(SMSG_GUILD_PERMISSIONS_QUERY_RESULTS, 4 * 15 + 1),
            rankId(rankId), purchasedTabsSize(purchasedTabsSize), rankRights(rankRights), remainingMoney(remainingMoney),
            rankBankMoneyPerDay(rankBankMoneyPerDay), tabs(std::move(tabs))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 * 15 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                packet << uint32_t(rankId);
                packet << uint32_t(purchasedTabsSize);
                packet << uint32_t(rankRights);
                packet << uint32_t(remainingMoney);
                packet.writeBits(MAX_GUILD_BANK_TABS, 23);

                for (const auto& tab : tabs)
                {
                    packet << uint32_t(tab.first);
                    packet << uint32_t(tab.second);
                }

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << uint32_t(rankId);
                packet << uint32_t(rankBankMoneyPerDay);
                packet << uint32_t(purchasedTabsSize);
                packet << uint32_t(rankRights);

                packet.writeBits(MAX_GUILD_BANK_TABS, 21);
                packet.flushBits();

                for (const auto& tab : tabs)
                {
                    packet << uint32_t(tab.second);
                    packet << uint32_t(tab.first);
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
