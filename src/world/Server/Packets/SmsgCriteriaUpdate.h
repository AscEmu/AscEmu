/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCriteriaUpdate : public ManagedPacket
    {
    public:
        uint32_t criteriaId;
        uint32_t counter;
        WoWGuid guid;
        uint32_t secsBitField;
        time_t progressDate;

        SmsgCriteriaUpdate() : SmsgCriteriaUpdate(0, 0, WoWGuid(), 0, 0)
        {
        }

        SmsgCriteriaUpdate(uint32_t criteriaId, uint32_t counter, WoWGuid guid, uint32_t secsBitField, time_t progressDate) :
            ManagedPacket(SMSG_CRITERIA_UPDATE, 0),
            criteriaId(criteriaId), counter(counter), guid(guid), secsBitField(secsBitField), progressDate(progressDate)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 8 + 4 + 4 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[4]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[2]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[0]);
                packet.flushBits();

                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[2]);

                packet << criteriaId;
                packet << uint32_t(0); // criteria is not time-limited (time-limited criteria are not tracked)

                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[1]);

                packet.appendPackedTime(progressDate);
                packet.writeByteSeq(guid[4]);

                packet << uint32_t(0); // elapsed time since criteria was started (not tracked)
                packet << uint32_t(0);

                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[0]);

                packet << uint64_t(counter);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << criteriaId;

                packet.appendPackGuid(counter);

                packet << guid;
                packet << uint32_t(0);
                packet << secsBitField;
                packet << uint32_t(0);
                packet << uint32_t(0);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
