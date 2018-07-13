/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgGuildBankBuyTab : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgGuildBankBuyTab() : CmsgGuildBankBuyTab(0)
        {
        }

        CmsgGuildBankBuyTab(uint64_t guid) :
            ManagedPacket(CMSG_GUILD_BANK_BUY_TAB, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
    };
}}
