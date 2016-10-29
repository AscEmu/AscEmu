/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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
 *
 */

#include "StdAfx.h"

 //////////////////////////////////////////////////////////////////////////////////////////
 // Mage Scripts
class FirestarterTalent : public Spell
{
    SPELL_FACTORY_FUNCTION(FirestarterTalent);

    void DoAfterHandleEffect(Unit* target, uint32 i)
    {
        if (p_caster != NULL && target != NULL && p_caster->HasAura(54741)) // Cronicman: Player has "Firestarter" aura so we remove it AFTER casting Flamestrike.
        {
            p_caster->RemoveAllAuraById(54741);
        }
    }
};

class MissileBarrage : public Spell
{
    SPELL_FACTORY_FUNCTION(MissileBarrage);

    void DoAfterHandleEffect(Unit* target, uint32 i)
    {
        if (p_caster != NULL && target != NULL && p_caster->HasAura(44401)) // Player has "Missile Barrage" aura so we remove it AFTER casting arcane missles.
        {
            p_caster->RemoveAllAuraById(44401);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warrior Scripts

//////////////////////////////////////////////////////////////////////////////////////////
// Warlock Scripts

//////////////////////////////////////////////////////////////////////////////////////////
// Shaman Scripts
class FireNova : public Spell
{
    SPELL_FACTORY_FUNCTION(FireNova);
    bool HasFireTotem = false;
    uint8 CanCast(bool tolerate)
    {
        uint8 result = Spell::CanCast(tolerate);

        if (result == SPELL_CANCAST_OK)
        {
            if (u_caster)
            {
                // If someone has a better solutionen, your welcome :-)
                int totem_ids[32] = {
                    //Searing Totems
                    2523, 3902, 3903, 3904, 7400, 7402, 15480, 31162, 31164, 31165,
                    //Magma Totems
                    8929, 7464, 7435, 7466, 15484, 31166, 31167,
                    //Fire Elementel
                    15439,
                    //Flametongue Totems
                    5950, 6012, 7423, 10557, 15485, 31132, 31158, 31133,
                    //Frost Resistance Totems
                    5926, 7412, 7413, 15486, 31171, 31172
                };
                Unit* totem;
                for (uint8 i = 0; i < 32; i++)
                {
                    totem = u_caster->summonhandler.GetSummonWithEntry(totem_ids[i]);   // Get possible firetotem
                    if (totem != NULL)
                    {
                        HasFireTotem = true;
                        CastSpell(totem);
                    }
                }
                if (!HasFireTotem)
                {
                    SetExtraCastResult(SPELL_EXTRA_ERROR_MUST_HAVE_FIRE_TOTEM);
                    result = SPELL_FAILED_CUSTOM_ERROR;
                }
            }
        }
        return result;
    }

    void CastSpell(Unit* totem)
    {
        uint32 fireNovaSpells = Spell::GetSpellInfo()->Id;
        //Cast spell. NOTICE All ranks are linked with a extra spell in HackFixes.cpp
        totem->CastSpellAoF(totem->GetPositionX(), totem->GetPositionY(), totem->GetPositionZ(), sSpellCustomizations.GetSpellInfo(fireNovaSpells), true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rogue Scripts
class CheatDeathAura : public AbsorbAura
{
public:

    static Aura* Create(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL) { return new CheatDeathAura(proto, duration, caster, target, temporary, i_caster); }

    CheatDeathAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster)
    {
        dSpell = sSpellCustomizations.GetSpellInfo(31231);
    }

    uint32 AbsorbDamage(uint32 School, uint32* dmg)
    {
        // Checking for 1 min cooldown
        if (dSpell == NULL || !p_target->Cooldown_CanCast(dSpell))
            return 0;

        // Check for proc chance
        if (RandomFloat(100.0f) > GetSpellInfo()->EffectBasePoints[0] + 1)
            return 0;

        // Check if damage will kill player.
        uint32 cur_hlth = p_target->GetHealth();
        if ((*dmg) < cur_hlth)
            return 0;

        uint32 max_hlth = p_target->GetMaxHealth();
        uint32 min_hlth = max_hlth / 10;

        /*
        looks like the following lines are not so good, we check and cast on spell id 31231_
        and adding the cooldown to it, but it looks like this spell is useless(all it's doing is_
        casting 45182, so we can do all this stuff on 45182 at first place), BUT_
        as long as proceeding cheat death is not so height (how many rogue at the same time_
        gonna get to this point?) so it's better to use it because we wont lose anything!!
        */
        p_target->CastSpell(p_target->GetGUID(), dSpell, true);

        // set dummy effect,
        // this spell is used to procced the post effect of cheat death later.
        // Move next line to SPELL::SpellEffectDummy ?!! well it's better in case of dbc changing!!
        p_target->CastSpell(p_target->GetGUID(), 45182, true);

        // Better to add custom cooldown procedure then fucking with entry, or not!!
        dSpell->RecoveryTime = 60000;
        p_target->Cooldown_Add(dSpell, NULL);

        // Calc abs and applying it
        uint32 real_dmg = (cur_hlth > min_hlth ? cur_hlth - min_hlth : 0);
        uint32 absorbed_dmg = *dmg - real_dmg;

        *dmg = real_dmg;
        return absorbed_dmg;
    }

private:

    SpellInfo* dSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priest Scripts
class DispersionSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(DispersionSpell);

    void DoAfterHandleEffect(Unit* target, uint32 i)
    {
        if (p_caster != NULL)
        {
            // Mana regeneration
            p_caster->CastSpell(target, 60069, true);
            // Remove snares and movement impairing effects and make player immune to them
            p_caster->CastSpell(target, 63230, true);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Druid Scripts
class InnervateSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(InnervateSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (p_caster != NULL && i == 0 && target != NULL)
            value = (uint32)(p_caster->GetBaseMana() * 0.225f);

        return value;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// DeathKnight Scripts
class BloodPlagueSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(BloodPlagueSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (p_caster != NULL && i == 0)
            value += (uint32)(p_caster->GetAP() * 0.055 * 1.15);

        return value;
    }
};

class IcyTouchSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(IcyTouchSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (p_caster != NULL && i == 0)
            value += (uint32)(p_caster->GetAP() * 0.1);

        return value;
    }
};

class FrostFeverSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(FrostFeverSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (p_caster != NULL && i == 0)
            value += (uint32)(p_caster->GetAP() * 0.055 * 1.15);

        return value;
    }
};

class BloodBoilSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(BloodBoilSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (p_caster != NULL && i == 0)
        {
            int32 ap = p_caster->GetAP();

            value += (uint32)(ap * 0.08);

            // Does additional damage if target has diseases (http://www.tankspot.com/forums/f14/48814-3-1-blood-boil-mechanics-tested.html)
            if (target != NULL && (target->HasAura(55078) || target->HasAura(55095)))
                value += (uint32)(ap * 0.015 + 95);
        }

        return value;
    }
};

class BloodStrikeSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(BloodStrikeSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (target != NULL)
        {
            uint32 count = target->GetAuraCountWithDispelType(DISPEL_DISEASE, m_caster->GetGUID());
            if (count)
                value += value * count * (GetSpellInfo()->EffectBasePoints[2] + 1) / 200;
        }

        return value;
    }

    void DoAfterHandleEffect(Unit* target, uint32 i)
    {
        if (p_caster == NULL || i != 1)
            return;

        Aura* aur = p_caster->FindAuraByNameHash(SPELL_HASH_SUDDEN_DOOM);

        if (aur == NULL)
            return;

        if (!Rand(aur->GetSpellInfo()->procChance))
            return;

        p_caster->CastSpell(target, 47632, false);
    }
};

class DeathCoilSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(DeathCoilSpell);

    uint8 CanCast(bool tolerate)
    {
        uint8 result = Spell::CanCast(tolerate);

        if (result == SPELL_CANCAST_OK)
        {
            if (m_caster != NULL && m_caster->IsInWorld())
            {
                Unit* target = m_caster->GetMapMgr()->GetUnit(m_targets.m_unitTarget);

                if (target == NULL || !(isAttackable(m_caster, target, false) || target->getRace() == RACE_UNDEAD))
                    result = SPELL_FAILED_BAD_TARGETS;
            }
        }

        return result;
    }
};

class RuneStrileSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(RuneStrileSpell);

    void HandleEffects(uint64 guid, uint32 i)
    {
        Spell::HandleEffects(guid, i);

        if (u_caster != NULL)
            u_caster->RemoveAura(56817);
    }
};

class AntiMagicShellAura : public AbsorbAura
{
    public:
    static Aura* Create(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL) { return new AntiMagicShellAura(proto, duration, caster, target, temporary, i_caster); }

    AntiMagicShellAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    int32 CalcAbsorbAmount()
    {
        Player* caster = GetPlayerCaster();
        if (caster != NULL)
            return caster->GetMaxHealth() * (GetSpellInfo()->EffectBasePoints[1] + 1) / 100;
        else
            return mod->m_amount;
    }

    int32 CalcPctDamage()
    {
        return GetSpellInfo()->EffectBasePoints[0] + 1;
    }
};

class SpellDeflectionAura : public AbsorbAura
{
    public:
    static Aura* Create(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL) { return new SpellDeflectionAura(proto, duration, caster, target, temporary, i_caster); }

    SpellDeflectionAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    uint32 AbsorbDamage(uint32 School, uint32* dmg)
    {
        uint32 mask = GetSchoolMask();
        if (!(mask & g_spellSchoolConversionTable[School]))
            return 0;

        Player* caster = GetPlayerCaster();
        if (caster == NULL)
            return 0;

        if (!Rand(caster->GetParryChance()))
            return 0;

        uint32 dmg_absorbed = *dmg * GetModAmount(0) / 100;
        *dmg -= dmg_absorbed;

        return dmg_absorbed;
    }
};

class BloodwormSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(BloodwormSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        return 2 + RandomUInt(2);
    }
};

class WillOfTheNecropolisAura : public AbsorbAura
{
    public:
    static Aura* Create(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL) { return new WillOfTheNecropolisAura(proto, duration, caster, target, temporary, i_caster); }

