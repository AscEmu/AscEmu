/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class CmsgGuildQueryRanks : public ManagedPacket
    {
    public:
        WoWGuid guildGuid;

        CmsgGuildQueryRanks() : ManagedPacket(CMSG_GUILD_QUERY_RANKS, 8)
        {
        }

        bool deserialise(WorldPacket& packet) override
        {
            if (packet.remaining() < expectedSize())
                return false;

            return internalDeserialise(packet);
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
                return m_minimum_size;
            else if (m_protocol.isMop())
                return 1; // packed guid: mask byte + only the non-zero guid bytes (1..9 bytes)
            return 0;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 guild;
                packet >> guild;
                guildGuid = WoWGuid::fromGuid128(guild);
                return true;
            }

            if (m_protocol.isCata())
            {
                guildGuid[2] = packet.readBit();
                guildGuid[3] = packet.readBit();
                guildGuid[0] = packet.readBit();
                guildGuid[6] = packet.readBit();
                guildGuid[4] = packet.readBit();
                guildGuid[7] = packet.readBit();
                guildGuid[5] = packet.readBit();
                guildGuid[1] = packet.readBit();

                packet.readByteSeq(guildGuid[3]);
                packet.readByteSeq(guildGuid[4]);
                packet.readByteSeq(guildGuid[5]);
                packet.readByteSeq(guildGuid[7]);
                packet.readByteSeq(guildGuid[1]);
                packet.readByteSeq(guildGuid[0]);
                packet.readByteSeq(guildGuid[6]);
                packet.readByteSeq(guildGuid[2]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                guildGuid[0] = packet.readBit();
                guildGuid[2] = packet.readBit();
                guildGuid[5] = packet.readBit();
                guildGuid[4] = packet.readBit();
                guildGuid[3] = packet.readBit();
                guildGuid[7] = packet.readBit();
                guildGuid[6] = packet.readBit();
                guildGuid[1] = packet.readBit();

                packet.readByteSeq(guildGuid[6]);
                packet.readByteSeq(guildGuid[0]);
                packet.readByteSeq(guildGuid[1]);
                packet.readByteSeq(guildGuid[7]);
                packet.readByteSeq(guildGuid[3]);
                packet.readByteSeq(guildGuid[2]);
                packet.readByteSeq(guildGuid[5]);
                packet.readByteSeq(guildGuid[4]);

                return true;
            }

            return false;
        }
    };
}
