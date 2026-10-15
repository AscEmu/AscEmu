/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/GuildMacros.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct GuildRankData
    {
        uint32_t id = 0;
        std::string name;
        std::array<uint32_t, MAX_GUILD_BANK_TABS> tabSlotsPerDay{};
        std::array<uint32_t, MAX_GUILD_BANK_TABS> tabRights{};
        uint32_t bankMoneyPerDay = 0;
        uint32_t rights = 0;
        uint32_t index = 0;
    };

    class SmsgGuildRank : public ManagedPacket
    {
    public:
        uint8_t ranksSize = 0;
        std::vector<GuildRankData> ranks;

        SmsgGuildRank() : SmsgGuildRank(0, {})
        {
        }

        SmsgGuildRank(uint8_t ranksSize, std::vector<GuildRankData> ranks) :
            ManagedPacket(SMSG_GUILD_RANK, 100),
            ranksSize(ranksSize), ranks(std::move(ranks))
        {
        }

    protected:
        size_t expectedSize() const override { return 100 + ranks.size() * (8 + MAX_GUILD_BANK_TABS * 8); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // every rank with order, rights, gold limit and the eight bank tabs; 8.x and later send the rank id as byte
                packet << uint32_t(ranks.size());
                for (const auto& rank : ranks)
                {
                    if (m_protocol.isWoD() || m_protocol.isLegion())
                        packet << uint32_t(rank.id);
                    else
                        packet << uint8_t(rank.id);
                    packet << int32_t(rank.index);
                    packet << uint32_t(rank.rights);
                    packet << uint32_t(rank.bankMoneyPerDay);
                    for (uint8_t i = 0; i < 8; ++i)
                    {
                        packet << uint32_t(i < MAX_GUILD_BANK_TABS ? rank.tabRights[i] : 0);
                        packet << uint32_t(i < MAX_GUILD_BANK_TABS ? rank.tabSlotsPerDay[i] : 0);
                    }
                    packet.writeBits(static_cast<uint32_t>(rank.name.length()), 7);
                    packet.flushBits();
                    packet.writeString(rank.name);
                }
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                ByteBuffer rankData(100);
                packet.writeBits(ranksSize, 18);

                for (const auto& rank : ranks)
                {
                    packet.writeBits(rank.name.length(), 7);

                    rankData << uint32_t(rank.id);

                    for (uint8_t j = 0; j < MAX_GUILD_BANK_TABS; ++j)
                    {
                        rankData << uint32_t(rank.tabSlotsPerDay[j]);
                        rankData << uint32_t(rank.tabRights[j]);
                    }

                    rankData << uint32_t(rank.bankMoneyPerDay);
                    rankData << uint32_t(rank.rights);

                    if (rank.name.length())
                        rankData.writeString(rank.name);

                    rankData << uint32_t(rank.index);
                }

                packet.flushBits();
                packet.append(rankData);

                return true;
            }
            else if (m_protocol.isMop())
            {
                ByteBuffer rankData(100);
                packet.writeBits(ranksSize, 17);

                for (const auto& rank : ranks)
                    packet.writeBits(rank.name.length(), 7);

                for (const auto& rank : ranks)
                {
                    rankData << uint32_t(rank.index);
                    rankData << uint32_t(rank.bankMoneyPerDay);

                    for (uint8_t j = 0; j < MAX_GUILD_BANK_TABS; ++j)
                    {
                        rankData << uint32_t(rank.tabSlotsPerDay[j]);
                        rankData << uint32_t(rank.tabRights[j]);
                    }

                    if (rank.name.length())
                        rankData.writeString(rank.name);

                    rankData << uint32_t(rank.id);
                    rankData << uint32_t(rank.rights);
                }

                packet.flushBits();
                packet.append(rankData);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
