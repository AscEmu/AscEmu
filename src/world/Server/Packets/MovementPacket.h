/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "Objects/MovementInfo.h"

#if VERSION_STRING < Cata
#if VERSION_STRING != Mop

namespace AscEmu::Packets
{
    class MovementPacket : public ManagedPacket
    {
    public:
        WoWGuid guid;
        MovementInfo info;

        MovementPacket() : MovementPacket(0, 0)
        {
        }

        MovementPacket(uint16_t opcode, size_t size) :
            ManagedPacket(opcode, size),
            info()
        {
        }

    private:
#if VERSION_STRING == Classic
        bool deserialiseClassic(WorldPacket& packet)
        {
            packet >> info.flags >> info.update_time
                >> info.position >> info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
                packet >> info.transport_guid >> info.transport_position
                        >> info.transport_position.o;

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
                packet >> info.pitch_rate;

            packet >> info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet >> info.jump_info.velocity >> info.jump_info.sinAngle >> info.jump_info.cosAngle >> info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseClassic(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.update_time
                << info.position << info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
                packet << info.transport_guid << info.transport_position
                        << info.transport_position.o;

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)))
                packet << info.pitch_rate;

            packet << info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet << info.jump_info.velocity << info.jump_info.sinAngle << info.jump_info.cosAngle << info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet << info.spline_elevation;

            return true;
        }
#elif VERSION_STRING == TBC
        bool deserialiseTbc(WorldPacket& packet)
        {
            packet >> info.flags >> info.flags2 >> info.update_time
                    >> info.position >> info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
                packet >> info.transport_guid >> info.transport_position
                        >> info.transport_position.o >> info.transport_time;

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || info.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
                packet >> info.pitch_rate;

            packet >> info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet >> info.jump_info.velocity >> info.jump_info.sinAngle >> info.jump_info.cosAngle >> info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseTbc(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.flags2 << info.update_time
                    << info.position << info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
                packet << info.transport_guid << info.transport_position
                        << info.transport_position.o << info.transport_time;

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || info.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
                packet << info.pitch_rate;

            packet << info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet << info.jump_info.velocity << info.jump_info.sinAngle << info.jump_info.cosAngle << info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet << info.spline_elevation;

            return true;
        }
#elif VERSION_STRING == WotLK
        bool deserialiseWotlk(WorldPacket& packet)
        {
            packet >> guid;
            packet >> info.flags >> info.flags2 >> info.update_time
                >> info.position >> info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
            {
                packet >> info.transport_guid >> info.transport_position
                    >> info.transport_position.o >> info.transport_time >> info.transport_seat;

                if (info.hasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
                    packet >> info.transport_time2;
            }

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || info.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
                packet >> info.pitch_rate;

            packet >> info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet >> info.jump_info.velocity >> info.jump_info.sinAngle >> info.jump_info.cosAngle >> info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseWotlk(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.flags2 << info.update_time
                << info.position << info.position.o;

            if (info.hasMovementFlag(MOVEFLAG_TRANSPORT))
            {
                packet << info.transport_guid << info.transport_position
                << info.transport_position.o << info.transport_time << info.transport_seat;

                if (info.hasMovementFlag2(MOVEFLAG2_INTERPOLATED_MOVE))
                    packet << info.transport_time2;
            }

            if (info.hasMovementFlag(MovementFlags(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING)) || info.hasMovementFlag2(MOVEFLAG2_ALLOW_PITCHING))
                packet << info.pitch_rate;

            packet << info.fall_time;

            if (info.hasMovementFlag(MOVEFLAG_FALLING))
                packet << info.jump_info.velocity << info.jump_info.sinAngle << info.jump_info.cosAngle << info.jump_info.xyspeed;

            if (info.hasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
                packet << info.spline_elevation;

            return true;
        }
#endif

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Classic
            return serialiseClassic(packet);
#elif VERSION_STRING == TBC
            return serialiseTbc(packet);
#elif VERSION_STRING == WotLK
            return serialiseWotlk(packet);
#endif
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Classic
            return deserialiseClassic(packet);
#elif VERSION_STRING == TBC
            return deserialiseTbc(packet);
#elif VERSION_STRING == WotLK
            return deserialiseWotlk(packet);
#endif
        }
    };
}

#endif
#endif
