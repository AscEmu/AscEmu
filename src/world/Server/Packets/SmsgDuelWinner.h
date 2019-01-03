/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgDuelWinner : public ManagedPacket
    {
    public:
        uint8_t winnerCondition; // 0 = enemy won, 1 = we won
        std::string ourName;
        std::string enemyName;

        SmsgDuelWinner() : SmsgDuelWinner(0, "", "")
        {
        }

        SmsgDuelWinner(uint8_t winnerCondition, std::string ourName, std::string enemyName) :
            ManagedPacket(SMSG_DUEL_WINNER, 0),
            winnerCondition(winnerCondition),
            ourName(ourName),
            enemyName(enemyName)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + ourName.length() + enemyName.length();
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << winnerCondition << ourName.c_str() << enemyName.c_str();
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
