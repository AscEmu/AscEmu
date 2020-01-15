/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgSetTradeItem : public ManagedPacket
    {
    public:
        uint8_t tradeSlot;
        uint8_t sourceBag;
        uint8_t sourceSlot;

        CmsgSetTradeItem() : CmsgSetTradeItem(0, 0, 0)
        {
        }

        CmsgSetTradeItem(uint8_t tradeSlot, uint8_t sourceBag, uint8_t sourceSlot) :
            ManagedPacket(CMSG_SET_TRADE_ITEM, 3),
            tradeSlot(tradeSlot),
            sourceBag(sourceBag),
            sourceSlot(sourceSlot)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Cata
            packet >> tradeSlot >> sourceBag >> sourceSlot;
#else
            packet >> sourceSlot >> tradeSlot >> sourceBag;
#endif
            return true;
        }
    };
}
