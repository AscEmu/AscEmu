/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildLogHolder.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGuildEventLogQueryResult : public ManagedPacket
    {
    public:
        GuildLogHolder const* eventLog = nullptr;

        SmsgGuildEventLogQueryResult() : SmsgGuildEventLogQueryResult(nullptr)
        {
        }

        SmsgGuildEventLogQueryResult(GuildLogHolder const* eventLog) :
            ManagedPacket(SMSG_GUILD_EVENT_LOG_QUERY_RESULT, 1 + (eventLog ? eventLog->getSize() : 0) * (1 + 8 + 4)),
            eventLog(eventLog)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + (eventLog ? eventLog->getSize() : 0) * (1 + 8 + 4); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                return false;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                if (eventLog)
                    eventLog->writeLogHolderPacket(packet);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
