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
    class MsgGuildEventLogQuery : public ManagedPacket
    {
    public:
        GuildLogHolder const* eventLog = nullptr;

        MsgGuildEventLogQuery() : MsgGuildEventLogQuery(nullptr)
        {
        }

        MsgGuildEventLogQuery(GuildLogHolder const* eventLog) :
            ManagedPacket(MSG_GUILD_EVENT_LOG_QUERY, 1 + (eventLog ? eventLog->getSize() : 0) * (1 + 8 + 4)),
            eventLog(eventLog)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + (eventLog ? eventLog->getSize() : 0) * (1 + 8 + 4); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                return false;

            if (eventLog)
                eventLog->writeLogHolderPacket(packet);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
