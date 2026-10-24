/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgStartTimer : public ManagedPacket
    {
    public:
        int32_t timerType;
        uint32_t timeLeft;
        uint32_t totalTime;

        SmsgStartTimer() : SmsgStartTimer(0, 0, 0)
        {
        }

        SmsgStartTimer(int32_t timerType, uint32_t timeLeft, uint32_t totalTime) :
            ManagedPacket(SMSG_START_TIMER, 8 + 4 + 8 + 1),
            timerType(timerType),
            timeLeft(timeLeft),
            totalTime(totalTime)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Dragonflight)
            {
                // 10.x+: total, type, time left in seconds, no player guid
                packet << int64_t(totalTime);
                packet << int32_t(timerType);
                packet << int64_t(timeLeft);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            if (m_protocol.expansion == WoW::Expansion::_Shadowlands)
            {
                // 9.x: total and time left as 64 bit seconds, then the type
                packet << int64_t(totalTime);
                packet << int64_t(timeLeft);
                packet << int32_t(timerType);
                return true;
            }

            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                // 6.x - 8.x: time left, total, type
                packet << uint32_t(timeLeft);
                packet << uint32_t(totalTime);
                packet << uint32_t(timerType);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
