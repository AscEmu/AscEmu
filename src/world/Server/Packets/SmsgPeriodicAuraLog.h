/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu::Packets
{
    class SmsgPeriodicAuraLog : public ManagedPacket
    {
    public:

        WoWGuid targetGuid;
        WoWGuid casterGuid;
        uint32_t spellId;
        uint32_t auraType;
        uint32_t amount;
        uint32_t overKill;
        uint32_t schoolMask;
        uint32_t absorbAmount;
        uint32_t resistedAmount;
        uint8_t isCritical;
        uint32_t miscValue;
        float gainMultiplier;

        SmsgPeriodicAuraLog() : SmsgPeriodicAuraLog(WoWGuid(), WoWGuid(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f)
        {
        }

        SmsgPeriodicAuraLog(WoWGuid targetGuid, WoWGuid casterGuid, uint32_t spellId, uint32_t auraType, uint32_t amount, uint32_t overKill, 
            uint32_t schoolMask, uint32_t absorbAmount, uint32_t resistedAmount, uint8_t isCritical, uint32_t miscValue, float gainMultiplier) :
            ManagedPacket(SMSG_PERIODICAURALOG, 30),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            spellId(spellId),
            auraType(auraType),
            amount(amount),
            overKill(overKill),
            schoolMask(schoolMask),
            absorbAmount(absorbAmount),
            resistedAmount(resistedAmount),
            isCritical(isCritical),
            miscValue(miscValue),
            gainMultiplier(gainMultiplier)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << targetGuid << casterGuid << spellId << uint32_t(1) << auraType;

            switch (auraType)
            {
            case 3:     //SPELL_AURA_PERIODIC_DAMAGE
            case 89:    //SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
#if VERSION_STRING > TBC
                    packet << amount << overKill << schoolMask << absorbAmount << resistedAmount << isCritical;
#else
                    packet << amount << schoolMask << absorbAmount << resistedAmount;
#endif
                    break;
            case 8:     //SPELL_AURA_PERIODIC_HEAL
            case 20:    //SPELL_AURA_MOD_TOTAL_HEALTH_REGEN_PCT
#if VERSION_STRING > TBC
                    packet << amount << overKill << absorbAmount << isCritical;
#else
                    packet << amount;
#endif
                    break;
            case 21:    //SPELL_AURA_MOD_TOTAL_MANA_REGEN_PCT
            case 24:    //SPELL_AURA_PERIODIC_ENERGIZE
                    packet << miscValue << amount;
                    break;
            case 64:    //SPELL_AURA_PERIODIC_MANA_LEECH
                    packet << miscValue << amount << gainMultiplier;
                    break;
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
