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
    class CmsgBuyItem : public ManagedPacket
    {
    public:
        WoWGuid sourceGuid;
        uint32_t itemEntry;
        int32_t slot;
        uint8_t amount;

        //cata specific
        uint8_t itemType = 0;

        CmsgBuyItem() : CmsgBuyItem(0, 0, 0, 0)
        {
        }

        CmsgBuyItem(uint64_t sourceGuid, uint32_t itemEntry, int32_t slot, uint8_t amount) :
            ManagedPacket(CMSG_BUY_ITEM, 14),
            sourceGuid(sourceGuid),
            itemEntry(itemEntry),
            slot(slot),
            amount(amount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t rawGuid;
#if VERSION_STRING >= Cata
            packet >> rawGuid >> itemType >> itemEntry >> slot >> amount;
#endif

#if VERSION_STRING == WotLK
            packet >> rawGuid >> itemEntry >> slot >> amount;
#endif

#if VERSION_STRING <= TBC
            packet >> rawGuid >> itemEntry >> slot >> amount;
#endif
            sourceGuid.Init(rawGuid);
            return true;
        }
    };
}}
