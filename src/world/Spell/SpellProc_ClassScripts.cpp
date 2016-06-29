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
// Warrior ProcScripts
class DamageShieldSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(DamageShieldSpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        // Allow only proc for player unit
        if (!mTarget->IsPlayer())
            return false;
        return true;
    }

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        Player* plr = static_cast<Player*>(mTarget);

        dmg_overwrite[0] = plr->GetBlockDamageReduction() * (mOrigSpell->EffectBasePoints[0] + 1) / 100;

        // plr->GetBlockDamageReduction() returns ZERO if player has no shield equipped
        if (dmg_overwrite[0] == 0)
            return true;

        return false;
    }
};

class JuggernautSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(JuggernautSpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (CastingSpell == NULL)
            return false;

        if (CastingSpell->custom_NameHash == SPELL_HASH_CHARGE)
            return true;
        else
            return false;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlock ProcScripts

//////////////////////////////////////////////////////////////////////////////////////////
// Shaman ProcScripts
class FrostBrandAttackSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(FrostBrandAttackSpellProc);

    void Init(Object* obj)
    {
        if (obj == NULL)
        {
            mDeleted = true;
            return;
        }

        mProcChance = static_cast< Item* >(obj)->GetItemProperties()->Delay * 9 / 600;
    }
};

class EarthShieldSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(EarthShieldSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        int32 value = mOrigSpell->EffectBasePoints[0];
        dmg_overwrite[0] = value;

        return false;
    }

    void CastSpell(Unit* victim, SpellEntry* CastingSpell, int* dmg_overwrite)
    {
        Unit* caster = mTarget->GetMapMgr()->GetUnit(mCaster);
        if (caster == NULL)
            return;

        Spell* spell = sSpellFactoryMgr.NewSpell(caster, mSpell, true, NULL);
        SpellCastTargets targets(mTarget->GetGUID());
        spell->prepare(&targets);
    }

};

class FlametongueWeaponSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(FlametongueWeaponSpellProc);

    void Init(Object* obj)
    {
        if (obj == NULL)
        {
            mDeleted = true;
            //initializing anyway all fields.
            mItemGUID = 0;
            damage = 0;
            return;
        }

        mItemGUID = obj->GetGUID();
        damage = 0;
        uint32 wp_speed;
        Item* item = static_cast< Item* >(obj);
        EnchantmentInstance* enchant = item->GetEnchantment(TEMP_ENCHANTMENT_SLOT);
        if (enchant != nullptr)
        {
            SpellEntry* sp = dbcSpell.LookupEntryForced(enchant->Enchantment->spell[0]);
            if (sp != nullptr && sp->custom_NameHash == SPELL_HASH_FLAMETONGUE_WEAPON__PASSIVE_)
            {
                wp_speed = item->GetItemProperties()->Delay;
                damage = (sp->EffectBasePoints[0] + 1) * wp_speed / 100000;
            }
        }
    }

    bool CanDelete(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0)//in this case misc is the item guid.
    {
        if (mSpell->Id == spellId && mCaster == casterGuid && misc == mItemGUID && !mDeleted)
            return true;

        return false;
    }

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (mTarget->IsPlayer())
            return true;
        return false;
    }

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        Item* item;

        if (weapon_damage_type == OFFHAND)
            item = static_cast< Player* >(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast< Player* >(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

        if (item != NULL && item->GetGUID() == mItemGUID)
        {
            dmg_overwrite[0] = damage;
            return false;
        }

        return true;
    }

private:
    uint64 mItemGUID;
    int damage;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rogue ProcScripts
class PoisonSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(PoisonSpellProc);

    PoisonSpellProc()
    {
        mItemGUID = 0;
        mProcPerMinute = 0;
    }

    void Init(Object* obj)
    {
        if (obj == NULL)
        {
            mDeleted = true;
            return;
        }

        mItemGUID = static_cast<Item*>(obj)->GetGUID();
        if (mProcPerMinute)
            mProcChance = static_cast<Item*>(obj)->GetItemProperties()->Delay * mProcPerMinute / 600;
    }

    bool CanDelete(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0)//in this case misc is the item guid.
    {
        if (mSpell->Id == spellId && mCaster == casterGuid && misc == mItemGUID && !mDeleted)
            return true;

        return false;
    }

    // Allow proc on ability cast (like eviscerate, envenom, fan of knives, rupture)
    bool CanProcOnTriggered(Unit* victim, SpellEntry* CastingSpell)
    {
        if (CastingSpell != NULL && (CastingSpell->SpellGroupType[0] & 0x120000 || CastingSpell->SpellGroupType[1] & 0x240008))
            return true;

        return false;
    }

    // Allow proc only if proccing hand is the one where poison was applied
    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        Item* item;

        if (weapon_damage_type == OFFHAND)
            item = static_cast<Player*>(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast<Player*>(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

        if (item != NULL && item->GetGUID() == mItemGUID)
            return false;

        return true;
    }

protected:
    uint64 mItemGUID;
    uint32 mProcPerMinute;
};

class WoundPoisonSpellProc : public PoisonSpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(WoundPoisonSpellProc);

    void Init(Object* obj)
    {
        mProcPerMinute = 21;

        PoisonSpellProc::Init(obj);
    }
};

