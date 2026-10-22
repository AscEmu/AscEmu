/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLootMoneyNotify : public ManagedPacket
    {
    public:
        uint32_t money;
        uint8_t playersNear; // 0 = "Your share of the loot is...", 1 = "You loot..."

        SmsgLootMoneyNotify() : SmsgLootMoneyNotify(0, 1)
        {
        }

        SmsgLootMoneyNotify(uint32_t money, uint8_t playersNear) :
            ManagedPacket(SMSG_LOOT_MONEY_NOTIFY, 0),
            money(money), playersNear(playersNear)
        {
        }

    protected:
        size_t expectedSize() const override { return 5; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion())
            {
                // 6.x and 7.x: the money and the sole looter bit
                packet << uint32_t(money);
                packet.writeBit(playersNear != 0);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 8.x and later: 64 bit money, the money modifier (none here) and the sole looter bit
                packet << uint64_t(money);
                packet << uint64_t(0);
                packet.writeBit(playersNear != 0);
                packet.flushBits();
                return true;
            }

            if (m_protocol.isMop())
            {
                // playersNear here means "at most one nearby player" (matches the ctor default of 1
                // for a solo loot) - real Mop protocol just carries this as a single flag bit that
                // toggles between "Your share is..." and "You loot..." chat text, no separate byte.
                packet.writeBit(playersNear != 0);
                packet.flushBits();
                packet << money;

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << money << playersNear;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
