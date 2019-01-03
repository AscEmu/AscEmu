/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCharCreate : public ManagedPacket
    {
    public:
        uint8_t result;

        SmsgCharCreate() : SmsgCharCreate(0)
        {
        }

        SmsgCharCreate(uint8_t result) :
            ManagedPacket(SMSG_CHAR_CREATE, 1),
            result(result)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
