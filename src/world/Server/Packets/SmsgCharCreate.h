/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCharCreate : public ManagedPacket
    {
    public:
        CharacterErrorCodes errorCode;

        explicit SmsgCharCreate(CharacterErrorCodes code) :
            ManagedPacket(SMSG_CHAR_CREATE, 1),
            errorCode(code)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            auto rawErrorCode = static_cast<uint8_t>(errorCode);

            if (m_protocol.isTheWarWithinOrLater())
            {
                // 11.x: result as a 32 bit value (same result codes as 10.x), 12.x with its own codes; the created character
                packet << uint32_t(m_protocol.isMidnight() ? toMidnightCharacterErrorCode(rawErrorCode) : toDragonflightCharacterErrorCode(rawErrorCode));
                packet << WoWGuid128();
                return true;
            }

            if (m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight())
            {
                // result, the created character
                packet << (m_protocol.isDragonflight() ? toDragonflightCharacterErrorCode(rawErrorCode) : m_protocol.isShadowlands() ? toShadowlandsCharacterErrorCode(rawErrorCode) : toBfACharacterErrorCode(rawErrorCode));
                packet << WoWGuid128();
                return true;
            }

            if (m_protocol.getExpansion() == WoW::Expansion::_Classic)
            {
                if (rawErrorCode >= static_cast<uint8_t>(E_CHAR_CREATE_SUCCESS))
                    --rawErrorCode;
            }

            packet << rawErrorCode;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
