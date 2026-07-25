/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLogoutResponse : public ManagedPacket
    {
    public:
        bool logoutDenied;
        uint32_t failReason;
        uint8_t result;
        bool instant;

        SmsgLogoutResponse() : SmsgLogoutResponse(false)
        {
        }

        explicit SmsgLogoutResponse(uint8_t result) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 0),
            logoutDenied(false),
            failReason(0),
            result(result),
            instant(false)
        {
        }

        explicit SmsgLogoutResponse(bool logoutDenied) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 0),
            logoutDenied(logoutDenied),
            failReason(logoutDenied ? 1u : 0u),
            result(0),
            instant(false)
        {
        }

        SmsgLogoutResponse(uint32_t logoutResult, bool instantLogout) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 0),
            logoutDenied(logoutResult != 0),
            failReason(logoutResult),
            result(0),
            instant(instantLogout)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_protocol.isClassic() ? 1 : 5;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isClassic())
                packet << result;
            else
                packet << failReason << static_cast<uint8_t>(instant ? 1 : 0);

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isClassic())
                packet >> result;
            else
                packet >> failReason >> result;

            return true;
        }
    };
}
