/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgRepairItem : public ManagedPacket
    {
    public:

        WoWGuid creatureGuid;
        uint64_t itemGuid;
        bool isInGuild;

        CmsgRepairItem() : CmsgRepairItem(0, 0, false)
        {
        }

        CmsgRepairItem(uint64_t creatureGuid, uint64_t itemGuid, bool isInGuild) :
            ManagedPacket(CMSG_REPAIR_ITEM, 17),
            creatureGuid(creatureGuid),
            itemGuid(itemGuid),
            isInGuild(isInGuild)
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
            packet >> unpackedGuid >> itemGuid >> isInGuild;
            creatureGuid.Init(unpackedGuid);

            return true;
        }
    };
}}
