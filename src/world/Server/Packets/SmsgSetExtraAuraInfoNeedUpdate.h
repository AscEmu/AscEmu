/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSetExtraAuraInfoNeedUpdate : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t visualSlot;
        uint32_t spellId;
        uint32_t maxDuration;
        uint32_t timeLeft;

        SmsgSetExtraAuraInfoNeedUpdate() : SmsgSetExtraAuraInfoNeedUpdate(WoWGuid(), 0, 0, 0, 0)
        {
        }

        SmsgSetExtraAuraInfoNeedUpdate(WoWGuid guid, uint8_t visualSlot, uint32_t spellId, uint32_t maxDuration, uint32_t timeLeft) :
            ManagedPacket(SMSG_SET_EXTRA_AURA_INFO_NEED_UPDATE, 21),
            guid(guid), visualSlot(visualSlot), spellId(spellId), maxDuration(maxDuration), timeLeft(timeLeft)
        {
        }

    protected:
        size_t expectedSize() const override { return 21; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (!m_protocol.isMop())
            {
                packet << guid;
                packet << uint8_t(visualSlot);
                packet << uint32_t(spellId);
                packet << uint32_t(maxDuration);
                packet << uint32_t(timeLeft);
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
