/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Macros/GuildMacros.hpp"
#include "Management/Guild/GuildEmblemInfo.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgGuildQueryResponse : public ManagedPacket
    {
    public:
        uint64_t guid = 0;
        uint32_t id = 0;
        std::string name;
        std::vector<std::string> rankNames;
        std::vector<uint32_t> rankIds;
        EmblemInfo emblemInfo;

        SmsgGuildQueryResponse() : SmsgGuildQueryResponse(0, 0, "", {}, {}, EmblemInfo{})
        {
        }

        SmsgGuildQueryResponse(uint64_t guid, uint32_t id, std::string name, std::vector<std::string> rankNames,
            std::vector<uint32_t> rankIds, EmblemInfo emblemInfo) :
            ManagedPacket(SMSG_GUILD_QUERY_RESPONSE, 8 * 32 + 200),
            guid(guid), id(id), name(std::move(name)), rankNames(std::move(rankNames)), rankIds(std::move(rankIds)), emblemInfo(emblemInfo)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 * 32 + 200; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << uint32_t(id);
                packet << name;

                for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
                {
                    if (i < rankNames.size())
                        packet << rankNames[i];
                    else
                        packet << uint8_t(0);
                }

                emblemInfo.writeEmblemInfoToPacket(packet);
                packet << uint32_t(rankNames.size());

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet << uint64_t(guid);
                packet << name;

                for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
                {
                    if (i < rankIds.size())
                        packet << rankNames[i];
                    else
                        packet << uint8_t(0);
                }

                for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
                {
                    if (i < rankIds.size())
                        packet << uint32_t(i);
                    else
                        packet << uint32_t(0);
                }

                for (uint8_t i = 0; i < MAX_GUILD_RANKS; ++i)
                {
                    if (i < rankIds.size())
                        packet << uint32_t(rankIds[i]);
                    else
                        packet << uint32_t(0);
                }

                emblemInfo.writeEmblemInfoToPacket(packet);
                packet << uint32_t(rankIds.size());

                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid wowGuid = guid;

                packet.writeBit(wowGuid[5]);
                packet.writeBit(1); // has data

                packet.writeBits(rankNames.size(), 21);
                packet.writeBit(wowGuid[5]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[7]);

                for (const auto& rankName : rankNames)
                    packet.writeBits(rankName.length(), 7);

                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[6]);
                packet.writeBits(name.length(), 7);

                packet.writeBit(wowGuid[3]);
                packet.writeBit(wowGuid[7]);
                packet.writeBit(wowGuid[2]);
                packet.writeBit(wowGuid[1]);
                packet.writeBit(wowGuid[0]);
                packet.writeBit(wowGuid[4]);
                packet.writeBit(wowGuid[6]);

                packet.flushBits();

                packet << uint32_t(emblemInfo.getBorderStyle());
                packet << uint32_t(emblemInfo.getStyle());

                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[7]);

                packet << uint32_t(emblemInfo.getColor());
                packet << uint32_t(0); // realm id, not tracked by AscEmu

                for (size_t i = 0; i < rankNames.size(); ++i)
                {
                    packet << uint32_t(i);
                    packet << uint32_t(i < rankIds.size() ? rankIds[i] : 0);
                    packet.writeString(rankNames[i]);
                }

                packet.writeString(name);
                packet << uint32_t(emblemInfo.getBorderColor());

                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[4]);

                packet << uint32_t(emblemInfo.getBackgroundColor());

                packet.writeByteSeq(wowGuid[1]);
                packet.writeByteSeq(wowGuid[6]);
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[3]);

                packet.writeByteSeq(wowGuid[2]);
                packet.writeByteSeq(wowGuid[6]);
                packet.writeByteSeq(wowGuid[4]);
                packet.writeByteSeq(wowGuid[0]);
                packet.writeByteSeq(wowGuid[7]);
                packet.writeByteSeq(wowGuid[3]);
                packet.writeByteSeq(wowGuid[5]);
                packet.writeByteSeq(wowGuid[1]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
