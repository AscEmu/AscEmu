/*
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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
        if (Target->isValidAttackableTarget(m_caster) && m_caster->CalcDistance(itr) <= pSpell->getEffectRadius(effectIndex))
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
}
