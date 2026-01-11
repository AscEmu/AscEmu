/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLogXpGain : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t normalXp;
        uint32_t restedXp;
        bool isQuestXp;

        SmsgLogXpGain() : SmsgLogXpGain(0, 0, 0, false)
        {
        }

        SmsgLogXpGain(uint64_t guid, uint32_t normalXp, uint32_t restedXp, bool isQuestXp) :
            ManagedPacket(SMSG_LOG_XPGAIN, 1),
            guid(guid),
            normalXp(normalXp),
            restedXp(restedXp),
            isQuestXp(isQuestXp)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4 + 4 + 4 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            if (isQuestXp == false)
                packet << guid << normalXp << uint8_t(0) << restedXp << float(1.0f);
            else
                packet << uint64_t(0) << normalXp << uint8_t(1) << uint8_t(0);

#else // Mop
            WoWGuid victim;
            victim.init(guid);

            packet.writeBit(0);
            packet.writeBit(victim[1]);
            packet.writeBit(victim[2]);
            packet.writeBit(victim[7]);
            packet.writeBit(victim[4]);
            packet.writeBit(victim[3]);
            packet.writeBit(0);
            packet.writeBit(victim[0]);
            packet.writeBit(victim[5]);
            packet.writeBit(victim[6]);
            packet.writeBit(0);
            packet.WriteByteSeq(victim[4]);
            packet.WriteByteSeq(victim[2]);
            packet << uint8_t(0);
            packet << float(1);
            packet.WriteByteSeq(victim[7]);
            packet.WriteByteSeq(victim[1]);
            packet.WriteByteSeq(victim[3]);
            packet.WriteByteSeq(victim[6]);
            packet << uint32_t(normalXp);

            if (!victim.isEmpty())
                packet << uint32_t(normalXp);

            packet.WriteByteSeq(victim[0]);
            packet.WriteByteSeq(victim[5]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
