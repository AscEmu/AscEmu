/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Objects/Units/Unit.hpp"
#include "Spell/SpellAura.hpp"
#include "Spell/SpellMgr.hpp"
#include "Spell/SpellProc.hpp"
#include "Spell/SpellScript.hpp"

enum WarlockSpells
{
    SPELL_BACKLASH_PROC                         = 34936,
    SPELL_DEMONIC_KNOWLEDGE_DUMMY_R1            = 35691,
    SPELL_DEMONIC_KNOWLEDGE                     = 35696,
    SPELL_DEMONIC_SACRIFICE                     = 18788,
    SPELL_DEMONIC_SACRIFICE_FELGUARD            = 35701,
    SPELL_DEMONIC_SACRIFICE_FELHUNTER           = 18792,
    SPELL_DEMONIC_SACRIFICE_IMP                 = 18789,
    SPELL_DEMONIC_SACRIFICE_SUCCUBUS            = 18791,
    SPELL_DEMONIC_SACRIFICE_VOIDWALKER          = 18790,
    SPELL_MASTER_DEMONOLOGIST_DUMMY_R5          = 23825,
    SPELL_MASTER_DEMONOLOGIST_SCRIPTED_R4       = 23832,
    SPELL_MASTER_DEMONOLOGIST_FELGUARD_R1       = 35702,
    SPELL_MASTER_DEMONOLOGIST_FELHUNTER_R1      = 23762,
    SPELL_MASTER_DEMONOLOGIST_IMP_R1            = 23759,
    SPELL_MASTER_DEMONOLOGIST_SUCCUBUS_R1       = 23761,
    SPELL_MASTER_DEMONOLOGIST_VOIDWALKER_R1     = 23760,
    SPELL_NIGHTFALL_R1                          = 18094,
    SPELL_SHADOW_TRANCE_PROC                    = 17941,
    SPELL_SOUL_LINK_DUMMY                       = 19028,
    SPELL_SOUL_LINK                             = 25228,
    SPELL_SUMMON_FELGUARD                       = 30146,
    SPELL_SUMMON_FELHUNTER                      = 691,
    SPELL_SUMMON_IMP                            = 688,
    SPELL_SUMMON_SUCCUBUS                       = 712,
    SPELL_SUMMON_VOIDWALKER                     = 697,
};

#if VERSION_STRING >= TBC
class Backlash : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // SpellInfo is missing charge count
        aur->setCharges(1, false);
    }

    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        // 8 sec cooldown
        proc->setProcInterval(8000);
        proc->setCastedOnProcOwner(true);
    }
};
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
class DemonicKnowledgeDummy : public SpellScript
{
    // TODO: wow wiki patch notes say this will work with enslaved demons
    // however in classic tbc it is removed when demon is enslaved. which way is correct?
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            plrOwner->addPetAuraSpell(SPELL_DEMONIC_KNOWLEDGE, true);
        else
            plrOwner->removePetAuraSpell(SPELL_DEMONIC_KNOWLEDGE);

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class DemonicKnowledge : public SpellScript
{
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (!apply)
            return SpellScriptExecuteState::EXECUTE_OK;

        // Calculate only for pet
        if (aur->getCasterGuid() != aur->getOwner()->getGuid())
            return SpellScriptExecuteState::EXECUTE_OK;

        // This is casted on pet, get pct amount from pet owner
        // And save pct amount to extra field
        if (aurEff->getEffectExtraField() == 0)
        {
            if (const auto* owner = aur->getOwner()->getUnitOwner())
            {
                if (const auto* dummyAurEff = owner->getAuraEffectWithAnyRankOfAndByEffect(SPELL_AURA_DUMMY, SPELL_DEMONIC_KNOWLEDGE_DUMMY_R1))
                    aurEff->setEffectExtraField(dummyAurEff->getEffectDamage());
            }
        }

        // If not found use spell default pct (12%)
        const auto pctValue = aurEff->getEffectExtraField() == 0 ?
            aurEff->getEffectDamage() :
            aurEff->getEffectExtraField();

        // "Increases Master's spell damage by a percentage of the active demon's Stamina plus Intellect."
        // TODO: aura should be updated if pet sta/int changes
        const int32_t stamina = aur->getOwner()->getStat(STAT_STAMINA);
        const int32_t intellect = aur->getOwner()->getStat(STAT_INTELLECT);
        aurEff->setEffectDamage((stamina + intellect) * pctValue / 100);

        return SpellScriptExecuteState::EXECUTE_OK;
    }
};
#endif

