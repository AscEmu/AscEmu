/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgStandstateUpdate : public ManagedPacket
    {
    public:
        uint8_t state;

        SmsgStandstateUpdate() : SmsgStandstateUpdate(0)
        {
        }

        SmsgStandstateUpdate(uint8_t state) :
            ManagedPacket(SMSG_STANDSTATE_UPDATE, 1),
            state(state)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << state;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}}
