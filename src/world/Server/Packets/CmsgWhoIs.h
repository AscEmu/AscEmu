/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgWhoIs : public ManagedPacket
    {
    public:
        std::string characterName;

        CmsgWhoIs() : CmsgWhoIs("")
        {
        }

        CmsgWhoIs(std::string characterName) :
            ManagedPacket(CMSG_WHOIS, 1),
            characterName(characterName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> characterName;
            return true;
        }
    };
}