#if VERSION_STRING < Mop
// Spell was removed from players in wotlk but removed from game in mop
class DemonicSacrifice : public SpellScript
{
    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override
    {
        const auto* unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SPELL_FAILED_UNKNOWN;

        const auto* minion = unitCaster->getPet();
        if (minion == nullptr || !minion->isPermanentSummon())
            return SPELL_FAILED_NO_PET;

        switch (minion->getCreatedBySpellId())
        {
            case SPELL_SUMMON_IMP:
            case SPELL_SUMMON_VOIDWALKER:
            case SPELL_SUMMON_SUCCUBUS:
            case SPELL_SUMMON_FELHUNTER:
            case SPELL_SUMMON_FELGUARD:
                return SPELL_CAST_SUCCESS;
            default: break;
        }

        return SPELL_FAILED_BAD_TARGETS;
    }

    void afterSpellEffect(Spell* spell, uint8_t effIndex, DamageInfo const& /*damageInfo*/) override
    {
        if (effIndex != EFF_INDEX_0)
            return;

        auto* const unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return;

        const auto* minion = spell->getUnitTarget();
        if (minion == nullptr)
            return;

        uint32_t spellId = 0;
        switch (minion->getCreatedBySpellId())
        {
            case SPELL_SUMMON_IMP:
                spellId = SPELL_DEMONIC_SACRIFICE_IMP;
                break;
            case SPELL_SUMMON_VOIDWALKER:
                spellId = SPELL_DEMONIC_SACRIFICE_VOIDWALKER;
                break;
            case SPELL_SUMMON_SUCCUBUS:
                spellId = SPELL_DEMONIC_SACRIFICE_SUCCUBUS;
                break;
            case SPELL_SUMMON_FELHUNTER:
                spellId = SPELL_DEMONIC_SACRIFICE_FELHUNTER;
                break;
            case SPELL_SUMMON_FELGUARD:
                spellId = SPELL_DEMONIC_SACRIFICE_FELGUARD;
                break;
            default: break;
        }

        if (spellId != 0)
            unitCaster->castSpell(unitCaster, spellId, true);
    }
};

class DemonicSacrificeEffect : public SpellScript
{
    // TODO: this overwrites "Override class script" effect
    // maybe add a proper script hook for it
    SpellScriptExecuteState beforeAuraEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_1)
            return SpellScriptExecuteState::EXECUTE_OK;

        if (apply)
        {
            // Make proc self on summon minion spells to remove aura
            const uint32_t procMask[3] = { 0x20000000, 0, 0 };
            // Some spells are magic and some not
            const auto procFlags = PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_MAGIC | PROC_ON_DONE_POSITIVE_SPELL_DAMAGE_CLASS_NONE;
            aur->getOwner()->addProcTriggerSpell(aur->getSpellInfo(), aur->getSpellInfo(), aur->getCasterGuid(), 100, procFlags, procMask, aur);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(aur->getSpellId(), aur->getCasterGuid());
        }

        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }

    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override
    {
        spellProc->getProcOwner()->removeAllAurasById(spellProc->getSpell()->getId());
        return SpellScriptExecuteState::EXECUTE_PREVENT;
    }
};
#endif

