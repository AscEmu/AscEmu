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
    class CmsgChannelSetOwner : public ManagedPacket
    {
    public:
        std::string name;
        std::string setName;

        CmsgChannelSetOwner() : CmsgChannelSetOwner("", "")
        {
        }

        CmsgChannelSetOwner(std::string name, std::string setName) :
            ManagedPacket(CMSG_CHANNEL_SET_OWNER, 0),
            name(name),
            setName(setName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> setName;
            return true;
        }
    };
}}
