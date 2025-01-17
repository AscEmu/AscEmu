/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGuildMemberDailyReset : public ManagedPacket
    {
#if VERSION_STRING >= Cata
    public:

        SmsgGuildMemberDailyReset() :
            ManagedPacket(SMSG_GUILD_MEMBER_DAILY_RESET, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
