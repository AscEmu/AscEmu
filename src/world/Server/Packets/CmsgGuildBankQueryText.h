/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankQueryText : public ManagedPacket
    {
    public:
        uint8_t tabId;

        CmsgGuildBankQueryText() : CmsgGuildBankQueryText(0)
        {}

        CmsgGuildBankQueryText(uint8_t tabId) :
            ManagedPacket(CMSG_GUILD_BANK_QUERY_TEXT, 1),
            tabId(tabId)
        {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet >> tabId;
                return true;
            }
            return false;
        }
    };
}
