/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgLfgSetRoles : public ManagedPacket
    {
    public:
        using RolesType = uint32_t;
        static constexpr uint16_t packetSize = 0;
        RolesType roles;

        CmsgLfgSetRoles() : CmsgLfgSetRoles(0)
        {
        }

        CmsgLfgSetRoles(RolesType roles) :
            ManagedPacket(CMSG_LFG_SET_ROLES, packetSize),
            roles(roles)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> roles;
            if (m_protocol.expansion >= WoW::Expansion::_Mop)
            {
                packet.readSkip<uint8_t>();
            }
            return true;
        }
    };
}
