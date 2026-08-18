/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgClientcacheVersion : public ManagedPacket
    {
    public:
        uint32_t version;

        SmsgClientcacheVersion() : SmsgClientcacheVersion(0) {}

        explicit SmsgClientcacheVersion(uint32_t version) :
            ManagedPacket(SMSG_CLIENTCACHE_VERSION, 4),
            version(version)
        {}

    protected:
        size_t expectedSize() const override { return sizeof(version); }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << uint32_t(version);
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
