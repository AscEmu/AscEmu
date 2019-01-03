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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> slot >> tabName >> tabIcon;
            return true;
        }
    };
}}
