/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCorpseReclaimDelay : public ManagedPacket
    {
    public:
        uint32_t reclaimTime;

        SmsgCorpseReclaimDelay() : SmsgCorpseReclaimDelay(0)
        {
        }

        SmsgCorpseReclaimDelay(uint32_t reclaimTime) :
            ManagedPacket(SMSG_CORPSE_RECLAIM_DELAY, 0),
            reclaimTime(reclaimTime)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << reclaimTime;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
