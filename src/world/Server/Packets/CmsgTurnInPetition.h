/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgTurnInPetition : public ManagedPacket
    {
    public:
        uint64_t itemGuid;

        //arena fields
        uint32_t iconColor;
        uint32_t icon;
        uint32_t borderColor;
        uint32_t border;
        uint32_t background;

        CmsgTurnInPetition() : CmsgTurnInPetition(0)
        {
        }

        CmsgTurnInPetition(uint64_t itemGuid) :
            ManagedPacket(CMSG_TURN_IN_PETITION, 8),
            itemGuid(itemGuid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> itemGuid;
            if (packet.size() >= 28)
                packet >> iconColor >> icon >> borderColor >> border >> background;
            return true;
        }
    };
}}
