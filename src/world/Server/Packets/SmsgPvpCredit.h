/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPvpCredit : public ManagedPacket
    {
    public:
        uint32_t honor;
        uint64_t victimGuid;
        uint32_t victimRank;

        SmsgPvpCredit() : SmsgPvpCredit(0, 0, 0)
        {
        }

        SmsgPvpCredit(uint32_t honor, uint64_t victimGuid, uint32_t victimRank) :
            ManagedPacket(SMSG_PVP_CREDIT, 0),
            honor(honor),
            victimGuid(victimGuid),
            victimRank(victimRank)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 8 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << honor << victimGuid << victimRank;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
