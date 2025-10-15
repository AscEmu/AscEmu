/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgMoveSetUnset : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint16_t opcode;

        SmsgMoveSetUnset() : SmsgMoveSetUnset(WoWGuid(), 0)
        {
        }

        SmsgMoveSetUnset(WoWGuid guid, uint16_t opcode) :
            ManagedPacket(opcode, 0),
            guid(guid),
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
            switch (opcode)
            {
                case SMSG_MOVE_UNSET_CAN_FLY:
                {
                    packet << guid << uint32_t(5);
                } break;
                case SMSG_MOVE_SET_CAN_FLY:
                {
                    packet << guid << uint32_t(2);
                } break;
                default:
                {
                    packet << guid << uint32_t(0);
                } break;
            }
#else //Cata and Mop
            switch (opcode)
            {
                case SMSG_MOVE_UNSET_CAN_FLY:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
#else //Mop
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
#endif
                } break;
                case SMSG_MOVE_SET_CAN_FLY:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
#else // Mop
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[1]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
#endif
                } break;
                case SMSG_FORCE_MOVE_ROOT:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[5]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
#else // Mop
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
#endif
                } break;
                case SMSG_FORCE_MOVE_UNROOT:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[1]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
#else // Mop
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
#endif
                } break;
                case SMSG_MOVE_GRAVITY_DISABLE:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[2]);
#else // Mop
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
#endif
                } break;
                case SMSG_MOVE_GRAVITY_ENABLE:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
#else // Mop
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[5]);
#endif
                } break;
                case SMSG_MOVE_SET_HOVER:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[7]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
                    packet << uint32_t(0);
#else // Mop
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[7]);
#endif

                } break;
                case SMSG_MOVE_UNSET_HOVER:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[0]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet << uint32_t(0);
#else // Mop
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[7]);
#endif
                } break;
                case SMSG_MOVE_NORMAL_FALL:
                {
#if VERSION_STRING == Cata
                    packet << uint32_t(0);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[6]);
#else // Mop
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[6]);
#endif
                } break;
                case SMSG_MOVE_FEATHER_FALL:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
#else // Mop
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[6]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[7]);
#endif
                } break;
                case SMSG_MOVE_LAND_WALK:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(0);
#else // Mop
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[1]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[5]);
#endif
                } break;
                case SMSG_MOVE_WATER_WALK:
                {
#if VERSION_STRING == Cata
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[5]);
                    packet.WriteByteSeq(guid[2]);
                    packet << uint32_t(0);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
#else // Mop
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[6]);
                    packet.WriteByteSeq(guid[4]);
                    packet.WriteByteSeq(guid[7]);
                    packet.WriteByteSeq(guid[0]);
                    packet.WriteByteSeq(guid[1]);
                    packet.WriteByteSeq(guid[6]);
                    packet.WriteByteSeq(guid[2]);
                    packet.WriteByteSeq(guid[3]);
                    packet.WriteByteSeq(guid[5]);
                    packet << uint32_t(0);
#endif
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
