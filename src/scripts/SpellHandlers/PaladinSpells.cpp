/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2007 Moon++ <http://www.moonplusplus.info/>
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
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/PowerType.h>

bool EyeForAnEye(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (apply)
        target->AddProcTriggerSpell(25997, pAura->GetSpellInfo()->getId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_CRIT_HIT_VICTIM | PROC_ON_RANGED_CRIT_ATTACK_VICTIM | PROC_ON_SPELL_CRIT_HIT_VICTIM, 0, NULL, NULL);
    else
        target->RemoveProcTriggerSpell(25997, pAura->m_casterGuid);

    return true;
}

bool HolyShock(uint8_t /*effectIndex*/, Spell* pSpell)
{
    ///\todo This function returns true on failures (invalid target, invalid spell). Verify this is the correct return value
    Unit* target = pSpell->GetUnitTarget();
    if (target == nullptr)
    {
        return true;
    }

    Player* caster = pSpell->p_caster;
    if (caster == nullptr)
    {
        return true;
    }

    uint32 spell_id = 0;

    if (isAttackable(caster, target))
    {
        // Cast offensive Holy Shock
        switch (pSpell->GetSpellInfo()->getId())
        {
            case 20473: // Rank 1
                spell_id = 25912;
                break;
            case 20929: // Rank 2
                spell_id = 25911;
                break;
            case 20930: // Rank 3
                spell_id = 25902;
                break;
            case 27174: // Rank 4
                spell_id = 27176;
                break;
            case 33072: // Rank 5
                spell_id = 33073;
                break;
            case 48824: // Rank 6
                spell_id = 48822;
                break;
            case 48825: // Rank 7
                spell_id = 48823;
                break;
            default: // Invalid case, spell handler is assigned to wrong spell
            {
                LOG_ERROR("(Offensive) Holy Shock spell handler assigned to invalid spell id [%u]", pSpell->GetSpellInfo()->getId());
                return true;
            }
        }
    }
    else
    {
        // Cast healing Holy Shock
        switch (pSpell->GetSpellInfo()->getId())
        {
            case 20473: // Rank 1
                spell_id = 25914;
                break;
            case 20929: // Rank 2
                spell_id = 25913;
                break;
            case 20930: // Rank 3
                spell_id = 25903;
                break;
            case 27174: // Rank 4
                spell_id = 27175;
                break;
            case 33072: // Rank 5
                spell_id = 33074;
                break;
            case 48824: // Rank 6
                spell_id = 48820;
                break;
            case 48825: // Rank 7
                spell_id = 48821;
                break;
            default: // Invalid case, spell handler is assigned to wrong spell
            {
                LOG_ERROR("(Defensive) Holy Shock spell handler assigned to invalid spell id [%u]", pSpell->GetSpellInfo()->getId());
                return true;
            }
        }
    }

    caster->CastSpell(target, spell_id, false);

    return true;
}

bool SealOfRighteousness(uint8_t effectIndex, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (effectIndex == 0)
    {
        if (apply)
            target->AddProcTriggerSpell(25742, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL);
        else
            target->RemoveProcTriggerSpell(25742, pAura->m_casterGuid);
    }

    return true;
}

bool SealOfCorruption(uint8_t effectIndex, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (effectIndex == 0)
    {
        if (apply)
        {
            target->AddProcTriggerSpell(53742, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL);
            target->AddProcTriggerSpell(53739, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL);
        }
        else
        {
            target->RemoveProcTriggerSpell(53742, pAura->m_casterGuid);
            target->RemoveProcTriggerSpell(53739, pAura->m_casterGuid);
        }
    }

    return true;
}

bool SealOfVengeance(uint8_t effectIndex, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (effectIndex == 0)
    {
        if (apply)
        {
            target->AddProcTriggerSpell(31803, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL);
            target->AddProcTriggerSpell(42463, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL);
        }
        else
        {
            target->RemoveProcTriggerSpell(31803, pAura->m_casterGuid);
            target->RemoveProcTriggerSpell(42463, pAura->m_casterGuid);
        }
    }

    return true;
}

