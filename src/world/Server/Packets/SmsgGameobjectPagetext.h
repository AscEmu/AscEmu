/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGameobjectPagetext : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgGameobjectPagetext() : SmsgGameobjectPagetext(0)
        {
        }

        SmsgGameobjectPagetext(uint64_t guid) :
            ManagedPacket(SMSG_GAMEOBJECT_PAGETEXT, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
