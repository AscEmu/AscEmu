/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGameobjectDespawnAnim : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgGameobjectDespawnAnim() : SmsgGameobjectDespawnAnim(0)
        {
        }

        SmsgGameobjectDespawnAnim(uint64_t guid) :
            ManagedPacket(SMSG_GAMEOBJECT_DESPAWN_ANIM, 0),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
