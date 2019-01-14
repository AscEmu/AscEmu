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
    class CmsgPushquesttoparty : public ManagedPacket
    {
    public:
        uint32_t questId;

        CmsgPushquesttoparty() : CmsgPushquesttoparty(0)
        {
        }

        CmsgPushquesttoparty(uint32_t questId) :
            ManagedPacket(CMSG_PUSHQUESTTOPARTY, 4),
            questId(questId)
        {
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> questId;
            return true;
        }
    };
}}
