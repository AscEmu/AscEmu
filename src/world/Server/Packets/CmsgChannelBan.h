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
    class CmsgChannelBan : public ManagedPacket
    {
    public:
        std::string name;
        std::string banName;

        CmsgChannelBan() : CmsgChannelBan("", "")
        {
        }

        CmsgChannelBan(std::string name, std::string banName) :
            ManagedPacket(CMSG_CHANNEL_BAN, 0),
            name(name),
            banName(banName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> banName;
            return true;
        }
    };
}}
