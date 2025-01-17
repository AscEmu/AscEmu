/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGroupJoinedBattleground : public ManagedPacket
    {
    public:
        int32_t errorOrBgType;

        SmsgGroupJoinedBattleground() : SmsgGroupJoinedBattleground(0)
        {
        }

        SmsgGroupJoinedBattleground(int32_t errorOrBgType) :
            ManagedPacket(SMSG_GROUP_JOINED_BATTLEGROUND, 4),
            errorOrBgType(errorOrBgType)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << errorOrBgType;    // < 0 = error; >= 0 bg type

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
