/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgSetRaidDifficulty : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t difficulty;
        uint32_t unknown;
        bool isInGroup;

        MsgSetRaidDifficulty() : MsgSetRaidDifficulty(0, 0, false)
        {
        }

        MsgSetRaidDifficulty(uint8_t difficulty, uint32_t unknown, bool isInGroup) :
            ManagedPacket(MSG_SET_RAID_DIFFICULTY, 12),
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
#endif
    };
}}
