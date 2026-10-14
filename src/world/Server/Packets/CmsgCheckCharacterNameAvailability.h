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
        uint32_t sequenceIndex{0};
        std::string name;

        CmsgCheckCharacterNameAvailability() : ManagedPacket(CMSG_CHECK_CHARACTER_NAME_AVAILABILITY, 6) {}

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet >> sequenceIndex;
            const uint32_t nameLength = packet.readBits(6);
            if (nameLength == 0U || nameLength > 63U || packet.rpos() + nameLength > packet.size())
                return false;

            name = packet.readString(nameLength);
            return true;
        }
    };
}
