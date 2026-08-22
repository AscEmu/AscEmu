/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <utility>
#include <vector>

struct QuestgiverInrangeStatus
{
    uint64_t rawGuid;
    int32_t status;
};

namespace AscEmu::Packets
{
    class SmsgQuestgiverStatusMultiple : public ManagedPacket
    {
    public:
        uint32_t inrangeCount;
        std::vector<QuestgiverInrangeStatus> questgiverSet;

        SmsgQuestgiverStatusMultiple() : SmsgQuestgiverStatusMultiple(0, {})
        {
        }

        SmsgQuestgiverStatusMultiple(uint32_t inrangeCount,
            std::vector<QuestgiverInrangeStatus> questgiverSet) :
            ManagedPacket(SMSG_QUESTGIVER_STATUS_MULTIPLE, 0),
            inrangeCount(inrangeCount),
            questgiverSet(std::move(questgiverSet))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            if (m_protocol.isMop())
                return 3 + (13 * inrangeCount);

            const size_t statusSize = m_protocol.expansion < WoW::Expansion::_Cata ? 1 : 4;
            return 4 + ((8 + statusSize) * inrangeCount);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBits(inrangeCount, 21);

                for (const auto& questGiver : questgiverSet)
                {
                    WoWGuid guid(questGiver.rawGuid);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[6]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[2]);
                }

                packet.flushBits();

                for (const auto& questGiver : questgiverSet)
                {
                    WoWGuid guid(questGiver.rawGuid);
                    packet.writeByteSeq(guid[6]);
                    packet.writeByteSeq(guid[2]);
                    packet.writeByteSeq(guid[7]);
                    packet.writeByteSeq(guid[5]);
                    packet.writeByteSeq(guid[4]);
                    packet << static_cast<uint32_t>(questGiver.status);
                    packet.writeByteSeq(guid[1]);
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[0]);
                }

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << inrangeCount;

                for (const auto& questGiver : questgiverSet)
                {
                    packet << questGiver.rawGuid;

                    if (m_protocol.expansion < WoW::Expansion::_Cata)
                        packet << static_cast<uint8_t>(questGiver.status);
                    else
                        packet << questGiver.status;
                }

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
