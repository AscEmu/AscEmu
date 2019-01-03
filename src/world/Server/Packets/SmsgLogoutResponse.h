/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLogoutResponse : public ManagedPacket
    {
    public:
        bool logout_denied;

        uint32_t fail_reason;
        uint8_t unk2;

        SmsgLogoutResponse() : SmsgLogoutResponse(false)
        {
        }

        SmsgLogoutResponse(bool logout_denied) :
            ManagedPacket(SMSG_LOGOUT_RESPONSE, 5),
            logout_denied(logout_denied),
            fail_reason(0),
            unk2(0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << (logout_denied ? uint32_t(1) : uint32_t(0)) << uint8_t(0);
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> fail_reason >> unk2;
            return true;
        }
    };
}}
