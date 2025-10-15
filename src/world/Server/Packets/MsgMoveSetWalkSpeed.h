/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    class MsgMoveSetWalkSpeed : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;

        MsgMoveSetWalkSpeed() : MsgMoveSetWalkSpeed(WoWGuid(), 0)
        {
        }

        MsgMoveSetWalkSpeed(WoWGuid guid, float rate) :
//Zyres: Due to wrong understanding of these opcodes the logic gets turned around here
#if VERSION_STRING < Cata
            ManagedPacket(SMSG_FORCE_WALK_SPEED_CHANGE, 0),
#elif VERSION_STRING == Cata
            ManagedPacket(MSG_MOVE_SET_WALK_SPEED, 0),
#elif VERSION_STRING == Mop
            ManagedPacket(SMSG_MOVE_SET_WALK_SPEED, 0),
#endif
            guid(guid),
            rate(rate)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << uint32_t(0) << rate;
#elif VERSION_STRING == Cata
            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[5]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[6]);
            packet.writeBit(guid[7]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[5]);
            packet << float(rate);
            packet.WriteByteSeq(guid[2]);
            packet << uint32_t(0);
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[3]);
#else // Mop
            packet.writeBit(guid[6]);
            packet.writeBit(guid[7]);
            packet.writeBit(guid[3]);
            packet.writeBit(guid[1]);
            packet.writeBit(guid[2]);
            packet.writeBit(guid[0]);
            packet.writeBit(guid[4]);
            packet.writeBit(guid[5]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[6]);
            packet << uint32_t(0);
            packet.WriteByteSeq(guid[4]);
            packet << float(rate);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[1]);
            packet.WriteByteSeq(guid[7]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
