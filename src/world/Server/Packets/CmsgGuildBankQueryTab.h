/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankQueryTab : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t tabId;
        bool full;

        CmsgGuildBankQueryTab() : CmsgGuildBankQueryTab(0, 0, false)
        {
        }

        CmsgGuildBankQueryTab(uint64_t guid, uint8_t tabId, bool full) :
            ManagedPacket(CMSG_GUILD_BANK_QUERY_TAB, 10),
            guid(guid),
            tabId(tabId),
            full(full)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isWoD() || m_protocol.isLegion() || m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                WoWGuid128 banker;
                packet >> banker;
                guid = WoWGuid::fromGuid128(banker).getRawGuid();
                packet >> tabId;
                full = packet.readBit();
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> guid >> tabId >> full;
                return true;
            }

            return false;
        }
    };
}
