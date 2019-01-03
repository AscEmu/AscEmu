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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> tradeSlot;
            return true;
        }
    };
}}
