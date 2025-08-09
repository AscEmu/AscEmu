/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "SpellAura.hpp"
#include "Definitions/SpellSchoolConversionTable.hpp"
#include "Definitions/DispelType.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Objects/Units/Creatures/Summons/SummonHandler.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
// Mage Scripts
class FirestarterTalent : public Spell
{
public:
    FirestarterTalent(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new FirestarterTalent(Caster, info, triggered, aur); }

    void DoAfterHandleEffect(Unit* target, uint32_t /*i*/) override
    {
        if (p_caster != NULL && target != NULL && p_caster->hasAurasWithId(54741)) // Cronicman: Player has "Firestarter" aura so we remove it AFTER casting Flamestrike.
        {
            p_caster->removeAllAurasById(54741);
        }
    }
};

class MissileBarrage : public Spell
{
public:
    MissileBarrage(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new MissileBarrage(Caster, info, triggered, aur); }

    void DoAfterHandleEffect(Unit* target, uint32_t /*i*/) override
    {
        if (p_caster != NULL && target != NULL && p_caster->hasAurasWithId(44401)) // Player has "Missile Barrage" aura so we remove it AFTER casting arcane missles.
        {
            p_caster->removeAllAurasById(44401);
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
public:
    FireNova(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new FireNova(Caster, info, triggered, aur); }

    bool HasFireTotem = false;
    SpellCastResult canCast(const bool tolerate, uint32_t* parameter1, uint32_t* parameter2)
    {
        auto result = Spell::canCast(tolerate, parameter1, parameter2);

#if VERSION_STRING >= WotLK
        if (result == SPELL_CAST_SUCCESS)
        {
            if (u_caster)
            {
                auto* totem = u_caster->getTotem(SUMMON_SLOT_TOTEM_FIRE);
                if (totem != nullptr)
                {
                    CastSpell(totem);
                }
                else
                {
                    *parameter1 = SPELL_EXTRA_ERROR_MUST_HAVE_FIRE_TOTEM;
                    result = SPELL_FAILED_CUSTOM_ERROR;
                }
            }
        }
#endif
        return result;
    }

    void CastSpell(Unit* totem)
    {
        uint32_t fireNovaSpells = Spell::getSpellInfo()->getId();
        //Cast spell. NOTICE All ranks are linked with a extra spell in HackFixes.cpp
        totem->castSpellLoc(totem->GetPosition(), sSpellMgr.getSpellInfo(fireNovaSpells), true);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Rogue Scripts
class CheatDeathAura : public AbsorbAura
{
public:
    CheatDeathAura(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster)
    {
        dSpell = sSpellMgr.getSpellInfo(31231);
    }

    static std::unique_ptr<Aura> Create(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
    {
        return std::make_unique<CheatDeathAura>(proto, duration, caster, target, temporary, i_caster);
    }

    uint32_t absorbDamage(SchoolMask /*School*/, uint32_t* dmg, bool checkOnly) override
    {
        // Checking for 1 min cooldown
        if (dSpell == NULL || getPlayerOwner()->hasSpellOnCooldown(dSpell))
            return 0;

        // Check for proc chance
        if (Util::getRandomFloat(100.0f) > getSpellInfo()->calculateEffectValue(0))
            return 0;

        // Check if damage will kill player.
        uint32_t cur_hlth = getPlayerOwner()->getHealth();
        if ((*dmg) < cur_hlth)
            return 0;

        if (checkOnly)
            return 0;

        uint32_t max_hlth = getPlayerOwner()->getMaxHealth();
        uint32_t min_hlth = max_hlth / 10;

        /*
        looks like the following lines are not so good, we check and cast on spell id 31231_
        and adding the cooldown to it, but it looks like this spell is useless(all it's doing is_
        casting 45182, so we can do all this stuff on 45182 at first place), BUT_
        as long as proceeding cheat death is not so height (how many rogue at the same time_
        gonna get to this point?) so it's better to use it because we wont lose anything!!
        */
        getPlayerOwner()->castSpell(getPlayerOwner()->getGuid(), dSpell, true);

        // set dummy effect,
        // this spell is used to procced the post effect of cheat death later.
        // Move next line to SPELL::SpellEffectDummy ?!! well it's better in case of dbc changing!!
        getPlayerOwner()->castSpell(getPlayerOwner()->getGuid(), 45182, true);

        // Better to add custom cooldown procedure then fucking with entry, or not!!
        getPlayerOwner()->addSpellCooldown(dSpell, nullptr, nullptr, 60000);

        // Calc abs and applying it
        uint32_t real_dmg = (cur_hlth > min_hlth ? cur_hlth - min_hlth : 0);
        uint32_t absorbed_dmg = *dmg - real_dmg;

        *dmg = real_dmg;
        return absorbed_dmg;
    }

private:
    SpellInfo const* dSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Priest Scripts
class DispersionSpell : public Spell
{
public:
    DispersionSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new DispersionSpell(Caster, info, triggered, aur); }

    void DoAfterHandleEffect(Unit* target, uint32_t /*i*/)
    {
        if (p_caster != NULL)
        {
            // Mana regeneration
            p_caster->castSpell(target, 60069, true);
            // Remove snares and movement impairing effects and make player immune to them
            p_caster->castSpell(target, 63230, true);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Druid Scripts
class InnervateSpell : public Spell
{
public:
    InnervateSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new InnervateSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* target, int32_t value)
    {
        if (p_caster != NULL && i == 0 && target != NULL)
            value = (uint32_t)(p_caster->getBaseMana() * 0.225f);

        return value;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// DeathKnight Scripts
class BloodPlagueSpell : public Spell
{
public:
    BloodPlagueSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new BloodPlagueSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* /*target*/, int32_t value)
    {
        if (p_caster != NULL && i == 0)
            value += (uint32_t)(p_caster->getCalculatedAttackPower() * 0.055 * 1.15);

        return value;
    }
};

class IcyTouchSpell : public Spell
{
public:
    IcyTouchSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new IcyTouchSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* /*target*/, int32_t value) override
    {
        if (p_caster != NULL && i == 0)
            value += (uint32_t)(p_caster->getCalculatedAttackPower() * 0.1);

        return value;
    }
};

class FrostFeverSpell : public Spell
{
public:
    FrostFeverSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new FrostFeverSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* /*target*/, int32_t value) override
    {
        if (p_caster != NULL && i == 0)
            value += (uint32_t)(p_caster->getCalculatedAttackPower() * 0.055 * 1.15);

        return value;
    }
};

class BloodBoilSpell : public Spell
{
public:
    BloodBoilSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new BloodBoilSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* target, int32_t value)
    {
        if (p_caster != NULL && i == 0)
        {
            int32_t ap = p_caster->getCalculatedAttackPower();

            value += (uint32_t)(ap * 0.08);

            // Does additional damage if target has diseases (http://www.tankspot.com/forums/f14/48814-3-1-blood-boil-mechanics-tested.html)
            if (target != NULL && (target->hasAurasWithId(55078) || target->hasAurasWithId(55095)))
                value += (uint32_t)(ap * 0.015 + 95);
        }

        return value;
    }
};

class BloodStrikeSpell : public Spell
{
public:
    BloodStrikeSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new BloodStrikeSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t /*i*/, Unit* target, int32_t value)
    {
        if (target != NULL)
        {
            uint32_t count = target->getAuraCountWithDispelType(DISPEL_DISEASE, m_caster->getGuid());
            if (count)
                value += value * count * (getSpellInfo()->calculateEffectValue(2)) / 200;
        }

        return value;
    }

    void DoAfterHandleEffect(Unit* target, uint32_t i)
    {
        if (p_caster == NULL || i != 1)
            return;

        uint32_t suddenDoom[] =
        {
            //SPELL_HASH_SUDDEN_DOOM
            49018,
            49529,
            49530,
            0
        };

        Aura* aur = p_caster->getAuraWithId(suddenDoom);
        if (aur == NULL)
            return;

        if (!Util::checkChance(aur->getSpellInfo()->getProcChance()))
            return;

        p_caster->castSpell(target, 47632, false);
    }
};

class DeathCoilSpell : public Spell
{
public:
    DeathCoilSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new DeathCoilSpell(Caster, info, triggered, aur); }

    SpellCastResult canCast(const bool tolerate, uint32_t* parameter1, uint32_t* parameter2)
    {
        auto result = Spell::canCast(tolerate, parameter1, parameter2);

        if (result == SPELL_CAST_SUCCESS)
        {
            if (m_caster != NULL && m_caster->IsInWorld())
            {
                Unit* target = m_caster->getWorldMap()->getUnit(m_targets.getUnitTargetGuid());

                if (target == NULL || !(m_caster->isValidTarget(target) || target->getRace() == RACE_UNDEAD))
                    result = SPELL_FAILED_BAD_TARGETS;
            }
        }

        return result;
    }
};

class RuneStrileSpell : public Spell
{
public:
    RuneStrileSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new RuneStrileSpell(Caster, info, triggered, aur); }

    void DoAfterHandleEffect(Unit* /*target*/, uint32_t /*i*/)
    {
        if (u_caster != NULL)
            u_caster->removeAllAurasById(56817);
    }
};

class AntiMagicShellAura : public AbsorbAura
{
public:
    AntiMagicShellAura(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    static std::unique_ptr<Aura> Create(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
    {
        return std::make_unique<AntiMagicShellAura>(proto, duration, caster, target, temporary, i_caster);
    }

    int32_t CalcAbsorbAmount(AuraEffectModifier* aurEff)
    {
        Player* caster = GetPlayerCaster();
        if (caster != NULL)
            return caster->getMaxHealth() * (getSpellInfo()->calculateEffectValue(1)) / 100;
        else
            return aurEff->getEffectDamage();
    }

    uint8_t CalcPctDamage()
    {
        return static_cast<uint8_t>(getSpellInfo()->calculateEffectValue(0));
    }
};

class SpellDeflectionAura : public AbsorbAura
{
#if VERSION_STRING >= TBC // support classic
public:
    SpellDeflectionAura(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    static std::unique_ptr<Aura> Create(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
    {
        return std::make_unique<SpellDeflectionAura>(proto, duration, caster, target, temporary, i_caster);
    }

    uint32_t absorbDamage(SchoolMask schoolMask, uint32_t* dmg, bool /*checkOnly*/) override
    {
        // Check if aura can absorb this school
        if (!(m_absorbSchoolMask & schoolMask))
            return 0;

        Player* caster = GetPlayerCaster();
        if (caster == NULL)
            return 0;

        if (!Util::checkChance(caster->getParryChance()))
            return 0;

        uint32_t dmg_absorbed = *dmg * getEffectDamage(0) / 100;
        *dmg -= dmg_absorbed;

        return dmg_absorbed;
    }
#endif
};

class BloodwormSpell : public Spell
{
public:
    BloodwormSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new BloodwormSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t /*i*/, Unit* /*target*/, int32_t /*value*/)
    {
        return 2 + Util::getRandomUInt(2);
    }
};

class WillOfTheNecropolisAura : public AbsorbAura
{
public:
    WillOfTheNecropolisAura(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
        : AbsorbAura(proto, duration, caster, target, temporary, i_caster) {}

    static std::unique_ptr<Aura> Create(SpellInfo* proto, int32_t duration, Object* caster, Unit* target, bool temporary = false, Item* i_caster = nullptr)
    {
        return std::make_unique<WillOfTheNecropolisAura>(proto, duration, caster, target, temporary, i_caster);
    }

    uint32_t absorbDamage(SchoolMask /*School*/, uint32_t* dmg, bool /*checkOnly*/) override
    {
        Unit* caster = GetUnitCaster();
        if (caster == NULL)
            return 0;

        int health_pct = caster->getHealthPct();
        uint32_t cur_health = caster->getHealth();
        uint32_t max_health = caster->getMaxHealth();
        uint32_t new_health_pct = (cur_health - *dmg) * 100 / max_health;

        // "Damage that would take you below $s1% health or taken while you are at $s1% health is reduced by $52284s1%."
        if ((health_pct > 35 && new_health_pct < 35) || health_pct == 35)
        {
            uint32_t dmg_absorbed = *dmg * (getSpellInfo()->calculateEffectValue(0)) / 100;
            *dmg -= dmg_absorbed;

            return dmg_absorbed;
        }
        else
            return 0;
    }
};

class VampiricBloodSpell : public Spell
{
public:
    VampiricBloodSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new VampiricBloodSpell(Caster, info, triggered, aur); }

    int32_t DoCalculateEffect(uint32_t i, Unit* /*target*/, int32_t value) override
    {
        if (i == 1 && p_caster != NULL)
            value = p_caster->getMaxHealth() * (getSpellInfo()->getEffectBasePoints(static_cast<uint8_t>(i)) + 1) / 100;

        return value;
    }
};

class HeartStrikeSpell : public Spell
{
public:
    HeartStrikeSpell(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) : Spell(Caster, info, triggered, aur) {}

    static Spell* Create(Object* Caster, SpellInfo *info, bool triggered, Aura* aur) { return new HeartStrikeSpell(Caster, info, triggered, aur); }

    void DoAfterHandleEffect(Unit* target, uint32_t i)
    {
        if (p_caster == NULL || i != 1)
            return;

        uint32_t suddenDoom[] =
        {
            // SPELL_HASH_SUDDEN_DOOM
            49018,
            49529,
            49530,
            0
        };

        Aura* aur = p_caster->getAuraWithId(suddenDoom);
        if (aur == NULL)
            return;

        if (!Util::checkChance(aur->getSpellInfo()->getProcChance()))
            return;

        p_caster->castSpell(target, 47632, false);
    }
};

void SpellMgr::setupSpellClassScripts()
{
    //////////////////////////////////////////////////////////////////////////////////////////
    // Mage
    addSpellById(2120, FirestarterTalent::Create);      // Rank 1
#if VERSION_STRING < Cata
    addSpellById(2121, FirestarterTalent::Create);      // Rank 2
    addSpellById(8422, FirestarterTalent::Create);      // Rank 3
    addSpellById(8423, FirestarterTalent::Create);      // Rank 4
    addSpellById(10215, FirestarterTalent::Create);     // Rank 5
    addSpellById(10216, FirestarterTalent::Create);     // Rank 6
#if VERSION_STRING >= TBC
    addSpellById(27086, FirestarterTalent::Create);     // Rank 7
#if VERSION_STRING == WotLK
    addSpellById(42925, FirestarterTalent::Create);     // Rank 8
    addSpellById(42926, FirestarterTalent::Create);     // Rank 9
#endif
#endif
#endif
    addSpellById(5143, MissileBarrage::Create);         // Rank 1
#if VERSION_STRING < Cata
    addSpellById(5144, MissileBarrage::Create);         // Rank 2
    addSpellById(5145, MissileBarrage::Create);         // Rank 3
    addSpellById(8416, MissileBarrage::Create);         // Rank 4
    addSpellById(8417, MissileBarrage::Create);         // Rank 5
    addSpellById(10211, MissileBarrage::Create);        // Rank 6
    addSpellById(10212, MissileBarrage::Create);        // Rank 7
    addSpellById(25345, MissileBarrage::Create);        // Rank 8
    addSpellById(27075, MissileBarrage::Create);        // Rank 9
#if VERSION_STRING >= TBC
    addSpellById(38699, MissileBarrage::Create);        // Rank 10
    addSpellById(38704, MissileBarrage::Create);        // Rank 11
#if VERSION_STRING == WotLK
    addSpellById(42843, MissileBarrage::Create);        // Rank 12
    addSpellById(42846, MissileBarrage::Create);        // Rank 13
#endif
#endif
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warrior

    //////////////////////////////////////////////////////////////////////////////////////////
    // Warlock

    //////////////////////////////////////////////////////////////////////////////////////////
    // Shaman
    addSpellById(1535, FireNova::Create);   // Rank 1
#if VERSION_STRING < Cata
    addSpellById(8498, FireNova::Create);   // Rank 2
    addSpellById(8499, FireNova::Create);   // Rank 3
    addSpellById(11314, FireNova::Create);  // Rank 4
    addSpellById(11315, FireNova::Create);  // Rank 5
#if VERSION_STRING >= TBC
    addSpellById(25546, FireNova::Create);  // Rank 6
    addSpellById(25547, FireNova::Create);  // Rank 7
#if VERSION_STRING == WotLK
    addSpellById(61649, FireNova::Create);  // Rank 8
    addSpellById(61657, FireNova::Create);  // Rank 9
#endif
#endif
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // Rogue
    addAuraById(31228, &CheatDeathAura::Create);   // Rank 1
    addAuraById(31229, &CheatDeathAura::Create);   // Rank 2
    addAuraById(31230, &CheatDeathAura::Create);   // Rank 3

    //////////////////////////////////////////////////////////////////////////////////////////
    // Priest
    addSpellById(47585, &DispersionSpell::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Druid
    addSpellById(29166, &InnervateSpell::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // DeathKnight
    addSpellById(55078, &BloodPlagueSpell::Create);
    addSpellById(45477, &IcyTouchSpell::Create);
    addSpellById(55095, &FrostFeverSpell::Create);

    addSpellById(48721, &BloodBoilSpell::Create);           // Rank 1
#if VERSION_STRING == WotLK
    addSpellById(49939, &BloodBoilSpell::Create);           // Rank 2
    addSpellById(49940, &BloodBoilSpell::Create);           // Rank 3
    addSpellById(49941, &BloodBoilSpell::Create);           // Rank 4
#endif
    addSpellById(45902, &BloodStrikeSpell::Create);         // Rank 1
#if VERSION_STRING == WotLK
    addSpellById(49926, &BloodStrikeSpell::Create);         // Rank 2
    addSpellById(49927, &BloodStrikeSpell::Create);         // Rank 3
    addSpellById(49928, &BloodStrikeSpell::Create);         // Rank 4
    addSpellById(49929, &BloodStrikeSpell::Create);         // Rank 5
    addSpellById(49930, &BloodStrikeSpell::Create);         // Rank 6
#endif
    addSpellById(47541, &DeathCoilSpell::Create);           // Rank 1
#if VERSION_STRING == WotLK
    addSpellById(49892, &DeathCoilSpell::Create);           // Rank 2
    addSpellById(49893, &DeathCoilSpell::Create);           // Rank 3
    addSpellById(49894, &DeathCoilSpell::Create);           // Rank 4
    addSpellById(49895, &DeathCoilSpell::Create);           // Rank 5
#endif
    addSpellById(56815, &RuneStrileSpell::Create);

    addAuraById(48707, &AntiMagicShellAura::Create);

#if VERSION_STRING == WotLK
    addAuraById(49145, &SpellDeflectionAura::Create);       // Rank 1
    addAuraById(49495, &SpellDeflectionAura::Create);       // Rank 2
    addAuraById(49497, &SpellDeflectionAura::Create);       // Rank 3
#endif

    addSpellById(50452, &BloodwormSpell::Create);

    addAuraById(52284, &WillOfTheNecropolisAura::Create);   // Rank 1
#if VERSION_STRING == WotLK
    addAuraById(52285, &WillOfTheNecropolisAura::Create);   // Rank 1
    addAuraById(52286, &WillOfTheNecropolisAura::Create);   // Rank 1
#endif
    addSpellById(55233, &VampiricBloodSpell::Create);

    addSpellById(55050, &HeartStrikeSpell::Create);         // Rank 1
#if VERSION_STRING == WotLK
    addSpellById(55258, &HeartStrikeSpell::Create);         // Rank 2
    addSpellById(55259, &HeartStrikeSpell::Create);         // Rank 3
    addSpellById(55260, &HeartStrikeSpell::Create);         // Rank 4
    addSpellById(55261, &HeartStrikeSpell::Create);         // Rank 5
    addSpellById(55262, &HeartStrikeSpell::Create);         // Rank 6
 #endif
}
