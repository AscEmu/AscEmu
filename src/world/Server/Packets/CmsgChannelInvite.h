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
    class CmsgChannelInvite : public ManagedPacket
    {
    public:
        std::string name;
        std::string inviteName;

        CmsgChannelInvite() : CmsgChannelInvite("", "")
        {
        }

        CmsgChannelInvite(std::string name, std::string inviteName) :
            ManagedPacket(CMSG_CHANNEL_INVITE, 0),
            name(name),
            inviteName(inviteName)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> name >> inviteName;
            return true;
        }
    };
}}
