/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgPlayedTime : public ManagedPacket
    {
    public:
        uint32_t totalPlayedTime;
        uint32_t playedTimeOnLevel;
        uint8_t displayInUi;

        SmsgPlayedTime() : SmsgPlayedTime(0, 0, 0)
        {
        }

        SmsgPlayedTime(uint32_t totalPlayedTime, uint32_t playedTimeOnLevel, uint8_t displayInUi) :
            ManagedPacket(SMSG_PLAYED_TIME, 8),
            totalPlayedTime(totalPlayedTime),
            playedTimeOnLevel(playedTimeOnLevel),
            displayInUi(displayInUi)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << totalPlayedTime << playedTimeOnLevel;

#if VERSION_STRING > TBC
            packet << displayInUi;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
