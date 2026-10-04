/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Utilities/utf8String.hpp"

#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class SmsgPetNameQuery : public ManagedPacket
    {
    public:
        uint32_t petNumber;
        utf8_string name;
        uint32_t timeStamp;
        uint8_t unknown;

        // 6.x and 7.x clients ask and are answered with the guid of the pet
        WoWGuid petGuid;

        SmsgPetNameQuery() : SmsgPetNameQuery(0, "", 0, 0)
        {
        }

        SmsgPetNameQuery(uint32_t petNumber, std::string name, uint32_t timeStamp, uint8_t unknown) :
            ManagedPacket(SMSG_PET_NAME_QUERY_RESPONSE, 9 + name.size() + 1),
            petNumber(petNumber),
            name(name),
            timeStamp(timeStamp),
            unknown(unknown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion())
            {
                // pet, allow, name length, declined names, timestamp, name
                packet << petGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet.writeBit(true);
                packet.writeBits(static_cast<uint32_t>(name.length()), 8);
                packet.writeBit(false);
                for (uint8_t i = 0; i < 5; ++i)
                    packet.writeBits(0, 7);
                packet.flushBits();
                packet << timeStamp;
                packet.writeString(name);
                return true;
            }

            packet << petNumber << name << timeStamp << unknown;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
