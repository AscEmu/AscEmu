/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Server/CharacterErrors.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgCharDelete : public ManagedPacket
    {
    public:
        uint8_t result{0};
        CharacterErrorCodes foreverResult{E_CHAR_DELETE_FAILED};
        bool useForeverResult{false};

        SmsgCharDelete() : ManagedPacket(SMSG_CHAR_DELETE, 1) {}

        explicit SmsgCharDelete(uint8_t result) :
            ManagedPacket(SMSG_CHAR_DELETE, 1),
            result(result)
        {
        }

        explicit SmsgCharDelete(CharacterErrorCodes result) :
            ManagedPacket(SMSG_CHAR_DELETE, 1),
            result(static_cast<uint8_t>(result)),
            foreverResult(result),
            useForeverResult(true)
        {
        }

    protected:
        size_t expectedSize() const override { return m_protocol.isForever() ? 4 : 1; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << uint32_t(useForeverResult ? foreverResult : static_cast<CharacterErrorCodes>(result));
                return true;
            }

            packet << result;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
