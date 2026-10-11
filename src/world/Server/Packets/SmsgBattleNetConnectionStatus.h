/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // 11.x: the state of the Battle.net connection of the session, sent on the glue screen (1 = connected)
    class SmsgBattleNetConnectionStatus : public ManagedPacket
    {
    public:
        uint8_t state;
        bool suppressNotification;

        SmsgBattleNetConnectionStatus() : SmsgBattleNetConnectionStatus(1, false) {}

        SmsgBattleNetConnectionStatus(uint8_t state, bool suppressNotification) :
            ManagedPacket(SMSG_BATTLE_NET_CONNECTION_STATUS, 1),
            state(state),
            suppressNotification(suppressNotification)
        {}

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isTheWarWithin())
                return false;

            packet.writeBits(state, 2);
            packet.writeBit(suppressNotification);
            packet.flushBits();
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
