/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPlayMusic : public ManagedPacket
    {
    public:
        uint32_t musicId;

        SmsgPlayMusic() : SmsgPlayMusic(0)
        {
        }

        explicit SmsgPlayMusic(uint32_t musicId) :
            ManagedPacket(SMSG_PLAY_MUSIC, 4),
            musicId(musicId)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << musicId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
