/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> guid >> tabId;
                return true;
            }

            return false;
        }
    };
}
