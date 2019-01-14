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
    class SmsgTrainerBuySucceeded : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t spellId;

        SmsgTrainerBuySucceeded() : SmsgTrainerBuySucceeded(0, 0)
        {
        }

        SmsgTrainerBuySucceeded(uint64_t guid, uint32_t spellId) :
            ManagedPacket(SMSG_TRAINER_BUY_SUCCEEDED, 12),
            guid(guid),
            spellId(spellId)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << spellId;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