bool JudgementLightWisdomJustice(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->GetUnitTarget();
    if (target == nullptr)
    {
        return true;
    }

    Player* caster = pSpell->p_caster;
    if (caster == nullptr)
    {
        return true;
    }

    // Search for a previous judgement casted by this caster. He can have only 1 judgement active at a time
    uint32 index = 0;
    uint32 judgements[] =
    { 
        //SPELL_HASH_JUDGEMENT_OF_LIGHT,
        20185,
        20267,
        20271,
        28775,
        57774,
        //SPELL_HASH_JUDGEMENT_OF_WISDOM,
        20186,
        20268,
        53408,
        //SPELL_HASH_JUDGEMENT_OF_JUSTICE,
        20184,
        53407,
        //SPELL_HASH_JUDGEMENT_OF_VENGEANCE,
        31804,
        //SPELL_HASH_JUDGEMENT_OF_CORRUPTION,
        53733,
        //SPELL_HASH_JUDGEMENT_OF_RIGHTEOUSNESS,
        20187,
        0
    };

    uint64 prev_target = caster->getSingleTargetGuidForAura(judgements, &index);
    if (prev_target)
    {
        Unit* t = caster->GetMapMgr()->GetUnit(prev_target);
        if (t != nullptr)
        {
            t->removeAllAurasById(judgements[index]);
        }

        caster->removeSingleTargetGuidForAura(judgements[index]);
    }

    // Search for seal to unleash its energy
    uint32 seals[] = { 20375, 20165, 20164, 21084, 31801, 53736, 20166, 0 };

    Aura* aura = caster->getAuraWithId(seals);
    if (aura == nullptr)
    {
        return true;
    }

    uint32 id = 0;
    switch (aura->GetSpellId())
    {
        case 20375:
            id = 20467;
            break;
        case 20165:
            id = 54158;
            break;
        case 20164:
            id = 54158;
            break;
        case 21084:
            id = 20187;
            break;
        case 31801:
            id = aura->GetSpellInfo()->getEffectBasePoints(2);
            break;
        case 53736:
            id = aura->GetSpellInfo()->getEffectBasePoints(2);
            break;
        case 20166:
            id = 54158;
            break;
        default:
        {
            LOG_ERROR("JudgementLightWisdomJustice handler assigned to invalid spell id [%u]", pSpell->GetSpellInfo()->getId());
            return true;
        }
    }

    caster->CastSpell(target, id, true);

    // Cast judgement spell
    switch (pSpell->GetSpellInfo()->getId())
    {
        // SPELL_HASH_JUDGEMENT_OF_JUSTICE:
        case 20184:
        case 53407:
            id = 20184;
            break;
        // SPELL_HASH_JUDGEMENT_OF_LIGHT:
        case 20185:
        case 20267:
        case 20271:
        case 28775:
        case 57774:
            id = 20185;
            break;
        // SPELL_HASH_JUDGEMENT_OF_WISDOM:
        case 20186:
        case 20268:
        case 53408:
            id = 20186;
            break;
        default:
        {
            LOG_ERROR("JudgementLightWisdomJustice cast spell felt to invalid NameHash id [%u]", pSpell->GetSpellInfo()->getId());
            return true;
        }
    }

    caster->CastSpell(target, id, true);

    caster->setSingleTargetGuidForAura(pSpell->GetSpellInfo()->getId(), target->GetGUID());

    return true;
}

bool JudgementOfLight(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* caster = pAura->GetUnitCaster();
    if (caster == nullptr)
        return true;

    if (apply)
        caster->AddProcTriggerSpell(20267, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK | PROC_TARGET_SELF, 0, NULL, NULL);
    else
        caster->RemoveProcTriggerSpell(20267, pAura->m_casterGuid);

    return true;
}

bool JudgementOfWisdom(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* caster = pAura->GetUnitCaster();
    if (caster == nullptr)
        return true;

    if (apply)
        caster->AddProcTriggerSpell(20268, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK | PROC_TARGET_SELF, 0, NULL, NULL);
    else
        caster->RemoveProcTriggerSpell(20268, pAura->m_casterGuid);

    return true;
}