    WillOfTheNecropolisAura(SpellInfo* proto, int32 duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = NULL)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    uint32 AbsorbDamage(uint32 School, uint32* dmg)
    {
        Unit* caster = GetUnitCaster();
        if (caster == NULL)
            return 0;

        int health_pct = caster->GetHealthPct();
        uint32 cur_health = caster->GetHealth();
        uint32 max_health = caster->GetMaxHealth();
        uint32 new_health_pct = (cur_health - *dmg) * 100 / max_health;

        // "Damage that would take you below $s1% health or taken while you are at $s1% health is reduced by $52284s1%."
        if ((health_pct > 35 && new_health_pct < 35) || health_pct == 35)
        {
            uint32 dmg_absorbed = *dmg * (GetSpellInfo()->EffectBasePoints[0] + 1) / 100;
            *dmg -= dmg_absorbed;

            return dmg_absorbed;
        }
        else
            return 0;
    }
};

class VampiricBloodSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(VampiricBloodSpell);

    int32 DoCalculateEffect(uint32 i, Unit* target, int32 value)
    {
        if (i == 1 && p_caster != NULL)
            value = p_caster->GetMaxHealth() * (GetSpellInfo()->EffectBasePoints[i] + 1) / 100;

        return value;
    }
};

class HeartStrikeSpell : public Spell
{
    SPELL_FACTORY_FUNCTION(HeartStrikeSpell);

