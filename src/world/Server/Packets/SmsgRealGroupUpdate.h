/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgRealGroupUpdate : public ManagedPacket
    {
    public:
        uint8_t groupType;
        uint32_t membersCount;

        SmsgRealGroupUpdate() : SmsgRealGroupUpdate(0, 0)
        {
        }

        SmsgRealGroupUpdate(uint8_t groupType, uint32_t membersCount) :
            ManagedPacket(SMSG_REAL_GROUP_UPDATE, 13),
            groupType(groupType),
            membersCount(membersCount)
        {
        }

    protected:
        size_t expectedSize() const override { return 13; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                packet << groupType;
                packet << membersCount;
                packet << uint64_t(0);    // unk

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
