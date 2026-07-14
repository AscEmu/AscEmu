/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellEnergizeLog : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        WoWGuid casterGuid;
        uint32_t spellId;
        uint32_t type;
        uint32_t amount;

        SmsgSpellEnergizeLog() : SmsgSpellEnergizeLog(WoWGuid(), WoWGuid(), 0, 0, 0)
        {
        }

        SmsgSpellEnergizeLog(WoWGuid targetGuid, WoWGuid casterGuid, uint32_t spellId, uint32_t type, uint32_t amount) :
            ManagedPacket(SMSG_SPELLENERGIZELOG, 8 + 8 + 4 + 4 + 4),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            spellId(spellId),
            type(type),
            amount(amount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << casterGuid << targetGuid << spellId << type << amount;
            }
            else // Mop
            {
                packet.writeBit(targetGuid[7]);
                packet.writeBit(targetGuid[3]);
                packet.writeBit(casterGuid[1]);
                packet.writeBit(targetGuid[4]);
                packet.writeBit(targetGuid[2]);
                packet.writeBit(casterGuid[3]);
                packet.writeBit(targetGuid[5]);

                packet.writeBit(0);

                packet.writeBit(casterGuid[7]);
                packet.writeBit(casterGuid[0]);
                packet.writeBit(casterGuid[2]);
                packet.writeBit(casterGuid[4]);
                packet.writeBit(casterGuid[6]);
                packet.writeBit(targetGuid[6]);
                packet.writeBit(targetGuid[1]);
                packet.writeBit(targetGuid[0]);
                packet.writeBit(casterGuid[5]);

                packet.writeByteSeq(targetGuid[0]);
                packet.writeByteSeq(casterGuid[5]);
                packet.writeByteSeq(targetGuid[6]);
                packet.writeByteSeq(casterGuid[6]);
                packet.writeByteSeq(targetGuid[2]);
                packet.writeByteSeq(casterGuid[0]);
                packet.writeByteSeq(targetGuid[1]);

                packet << amount;

                packet.writeByteSeq(targetGuid[4]);
                packet.writeByteSeq(casterGuid[1]);
                packet.writeByteSeq(casterGuid[7]);
                packet.writeByteSeq(targetGuid[5]);
                packet.writeByteSeq(casterGuid[2]);
                packet.writeByteSeq(casterGuid[3]);
                packet.writeByteSeq(targetGuid[7]);
                packet.writeByteSeq(casterGuid[4]);
                packet.writeByteSeq(targetGuid[3]);

                packet << spellId << type;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
