/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSplineSetSpeed : public ManagedPacket
    {
    public:
        WoWGuid guid;
        float rate;
        uint16_t opcode;

        SmsgSplineSetSpeed() : SmsgSplineSetSpeed(WoWGuid(), 0, 0)
        {
        }

        SmsgSplineSetSpeed(WoWGuid guid, float rate, uint16_t opcode) :
            ManagedPacket(opcode, 0),
            rate(rate),
            opcode(opcode)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid << rate;
#elif VERSION_STRING == Cata
            switch (opcode)
            {
                case SMSG_SPLINE_SET_FLIGHT_BACK_SPEED:
                {
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.WriteByteSeq(guid[5]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                } break;
                case SMSG_SPLINE_SET_FLIGHT_SPEED:
                {
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet << float(rate);
                } break;
                case SMSG_SPLINE_SET_PITCH_RATE:
                {
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[4]);
                } break;
                case SMSG_SPLINE_SET_RUN_BACK_SPEED:
                {
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                } break;
                case SMSG_SPLINE_SET_RUN_SPEED:
                {
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                } break;
                case SMSG_SPLINE_SET_SWIM_BACK_SPEED:
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                } break;
                case SMSG_SPLINE_SET_SWIM_SPEED:
                {
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[4]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                } break;
                case SMSG_SPLINE_SET_TURN_RATE:
                {
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                } break;
                case SMSG_SPLINE_SET_WALK_SPEED:
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet << float(rate);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                } break;
                default:
                    break;
            }

#else // TODO: Mop
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
