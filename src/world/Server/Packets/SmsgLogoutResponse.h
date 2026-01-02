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

        SmsgLogoutResponse() : SmsgLogoutResponse(false)
        {
        }

#if VERSION_STRING == Classic
        SmsgLogoutResponse(uint8_t result) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 1),
            result(result)
#else
        SmsgLogoutResponse(bool logout_denied) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 5),
            logout_denied(logout_denied),
            fail_reason(0),
            result(0)
#endif
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Classic
            packet << result;
            return true;
#else
            packet << (logout_denied ? uint32_t(1) : uint32_t(0)) << uint8_t(0);
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
