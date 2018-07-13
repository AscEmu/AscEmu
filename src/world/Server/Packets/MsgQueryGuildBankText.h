/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
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
            ManagedPacket(MSG_QUERY_GUILD_BANK_TEXT, 1 + tabInfo.size() + 1),
            tabId(tabId),
            tabInfo(tabInfo)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << tabId;
            if (tabInfo.empty())
                packet << uint8_t(0);
            else
                packet << tabInfo;

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> tabId;
            return true;
        }
    };
}}
