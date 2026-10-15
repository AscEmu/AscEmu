/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildSetRank : public ManagedPacket
    {
    public:
        uint32_t newRankId;
        uint32_t newRights;
        std::string rankName;
        uint32_t moneyPerDay;

        // cata specific
        uint32_t oldRankId = 0;
        uint32_t oldRights = 0;

        GuildBankRightsAndSlotsVec _rightsAndSlots;

        CmsgGuildSetRank() : CmsgGuildSetRank(0, 0, "", 0)
        {
        }

        CmsgGuildSetRank(uint32_t newRankId, uint32_t newRights, std::string rankName, uint32_t moneyPerDay) :
            ManagedPacket(CMSG_GUILD_SET_RANK, 4 + 4 + 1 + 4),
            newRankId(newRankId),
            newRights(newRights),
            rankName(rankName),
            moneyPerDay(moneyPerDay)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // rank, order, rights (6.x and 7.x: the old rights follow), gold limit, the eight tabs, name;
                // 8.x and later: the old rights behind the name
                const bool oldRightsFirst = m_protocol.isWoD() || m_protocol.isLegion();

                packet >> newRankId;
                packet.readSkip<int32_t>();         // rank order
                packet >> newRights;
                if (oldRightsFirst)
                    packet >> oldRights;
                packet >> moneyPerDay;

                for (uint8_t tabId = 0; tabId < 8; ++tabId)
                {
                    uint32_t bankRights;
                    uint32_t slots;
                    packet >> bankRights >> slots;
                    if (tabId < MAX_GUILD_BANK_TABS)
                        _rightsAndSlots.push_back(GuildBankRightsAndSlots(tabId, static_cast<uint8_t>(bankRights), slots));
                }

                packet.resetBitPos();
                rankName = packet.readString(packet.readBits(7));
                if (!oldRightsFirst)
                    packet >> oldRights;
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> newRankId >> newRights >> rankName >> moneyPerDay;

                readBankRightsAndSlots(packet);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet >> oldRankId >> oldRights >> newRights;

                readBankRightsAndSlots(packet);

                packet >> moneyPerDay >> newRankId;

                const uint32_t nameLength = packet.readBits(7);
                rankName = packet.readString(nameLength);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> oldRankId;

                readBankRightsAndSlots(packet);

                packet >> moneyPerDay >> oldRights >> newRights >> newRankId;

                const uint32_t nameLength = packet.readBits(7);
                rankName = packet.readString(nameLength);

                return true;
            }

            return false;
        }

    private:
        void readBankRightsAndSlots(WorldPacket& packet)
        {
            GuildBankRightsAndSlotsVec rightsAndSlots(MAX_GUILD_BANK_TABS);
            for (uint8_t tabId = 0; tabId < MAX_GUILD_BANK_TABS; ++tabId)
            {
                uint32_t bankRights;
                uint32_t slots;

                packet >> bankRights;
                packet >> slots;
                rightsAndSlots[tabId] = GuildBankRightsAndSlots(tabId, static_cast<uint8_t>(bankRights), slots);

                _rightsAndSlots.push_back(rightsAndSlots[tabId]);
            }
        }
    };
}
