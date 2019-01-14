/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgRemoveGlyph : public ManagedPacket
    {
#if VERSION_STRING > TBC
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> glyphNumber;
            return true;
        }
#endif
    };
}}
