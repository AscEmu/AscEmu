/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class MsgQueryGuildBankText : public ManagedPacket
    {
    public:
        uint8_t tabId;
        std::string tabInfo;

        MsgQueryGuildBankText() : MsgQueryGuildBankText(0, "")
        {
        }

        MsgQueryGuildBankText(uint8_t tabId, std::string tabInfo) :
            ManagedPacket(MSG_QUERY_GUILD_BANK_TEXT, 1),
            tabId(tabId),
            tabInfo(tabInfo)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.expansion < WoW::Expansion::_Cata ? 1 + tabInfo.size() + 1 : 0;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
            {
                packet.writeBits(tabInfo.length(), 14);
                packet << tabId;
                packet.writeString(tabInfo);
            }
            else
            {
                packet << tabId;
                if (tabInfo.empty())
                    packet << uint8_t(0);
                else
                    packet << tabInfo;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Cata)
                return false;

            packet >> tabId;
            return true;
        }
    };
}
