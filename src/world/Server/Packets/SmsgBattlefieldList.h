/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/Battleground/Battleground.hpp"
#include "Management/Battleground/BattlegroundDefines.hpp"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgBattlefieldList : public ManagedPacket
    {
    public:
        // WotLK / TBC / Classic - raw player guid, only sent when 'from' requests it (WotLK)
        uint64_t playerGuid = 0;
        // Cata+/Mop - guid used for the bit-packed layout (currently always the null guid at call sites)
        WoWGuid guid;
        // WotLK only - which UI requested the list (Battlemaster vs BG UI)
        uint8_t from = 0;
        uint32_t battlegroundType = 0;
        // filtered, joinable, non-ended battleground instance ids for battlegroundType
        std::vector<uint32_t> bgInstanceIds;

        // WotLK random-bg rewards, only used when battlegroundType == BattlegroundDef::TYPE_RANDOM
        bool hasWonRbgToday = false;
        uint32_t honorPointsForWinning = 0;
        uint32_t arenaPointsForWinning = 0;
        uint32_t honorPointsForLosing = 0;

        SmsgBattlefieldList() : ManagedPacket(SMSG_BATTLEFIELD_LIST, 0)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
                return 18;

            return 38 + bgInstanceIds.size();
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWotlk())
            {
                // Send 0 instead of GUID when using the BG UI instead of Battlemaster
                if (from == 0)
                    packet << uint64_t(playerGuid);
                else
                    packet << uint64_t(0);

                packet << from;
                packet << battlegroundType;                                     // typeid

                packet << uint8_t(0);                                                     // unk
                packet << uint8_t(0);                                                     // unk

                // Rewards
                packet << uint8_t(0);                                                     // 3.3.3 hasWin
                packet << uint32_t(0);                                                    // 3.3.3 winHonor
                packet << uint32_t(0);                                                    // 3.3.3 winArena
                packet << uint32_t(0);                                                    // 3.3.3 lossHonor

                const uint8_t isRandom = battlegroundType == BattlegroundDef::TYPE_RANDOM;
                packet << isRandom;                                                       // 3.3.3 isRandom

                // Random bgs
                if (isRandom == 1)
                {
                    // rewards
                    packet << uint8_t(hasWonRbgToday);
                    packet << honorPointsForWinning;
                    packet << arenaPointsForWinning;
                    packet << honorPointsForLosing;
                }

                if (Battleground::isTypeArena(battlegroundType))
                {
                    packet << uint32_t(0);
                    return true;
                }

                packet << uint32_t(bgInstanceIds.size()); // Count

                for (uint32_t bgId : bgInstanceIds)
                    packet << bgId;
            }
            else if (m_protocol.isLegacy())
            {
                packet << uint64_t(playerGuid);
                packet << battlegroundType;

                if (Battleground::isTypeArena(battlegroundType))
                {
                    packet << uint8_t(5);
                    packet << uint32_t(0);
                }
                else
                {
                    packet << uint8_t(0);

                    packet << uint32_t(bgInstanceIds.size()); // Count

                    for (uint32_t bgId : bgInstanceIds)
                        packet << bgId;
                }
            }
            else if (m_protocol.isCata())
            {
                packet << int32_t(0);
                packet << int32_t(0);
                packet << int32_t(0);
                packet << int32_t(battlegroundType);
                packet << int32_t(0);
                packet << int32_t(0);
                packet << int32_t(0);
                packet << uint8_t(80);
                packet << uint8_t(10);

                packet.writeBit(guid[0]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[7]);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBits(bgInstanceIds.size(), 24);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(1);
                packet.writeBit(guid[5]);
                packet.writeBit(0);
                packet.flushBits();

                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[5]);

                for (uint32_t bgId : bgInstanceIds)
                    packet << int32_t(bgId);

                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
            }
            else if (m_protocol.isMop())
            {
                packet << uint32_t(0);
                packet << uint32_t(0);
                packet << uint8_t(10);
                packet << uint32_t(0);
                packet << uint32_t(0);
                packet << uint32_t(battlegroundType);
                packet << uint32_t(0);
                packet << uint8_t(80);
                packet << uint32_t(0);

                packet.writeBit(guid[0]);
                packet.writeBit(0);
                packet.writeBit(guid[4]);
                packet.writeBit(0);
                packet.writeBit(guid[2]);
                packet.writeBit(0);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(0);
                packet.writeBit(guid[3]);

                packet.writeBits(bgInstanceIds.size(), 22);

                packet.flushBits();

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);

                for (uint32_t bgId : bgInstanceIds)
                    packet << int32_t(bgId);
            }
            else
            {
                return false;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
