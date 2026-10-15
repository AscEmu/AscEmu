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
    class SmsgPetitionQueryResponse : public ManagedPacket
    {
    public:
        uint32_t charterId;
        uint64_t leaderGuid;
        utf8_string name;
        uint32_t type;
        uint32_t slots;

        SmsgPetitionQueryResponse() : SmsgPetitionQueryResponse(0, 0, "", 0, 0)
        {
        }

        SmsgPetitionQueryResponse(uint32_t charterId, uint64_t leaderGuid, std::string name, uint32_t type, uint32_t slots) :
            ManagedPacket(SMSG_PETITION_QUERY_RESPONSE, 0),
            charterId(charterId),
            leaderGuid(leaderGuid),
            name(name),
            type(type),
            slots(slots)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 8 + name.size() + 1 + 1 + 60; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // the petition with its signature limits, level range and texts; 11.x and later send the static type
                // as byte and longer title and choice texts
                const bool isGuild = type == CHARTER_TYPE_GUILD;
                const bool longTexts = m_protocol.isTheWarWithinOrLater();

                packet << uint32_t(charterId);
                packet.writeBit(true);                                  // allow
                packet.flushBits();

                packet << int32_t(charterId);
                packet << WoWGuid(leaderGuid).toGuid128(m_protocol.realmId, 0);
                packet << int32_t(isGuild ? 9 : slots);                 // minimum signatures
                packet << int32_t(isGuild ? 9 : slots);                 // maximum signatures
                packet << int32_t(0);                                   // deadline
                packet << int32_t(0);                                   // issue date
                packet << int32_t(0);                                   // allowed guild
                packet << int32_t(0);                                   // allowed classes
                packet << int32_t(0);                                   // allowed races
                packet << int16_t(0);                                   // allowed gender
                packet << int32_t(isGuild ? 1 : 80);                    // minimum level
                packet << int32_t(80);                                  // maximum level
                packet << int32_t(0);                                   // choices
                if (longTexts)
                    packet << int8_t(isGuild ? 0 : 1);                  // static type
                else
                    packet << int32_t(isGuild ? 0 : 1);
                packet << uint32_t(0);                                  // muid
                packet.writeBits(static_cast<uint32_t>(name.length()), longTexts ? 8 : 7);
                packet.writeBits(0, 12);                                // body text
                for (uint8_t i = 0; i < 10; ++i)
                    packet.writeBits(0, longTexts ? 7 : 6);             // choice texts
                packet.flushBits();
                packet.writeString(name);
                return true;
            }

            packet << charterId << leaderGuid << name << uint8_t(0);
            if (type == CHARTER_TYPE_GUILD)
                packet << uint32_t(9) << uint32_t(9);
            else
                packet << uint32_t(slots) << uint32_t(slots);

            packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint16_t(0);

            if (type == CHARTER_TYPE_GUILD)
                packet << uint32_t(1) << uint32_t(80);
            else
                packet << uint32_t(80) << uint32_t(80);

            packet << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint32_t(0) << uint16_t(0);

            if (type == CHARTER_TYPE_GUILD)
                packet << uint32_t(0);
            else
                packet << uint32_t(1);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
