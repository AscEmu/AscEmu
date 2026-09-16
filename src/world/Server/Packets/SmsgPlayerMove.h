/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/MovementInfo.hpp"

namespace AscEmu::Packets
{
    // movement of a player for the clients around it: SMSG_PLAYER_MOVE since Cata, the movement opcode itself before
    class SmsgPlayerMove : public ManagedPacket
    {
    public:
        MovementInfo mi;
        bool withGuid = true;
        uint16_t legacyOpcode = 0;

        SmsgPlayerMove() : SmsgPlayerMove(MovementInfo())
        {
        }

        SmsgPlayerMove(MovementInfo mi, bool withGuid = true, uint16_t legacyOpcode = 0) :
            ManagedPacket(SMSG_PLAYER_MOVE, 0),
            mi(mi), withGuid(withGuid), legacyOpcode(legacyOpcode)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(mi); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Cata)
            {
                if (legacyOpcode == 0)
                    return false;

                packet.setOpcode(legacyOpcode);
            }

            mi.write(packet, m_protocol, withGuid);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
