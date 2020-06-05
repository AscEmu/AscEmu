/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

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
#if VERSION_STRING < Mop
            packet << casterGuid << targetGuid << spellId << type << amount;
#else
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

            packet.WriteByteSeq(targetGuid[0]);
            packet.WriteByteSeq(casterGuid[5]);
            packet.WriteByteSeq(targetGuid[6]);
            packet.WriteByteSeq(casterGuid[6]);
            packet.WriteByteSeq(targetGuid[2]);
            packet.WriteByteSeq(casterGuid[0]);
            packet.WriteByteSeq(targetGuid[1]);

            packet << amount;

            packet.WriteByteSeq(targetGuid[4]);
            packet.WriteByteSeq(casterGuid[1]);
            packet.WriteByteSeq(casterGuid[7]);
            packet.WriteByteSeq(targetGuid[5]);
            packet.WriteByteSeq(casterGuid[2]);
            packet.WriteByteSeq(casterGuid[3]);
            packet.WriteByteSeq(targetGuid[7]);
            packet.WriteByteSeq(casterGuid[4]);
            packet.WriteByteSeq(targetGuid[3]);

            packet << spellId << type;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
