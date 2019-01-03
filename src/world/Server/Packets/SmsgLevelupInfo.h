/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgLevelupInfo : public ManagedPacket
    {
    public:
        uint32_t level;
        uint32_t hp;
        uint32_t mana;

        uint32_t stat0;
        uint32_t stat1;
        uint32_t stat2;
        uint32_t stat3;
        uint32_t stat4;

        SmsgLevelupInfo() : SmsgLevelupInfo(0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLevelupInfo(uint32_t level, uint32_t hp, uint32_t mana, uint32_t stat0,
            uint32_t stat1, uint32_t stat2, uint32_t stat3, uint32_t stat4) :
            ManagedPacket(SMSG_LEVELUP_INFO, 0),
            level(level),
            hp(hp),
            mana(mana),
            stat0(stat0),
            stat1(stat1),
            stat2(stat2),
            stat3(stat3),
            stat4(stat4)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 4 + 4 + (4 * 6) + (4 * 5);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << level << hp << mana;

            for (uint8_t i = 0; i < 6; ++i)
                packet << uint32_t(0);

            packet << stat0 << stat1 << stat2 << stat3 << stat4;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
