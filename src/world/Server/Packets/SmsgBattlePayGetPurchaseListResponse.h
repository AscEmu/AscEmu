/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // purchases of the in game shop; the server has no shop, the list is empty
    class SmsgBattlePayGetPurchaseListResponse : public ManagedPacket
    {
    public:
        uint32_t result;

        SmsgBattlePayGetPurchaseListResponse() : SmsgBattlePayGetPurchaseListResponse(0)
        {
        }

        explicit SmsgBattlePayGetPurchaseListResponse(uint32_t result) :
            ManagedPacket(SMSG_BATTLE_PAY_GET_PURCHASE_LIST_RESPONSE, 4 + 4),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isLegion())
            {
                packet << result;
                packet << uint32_t(0);                  // purchases
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
