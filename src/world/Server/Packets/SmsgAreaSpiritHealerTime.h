/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgAreaSpiritHealerTime : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t restTime;

        SmsgAreaSpiritHealerTime() : SmsgAreaSpiritHealerTime(0, 0)
        {
        }

        SmsgAreaSpiritHealerTime(uint64_t guid, uint32_t restTime) :
            ManagedPacket(SMSG_AREA_SPIRIT_HEALER_TIME, 12),
            guid(guid),
            restTime(restTime)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << restTime;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
