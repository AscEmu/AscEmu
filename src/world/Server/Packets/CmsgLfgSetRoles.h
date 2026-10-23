/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "LfgPacketHelpers.h"
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
            if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
            {
                // 10.x+: party index bit, roles byte, party index
                const bool hasPartyIndex = packet.readBit();
                uint8_t rolesByte = 0;
                packet >> rolesByte;
                roles = rolesByte;
                if (hasPartyIndex)
                    packet.readSkip<uint8_t>();
                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                packet >> roles;
                packet.readSkip<uint8_t>();                     // party index
                return true;
            }
            if (m_protocol.isMop())
            {
                packet >> roles;
                packet.readSkip<uint8_t>();

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                uint8_t rolesByte = 0;
                packet >> rolesByte;
                roles = rolesByte;

                return true;
            }

            return false;
        }
    };
}
