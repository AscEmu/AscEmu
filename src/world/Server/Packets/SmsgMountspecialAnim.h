/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgMountspecialAnim : public ManagedPacket
    {
    public:
        uint64_t playerGuid;

        SmsgMountspecialAnim() : SmsgMountspecialAnim(0)
        {
        }

        SmsgMountspecialAnim(uint64_t playerGuid) :
            ManagedPacket(SMSG_MOUNTSPECIAL_ANIM, 0),
            playerGuid(playerGuid)
        {
        }

    protected:

        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
