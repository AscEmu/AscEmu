/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class PACKET_NAME_REPLACE_ME : public ManagedPacket
    {
    public:
        uint8_t ex1;
        uint32_t ex2;

        PACKET_NAME_REPLACE_ME() : PACKET_NAME_REPLACE_ME(0, 0)
        {
        }

        PACKET_NAME_REPLACE_ME(uint8_t ex1, uint32_t ex2) :
            ManagedPacket(OPCODE_REPLACE_ME, MINIMUM_PACKET_SIZE_REPLACE_ME),
            ex1(ex1),
            ex2(ex2)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            return true;
        }
    };
}}
