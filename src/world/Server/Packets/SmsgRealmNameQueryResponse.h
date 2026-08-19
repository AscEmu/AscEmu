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
    class SmsgRealmNameQueryResponse : public ManagedPacket
    {
    public:
        uint32_t realmId = 0;
        std::string realmName;
        bool found = false;
        bool isLocalRealm = false;

        SmsgRealmNameQueryResponse() : ManagedPacket(SMSG_REALM_NAME_QUERY_RESPONSE, 0)
        {
        }

    protected:
        size_t expectedSize() const override { return 5 + (found ? (realmName.size() * 2 + 4) : 0); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
                return false;

            packet << static_cast<uint8_t>(!found);
            packet << realmId;

            if (found)
            {
                packet.writeBits(realmName.size(), 8);
                packet.writeBit(isLocalRealm);
                packet.writeBits(realmName.size(), 8);
                packet.flushBits();

                packet.writeString(realmName);
                packet.writeString(realmName);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
