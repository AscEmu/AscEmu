/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Management/Item.h"
#include "Management/ItemInterface.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Map/MapMgr.h"
#include "SpellMgr.h"
#include "SpellAuras.h"
#include "Definitions/ProcFlags.h"
#include "Definitions/SpellIsFlags.h"
#include "Definitions/SpellEffectTarget.h"
#include "SpellHelpers.h"

using ascemu::World::Spell::Helpers::spellModFlatIntValue;
using ascemu::World::Spell::Helpers::spellModPercentageIntValue;

//////////////////////////////////////////////////////////////////////////////////////////
// Warrior ProcScripts
class DamageShieldSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new DamageShieldSpellProc(); }

    bool CanProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/)
    {
        // Allow only proc for player unit
        if (!mTarget->isPlayer())
            return false;
        return true;
    }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        Player* plr = static_cast<Player*>(mTarget);

        dmgOverwrite[0] = plr->GetBlockDamageReduction() * (mOrigSpell->getEffectBasePoints(0) + 1) / 100;

        // plr->GetBlockDamageReduction() returns ZERO if player has no shield equipped
        if (dmgOverwrite[0] == 0)
            return true;

        return false;
    }
};

class JuggernautSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new JuggernautSpellProc(); }

    bool CanProc(Unit* /*victim*/, SpellInfo const* castingSpell)
    {
        if (castingSpell == nullptr)
            return false;

        switch (castingSpell->getId())
        {
            //SPELL_HASH_CHARGE
            case 100:
            case 6178:
            case 7370:
            case 11578:
            case 20508:
            case 22120:
            case 22911:
            case 24023:
            case 24193:
            case 24315:
            case 24408:
            case 25821:
            case 25999:
            case 26184:
            case 26185:
            case 26186:
            case 26202:
            case 28343:
            case 29320:
            case 29847:
            case 31426:
            case 31733:
            case 32323:
            case 33709:
            case 34846:
            case 35412:
            case 35570:
            case 35754:
            case 36058:
            case 36140:
            case 36509:
            case 37511:
            case 38461:
            case 39574:
            case 40602:
            case 41581:
            case 42003:
            case 43519:
            case 43651:
            case 43807:
            case 44357:
            case 44884:
            case 49758:
            case 50582:
            case 51492:
            case 51756:
            case 51842:
            case 52538:
            case 52577:
            case 52856:
            case 53148:
            case 54460:
            case 55317:
            case 55530:
            case 57627:
            case 58619:
            case 58991:
            case 59040:
            case 59611:
            case 60067:
            case 61685:
            case 62563:
            case 62613:
            case 62614:
            case 62874:
            case 62960:
            case 62961:
            case 62977:
            case 63003:
            case 63010:
            case 63661:
            case 63665:
            case 64591:
            case 64719:
            case 65927:
            case 66481:
            case 68282:
            case 68284:
            case 68301:
            case 68307:
            case 68321:
            case 68498:
            case 68501:
            case 68762:
            case 68763:
            case 68764:
            case 71553:
            case 74399:
                return true;
            default:
                return false;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Warlock ProcScripts

//////////////////////////////////////////////////////////////////////////////////////////
// Shaman ProcScripts
class FrostBrandAttackSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new FrostBrandAttackSpellProc(); }

    void Init(Object* obj)
    {
        if (obj == nullptr)
        {
            mDeleted = true;
            return;
        }

        mProcChance = static_cast< Item* >(obj)->getItemProperties()->Delay * 9 / 600;
    }
};

class EarthShieldSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new EarthShieldSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        int32 value = mOrigSpell->getEffectBasePoints(0);
        dmgOverwrite[0] = value;

        return false;
    }

    void CastSpell(Unit* /*victim*/, SpellInfo* /*castingSpell*/, int* /*dmgOverwrite*/)
    {
        Unit* caster = mTarget->GetMapMgr()->GetUnit(mCaster);
        if (caster == nullptr)
            return;

        Spell* spell = sSpellMgr.newSpell(caster, mSpell, true, nullptr);
        SpellCastTargets targets(mTarget->getGuid());
        spell->prepare(&targets);
    }

};

class FlametongueWeaponSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new FlametongueWeaponSpellProc(); }

    void Init(Object* obj)
    {
        if (obj == nullptr)
        {
            mDeleted = true;
            //initializing anyway all fields.
            mItemGUID = 0;
            damage = 0;
            return;
        }

        mItemGUID = obj->getGuid();
        damage = 0;
        uint32 wp_speed;
        Item* item = static_cast< Item* >(obj);
        EnchantmentInstance* enchant = item->GetEnchantment(TEMP_ENCHANTMENT_SLOT);
        if (enchant != nullptr)
        {
            SpellInfo const* sp = sSpellMgr.getSpellInfo(enchant->Enchantment->spell[0]);
            if (sp != nullptr)
            {
                switch (sp->getId())
                {
                    //SPELL_HASH_FLAMETONGUE_WEAPON__PASSIVE_
                    case 10400:
                    case 15567:
                    case 15568:
                    case 15569:
                    case 16311:
                    case 16312:
                    case 16313:
                    case 58784:
                    case 58791:
                    case 58792:
                    {
                        wp_speed = item->getItemProperties()->Delay;
                        damage = (sp->getEffectBasePoints(0) + 1) * wp_speed / 100000;
                    } break;
                }
            }
        }
    }

    bool CanDelete(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0)//in this case misc is the item guid.
    {
        if (mSpell->getId() == spellId && mCaster == casterGuid && misc == mItemGUID && !mDeleted)
            return true;

        return false;
    }

    bool CanProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/)
    {
        if (mTarget->isPlayer())
            return true;
        return false;
    }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*CastingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 weaponDamageType)
    {
        Item* item;

        if (weaponDamageType == OFFHAND)
            item = static_cast< Player* >(mTarget)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast< Player* >(mTarget)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

        if (item != nullptr && item->getGuid() == mItemGUID)
        {
            dmgOverwrite[0] = damage;
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
public:

    static SpellProc* Create() { return new PoisonSpellProc(); }

    PoisonSpellProc()
    {
        mItemGUID = 0;
        mProcPerMinute = 0;
    }

    void Init(Object* obj)
    {
        if (obj == nullptr)
        {
            mDeleted = true;
            return;
        }

        mItemGUID = static_cast<Item*>(obj)->getGuid();
        if (mProcPerMinute)
            mProcChance = static_cast<Item*>(obj)->getItemProperties()->Delay * mProcPerMinute / 600;
    }

    bool CanDelete(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0)//in this case misc is the item guid.
    {
        if (mSpell->getId() == spellId && mCaster == casterGuid && misc == mItemGUID && !mDeleted)
            return true;

        return false;
    }

    // Allow proc on ability cast (like eviscerate, envenom, fan of knives, rupture)
    bool CanProcOnTriggered(Unit* /*victim*/, SpellInfo const* castingSpell)
    {
        if (castingSpell != nullptr && (castingSpell->getSpellFamilyFlags(0) & 0x120000 || castingSpell->getSpellFamilyFlags(1) & 0x240008))
            return true;

        return false;
    }

    // Allow proc only if proccing hand is the one where poison was applied
    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 weaponDamageType)
    {
        Item* item;

        if (weaponDamageType == OFFHAND)
            item = static_cast<Player*>(mTarget)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast<Player*>(mTarget)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

        if (item != nullptr && item->getGuid() == mItemGUID)
            return false;

        return true;
    }

protected:
    uint64 mItemGUID;
    uint32 mProcPerMinute;
};

class WoundPoisonSpellProc : public PoisonSpellProc
{
public:

    static SpellProc* Create() { return new WoundPoisonSpellProc(); }

