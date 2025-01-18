/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Players/Player.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellScript.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "CommonTime.hpp"
#include "Utilities/Random.hpp"
#include "Utilities/Util.hpp"

enum SpellItems
{
    SPELL_CHAOS_BANE_AOE                        = 71904,
    SPELL_CHAOS_BANE_BUFF                       = 73422,
    SPELL_DEATHBRINGERS_WILL_NORMAL_DUMMY       = 71519,
    SPELL_DEATHBRINGERS_WILL_NORMAL_STRENGTH    = 71484, // Strength of the Taunka +600 str
    SPELL_DEATHBRINGERS_WILL_NORMAL_AGILITY     = 71485, // Agility of the Vrykul +600 agi
    SPELL_DEATHBRINGERS_WILL_NORMAL_ATTACKPOW   = 71486, // Power of the Taunka +1200 ap
    SPELL_DEATHBRINGERS_WILL_NORMAL_CRIT        = 71491, // Aim of the Iron Dwarves +600 crit
    SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE       = 71492, // Speed of the Vrykul +600 haste
    SPELL_DEATHBRINGERS_WILL_HEROIC_DUMMY       = 71562,
    SPELL_DEATHBRINGERS_WILL_HEROIC_AGILITY     = 71556, // Agility of the Vrykul +700 agi
    SPELL_DEATHBRINGERS_WILL_HEROIC_ATTACKPOW   = 71558, // Power of the Taunka +1400 ap
    SPELL_DEATHBRINGERS_WILL_HEROIC_CRIT        = 71559, // Aim of the Iron Dwarves +700 crit
    SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE       = 71560, // Speed of the Vrykul +700 haste
    SPELL_DEATHBRINGERS_WILL_HEROIC_STRENGTH    = 71561, // Strength of the Taunka +700 str
    SPELL_MURLOC_COSTUME                        = 42365,
    SPELL_ORB_OF_DECEPTION                      = 16739,
    SPELL_SHADOWMOURNE_DUMMY                    = 71903,
    SPELL_SOUL_FRAGMENT                         = 71905,
    SPELL_SOUL_FRAGMENT_VISUAL_BIG              = 72523,
    SPELL_SOUL_FRAGMENT_VISUAL_SMALL            = 72521,

    ITEM_DEATHBRINGERS_WILL_NORMAL              = 50362,
    ITEM_DEATHBRINGERS_WILL_HEROIC              = 50363,

    CREATURE_MURLOC_COSTUME                     = 23754,
};

// General script to set internal cooldown (ICD) for item procs
class ItemInternalCooldown : public SpellScript
{
public:
    ItemInternalCooldown(uint32_t internalCooldown) : _internalCooldown(internalCooldown) {}

    void onCreateSpellProc(SpellProc* spellProc, Object* /*obj*/) override
    {
        // If you need to script the proc spell and you need to use onCreateSpellProc hook,
        // remember to call this class function!
        spellProc->setProcInterval(_internalCooldown);
    }

private:
    uint32_t _internalCooldown = 0;
};

// Deathbringer's Will (item 50362 and item 50363) has a chance to give randomized effect for certain classes
// handles both normal and heroic versions
class DeathbringersWill : public ItemInternalCooldown
{
public:
    uint32_t normalProcs[MAX_PLAYER_CLASSES][3] =
    {
        { },
        // Warrior
        { SPELL_DEATHBRINGERS_WILL_NORMAL_STRENGTH, SPELL_DEATHBRINGERS_WILL_NORMAL_CRIT, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE },
        // Paladin
        { SPELL_DEATHBRINGERS_WILL_NORMAL_STRENGTH, SPELL_DEATHBRINGERS_WILL_NORMAL_CRIT, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE },
        // Hunter
        { SPELL_DEATHBRINGERS_WILL_NORMAL_AGILITY, SPELL_DEATHBRINGERS_WILL_NORMAL_CRIT, SPELL_DEATHBRINGERS_WILL_NORMAL_ATTACKPOW },
        // Rogue
        { SPELL_DEATHBRINGERS_WILL_NORMAL_AGILITY, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE, SPELL_DEATHBRINGERS_WILL_NORMAL_ATTACKPOW },
        // Priest
        { },
        // Death Knight
        { SPELL_DEATHBRINGERS_WILL_NORMAL_STRENGTH, SPELL_DEATHBRINGERS_WILL_NORMAL_CRIT, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE },
        // Shaman
        { SPELL_DEATHBRINGERS_WILL_NORMAL_AGILITY, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE, SPELL_DEATHBRINGERS_WILL_NORMAL_ATTACKPOW },
        // Mage
        { },
        // Warlock
        { },
        // Monk
        { },
        // Druid
        { SPELL_DEATHBRINGERS_WILL_NORMAL_STRENGTH, SPELL_DEATHBRINGERS_WILL_NORMAL_AGILITY, SPELL_DEATHBRINGERS_WILL_NORMAL_HASTE }
    };

