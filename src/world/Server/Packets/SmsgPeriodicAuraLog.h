/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

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
        uint32_t overKillOrOverHeal;
        uint32_t schoolMask;
        uint32_t absorbAmount;
        uint32_t resistedAmount;
        uint8_t isCritical;
        uint32_t miscValue;
        float gainMultiplier;

        SmsgPeriodicAuraLog() : SmsgPeriodicAuraLog(WoWGuid(), WoWGuid(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f)
        {
        }

        SmsgPeriodicAuraLog(WoWGuid targetGuid, WoWGuid casterGuid, uint32_t spellId, uint32_t auraType, uint32_t amount, uint32_t overKillOrOverHeal,
            uint32_t schoolMask, uint32_t absorbAmount, uint32_t resistedAmount, uint8_t isCritical, uint32_t miscValue, float gainMultiplier) :
            ManagedPacket(SMSG_PERIODICAURALOG, 30),
            targetGuid(targetGuid),
            casterGuid(casterGuid),
            spellId(spellId),
            auraType(auraType),
            amount(amount),
            overKillOrOverHeal(overKillOrOverHeal),
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
            if (m_protocol.isLegion())
            {
                // target, caster, spell, one effect: type, amount, over heal or kill, school or power, absorbed or
                // amplitude, resisted, crit, debug info, sandbox scaling
                int32_t overHealOrKill = 0;
                int32_t schoolOrPower = 0;
                int32_t absorbedOrAmplitude = 0;
                int32_t resisted = 0;

                switch (auraType)
                {
                    case 3:     // SPELL_AURA_PERIODIC_DAMAGE
                    case 89:    // SPELL_AURA_PERIODIC_DAMAGE_PERCENT
                        overHealOrKill = static_cast<int32_t>(overKillOrOverHeal);
                        schoolOrPower = static_cast<int32_t>(schoolMask);
                        absorbedOrAmplitude = static_cast<int32_t>(absorbAmount);
                        resisted = static_cast<int32_t>(resistedAmount);
                        break;
                    case 8:     // SPELL_AURA_PERIODIC_HEAL
                    case 20:    // SPELL_AURA_PERIODIC_HEAL_PCT
                        overHealOrKill = static_cast<int32_t>(overKillOrOverHeal);
                        absorbedOrAmplitude = static_cast<int32_t>(absorbAmount);
                        break;
                    case 21:    // SPELL_AURA_PERIODIC_POWER_PCT
                    case 24:    // SPELL_AURA_PERIODIC_ENERGIZE
                        schoolOrPower = static_cast<int32_t>(miscValue);
                        break;
                    case 64:    // SPELL_AURA_PERIODIC_MANA_LEECH
                        schoolOrPower = static_cast<int32_t>(miscValue);
                        absorbedOrAmplitude = static_cast<int32_t>(gainMultiplier);
                        break;
                    default:
                        break;
                }

                packet << targetGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << casterGuid.toGuid128(m_protocol.realmId, m_receiverMapId);
                packet << int32_t(spellId);
                packet << uint32_t(1);
                packet.writeBit(false);                 // log data
                packet.flushBits();

                packet << int32_t(auraType);
                packet << int32_t(amount);
                packet << overHealOrKill;
                packet << schoolOrPower;
                packet << absorbedOrAmplitude;
                packet << resisted;
                packet.writeBit(isCritical != 0);
                packet.writeBit(false);
                packet.writeBit(false);
                packet.flushBits();
                return true;
            }

            packet << targetGuid << casterGuid << spellId << uint32_t(1) << auraType;

            switch (auraType)
            {
            case 3:     //SPELL_AURA_PERIODIC_DAMAGE
            case 89:    //SPELL_AURA_PERIODIC_DAMAGE_PERCENT;
            {
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << amount << overKillOrOverHeal << schoolMask << absorbAmount << resistedAmount << isCritical;
                else
                    packet << amount << schoolMask << absorbAmount << resistedAmount;
            } break;
            case 8:     //SPELL_AURA_PERIODIC_HEAL
            case 20:    //SPELL_AURA_PERIODIC_HEAL_PCT
            {
                if (m_protocol.expansion > WoW::Expansion::_TBC)
                    packet << amount << overKillOrOverHeal << absorbAmount << isCritical;
                else
                    packet << amount;
            } break;
            case 21:    //SPELL_AURA_PERIODIC_POWER_PCT
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
