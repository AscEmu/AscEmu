/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgRatedBgInfo : public ManagedPacket
    {
#if VERSION_STRING > WotLK
    public:
        uint32_t unk;

        SmsgRatedBgInfo() : SmsgRatedBgInfo(0)
        {
        }

        SmsgRatedBgInfo(uint32_t unk) :
            ManagedPacket(SMSG_RATED_BG_INFO, 72),
            unk(unk)
        {
        }

    protected:
        size_t expectedSize() const override { return 72; }

        bool internalSerialise(WorldPacket& packet) override
        {
            for (uint8_t i = 0; i < 18; ++i)
            {
                packet << uint32_t(unk);
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
#endif
    };
}
