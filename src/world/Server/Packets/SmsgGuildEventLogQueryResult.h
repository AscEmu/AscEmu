/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Guild/GuildEventLog.hpp"
#include <ctime>
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
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // every entry with both players, the type, the rank and the seconds since the event
                const std::time_t now = ::time(nullptr);
                uint32_t count = 0;
                if (eventLog != nullptr)
                {
                    for (const auto& entry : eventLog->getLog())
                    {
                        if (dynamic_cast<GuildEventLogEntry const*>(entry.get()) != nullptr)
                            ++count;
                    }
                }

                packet << uint32_t(count);
                if (eventLog == nullptr)
                    return true;

                for (const auto& entry : eventLog->getLog())
                {
                    const auto eventEntry = dynamic_cast<GuildEventLogEntry const*>(entry.get());
                    if (eventEntry == nullptr)
                        continue;

                    packet << WoWGuid(eventEntry->getPlayerGuid1(), 0, HIGHGUID_TYPE_PLAYER).toGuid128(m_protocol.realmId, 0);
                    packet << WoWGuid(eventEntry->getPlayerGuid2(), 0, HIGHGUID_TYPE_PLAYER).toGuid128(m_protocol.realmId, 0);
                    packet << uint8_t(eventEntry->getEventType());
                    packet << uint8_t(eventEntry->getNewRank());
                    packet << uint32_t(now - static_cast<std::time_t>(eventEntry->getTimestamp()));
                }
                return true;
            }

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
