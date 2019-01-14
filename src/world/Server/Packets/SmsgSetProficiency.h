/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
#if VERSION_STRING != Mop
            packet << itemClass << proficiency;
#elif VERSION_STRING == Mop
            packet << proficiency << itemClass;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
