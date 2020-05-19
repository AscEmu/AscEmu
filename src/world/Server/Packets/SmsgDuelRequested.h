/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgDuelRequested : public ManagedPacket
    {
    public:
        uint64_t gameobjectGuid;
        uint64_t playerGuid;
        
        SmsgDuelRequested() : SmsgDuelRequested(0, 0)
        {
        }

        SmsgDuelRequested(uint64_t gameobjectGuid, uint64_t playerGuid) :
            ManagedPacket(SMSG_DUEL_REQUESTED, 16),
            gameobjectGuid(gameobjectGuid),
            playerGuid(playerGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return 16; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << gameobjectGuid << playerGuid;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
