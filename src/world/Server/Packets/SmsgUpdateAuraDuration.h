/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

#ifdef AE_TBC
namespace AscEmu { namespace Packets
{
    class SmsgUpdateAuraDuration : public ManagedPacket
    {
    public:
        uint8_t slot;
        uint32_t duration;

        SmsgUpdateAuraDuration() : SmsgUpdateAuraDuration(0, 0)
        {
        }

        SmsgUpdateAuraDuration(uint8_t slot, uint32_t duration) :
            ManagedPacket(SMSG_UPDATE_AURA_DURATION, sizeof(uint8_t) + sizeof(uint32_t)),
            slot(slot),
            duration(duration)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << slot << duration;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> slot >> duration;
            return true;
        }
    };
}}
#endif
