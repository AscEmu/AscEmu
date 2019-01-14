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
    class CmsgGuildBankBuyTab : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t tabId;

        CmsgGuildBankBuyTab() : CmsgGuildBankBuyTab(0, 0)
        {
        }

        CmsgGuildBankBuyTab(uint64_t guid, uint8_t tabId) :
            ManagedPacket(CMSG_GUILD_BANK_BUY_TAB, 9),
            guid(guid),
            tabId(tabId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> tabId;
            return true;
        }
    };
}}
