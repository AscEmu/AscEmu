/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
            ManagedPacket(CMSG_BUY_ITEM, 13),
            sourceGuid(sourceGuid),
            itemEntry(itemEntry),
            slot(slot),
            amount(amount)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t rawGuid = 0;

            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                packet >> rawGuid >> itemType >> itemEntry >> slot >> amount;
            else if (m_protocol.expansion == WoW::Expansion::_WotLK)
                packet >> rawGuid >> itemEntry >> slot >> amount;
            else if (m_protocol.expansion <= WoW::Expansion::_TBC)
                packet >> rawGuid >> itemEntry >> amount;

            sourceGuid.init(rawGuid);
            return true;
        }
    };
}
