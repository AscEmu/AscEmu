/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    struct SmsgLfGuildBrowseEntry
    {
        WoWGuid guildGuid;
        std::string name;
        std::string comment;
        uint32_t emblemColor = 0;
        uint32_t emblemBorderStyle = 0;
        uint32_t emblemStyle = 0;
        uint32_t emblemBackgroundColor = 0;
        uint32_t emblemBorderColor = 0;
        uint32_t level = 0;
        uint32_t interests = 0;
        uint32_t availability = 0;
        uint32_t classRoles = 0;
        uint32_t membersCount = 0;
        uint8_t hasRequest = 0;
    };

    // NOTE: SMSG_LF_GUILD_BROWSE_UPDATED has two distinct call sites in GuildHandler.cpp.
    // When there are no matching guilds, the original code sent a completely empty packet
    // (no bit-count, no data at all) rather than a packet with a zero guild count encoded.
    // That quirk is preserved here: an empty `guilds` vector serialises to zero bytes.
    class SmsgLfGuildBrowseUpdated : public ManagedPacket
    {
    public:
        std::vector<SmsgLfGuildBrowseEntry> guilds;

        SmsgLfGuildBrowseUpdated() : SmsgLfGuildBrowseUpdated(std::vector<SmsgLfGuildBrowseEntry>{})
        {
        }

        SmsgLfGuildBrowseUpdated(std::vector<SmsgLfGuildBrowseEntry> guilds) :
            ManagedPacket(SMSG_LF_GUILD_BROWSE_UPDATED, guilds.empty() ? 0 : 3 + guilds.size() * 65),
            guilds(std::move(guilds))
        {
        }

    protected:
        size_t expectedSize() const override { return guilds.empty() ? 0 : 3 + guilds.size() * 65; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (guilds.empty())
                return true;

            if (m_protocol.isCata())
            {
                ByteBuffer bufferData(65 * guilds.size());
                packet.writeBits(guilds.size(), 19);

                for (const auto& g : guilds)
                {
                    const WoWGuid& guildGUID = g.guildGuid;

                    packet.writeBit(guildGUID[7]);
                    packet.writeBit(guildGUID[5]);

                    packet.writeBits(g.name.size(), 8);

                    packet.writeBit(guildGUID[0]);

                    packet.writeBits(g.comment.size(), 11);

                    packet.writeBit(guildGUID[4]);
                    packet.writeBit(guildGUID[1]);
                    packet.writeBit(guildGUID[2]);
                    packet.writeBit(guildGUID[6]);
                    packet.writeBit(guildGUID[3]);

                    bufferData << uint32_t(g.emblemColor);
                    bufferData << uint32_t(g.emblemBorderStyle);
                    bufferData << uint32_t(g.emblemStyle);

                    bufferData.writeString(g.comment);

                    bufferData << uint8_t(0);

                    bufferData.writeByteSeq(guildGUID[5]);

                    bufferData << uint32_t(g.interests);

                    bufferData.writeByteSeq(guildGUID[6]);
                    bufferData.writeByteSeq(guildGUID[4]);

                    bufferData << uint32_t(g.level);

                    bufferData.writeString(g.name);

                    bufferData << uint32_t(0); // Achievment

                    bufferData.writeByteSeq(guildGUID[7]);

                    bufferData << uint8_t(g.hasRequest);

                    bufferData.writeByteSeq(guildGUID[2]);
                    bufferData.writeByteSeq(guildGUID[0]);

                    bufferData << uint32_t(g.availability);

                    bufferData.writeByteSeq(guildGUID[1]);

                    bufferData << uint32_t(g.emblemBackgroundColor);
                    bufferData << uint32_t(0);
                    bufferData << uint32_t(g.emblemBorderColor);
                    bufferData << uint32_t(g.classRoles);

                    bufferData.writeByteSeq(guildGUID[3]);
                    bufferData << uint32_t(g.membersCount);
                }

                packet.flushBits();
                packet.append(bufferData);

                return true;
            }
            else if (m_protocol.isMop())
            {
                ByteBuffer bufferData(65 * guilds.size());
                packet.writeBits(guilds.size(), 18);

                for (const auto& g : guilds)
                {
                    const WoWGuid& guildGUID = g.guildGuid;

                    packet.writeBit(guildGUID[6]);
                    packet.writeBit(guildGUID[5]);
                    packet.writeBit(guildGUID[4]);
                    packet.writeBit(guildGUID[0]);
                    packet.writeBit(guildGUID[1]);

                    packet.writeBits(g.comment.size(), 10);

                    packet.writeBit(guildGUID[3]);

                    packet.writeBits(g.name.size(), 7);

                    packet.writeBit(guildGUID[7]);
                    packet.writeBit(guildGUID[2]);

                    bufferData.writeByteSeq(guildGUID[3]);

                    bufferData << uint32_t(g.emblemStyle);
                    bufferData << uint8_t(g.hasRequest);

                    bufferData.writeByteSeq(guildGUID[0]);

                    bufferData << uint32_t(0); // Achievement points - not tracked at this point

                    bufferData.writeByteSeq(guildGUID[2]);

                    bufferData << uint32_t(g.interests);
                    bufferData << uint32_t(g.emblemBackgroundColor);
                    bufferData << uint32_t(g.level);
                    bufferData << uint32_t(g.availability);
                    bufferData << uint32_t(g.classRoles);

                    bufferData.writeByteSeq(guildGUID[5]);

                    bufferData << uint8_t(0); // Unk

                    bufferData.writeString(g.name);

                    bufferData << uint32_t(0); // Unk
                    bufferData << uint32_t(g.emblemBorderStyle);

                    bufferData.writeByteSeq(guildGUID[7]);

                    bufferData << uint32_t(g.emblemColor);

                    bufferData.writeByteSeq(guildGUID[6]);

                    bufferData << uint32_t(0); // Unk

                    bufferData.writeString(g.comment);

                    bufferData << uint32_t(g.emblemBorderColor);
                    bufferData << uint32_t(g.membersCount);

                    bufferData.writeByteSeq(guildGUID[1]);
                    bufferData.writeByteSeq(guildGUID[4]);
                }

                packet.flushBits();
                packet.append(bufferData);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
