/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include "ManagedPacket.h"
#include "Spell/SpellCastTargets.hpp"

namespace AscEmu::Packets
{
    class CmsgPetCastSpell : public ManagedPacket
    {
    public:
        uint64_t petGuid;
        uint8_t castCount;
        uint32_t spellId;
        uint8_t castFlags;

        SpellCastTargets targets;

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        bool hasSrcLocation = false;    // since 184141
        bool hasDestLocation = false;   // since 184141

        CmsgPetCastSpell() : CmsgPetCastSpell(0, 0, 0, 0)
        {
        }

        CmsgPetCastSpell(uint64_t petGuid, uint8_t castCount, uint32_t spellId, uint8_t castFlags) :
            ManagedPacket(CMSG_PET_CAST_SPELL, 12),
            petGuid(petGuid),
            castCount(castCount),
            spellId(spellId),
            castFlags(castFlags)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
            packet >> petGuid >> castCount >> spellId >> castFlags;

            targets.read(packet);

            if (castFlags & 0x02)
            {
                hasAdditionalData = true;
                packet >> projectilePitch >> projectileSpeed >> hasMovementData;
            }
#else // Mop

#endif

            return true;
        }
    };
}
