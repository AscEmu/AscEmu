/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/PowerType.h>

bool Execute(uint8_t effectIndex, Spell* pSpell)
{
    if (pSpell->p_caster == NULL || pSpell->GetUnitTarget() == NULL)
    {
        return true;
    }

    Player* Caster = pSpell->p_caster;
    Unit* Target = pSpell->GetUnitTarget();

    uint32 rage = Caster->getPower(POWER_TYPE_RAGE);

    if (Caster->HasAura(58367))     // Glyph of Execution: Your Execute ability deals damage as if you had 10 additional rage.
    {
        rage += 10;
    }

    uint32 toadd = 0;
    int32 dmg = 0;
    uint32 multiple[] = { 0, 3, 6, 9, 12, 15, 18, 21, 30, 38, };

    if (rage >= 30)
    {
        toadd = (multiple[pSpell->getSpellInfo()->custom_RankNumber] * 30);
    }
    else
    {
        toadd = (multiple[pSpell->getSpellInfo()->custom_RankNumber] * rage);
    }

    dmg = pSpell->CalculateEffect(effectIndex, pSpell->GetUnitTarget());
    dmg += Caster->getAttackPower() / 5;
    dmg += toadd;

    Caster->Strike(Target, 0, pSpell->getSpellInfo(), 0, 0, dmg, false, false);

    return true;
}

bool Vigilance(uint8_t /*effectIndex*/, Spell* pSpell)
{
    if (!pSpell->p_caster)
    {
        return true;
    }

    pSpell->p_caster->ClearCooldownForSpell(355);   // Taunt

    return true;
}

bool DamageShield(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (apply)
        target->AddProcTriggerSpell(59653, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK_VICTIM | PROC_ON_BLOCK_VICTIM, 0, NULL, NULL);
    else
        target->RemoveProcTriggerSpell(59653, pAura->m_casterGuid);

    return true;
}

bool HeroicFury(uint8_t /*effectIndex*/, Spell* s)
{
    Player* p_caster = s->p_caster;

    if (!p_caster)
    {
        return false;
    }

    if (p_caster->HasSpell(20252))
    {
        p_caster->ClearCooldownForSpell(20252);
    }

    for (uint32 x = MAX_NEGATIVE_AURAS_EXTEDED_START; x < MAX_NEGATIVE_AURAS_EXTEDED_END; ++x)
    {
        if (p_caster->m_auras[x])
        {
            for (uint8 y = 0; y < 3; ++y)
            {
                switch (p_caster->m_auras[x]->GetSpellInfo()->getEffectApplyAuraName(y))
                {
                    case SPELL_AURA_MOD_ROOT:
                    case SPELL_AURA_MOD_DECREASE_SPEED:
                        p_caster->m_auras[x]->Remove();
                        break;
                }
            }
        }
    }

    return true;
}

bool Charge(uint8_t effectIndex, Spell* s)
{
    if (!s->u_caster)
    {
        return false;
    }

    uint32 rage_to_gen = s->getSpellInfo()->getEffectBasePoints(effectIndex) + 1;
    if (s->p_caster)
    {
        for (std::set<uint32>::iterator itr = s->p_caster->mSpells.begin(); itr != s->p_caster->mSpells.end(); ++itr)
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
    s->u_caster->modPower(POWER_TYPE_RAGE, rage_to_gen);

    return true;
}

bool LastStand(uint8_t /*effectIndex*/, Spell* s)
{
    Player* playerTarget = s->GetPlayerTarget();

    if (!playerTarget)
    {
        return false;
    }

    SpellCastTargets tgt;
    tgt.m_unitTarget = playerTarget->getGuid();

    SpellInfo const* inf = sSpellMgr.getSpellInfo(12976);
    Spell* spe = sSpellMgr.newSpell(s->u_caster, inf, true, NULL);
    spe->prepare(&tgt);

    return true;
}

bool BerserkerRage(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u = a->GetTarget();
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
        p_target->rageFromDamageTaken += 100;
    }
    else
    {
        p_target->rageFromDamageTaken -= 100;
    }

    for (uint8 i = 0; i < 3; i++)
    {
        if (apply)
        {
            p_target->MechanicsDispels[a->GetSpellInfo()->getEffectMiscValue(i)]++;
            p_target->RemoveAllAurasByMechanic(a->GetSpellInfo()->getEffectMiscValue(i), 0, false);
        }
        else
        {
            p_target->MechanicsDispels[a->GetSpellInfo()->getEffectMiscValue(i)]--;
        }
    }

    return true;
}

bool SweepingStrikes(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    if (apply)
        m_target->AddExtraStrikeTarget(a->GetSpellInfo(), 10);
    else
        m_target->RemoveExtraStrikeTarget(a->GetSpellInfo());

    return true;
}

bool TacticalAndStanceMastery(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* u_target = a->GetTarget();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (p_target == NULL)
        return true;

    if (apply)
        p_target->m_retainedrage += (a->GetModAmount(effectIndex) * 10);     //don't really know if value is all value or needs to be multiplied with 10
    else
        p_target->m_retainedrage -= (a->GetModAmount(effectIndex) * 10);

    return true;
}

bool SecondWind(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->SetTriggerStunOrImmobilize(29841, 100, true);  //fixed 100% chance
    else
        caster->SetTriggerStunOrImmobilize(0, 0, true);

    return true;
}

bool SecondWind2(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Player* caster = a->GetPlayerCaster();

    if (caster == NULL)
        return true;

    if (apply)
        caster->SetTriggerStunOrImmobilize(29842, 100, true);  //fixed 100% chance
    else
        caster->SetTriggerStunOrImmobilize(0, 0, true);

    return true;
}

bool ArmoredToTheTeeth(uint8_t /*effectIndex*/, Spell* /*s*/)
{
    // Same as Death Knight's Bladed Armor. See DeathKnightSpells.cpp line 276 for detailed explanation.
    return true;
};

void SetupWarriorSpells(ScriptMgr* mgr)
{
    uint32 ExecuteIds[] =
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

    uint32 tacticalandstancemasteryids[] =
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

    uint32 ArmoredToTheTeethIDs[] =
    {
        61216,
        61221,
        61222,
        0
    };
    mgr->register_dummy_spell(ArmoredToTheTeethIDs, &ArmoredToTheTeeth);
}
