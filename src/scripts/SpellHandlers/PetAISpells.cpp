/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Setup.h"
#include "Units/Summons/Summon.h"
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Units/Creatures/Pet.h"
#include "Spell/Spell.h"
#include "Server/Script/ScriptMgr.h"
#include <Spell/Definitions/PowerType.h>

class ArmyOfTheDeadGhoulAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ArmyOfTheDeadGhoulAI)
    explicit ArmyOfTheDeadGhoulAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->m_canMove = false;
    }

    void OnLoad() override
    {
        RegisterAIUpdateEvent(200);

        if (getCreature()->isSummon())
        {
            auto summon = dynamic_cast<Summon*>(getCreature());

            const auto parentBonus = summon->getUnitOwner()->GetDamageDoneMod(SCHOOL_NORMAL) * 0.04f;

            summon->setMinDamage(summon->getMinDamage() + parentBonus);
            summon->setMaxDamage(summon->getMaxDamage() + parentBonus);
        }
    }

    void AIUpdate() override
    {
        getCreature()->castSpell(getCreature()->getGuid(), 20480, false);
        RemoveAIUpdateEvent();
        getCreature()->GetAIInterface()->m_canMove = true;
    }
};

class ShadowFiendAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowFiendAI)
    explicit ShadowFiendAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnLoad() override
    {
        if (getCreature()->isPet())
        {
            auto pet = dynamic_cast<Pet*>(getCreature());
            auto playerOwner = dynamic_cast<Player*>(pet->getPlayerOwner());

            const auto ownerBonus = static_cast<float>(playerOwner->GetDamageDoneMod(SCHOOL_SHADOW) * 0.375f); // 37.5%
            pet->BaseAttackType = SCHOOL_SHADOW; // Melee hits are supposed to do damage with the shadow school
            pet->setBaseAttackTime(MELEE, 1500); // Shadowfiend is supposed to do 10 attacks, sometimes it can be 11
            pet->setMinDamage(pet->getMinDamage() + ownerBonus);
            pet->setMaxDamage(pet->getMaxDamage() + ownerBonus);
            pet->BaseDamage[0] += ownerBonus;
            pet->BaseDamage[1] += ownerBonus;

            const auto unitTarget = pet->GetMapMgr()->GetUnit(playerOwner->getTargetGuid());
            if (unitTarget != nullptr && isAttackable(playerOwner, unitTarget))
            {
                pet->GetAIInterface()->AttackReaction(unitTarget, 1);
                pet->GetAIInterface()->setNextTarget(unitTarget);
            }
        }
    }
};

class MirrorImageAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MirrorImageAI)
    explicit MirrorImageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnLoad() override
    {
        if (getCreature()->isSummon())
        {
            auto summon = dynamic_cast<Summon*>(getCreature());
            auto unitOwner = summon->getUnitOwner();

            unitOwner->castSpell(getCreature(), 45204, true);   // clone me
            unitOwner->castSpell(getCreature(), 58838, true);   // inherit threat list

            // Mage mirror image spell
            if (getCreature()->getCreatedBySpellId() == 58833)
            {
                getCreature()->setMaxHealth(2500);
                getCreature()->setHealth(2500);
                getCreature()->setMaxPower(POWER_TYPE_MANA, unitOwner->getMaxPower(POWER_TYPE_MANA));
                getCreature()->setPower(POWER_TYPE_MANA, unitOwner->getPower(POWER_TYPE_MANA));

                DBC::Structures::SpellRangeEntry const* range = nullptr;

                AI_Spell sp1{};
                sp1.entryId = 59638;
                sp1.spell = sSpellMgr.getSpellInfo(sp1.entryId);
                if (!sp1.spell)
                    return;

                sp1.spellType = STYPE_DAMAGE;
                sp1.agent = AGENT_SPELL;
                sp1.spelltargetType = TTYPE_SINGLETARGET;
                sp1.cooldown = 0;
                sp1.cooldowntime = 0;
                sp1.Misc2 = 0;
                sp1.procCount = 0;
                sp1.procChance = 100;
                range = sSpellRangeStore.LookupEntry(sp1.spell->getRangeIndex());
                sp1.minrange = GetMinRange(range);
                sp1.maxrange = GetMaxRange(range);

                getCreature()->GetAIInterface()->addSpellToList(&sp1);

                AI_Spell sp2{};
                sp2.entryId = 59637;
                sp2.spell = sSpellMgr.getSpellInfo(sp2.entryId);
                if (!sp2.spell)
                    return;

                sp2.spellType = STYPE_DAMAGE;
                sp2.agent = AGENT_SPELL;
                sp2.spelltargetType = TTYPE_SINGLETARGET;
                sp2.cooldown = 0;
                sp2.cooldowntime = 0;
                sp2.Misc2 = 0;
                sp2.procCount = 0;
                sp2.procChance = 100;
                range = sSpellRangeStore.LookupEntry(sp2.spell->getRangeIndex());
                sp2.minrange = GetMinRange(range);
                sp2.maxrange = GetMaxRange(range);

                getCreature()->GetAIInterface()->addSpellToList(&sp2);
            }
        }
    }
};


class DancingRuneWeaponAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DancingRuneWeaponAI)
    explicit DancingRuneWeaponAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        dpsCycle = 0;
        dpsSpell = 0;
    }

    void OnLoad() override
    {
        getCreature()->setDisplayId(getCreature()->GetCreatureProperties()->Female_DisplayID);
        getCreature()->setBaseAttackTime(MELEE, 2000);

        if (getCreature()->isSummon())
        {
            auto summon = dynamic_cast<Summon*>(getCreature());
            auto unitOwner = summon->getUnitOwner();

            if (unitOwner->isPlayer())
            {
                auto playerOwner = dynamic_cast<Player*>(unitOwner);
                const auto item = playerOwner->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
                if (item != nullptr)
                {
                    for (uint8 si = 0; si < 5; si++)
                    {
                        if (item->getItemProperties()->Spells[si].Id == 0)
                            continue;

                        if (item->getItemProperties()->Spells[si].Trigger == CHANCE_ON_HIT)
                            procSpell[si] = item->getItemProperties()->Spells[si].Id;
                    }

                    summon->setVirtualItemSlotId(MELEE, item->getEntry());
                    summon->setBaseAttackTime(MELEE, item->getItemProperties()->Delay);
                }

#if VERSION_STRING == WotLK
                playerOwner->setPower(POWER_TYPE_RUNIC_POWER, 0);
#endif
            }

            summon->setMinDamage(float(unitOwner->GetDamageDoneMod(SCHOOL_NORMAL)));
            summon->setMaxDamage(float(unitOwner->GetDamageDoneMod(SCHOOL_NORMAL)));
        }
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        RemoveAIUpdateEvent();
        dpsCycle = 0;
    }

    void AIUpdate() override
    {
        const auto currentTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (!getCreature()->isCastingSpell() && currentTarget)
        {
            switch (dpsCycle)
            {
                case 0:
                    dpsSpell = 49921; // Plague Strike
                    break;
                case 1:
                    dpsSpell = 49909; // Icy Touch
                    break;
                case 2:
                case 3:
                    dpsSpell = 55262; // Heart Strike x 2
                    break;
                case 4:
                    dpsSpell = 51425; // Obliterate
                    break;
                case 5:
                    dpsSpell = 49895; // Death Coil
                    break;
                case 6:
                case 7:
                    dpsSpell = 51425; // Obliterate x 2
                    break;
                case 8:
                case 9:
                    dpsSpell = 55262; // Heart Strike x 2
                    break;
                case 10:
                case 11:
                    dpsSpell = 49895; // Death Coil x 2
                    break;
            default: 
                break;
            }
            dpsCycle++;

            if (dpsCycle > 11)
                dpsCycle = 0;

            const auto nextSpell = sSpellMgr.getSpellInfo(dpsSpell);
            if (nextSpell)
                getCreature()->castSpell(currentTarget, nextSpell, true);
        }
    }

    void OnHit(Unit* mTarget, float /*fAmount*/) override
    {
        for (auto p : procSpell)
        {
            if (p != 0)
            {
                const auto spellProc = sSpellMgr.getSpellInfo(p);
                if (!spellProc)
                    return;

                const auto randomProcChance = Util::getRandomUInt(99);
                uint32_t spellProcChance = spellProc->getProcChance();
                if (spellProcChance < 1)
                    spellProcChance = 10; // Got to be fair :P

                if (randomProcChance <= spellProcChance)
                {
                    const auto victim = spellProc->custom_self_cast_only ? getCreature() : mTarget;
                    getCreature()->castSpell(victim, spellProc, true);
                }
            }
        }
    }
private:

    int dpsCycle;
    int dpsSpell;
    int procSpell[5];
};

class FrostBroodVanquisherAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FrostBroodVanquisherAI)
    explicit FrostBroodVanquisherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
    }

    void OnLoad() override
    {
        getCreature()->setAnimationFlags(UNIT_BYTE1_FLAG_HOVER);
    }

    void OnLastPassengerLeft(Unit *passenger) override
    {
        if (getCreature()->getSummonedByGuid() == passenger->getGuid())
            getCreature()->Despawn(1 * 1000, 0);
    }
};

void SetupPetAISpells(ScriptMgr* mgr)
{
    mgr->register_creature_script(24207, &ArmyOfTheDeadGhoulAI::Create);
    mgr->register_creature_script(19668, &ShadowFiendAI::Create);
    mgr->register_creature_script(27893, &DancingRuneWeaponAI::Create);
    mgr->register_creature_script(31216, &MirrorImageAI::Create);
    mgr->register_creature_script(28670, &FrostBroodVanquisherAI::Create);
}
