/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgLootMasterGive : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;
        WoWGuid playerGuid;
        uint8_t slot;

        CmsgLootMasterGive() : CmsgLootMasterGive(0, 0, 0)
        {
        }

        CmsgLootMasterGive(uint64_t creatureGuid, uint64_t playerGuid, uint8_t slot) :
            ManagedPacket(CMSG_LOOT_MASTER_GIVE, 0),
            creatureGuid(creatureGuid),
            playerGuid(playerGuid),
            slot(slot)
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
            uint64_t unpackedPlayerGuid;

            packet >> unpackedCreatureGuid >> slot >> unpackedPlayerGuid;

            creatureGuid = WoWGuid(unpackedCreatureGuid);
            playerGuid = WoWGuid(unpackedPlayerGuid);

            return true;
        }
    };
}}
