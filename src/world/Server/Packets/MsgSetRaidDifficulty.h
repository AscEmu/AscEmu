/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class MsgSetRaidDifficulty : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t difficulty;
        uint32_t unknown;
        bool isInGroup;

        MsgSetRaidDifficulty() : MsgSetRaidDifficulty(0, 0, false)
        {
        }

        MsgSetRaidDifficulty(uint8_t difficulty, uint32_t unknown, bool isInGroup) :
            ManagedPacket(MSG_SET_RAID_DIFFICULTY, 0),
            difficulty(difficulty),
            unknown(unknown),
            isInGroup(isInGroup)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 12;
        }

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
#endif
    };
}
