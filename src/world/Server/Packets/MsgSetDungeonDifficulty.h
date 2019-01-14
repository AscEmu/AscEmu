/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
            ManagedPacket(MSG_SET_DUNGEON_DIFFICULTY, 12),
            difficulty(difficulty),
            unknown(unknown),
            isInGroup(isInGroup)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint32_t(difficulty) << unknown << uint32_t(isInGroup);
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> difficulty;
            return true;
        }
    };
}}
