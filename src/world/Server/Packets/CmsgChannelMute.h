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
    class CmsgChannelMute : public ManagedPacket
    {
    public:
        std::string name;
        std::string muteName;

        CmsgChannelMute() : CmsgChannelMute("", "")
        {
        }

        CmsgChannelMute(std::string name, std::string muteName) :
            ManagedPacket(CMSG_CHANNEL_MUTE, 0),
            name(name),
            muteName(muteName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> muteName;
            return true;
        }
    };
}}
