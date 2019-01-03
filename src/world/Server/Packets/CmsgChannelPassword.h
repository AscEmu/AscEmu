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
    class CmsgChannelPassword : public ManagedPacket
    {
    public:
        std::string name;
        std::string password;

        CmsgChannelPassword() : CmsgChannelPassword("", "")
        {
        }

        CmsgChannelPassword(std::string name, std::string password) :
            ManagedPacket(CMSG_CHANNEL_PASSWORD, 0),
            name(name),
            password(password)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> password;
            return true;
        }
    };
}}
