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
    class SmsgBattlefieldStatusFailed : public ManagedPacket
    {
    public:
        WoWGuid playerGuid;
        uint32_t bgType;
        uint32_t queueSlot;
        uint32_t result;
        uint32_t joinTime;

        SmsgBattlefieldStatusFailed() : SmsgBattlefieldStatusFailed(WoWGuid(), 0, 0, BattlegroundDef::JOIN_RESULT_NONE, 0)
        {
        }

        SmsgBattlefieldStatusFailed(WoWGuid playerGuid, uint32_t bgType, uint32_t queueSlot, uint32_t result, uint32_t joinTime) :
            ManagedPacket(SMSG_BATTLEFIELD_STATUS_FAILED, 8 + 8 + 8 + 4 + 4 + 4 + 4),
            playerGuid(playerGuid),
            bgType(bgType),
            queueSlot(queueSlot),
            result(result),
            joinTime(joinTime)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            WoWGuid bgGuid(bgType, 0, HIGHGUID_TYPE_BATTLEGROUND);
            WoWGuid unkGuid;

            if (m_protocol.isCata())
            {
                packet.writeBit(bgGuid[3]);
                packet.writeBit(unkGuid[3]);
                packet.writeBit(playerGuid[3]);
                packet.writeBit(unkGuid[0]);
                packet.writeBit(bgGuid[6]);
                packet.writeBit(playerGuid[5]);
                packet.writeBit(playerGuid[6]);
                packet.writeBit(playerGuid[4]);

                packet.writeBit(playerGuid[2]);
                packet.writeBit(unkGuid[1]);
                packet.writeBit(bgGuid[1]);
                packet.writeBit(unkGuid[5]);
                packet.writeBit(unkGuid[6]);
                packet.writeBit(playerGuid[1]);
                packet.writeBit(bgGuid[7]);
                packet.writeBit(unkGuid[4]);

                packet.writeBit(bgGuid[2]);
                packet.writeBit(bgGuid[5]);
                packet.writeBit(unkGuid[7]);
                packet.writeBit(bgGuid[4]);
                packet.writeBit(bgGuid[0]);
                packet.writeBit(playerGuid[0]);
                packet.writeBit(unkGuid[2]);
                packet.writeBit(playerGuid[7]);

                packet.writeByteSeq(bgGuid[1]);

                packet << uint32_t(1);
                packet << queueSlot;

                packet.writeByteSeq(playerGuid[6]);
                packet.writeByteSeq(playerGuid[3]);
                packet.writeByteSeq(playerGuid[7]);
                packet.writeByteSeq(playerGuid[4]);
                packet.writeByteSeq(bgGuid[0]);
                packet.writeByteSeq(playerGuid[5]);
                packet.writeByteSeq(bgGuid[7]);
                packet.writeByteSeq(bgGuid[6]);
                packet.writeByteSeq(bgGuid[2]);
                packet.writeByteSeq(unkGuid[6]);
                packet.writeByteSeq(unkGuid[3]);
                packet.writeByteSeq(playerGuid[1]);
                packet.writeByteSeq(bgGuid[3]);
                packet.writeByteSeq(unkGuid[0]);
                packet.writeByteSeq(unkGuid[1]);
                packet.writeByteSeq(unkGuid[4]);
                packet.writeByteSeq(playerGuid[0]);
                packet.writeByteSeq(bgGuid[5]);
                packet.writeByteSeq(unkGuid[7]);
                packet.writeByteSeq(bgGuid[4]);
                packet.writeByteSeq(playerGuid[2]);

                packet << result;

                packet.writeByteSeq(unkGuid[2]);

                packet << joinTime;

                packet.writeByteSeq(unkGuid[5]);

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet << joinTime;
                packet << uint32_t(0);
                packet << queueSlot;
                packet << result;

                packet.writeBit(unkGuid[7]);
                packet.writeBit(bgGuid[2]);
                packet.writeBit(bgGuid[7]);
                packet.writeBit(unkGuid[5]);
                packet.writeBit(playerGuid[2]);
                packet.writeBit(bgGuid[6]);
                packet.writeBit(playerGuid[7]);
                packet.writeBit(playerGuid[3]);
                packet.writeBit(bgGuid[0]);
                packet.writeBit(bgGuid[3]);
                packet.writeBit(unkGuid[4]);
                packet.writeBit(playerGuid[1]);
                packet.writeBit(unkGuid[0]);
                packet.writeBit(playerGuid[0]);
                packet.writeBit(unkGuid[2]);
                packet.writeBit(bgGuid[4]);
                packet.writeBit(playerGuid[4]);
                packet.writeByteSeq(bgGuid[1]);
                packet.writeBit(unkGuid[3]);
                packet.writeBit(bgGuid[5]);
                packet.writeBit(unkGuid[1]);
                packet.writeBit(playerGuid[6]);
                packet.writeBit(playerGuid[5]);
                packet.writeBit(unkGuid[6]);

                packet.writeByteSeq(unkGuid[1]);
                packet.writeByteSeq(unkGuid[2]);
                packet.writeByteSeq(unkGuid[7]);
                packet.writeByteSeq(bgGuid[6]);
                packet.writeByteSeq(bgGuid[0]);
                packet.writeByteSeq(playerGuid[5]);
                packet.writeByteSeq(playerGuid[0]);
                packet.writeByteSeq(bgGuid[1]);
                packet.writeByteSeq(bgGuid[7]);
                packet.writeByteSeq(playerGuid[6]);
                packet.writeByteSeq(unkGuid[0]);
                packet.writeByteSeq(bgGuid[5]);
                packet.writeByteSeq(unkGuid[6]);
                packet.writeByteSeq(playerGuid[1]);
                packet.writeByteSeq(bgGuid[2]);
                packet.writeByteSeq(playerGuid[7]);
                packet.writeByteSeq(playerGuid[2]);
                packet.writeByteSeq(playerGuid[3]);
                packet.writeByteSeq(unkGuid[5]);
                packet.writeByteSeq(playerGuid[4]);
                packet.writeByteSeq(unkGuid[3]);
                packet.writeByteSeq(bgGuid[3]);
                packet.writeByteSeq(unkGuid[4]);
                packet.writeByteSeq(bgGuid[4]);

                return true;
            }

            // Classic - WotLK report queue errors with SMSG_GROUP_JOINED_BATTLEGROUND
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
