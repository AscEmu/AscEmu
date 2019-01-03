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
    class CmsgActivatetaxi : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t srcNode;
        uint32_t destNode;

        CmsgActivatetaxi() : CmsgActivatetaxi(0, 0, 0)
        {
        }

        CmsgActivatetaxi(uint64_t guid, uint32_t srcNode, uint32_t destNode) :
            ManagedPacket(CMSG_ACTIVATETAXI, 8 + 4 + 4),
            guid(guid),
            srcNode(srcNode),
            destNode(destNode)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> srcNode >> destNode;
            return true;
        }
    };
}}
