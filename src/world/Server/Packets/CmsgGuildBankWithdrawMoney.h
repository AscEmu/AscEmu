/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankWithdrawMoney : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t money;
        uint64_t money64;   // Cata

        CmsgGuildBankWithdrawMoney() : CmsgGuildBankWithdrawMoney(0, 0)
        {
        }

        CmsgGuildBankWithdrawMoney(uint64_t guid, uint32_t money) :
            ManagedPacket(CMSG_GUILD_BANK_WITHDRAW_MONEY, 12),
            guid(guid),
            money(money)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;

            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                packet >> money;
            }
            else // Cata and greater
            {
                packet >> money64;
                money = static_cast<uint32_t>(money);
            }
            return true;
        }
    };
}
