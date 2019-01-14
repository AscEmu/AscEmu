/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            packet.read_skip<uint32_t>();
            packet.read_skip<uint64_t>();
            packet >> name;
            packet.read_skip<std::string>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint16_t>();
            packet.read_skip<uint32_t>();
            packet.read_skip<uint32_t>();
            packet >> signerCount;
            for (uint8_t s = 0; s < 10; ++s)
                packet.read_skip<std::string>();

            packet >> arenaIndex;
            packet.read_skip<uint32_t>();

            creatureGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
