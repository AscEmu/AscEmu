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
    class CmsgSetGuildBankText : public ManagedPacket
    {
    public:
#if VERSION_STRING < Cata
        uint8_t tabId;
#else
        uint32_t tabId;
#endif
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> tabId;
#if VERSION_STRING < Cata
            packet >> text;
#else
            const uint32_t textLen = packet.readBits(14);
            text = packet.ReadString(textLen);
#endif
            return true;
        }
    };
}}
