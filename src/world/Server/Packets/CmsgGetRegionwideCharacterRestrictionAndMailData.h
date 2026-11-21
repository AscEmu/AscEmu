/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class CmsgGetRegionwideCharacterRestrictionAndMailData : public ManagedPacket
    {
    public:
        std::vector<WoWGuid> characters;

        CmsgGetRegionwideCharacterRestrictionAndMailData() : ManagedPacket(CMSG_GET_REGIONWIDE_CHARACTER_RESTRICTION_AND_MAIL_DATA, 4)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override { return false; }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            // the characters of the list the client just received
            const uint32_t count = packet.read<uint32_t>();
            if (count > 64)
                return false;

            characters.reserve(count);
            for (uint32_t i = 0; i < count; ++i)
            {
                WoWGuid128 clientGuid;
                packet >> clientGuid;
                characters.push_back(WoWGuid::fromGuid128(clientGuid));
            }

            return !packet.hadReadFailure();
        }
    };
}
