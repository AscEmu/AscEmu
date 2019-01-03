/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgInstanceDifficulty : public ManagedPacket
    {
    public:
        uint32_t difficulty;
        uint32_t unk1;

        SmsgInstanceDifficulty() : SmsgInstanceDifficulty(0)
        {
        }

        SmsgInstanceDifficulty(uint32_t difficulty) :
            ManagedPacket(SMSG_INSTANCE_DIFFICULTY, 0),
            difficulty(difficulty),
            unk1(0)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << difficulty << unk1;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
