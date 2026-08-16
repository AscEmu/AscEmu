/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRequestPvpRewardsResponse : public ManagedPacket
    {
    public:
        uint32_t currencyWeekCapConquestPoints;
        uint32_t currencyOnWeekConquestPoints;
        uint32_t currencyWeekCapConquestArena;
        uint32_t currencyOnWeekConquestRandomBattleground;
        uint32_t currencyOnWeekConquestArena;
        uint32_t currencyWeekCapConquestPoints2;

        SmsgRequestPvpRewardsResponse() : SmsgRequestPvpRewardsResponse(0, 0, 0, 0, 0, 0)
        {
        }

        SmsgRequestPvpRewardsResponse(uint32_t currencyWeekCapConquestPoints, uint32_t currencyOnWeekConquestPoints, uint32_t currencyWeekCapConquestArena,
            uint32_t currencyOnWeekConquestRandomBattleground, uint32_t currencyOnWeekConquestArena, uint32_t currencyWeekCapConquestPoints2) :
            ManagedPacket(SMSG_REQUEST_PVP_REWARDS_RESPONSE, 24),
            currencyWeekCapConquestPoints(currencyWeekCapConquestPoints),
            currencyOnWeekConquestPoints(currencyOnWeekConquestPoints),
            currencyWeekCapConquestArena(currencyWeekCapConquestArena),
            currencyOnWeekConquestRandomBattleground(currencyOnWeekConquestRandomBattleground),
            currencyOnWeekConquestArena(currencyOnWeekConquestArena),
            currencyWeekCapConquestPoints2(currencyWeekCapConquestPoints2)
        {
        }

    protected:
        size_t expectedSize() const override { return 24; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                return false;

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << currencyWeekCapConquestPoints;               // week cap conquest points
                packet << currencyOnWeekConquestPoints;                // on week conquest points
                packet << currencyWeekCapConquestArena;                // week cap conquest arena
                packet << currencyOnWeekConquestRandomBattleground;    // on week conquest random baattleground
                packet << currencyOnWeekConquestArena;                 // on week conquest arena
                packet << currencyWeekCapConquestPoints2;              // week cap conquest points
            }
            else if (m_protocol.isMop())
            {
                packet << currencyWeekCapConquestPoints;               // week cap conquest points
                packet << currencyOnWeekConquestArena;                 // on week conquest arena
                packet << currencyOnWeekConquestRandomBattleground;    // on week conquest random baattleground
                packet << currencyOnWeekConquestArena;                 // on week conquest arena
                packet << uint32_t(0); //unk
                packet << currencyOnWeekConquestArena;                 // on week conquest arena
                packet << uint32_t(0); //unk
                packet << uint32_t(0);                                 // week cap conquest rbg?
                packet << currencyOnWeekConquestPoints;                // on week conquest points
                packet << currencyWeekCapConquestArena;                // week cap conquest arena
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
