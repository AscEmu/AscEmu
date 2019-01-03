/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgSetFactionAtWar : public ManagedPacket
    {
    public:
        uint32_t id;
        uint8_t state;

        CmsgSetFactionAtWar() : CmsgSetFactionAtWar(0, 0)
        {
        }

        CmsgSetFactionAtWar(uint32_t id, uint8_t state) :
            ManagedPacket(CMSG_SET_FACTION_ATWAR, 5),
            id(id),
            state(state)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> id >> state;
            return true;
        }
    };
}}
