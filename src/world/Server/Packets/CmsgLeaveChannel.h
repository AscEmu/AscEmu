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
    class CmsgLeaveChannel : public ManagedPacket
    {
    public:
        uint32_t code;
        std::string name;

        CmsgLeaveChannel() : CmsgLeaveChannel(0, "")
        {
        }

        CmsgLeaveChannel(uint32_t code, std::string name) :
            ManagedPacket(CMSG_LEAVE_CHANNEL, 0),
            code(code),
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
            packet >> code >> name;
            return true;
        }
    };
}}
