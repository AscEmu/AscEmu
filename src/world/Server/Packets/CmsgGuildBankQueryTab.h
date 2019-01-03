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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> tabId >> full;
            return true;
        }
    };
}}
