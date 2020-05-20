/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <utility>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgSpellDamageShield : public ManagedPacket
    {
    public:
        uint64_t victimGuid;
        uint64_t attackerGuid;
        uint32_t spellId;
        uint32_t damage;
        uint32_t schoolMask;

        SmsgSpellDamageShield() : SmsgSpellDamageShield(0, 0, 0, 0, 0)
        {
        }

        SmsgSpellDamageShield(uint64_t victimGuid, uint64_t attackerGuid, uint32_t spellId, uint32_t damage, uint32_t schoolMask) :
            ManagedPacket(SMSG_SPELLDAMAGESHIELD, 28),
            victimGuid(victimGuid),
            attackerGuid(attackerGuid),
            spellId(spellId),
            damage(damage),
            schoolMask(schoolMask)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << victimGuid << attackerGuid << spellId << damage << schoolMask;
#if VERSION_STRING >= Cata
            packet << uint32_t(0);  // resisted damage
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
