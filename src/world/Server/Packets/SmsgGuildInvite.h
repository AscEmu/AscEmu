/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGuildInvite : public ManagedPacket
    {
    public:
        std::string inviterName;
        std::string guildName;

#if VERSION_STRING >= Cata
        uint32_t guildLevel;
        EmblemInfo mEmblemInfo;
        uint32_t guildId;
        uint64_t guildGuid;
#endif

        SmsgGuildInvite() : SmsgGuildInvite("", "")
        {
        }

        SmsgGuildInvite(std::string inviterName, std::string guildName) :
            ManagedPacket(SMSG_GUILD_INVITE, 2 + inviterName.size() + guildName.size()),
            inviterName(inviterName),
            guildName(guildName)
        {
        }

#if VERSION_STRING >= Cata
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
#endif

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << inviterName << guildName;
#else
            packet << uint32_t(guildLevel);
            packet << uint32_t(mEmblemInfo.getBorderStyle());
            packet << uint32_t(mEmblemInfo.getBorderColor());
            packet << uint32_t(mEmblemInfo.getStyle());
            packet << uint32_t(mEmblemInfo.getBackgroundColor());
            packet << uint32_t(mEmblemInfo.getColor());

            ObjectGuid oldGuildGuid = MAKE_NEW_GUID(guildId, 0, guildId ? uint32_t(HIGHGUID_TYPE_GUILD) : 0);
            ObjectGuid newGuildGuid = guildGuid;

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

            packet.WriteByteSeq(newGuildGuid[1]);
            packet.WriteByteSeq(oldGuildGuid[3]);
            packet.WriteByteSeq(newGuildGuid[6]);
            packet.WriteByteSeq(oldGuildGuid[2]);
            packet.WriteByteSeq(oldGuildGuid[1]);
            packet.WriteByteSeq(newGuildGuid[0]);

            packet.WriteString(guildName);

            packet.WriteByteSeq(newGuildGuid[7]);
            packet.WriteByteSeq(newGuildGuid[2]);

            packet.WriteString(inviterName.c_str());

            packet.WriteByteSeq(oldGuildGuid[7]);
            packet.WriteByteSeq(oldGuildGuid[6]);
            packet.WriteByteSeq(oldGuildGuid[5]);
            packet.WriteByteSeq(oldGuildGuid[0]);

            packet.WriteByteSeq(newGuildGuid[4]);

            packet.WriteString(guildName);

            packet.WriteByteSeq(newGuildGuid[5]);
            packet.WriteByteSeq(newGuildGuid[3]);

            packet.WriteByteSeq(oldGuildGuid[4]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