class InstantPoisonSpellProc : public PoisonSpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(InstantPoisonSpellProc);

    void Init(Object* obj)
    {
        mProcPerMinute = 8;

        PoisonSpellProc::Init(obj);
    }
};

class CutToTheChaseSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(CutToTheChaseSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        Aura* aura = mTarget->FindAuraByNameHash(SPELL_HASH_SLICE_AND_DICE);
        if (aura)
        {
            // Duration of 5 combo maximum
            int32 dur = 21 * MSTIME_SECOND;

            SM_FIValue(mTarget->SM_FDur, &dur, aura->GetSpellProto()->SpellGroupType);
            SM_PIValue(mTarget->SM_PDur, &dur, aura->GetSpellProto()->SpellGroupType);

            // Set new aura's duration, reset event timer and set client visual aura
            aura->SetDuration(dur);
            sEventMgr.ModifyEventTimeLeft(aura, EVENT_AURA_REMOVE, aura->GetDuration());
            mTarget->ModVisualAuraStackCount(aura, 0);
        }

        return true;
    }
};

class DeadlyBrewSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(DeadlyBrewSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        mTarget->CastSpell(static_cast<Unit*>(NULL), 3409, true);    //Spell Id 3409: Crippling Poison

        return true;
    }
};

class WaylaySpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(WaylaySpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 0x204;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priest ProcScripts
class ImprovedSpiritTapSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(ImprovedSpiritTapSpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    uint32 CalcProcChance(Unit* victim, SpellEntry* CastingSpell)
    {
        if (CastingSpell == NULL)
            return 0;

        if (CastingSpell->custom_NameHash == SPELL_HASH_MIND_FLAY)
            return 50;

        if (CastingSpell->custom_NameHash == SPELL_HASH_MIND_BLAST || CastingSpell->custom_NameHash == SPELL_HASH_SHADOW_WORD__DEATH)
            return 100;

        return 0;
    }
};

class SpiritTapSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(SpiritTapSpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_GAIN_EXPIERIENCE;
    }
};

class HolyConcentrationSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(HolyConcentrationSpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_SPELL_CRIT_HIT;
        mProcClassMask[0] = 0x1800;
        mProcClassMask[1] = 0x4;
        mProcClassMask[2] = 0x1000;
    }
};

class DivineAegisSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(DivineAegisSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        if (CastingSpell == NULL)
            return true;

        if (!CastingSpell->HasEffect(SPELL_EFFECT_HEAL))
            return true;

        dmg_overwrite[0] = dmg * (mOrigSpell->EffectBasePoints[0] + 1) / 100;

        return false;
    }
};

class ImprovedDevouringPlagueSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(ImprovedDevouringPlagueSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // Get dmg amt for 1 tick
        dmg = CastingSpell->EffectBasePoints[0] + 1;

        // Get total ticks
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(CastingSpell->DurationIndex)) / CastingSpell->EffectAmplitude[0];

        dmg_overwrite[0] = dmg * ticks * (mOrigSpell->EffectBasePoints[0] + 1) / 100;

        return false;
    }
};

class VampiricEmbraceSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(VampiricEmbraceSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // Only proc for damaging shadow spells
        if (CastingSpell->School != SCHOOL_SHADOW || !IsDamagingSpell(CastingSpell))
            return true;

        // Only proc for single target spells
        if (!(HasTargetType(CastingSpell, EFF_TARGET_SINGLE_ENEMY) || HasTargetType(CastingSpell, EFF_TARGET_SELECTED_ENEMY_CHANNELED)))
            return true;

        dmg_overwrite[0] = dmg;
        dmg_overwrite[1] = dmg;

        return false;
    }
};

class VampiricTouchEnergizeSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(VampiricTouchEnergizeSpellProc);

    void Init(Object* obj)
    {
        mReplenishmentSpell = dbcSpell.LookupEntryForced(57669);
    }

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // Check for Mind Blast hit from this proc caster
        if (CastingSpell == NULL || CastingSpell->custom_NameHash != SPELL_HASH_MIND_BLAST || mCaster != victim->GetGUID())
            return true;

        // Cast Replenishment
        victim->CastSpell(victim, mReplenishmentSpell, true);

        return true;
    }

private:
    SpellEntry* mReplenishmentSpell;
};

class VampiricTouchDispelDamageSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(VampiricTouchDispelDamageSpellProc);

    void Init(Object* obj)
    {
        mDispelDmg = 8 * (mOrigSpell->EffectBasePoints[1] + 1);
    }

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // For PROC_ON_PRE_DISPELL_AURA_VICTIM, parameter dmg has aur->GetSpellId()
        SpellEntry* sp = dbcSpell.LookupEntryForced(dmg);

        if (CastingSpell == NULL || sp == NULL || sp->custom_NameHash != SPELL_HASH_VAMPIRIC_TOUCH)
            return true;

        dmg_overwrite[0] = mDispelDmg;

        return false;
    }

private:
    int32 mDispelDmg;
};

class EmpoweredRenewSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(EmpoweredRenewSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // Get heal amt for 1 tick
        dmg = CastingSpell->EffectBasePoints[0] + 1;

        // Get total ticks
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(CastingSpell->DurationIndex)) / CastingSpell->EffectAmplitude[0];

        // Total periodic effect is a single tick amount multiplied by number of ticks
        dmg_overwrite[0] = dmg * ticks * (mOrigSpell->EffectBasePoints[0] + 1) / 100;

        return false;
    }

    void CastSpell(Unit* victim, SpellEntry* CastingSpell, int* dmg_overwrite)
    {
        SpellCastTargets targets;
        targets.m_unitTarget = victim->GetGUID();

        Spell* spell = sSpellFactoryMgr.NewSpell(mTarget, mSpell, true, NULL);
        spell->forced_basepoints[0] = dmg_overwrite[0];
        spell->forced_basepoints[1] = dmg_overwrite[1];
        spell->forced_basepoints[2] = dmg_overwrite[2];
        spell->ProcedOnSpell = CastingSpell;

        spell->prepare(&targets);
    }
};

class ImprovedMindBlastSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(ImprovedMindBlastSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // If spell is not Mind Blast (by SpellGroupType) or player is not on shadowform, don't proc
        if (!(CastingSpell->SpellGroupType[0] & mProcClassMask[0] && mTarget->IsPlayer() && static_cast<Player*>(mTarget)->GetShapeShift() == FORM_SHADOW))
            return true;

        return false;
    }
};

class BodyAndSoulDummySpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(BodyAndSoulDummySpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (victim != NULL && mTarget->GetGUID() == victim->GetGUID())
            return true;

        return false;
    }
};

class BodyAndSoulSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(BodyAndSoulSpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 1;
    }
};

class MiserySpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(MiserySpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 0x8000;
        mProcClassMask[1] = 0x400;
        mProcClassMask[2] = 0x40;
    }
};

class PrayerOfMendingProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(PrayerOfMendingProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        Aura* aura = mTarget->FindAura(mSpell->Id);
        if (aura == NULL)
            return true;

        Unit* caster = static_cast<Player*>(aura->GetCaster());
        if (caster == NULL)
        {
            mTarget->RemoveAuraByNameHash(mSpell->custom_NameHash);
            return true;
        }

        int32 value = aura->GetModAmount(0);

        caster->CastSpell(mTarget, 33110, value, true);

        int32 count = mTarget->GetAuraStackCount(mSpell->Id);

        if (count <= 1)
            return true;

        Player* plr = static_cast<Player*>(mTarget);
        Group* grp = plr->GetGroup();

        if (grp == NULL)
            return true;

        Player* new_plr = grp->GetRandomPlayerInRangeButSkip(plr, 40.0f, plr);

        mTarget->RemoveAllAuraByNameHash(mSpell->custom_NameHash);

        if (new_plr != NULL)
            caster->CastSpell(new_plr, mSpell, value, count - 1, true);

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Paladin ProcScripts
class SealOfCommandSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(SealOfCommandSpellProc);

    void Init(Object* obj)
    {
        // default chance of proc
        mProcChance = 25;

        mProcFlags = PROC_ON_MELEE_ATTACK;

        /* The formula for SoC proc rate is: [ 7 / (60 / Weapon Speed) - from wowwiki */
        if (!mTarget->IsPlayer())
            return;

        uint32 weapspeed = 1;

        auto item = static_cast<Player*>(mTarget)->GetItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (item != nullptr)
            weapspeed = item->GetItemProperties()->Delay;

        mProcChance = 7 * weapspeed / 600;
        if (mProcChance >= 50)
            mProcChance = 50;
    }
};

class EyeForAnEyeSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(EyeForAnEyeSpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // If this player died by crit damage, don't do dmg back
        if (!mTarget->isAlive())
            return true;

        // Prevent proc on healing criticals
        if (CastingSpell != NULL && !(CastingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
            return true;

        dmg_overwrite[0] = dmg * (mOrigSpell->EffectBasePoints[0] + 1) / 100;

        int max_dmg = mTarget->GetMaxHealth() / 2;

        if (dmg_overwrite[0] > max_dmg)
            dmg_overwrite[0] = max_dmg;

        return false;
    }
};

class GraceOfTheNaaruSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(GraceOfTheNaaruSpellProc);

    void Init(Object* obj)
    {
        this->mProcClassMask[0] = 0x80000000;
    }
};

class SpiritualAttunementSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(SpiritualAttunementSpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (CastingSpell == NULL || !IsHealingSpell(CastingSpell))
            return false;

        return true;
    }
};

class PaladinSealsSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(PaladinSealsSpellProc);

    void Init(Object* obj)
    {
        this->mProcFlags = PROC_ON_MELEE_ATTACK;
    }
};

class SealOfCorruptionSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(SealOfCorruptionSpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (victim == NULL || victim->FindAuraCountByHash(SPELL_HASH_BLOOD_CORRUPTION) < 5)
            return false;

        return true;
    }
};

class SealOfVengeanceSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(SealOfVengeanceSpellProc);

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (victim == NULL || victim->FindAuraCountByHash(SPELL_HASH_HOLY_VENGEANCE) < 5)
            return false;

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mage ProcScripts
class HotStreakSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(HotStreakSpellProc);

    void Init(Object* obj)
    {
        mCritsInARow = 0;
    }

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        // Check for classmask. Should proc only if CastingSpell is one listed in http://www.wowhead.com/spell=44448
        if (!CheckClassMask(victim, CastingSpell))
            return true;

        // If was not a crit, reset counter and don't proc
        if (!(flag & PROC_ON_SPELL_CRIT_HIT))
        {
            mCritsInARow = 0;
            return true;
        }

        // If was not at least 2nd crit in a row, don't proc
        if (++mCritsInARow < 2)
            return true;

        return false;
    }

private:
    int mCritsInARow;
};

//////////////////////////////////////////////////////////////////////////////////////////
// DeathKnight ProcScripts
class ButcherySpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(ButcherySpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        dmg_overwrite[0] = mOrigSpell->EffectBasePoints[0] + 1;

        return false;
    }
};

class BladeBarrierSpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(BladeBarrierSpellProc);

    void Init(Object* obj)
    {
        mProcFlags = PROC_ON_CAST_SPELL;

        mProcClassMask[0] = mOrigSpell->EffectSpellClassMask[0][0];
        mProcClassMask[1] = mOrigSpell->EffectSpellClassMask[0][1];
        mProcClassMask[2] = mOrigSpell->EffectSpellClassMask[0][2];

        dk = static_cast<DeathKnight*>(mTarget);
    }

    bool CanProc(Unit* victim, SpellEntry* CastingSpell)
    {
        if (dk->IsAllRunesOfTypeInUse(RUNE_BLOOD))
            return true;
        return false;
    }

    private:
    DeathKnight* dk;
};

class DeathRuneMasterySpellProc : public SpellProc
{
    SPELL_PROC_FACTORY_FUNCTION(DeathRuneMasterySpellProc);

    bool DoEffect(Unit* victim, SpellEntry* CastingSpell, uint32 flag, uint32 dmg, uint32 abs, int* dmg_overwrite, uint32 weapon_damage_type)
    {
        DeathKnight* dk = static_cast<DeathKnight*>(mTarget);

        if (dk->GetRuneType(dk->GetLastUsedUnitSlot()) == RUNE_DEATH)
            return true;

        uint8 count = 2;
        for (uint8 x = 0; x < MAX_RUNES && count; ++x)
            if ((dk->GetRuneType(x) == RUNE_FROST || dk->GetRuneType(x) == RUNE_UNHOLY) && !dk->GetRuneIsUsed(x))
            {
                dk->ConvertRune(x, RUNE_DEATH);
                --count;
            }

        return true;
    }
};

