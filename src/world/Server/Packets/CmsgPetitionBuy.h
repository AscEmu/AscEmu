/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetitionBuy : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;
        std::string name;
        uint32_t signerCount = 0;
        uint32_t arenaIndex = 0;

        CmsgPetitionBuy() : CmsgPetitionBuy("")
        {
        }

        CmsgPetitionBuy(std::string name) :
            ManagedPacket(CMSG_PETITION_BUY, 0),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            packet.readSkip<uint32_t>();
            packet.readSkip<uint64_t>();
            packet >> name;
            packet.readSkip<std::string>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint16_t>();
            packet.readSkip<uint32_t>();
            packet.readSkip<uint32_t>();
            packet >> signerCount;
            for (uint8_t s = 0; s < 10; ++s)
                packet.readSkip<std::string>();

            packet >> arenaIndex;
            packet.readSkip<uint32_t>();

            creatureGuid.init(unpackedGuid);
            return true;
        }
    };
}
