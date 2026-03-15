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
    class CmsgCastSpell : public ManagedPacket
    {
    public:
        uint32_t spellId;
        uint8_t castCount;
        uint8_t castFlags;

        SpellCastTargets targets;

        uint32_t glyphSlot = 0;     // since 15595

        bool hasAdditionalData = false;

        float projectilePitch = 0.0f;
        float projectileSpeed = 0.0f;

        bool hasMovementData = false;

        CmsgCastSpell() : CmsgCastSpell(0, 0, 0)
        {
        }

        CmsgCastSpell(uint32_t _spellId, uint8_t _castCount, uint8_t _flags) :
            ManagedPacket(CMSG_CAST_SPELL, 0),
            spellId(_spellId),
            castCount(_castCount),
            castFlags(_flags)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING < Mop
#if VERSION_STRING <= TBC
            packet >> spellId >> castCount;
#elif VERSION_STRING == WotLK
            packet >> castCount >> spellId >> castFlags;
#elif VERSION_STRING == Cata
            packet >> castCount >> spellId >> glyphSlot >> castFlags;
#endif
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
