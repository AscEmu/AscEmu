/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgNameQuery : public ManagedPacket
    {
    public:
        WoWGuid guid;

#if VERSION_STRING == Mop
        uint32_t realmID = 0;
        CmsgNameQuery() : ManagedPacket(CMSG_NAME_QUERY, 0) {}
#else
        CmsgNameQuery() : ManagedPacket(CMSG_NAME_QUERY, 8) {}
#endif

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint8_t mask[8] = { 0 };
#if VERSION_STRING == Mop
            packet >> realmID;

            for (int i = 0; i < 8; ++i)
                mask[i] = packet.readBit();

            packet.flushBits();

            uint8_t guidBytes[8] = {0};

            for (int i = 0; i < 8; ++i)
            {
                if (mask[i])
                    packet >> guidBytes[i];
            }

            uint64_t fullGuid = 0;
            memcpy(&fullGuid, guidBytes, 8);
            guid.init(fullGuid);
#else
            uint64_t fullGuid;
            packet >> fullGuid;
            guid.init(fullGuid);
#endif
            return true;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            packet << uint32_t(realmID);

            uint64_t fullGuid = guid.getRawGuid();
            uint8_t* bytes = reinterpret_cast<uint8_t*>(&fullGuid);

            for (int i = 0; i < 8; ++i)
                packet.writeBit(bytes[i] != 0);

            packet.flushBits();

            for (int i = 0; i < 8; ++i)
                if (bytes[i] != 0)
                    packet << bytes[i];
#else
            packet << guid.getRawGuid();
#endif
            return true;
        }
    };
}
