/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Server/CharacterErrors.h"
#include "shared/WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCharCreate : public ManagedPacket
    {
    public:
        CharacterErrorCodes result;
        uint32_t foreverRealmId{0};
        uint64_t foreverCharacterGuid{0};

        SmsgCharCreate() : SmsgCharCreate(E_CHAR_CREATE_ERROR) {}

        explicit SmsgCharCreate(CharacterErrorCodes result, uint32_t foreverRealmId = 0, uint64_t foreverCharacterGuid = 0) :
            ManagedPacket(SMSG_CHAR_CREATE, 1),
            result(result),
            foreverRealmId(foreverRealmId),
            foreverCharacterGuid(foreverCharacterGuid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? 16 : 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << uint32_t(result);
                const std::vector<uint8_t> packedGuid = WoWGuid::createModernPlayer(foreverRealmId, foreverCharacterGuid).packModern();
                packet.append(packedGuid.data(), packedGuid.size());
                return true;
            }

            packet << static_cast<uint8_t>(result);
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
