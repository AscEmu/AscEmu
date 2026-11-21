/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Server/CharacterErrors.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCheckCharacterNameAvailabilityResult : public ManagedPacket
    {
    public:
        uint32_t sequenceIndex;
        CharacterErrorCodes errorCode;

        SmsgCheckCharacterNameAvailabilityResult(uint32_t sequenceIndex, CharacterErrorCodes errorCode) :
            ManagedPacket(SMSG_CHECK_CHARACTER_NAME_AVAILABILITY_RESULT, 8),
            sequenceIndex(sequenceIndex),
            errorCode(errorCode)
        {
        }

    protected:
        size_t expectedSize() const override { return 4 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // request counter, result; a free name is answered with 0
            packet << sequenceIndex;
            packet << uint32_t(errorCode == E_CHAR_NAME_SUCCESS ? 0 : toMidnightCharacterErrorCode(static_cast<uint8_t>(errorCode)));
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
