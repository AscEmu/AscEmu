/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgGroupList : public ManagedPacket
    {
    public:
        bool sendEmptyList;

        SmsgGroupList(bool sendEmptyList = true) :
            ManagedPacket(SMSG_GROUP_LIST, 28),
            sendEmptyList(sendEmptyList)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (sendEmptyList)
            {
#if VERSION_STRING <= TBC
                packet << uint64_t(0) << uint64_t(0) << uint64_t(0);
#endif

#if VERSION_STRING > TBC
                packet << uint8_t(0x10) << uint8_t(0) << uint8_t(0) << uint8_t(0);
                packet << uint64_t(0) << uint32_t(0) << uint32_t(0) << uint64_t(0);
#endif
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
