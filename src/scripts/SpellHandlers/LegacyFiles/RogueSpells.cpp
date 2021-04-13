/*
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2011 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/SpellIsFlags.h>
#include <Spell/Definitions/SpellMechanics.h>

//Alice : Correct formula for Rogue - Preparation

bool Preparation(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster()) return true;

    pSpell->getPlayerCaster()->clearCooldownForSpell(5277);          // Evasion Rank 1
    pSpell->getPlayerCaster()->clearCooldownForSpell(26669);         // Evasion Rank 2
    pSpell->getPlayerCaster()->clearCooldownForSpell(2983);          // Sprint Rank 1
    pSpell->getPlayerCaster()->clearCooldownForSpell(8696);          // Sprint Rank 2
    pSpell->getPlayerCaster()->clearCooldownForSpell(11305);         // Sprint Rank 3
    pSpell->getPlayerCaster()->clearCooldownForSpell(1856);          // Vanish Rank 1
    pSpell->getPlayerCaster()->clearCooldownForSpell(1857);          // Vanish Rank 2
    pSpell->getPlayerCaster()->clearCooldownForSpell(26889);         // Vanish Rank 3
    pSpell->getPlayerCaster()->clearCooldownForSpell(14177);         // Cold Blood
    pSpell->getPlayerCaster()->clearCooldownForSpell(36554);         // Shadowstep
    if (pSpell->getPlayerCaster()->HasAura(56819))                   // Glyph of Preparation item = 42968 casts 57127 that apply aura 56819.
    {
        pSpell->getPlayerCaster()->clearCooldownForSpell(13877);     // Blade Flurry
        pSpell->getPlayerCaster()->clearCooldownForSpell(51722);     // Dismantle
        pSpell->getPlayerCaster()->clearCooldownForSpell(1766);      // Kick
    }
    return true;
}

bool Shiv(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* pTarget = pSpell->GetUnitTarget();
    if (!pSpell->getPlayerCaster() || !pTarget) return true;

    pSpell->getPlayerCaster()->castSpell(pTarget->getGuid(), 5940, true);

    Item* it = pSpell->getPlayerCaster()->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
    if (!it)
        return true;

    EnchantmentInstance* ench = it->GetEnchantment(TEMP_ENCHANTMENT_SLOT);
    if (ench)
    {
        DBC::Structures::SpellItemEnchantmentEntry const* Entry = ench->Enchantment;
        for (uint8_t c = 0; c < 3; c++)
        {
            if (Entry->type[c] && Entry->spell[c])
            {
                SpellInfo const* sp = sSpellMgr.getSpellInfo(Entry->spell[c]);
                if (!sp) return true;

                if (sp->custom_c_is_flags & SPELL_FLAG_IS_POISON)
                {
                    pSpell->getPlayerCaster()->castSpell(pTarget->getGuid(), Entry->spell[c], true);
                }
            }
        }
    }
    return true;
}

bool ImprovedSprint(uint8_t effectIndex, Spell* pSpell)
{
    if (effectIndex == 0)
    {
        Unit* target = pSpell->GetUnitTarget();
        if (target == NULL)
            return true;

        target->RemoveAllAurasByMechanic(MECHANIC_ENSNARED, 0, true);
        target->RemoveAllAurasByMechanic(MECHANIC_ROOTED, 0, true);
    }

    return true;
}

bool CloakOfShadows(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget || !unitTarget->isAlive())
        return false;

    Aura* pAura;
    for (uint32_t j = MAX_NEGATIVE_AURAS_EXTEDED_START; j < MAX_NEGATIVE_AURAS_EXTEDED_END; ++j)
    {
        pAura = unitTarget->m_auras[j];
        if (pAura != NULL && !pAura->IsPassive()
            && pAura->isNegative()
            && !(pAura->getSpellInfo()->getAttributes() & ATTRIBUTES_IGNORE_INVULNERABILITY)
            && pAura->getSpellInfo()->getFirstSchoolFromSchoolMask() != 0
            )
            pAura->removeAura();
    }

    return true;
}

bool CheatDeath(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();
    Player* p_target = NULL;

    if (u_target->isPlayer())
        p_target = static_cast<Player*>(u_target);

    if (p_target != NULL)
    {
        int32_t m = (int32_t)(8.0f * p_target->CalcRating(PCR_MELEE_CRIT_RESILIENCE));
        if (m > 90)
            m = 90;

        float val;

        if (apply)
        {
            val = -m / 100.0f;
        }
        else
        {
            val = m / 100.0f;
        }

        for (uint32_t x = 0; x < 7; x++)
            p_target->DamageTakenPctMod[x] += val;
    }

    return true;
}

bool MasterOfSubtlety(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();
    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    int32_t amount = a->getEffectDamage(effectIndex);

    if (apply)
    {
        p_target->m_outStealthDamageBonusPct += amount;
        p_target->m_outStealthDamageBonusPeriod = 6;        // 6 seconds
        p_target->m_outStealthDamageBonusTimer = 0;         // reset it
    }
    else
    {
        p_target->m_outStealthDamageBonusPct -= amount;
        p_target->m_outStealthDamageBonusPeriod = 6;        // 6 seconds
        p_target->m_outStealthDamageBonusTimer = 0;         // reset it
    }

    return true;
}

bool PreyOnTheWeakPeriodicDummy(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();
    Player* p_target = NULL;

    if (!apply)
        return true;

    if (m_target->isPlayer())
        p_target = static_cast<Player*>(m_target);

    if (p_target != NULL && p_target->getClass() == ROGUE)
    {

        Unit* target = p_target->GetMapMgr()->GetUnit(p_target->CombatStatus.GetPrimaryAttackTarget());
        if (target == NULL)
            return true;

        uint32_t plrHP = p_target->getHealth();
        uint32_t targetHP = target->getHealth();

        if (plrHP > targetHP)
            p_target->castSpell(p_target, 58670, true);
    }

    return true;
}

bool KillingSpreePeriodicDummy(uint8_t /*effectIndex*/, Aura* a, bool /*apply*/)
{
    Unit* m_target = a->getOwner();
    if (!m_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(m_target);

    //Find targets around aura's target in range of 10 yards.
    //It can hit same target multiple times.
    for (const auto& itr : p_target->getInRangeObjectsSet())
    {
        if (itr)
        {
            //Get the range of 10 yards from Effect 1
            float r = static_cast<float>(a->getSpellInfo()->getEffectRadiusIndex(1));

            //Get initial position of aura target (caster)
            LocationVector source = p_target->GetPosition();

            //Calc distance to the target
            float dist = itr->CalcDistance(source);

            //Radius check
            if (dist <= r)
            {
                //Avoid targeting anything that is not unit and not alive
                if (!itr->isCreatureOrPlayer() || !static_cast<Unit*>(itr)->isAlive())
                    continue;

                uint64_t spellTarget = itr->getGuid();
                //SPELL_EFFECT_TELEPORT
                p_target->castSpell(spellTarget, 57840, true);
                //SPELL_EFFECT_NORMALIZED_WEAPON_DMG and triggering 57842 with the same effect
                p_target->castSpell(spellTarget, 57841, true);
            }

        }
    }
    return true;
}

bool KillingSpreeEffectDummy(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (p_caster == NULL)
        return true;

    //SPELL_EFFECT_BREAK_PLAYER_TARGETING
    //and applying 20% SPELL_AURA_MOD_DAMAGE_PERCENT_DONE
    p_caster->castSpell(p_caster, 61851, true);

    return true;
}

void SetupLegacyRogueSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_spell(5938, &Shiv);
    mgr->register_dummy_spell(14185, &Preparation);
    mgr->register_dummy_spell(30918, &ImprovedSprint);

    mgr->register_dummy_spell(35729, &CloakOfShadows);

    mgr->register_dummy_aura(45182, &CheatDeath);


    uint32_t masterofsubtletyids[] =
    {
        31223,
        31222,
        31221,
        0
    };
    mgr->register_dummy_aura(masterofsubtletyids, &MasterOfSubtlety);

    uint32_t preyontheweakids[] =
    {
        51685,
        51686,
        51687,
        51688,
        51689,
        0
    };
    mgr->register_dummy_aura(preyontheweakids, &PreyOnTheWeakPeriodicDummy);

    mgr->register_dummy_aura(51690, &KillingSpreePeriodicDummy);
    mgr->register_dummy_spell(51690, &KillingSpreeEffectDummy);
}