    void Init(Object* obj)
    {
        mProcPerMinute = 21;

        PoisonSpellProc::Init(obj);
    }
};

class InstantPoisonSpellProc : public PoisonSpellProc
{
public:

    static SpellProc* Create() { return new InstantPoisonSpellProc(); }

    void Init(Object* obj)
    {
        mProcPerMinute = 8;

        PoisonSpellProc::Init(obj);
    }
};

class CutToTheChaseSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new CutToTheChaseSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        uint32 sliceAndDice[] =
        {
            //SPELL_HASH_SLICE_AND_DICE
            5171,
            6434,
            6774,
            30470,
            43547,
            60847,
            0
        };

        Aura* aura = mTarget->getAuraWithId(sliceAndDice);
        if (aura)
        {
            // Duration of 5 combo maximum
            int32 dur = 21 * TimeVarsMs::Second;

            spellModFlatIntValue(mTarget->SM_FDur, &dur, aura->GetSpellInfo()->getSpellFamilyFlags());
            spellModPercentageIntValue(mTarget->SM_PDur, &dur, aura->GetSpellInfo()->getSpellFamilyFlags());

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
public:

    static SpellProc* Create() { return new DeadlyBrewSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        mTarget->castSpell(static_cast<Unit*>(nullptr), 3409, true);    //Spell Id 3409: Crippling Poison

        return true;
    }
};

class WaylaySpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new WaylaySpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 0x204;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priest ProcScripts
class ImprovedSpiritTapSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new ImprovedSpiritTapSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_SPELL_CRIT_HIT;
    }

    uint32 CalcProcChance(Unit* /*victim*/, SpellInfo const* castingSpell)
    {
        if (castingSpell == nullptr)
            return 0;

        switch (castingSpell->getId())
        {
            //SPELL_HASH_MIND_FLAY
            case 15407:
            case 16568:
            case 17165:
            case 17311:
            case 17312:
            case 17313:
            case 17314:
            case 18807:
            case 22919:
            case 23953:
            case 25387:
            case 26044:
            case 26143:
            case 28310:
            case 29407:
            case 29570:
            case 32417:
            case 35507:
            case 37276:
            case 37330:
            case 37621:
            case 38243:
            case 40842:
            case 42396:
            case 43512:
            case 46562:
            case 48155:
            case 48156:
            case 52586:
            case 54339:
            case 54805:
            case 57779:
            case 57941:
            case 58381:
            case 59367:
            case 59974:
            case 60006:
            case 60472:
            case 65488:
            case 68042:
            case 68043:
            case 68044:
                return 50;
            //SPELL_HASH_MIND_BLAST
            case 8092:
            case 8102:
            case 8103:
            case 8104:
            case 8105:
            case 8106:
            case 10945:
            case 10946:
            case 10947:
            case 13860:
            case 15587:
            case 17194:
            case 17287:
            case 20830:
            case 25372:
            case 25375:
            case 26048:
            case 31516:
            case 37531:
            case 38259:
            case 41374:
            case 48126:
            case 48127:
            case 52722:
            case 58850:
            case 60447:
            case 60453:
            case 60500:
            case 65492:
            case 68038:
            case 68039:
            case 68040:
            //SPELL_HASH_SHADOW_WORD__DEATH
            case 32379:
            case 32409:
            case 32996:
            case 41375:
            case 47697:
            case 48157:
            case 48158:
            case 51818:
            case 56920:
                return 100;
            default:
                return 0;
        }
    }
};

class SpiritTapSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SpiritTapSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_GAIN_EXPIERIENCE;
    }
};

class HolyConcentrationSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new HolyConcentrationSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_SPELL_CRIT_HIT;
        mProcClassMask[0] = 0x1800;
        mProcClassMask[1] = 0x4;
        mProcClassMask[2] = 0x1000;
    }
};

class DivineAegisSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new DivineAegisSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        if (castingSpell == nullptr)
            return true;

        if (!castingSpell->hasEffect(SPELL_EFFECT_HEAL))
            return true;

        dmgOverwrite[0] = dmg * (mOrigSpell->getEffectBasePoints(0) + 1) / 100;

        return false;
    }
};

class ImprovedDevouringPlagueSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new ImprovedDevouringPlagueSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        // Get dmg amt for 1 tick
        dmg = castingSpell->getEffectBasePoints(0) + 1;

        // Get total ticks
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(castingSpell->getDurationIndex())) / castingSpell->getEffectAmplitude(0);

        dmgOverwrite[0] = dmg * ticks * (mOrigSpell->getEffectBasePoints(0) + 1) / 100;

        return false;
    }
};

class VampiricEmbraceSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new VampiricEmbraceSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        // Only proc for damaging shadow spells
        if (castingSpell->getSchool() != SCHOOL_SHADOW || !castingSpell->isDamagingSpell())
            return true;

        // Only proc for single target spells
        if (!(castingSpell->hasTargetType(EFF_TARGET_SINGLE_ENEMY) || castingSpell->hasTargetType(EFF_TARGET_SELECTED_ENEMY_CHANNELED)))
            return true;

        dmgOverwrite[0] = dmg;
        dmgOverwrite[1] = dmg;

        return false;
    }
};

class VampiricTouchEnergizeSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new VampiricTouchEnergizeSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mReplenishmentSpell = sSpellMgr.getSpellInfo(57669);
    }

    bool DoEffect(Unit* victim, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        // Check for Mind Blast hit from this proc caster
        if (castingSpell == nullptr || mCaster != victim->getGuid())
            return true;

        switch (castingSpell->getId())
        {
            //SPELL_HASH_MIND_BLAST
            case 8092:
            case 8102:
            case 8103:
            case 8104:
            case 8105:
            case 8106:
            case 10945:
            case 10946:
            case 10947:
            case 13860:
            case 15587:
            case 17194:
            case 17287:
            case 20830:
            case 25372:
            case 25375:
            case 26048:
            case 31516:
            case 37531:
            case 38259:
            case 41374:
            case 48126:
            case 48127:
            case 52722:
            case 58850:
            case 60447:
            case 60453:
            case 60500:
            case 65492:
            case 68038:
            case 68039:
            case 68040:
                break;
            default:
                return true;
        }

        // Cast Replenishment
        victim->castSpell(victim, mReplenishmentSpell, true);

        return true;
    }

private:
    SpellInfo const* mReplenishmentSpell;
};

class VampiricTouchDispelDamageSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new VampiricTouchDispelDamageSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mDispelDmg = 8 * (mOrigSpell->getEffectBasePoints(1) + 1);
    }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        // For PROC_ON_PRE_DISPELL_AURA_VICTIM, parameter dmg has aur->GetSpellId()
        SpellInfo const* sp = sSpellMgr.getSpellInfo(dmg);

        if (castingSpell == nullptr || sp == nullptr)
            return true;

        switch (sp->getId())
        {
            //SPELL_HASH_VAMPIRIC_TOUCH
            case 34914:
            case 34916:
            case 34917:
            case 34919:
            case 48159:
            case 48160:
            case 52723:
            case 52724:
            case 60501:
            case 64085:
            case 65490:
            case 68091:
            case 68092:
            case 68093:
                break;
            default:
                return true;
        }

        dmgOverwrite[0] = mDispelDmg;

        return false;
    }

private:
    int32 mDispelDmg;
};

class EmpoweredRenewSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new EmpoweredRenewSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weapon_damage_type*/)
    {
        // Get heal amt for 1 tick
        dmg = castingSpell->getEffectBasePoints(0) + 1;

        // Get total ticks
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(castingSpell->getDurationIndex())) / castingSpell->getEffectAmplitude(0);

        // Total periodic effect is a single tick amount multiplied by number of ticks
        dmgOverwrite[0] = dmg * ticks * (mOrigSpell->getEffectBasePoints(0) + 1) / 100;

        return false;
    }

    void CastSpell(Unit* victim, SpellInfo* CastingSpell, int* dmg_overwrite)
    {
        SpellCastTargets targets;
        targets.m_unitTarget = victim->getGuid();

        Spell* spell = sSpellMgr.newSpell(mTarget, mSpell, true, nullptr);
        spell->forced_basepoints[0] = dmg_overwrite[0];
        spell->forced_basepoints[1] = dmg_overwrite[1];
        spell->forced_basepoints[2] = dmg_overwrite[2];
        spell->ProcedOnSpell = CastingSpell;

        spell->prepare(&targets);
    }
};

class ImprovedMindBlastSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new ImprovedMindBlastSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        // If spell is not Mind Blast (by SpellGroupType) or player is not on shadowform, don't proc
        if (!(castingSpell->getSpellFamilyFlags(0) & mProcClassMask[0] && mTarget->isPlayer() && static_cast<Player*>(mTarget)->getShapeShiftForm() == FORM_SHADOW))
            return true;

        return false;
    }
};

class BodyAndSoulDummySpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new BodyAndSoulDummySpellProc(); }

    bool CanProc(Unit* victim, SpellInfo const* /*castingSpell*/)
    {
        if (victim != nullptr && mTarget->getGuid() == victim->getGuid())
            return true;

        return false;
    }
};

class BodyAndSoulSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new BodyAndSoulSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 1;
    }
};

class MiserySpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new MiserySpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_CAST_SPELL;
        mProcClassMask[0] = 0x8000;
        mProcClassMask[1] = 0x400;
        mProcClassMask[2] = 0x40;
    }
};

class PrayerOfMendingProc : public SpellProc
{
public:

    static SpellProc* Create() { return new PrayerOfMendingProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        Aura* aura = mTarget->getAuraWithId(mSpell->getId());
        if (aura == nullptr)
            return true;

        Unit* caster = static_cast<Player*>(aura->GetCaster());
        if (caster == nullptr)
        {
            mTarget->removeAllAurasById(mSpell->getId());
            return true;
        }

        int32 value = aura->GetModAmount(0);

        caster->castSpell(mTarget, 33110, value, true);

        int32 count = mTarget->GetAuraStackCount(mSpell->getId());

        if (count <= 1)
            return true;

        Player* plr = static_cast<Player*>(mTarget);
        Group* grp = plr->GetGroup();

        if (grp == nullptr)
            return true;

        Player* new_plr = grp->GetRandomPlayerInRangeButSkip(plr, 40.0f, plr);

        mTarget->removeAllAurasById(mSpell->getId());

        if (new_plr != nullptr)
            caster->castSpell(new_plr, mSpell, value, count - 1, true);

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Paladin ProcScripts
class SealOfCommandSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SealOfCommandSpellProc(); }

    void Init(Object* /*obj*/)
    {
        // default chance of proc
        mProcChance = 25;

        mProcFlags = PROC_ON_MELEE_ATTACK;

        /* The formula for SoC proc rate is: [ 7 / (60 / Weapon Speed) - from wowwiki */
        if (!mTarget->isPlayer())
            return;

        uint32 weapspeed = 1;

        auto item = static_cast<Player*>(mTarget)->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (item != nullptr)
            weapspeed = item->getItemProperties()->Delay;

        mProcChance = 7 * weapspeed / 600;
        if (mProcChance >= 50)
            mProcChance = 50;
    }
};

class EyeForAnEyeSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new EyeForAnEyeSpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 dmg, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weapon_damage_type*/)
    {
        // If this player died by crit damage, don't do dmg back
        if (!mTarget->isAlive())
            return true;

        // Prevent proc on healing criticals
        if (castingSpell != nullptr && !(castingSpell->custom_c_is_flags & SPELL_FLAG_IS_DAMAGING))
            return true;

        dmgOverwrite[0] = dmg * (mOrigSpell->getEffectBasePoints(0) + 1) / 100;

        int max_dmg = mTarget->getMaxHealth() / 2;

        if (dmgOverwrite[0] > max_dmg)
            dmgOverwrite[0] = max_dmg;

        return false;
    }
};

class GraceOfTheNaaruSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new GraceOfTheNaaruSpellProc(); }

    void Init(Object* /*obj*/)
    {
        this->mProcClassMask[0] = 0x80000000;
    }
};

class SpiritualAttunementSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SpiritualAttunementSpellProc(); }

    bool CanProc(Unit* /*victim*/, SpellInfo const* castingSpell)
    {
        if (castingSpell == nullptr || !castingSpell->isHealingSpell())
            return false;

        return true;
    }
};

class PaladinSealsSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new PaladinSealsSpellProc(); }

    void Init(Object* /*obj*/)
    {
        this->mProcFlags = PROC_ON_MELEE_ATTACK;
    }
};

class SealOfCorruptionSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SealOfCorruptionSpellProc(); }

    bool CanProc(Unit* victim, SpellInfo const* /*castingSpell*/)
    {
        if (victim == nullptr || victim->getAuraCountForId(53742) < 5)
            return false;

        return true;
    }
};

class SealOfVengeanceSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SealOfVengeanceSpellProc(); }

    bool CanProc(Unit* victim, SpellInfo const* /*castingSpell*/)
    {
        if (victim == nullptr || victim->getAuraCountForId(31803) < 5)
            return false;

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mage ProcScripts
class HotStreakSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new HotStreakSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mCritsInARow = 0;
    }

    bool DoEffect(Unit* victim, SpellInfo const* castingSpell, uint32 flag, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
    {
        // Check for classmask. Should proc only if CastingSpell is one listed in http://www.wowhead.com/spell=44448
        if (!CheckClassMask(victim, castingSpell))
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
public:

    static SpellProc* Create() { return new ButcherySpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/)
    {
        dmgOverwrite[0] = mOrigSpell->getEffectBasePoints(0) + 1;

        return false;
    }
};

class BladeBarrierSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new BladeBarrierSpellProc(); }

    void Init(Object* /*obj*/)
    {
        mProcFlags = PROC_ON_CAST_SPELL;

        mProcClassMask[0] = mOrigSpell->getEffectSpellClassMask(0, 0);
        mProcClassMask[1] = mOrigSpell->getEffectSpellClassMask(0, 1);
        mProcClassMask[2] = mOrigSpell->getEffectSpellClassMask(0, 2);

        dk = static_cast<DeathKnight*>(mTarget);
    }

    bool CanProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/)
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
public:

    static SpellProc* Create() { return new DeathRuneMasterySpellProc(); }

    bool DoEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/)
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
    // SPELL_HASH_FROSTBRAND_ATTACK
    uint32 frostbrandAttack[] =
    {
        8034,
        8037,
        10458,
        16352,
        16353,
        25501,
        38617,
        54609,
        58797,
        58798,
        58799,
        64186,
        0
    };

    AddById(frostbrandAttack, &FrostBrandAttackSpellProc::Create);

    AddById(10444, &FlametongueWeaponSpellProc::Create);
    AddById(379, &EarthShieldSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Rogue
    uint32 woundPoison[] =
    {
        // SPELL_HASH_WOUND_POISON_VII
        57975,
        57978,
        // SPELL_HASH_WOUND_POISON_VI
        57974,
        57977,
        // SPELL_HASH_WOUND_POISON_V
        27188,
        27189,
        // SPELL_HASH_WOUND_POISON_IV
        13224,
        13227,
        // SPELL_HASH_WOUND_POISON_III
        13223,
        13226,
        // SPELL_HASH_WOUND_POISON_II
        13222,
        13225,
        // SPELL_HASH_WOUND_POISON
        13218,
        13219,
        30984,
        36974,
        39665,
        43461,
        54074,
        65962,
        0
    };
    AddById(woundPoison, &WoundPoisonSpellProc::Create);

    uint32 instantPoison[] =
    {
        // SPELL_HASH_INSTANT_POISON_IX
        57965,
        57968,
        // SPELL_HASH_INSTANT_POISON_VIII
        57964,
        57967,
        // SPELL_HASH_INSTANT_POISON_VII
        26890,
        26891,
        // SPELL_HASH_INSTANT_POISON_VI
        11337,
        11340,
        // SPELL_HASH_INSTANT_POISON_V
        11336,
        11339,
        // SPELL_HASH_INSTANT_POISON_IV
        11335,
        11338,
        // SPELL_HASH_INSTANT_POISON_III
        8688,
        8689,
        // SPELL_HASH_INSTANT_POISON_II
        8685,
        8686,
        // SPELL_HASH_INSTANT_POISON
        8679,
        8680,
        28428,
        41189,
        59242,
        0
    };
    AddById(instantPoison, &InstantPoisonSpellProc::Create);

    uint32 deadlyPoison[] =
    {
        // SPELL_HASH_DEADLY_POISON_IX
        57970,
        57973,
        // SPELL_HASH_DEADLY_POISON_VIII
        57969,
        57972,
        // SPELL_HASH_DEADLY_POISON_VII
        27186,
        27187,
        // SPELL_HASH_DEADLY_POISON_VI
        26967,
        26968,
        // SPELL_HASH_DEADLY_POISON_V
        25349,
        25351,
        // SPELL_HASH_DEADLY_POISON_IV
        11354,
        11356,
        // SPELL_HASH_DEADLY_POISON_III
        11353,
        11355,
        // SPELL_HASH_DEADLY_POISON_II
        2819,
        2824,
        // SPELL_HASH_DEADLY_POISON
        2818,
        2823,
        3583,
        10022,
        13582,
        21787,
        21788,
        32970,
        32971,
        34616,
        34655,
        34657,
        36872,
        38519,
        38520,
        41191,
        41192,
        41485,
        43580,
        43581,
        56145,
        56149,
        59479,
        59482,
        63755,
        63756,
        67710,
        67711,
        68315,
        72329,
        72330,
        0
    };
    AddById(deadlyPoison, &PoisonSpellProc::Create);

    uint32 cripplingPoison[] =
    {
        //SPELL_HASH_CRIPPLING_POISON
        3408,
        3409,
        25809,
        30981,
        44289,
        0
    };
    AddById(cripplingPoison, &PoisonSpellProc::Create);

    uint32 mindNumbingPoison[] =
    {
        //SPELL_HASH_MIND_NUMBING_POISON
        5760,
        5761,
        25810,
        34615,
        41190,
        0
    };
    AddById(mindNumbingPoison, &PoisonSpellProc::Create);

    uint32 cutToTheChase[] =
    {
        //SPELL_HASH_CUT_TO_THE_CHASE
        51664,
        51665,
        51667,
        51668,
        51669,
        0
    };
    AddById(cutToTheChase, &CutToTheChaseSpellProc::Create);

    uint32 deadlyBrew[] =
    {
        //SPELL_HASH_DEADLY_BREW
        51625,
        51626,
        0
    };
    AddById(deadlyBrew, &DeadlyBrewSpellProc::Create);

    uint32 waylay[] =
    {
        //SPELL_HASH_WAYLAY
        51692,
        51693,
        51696,
        0
    };
    AddById(waylay, &WaylaySpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Priest
    uint32 improvedSpiritTap[] =
    {
        //SPELL_HASH_IMPROVED_SPIRIT_TAP
        15337,
        15338,
        49694,
        59000,
        0
    };
    AddById(improvedSpiritTap, &ImprovedSpiritTapSpellProc::Create);

    uint32 holyConcentration[] =
    {
        //SPELL_HASH_HOLY_CONCENTRATION
        34753,
        34754,
        34859,
        34860,
        63724,
        63725,
        0
    };
    AddById(holyConcentration, &HolyConcentrationSpellProc::Create);

    uint32 divineAegis[] =
    {
        //SPELL_HASH_DIVINE_AEGIS
        47509,
        47511,
        47515,
        47753,
        54704,
        0
    };
    AddById(divineAegis, &DivineAegisSpellProc::Create);

    uint32 improvedDevouringPlague[] =
    {
        //SPELL_HASH_IMPROVED_DEVOURING_PLAGUE
        63625,
        63626,
        63627,
        63675,
        75999,
        0
    };
    AddById(improvedDevouringPlague, &ImprovedDevouringPlagueSpellProc::Create);

    uint32 vampiricEmbrace[] =
    {
        //SPELL_HASH_VAMPIRIC_EMBRACE
        15286,
        15290,
        71269,
        0
    };
    AddById(vampiricEmbrace, &VampiricEmbraceSpellProc::Create);

    uint32 empoweredRenew[] =
    {
        //SPELL_HASH_EMPOWERED_RENEW
        63534,
        63542,
        63543,
        63544,
        0
    };
    AddById(empoweredRenew, &EmpoweredRenewSpellProc::Create);

    uint32 misery[] =
    {
        //SPELL_HASH_MISERY
        33191,
        33192,
        33193,
        33196,
        33197,
        33198,
        0
    };
    AddById(misery, &MiserySpellProc::Create);

    uint32 prayerOfMending[] =
    {
        //SPELL_HASH_PRAYER_OF_MENDING
        33076,
        33110,
        41635,
        41637,
        44583,
        44586,
        46045,
        48110,
        48111,
        48112,
        48113,
        0
    };
    AddById(prayerOfMending, &PrayerOfMendingProc::Create);

    uint32 spiritTap[] =
    {
        //SPELL_HASH_SPIRIT_TAP
        15270,
        15271,
        15335,
        15336,
        0
    };
    AddById(spiritTap, &SpiritTapSpellProc::Create);

    AddById(34919, &VampiricTouchEnergizeSpellProc::Create);
    AddById(64085, &VampiricTouchDispelDamageSpellProc::Create);
    AddById(48301, &ImprovedMindBlastSpellProc::Create);

    AddById(64128, &BodyAndSoulSpellProc::Create);
    AddById(65081, &BodyAndSoulSpellProc::Create);
    AddById(64134, &BodyAndSoulDummySpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Paladin
    uint32 sealOfCommand[] =
    {
        //SPELL_HASH_SEAL_OF_COMMAND
        20375,
        20424,
        29385,
        33127,
        41469,
        42058,
        57769,
        57770,
        66004,
        68020,
        68021,
        68022,
        69403,
        0
    };
    AddById(sealOfCommand, &SealOfCommandSpellProc::Create);

    uint32 eyeForAnEye[] =
    {
        //SPELL_HASH_EYE_FOR_AN_EYE
        9799,
        25988,
        25997,
        0
    };
    AddById(eyeForAnEye, &EyeForAnEyeSpellProc::Create);

    AddById(43742, &GraceOfTheNaaruSpellProc::Create);

    uint32 spiritualAttunement[] =
    {
        //SPELL_HASH_SPIRITUAL_ATTUNEMENT
        31785,
        31786,
        33776,
        0
    };
    AddById(spiritualAttunement, &SpiritualAttunementSpellProc::Create);

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

    uint32 bladeBarrier[] =
    {
        //SPELL_HASH_BLADE_BARRIER
        49182,
        49500,
        49501,
        51789,
        55225,
        55226,
        64855,
        64856,
        64858,
        64859,
        0
    };
    AddById(bladeBarrier, &BladeBarrierSpellProc::Create);
}
