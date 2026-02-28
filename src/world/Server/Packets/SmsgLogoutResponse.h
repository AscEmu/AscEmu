/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgLogoutResponse : public ManagedPacket
    {
    public:
        bool logout_denied;

        uint32_t fail_reason;
        uint8_t result;
        bool instant;

        SmsgLogoutResponse() : SmsgLogoutResponse(false)
        {
        }

#if VERSION_STRING == Classic
        SmsgLogoutResponse(uint8_t result) : ManagedPacket(SMSG_LOGOUT_RESPONSE, 1),
            result(result),
            instant(false)
        {}
#else
        SmsgLogoutResponse(bool logout_denied) : ManagedPacket(SMSG_LOGOUT_RESPONSE, 5),
            logout_denied(logout_denied),
            fail_reason(logout_denied ? 1u : 0u),
            result(0),
            instant(false)
        {}
#endif

#if VERSION_STRING == Mop
        // MoP : LogoutResult (0=ok, 1=combat, 2=duel/frozen, 3=falling), Instant (true = no countdown)
        SmsgLogoutResponse(uint32_t logoutResult, bool instantLogout) : ManagedPacket(SMSG_LOGOUT_RESPONSE, 5),
            logout_denied(logoutResult != 0),
            fail_reason(logoutResult),
            result(0),
            instant(instantLogout)
        {}
#endif

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Classic
            packet << result;
            return true;
#else
            packet << uint32_t(fail_reason) << uint8_t(instant ? 1 : 0);
            return true;
#endif
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Classic
            packet >> result;
            return true;
#else
            packet >> fail_reason >> result;
            return true;
#endif
        }
    };
}
