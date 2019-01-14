/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class MsgPetitionDecline : public ManagedPacket
    {
    public:
        uint64_t itemGuid = 0;
        uint64_t playerGuid;

        MsgPetitionDecline() : MsgPetitionDecline(0)
        {
        }

        MsgPetitionDecline(uint64_t playerGuid) :
            ManagedPacket(MSG_PETITION_DECLINE, 8),
            playerGuid(playerGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;
            return true;
        }
    };
}}
