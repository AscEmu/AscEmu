/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
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

//////////////////////////////////////////////////////////////////////////////////////////
// Warrior ProcScripts
class DamageShieldSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new DamageShieldSpellProc(); }

    bool canProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        // Allow only proc for player unit
        if (!getProcOwner()->isPlayer())
            return false;
        return true;
    }

    bool doEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/) override
    {
        Player* plr = static_cast<Player*>(getProcOwner());

        dmgOverwrite[0] = plr->GetBlockDamageReduction() * (getOriginalSpell()->calculateEffectValue(0)) / 100;

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

    bool canProc(Unit* /*victim*/, SpellInfo const* castingSpell) override
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

    void init(Object* obj) override
    {
        if (obj == nullptr)
        {
            deleteProc();
            return;
        }

        setProcChance(static_cast< Item* >(obj)->getItemProperties()->Delay * 9 / 600);
    }
};

class FlametongueWeaponSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new FlametongueWeaponSpellProc(); }

    void init(Object* obj) override
    {
        if (obj == nullptr)
        {
            deleteProc();
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
                        damage = (sp->calculateEffectValue(0)) * wp_speed / 100000;
                    } break;
                }
            }
        }
    }

    bool canDeleteProc(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0) override //in this case misc is the item guid.
    {
        if (getSpell()->getId() == spellId && getCasterGuid() == casterGuid && misc == mItemGUID && !isDeleted())
            return true;

        return false;
    }

    bool canProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
    {
        if (getProcOwner()->isPlayer())
            return true;
        return false;
    }

    bool doEffect(Unit* /*victim*/, SpellInfo const* /*CastingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 weaponDamageType) override
    {
        Item* item;

        if (weaponDamageType == OFFHAND)
            item = static_cast< Player* >(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast< Player* >(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

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

    void init(Object* obj) override
    {
        if (obj == nullptr)
        {
            deleteProc();
            return;
        }

        mItemGUID = static_cast<Item*>(obj)->getGuid();
        if (mProcPerMinute)
            setProcChance(static_cast<Item*>(obj)->getItemProperties()->Delay * mProcPerMinute / 600);
    }

    bool canDeleteProc(uint32 spellId, uint64 casterGuid = 0, uint64 misc = 0) override//in this case misc is the item guid.
    {
        if (getSpell()->getId() == spellId && getCasterGuid() == casterGuid && misc == mItemGUID && !isDeleted())
            return true;

        return false;
    }

    // Allow proc on ability cast (like eviscerate, envenom, fan of knives, rupture)
    bool canProcOnTriggered(Unit* /*victim*/, SpellInfo const* castingSpell)
    {
        if (castingSpell != nullptr && (castingSpell->getSpellFamilyFlags(0) & 0x120000 || castingSpell->getSpellFamilyFlags(1) & 0x240008))
            return true;

        return false;
    }

    // Allow proc only if proccing hand is the one where poison was applied
    bool doEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 weaponDamageType) override
    {
        Item* item;

        if (weaponDamageType == OFFHAND)
            item = static_cast<Player*>(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_OFFHAND);
        else
            item = static_cast<Player*>(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);

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

    void init(Object* obj) override
    {
        mProcPerMinute = 21;

        PoisonSpellProc::init(obj);
    }
};

class InstantPoisonSpellProc : public PoisonSpellProc
{
public:

    static SpellProc* Create() { return new InstantPoisonSpellProc(); }

    void init(Object* obj) override
    {
        mProcPerMinute = 8;

        PoisonSpellProc::init(obj);
    }
};

class WaylaySpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new WaylaySpellProc(); }

    void init(Object* /*obj*/) override
    {
        setProcFlags(PROC_ON_DONE_MELEE_SPELL_HIT);
        setProcClassMask(0, 0x204);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priest ProcScripts
class SpiritTapSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SpiritTapSpellProc(); }

    void init(Object* /*obj*/) override
    {
        setProcFlags(PROC_ON_KILL);
    }
};

class ImprovedDevouringPlagueSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new ImprovedDevouringPlagueSpellProc(); }

    bool doEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weaponDamageType*/) override
    {
        // Get dmg amt for 1 tick
        const uint32_t dmg = castingSpell->calculateEffectValue(0);

        // Get total ticks
        auto amplitude = castingSpell->getEffectAmplitude(0) == 0 ? 1 : castingSpell->getEffectAmplitude(0);
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(castingSpell->getDurationIndex())) / amplitude;

        dmgOverwrite[0] = dmg * ticks * (getOriginalSpell()->calculateEffectValue(0)) / 100;

        return false;
    }
};

class EmpoweredRenewSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new EmpoweredRenewSpellProc(); }

    bool doEffect(Unit* /*victim*/, SpellInfo const* castingSpell, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* dmgOverwrite, uint32 /*weapon_damage_type*/) override
    {
        // Get heal amt for 1 tick
        const uint32_t dmg = castingSpell->calculateEffectValue(0);

        // Get total ticks
        int ticks = GetDuration(sSpellDurationStore.LookupEntry(castingSpell->getDurationIndex())) / castingSpell->getEffectAmplitude(0);

        // Total periodic effect is a single tick amount multiplied by number of ticks
        dmgOverwrite[0] = dmg * ticks * (getOriginalSpell()->calculateEffectValue(0)) / 100;

        return false;
    }

    void castSpell(Unit* victim, SpellInfo const* CastingSpell) override
    {
        SpellCastTargets targets(victim->getGuid());

        Spell* spell = sSpellMgr.newSpell(getProcOwner(), getSpell(), true, nullptr);
        spell->forced_basepoints[0] = getOverrideEffectDamage(0);
        spell->forced_basepoints[1] = getOverrideEffectDamage(1);
        spell->forced_basepoints[2] = getOverrideEffectDamage(2);
        spell->ProcedOnSpell = CastingSpell;

        spell->prepare(&targets);
    }
};

class MiserySpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new MiserySpellProc(); }

    void init(Object* /*obj*/) override
    {
        setProcFlags(PROC_ON_DONE_NEGATIVE_SPELL_DAMAGE_CLASS_MAGIC);
        setProcClassMask(0, 0x8000);
        setProcClassMask(1, 0x400);
        setProcClassMask(2, 0x40);
    }
};

class PrayerOfMendingProc : public SpellProc
{
public:

    static SpellProc* Create() { return new PrayerOfMendingProc(); }

    bool doEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/) override
    {
        Aura* aura = getProcOwner()->getAuraWithId(getSpell()->getId());
        if (aura == nullptr)
            return true;

        Unit* caster = static_cast<Player*>(aura->getCaster());
        if (caster == nullptr)
        {
            getProcOwner()->removeAllAurasById(getSpell()->getId());
            return true;
        }

        int32 value = aura->getEffectDamage(0);

        caster->castSpell(getProcOwner(), 33110, value, true);

        int32 count = getProcOwner()->GetAuraStackCount(getSpell()->getId());

        if (count <= 1)
            return true;

        Player* plr = static_cast<Player*>(getProcOwner());
        Group* grp = plr->getGroup();

        if (grp == nullptr)
            return true;

        Player* new_plr = grp->GetRandomPlayerInRangeButSkip(plr, 40.0f, plr);

        getProcOwner()->removeAllAurasById(getSpell()->getId());

        if (new_plr != nullptr)
            caster->castSpell(new_plr, getSpell(), value, count - 1, true);

        return true;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Paladin ProcScripts
class SealOfCommandSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SealOfCommandSpellProc(); }

    void init(Object* /*obj*/) override
    {
        // default chance of proc
        setProcChance(25);

        setProcFlags(PROC_ON_DONE_MELEE_HIT);

        /* The formula for SoC proc rate is: [ 7 / (60 / Weapon Speed) - from wowwiki */
        if (!getProcOwner()->isPlayer())
            return;

        uint32 weapspeed = 1;

        auto item = static_cast<Player*>(getProcOwner())->getItemInterface()->GetInventoryItem(EQUIPMENT_SLOT_MAINHAND);
        if (item != nullptr)
            weapspeed = item->getItemProperties()->Delay;

        setProcChance(7 * weapspeed / 600);
        if (getProcChance() >= 50)
            setProcChance(50);
    }
};

class GraceOfTheNaaruSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new GraceOfTheNaaruSpellProc(); }

    void init(Object* /*obj*/) override
    {
        this->setProcClassMask(0, 0x80000000);
    }
};

class SpiritualAttunementSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new SpiritualAttunementSpellProc(); }

    bool canProc(Unit* /*victim*/, SpellInfo const* castingSpell) override
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

    void init(Object* /*obj*/) override
    {
        this->setProcFlags(PROC_ON_DONE_MELEE_HIT);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// DeathKnight ProcScripts
class BladeBarrierSpellProc : public SpellProc
{
public:

    static SpellProc* Create() { return new BladeBarrierSpellProc(); }

    void init(Object* /*obj*/) override
    {
        setProcFlags(PROC_ON_DONE_MELEE_SPELL_HIT);

        setProcClassMask(0, getOriginalSpell()->getEffectSpellClassMask(0, 0));
        setProcClassMask(1, getOriginalSpell()->getEffectSpellClassMask(0, 1));
        setProcClassMask(2, getOriginalSpell()->getEffectSpellClassMask(0, 2));

        dk = static_cast<DeathKnight*>(getProcOwner());
    }

    bool canProc(Unit* /*victim*/, SpellInfo const* /*castingSpell*/) override
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

    bool doEffect(Unit* /*victim*/, SpellInfo const* /*castingSpell*/, uint32 /*flag*/, uint32 /*dmg*/, uint32 /*abs*/, int* /*dmgOverwrite*/, uint32 /*weaponDamageType*/) override
    {
        DeathKnight* dk = static_cast<DeathKnight*>(getProcOwner());

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
    addById(58872, &DamageShieldSpellProc::Create);     // Rank 1
    addById(58874, &DamageShieldSpellProc::Create);     // Rank 2

    addById(65156, &JuggernautSpellProc::Create);

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

    addByIds(frostbrandAttack, &FrostBrandAttackSpellProc::Create);

    addById(10444, &FlametongueWeaponSpellProc::Create);

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
    addByIds(woundPoison, &WoundPoisonSpellProc::Create);

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
    addByIds(instantPoison, &InstantPoisonSpellProc::Create);

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
    addByIds(deadlyPoison, &PoisonSpellProc::Create);

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
    addByIds(cripplingPoison, &PoisonSpellProc::Create);

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
    addByIds(mindNumbingPoison, &PoisonSpellProc::Create);

    uint32 waylay[] =
    {
        //SPELL_HASH_WAYLAY
        51692,
        51693,
        51696,
        0
    };
    addByIds(waylay, &WaylaySpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Priest
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
    addByIds(improvedDevouringPlague, &ImprovedDevouringPlagueSpellProc::Create);

    uint32 empoweredRenew[] =
    {
        //SPELL_HASH_EMPOWERED_RENEW
        63534,
        63542,
        63543,
        63544,
        0
    };
    addByIds(empoweredRenew, &EmpoweredRenewSpellProc::Create);

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
    addByIds(misery, &MiserySpellProc::Create);

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
    addByIds(prayerOfMending, &PrayerOfMendingProc::Create);

    uint32 spiritTap[] =
    {
        //SPELL_HASH_SPIRIT_TAP
        15270,
        15271,
        15335,
        15336,
        0
    };
    addByIds(spiritTap, &SpiritTapSpellProc::Create);

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
    addByIds(sealOfCommand, &SealOfCommandSpellProc::Create);

    addById(43742, &GraceOfTheNaaruSpellProc::Create);

    uint32 spiritualAttunement[] =
    {
        //SPELL_HASH_SPIRITUAL_ATTUNEMENT
        31785,
        31786,
        33776,
        0
    };
    addByIds(spiritualAttunement, &SpiritualAttunementSpellProc::Create);

    addById(20167, &PaladinSealsSpellProc::Create);
    addById(20168, &PaladinSealsSpellProc::Create);
    addById(20170, &PaladinSealsSpellProc::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // DeathKnight
    addById(50806, &DeathRuneMasterySpellProc::Create);

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
    addByIds(bladeBarrier, &BladeBarrierSpellProc::Create);
}
