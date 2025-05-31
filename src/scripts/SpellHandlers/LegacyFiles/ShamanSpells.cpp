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
#include "Management/ItemInterface.h"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/Summons/SummonDefines.hpp"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/Definitions/PowerType.hpp"
#include "Spell/Definitions/ProcFlags.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Spell Defs
bool FlametongueWeaponPassive(uint8_t /*effectIndex*/, Aura* pAura, bool apply)
{
    Unit* target = pAura->getOwner();
    if (apply)
    {
        // target is always a player
        Item* item = static_cast<Player*>(target)->getItemInterface()->GetItemByGUID(pAura->itemCasterGUID);
        target->addProcTriggerSpell(10444, pAura->getSpellInfo()->getId(), pAura->getCasterGuid(), pAura->getSpellInfo()->getProcChance(), PROC_ON_DONE_MELEE_HIT, EXTRA_PROC_NULL, NULL, NULL, pAura, item);
    }
    else
        target->removeProcTriggerSpell(10444, pAura->getCasterGuid(), pAura->itemCasterGUID);
    return true;
}

bool SkyShatterRegalia(uint8_t /*effectIndex*/, Spell* s)
{
    // Shaman - Skyshatter Regalia - Two Piece Bonus
    // it checks for earth, air, water, fire totems and triggers Totemic Mastery spell 38437.

    if (!s->getPlayerCaster())
        return false;

    if (s->getPlayerCaster()->getSummonInterface()->hasTotemInSlot(SUMMON_SLOT_TOTEM_FIRE) &&
        s->getPlayerCaster()->getSummonInterface()->hasTotemInSlot(SUMMON_SLOT_TOTEM_EARTH) &&
        s->getPlayerCaster()->getSummonInterface()->hasTotemInSlot(SUMMON_SLOT_TOTEM_WATER) &&
        s->getPlayerCaster()->getSummonInterface()->hasTotemInSlot(SUMMON_SLOT_TOTEM_AIR))
    {
        auto aur = sSpellMgr.newAura(sSpellMgr.getSpellInfo(38437), 5000, s->getPlayerCaster(), s->getPlayerCaster(), true);

        for (uint8_t j = 0; j < 3; j++)
            aur->addAuraEffect(static_cast<AuraEffect>(aur->getSpellInfo()->getEffectRadiusIndex(j)), aur->getSpellInfo()->getEffectBasePoints(j) + 1, aur->getSpellInfo()->getEffectMiscValue(j), 1.0f, false, j);

        s->getPlayerCaster()->addAura(std::move(aur));
    }

    return true;
}

bool ManaTide(uint8_t /*effectIndex*/, Spell* s)
{
    Unit* unitTarget = s->getUnitTarget();

    if (unitTarget == NULL || unitTarget->isDead() || unitTarget->getClass() == WARRIOR || unitTarget->getClass() == ROGUE)
        return false;

    uint32_t gain = (uint32_t)(unitTarget->getMaxPower(POWER_TYPE_MANA) * 0.06);
    unitTarget->energize(unitTarget, 16191, gain, POWER_TYPE_MANA);

    return true;
}

bool Reincarnation(uint8_t /*effectIndex*/, Aura* a, bool apply)
{
    Unit* u_target = a->getOwner();

    if (!u_target->isPlayer())
        return true;

    Player* p_target = static_cast<Player*>(u_target);

    if (apply)
        p_target->m_reincarnation = true;
    else
        p_target->m_reincarnation = false;

    return true;
}

void SetupLegacyShamanSpells(ScriptMgr* mgr)
{
    uint32_t FlametongueWeaponPassiveIds[] = { 10400, 15567, 15568, 15569, 16311, 16312, 16313, 58784, 58791, 58792, 0 };
    mgr->register_dummy_aura(FlametongueWeaponPassiveIds, &FlametongueWeaponPassive);

    mgr->register_dummy_spell(38443, &SkyShatterRegalia);

    mgr->register_dummy_spell(39610, &ManaTide);

    mgr->register_dummy_aura(20608, &Reincarnation);
}
