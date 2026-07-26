/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLearnedDanceMoves : public ManagedPacket
    {
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
            if (m_protocol.expansion <= WoW::Expansion::_TBC ||
                m_protocol.expansion == WoW::Expansion::_Mop)
                return false;

            packet << unknown1 << unknown2;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
