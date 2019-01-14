/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgGuildBankQueryTextResult : public ManagedPacket
    {
#if VERSION_STRING >= Cata
    public:
        uint32_t tabId;
        std::string tabInfo;

        SmsgGuildBankQueryTextResult() : SmsgGuildBankQueryTextResult(0, "")
        {
        }

        SmsgGuildBankQueryTextResult(uint32_t tabId, std::string tabInfo) :
            ManagedPacket(SMSG_GUILD_BANK_QUERY_TEXT_RESULT, 0),
            tabId(tabId),
            tabInfo(tabInfo)
        {
        }

    protected:
        size_t expectedSize() const override { return 1 + tabInfo.size() + 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet.writeBits(tabInfo.length(), 14);
            packet << tabId;
            packet.WriteString(tabInfo);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}}
