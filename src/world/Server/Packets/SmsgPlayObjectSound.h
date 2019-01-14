/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPlayObjectSound : public ManagedPacket
    {
    public:
        uint32_t soundId;
        uint64_t objectGuid;

        SmsgPlayObjectSound() : SmsgPlayObjectSound(0, 0)
        {
        }

        SmsgPlayObjectSound(uint32_t soundId, uint64_t objectGuid) :
            ManagedPacket(SMSG_PLAY_OBJECT_SOUND, 0),
            soundId(soundId),
            objectGuid(objectGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << soundId << objectGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
