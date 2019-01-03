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
    class CmsgChannelUnmute : public ManagedPacket
    {
    public:
        std::string name;
        std::string unmuteName;

        CmsgChannelUnmute() : CmsgChannelUnmute("", "")
        {
        }

        CmsgChannelUnmute(std::string name, std::string unmuteName) :
            ManagedPacket(CMSG_CHANNEL_UNMUTE, 0),
            name(name),
            unmuteName(unmuteName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> unmuteName;
            return true;
        }
    };
}}
