/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "AEVersion.hpp"
#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class MsgSetDungeonDifficulty : public ManagedPacket
    {
    public:
        uint8_t difficulty;
        uint32_t unknown;
        bool isInGroup;

        MsgSetDungeonDifficulty() : MsgSetDungeonDifficulty(0, 0, false)
        {
        }

        MsgSetDungeonDifficulty(uint8_t difficulty, uint32_t unknown, bool isInGroup) :
            ManagedPacket(MSG_SET_DUNGEON_DIFFICULTY, 1),
            difficulty(difficulty),
            unknown(unknown),
            isInGroup(isInGroup)
        {
        }

    protected:
#if VERSION_STRING == Mop
        size_t expectedSize() const override { return 4; }
#else
        size_t expectedSize() const override { return 12; }
#endif

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            packet << uint32_t(difficulty);
#else
            packet << uint32_t(difficulty) << unknown << uint32_t(isInGroup);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> difficulty;
            return true;
        }
    };
}
