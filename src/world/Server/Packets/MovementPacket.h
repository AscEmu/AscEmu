/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "GameTBC/Data/MovementInfoTBC.h"

namespace AscEmu { namespace Packets
{
    class MovementPacket : public ManagedPacket
    {
    public:
        MovementInfo info;

    private:
        bool deserialiseTbc(WorldPacket& packet)
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid;
            info.guid = WoWGuid(unpacked_guid);

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
            packet << info.guid.GetOldGuid() << info.flags << info.flags2 << info.time
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

    public:
        MovementPacket(uint16_t opcode, size_t size) :
            ManagedPacket(opcode, size),
            info()
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == TBC
            return serialiseTbc(packet);
#else
            // TODO Implement different versions
            return serialiseTbc(packet);
#endif
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == TBC
            return deserialiseTbc(packet);
#else
            // TODO: Implement different versions
            return deserialiseTbc(packet);
#endif
        }
    };
}}
