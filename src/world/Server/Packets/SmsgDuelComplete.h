/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgDuelComplete : public ManagedPacket
    {
    public:
        uint8_t isCompleted;

        SmsgDuelComplete() : SmsgDuelComplete(0)
        {
        }

        SmsgDuelComplete(uint8_t isCompleted) :
            ManagedPacket(SMSG_DUEL_COMPLETE, 1),
            isCompleted(isCompleted)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << isCompleted;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
