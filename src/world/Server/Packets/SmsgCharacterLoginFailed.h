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
    class SmsgCharacterLoginFailed : public ManagedPacket
    {
    public:
        uint8_t result;

        SmsgCharacterLoginFailed() : SmsgCharacterLoginFailed(0)
        {
        }

        SmsgCharacterLoginFailed(uint8_t result) :
            ManagedPacket(SMSG_CHARACTER_LOGIN_FAILED, 1),
            result(result)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isBfA() || m_protocol.isShadowlands() || m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater())
            {
                // 11.x uses the result codes of 10.x, 12.x has its own
                packet << (m_protocol.isMidnight() ? toMidnightCharacterErrorCode(result) : m_protocol.isDragonflight() || m_protocol.isTheWarWithinOrLater() ? toDragonflightCharacterErrorCode(result) : m_protocol.isShadowlands() ? toShadowlandsCharacterErrorCode(result) : toBfACharacterErrorCode(result));
                return true;
            }

            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
