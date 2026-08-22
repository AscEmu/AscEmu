/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgDuelCountdown : public ManagedPacket
    {
    public:
        uint32_t timeInMs;

        SmsgDuelCountdown() : SmsgDuelCountdown(0)
        {
        }

        SmsgDuelCountdown(uint32_t timeInMs) :
            ManagedPacket(SMSG_DUEL_COUNTDOWN, 4),
            timeInMs(timeInMs)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_Classic)
            {
                packet << timeInMs;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
