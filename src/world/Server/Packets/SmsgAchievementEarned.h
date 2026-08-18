/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAchievementEarned : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t achievementId;
        uint32_t timeBitField;
        time_t completionTime;

        SmsgAchievementEarned() : SmsgAchievementEarned(WoWGuid(), 0, 0, 0)
        {
        }

        SmsgAchievementEarned(WoWGuid guid, uint32_t achievementId, uint32_t timeBitField, time_t completionTime) :
            ManagedPacket(SMSG_ACHIEVEMENT_EARNED, 30),
            guid(guid), achievementId(achievementId), timeBitField(timeBitField), completionTime(completionTime)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 8 + 4 + 4 + 4 + 4 + 3; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[3]);
                packet.writeBit(0);            // not notify player
                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[5]);

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[6]);
                packet.appendPackedTime(completionTime);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[7]);
                packet << achievementId;
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet << uint32_t(1);         // realmId
                packet.writeByteSeq(guid[5]);
                packet << uint32_t(1);         // realmId
                packet.writeByteSeq(guid[2]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << guid;
                packet << achievementId;
                packet << timeBitField;
                packet << uint32_t(0);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
