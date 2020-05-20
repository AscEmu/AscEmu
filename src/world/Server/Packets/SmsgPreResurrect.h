/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPreResurrect : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint64_t guid;
        
        SmsgPreResurrect() : SmsgPreResurrect(0)
        {
        }

        SmsgPreResurrect(uint64_t guid) :
            ManagedPacket(SMSG_PRE_RESURRECT, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet.appendPackGUID(guid);

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}
