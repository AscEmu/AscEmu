/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildBankWithdrawMoney : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t money;

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
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> money;
            return true;
        }
    };
}}
