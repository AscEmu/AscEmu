/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    class MsgGuildPermissions : public ManagedPacket
    {
    public:
        uint32_t rankId = 0;
        uint32_t rankRights = 0;
        uint32_t remainingMoney = 0;
        uint8_t purchasedTabsSize = 0;
        std::vector<std::pair<uint32_t, uint32_t>> tabs; // {rights, remainingSlots}, MAX_GUILD_BANK_TABS entries

        MsgGuildPermissions() : MsgGuildPermissions(0, 0, 0, 0, {})
        {
        }

        MsgGuildPermissions(uint32_t rankId, uint32_t rankRights, uint32_t remainingMoney, uint8_t purchasedTabsSize,
            std::vector<std::pair<uint32_t, uint32_t>> tabs) :
            ManagedPacket(MSG_GUILD_PERMISSIONS, 4 * 15 + 1),
            rankId(rankId), rankRights(rankRights), remainingMoney(remainingMoney), purchasedTabsSize(purchasedTabsSize), tabs(std::move(tabs))
        {
        }

    protected:
        size_t expectedSize() const override { return 4 * 15 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                return false;

            packet << uint32_t(rankId);
            packet << uint32_t(rankRights);
            packet << uint32_t(remainingMoney);
            packet << uint8_t(purchasedTabsSize);

            for (const auto& tab : tabs)
            {
                packet << uint32_t(tab.first);
                packet << uint32_t(tab.second);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
