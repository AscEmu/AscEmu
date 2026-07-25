/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildEmblemInfo.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgGuildInvite : public ManagedPacket
    {
    public:
        std::string inviterName;
        std::string guildName;

        uint32_t guildLevel = 0;
        EmblemInfo mEmblemInfo {};
        uint32_t guildId = 0;
        uint64_t guildGuid = 0;

        SmsgGuildInvite() : SmsgGuildInvite("", "")
        {
        }

        SmsgGuildInvite(std::string inviterName, std::string guildName) :
            ManagedPacket(SMSG_GUILD_INVITE, 2 + inviterName.size() + guildName.size()),
            inviterName(inviterName),
            guildName(guildName)
        {
        }

        SmsgGuildInvite(std::string inviterName, std::string guildName, uint32_t guildLevel, EmblemInfo mEmblemInfo, uint32_t guildId, uint64_t guildGuid) :
            ManagedPacket(SMSG_GUILD_INVITE, 100),
            inviterName(inviterName),
            guildName(guildName),
            guildLevel(guildLevel),
            mEmblemInfo(mEmblemInfo),
            guildId(guildId),
            guildGuid(guildGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                packet << inviterName << guildName;
            }
            else
            {
                packet << uint32_t(guildLevel);
                packet << uint32_t(mEmblemInfo.getBorderStyle());
                packet << uint32_t(mEmblemInfo.getBorderColor());
                packet << uint32_t(mEmblemInfo.getStyle());
                packet << uint32_t(mEmblemInfo.getBackgroundColor());
                packet << uint32_t(mEmblemInfo.getColor());

                WoWGuid oldGuildGuid(guildId, 0, guildId ? uint32_t(HIGHGUID_TYPE_GUILD) : 0);
                WoWGuid newGuildGuid = guildGuid;

                packet.writeBit(newGuildGuid[3]);
                packet.writeBit(newGuildGuid[2]);

                packet.writeBits(guildName.length(), 8);

                packet.writeBit(newGuildGuid[1]);

                packet.writeBit(oldGuildGuid[6]);
                packet.writeBit(oldGuildGuid[4]);
                packet.writeBit(oldGuildGuid[1]);
                packet.writeBit(oldGuildGuid[5]);
                packet.writeBit(oldGuildGuid[7]);
                packet.writeBit(oldGuildGuid[2]);

                packet.writeBit(newGuildGuid[7]);
                packet.writeBit(newGuildGuid[0]);
                packet.writeBit(newGuildGuid[6]);

                packet.writeBits(guildName.length(), 8);

                packet.writeBit(oldGuildGuid[3]);
                packet.writeBit(oldGuildGuid[0]);

                packet.writeBit(newGuildGuid[5]);

                packet.writeBits(inviterName.size(), 7);

                packet.writeBit(newGuildGuid[4]);

                packet.flushBits();

                packet.writeByteSeq(newGuildGuid[1]);
                packet.writeByteSeq(oldGuildGuid[3]);
                packet.writeByteSeq(newGuildGuid[6]);
                packet.writeByteSeq(oldGuildGuid[2]);
                packet.writeByteSeq(oldGuildGuid[1]);
                packet.writeByteSeq(newGuildGuid[0]);

                packet.writeString(guildName);

                packet.writeByteSeq(newGuildGuid[7]);
                packet.writeByteSeq(newGuildGuid[2]);

                packet.writeString(inviterName);

                packet.writeByteSeq(oldGuildGuid[7]);
                packet.writeByteSeq(oldGuildGuid[6]);
                packet.writeByteSeq(oldGuildGuid[5]);
                packet.writeByteSeq(oldGuildGuid[0]);

                packet.writeByteSeq(newGuildGuid[4]);

                packet.writeString(guildName);

                packet.writeByteSeq(newGuildGuid[5]);
                packet.writeByteSeq(newGuildGuid[3]);

                packet.writeByteSeq(oldGuildGuid[4]);
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
