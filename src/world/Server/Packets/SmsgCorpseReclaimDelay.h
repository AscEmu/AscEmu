/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
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
            if (m_protocol.isMop())
            {
                packet.writeBit(reclaimTime == 0);
                packet.flushBits();

                if (reclaimTime != 0)
                    packet << reclaimTime;

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << reclaimTime;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
