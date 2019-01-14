/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

#if VERSION_STRING < Cata
#if VERSION_STRING != Mop

#include "Data/MovementInfo.h"

namespace AscEmu { namespace Packets
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
            packet >> info.flags >> info.flags2 >> info.time
                >> info.position >> info.position.o;

            if (info.isSwimmingOrFlying())
                packet >> info.pitch;

            packet >> info.fall_time;

            if (info.isFallingOrRedirected())
                packet >> info.redirect_velocity >> info.redirect_sin >> info.redirect_cos >> info.redirect_2d_speed;

            if (info.isSplineMover())
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseClassic(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.flags2 << info.time
                << info.position << info.position.o;

            if (info.isSwimmingOrFlying())
                packet << info.pitch;

            packet << info.fall_time;

            if (info.isFallingOrRedirected())
                packet << info.redirect_velocity << info.redirect_sin << info.redirect_cos << info.redirect_2d_speed;

            if (info.isSplineMover())
                packet << info.spline_elevation;

            return true;
        }
#elif VERSION_STRING == TBC
        bool deserialiseTbc(WorldPacket& packet)
        {
            packet >> info.flags >> info.flags2 >> info.time
                    >> info.position >> info.position.o;

            if (info.isOnTransport())
                packet >> info.transport_data.transportGuid >> info.transport_data.relativePosition
                        >> info.transport_data.relativePosition.o >> info.transport_time;

            if (info.isSwimmingOrFlying())
                packet >> info.pitch;

            packet >> info.fall_time;

            if (info.isFallingOrRedirected())
                packet >> info.redirect_velocity >> info.redirect_sin >> info.redirect_cos >> info.redirect_2d_speed;

            if (info.isSplineMover())
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseTbc(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.flags2 << info.time
                    << info.position << info.position.o;

            if (info.isOnTransport())
                packet << info.transport_data.transportGuid << info.transport_data.relativePosition
                        << info.transport_data.relativePosition.o << info.transport_time;

            if (info.isSwimmingOrFlying())
                packet << info.pitch;

            packet << info.fall_time;

            if (info.isFallingOrRedirected())
                packet << info.redirect_velocity << info.redirect_sin << info.redirect_cos << info.redirect_2d_speed;

            if (info.isSplineMover())
                packet << info.spline_elevation;

            return true;
        }
#elif VERSION_STRING == WotLK
        bool deserialiseWotlk(WorldPacket& packet)
        {
            packet >> guid;
            packet >> info.flags >> info.flags2 >> info.time
                >> info.position >> info.position.o;

            if (info.isOnTransport())
            {
                packet >> info.transport_data.transportGuid >> info.transport_data.relativePosition
                    >> info.transport_data.relativePosition.o >> info.transport_time >> info.transport_seat;

                if (info.isInterpolated())
                    packet >> info.transport_time2;
            }

            if (info.isSwimmingOrFlying())
                packet >> info.pitch;

            packet >> info.fall_time;

            if (info.isFallingOrRedirected())
                packet >> info.redirect_velocity >> info.redirect_sin >> info.redirect_cos >> info.redirect_2d_speed;

            if (info.isSplineMover())
                packet >> info.spline_elevation;

            return !packet.hadReadFailure();
        }

        bool serialiseWotlk(WorldPacket& packet)
        {
            packet << guid;
            packet << info.flags << info.flags2 << info.time
                << info.position << info.position.o;

            if (info.isOnTransport())
            {
                packet << info.transport_data.transportGuid << info.transport_data.relativePosition
                << info.transport_data.relativePosition.o << info.transport_time << info.transport_seat;

                if (info.isInterpolated())
                    packet << info.transport_time2;
            }

            if (info.isSwimmingOrFlying())
                packet << info.pitch;

            packet << info.fall_time;

            if (info.isFallingOrRedirected())
                packet << info.redirect_velocity << info.redirect_sin << info.redirect_cos << info.redirect_2d_speed;

            if (info.isSplineMover())
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
}}

#endif
#endif
