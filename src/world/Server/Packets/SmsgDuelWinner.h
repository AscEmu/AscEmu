/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
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
        size_t expectedSize() const override { return 1 + ourName.length() + enemyName.length(); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << winnerCondition << ourName.c_str() << enemyName.c_str();
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBit(winnerCondition);
                packet.writeBits(enemyName.length(), 6);
                packet.writeBits(ourName.length(), 6);

                packet.flushBits();

                packet << uint32_t(0);            // our realmId
                packet.writeString(enemyName);

                packet << uint32_t(0);            // enemy realmId
                packet.writeString(ourName);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
