/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellHealLog : public ManagedPacket
    {
    public:
        WoWGuid targetGuid;
        WoWGuid casterGuid;
        uint32_t spellId;
        uint32_t healed;
        uint32_t overHealed;
        uint32_t absorb;
        uint8_t isCritical;
        uint8_t unused;

        SmsgSpellHealLog() : SmsgSpellHealLog(WoWGuid(), WoWGuid(), 0, 0, 0, 0, 0)
        {
        }

        SmsgSpellHealLog(WoWGuid targetGuid, WoWGuid casterGuid, uint32_t spellId, uint32_t healed, uint32_t overHealed, uint32_t absorb, uint8_t isCritical) :
            ManagedPacket(SMSG_SPELLHEALLOG, 8 + 8 + 4 + 4 + 4 + 4 + 1 + 1),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            spellId(spellId),
            healed(healed),
            overHealed(overHealed),
            absorb(absorb),
            isCritical(isCritical),
            unused(0)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << casterGuid << spellId << healed;

            if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << overHealed << absorb;
            }

            packet << isCritical;

            if (m_protocol.expansion > WoW::Expansion::_Classic)
            {
                packet << unused;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
