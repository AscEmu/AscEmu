/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgArenaError : public ManagedPacket
    {
    public:
        uint32_t unknown;
        uint8_t type;

        SmsgArenaError() : SmsgArenaError(0, 0)
        {
        }

        SmsgArenaError(uint32_t unknown, uint8_t type) :
            ManagedPacket(SMSG_ARENA_ERROR, 5),
            unknown(unknown),
            type(type)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown << type;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
