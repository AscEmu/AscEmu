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
    class CmsgChannelModerator : public ManagedPacket
    {
    public:
        std::string name;
        std::string modName;

        CmsgChannelModerator() : CmsgChannelModerator("", "")
        {
        }

        CmsgChannelModerator(std::string name, std::string modName) :
            ManagedPacket(CMSG_CHANNEL_MODERATOR, 0),
            name(name),
            modName(modName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> modName;
            return true;
        }
    };
}}
