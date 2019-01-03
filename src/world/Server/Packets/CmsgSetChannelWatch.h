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
    class CmsgSetChannelWatch : public ManagedPacket
    {
    public:
        std::string name;

        CmsgSetChannelWatch() : CmsgSetChannelWatch("")
        {
        }

        CmsgSetChannelWatch(std::string name) :
            ManagedPacket(CMSG_SET_CHANNEL_WATCH, 0),
            name(name)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
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
