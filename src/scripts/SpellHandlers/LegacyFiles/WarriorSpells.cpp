/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/PowerType.hpp"

bool Execute(uint8_t effectIndex, Spell* pSpell)
{
    if (pSpell->getPlayerCaster() == NULL || pSpell->getUnitTarget() == NULL)
    {
        return true;
    }

    Player* Caster = pSpell->getPlayerCaster();
    Unit* Target = pSpell->getUnitTarget();

    uint32_t rage = Caster->getPower(POWER_TYPE_RAGE);

    if (Caster->hasAurasWithId(58367))     // Glyph of Execution: Your Execute ability deals damage as if you had 10 additional rage.
    {
        rage += 10;
    }

    uint32_t toadd = 0;
    int32_t dmg = 0;
    uint32_t multiple[] = { 0, 3, 6, 9, 12, 15, 18, 21, 30, 38, };

    const uint8_t rank = pSpell->getSpellInfo()->hasSpellRanks() ? pSpell->getSpellInfo()->getRankInfo()->getRank() : 1;
    if (rage >= 30)
    {
        toadd = (multiple[rank] * 30);
    }
    else
    {
        toadd = (multiple[rank] * rage);
    }

    dmg = pSpell->calculateEffect(effectIndex);
    dmg += Caster->getAttackPower() / 5;
    dmg += toadd;

    Caster->strike(Target, MELEE, pSpell->getSpellInfo(), 0, 0, dmg, false, false);

    return true;
}

bool Vigilance(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->getPlayerCaster())
    {
        return true;
    }

    pSpell->getPlayerCaster()->clearCooldownForSpell(355);   // Taunt

    return true;
}

bool DamageShield(uint8_t /*effectIndex*/, Aura* /*pAura*/, bool /*apply*/)
{
    /*Unit* target = pAura->getOwner();

    if (apply)
        target->addProcTriggerSpell(59653, pAura->getSpellId(), pAura->getCasterGuid(), pAura->getSpellInfo()->getProcChance(), SpellProcFlags(PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_BLOCK_VICTIM), EXTRA_PROC_NULL, 0, NULL, NULL);
    else
        target->removeProcTriggerSpell(59653, pAura->getCasterGuid());
        */
    return true;
}

bool HeroicFury(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->getPlayerCaster();

    if (!p_caster)
    {
        return false;
    }

    if (p_caster->hasSpell(20252))
    {
        p_caster->clearCooldownForSpell(20252);
    }

    SpellMechanic mechanics[3] =
    {
        MECHANIC_ENSNARED,
        MECHANIC_ROOTED,
        MECHANIC_NONE
    };

    p_caster->removeAllAurasBySpellMechanic(mechanics);

    return true;
}

bool Charge(uint8_t effectIndex, Spell* s)
{
    if (!s->getUnitCaster())
    {
        return false;
    }

    uint32_t rage_to_gen = s->getSpellInfo()->getEffectBasePoints(effectIndex) + 1;
    if (s->getPlayerCaster())
    {
        for (std::set<uint32_t>::iterator itr = s->getPlayerCaster()->getSpellSet().begin(); itr != s->getPlayerCaster()->getSpellSet().end(); ++itr)
        {
            if (*itr == 12697)
            {
                rage_to_gen += 100;
            }

            if (*itr == 12285)
            {
                rage_to_gen += 50;
            }
        }
    }

    // Add the rage to the caster
    s->getUnitCaster()->modPower(POWER_TYPE_RAGE, rage_to_gen);

    return true;
}

bool LastStand(uint8_t /*effectIndex*/, Spell* s)
{
    Player* playerTarget = s->getPlayerTarget();

    if (!playerTarget)
    {
        return false;
    }

    SpellCastTargets tgt(playerTarget->getGuid());

    SpellInfo const* inf = sSpellMgr.getSpellInfo(12976);
    Spell* spe = sSpellMgr.newSpell(s->getUnitCaster(), inf, true, NULL);
    spe->prepare(&tgt);

    return true;
}

