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
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // an empty reward block: mask, money, xp, item, currency and bonus counts, the optional values
                const auto writeReward = [&packet]()
                {
                    for (uint8_t i = 0; i < 6; ++i)
                        packet << uint32_t(0);
                };

                if (m_protocol.expansion == WoW::Expansion::_WoD)
                {
                    // 6.x: the weekly conquest values, then the rated and arena rewards
                    packet << currencyOnWeekConquestPoints;
                    packet << currencyWeekCapConquestPoints;
                    packet << currencyOnWeekConquestArena;
                    packet << currencyWeekCapConquestArena;
                    packet << uint32_t(0);                                      // ashran on week
                    packet << uint32_t(0);                                      // ashran week cap
                    packet << currencyOnWeekConquestRandomBattleground;
                    packet << uint32_t(0);                                      // battleground week cap
                    packet << uint32_t(0);                                      // conquest per rated battleground win
                    packet << uint32_t(0);                                      // conquest per arena win
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        writeReward();
                        packet.writeBit(false);
                        packet.flushBits();
                    }
                    return true;
                }

                if (m_protocol.expansion == WoW::Expansion::_Legion)
                {
                    // 7.x: random battleground reward, the win flags of the day, then the six rated and arena rewards
                    const auto writeLegionReward = [&]()
                    {
                        writeReward();
                        for (uint8_t i = 0; i < 4; ++i)
                            packet.writeBit(false);
                        packet.flushBits();
                    };
                    writeLegionReward();
                    for (uint8_t i = 0; i < 6; ++i)
                        packet.writeBit(false);
                    packet.flushBits();
                    for (uint8_t i = 0; i < 6; ++i)
                        writeLegionReward();
                    return true;
                }

                // 8.x+ clients get no answer
                return false;
            }

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
