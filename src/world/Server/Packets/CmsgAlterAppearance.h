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
    class CmsgAlterAppearance : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t hair;
        uint32_t hairColor;
        uint32_t facialHairOrPiercing;
        uint32_t skinColor;

        CmsgAlterAppearance() : CmsgAlterAppearance(0, 0, 0, 0)
        {
        }

        CmsgAlterAppearance(uint32_t hair, uint32_t hairColor, uint32_t facialHairOrPiercing, uint32_t skinColor) :
            ManagedPacket(CMSG_ALTER_APPEARANCE, 16),
            hair(hair),
            hairColor(hairColor),
            facialHairOrPiercing(facialHairOrPiercing),
            skinColor(skinColor)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> hair >> hairColor >> facialHairOrPiercing >> skinColor;
            return true;
        }
#endif
    };
}}