    uint32_t heroicProcs[MAX_PLAYER_CLASSES][3] =
    {
        { },
        // Warrior
        { SPELL_DEATHBRINGERS_WILL_HEROIC_STRENGTH, SPELL_DEATHBRINGERS_WILL_HEROIC_CRIT, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE },
        // Paladin
        { SPELL_DEATHBRINGERS_WILL_HEROIC_STRENGTH, SPELL_DEATHBRINGERS_WILL_HEROIC_CRIT, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE },
        // Hunter
        { SPELL_DEATHBRINGERS_WILL_HEROIC_AGILITY, SPELL_DEATHBRINGERS_WILL_HEROIC_CRIT, SPELL_DEATHBRINGERS_WILL_HEROIC_ATTACKPOW },
        // Rogue
        { SPELL_DEATHBRINGERS_WILL_HEROIC_AGILITY, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE, SPELL_DEATHBRINGERS_WILL_HEROIC_ATTACKPOW },
        // Priest
        { },
        // Death Knight
        { SPELL_DEATHBRINGERS_WILL_HEROIC_STRENGTH, SPELL_DEATHBRINGERS_WILL_HEROIC_CRIT, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE },
        // Shaman
        { SPELL_DEATHBRINGERS_WILL_HEROIC_AGILITY, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE, SPELL_DEATHBRINGERS_WILL_HEROIC_ATTACKPOW },
        // Mage
        { },
        // Warlock
        { },
        // Monk
        { },
        // Druid
        { SPELL_DEATHBRINGERS_WILL_HEROIC_STRENGTH, SPELL_DEATHBRINGERS_WILL_HEROIC_AGILITY, SPELL_DEATHBRINGERS_WILL_HEROIC_HASTE },
    };

    DeathbringersWill(uint32_t internalCooldown) : ItemInternalCooldown(internalCooldown) {}

    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // On dummy aura apply make it proc self
        if (apply)
            aur->getOwner()->addProcTriggerSpell(aur->getSpellInfo(), aur->getCasterGuid(), aur);
        else
            aur->getOwner()->removeProcTriggerSpell(aur->getSpellId(), aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onCreateSpellProc(SpellProc* spellProc, Object* obj) override
    {
        isHeroic = spellProc->getSpell()->getId() == SPELL_DEATHBRINGERS_WILL_HEROIC_DUMMY;
        itemProp = sMySQLStore.getItemProperties(isHeroic ? ITEM_DEATHBRINGERS_WILL_HEROIC : ITEM_DEATHBRINGERS_WILL_NORMAL);
        if (itemProp == nullptr)
        {
            spellProc->deleteProc();
            return;
        }

        ItemInternalCooldown::onCreateSpellProc(spellProc, obj);
    }

    bool canProc(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (!spellProc->getProcOwner()->isPlayer())
            return false;

        // Check for correct class
        const auto plr = static_cast<Player*>(spellProc->getProcOwner());
        if (plr->getClass() != WARRIOR &&
            plr->getClass() != PALADIN &&
            plr->getClass() != HUNTER &&
            plr->getClass() != ROGUE &&
            plr->getClass() != DEATHKNIGHT &&
            plr->getClass() != SHAMAN &&
            plr->getClass() != DRUID)
            return false;

        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        const auto plr = static_cast<Player*>(spellProc->getProcOwner());

        // Cast random effect
        if (isHeroic)
            plr->castSpell(plr, heroicProcs[plr->getClass()][Util::getRandomUInt(2)], true);
        else
            plr->castSpell(plr, normalProcs[plr->getClass()][Util::getRandomUInt(2)], true);

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

private:
    ItemProperties const* itemProp = nullptr;
    bool isHeroic = false;
};

class MurlocCostume : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* /*aur*/, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptExecuteState::EXECUTE_OK;

        // Spell is missing the misc value
        aurEff->setEffectMiscValue(CREATURE_MURLOC_COSTUME);
        return SpellScriptExecuteState::EXECUTE_OK;
    }
};

class OrbOfDeception : public SpellScript
{
public:
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptExecuteState::EXECUTE_OK;