void SpellProcMgr::SetupSpellProcClassScripts()
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Warrior
    // Add where spellIconID = 3214
    AddById(58872, &DamageShieldSpellProc::Create);     // Rank 1
    AddById(58874, &DamageShieldSpellProc::Create);     // Rank 2

    AddById(65156, &JuggernautSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warlock

    //////////////////////////////////////////////////////////////////////////////////////////
    // Shaman
    AddByNameHash(SPELL_HASH_FROSTBRAND_ATTACK, &FrostBrandAttackSpellProc::Create);

    AddById(10444, &FlametongueWeaponSpellProc::Create);
    AddById(379, &EarthShieldSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Rogue
    AddByNameHash(SPELL_HASH_WOUND_POISON_VII, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON_VI, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON_V, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON_IV, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON_III, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON_II, &WoundPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_WOUND_POISON, &WoundPoisonSpellProc::Create);

    AddByNameHash(SPELL_HASH_INSTANT_POISON_IX, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_VIII, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_VII, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_VI, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_V, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_IV, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_III, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON_II, &InstantPoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_INSTANT_POISON, &InstantPoisonSpellProc::Create);

    AddByNameHash(SPELL_HASH_DEADLY_POISON_IX, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_VIII, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_VII, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_VI, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_V, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_IV, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_III, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON_II, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_POISON, &PoisonSpellProc::Create);

    AddByNameHash(SPELL_HASH_CRIPPLING_POISON, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_MIND_NUMBING_POISON, &PoisonSpellProc::Create);
    AddByNameHash(SPELL_HASH_CUT_TO_THE_CHASE, &CutToTheChaseSpellProc::Create);
    AddByNameHash(SPELL_HASH_DEADLY_BREW, &DeadlyBrewSpellProc::Create);
    AddByNameHash(SPELL_HASH_WAYLAY, &WaylaySpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Priest
    AddByNameHash(SPELL_HASH_IMPROVED_SPIRIT_TAP, &ImprovedSpiritTapSpellProc::Create);
    AddByNameHash(SPELL_HASH_HOLY_CONCENTRATION, &HolyConcentrationSpellProc::Create);
    AddByNameHash(SPELL_HASH_DIVINE_AEGIS, &DivineAegisSpellProc::Create);
    AddByNameHash(SPELL_HASH_IMPROVED_DEVOURING_PLAGUE, &ImprovedDevouringPlagueSpellProc::Create);
    AddByNameHash(SPELL_HASH_VAMPIRIC_EMBRACE, &VampiricEmbraceSpellProc::Create);
    AddByNameHash(SPELL_HASH_EMPOWERED_RENEW, &EmpoweredRenewSpellProc::Create);
    AddByNameHash(SPELL_HASH_MISERY, &MiserySpellProc::Create);
    AddByNameHash(SPELL_HASH_PRAYER_OF_MENDING, &PrayerOfMendingProc::Create);
    AddByNameHash(SPELL_HASH_SPIRIT_TAP, &SpiritTapSpellProc::Create);

    AddById(34919, &VampiricTouchEnergizeSpellProc::Create);
    AddById(64085, &VampiricTouchDispelDamageSpellProc::Create);
    AddById(48301, &ImprovedMindBlastSpellProc::Create);

    AddById(64128, &BodyAndSoulSpellProc::Create);
    AddById(65081, &BodyAndSoulSpellProc::Create);
    AddById(64134, &BodyAndSoulDummySpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Paladin
    AddByNameHash(SPELL_HASH_SEAL_OF_COMMAND, &SealOfCommandSpellProc::Create);
    AddByNameHash(SPELL_HASH_EYE_FOR_AN_EYE, &EyeForAnEyeSpellProc::Create);
    AddByNameHash(SPELL_HASH_GRACE_OF_THE_NAARU, &GraceOfTheNaaruSpellProc::Create);
    AddByNameHash(SPELL_HASH_SPIRITUAL_ATTUNEMENT, &SpiritualAttunementSpellProc::Create);

    AddById(20167, &PaladinSealsSpellProc::Create);
    AddById(20168, &PaladinSealsSpellProc::Create);
    AddById(20170, &PaladinSealsSpellProc::Create);
    AddById(53739, &SealOfCorruptionSpellProc::Create);
    AddById(42463, &SealOfVengeanceSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Mage
    AddById(48108, &HotStreakSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // DeathKnight
    AddById(50163, &ButcherySpellProc::Create);
    AddById(50806, &DeathRuneMasterySpellProc::Create);

    AddByNameHash(SPELL_HASH_BLADE_BARRIER, &BladeBarrierSpellProc::Create);
}
