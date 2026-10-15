/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgQuestupdateAddKill : public ManagedPacket
    {
    public:
        uint32_t questId;
        uint32_t mobEntry;
        uint32_t count;
        uint32_t tCount;
        WoWGuid guid;

        SmsgQuestupdateAddKill() : SmsgQuestupdateAddKill(0, 0, 0, 0, 0)
        {}

        SmsgQuestupdateAddKill(uint32_t questId, uint32_t mobEntry, uint32_t count, uint32_t tCount, WoWGuid guid) :
            ManagedPacket(SMSG_QUESTUPDATE_ADD_KILL, 4 + 4 + 4 + 4 + 8),
            questId(questId),
            mobEntry(mobEntry),
            count(count),
            tCount(tCount),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // victim, quest, object, count, required, objective type (11.x and later as 32 bit value)
                const int32_t signedEntry = static_cast<int32_t>(mobEntry);
                const uint8_t objectiveType = signedEntry < 0 ? 2 : 0;     // 2 = gameobject, 0 = creature
                const int32_t objectId = signedEntry < 0 ? -signedEntry : signedEntry;

                packet << guid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(questId);
                packet << int32_t(objectId);
                packet << uint16_t(count);
                packet << uint16_t(tCount);
                if (m_protocol.isTheWarWithinOrLater())
                    packet << uint32_t(objectiveType);
                else
                    packet << uint8_t(objectiveType);
                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << questId << mobEntry << count << tCount << guid;

                return true;
            }
            else if (m_protocol.isMop())
            {
                // objective type and object id must match the objective list sent in SMSG_QUEST_QUERY_RESPONSE
                const int32_t signedEntry = static_cast<int32_t>(mobEntry);
                const uint8_t objectiveType = signedEntry < 0 ? 2 : 0; // 2 = gameobject, 0 = npc
                const uint32_t objectId = signedEntry < 0 ? static_cast<uint32_t>(-signedEntry) : mobEntry;

                packet << uint16_t(count);
                packet << uint8_t(objectiveType);
                packet << questId;
                packet << uint16_t(tCount);
                packet << objectId;

                packet.writeBit(guid[0]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.flushBits();

                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[6]);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
