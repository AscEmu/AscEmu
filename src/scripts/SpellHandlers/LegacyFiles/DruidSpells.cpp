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
#include "Objects/Units/Unit.hpp"
#include "Server/EventMgr.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"

bool Starfall(uint8_t effectIndex, Spell* pSpell)
{
    Unit* m_caster = pSpell->getUnitCaster();
    if (m_caster == NULL)
        return true;

    uint8_t am = 0;
    for (const auto& itr : m_caster->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreatureOrPlayer())
            continue;

        Unit* Target = static_cast<Unit*>(itr);
        if (Target->isValidTarget(m_caster) && m_caster->CalcDistance(itr) <= pSpell->getEffectRadius(effectIndex))
        {
            m_caster->castSpell(Target, pSpell->getSpellInfo()->calculateEffectValue(effectIndex, m_caster), true);
            ++am;
            if (am >= 2)
                return true;
        }
    }
    return true;
}

bool PredatoryStrikes(uint8_t effectIndex, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();
    int32_t realamount = 0;

    realamount = (a->getEffectDamage(effectIndex) * m_target->getLevel()) / 100;

    if (apply)
    {
        m_target->modAttackPowerMods(realamount);
    }
    else
        m_target->modAttackPowerMods(-realamount);

    m_target->calculateDamage();

    return true;
}

bool Tranquility(uint8_t effectIndex, Aura* a, bool apply)
{
    if (apply)
        sEventMgr.AddEvent(a, &Aura::EventPeriodicHeal1, (uint32_t)a->getEffectDamage(effectIndex), EVENT_AURA_PERIODIC_HEAL, 2000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    else
        sEventMgr.RemoveEvents(a, EVENT_AURA_PERIODIC_HEAL);

    return true;
}

bool LifeBloom(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* m_target = a->getOwner();

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
    for (uint16_t x = AuraSlots::POSITIVE_SLOT_START; x < AuraSlots::POSITIVE_SLOT_END; ++x)
    {
        if (auto* const aur = m_target->getAuraWithAuraSlot(x))
        {
            if (aur->getSpellId() == a->getSpellId())
            {
                aur->m_ignoreunapply = true;
                if (aur->getTimeLeft())
                    expired = false;
                aur->removeAura();
            }
        }
    }

    // you can't do this here! Breaks unix lib load.
    /*if (expired)
    {

        Spell* spell = sSpellMgr.newSpell(pCaster, a->getSpellInfo(), true, NULL);
        spell->SetUnitTarget(m_target);
        spell->Heal(a->getEffectDamage(effectIndex));
        delete spell;
    }*/

    return true;
}

void SetupLegacyDruidSpells(ScriptMgr* mgr)
{
    uint32_t StarfallIds[] =
    {
        50286, // Rank 1
        53196, // Rank 2
        53197, // Rank 3
        53198, // Rank 4
        0,
    };
    mgr->register_dummy_spell(StarfallIds, &Starfall);

    uint32_t predatorystrikesids[] =
    {
        16972,
        16974,
        16975,
        0
    };
    mgr->register_dummy_aura(predatorystrikesids, &PredatoryStrikes);

    uint32_t tranquilityids[] =
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

    uint32_t lifebloomids[] =
    {
        33763,
        48450,
        48451,
        0
    };
    mgr->register_dummy_aura(lifebloomids, &LifeBloom);
}
