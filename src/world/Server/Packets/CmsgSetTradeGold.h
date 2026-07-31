/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> tradeGoldAmount;
            }
            else // > WotLK
            {
                uint64_t tradeAmount = 0;
                packet >> tradeAmount;

                tradeGoldAmount = static_cast<uint32_t>(tradeAmount);
            }
            return true;
        }
    };
}
