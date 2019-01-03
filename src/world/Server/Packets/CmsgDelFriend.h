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
    class CmsgDelFriend : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgDelFriend() : CmsgDelFriend(0)
        {
        }

        CmsgDelFriend(uint64_t guid) :
            ManagedPacket(CMSG_DEL_FRIEND, 8),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid;
            return true;
        }
    };
}}
