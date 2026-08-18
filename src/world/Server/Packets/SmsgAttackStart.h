/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAttackStart : public ManagedPacket
    {
    public:
        uint64_t attackerGuid;
        uint64_t victimGuid;

        SmsgAttackStart() : SmsgAttackStart(0, 0)
        {
        }

        SmsgAttackStart(uint64_t attackerGuid, uint64_t victimGuid) :
            ManagedPacket(SMSG_ATTACK_START, 0),
            attackerGuid(attackerGuid),
            victimGuid(victimGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << attackerGuid << victimGuid;

                return true;
            }
            else if (m_protocol.isMop())
            {
                WoWGuid attGuid = attackerGuid;
                WoWGuid vicGuid = victimGuid;

                packet.writeBit(attGuid[7]);
                packet.writeBit(vicGuid[7]);
                packet.writeBit(vicGuid[3]);
                packet.writeBit(attGuid[3]);
                packet.writeBit(attGuid[5]);
                packet.writeBit(vicGuid[4]);
                packet.writeBit(vicGuid[1]);
                packet.writeBit(attGuid[4]);
                packet.writeBit(vicGuid[0]);
                packet.writeBit(attGuid[6]);
                packet.writeBit(vicGuid[5]);
                packet.writeBit(attGuid[2]);
                packet.writeBit(vicGuid[6]);
                packet.writeBit(attGuid[1]);
                packet.writeBit(vicGuid[2]);
                packet.writeBit(attGuid[0]);

                packet.writeByteSeq(vicGuid[5]);
                packet.writeByteSeq(vicGuid[0]);
                packet.writeByteSeq(attGuid[5]);
                packet.writeByteSeq(vicGuid[4]);
                packet.writeByteSeq(vicGuid[6]);
                packet.writeByteSeq(attGuid[6]);
                packet.writeByteSeq(attGuid[1]);
                packet.writeByteSeq(attGuid[0]);
                packet.writeByteSeq(vicGuid[7]);
                packet.writeByteSeq(attGuid[4]);
                packet.writeByteSeq(vicGuid[2]);
                packet.writeByteSeq(attGuid[3]);
                packet.writeByteSeq(attGuid[7]);
                packet.writeByteSeq(attGuid[2]);
                packet.writeByteSeq(vicGuid[3]);
                packet.writeByteSeq(vicGuid[1]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
