/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildRanksUpdate : public ManagedPacket
    {
    public:
        uint64_t setterGuid = 0;
        uint64_t targetGuid = 0;
        uint32_t rank = 0;
        bool isDemote = false;

        SmsgGuildRanksUpdate() : SmsgGuildRanksUpdate(0, 0, 0, false)
        {
        }

        SmsgGuildRanksUpdate(uint64_t setterGuid, uint64_t targetGuid, uint32_t rank, bool isDemote) :
            ManagedPacket(SMSG_GUILD_RANKS_UPDATE, 100),
            setterGuid(setterGuid), targetGuid(targetGuid), rank(rank), isDemote(isDemote)
        {
        }

    protected:
        size_t expectedSize() const override { return 100; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.isCata())
            {
                WoWGuid tarGuid = targetGuid;
                WoWGuid setGuid = setterGuid;

                packet.writeBit(setGuid[7]);
                packet.writeBit(setGuid[2]);
                packet.writeBit(tarGuid[2]);
                packet.writeBit(setGuid[1]);
                packet.writeBit(tarGuid[1]);
                packet.writeBit(tarGuid[7]);
                packet.writeBit(tarGuid[0]);
                packet.writeBit(tarGuid[5]);
                packet.writeBit(tarGuid[4]);
                packet.writeBit(isDemote);
                packet.writeBit(setGuid[5]);
                packet.writeBit(setGuid[0]);
                packet.writeBit(tarGuid[6]);
                packet.writeBit(setGuid[3]);
                packet.writeBit(setGuid[6]);
                packet.writeBit(tarGuid[3]);
                packet.writeBit(setGuid[4]);

                packet.flushBits();

                packet << uint32_t(rank);
                packet.writeByteSeq(setGuid[3]);
                packet.writeByteSeq(tarGuid[7]);
                packet.writeByteSeq(setGuid[6]);
                packet.writeByteSeq(setGuid[2]);
                packet.writeByteSeq(tarGuid[5]);
                packet.writeByteSeq(tarGuid[0]);
                packet.writeByteSeq(setGuid[7]);
                packet.writeByteSeq(setGuid[5]);
                packet.writeByteSeq(tarGuid[2]);
                packet.writeByteSeq(tarGuid[1]);
                packet.writeByteSeq(setGuid[0]);
                packet.writeByteSeq(setGuid[4]);
                packet.writeByteSeq(setGuid[1]);
                packet.writeByteSeq(tarGuid[3]);
                packet.writeByteSeq(tarGuid[6]);
                packet.writeByteSeq(tarGuid[4]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid tarGuid = targetGuid;
                WoWGuid setGuid = setterGuid;

                packet.writeBit(tarGuid[5]);
                packet.writeBit(tarGuid[6]);
                packet.writeBit(setGuid[0]);
                packet.writeBit(setGuid[1]);
                packet.writeBit(tarGuid[3]);
                packet.writeBit(setGuid[4]);
                packet.writeBit(tarGuid[2]);
                packet.writeBit(setGuid[6]);
                packet.writeBit(setGuid[3]);
                packet.writeBit(setGuid[7]);
                packet.writeBit(tarGuid[4]);
                packet.writeBit(tarGuid[0]);
                packet.writeBit(tarGuid[1]);
                packet.writeBit(setGuid[2]);
                packet.writeBit(tarGuid[7]);
                packet.writeBit(isDemote);
                packet.writeBit(setGuid[5]);

                packet.flushBits();

                packet.writeByteSeq(tarGuid[2]);
                packet.writeByteSeq(setGuid[1]);
                packet.writeByteSeq(tarGuid[6]);
                packet.writeByteSeq(tarGuid[1]);
                packet.writeByteSeq(tarGuid[5]);
                packet.writeByteSeq(setGuid[0]);
                packet << uint32_t(rank);
                packet.writeByteSeq(setGuid[3]);
                packet.writeByteSeq(setGuid[7]);
                packet.writeByteSeq(tarGuid[7]);
                packet.writeByteSeq(setGuid[2]);
                packet.writeByteSeq(tarGuid[3]);
                packet.writeByteSeq(tarGuid[4]);
                packet.writeByteSeq(setGuid[6]);
                packet.writeByteSeq(setGuid[5]);
                packet.writeByteSeq(tarGuid[0]);
                packet.writeByteSeq(setGuid[4]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
