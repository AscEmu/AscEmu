/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgClearTradeItem : public ManagedPacket
    {
    public:
        uint8_t tradeSlot;

        CmsgClearTradeItem() : CmsgClearTradeItem(0)
        {
        }

        CmsgClearTradeItem(uint8_t tradeSlot) :
            ManagedPacket(CMSG_CLEAR_TRADE_ITEM, 0),
            tradeSlot(tradeSlot)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet >> tradeSlot;
                return true;
            }

            return false;
        }
    };
}
