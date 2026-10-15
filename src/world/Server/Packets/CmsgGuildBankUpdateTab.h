/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGuildBankUpdateTab : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t slot = 0;
        std::string tabName;
        std::string tabIcon;

        CmsgGuildBankUpdateTab() : CmsgGuildBankUpdateTab(0)
        {
        }

        CmsgGuildBankUpdateTab(uint64_t guid) :
            ManagedPacket(CMSG_GUILD_BANK_UPDATE_TAB, 11),
            guid(guid)
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
                packet >> slot;

                packet.resetBitPos();
                const uint32_t nameLength = packet.readBits(7);
                const uint32_t iconLength = packet.readBits(9);
                tabName = packet.readString(nameLength);
                tabIcon = packet.readString(iconLength);
                return true;
            }

            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet >> guid >> slot >> tabName >> tabIcon;
                return true;
            }

            return false;
        }
    };
}