        uint32_t displayId = 0;
        switch (aur->getOwner()->getRace())
        {
            case RACE_ORC:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10139 : 10140;
                break;
            case RACE_TAUREN:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10136 : 10147;
                break;
            case RACE_TROLL:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10135 : 10134;
                break;
            case RACE_UNDEAD:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10146 : 10145;
                break;
            case RACE_GNOME:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10148 : 10149;
                break;
            case RACE_DWARF:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10141 : 10142;
                break;
            case RACE_HUMAN:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10137 : 10138;
                break;
            case RACE_NIGHTELF:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 10143 : 10144;
                break;
#if VERSION_STRING >= TBC
            case RACE_BLOODELF:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 17829 : 17830;
                break;
            case RACE_DRAENEI:
                displayId = aur->getOwner()->getGender() == GENDER_MALE ? 17827 : 17828;
                break;
#endif
            default:
                return SpellScriptExecuteState::EXECUTE_PREVENT;
        }

        aurEff->setEffectFixedDamage(displayId);

        // Let restoreDisplayId check if form can be applied
        aur->getOwner()->restoreDisplayId();
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

// Shadowmourne grants stackable Soul Fragments on hits and on 10th stack an aoe and a buff
class Shadowmourne : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        // On dummy aura apply make it proc self
        if (apply)
            aur->getOwner()->addProcTriggerSpell(aur->getSpellInfo(), aur->getCasterGuid(), aur);
        else
            aur->getOwner()->removeProcTriggerSpell(aur->getSpellId(), aur->getCasterGuid());

        return SpellScriptCheckDummy::DUMMY_OK;
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        // Also remove Soul Fragments
        aur->getOwner()->removeAllAurasById(SPELL_SOUL_FRAGMENT);
    }

    bool canProc(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        if (!spellProc->getProcOwner()->isPlayer())
            return false;

        // Check if caster has Chaos Bane buff active which prevents gathering Soul Fragments
        if (spellProc->getProcOwner()->hasAurasWithId(SPELL_CHAOS_BANE_BUFF))
            return false;

        return true;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* victim, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        // Check stacks
        const auto aur = spellProc->getProcOwner()->getAuraWithId(SPELL_SOUL_FRAGMENT);
        if (aur != nullptr && aur->getStackCount() >= 9)
        {
            // Cast aoe spell
            spellProc->getProcOwner()->castSpell(victim, SPELL_CHAOS_BANE_AOE, true);
            // Cast self buff
            spellProc->getProcOwner()->castSpell(spellProc->getProcOwner(), SPELL_CHAOS_BANE_BUFF, true);
            // Remove Soul Fragments
            spellProc->getProcOwner()->removeAllAurasById(SPELL_SOUL_FRAGMENT);
        }
        else
        {
            // Add new Soul Fragment
            spellProc->getProcOwner()->castSpell(spellProc->getProcOwner(), SPELL_SOUL_FRAGMENT, true);
        }

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};

class SoulFragment : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override
    {
        // On stack 1 to 5th the aura should have small visual effect
        aur->getOwner()->castSpell(aur->getOwner(), SPELL_SOUL_FRAGMENT_VISUAL_SMALL, true);
    }

    void onAuraRemove(Aura* aur, AuraRemoveMode /*mode*/) override
    {
        aur->getOwner()->removeAllAurasById(SPELL_SOUL_FRAGMENT_VISUAL_SMALL);
        aur->getOwner()->removeAllAurasById(SPELL_SOUL_FRAGMENT_VISUAL_BIG);
    }

    void onAuraRefreshOrGainNewStack(Aura* aur, uint32_t newStack, uint32_t /*oldStack*/) override
    {
        // On 6th stack the visual should change bigger
        if (newStack == 6)
        {
            aur->getOwner()->removeAllAurasById(SPELL_SOUL_FRAGMENT_VISUAL_SMALL);
            aur->getOwner()->castSpell(aur->getOwner(), SPELL_SOUL_FRAGMENT_VISUAL_BIG, true);
        }
    }
};

void setupItemSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyItemSpells_1(mgr);

    uint32_t deathbringerWillIds[] =
    {
        SPELL_DEATHBRINGERS_WILL_NORMAL_DUMMY,
        SPELL_DEATHBRINGERS_WILL_HEROIC_DUMMY,
        0
    };
    mgr->register_spell_script(deathbringerWillIds, new DeathbringersWill(105 * TimeVarsMs::Second));

    mgr->register_spell_script(SPELL_MURLOC_COSTUME, new MurlocCostume);
    mgr->register_spell_script(SPELL_ORB_OF_DECEPTION, new OrbOfDeception);

    mgr->register_spell_script(SPELL_SHADOWMOURNE_DUMMY, new Shadowmourne);
    mgr->register_spell_script(SPELL_SOUL_FRAGMENT, new SoulFragment);
}
