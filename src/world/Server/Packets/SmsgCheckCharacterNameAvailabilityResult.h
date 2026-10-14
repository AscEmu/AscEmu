/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgCheckCharacterNameAvailabilityResult : public ManagedPacket
    {
    public:
        uint32_t sequenceIndex{0};
        uint32_t result{0};

        SmsgCheckCharacterNameAvailabilityResult(uint32_t sequenceIndex = 0, uint32_t result = 0) :
            ManagedPacket(SMSG_CHECK_CHARACTER_NAME_AVAILABILITY_RESULT, 8),
            sequenceIndex(sequenceIndex),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << sequenceIndex << result;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
