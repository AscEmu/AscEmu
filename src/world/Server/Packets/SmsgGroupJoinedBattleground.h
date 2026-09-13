/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Battleground/BattlegroundDefines.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgGroupJoinedBattleground : public ManagedPacket
    {
    public:
        int32_t status;
        uint32_t mapId;
        uint64_t causerGuid;

        SmsgGroupJoinedBattleground() : SmsgGroupJoinedBattleground(BattlegroundDef::GROUP_JOIN_STATUS_FAIL)
        {
        }

        SmsgGroupJoinedBattleground(int32_t status, uint32_t mapId = 0, uint64_t causerGuid = 0) :
            ManagedPacket(SMSG_GROUP_JOINED_BATTLEGROUND, 4 + 8),
            status(status),
            mapId(mapId),
            causerGuid(causerGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                if (status > 0 && m_protocol.isClassic())
                    packet << static_cast<int32_t>(mapId);
                else
                    packet << status;

                // only the WotLK client reads the guid of the player who caused the error, and only for these two results
                if (m_protocol.expansion == WoW::Expansion::_WotLK &&
                    (status == BattlegroundDef::GROUP_JOIN_STATUS_JOIN_TIMED_OUT || status == BattlegroundDef::GROUP_JOIN_STATUS_JOIN_FAILED))
                    packet << causerGuid;

                return true;
            }

            // Cata and later report queue errors with SMSG_BATTLEFIELD_STATUS_FAILED
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
