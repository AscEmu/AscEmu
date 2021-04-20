/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgCastFailed : public ManagedPacket
    {
    public:
        uint8_t multiCast;
        uint32_t spellId;
        uint8_t errorMsg;

        uint32_t extra1;
        uint32_t extra2;

        SmsgCastFailed() : SmsgCastFailed(0, 0, 0, 0, 0)
        {
        }

        SmsgCastFailed(uint8_t multiCast, uint32_t spellId, uint8_t errorMsg, uint32_t extra1, uint32_t extra2) :
            ManagedPacket(SMSG_CAST_FAILED, 0),
            multiCast(multiCast),
            spellId(spellId),
            errorMsg(errorMsg),
            extra1(extra1),
            extra2(extra2)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 1 + 4 + 1 + 4 + 4;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            packet << spellId << errorMsg << multiCast;
            if (extra1 || extra2)
                packet << extra1;

            if (extra2)
                packet << extra2;
#else
            packet << multiCast << spellId << errorMsg;

            if (extra1 || extra2)
                packet << extra1;

            if (extra2)
                packet << extra2;
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
