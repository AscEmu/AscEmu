/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgLootRoll : public ManagedPacket
    {
    public:
        WoWGuid objectGuid;
        uint32_t slot;
        uint8_t choice;

        CmsgLootRoll() : CmsgLootRoll(0, 0, 0)
        {
        }

        CmsgLootRoll(uint64_t creatureGuid, uint32_t slot, uint8_t choice) :
            ManagedPacket(CMSG_LOOT_ROLL, 0),
            objectGuid(creatureGuid),
            slot(slot),
            choice(choice)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedCreatureGuid;

            packet >> unpackedCreatureGuid >> slot >> choice;

            objectGuid = WoWGuid(unpackedCreatureGuid);

            return true;
        }
    };
}}
