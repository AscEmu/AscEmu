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
#include "Objects/Faction.h"
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"

bool Starfall(uint8_t effectIndex, Spell* pSpell)
{
    Unit* m_caster = pSpell->u_caster;
    if (m_caster == NULL)
        return true;

    uint8 am = 0;
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer())
            continue;

        Unit* Target = static_cast<Unit*>(itr);
        if (isAttackable(Target, m_caster) && m_caster->CalcDistance(itr) <= pSpell->GetRadius(effectIndex))
        {
            m_caster->castSpell(Target, pSpell->CalculateEffect(effectIndex, Target), true);
            ++am;
            if (am >= 2)
                return true;
        }
    }
    return true;
}

bool ImprovedLeaderOfThePack(uint8_t /*effectIndex*/, Spell* s)
{
    if (s->p_caster == NULL)
        return false;

    s->p_caster->AddProcTriggerSpell(34299, 34299, s->p_caster->getGuid(), 100, PROC_ON_CRIT_ATTACK | static_cast<uint32>(PROC_TARGET_SELF), 0, NULL, NULL);

    return true;
}

bool PredatoryStrikes(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();
    int32 realamount = 0;


    realamount = (a->GetModAmount(effectIndex) * m_target->getLevel()) / 100;

    if (apply)
    {
        a->SetPositive();
        m_target->modAttackPowerMods(realamount);
    }
    else
        m_target->modAttackPowerMods(-realamount);

    m_target->CalcDamage();

    return true;
}

bool Furor(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* u_target = a->GetTarget();

    if (!u_target->isPlayer())
        return true;
    Player* p_target = static_cast<Player*>(u_target);

    if (p_target == NULL)
        return true;

    if (apply)
        p_target->m_furorChance += a->GetModAmount(effectIndex);
    else
        p_target->m_furorChance -= a->GetModAmount(effectIndex);

    return true;
}

bool Tranquility(uint8_t effectIndex, Aura* a, bool apply)
{
    if (apply)
        sEventMgr.AddEvent(a, &Aura::EventPeriodicHeal1, (uint32)a->GetModAmount(effectIndex), EVENT_AURA_PERIODIC_HEAL, 2000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else
        sEventMgr.RemoveEvents(a, EVENT_AURA_PERIODIC_HEAL);

    return true;
}

bool LifeBloom(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* m_target = a->GetTarget();

    if (apply)
        return true;

    // apply ONCE only.
    if (a->m_ignoreunapply)
        return true;

    Unit* pCaster = a->GetUnitCaster();
    if (pCaster == NULL)
        pCaster = m_target;

    // Remove other Lifeblooms - but do NOT handle unapply again
    bool expired = true;
    for (uint32 x = MAX_POSITIVE_AURAS_EXTEDED_START; x < MAX_POSITIVE_AURAS_EXTEDED_END; x++)
    {
        if (m_target->m_auras[x])
        {
            if (m_target->m_auras[x]->GetSpellId() == a->GetSpellId())
            {
                m_target->m_auras[x]->m_ignoreunapply = true;
                if (m_target->m_auras[x]->GetTimeLeft())
                    expired = false;
                m_target->m_auras[x]->Remove();
            }
        }
    }

    // you can't do this here! Breaks unix lib load.
    /*if (expired)
    {
        
        Spell* spell = sSpellMgr.newSpell(pCaster, a->GetSpellInfo(), true, NULL);
        spell->SetUnitTarget(m_target);
        spell->Heal(a->GetModAmount(effectIndex));
        delete spell;
    }*/

    return true;
}

bool LeaderOfThePack(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->GetTarget();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (apply)
        p_target->AddShapeShiftSpell(24932);
    else
        p_target->RemoveShapeShiftSpell(24932);

    return true;
}

void SetupDruidSpells(ScriptMgr* mgr)
{
    uint32 StarfallIds[] =
    {
        50286, // Rank 1
        53196, // Rank 2
        53197, // Rank 3
        53198, // Rank 4
        0,
    };
    mgr->register_dummy_spell(StarfallIds, &Starfall);

    mgr->register_dummy_spell(34297, &ImprovedLeaderOfThePack);
    mgr->register_dummy_spell(34300, &ImprovedLeaderOfThePack);

    uint32 predatorystrikesids[] =
    {
        16972,
        16974,
        16975,
        0
    };
    mgr->register_dummy_aura(predatorystrikesids, &PredatoryStrikes);

    uint32 furorids[] =
    {
        17056,
        17058,
        17059,
        17060,
        17061,
        0
    };
    mgr->register_dummy_aura(furorids, &Furor);

    uint32 tranquilityids[] =
    {
        740,
        8918,
        9862,
        9863,
        26983,
        48446,
        48447,
        0
    };
    mgr->register_dummy_aura(tranquilityids, &Tranquility);

    uint32 lifebloomids[] =
    {
        33763,
        48450,
        48451,
        0
    };
    mgr->register_dummy_aura(lifebloomids, &LifeBloom);

    mgr->register_dummy_aura(17007, &LeaderOfThePack);
}
