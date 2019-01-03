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
    class CmsgLootMethod : public ManagedPacket
    {
    public:
        uint32_t method;
        WoWGuid guid;
        uint32_t threshold;

        CmsgLootMethod() : CmsgLootMethod(0, 0, 0)
        {
        }

        CmsgLootMethod(uint32_t method, uint64_t guid, uint32_t threshold) :
            ManagedPacket(CMSG_LOOT_METHOD, 16),
            method(method),
            guid(guid),
            threshold(threshold)
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
            packet >> method >> unpackedGuid >> threshold;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
