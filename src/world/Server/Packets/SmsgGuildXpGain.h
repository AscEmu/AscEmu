/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildXpGain : public ManagedPacket
    {
    public:
        uint64_t xp = 0;

        SmsgGuildXpGain() : SmsgGuildXpGain(0)
        {
        }

        SmsgGuildXpGain(uint64_t xp) :
            ManagedPacket(SMSG_GUILD_XP_GAIN, 8),
            xp(xp)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet << uint64_t(xp);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
