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
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Spell/SpellAuras.h"
#include "Server/Script/ScriptMgr.h"
#include "Spell/Definitions/ProcFlags.h"
#include <Spell/Definitions/PowerType.h>

//////////////////////////////////////////////////////////////////////////////////////////
// Spell Defs
bool FlametongueWeaponPassive(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* target = pAura->GetTarget();

    if (apply)
    {
        // target is always a player
        Item* item = static_cast<Player*>(target)->getItemInterface()->GetItemByGUID(pAura->itemCasterGUID);
        target->AddProcTriggerSpell(10444, pAura->GetSpellInfo()->getId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), PROC_ON_MELEE_ATTACK, 0, NULL, NULL, item);
    }
    else
        target->RemoveProcTriggerSpell(10444, pAura->m_casterGuid, pAura->itemCasterGUID);

    return true;
}

bool SkyShatterRegalia(uint8_t /*effectIndex*/, Spell* s)
{
    // Shaman - Skyshatter Regalia - Two Piece Bonus
    // it checks for earth, air, water, fire totems and triggers Totemic Mastery spell 38437.

    if (!s->p_caster)
        return false;

    if (s->p_caster->summonhandler.HasSummonInSlot(0) &&
        s->p_caster->summonhandler.HasSummonInSlot(1) &&
        s->p_caster->summonhandler.HasSummonInSlot(2) &&
        s->p_caster->summonhandler.HasSummonInSlot(3))
    {
        Aura* aur = sSpellMgr.newAura(sSpellMgr.getSpellInfo(38437), 5000, s->p_caster, s->p_caster, true);

        for (uint8 j = 0; j < 3; j++)
            aur->AddMod(aur->GetSpellInfo()->getEffectRadiusIndex(j), aur->GetSpellInfo()->getEffectBasePoints(j) + 1, aur->GetSpellInfo()->getEffectMiscValue(j), j);

        s->p_caster->AddAura(aur);
    }

    return true;
}

bool ManaTide(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->GetUnitTarget();

    if (unitTarget == NULL || unitTarget->isDead() || unitTarget->getClass() == WARRIOR || unitTarget->getClass() == ROGUE)
        return false;

    uint32 gain = (uint32)(unitTarget->getMaxPower(POWER_TYPE_MANA) * 0.06);
    unitTarget->Energize(unitTarget, 16191, gain, POWER_TYPE_MANA);

    return true;
}

bool EarthShieldDummyAura(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* m_target = pAura->GetTarget();

    if (apply)
        m_target->AddProcTriggerSpell(379, pAura->GetSpellId(), pAura->m_casterGuid, pAura->GetSpellInfo()->getProcChance(), pAura->GetSpellInfo()->getProcFlags() & ~PROC_ON_SPELL_LAND_VICTIM, pAura->GetSpellInfo()->getProcCharges(), NULL, NULL);
    else if (m_target->GetAuraStackCount(pAura->GetSpellId()) == 1)
        m_target->RemoveProcTriggerSpell(379, pAura->m_casterGuid);

    return true;
}

bool Reincarnation(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->GetTarget();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (apply)
        p_target->bReincarnation = true;
    else
        p_target->bReincarnation = false;

    return true;
}

void SetupShamanSpells(ScriptMgr* mgr)
{
    uint32 FlametongueWeaponPassiveIds[] = { 10400, 15567, 15568, 15569, 16311, 16312, 16313, 58784, 58791, 58792, 0 };
    mgr->register_dummy_aura(FlametongueWeaponPassiveIds, &FlametongueWeaponPassive);

    mgr->register_dummy_spell(38443, &SkyShatterRegalia);

    mgr->register_dummy_spell(39610, &ManaTide);

    uint32 earthshielddummyauraids[] =
    {
        974,
        32593,
        32594,
        49283,
        49284,
        0
    };
    mgr->register_dummy_aura(earthshielddummyauraids, &EarthShieldDummyAura);

    mgr->register_dummy_aura(20608, &Reincarnation);
}
