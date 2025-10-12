/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSplineMoveSetUnset : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint16_t opcode;

        SmsgSplineMoveSetUnset() : SmsgSplineMoveSetUnset(WoWGuid(), 0)
        {
        }

        SmsgSplineMoveSetUnset(WoWGuid guid, uint16_t opcode) :
            ManagedPacket(opcode, 0),
            guid(guid),
            opcode(opcode)
        {
        }

    protected:

        size_t expectedSize() const override
        {
            return 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet << guid;
#else //Cata and Mop
            switch (opcode)
            {
                case SMSG_SPLINE_MOVE_WATER_WALK:
                {
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[1]);

                } break;
                case SMSG_SPLINE_MOVE_LAND_WALK:
                {
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[6]);

                } break;
                case SMSG_SPLINE_MOVE_FEATHER_FALL:
                {
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);

                } break;
                case SMSG_SPLINE_MOVE_NORMAL_FALL:
                {
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);

                } break;
                case SMSG_SPLINE_MOVE_SET_HOVER:
                {
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);

                } break;
                case SMSG_SPLINE_MOVE_UNSET_HOVER:
                {
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[1]);

                } break;
                case SMSG_SPLINE_MOVE_SET_FLYING:
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);

                } break;
                case SMSG_SPLINE_MOVE_UNSET_FLYING:
                {
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);

                } break;
                case SMSG_SPLINE_MOVE_ROOT:
                {
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);

                } break;
                case SMSG_SPLINE_MOVE_UNROOT:
                {
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[4]);

                } break;
                case SMSG_SPLINE_MOVE_START_SWIM:
                {
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);

                } break;
                case SMSG_SPLINE_MOVE_STOP_SWIM:
                {
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);

                } break;
                case SMSG_SPLINE_MOVE_GRAVITY_DISABLE:
                {
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[0]);

                } break;
                case SMSG_SPLINE_MOVE_GRAVITY_ENABLE:
                {
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);

                } break;
                case SMSG_SPLINE_MOVE_SET_WALK_MODE:
                {
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);

                } break;
                case SMSG_SPLINE_MOVE_SET_RUN_MODE:
                {
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);

                } break;
                default:
                    break;
            }
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override { return false; }
    };
}
