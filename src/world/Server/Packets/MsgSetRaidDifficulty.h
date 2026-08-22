/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgSetRaidDifficulty : public ManagedPacket
    {
    public:
        uint32_t difficulty;
        uint32_t unknown;
        bool isInGroup;

        MsgSetRaidDifficulty() : MsgSetRaidDifficulty(0, 0, false)
        {
        }

        MsgSetRaidDifficulty(uint8_t difficulty, uint32_t unknown, bool isInGroup) :
            ManagedPacket(MSG_SET_RAID_DIFFICULTY, 4),
            difficulty(difficulty),
            unknown(unknown),
            isInGroup(isInGroup)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return 0;
            return m_protocol.isMop() ? 4 : 12;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            if (m_protocol.isMop())
            {
                packet.initialize(SMSG_SET_RAID_DIFFICULTY, 4);
                packet << uint32_t(difficulty);
                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << uint32_t(difficulty) << unknown << uint32_t(isInGroup);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_TBC)
                return false;

            packet >> difficulty;
            return true;
        }
    };
}
