/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildReputationWeeklyCap : public ManagedPacket
    {
    public:
        uint32_t cap = 0;

        SmsgGuildReputationWeeklyCap() : SmsgGuildReputationWeeklyCap(0)
        {
        }

        SmsgGuildReputationWeeklyCap(uint32_t cap) :
            ManagedPacket(SMSG_GUILD_REPUTATION_WEEKLY_CAP, 4),
            cap(cap)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet << uint32_t(cap);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