bool BerserkerRage(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u = a->getOwner();
    Player* p_target = NULL;

    if (u->isPlayer())
    {
        p_target = static_cast<Player*>(u);
    }

    if (p_target == NULL)
    {
        return true;
    }

    if (apply)
    {
        p_target->m_rageFromDamageTaken += 100;
    }
    else
    {
        p_target->m_rageFromDamageTaken -= 100;
    }

    SpellMechanic mechanics[4] = { MECHANIC_NONE };
    for (uint8_t i = 0; i < 3; i++)
    {
        if (apply)
        {
            p_target->m_mechanicsDispels[a->getSpellInfo()->getEffectMiscValue(i)]++;
            mechanics[i] = static_cast<SpellMechanic>(a->getSpellInfo()->getEffectMiscValue(i));
        }
        else
        {
            p_target->m_mechanicsDispels[a->getSpellInfo()->getEffectMiscValue(i)]--;
        }
    }

    if (apply)
        p_target->removeAllAurasBySpellMechanic(mechanics);

    return true;
}

bool SweepingStrikes(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();

    if (apply)
        m_target->addExtraStrikeTarget(a->getSpellInfo(), 10);
    else
        m_target->removeExtraStrikeTarget(a->getSpellInfo());

    return true;
}

bool TacticalAndStanceMastery(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (p_target == NULL)
        return true;

    if (apply)
        p_target->m_retaineDrage += (a->getEffectDamage(effectIndex) * 10);     //don't really know if value is all value or needs to be multiplied with 10
    else
        p_target->m_retaineDrage -= (a->getEffectDamage(effectIndex) * 10);

    return true;
}

bool SecondWind(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->setTriggerStunOrImmobilize(29841, 100, true);  //fixed 100% chance
    else
        caster->setTriggerStunOrImmobilize(0, 0, true);

    return true;
}

bool SecondWind2(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->setTriggerStunOrImmobilize(29842, 100, true);  //fixed 100% chance
    else
        caster->setTriggerStunOrImmobilize(0, 0, true);

    return true;
}

bool ArmoredToTheTeeth(uint8_t /*effectIndex*/, Spell* /*s*/)
{
    // Same as Death Knight's Bladed Armor. See DeathKnightSpells.cpp line 276 for detailed explanation.
    return true;
};

void SetupLegacyWarriorSpells(ScriptMgr* mgr)
{
    uint32_t ExecuteIds[] =
    {
        5308,  // Rank 1
        20658, // Rank 2
        20660, // Rank 3
        20661, // Rank 4
        20662, // Rank 5
        25234, // Rank 6
        25236, // Rank 7
        47470, // Rank 8
        47471, // Rank 9
        0,
    };
    mgr->register_dummy_spell(ExecuteIds, &Execute);
    mgr->register_script_effect(50725, &Vigilance);

    mgr->register_dummy_aura(58872, &DamageShield);
    mgr->register_dummy_aura(58874, &DamageShield);

    mgr->register_dummy_spell(60970, &HeroicFury);

    mgr->register_dummy_spell(100, &Charge);
    mgr->register_dummy_spell(6178, &Charge);
    mgr->register_dummy_spell(11578, &Charge);

    mgr->register_dummy_spell(12975, &LastStand);

    mgr->register_dummy_aura(18499, &BerserkerRage);

    mgr->register_dummy_aura(12328, &SweepingStrikes);

    uint32_t tacticalandstancemasteryids[] =
    {
        12295,
        12676,
        12677,
        12678,
        0
    };
    mgr->register_dummy_aura(tacticalandstancemasteryids, &TacticalAndStanceMastery);

    mgr->register_dummy_aura(29834, &SecondWind);
    mgr->register_dummy_aura(29838, &SecondWind2);

    uint32_t ArmoredToTheTeethIDs[] =
    {
        61216,
        61221,
        61222,
        0
    };
    mgr->register_dummy_spell(ArmoredToTheTeethIDs, &ArmoredToTheTeeth);
}
