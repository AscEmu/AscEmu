/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPhaseShiftChange : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t phaseId;
        WoWGuid guid;
        uint32_t phaseFlags;
        uint32_t mapId;

        SmsgPhaseShiftChange() : SmsgPhaseShiftChange(0, WoWGuid())
        {
        }

        SmsgPhaseShiftChange(uint32_t phaseId, WoWGuid guid, uint32_t phaseFlags = 0, uint32_t mapId = 0) :
            ManagedPacket(SMSG_PHASE_SHIFT_CHANGE, 4),
            phaseId(phaseId),
            guid(guid),
            phaseFlags(phaseFlags),
            mapId(mapId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << phaseId;

#elif VERSION_STRING == Cata
            packet.writeBit(guid[2]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[7]);

            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[4]);

            packet << uint32_t(0);              // size AreaId swaps

            packet.WriteByteSeq(guid[1]);

            packet << uint32_t(phaseFlags);     // flags

            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[6]);

            packet << uint32_t(0);              // unknown

            packet << uint32_t(1 * 2);          // size phaseIds
            packet << uint16_t(phaseId);

            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[0]);

            packet << uint32_t(1);              // size visible mapIds
            packet << uint16_t(mapId);

            packet.WriteByteSeq(guid[5]);

#elif VERSION_STRING == Mop
            packet.writeBit(guid[0]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[5]);

            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[2]);

            packet << uint32_t(1);                // size phaseIds
            packet << uint16_t(phaseId);

            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[6]);

            packet << uint32_t(0);                // unknown

            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[7]);

            packet << uint32_t(1);              // size visible mapIds
            packet << uint16_t(mapId);

            packet << uint32_t(0);              // size AreaId swaps

            packet.WriteByteSeq(guid[5]);

            packet << uint32_t(phaseFlags);     // flags
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
