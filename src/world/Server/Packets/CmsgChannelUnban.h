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
    class CmsgChannelUnban : public ManagedPacket
    {
    public:
        std::string name;
        std::string unbanName;

        CmsgChannelUnban() : CmsgChannelUnban("", "")
        {
        }

        CmsgChannelUnban(std::string name, std::string unbanName) :
            ManagedPacket(CMSG_CHANNEL_UNBAN, 0),
            name(name),
            unbanName(unbanName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> unbanName;
            return true;
        }
    };
}}
