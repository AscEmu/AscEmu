/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLearnedDanceMoves : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t unknown1;
        uint32_t unknown2;

        SmsgLearnedDanceMoves() : SmsgLearnedDanceMoves(0, 0)
        {
        }

        SmsgLearnedDanceMoves(uint32_t unknown1, uint32_t unknown2) :
            ManagedPacket(SMSG_LEARNED_DANCE_MOVES, 8),
            unknown1(unknown1),
            unknown2(unknown2)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << unknown1 << unknown2;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
