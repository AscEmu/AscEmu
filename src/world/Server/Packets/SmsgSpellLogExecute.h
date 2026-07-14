/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgSpellLogExecute : public ManagedPacket
    {
    public:
        uint32_t spellId;
        WoWGuid casterGuid;
        uint32_t unk1;
        uint32_t spellVisual;
        uint32_t unk2;
        bool hasTarget;
        uint64_t targetGuid;
        uint32_t spellDamage;

        SmsgSpellLogExecute(uint32_t spellId, WoWGuid casterGuid, uint32_t unk1, uint32_t spellVisual,
            uint32_t unk2, bool hasTarget, uint64_t targetGuid, uint32_t spellDamage) :
            ManagedPacket(SMSG_SPELLLOGMISS, 4 + 8 + 1 + 4 + 8 + 1),
            spellId(spellId),
            casterGuid(casterGuid),
            unk1(unk1),
            spellVisual(spellVisual),
            unk2(unk2),
            hasTarget(hasTarget),
            targetGuid(targetGuid),
            spellDamage(spellDamage)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            packet << casterGuid << spellId << unk1 << spellVisual << unk2;

            if (hasTarget)
                packet << targetGuid;

            packet << spellDamage;

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
