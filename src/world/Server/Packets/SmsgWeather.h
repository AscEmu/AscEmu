/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgWeather : public ManagedPacket
    {
#if VERSION_STRING != Cata
    public:
        uint32_t type;
        float_t density;
        uint32_t sound;

        SmsgWeather() : SmsgWeather(0, 0, 0)
        {
        }

        SmsgWeather(uint32_t type, float_t density, uint32_t sound) :
            ManagedPacket(SMSG_WEATHER, 13),
            type(type),
            density(density),
            sound(sound)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << type << density << sound << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override { return false; }
#endif
    };
}}
