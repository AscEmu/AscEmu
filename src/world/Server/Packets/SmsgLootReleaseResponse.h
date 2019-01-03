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
    class SmsgLootReleaseResponse : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t response;

        SmsgLootReleaseResponse() : SmsgLootReleaseResponse(0, 0)
        {
        }

        SmsgLootReleaseResponse(uint64_t guid, uint8_t response) :
            ManagedPacket(SMSG_LOOT_RELEASE_RESPONSE, 0),
            guid(guid),
            response(response)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << response;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
