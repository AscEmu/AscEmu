/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildBankQueryText : public ManagedPacket
    {
#if VERSION_STRING >= Cata
    public:
        uint8_t tabId;

        CmsgGuildBankQueryText() : CmsgGuildBankQueryText(0)
        {
        }

        CmsgGuildBankQueryText(uint8_t tabId) :
            ManagedPacket(CMSG_GUILD_BANK_QUERY_TEXT, 1),
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
            packet >> tabId;
            return true;
        }
#endif
    };
}}
