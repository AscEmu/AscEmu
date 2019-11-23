/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGameobjectCustomAnim : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t state;

        SmsgGameobjectCustomAnim() : SmsgGameobjectCustomAnim(0, 0)
        {
        }

        SmsgGameobjectCustomAnim(uint64_t guid, uint32_t state) :
            ManagedPacket(SMSG_GAMEOBJECT_CUSTOM_ANIM, 0),
            guid(guid),
            state(state)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << state;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
