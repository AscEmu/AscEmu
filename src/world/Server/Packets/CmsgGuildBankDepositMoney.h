/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankDepositMoney : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t money;

        uint64_t money64 = 0;   // Cata

        CmsgGuildBankDepositMoney() : CmsgGuildBankDepositMoney(0, 0)
        {
        }

        CmsgGuildBankDepositMoney(uint64_t guid, uint32_t money) :
            ManagedPacket(CMSG_GUILD_BANK_DEPOSIT_MONEY, 12),
            guid(guid),
            money(money)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> guid;
                packet >> money;
            }
            else // >=Cata
            {
                packet >> guid;
                packet >> money64;
                money = static_cast<uint32_t>(money64);
            }
            return true;
        }
    };
}
