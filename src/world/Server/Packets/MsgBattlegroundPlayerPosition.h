/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

#include "Objects/Units/Players/Player.hpp"

namespace AscEmu::Packets
{
    class MsgBattlegroundPlayerPosition : public ManagedPacket
    {
    public:
        uint32_t unknown1;
        uint32_t flagHolderCount;

        Player* alliancePlayer;
        Player* hordePlayer;

        MsgBattlegroundPlayerPosition() : MsgBattlegroundPlayerPosition(0, 0, nullptr, nullptr)
        {
        }

        MsgBattlegroundPlayerPosition(uint32_t unknown1, uint32_t flagHolderCount, Player* alliancePlayer, Player* hordePlayer) :
            ManagedPacket(MSG_BATTLEGROUND_PLAYER_POSITIONS, 4 + 4 + 16 * flagHolderCount),
            unknown1(unknown1),
            flagHolderCount(flagHolderCount),
            alliancePlayer(alliancePlayer),
            hordePlayer(hordePlayer)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                const WoWGuid allianceGuid(alliancePlayer != nullptr ? alliancePlayer->getGuid() : 0);
                const WoWGuid hordeGuid(hordePlayer != nullptr ? hordePlayer->getGuid() : 0);
                const uint32_t allianceCount = alliancePlayer != nullptr ? 1 : 0;
                const uint32_t hordeCount = hordePlayer != nullptr ? 1 : 0;

                packet.writeBits(allianceCount, 22);
                for (uint32_t i = 0; i < allianceCount; ++i)
                {
                    packet.writeBit(allianceGuid[3]);
                    packet.writeBit(allianceGuid[5]);
                    packet.writeBit(allianceGuid[1]);
                    packet.writeBit(allianceGuid[6]);
                    packet.writeBit(allianceGuid[7]);
                    packet.writeBit(allianceGuid[0]);
                    packet.writeBit(allianceGuid[2]);
                    packet.writeBit(allianceGuid[4]);
                }

                packet.writeBits(hordeCount, 22);
                for (uint32_t i = 0; i < hordeCount; ++i)
                {
                    packet.writeBit(hordeGuid[6]);
                    packet.writeBit(hordeGuid[5]);
                    packet.writeBit(hordeGuid[4]);
                    packet.writeBit(hordeGuid[7]);
                    packet.writeBit(hordeGuid[2]);
                    packet.writeBit(hordeGuid[1]);
                    packet.writeBit(hordeGuid[0]);
                    packet.writeBit(hordeGuid[3]);
                }

                packet.flushBits();

                for (uint32_t i = 0; i < hordeCount; ++i)
                {
                    packet.writeByteSeq(hordeGuid[2]);
                    packet.writeByteSeq(hordeGuid[1]);
                    packet << hordePlayer->GetPositionY();
                    packet.writeByteSeq(hordeGuid[5]);
                    packet.writeByteSeq(hordeGuid[4]);
                    packet.writeByteSeq(hordeGuid[7]);
                    packet.writeByteSeq(hordeGuid[0]);
                    packet.writeByteSeq(hordeGuid[6]);
                    packet.writeByteSeq(hordeGuid[3]);
                    packet << hordePlayer->GetPositionX();
                }

                for (uint32_t i = 0; i < allianceCount; ++i)
                {
                    packet.writeByteSeq(allianceGuid[6]);
                    packet << alliancePlayer->GetPositionX();
                    packet.writeByteSeq(allianceGuid[5]);
                    packet.writeByteSeq(allianceGuid[3]);
                    packet << alliancePlayer->GetPositionY();
                    packet.writeByteSeq(allianceGuid[1]);
                    packet.writeByteSeq(allianceGuid[7]);
                    packet.writeByteSeq(allianceGuid[0]);
                    packet.writeByteSeq(allianceGuid[2]);
                    packet.writeByteSeq(allianceGuid[4]);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << unknown1 << flagHolderCount;
                if (alliancePlayer != nullptr)
                    packet << alliancePlayer->getGuid() << alliancePlayer->GetPositionX() << alliancePlayer->GetPositionY();

                if (hordePlayer != nullptr)
                    packet << hordePlayer->getGuid() << hordePlayer->GetPositionX() << hordePlayer->GetPositionY();

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
