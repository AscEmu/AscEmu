/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSetProficiency : public ManagedPacket
    {
    public:
        uint8_t itemClass;
        uint32_t proficiency;

        SmsgSetProficiency() : SmsgSetProficiency(0, 0)
        {
        }

        SmsgSetProficiency(uint8_t item_class, uint32_t proficiency) :
            ManagedPacket(SMSG_SET_PROFICIENCY, 0),
            itemClass(item_class),
            proficiency(proficiency)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 2 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion != WoW::Expansion::_Mop)
            {
                packet << itemClass << proficiency;
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                packet << proficiency << itemClass;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
