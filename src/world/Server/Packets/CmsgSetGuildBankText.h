/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSetGuildBankText : public ManagedPacket
    {
    public:
        uint32_t tabId;
        std::string text;

        CmsgSetGuildBankText() : CmsgSetGuildBankText(0, "")
        {
        }

        CmsgSetGuildBankText(uint8_t tabId, std::string text) :
            ManagedPacket(CMSG_SET_GUILD_BANK_TEXT, 2),
            tabId(tabId),
            text(text)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                uint8_t tabId8;
                packet >> tabId8;
                tabId = tabId8;

                packet >> text;

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Mop)
            {
                packet >> tabId;

                const uint32_t textLen = packet.readBits(14);
                text = packet.readString(textLen);

                return true;
            }

            return false;
        }
    };
}
