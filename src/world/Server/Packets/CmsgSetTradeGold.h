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
    class CmsgSetTradeGold : public ManagedPacket
    {
    public:
        uint32_t tradeGoldAmount;

        CmsgSetTradeGold() : CmsgSetTradeGold(0)
        {
        }

        CmsgSetTradeGold(uint32_t tradeGoldAmount) :
            ManagedPacket(CMSG_SET_TRADE_GOLD, 4),
            tradeGoldAmount(tradeGoldAmount)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> tradeGoldAmount;
            return true;
        }
    };
}}
