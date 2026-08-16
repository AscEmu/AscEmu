/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildMaxDailyXp : public ManagedPacket
    {
    public:
        uint64_t maxDailyXp;

        SmsgGuildMaxDailyXp() : SmsgGuildMaxDailyXp(0)
        {
        }

        SmsgGuildMaxDailyXp(uint64_t maxDailyXp) :
            ManagedPacket(SMSG_GUILD_MAX_DAILY_XP, 8),
            maxDailyXp(maxDailyXp)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint64_t(maxDailyXp);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
