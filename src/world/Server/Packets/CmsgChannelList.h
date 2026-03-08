/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgChannelList : public ManagedPacket
    {
    public:
        std::string name;

        CmsgChannelList() : CmsgChannelList("")
        {
        }

        CmsgChannelList(std::string name) :
            ManagedPacket(CMSG_CHANNEL_LIST, 0),
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
#if VERSION_STRING <= Cata
            packet >> name;
#else
            const uint32_t nameLength = packet.readBits(7);
            name = packet.ReadString(nameLength);
#endif
            return true;
        }
    };
}
