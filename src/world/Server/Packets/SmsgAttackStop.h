/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAttackStop : public ManagedPacket
    {
    public:
        WoWGuid attackerGuid;
        WoWGuid victimGuid;
        bool victimIsDead;

        SmsgAttackStop() : SmsgAttackStop(WoWGuid(), WoWGuid(), false)
        {
        }

        SmsgAttackStop(WoWGuid attackerGuid, WoWGuid victimGuid, bool victimIsDead) :
            ManagedPacket(SMSG_ATTACK_STOP, 0),
            attackerGuid(attackerGuid),
            victimGuid(victimGuid),
            victimIsDead(victimIsDead)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 8 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << attackerGuid;
                if (victimGuid.getRawGuid() == 0)
                    packet << uint8_t(0);
                else
                    packet << victimGuid;

                packet << uint32_t(0);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(victimGuid[5]);
                packet.writeBit(victimGuid[6]);
                packet.writeBit(attackerGuid[3]);
                packet.writeBit(attackerGuid[6]);
                packet.writeBit(attackerGuid[7]);
                packet.writeBit(attackerGuid[2]);
                packet.writeBit(attackerGuid[5]);
                packet.writeBit(victimGuid[4]);

                packet.writeBit((victimGuid.getRawGuid() != 0) && victimIsDead);

                packet.writeBit(victimGuid[3]);
                packet.writeBit(victimGuid[0]);
                packet.writeBit(victimGuid[2]);
                packet.writeBit(victimGuid[7]);
                packet.writeBit(attackerGuid[4]);
                packet.writeBit(attackerGuid[1]);
                packet.writeBit(attackerGuid[0]);
                packet.writeBit(victimGuid[1]);

                packet.flushBits();

                packet.writeByteSeq(victimGuid[0]);
                packet.writeByteSeq(victimGuid[3]);
                packet.writeByteSeq(victimGuid[5]);
                packet.writeByteSeq(victimGuid[2]);
                packet.writeByteSeq(attackerGuid[0]);
                packet.writeByteSeq(attackerGuid[6]);
                packet.writeByteSeq(attackerGuid[3]);
                packet.writeByteSeq(victimGuid[4]);
                packet.writeByteSeq(attackerGuid[1]);
                packet.writeByteSeq(attackerGuid[4]);
                packet.writeByteSeq(victimGuid[6]);
                packet.writeByteSeq(attackerGuid[5]);
                packet.writeByteSeq(attackerGuid[7]);
                packet.writeByteSeq(attackerGuid[2]);
                packet.writeByteSeq(victimGuid[1]);
                packet.writeByteSeq(victimGuid[7]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
