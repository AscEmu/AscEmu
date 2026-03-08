/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu::Packets
{
    class CmsgLfgSetRoles : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
#if VERSION_STRING < Mop
        using RolesType = uint8_t;
        static constexpr uint16_t PacketSize = 1;
#else
        using RolesType = uint32_t;
        static constexpr uint16_t PacketSize = 5;
#endif
        RolesType roles;

        CmsgLfgSetRoles() : CmsgLfgSetRoles(0)
        {
        }

        CmsgLfgSetRoles(RolesType roles) :
            ManagedPacket(CMSG_LFG_SET_ROLES, PacketSize),
            roles(roles)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> roles;
#if VERSION_STRING >= Mop
            packet.read_skip<uint8_t>();
#endif
            return true;
        }
#endif
    };
}
