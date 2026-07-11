/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgRemoveGlyph : public ManagedPacket
    {
    public:
        uint16_t glyphNumber;

        CmsgRemoveGlyph() : CmsgRemoveGlyph(0)
        {
        }

        CmsgRemoveGlyph(uint16_t glyphNumber) :
            ManagedPacket(CMSG_REMOVE_GLYPH, 2),
            glyphNumber(glyphNumber)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> glyphNumber;
            return true;
        }
    };
}
