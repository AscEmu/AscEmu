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
    class CmsgGuildLeader : public ManagedPacket
    {
    public:
        std::string name;

        CmsgGuildLeader() : CmsgGuildLeader("")
        {
        }

        CmsgGuildLeader(std::string name) :
            ManagedPacket(CMSG_GUILD_LEADER, 1),
            name(name)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name;
            return true;
        }
    };
}}
