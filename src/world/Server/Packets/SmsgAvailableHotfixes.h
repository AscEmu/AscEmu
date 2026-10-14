/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgAvailableHotfixes : public ManagedPacket
    {
    public:
        SmsgAvailableHotfixes(int32_t virtualRealmAddress, uint32_t hotfixCount) :
            ManagedPacket(SMSG_AVAILABLE_HOTFIXES, sizeof(int32_t) + sizeof(uint32_t)),
            virtualRealmAddress(virtualRealmAddress),
            hotfixCount(hotfixCount)
        {
        }

    protected:
        size_t expectedSize() const override { return sizeof(int32_t) + sizeof(uint32_t); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << virtualRealmAddress << hotfixCount;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }

    private:
        int32_t virtualRealmAddress;
        uint32_t hotfixCount;
    };
}
