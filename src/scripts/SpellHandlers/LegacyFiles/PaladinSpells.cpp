/*
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Master.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/Definitions/PowerType.hpp"

bool HolyShock(uint8_t /*effectIndex*/, Spell* pSpell)
{
    ///\todo This function returns true on failures (invalid target, invalid spell). Verify this is the correct return value
    Unit* target = pSpell->getUnitTarget();
    if (target == nullptr)
    {
        return true;
    }

    Player* caster = pSpell->getPlayerCaster();
    if (caster == nullptr)
    {
        return true;
    }

    uint32_t spell_id = 0;

    if (caster->isValidAttackableTarget(target))
    {
        // Cast offensive Holy Shock
        switch (pSpell->getSpellInfo()->getId())
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
                DLLLogDetail("(Offensive) Holy Shock spell handler assigned to invalid spell id [%u]", pSpell->getSpellInfo()->getId());
                return true;
            }
        }
    }
    else
    {
        // Cast healing Holy Shock
        switch (pSpell->getSpellInfo()->getId())
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
                DLLLogDetail("(Defensive) Holy Shock spell handler assigned to invalid spell id [%u]", pSpell->getSpellInfo()->getId());
                return true;
            }
        }
    }

    caster->castSpell(target, spell_id, false);

    return true;
}

bool RighteousDefense(uint8_t /*effectIndex*/, Spell* s)
{
    //we will try to lure 3 enemies from our target

    Unit* unitTarget = s->getUnitTarget();

    if (!unitTarget || !s->getUnitCaster())
        return false;

    Unit* targets[3];
    uint32_t targets_got = 0;

    for (const auto& itr : unitTarget->getInRangeObjectsSet())
    {
        if (itr)
        {
            // don't add objects that are not creatures and that are dead
            if (!itr->isCreature() || !static_cast<Creature*>(itr)->isAlive())
                continue;

            Creature* cr = static_cast<Creature*>(itr);
            if (cr->getAIInterface()->getCurrentTarget() == unitTarget)
                targets[targets_got++] = cr;

            if (targets_got == 3)
                break;
        }
    }

    for (uint32_t j = 0; j < targets_got; j++)
    {
        //set threat to this target so we are the msot hated
        float threat_to_him = targets[j]->getThreatManager().getThreat(unitTarget);
        float threat_to_us = targets[j]->getThreatManager().getThreat(s->getUnitCaster());
        float threat_dif = threat_to_him - threat_to_us;
        if (threat_dif > 0) //should nto happen
            targets[j]->getThreatManager().matchUnitThreatToHighestThreat(s->getUnitCaster());

        targets[j]->getAIInterface()->onHostileAction(s->getUnitCaster());
        targets[j]->getAIInterface()->setCurrentTarget(s->getUnitCaster());
    }

    return true;
}

bool Illumination(uint8_t /*effectIndex*/, Spell* s)
{
    switch (s->getTriggeredByAura() == NULL ? s->getSpellInfo()->getId() : s->getTriggeredByAura()->getSpellId())
    {
        case 20210:
        case 20212:
        case 20213:
        case 20214:
        case 20215:
        {
            if (s->getPlayerCaster() == NULL)
                return false;
            SpellInfo const* sp = s->getPlayerCaster()->m_lastHealSpell ? s->getPlayerCaster()->m_lastHealSpell : s->getSpellInfo();
            s->getPlayerCaster()->energize(s->getPlayerCaster(), 20272, 60 * s->getPlayerCaster()->getBaseMana() * sp->getManaCostPercentage() / 10000, POWER_TYPE_MANA);
        }
        break;


    }
    return true;
}

bool JudgementOfTheWise(uint8_t /*effectIndex*/, Spell* s)
{
    if (!s->getPlayerCaster())
        return false;

    s->getPlayerCaster()->energize(s->getPlayerCaster(), 31930, uint32_t(0.15f * s->getPlayerCaster()->getBaseMana()), POWER_TYPE_MANA);
    s->getPlayerCaster()->castSpell(s->getPlayerCaster(), 57669, false);

    return true;
}

bool GuardedByTheLight(uint8_t /*effectIndex*/, Spell* s)
{
    if (!s->getPlayerCaster())
        return false;

    if (Aura* aura = s->getPlayerCaster()->getAuraWithId(54428))
        aura->refreshOrModifyStack();

    return true;
}

void SetupLegacyPaladinSpells(ScriptMgr* mgr)
{
    uint32_t HolyShockIds[] = { 20473, 20929, 20930, 27174, 33072, 48824, 48825, 0 };
    mgr->register_dummy_spell(HolyShockIds, &HolyShock);

    mgr->register_script_effect(63521, &GuardedByTheLight);

    mgr->register_dummy_spell(31789, &RighteousDefense);
    mgr->register_dummy_spell(18350, &Illumination);

    mgr->register_dummy_spell(54180, &JudgementOfTheWise);
}