bool RighteousDefense(uint8_t /*effectIndex*/, Spell* s)
{
    //we will try to lure 3 enemies from our target

    Unit* unitTarget = s->GetUnitTarget();

    if (!unitTarget || !s->u_caster)
        return false;

    Unit* targets[3];
    uint32 targets_got = 0;

    for (std::set<Object*>::iterator itr = unitTarget->GetInRangeSetBegin(), i2; itr != unitTarget->GetInRangeSetEnd();)
    {
        i2 = itr++;

        // don't add objects that are not creatures and that are dead
        if (!(*i2)->IsCreature() || !static_cast<Creature*>((*i2))->isAlive())
            continue;

        Creature* cr = static_cast<Creature*>((*i2));
        if (cr->GetAIInterface()->getNextTarget() == unitTarget)
            targets[targets_got++] = cr;

        if (targets_got == 3)
            break;
    }

    for (uint32 j = 0; j < targets_got; j++)
    {
        //set threat to this target so we are the msot hated
        uint32 threat_to_him = targets[j]->GetAIInterface()->getThreatByPtr(unitTarget);
        uint32 threat_to_us = targets[j]->GetAIInterface()->getThreatByPtr(s->u_caster);
        int threat_dif = threat_to_him - threat_to_us;
        if (threat_dif > 0) //should nto happen
            targets[j]->GetAIInterface()->modThreatByPtr(s->u_caster, threat_dif);

        targets[j]->GetAIInterface()->AttackReaction(s->u_caster, 1, 0);
        targets[j]->GetAIInterface()->setNextTarget(s->u_caster);
    }

    return true;
}

bool Illumination(uint8_t /*effectIndex*/, Spell* s)
{
    switch (s->m_triggeredByAura == NULL ? s->GetSpellInfo()->getId() : s->m_triggeredByAura->GetSpellId())
    {
        case 20210:
        case 20212:
        case 20213:
        case 20214:
        case 20215:
        {
            if (s->p_caster == NULL)
                return false;
            SpellInfo* sp = s->p_caster->last_heal_spell ? s->p_caster->last_heal_spell : s->GetSpellInfo();
            s->p_caster->Energize(s->p_caster, 20272, 60 * s->u_caster->GetBaseMana() * sp->getManaCostPercentage() / 10000, POWER_TYPE_MANA);
        }
        break;


    }
    return true;
}

bool JudgementOfTheWise(uint8_t /*effectIndex*/, Spell* s)
{
    if (!s->p_caster)
        return false;

    s->p_caster->Energize(s->p_caster, 31930, uint32(0.15f * s->p_caster->GetBaseMana()), POWER_TYPE_MANA);
    s->p_caster->CastSpell(s->p_caster, 57669, false);

    return true;
}

bool GuardedByTheLight(uint8_t /*effectIndex*/, Spell* s)
{
    if (!s->p_caster)
        return false;

    if (Aura* aura = s->p_caster->getAuraWithId(54428))
        aura->Refresh();

    return true;
}

void SetupPaladinSpells(ScriptMgr* mgr)
{
    mgr->register_dummy_aura(9799, &EyeForAnEye);
    mgr->register_dummy_aura(25988, &EyeForAnEye);

    uint32 HolyShockIds[] = { 20473, 20929, 20930, 27174, 33072, 48824, 48825, 0 };
    mgr->register_dummy_spell(HolyShockIds, &HolyShock);

    mgr->register_dummy_aura(21084, &SealOfRighteousness);
    mgr->register_dummy_aura(53736, &SealOfCorruption);
    mgr->register_dummy_aura(31801, &SealOfVengeance);

    mgr->register_script_effect(20271, &JudgementLightWisdomJustice); //light
    mgr->register_script_effect(53408, &JudgementLightWisdomJustice);
    mgr->register_script_effect(53407, &JudgementLightWisdomJustice);

    mgr->register_script_effect(63521, &GuardedByTheLight);

    mgr->register_dummy_aura(20185, &JudgementOfLight);
    mgr->register_dummy_aura(20186, &JudgementOfWisdom);

    mgr->register_dummy_spell(31789, &RighteousDefense);
    mgr->register_dummy_spell(18350, &Illumination);

    mgr->register_dummy_spell(54180, &JudgementOfTheWise);
}
