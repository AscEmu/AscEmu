/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgMoveUnsetCanFly : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t unk;

        SmsgMoveUnsetCanFly() : SmsgMoveUnsetCanFly(0)
        {
        }

        SmsgMoveUnsetCanFly(uint64_t guid) :
            ManagedPacket(SMSG_MOVE_UNSET_CAN_FLY, 0),
            guid(guid),
            unk(5)
        {
        }

    protected:

        bool internalSerialise(WorldPacket& packet) override
        {
            // Unk constant
            packet << guid << unk;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> guid >> unk;
            return true;
        }
    };
}}
