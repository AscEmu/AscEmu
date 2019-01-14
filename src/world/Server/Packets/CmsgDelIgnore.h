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
    class CmsgDelIgnore : public ManagedPacket
    {
    public:
        uint64_t guid;

        CmsgDelIgnore() : CmsgDelIgnore(0)
        {
        }

        CmsgDelIgnore(uint64_t guid) :
            ManagedPacket(CMSG_DEL_IGNORE, 4),
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
