/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgTriggerMovie : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint32_t movieId;

        SmsgTriggerMovie() : SmsgTriggerMovie(0)
        {
        }

        SmsgTriggerMovie(uint32_t movieId) :
            ManagedPacket(SMSG_TRIGGER_MOVIE, 4),
            movieId(movieId)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << movieId;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
