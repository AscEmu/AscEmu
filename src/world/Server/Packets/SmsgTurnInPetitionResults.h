/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgTurnInPetitionResults : public ManagedPacket
    {
    public:
        uint32_t result = 0;

        SmsgTurnInPetitionResults() : SmsgTurnInPetitionResults(0)
        {
        }

        SmsgTurnInPetitionResults(uint32_t result) :
            ManagedPacket(SMSG_TURN_IN_PETITION_RESULTS, 4),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << uint32_t(result);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(result, 4);
                packet.flushBits();
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
