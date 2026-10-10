/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    // cooldown of the character restore on the character list (6.x to 10.x clients)
    class SmsgUndeleteCooldownStatusResponse : public ManagedPacket
    {
    public:
        bool onCooldown;
        uint32_t maxCooldown;
        uint32_t currentCooldown;

        SmsgUndeleteCooldownStatusResponse() : SmsgUndeleteCooldownStatusResponse(false, 0, 0)
        {
        }

        SmsgUndeleteCooldownStatusResponse(bool onCooldown, uint32_t maxCooldown, uint32_t currentCooldown) :
            ManagedPacket(SMSG_UNDELETE_COOLDOWN_STATUS_RESPONSE, 1 + 4 + 4),
            onCooldown(onCooldown),
            maxCooldown(maxCooldown),
            currentCooldown(currentCooldown)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD && m_protocol.expansion <= WoW::Expansion::_Dragonflight)
            {
                packet.writeBit(onCooldown);
                packet.flushBits();
                packet << maxCooldown << currentCooldown;
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
