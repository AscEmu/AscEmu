/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <string>

namespace AscEmu::Packets
{
    class CmsgCheckCharacterNameAvailability : public ManagedPacket
    {
    public:
        uint32_t sequenceIndex = 0;
        std::string name;
        std::string surname;

        CmsgCheckCharacterNameAvailability() : ManagedPacket(CMSG_CHECK_CHARACTER_NAME_AVAILABILITY, 6)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // request counter; name length, three flag bits, surname length; name, surname
            packet >> sequenceIndex;
            const uint32_t nameLength = packet.readBits(6);
            packet.readBits(3);
            const uint32_t surnameLength = packet.readBits(6);
            packet.resetBitPos();

            name = packet.readString(nameLength);
            if (surnameLength != 0)
                surname = packet.readString(surnameLength);

            return !packet.hadReadFailure();
        }
    };
}
