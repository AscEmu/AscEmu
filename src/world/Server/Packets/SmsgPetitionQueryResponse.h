/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPetitionQueryResponse : public ManagedPacket
    {
    public:
        uint32_t charterId;
        uint64_t leaderGuid;
        std::string name;
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
}}
