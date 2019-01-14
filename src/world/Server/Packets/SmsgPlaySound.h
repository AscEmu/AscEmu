/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPlaySound : public ManagedPacket
    {
    public:
        uint32_t soundId;

        SmsgPlaySound() : SmsgPlaySound(0)
        {
        }

        SmsgPlaySound(uint32_t soundId) :
            ManagedPacket(SMSG_PLAY_SOUND, 0),
            soundId(soundId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << soundId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
