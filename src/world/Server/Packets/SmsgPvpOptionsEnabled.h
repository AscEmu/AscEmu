/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPvpOptionsEnabled : public ManagedPacket
    {
#if VERSION_STRING > WotLK
    public:
        bool wargamesEnabled;
        bool ratedBgEnabled;
        bool ratedArenasEnabled;

        SmsgPvpOptionsEnabled() : SmsgPvpOptionsEnabled(true, true, true)
        {
        }

        SmsgPvpOptionsEnabled(bool wargamesEnabled, bool ratedBgEnabled, bool ratedArenasEnabled) :
            ManagedPacket(SMSG_PVP_OPTIONS_ENABLED, 1),
            wargamesEnabled(wargamesEnabled),
            ratedBgEnabled(ratedBgEnabled),
            ratedArenasEnabled(ratedArenasEnabled)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet.writeBit(1);
            packet.writeBit(wargamesEnabled);
            packet.writeBit(1);
            packet.writeBit(ratedBgEnabled);
            packet.writeBit(ratedArenasEnabled);

            packet.flushBits();

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}
