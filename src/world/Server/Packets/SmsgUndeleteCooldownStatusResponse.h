/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/
#pragma once
#include "ManagedPacket.h"
#include <cstdint>
namespace AscEmu::Packets
{
    class SmsgUndeleteCooldownStatusResponse : public ManagedPacket
    {
    public:
        uint32_t maxCooldown;
        uint32_t currentCooldown;
        bool onCooldown;
        SmsgUndeleteCooldownStatusResponse(uint32_t maxCooldown = 0, uint32_t currentCooldown = 0, bool onCooldown = false) :
            ManagedPacket(SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, 9), maxCooldown(maxCooldown), currentCooldown(currentCooldown), onCooldown(onCooldown) {}
    protected:
        size_t expectedSize() const override { return 9; }
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isForever())
                return false;
            packet << maxCooldown << currentCooldown;
            packet.writeBit(onCooldown);
            packet.flushBits();
            return true;
        }
        bool internalDeserialise(WorldPacket&) override { return false; }
    };
}