#if VERSION_STRING < Cata
class MasterDemonologistDummy : public SpellScript
{
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply) override
    {
        if (aurEff->getEffectIndex() != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        const auto* const scriptedSpellR4 = sSpellMgr.getSpellInfo(SPELL_MASTER_DEMONOLOGIST_SCRIPTED_R4);
        const auto* scriptedSpell = sSpellMgr.getEquivalentSpellRankFor(aur->getSpellInfo(), scriptedSpellR4);
#if VERSION_STRING == Classic
        if (aur->getSpellId() == SPELL_MASTER_DEMONOLOGIST_DUMMY_R5)
        {
            // Classic is missing rank 5 scripted spell from Spell.dbc
            // Use rank 4 here and handle this in scripted spell effect
            scriptedSpell = scriptedSpellR4;
        }
#endif
        if (scriptedSpell == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const plrOwner = aur->getPlayerOwner();
        if (plrOwner == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        if (apply)
            plrOwner->addPetAuraSpell(scriptedSpell->getId(), true);
        else
            plrOwner->removePetAuraSpell(scriptedSpell->getId());

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

class MasterDemonologist : public SpellScript
{
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const minion = spell->getUnitCaster();
        if (minion == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        auto* const warlock = spell->getUnitTarget();
        if (warlock == nullptr)
            return SpellScriptCheckDummy::DUMMY_OK;

        uint8_t spellRank = 1;
        if (spell->getSpellInfo()->hasSpellRanks())
        {
            spellRank = spell->getSpellInfo()->getRankInfo()->getRank();

#if VERSION_STRING == Classic
            // In Classic rank 5 is missing so rank 4 was forced from dummy spell
            if (spellRank == 4)
            {
                // Get real rank from dummy effect in this case
                // There can be only one dummy rank active
                if (const auto* dummyEffect = warlock->getAuraEffectWithAnyRankOfAndByEffect(SPELL_AURA_DUMMY, SPELL_MASTER_DEMONOLOGIST_DUMMY_R5))
                {
                    if (const auto* rankInfo = dummyEffect->getAura()->getSpellInfo()->getRankInfo())
                        spellRank = rankInfo->getRank();
                }
            }
#endif
        }

        uint32_t spellId = 0;
        switch (minion->getCreatedBySpellId())
        {
            case SPELL_SUMMON_IMP:
                spellId = SPELL_MASTER_DEMONOLOGIST_IMP_R1;
                break;
            case SPELL_SUMMON_VOIDWALKER:
                spellId = SPELL_MASTER_DEMONOLOGIST_VOIDWALKER_R1;
                break;
            case SPELL_SUMMON_SUCCUBUS:
                spellId = SPELL_MASTER_DEMONOLOGIST_SUCCUBUS_R1;
                break;
            case SPELL_SUMMON_FELHUNTER:
                spellId = SPELL_MASTER_DEMONOLOGIST_FELHUNTER_R1;
                break;
            case SPELL_SUMMON_FELGUARD:
                spellId = SPELL_MASTER_DEMONOLOGIST_FELGUARD_R1;
                break;
            default:
                break;
        }

        if (const auto* effectInfo = sSpellMgr.getSpellInfo(spellId))
        {
            if (const auto* rankInfo = effectInfo->getRankInfo())
                effectInfo = rankInfo->getSpellWithRank(spellRank);

            warlock->castSpell(warlock, effectInfo, true);
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

#if VERSION_STRING < WotLK
class MasterDemonologistResistance : public SpellScript
{
    bool isFelguardSpell(SpellInfo const* spellInfo) const
    {
        if (spellInfo->hasSpellRanks())
            return spellInfo->getRankInfo()->getFirstSpell()->getId() == SPELL_MASTER_DEMONOLOGIST_FELGUARD_R1;

        return spellInfo->getId() == SPELL_MASTER_DEMONOLOGIST_FELGUARD_R1;
    }

    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* damage) override
    {
        const auto* unitCaster = spell->getUnitCaster();
        if (unitCaster == nullptr)
            return SpellScriptEffectDamage::DAMAGE_DEFAULT;

        const uint8_t spellRank = spell->getSpellInfo()->hasSpellRanks() ?
            spell->getSpellInfo()->getRankInfo()->getRank() :
            1;
        if (isFelguardSpell(spell->getSpellInfo()))
        {
            if (effIndex != EFF_INDEX_1)
                return SpellScriptEffectDamage::DAMAGE_DEFAULT;

            // Rank 1 increases all resistances by .1 per level
            *damage = unitCaster->getLevel() * (1 * spellRank) / 10;
        }
        else
        {
            if (effIndex != EFF_INDEX_0)
                return SpellScriptEffectDamage::DAMAGE_DEFAULT;

            // Rank 1 increases all resistances by .2 per level.
            *damage = unitCaster->getLevel() * (2 * spellRank) / 10;
        }

        return SpellScriptEffectDamage::DAMAGE_DEFAULT;
    }
};
#endif
#endif

#if VERSION_STRING < Mop
class NightfallDummy : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool apply) override
    {
        if (apply)
        {
            // Should proc on Corruption and Drain Life
            const uint32_t procMask[3] = { 0xA, 0, 0 };
            aur->getOwner()->addProcTriggerSpell(sSpellMgr.getSpellInfo(SPELL_SHADOW_TRANCE_PROC), aur, aur->getCasterGuid(), procMask);
        }
        else
        {
            aur->getOwner()->removeProcTriggerSpell(SPELL_SHADOW_TRANCE_PROC, aur->getCasterGuid());
        }

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};
#endif

class ShadowTrance : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override
    {
        // SpellInfo is missing charge count
        aur->setCharges(1, false);
    }

    void onCreateSpellProc(SpellProc* proc, Object* /*obj*/) override
    {
        proc->setCastedOnProcOwner(true);
    }

    bool canProc(SpellProc* proc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo damageInfo) override
    {
        // Cannot proc if warlock already has the aura
        if (proc->getProcOwner()->hasAurasWithId(SPELL_SHADOW_TRANCE_PROC))
            return false;

        // Should proc only when dealing damage
        if (damageInfo.realDamage == 0)
            return false;

        return true;
    }
};

class SoulLinkDummy : public SpellScript
{
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override
    {
        if (effIndex != EFF_INDEX_0)
            return SpellScriptCheckDummy::DUMMY_OK;

        // Effect has pet target so no need to do further checks
        if (auto* const unitTarget = spell->getUnitTarget())
            unitTarget->castSpell(unitTarget, SPELL_SOUL_LINK, true);

        return SpellScriptCheckDummy::DUMMY_OK;
    }
};

void setupWarlockSpells(ScriptMgr* mgr)
{
    // Call legacy script setup
    SetupLegacyWarlockSpells(mgr);

#if VERSION_STRING >= TBC && VERSION_STRING < Legion
    mgr->register_spell_script(SPELL_BACKLASH_PROC, new Backlash);
#endif

#if VERSION_STRING == TBC || VERSION_STRING == WotLK
    mgr->register_spell_script(SPELL_DEMONIC_KNOWLEDGE_DUMMY_R1, new DemonicKnowledgeDummy);
    mgr->register_spell_script(SPELL_DEMONIC_KNOWLEDGE, new DemonicKnowledge);
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_DEMONIC_SACRIFICE, new DemonicSacrifice);
    uint32_t demonicSacrificeAuras[] =
    {
#if VERSION_STRING >= TBC
        SPELL_DEMONIC_SACRIFICE_FELGUARD,
#endif
        SPELL_DEMONIC_SACRIFICE_FELHUNTER,
        SPELL_DEMONIC_SACRIFICE_IMP,
        SPELL_DEMONIC_SACRIFICE_SUCCUBUS,
        SPELL_DEMONIC_SACRIFICE_VOIDWALKER,
        0
    };
    mgr->register_spell_script(demonicSacrificeAuras, new DemonicSacrificeEffect);
#endif

#if VERSION_STRING < Cata
    mgr->register_spell_script(SPELL_MASTER_DEMONOLOGIST_DUMMY_R5, new MasterDemonologistDummy);
    mgr->register_spell_script(SPELL_MASTER_DEMONOLOGIST_SCRIPTED_R4, new MasterDemonologist);
#if VERSION_STRING < WotLK
    uint32_t masterDemonologistResistanceAuras[] =
    {
#if VERSION_STRING == TBC
        SPELL_MASTER_DEMONOLOGIST_FELGUARD_R1,
#endif
        SPELL_MASTER_DEMONOLOGIST_FELHUNTER_R1,
        0
    };
    mgr->register_spell_script(masterDemonologistResistanceAuras, new MasterDemonologistResistance);
#endif
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_NIGHTFALL_R1, new NightfallDummy);
#endif

#if VERSION_STRING < Mop
    mgr->register_spell_script(SPELL_SOUL_LINK_DUMMY, new SoulLinkDummy);
#endif

    mgr->register_spell_script(SPELL_SHADOW_TRANCE_PROC, new ShadowTrance);
}
