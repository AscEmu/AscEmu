/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgServerTimeOffset : public ManagedPacket
    {
    public:
        explicit SmsgServerTimeOffset(uint64_t serverTime) : ManagedPacket(SMSG_SERVER_TIME_OFFSET, sizeof(uint64_t)), serverTime(serverTime) {}

    protected:
        size_t expectedSize() const override { return sizeof(uint64_t); }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;

            packet << serverTime;
            return true;
        }

        bool internalDeserialise(WorldPacket&) override { return false; }

    private:
        uint64_t serverTime;
    };
}
