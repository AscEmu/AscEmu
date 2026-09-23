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
