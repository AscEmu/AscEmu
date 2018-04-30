/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
            ManagedPacket(SMSG_INSTANCE_DIFFICULTY, sizeof(uint32_t) * 2),
            difficulty(difficulty),
            unk1(0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            // All versions share same implementation
            packet << difficulty << unk1;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> difficulty >> unk1;
            return true;
        }
    };
}}