    void DoAfterHandleEffect(Unit* target, uint32 i)
    {
        if (p_caster == NULL || i != 1)
            return;

        Aura* aur = p_caster->FindAuraByNameHash(SPELL_HASH_SUDDEN_DOOM);

        if (aur == NULL)
            return;

        if (!Rand(aur->GetSpellInfo()->procChance))
            return;

        p_caster->CastSpell(target, 47632, false);
    }
};

void SpellFactoryMgr::SetupSpellClassScripts()
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Mage
    AddSpellById(2120, FirestarterTalent::Create);   //Rank 1
    AddSpellById(2121, FirestarterTalent::Create);   //Rank 2
    AddSpellById(8422, FirestarterTalent::Create);   //Rank 3
    AddSpellById(8423, FirestarterTalent::Create);   //Rank 4
    AddSpellById(10215, FirestarterTalent::Create);   //Rank 5
    AddSpellById(10216, FirestarterTalent::Create);   //Rank 6
    AddSpellById(27086, FirestarterTalent::Create);   //Rank 7
    AddSpellById(42925, FirestarterTalent::Create);   //Rank 8
    AddSpellById(42926, FirestarterTalent::Create);   //Rank 9

    AddSpellById(5143, MissileBarrage::Create);   //Rank 1
    AddSpellById(5144, MissileBarrage::Create);   //Rank 2
    AddSpellById(5145, MissileBarrage::Create);   //Rank 3
    AddSpellById(8416, MissileBarrage::Create);   //Rank 4
    AddSpellById(8417, MissileBarrage::Create);   //Rank 5
    AddSpellById(10211, MissileBarrage::Create);   //Rank 6
    AddSpellById(10212, MissileBarrage::Create);   //Rank 7
    AddSpellById(25345, MissileBarrage::Create);   //Rank 8
    AddSpellById(27075, MissileBarrage::Create);   //Rank 9
    AddSpellById(38699, MissileBarrage::Create);   //Rank 10
    AddSpellById(38704, MissileBarrage::Create);   //Rank 11
    AddSpellById(42843, MissileBarrage::Create);   //Rank 12
    AddSpellById(42846, MissileBarrage::Create);   //Rank 13

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warrior

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warlock

    //////////////////////////////////////////////////////////////////////////////////////////
    // Shaman
    AddSpellById(1535, FireNova::Create);   //Rank 1
    AddSpellById(8498, FireNova::Create);   //Rank 2
    AddSpellById(8499, FireNova::Create);   //Rank 3
    AddSpellById(11314, FireNova::Create);  //Rank 4
    AddSpellById(11315, FireNova::Create);  //Rank 5
    AddSpellById(25546, FireNova::Create);  //Rank 6
    AddSpellById(25547, FireNova::Create);  //Rank 7
    AddSpellById(61649, FireNova::Create);  //Rank 8
    AddSpellById(61657, FireNova::Create);  //Rank 9

    //////////////////////////////////////////////////////////////////////////////////////////
    // Rogue
    AddAuraById(31228, &CheatDeathAura::Create);   // Rank 1
    AddAuraById(31229, &CheatDeathAura::Create);   // Rank 2
    AddAuraById(31230, &CheatDeathAura::Create);   // Rank 3

    //////////////////////////////////////////////////////////////////////////////////////////
    // Priest
    AddSpellById(47585, &DispersionSpell::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Druid
    AddSpellByNameHash(SPELL_HASH_INNERVATE, &InnervateSpell::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // DeathKnight
    AddSpellById(55078, &BloodPlagueSpell::Create);
    AddSpellById(45477, &IcyTouchSpell::Create);
    AddSpellById(55095, &FrostFeverSpell::Create);

    AddSpellById(48721, &BloodBoilSpell::Create);   // Rank 1
    AddSpellById(49939, &BloodBoilSpell::Create);   // Rank 2
    AddSpellById(49940, &BloodBoilSpell::Create);   // Rank 3
    AddSpellById(49941, &BloodBoilSpell::Create);   // Rank 4

    AddSpellById(45902, &BloodStrikeSpell::Create);   // Rank 1
    AddSpellById(49926, &BloodStrikeSpell::Create);   // Rank 2
    AddSpellById(49927, &BloodStrikeSpell::Create);   // Rank 3
    AddSpellById(49928, &BloodStrikeSpell::Create);   // Rank 4
    AddSpellById(49929, &BloodStrikeSpell::Create);   // Rank 5
    AddSpellById(49930, &BloodStrikeSpell::Create);   // Rank 6

    AddSpellById(47541, &DeathCoilSpell::Create);   // Rank 1
    AddSpellById(49892, &DeathCoilSpell::Create);   // Rank 2
    AddSpellById(49893, &DeathCoilSpell::Create);   // Rank 3
    AddSpellById(49894, &DeathCoilSpell::Create);   // Rank 4
    AddSpellById(49895, &DeathCoilSpell::Create);   // Rank 5

    AddSpellById(56815, &RuneStrileSpell::Create);

    AddAuraById(48707, &AntiMagicShellAura::Create);

    AddAuraById(49145, &SpellDeflectionAura::Create);   // Rank 1
    AddAuraById(49495, &SpellDeflectionAura::Create);   // Rank 2
    AddAuraById(49497, &SpellDeflectionAura::Create);   // Rank 3

    AddSpellById(50452, &BloodwormSpell::Create);

    AddAuraById(52284, &WillOfTheNecropolisAura::Create);   // Rank 1
    AddAuraById(52285, &WillOfTheNecropolisAura::Create);   // Rank 1
    AddAuraById(52286, &WillOfTheNecropolisAura::Create);   // Rank 1

    AddSpellById(55233, &VampiricBloodSpell::Create);

    AddSpellById(55050, &HeartStrikeSpell::Create);   // Rank 1
    AddSpellById(55258, &HeartStrikeSpell::Create);   // Rank 2
    AddSpellById(55259, &HeartStrikeSpell::Create);   // Rank 3
    AddSpellById(55260, &HeartStrikeSpell::Create);   // Rank 4
    AddSpellById(55261, &HeartStrikeSpell::Create);   // Rank 5
    AddSpellById(55262, &HeartStrikeSpell::Create);   // Rank 6
}
