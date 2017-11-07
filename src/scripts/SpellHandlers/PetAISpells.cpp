/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
public:

    ADD_CREATURE_FACTORY_FUNCTION(ArmyOfTheDeadGhoulAI);
    ArmyOfTheDeadGhoulAI(Creature* c) : CreatureAIScript(c)
    {
        getCreature()->GetAIInterface()->m_canMove = false;
    }

    void OnLoad()
    {

        RegisterAIUpdateEvent(200);

        if (getCreature()->IsSummon())
        {
            Summon* s = static_cast<Summon*>(getCreature());

            float parent_bonus = s->GetOwner()->GetDamageDoneMod(SCHOOL_NORMAL) * 0.04f;

            s->SetMinDamage(s->GetMinDamage() + parent_bonus);
            s->SetMaxDamage(s->GetMaxDamage() + parent_bonus);
        }
    }

    void AIUpdate()
    {
        getCreature()->CastSpell(getCreature()->GetGUID(), 20480, false);
        RemoveAIUpdateEvent();
        getCreature()->GetAIInterface()->m_canMove = true;
    }
};

class ShadowFiendAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(ShadowFiendAI);
    ShadowFiendAI(Creature* c) : CreatureAIScript(c)
    {
    }

    void OnLoad()
    {
        if (getCreature()->IsPet())
        {
            Pet* s = static_cast<Pet*>(getCreature());
            Player* owner = s->GetPetOwner();

            float owner_bonus = static_cast<float>(owner->GetDamageDoneMod(SCHOOL_SHADOW) * 0.375f); // 37.5%
            s->BaseAttackType = SCHOOL_SHADOW; // Melee hits are supposed to do damage with the shadow school
            s->SetBaseAttackTime(MELEE, 1500); // Shadowfiend is supposed to do 10 attacks, sometimes it can be 11
            s->SetMinDamage(s->GetMinDamage() + owner_bonus);
            s->SetMaxDamage(s->GetMaxDamage() + owner_bonus);
            s->BaseDamage[0] += owner_bonus;
            s->BaseDamage[1] += owner_bonus;

            Unit* uTarget = s->GetMapMgr()->GetUnit(owner->GetTargetGUID());
            if ((uTarget != NULL) && isAttackable(owner, uTarget))
            {
                s->GetAIInterface()->AttackReaction(uTarget, 1);
                s->GetAIInterface()->setNextTarget(uTarget);
            }
        }
    }
};

class MirrorImageAI : public CreatureAIScript
{
public:

    ADD_CREATURE_FACTORY_FUNCTION(MirrorImageAI);
    MirrorImageAI(Creature* c) : CreatureAIScript(c)
    {
    }

    void OnLoad()
    {
        if (getCreature()->IsSummon())
        {
            Summon* s = static_cast<Summon*>(getCreature());
            Unit* owner = s->GetOwner();

            owner->CastSpell(getCreature(), 45204, true);   // clone me
            owner->CastSpell(getCreature(), 58838, true);   // inherit threat list

            // Mage mirror image spell
            if (getCreature()->GetCreatedBySpell() == 58833)
            {
                getCreature()->SetMaxHealth(2500);
                getCreature()->SetHealth(2500);
                getCreature()->SetMaxPower(POWER_TYPE_MANA, owner->GetMaxPower(POWER_TYPE_MANA));
                getCreature()->SetPower(POWER_TYPE_MANA, owner->GetPower(POWER_TYPE_MANA));

                DBC::Structures::SpellRangeEntry const* range = NULL;

                AI_Spell sp1;
                sp1.entryId = 59638;
                sp1.spell = sSpellCustomizations.GetSpellInfo(sp1.entryId);
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

                AI_Spell sp2;
                sp2.entryId = 59637;
                sp2.spell = sSpellCustomizations.GetSpellInfo(sp2.entryId);
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
public:

    ADD_CREATURE_FACTORY_FUNCTION(DancingRuneWeaponAI);
    DancingRuneWeaponAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        dpsCycle = 0;
        dpsSpell = 0;
    }

    void OnLoad()
    {
        getCreature()->SetDisplayId(getCreature()->GetCreatureProperties()->Female_DisplayID);
        getCreature()->SetBaseAttackTime(MELEE, 2000);

        if (getCreature()->IsSummon())
        {
            Summon* s = static_cast<Summon*>(getCreature());
            Unit* owner = s->GetOwner();

            if (owner->IsPlayer())
            {
                Player* pOwner = static_cast<Player*>(owner);
                Item* item = pOwner->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

                if (item != NULL)
                {
                    for (uint8 s = 0; s < 5; s++)
                    {
                        if (item->GetItemProperties()->Spells[s].Id == 0)
                            continue;

                        if (item->GetItemProperties()->Spells[s].Trigger == CHANCE_ON_HIT)
                            procSpell[s] = item->GetItemProperties()->Spells[s].Id;
                    }

                    s->SetEquippedItem(MELEE, item->GetEntry());
                    s->SetBaseAttackTime(MELEE, item->GetItemProperties()->Delay);
                }

                pOwner->SetPower(POWER_TYPE_RUNIC_POWER, 0);
            }

            s->SetMinDamage(float(owner->GetDamageDoneMod(SCHOOL_NORMAL)));
            s->SetMaxDamage(float(owner->GetDamageDoneMod(SCHOOL_NORMAL)));
        }
    }

    void OnCombatStart(Unit* mTarget)
    {
        RegisterAIUpdateEvent(getCreature()->GetBaseAttackTime(MELEE));
    }

    void OnCombatStop(Unit* mTarget)
    {
        RemoveAIUpdateEvent();
        dpsCycle = 0;
    }

    void AIUpdate()
    {
        Unit* curtarget = getCreature()->GetAIInterface()->getNextTarget();
        if (getCreature()->GetCurrentSpell() == NULL && curtarget)
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
            }
            dpsCycle++;
            if (dpsCycle > 11)
                dpsCycle = 0;

            SpellInfo* MyNextSpell = sSpellCustomizations.GetSpellInfo(dpsSpell);
            if (MyNextSpell != NULL)
                getCreature()->CastSpell(curtarget, MyNextSpell, true);

        }
    }

    void OnHit(Unit* mTarget, float fAmount)
    {
        for (uint8 p = 0; p < 5; p++)
        {
            if (procSpell[p] != 0)
            {
                SpellInfo* mProc = sSpellCustomizations.GetSpellInfo(procSpell[p]);
                if (!mProc)
                    return;
                int x = RandomUInt(99);
                uint32 proc = mProc->getProcChance();
                if (proc < 1)
                    proc = 10; // Got to be fair :P

                if ((uint32)x <= proc)
                {
                    Unit* Vic = mProc->custom_self_cast_only ? getCreature() : mTarget;
                    getCreature()->CastSpell(Vic, mProc, true);
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
public:

    ADD_CREATURE_FACTORY_FUNCTION(FrostBroodVanquisherAI);
    FrostBroodVanquisherAI(Creature* c) : CreatureAIScript(c)
    {
    }

    void OnLoad()
    {
        getCreature()->setByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    }

    void OnLastPassengerLeft(Unit *passenger)
    {
        if (getCreature()->GetSummonedByGUID() == passenger->GetGUID())
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
};
