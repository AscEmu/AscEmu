/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCrossedInebriationThreshold : public ManagedPacket
    {
    public:
        uint64_t playerGuid;
        uint32_t drunkState;
        uint32_t itemId;

        SmsgCrossedInebriationThreshold() : SmsgCrossedInebriationThreshold(0, 0, 0)
        {
        }

        SmsgCrossedInebriationThreshold(uint64_t playerGuid, uint32_t drunkState, uint32_t itemId) :
            ManagedPacket(SMSG_CROSSED_INEBRIATION_THRESHOLD, 0),
            playerGuid(playerGuid),
            drunkState(drunkState),
            itemId(itemId)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << playerGuid << drunkState << itemId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
