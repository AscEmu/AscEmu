/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellMgr.hpp"

#include "Spell/Spell.hpp"
#include "Spell/SpellInfo.hpp"
#include "SpellAura.hpp"
#include "Logging/Logger.hpp"
#include "Spell/Definitions/AuraEffects.hpp"
#include "Spell/Definitions/SpellDamageType.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Spell/Definitions/SpellFamily.hpp"
#include "Map/Area/AreaStorage.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Definitions.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#if VERSION_STRING < WotLK
#include "Definitions/SpellEffectTarget.hpp"
#endif

#if VERSION_STRING < Cata
#include "Server/World.h"
#endif

bool SpellArea::fitsToRequirements(Player* player, uint32_t newZone, uint32_t newArea) const
{
    if (gender != GENDER_NONE)
    {
        if (player == nullptr || gender != player->getGender())
            return false;
    }

    if (raceMask != 0)
    {
        if (player == nullptr || !(raceMask & player->getRaceMask()))
            return false;
    }

    if (areaId != 0)
    {
        if (newZone != areaId && newArea != areaId)
            return false;
    }

    if (questStart != 0)
    {
        if (player == nullptr || !player->hasQuestInQuestLog(questStart))
            return false;
    }

    if (questEnd != 0)
    {
        if (player == nullptr || player->hasQuestFinished(questEnd))
            return false;
    }

    if (auraSpell != 0)
    {
        const auto auraId = static_cast<uint32_t>(std::abs(auraSpell));
        if (player == nullptr || (auraSpell > 0 && !player->hasAurasWithId(auraId)) || (auraSpell < 0 && player->hasAurasWithId(auraId)))
            return false;
    }

    // Misc Conditions
    switch (spellId)
    {
        case 58600: // Restricted Flight Zone (Dalaran)
        {
            if (player == nullptr)
                return false;

#if VERSION_STRING >= TBC
            if (!player->hasAuraWithAuraEffect(SPELL_AURA_ENABLE_FLIGHT2) && !player->hasAuraWithAuraEffect(SPELL_AURA_FLY))
                return false;
#endif
            break;
        }
    }

    return true;
}

SpellMgr::SpellMgr() = default;
SpellMgr::~SpellMgr() = default;

SpellMgr& SpellMgr::getInstance()
{
    static SpellMgr mInstance;
    return mInstance;
}

void SpellMgr::initialize()
{
    // Load spell data from DBC
    loadSpellInfoData();

    for (auto& itr : mSpellInfoMapStore)
    {
        const auto& spellInfo = itr.second;

        // Custom values
        // todo: if possible, get rid of these
        setSpellEffectAmplitude(spellInfo.get());
        setSpellMissingCIsFlags(spellInfo.get());
    }

    // Hackfixes
    applyHackFixes();
}

void SpellMgr::finalize()
{
    sLogger.info("SpellMgr : Cleaning up SpellMgr...");

    mSpellTargetConstraintMap.clear();
    mSpellInfoMapStore.clear();
}

void SpellMgr::loadSpellDataFromDatabase()
{
    // Load spell related SQL tables
    loadSpellRanks();
    loadSpellCustomOverride();
    loadSpellCoefficientOverride();
    loadSpellAIThreat();
    loadSpellEffectOverride();
    loadSpellAreas();
    loadSpellRequired();
    loadSpellTargetConstraints();
    loadSpellDisabled();

    // Load skill DBC files
    loadSkillLineAbilityMap();
}

void SpellMgr::calculateSpellCoefficients()
{
    for (auto& itr : mSpellInfoMapStore)
    {
        const auto& spellInfo = itr.second;
        setSpellCoefficient(spellInfo.get());
    }
}

void SpellMgr::loadSpellScripts()
{
    // Setup legacy scripts
    setupSpellScripts();
}

Spell* SpellMgr::newSpell(Object* caster, SpellInfo const* info, bool triggered, Aura* aur)
{
    if (info == nullptr)
    {
        sLogger.failure("You tried to create a Spell without SpellInfo. This is not possible!");
        return nullptr;
    }

    // Spells with legacy script
    if (info->spellScriptLink != nullptr)
        return (*SpellScriptLinker(info->spellScriptLink))(caster, getMutableSpellInfo(info->getId()), triggered, aur);

    // Standard spells without a script
    return new Spell(caster, info, triggered, aur);
}

std::unique_ptr<Aura> SpellMgr::newAura(SpellInfo const* spellInfo, int32_t duration, Object* caster, Unit* target, bool temporary /*= false*/, Item* i_caster /*= nullptr*/)
{
    //\brief... nullptr when downgrading ae from wotlk to tbc (active auras from newer client versions should be removed before entering tbc)

    if (spellInfo == nullptr)
    {
        sLogger.failure("You tried to create an Aura without SpellInfo. This is not possible!");
        return nullptr;
    }

    // Auras with a script
    if (spellInfo->auraScriptLink != nullptr)
        return (*AuraScriptLinker(spellInfo->auraScriptLink))(getMutableSpellInfo(spellInfo->getId()), duration, caster, target, temporary, i_caster);
    // Auras with absorb effect
    else if (spellInfo->hasEffectApplyAuraName(SPELL_AURA_SCHOOL_ABSORB))
        return (*AuraScriptLinker(&AbsorbAura::Create))(getMutableSpellInfo(spellInfo->getId()), duration, caster, target, temporary, i_caster);
    
    // Standard auras without a script
    return std::make_unique<Aura>(spellInfo, duration, caster, target, temporary, i_caster);
}

void SpellMgr::addSpellById(const uint32_t spellId, SpellScriptLinker spellScript)
{
    auto spellInfo = getMutableSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        sLogger.failure("SpellMgr::addSpellById : Unknown spell id {} tried to register a spell script, skipped", spellId);
        return;
    }
    addSpellBySpellInfo(spellInfo, spellScript);
}

void SpellMgr::addAuraById(const uint32_t spellId, AuraScriptLinker auraScript)
{
    auto spellInfo = getMutableSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        sLogger.failure("SpellMgr::addAuraById : Unknown spell id {} tried to register an aura script, skipped", spellId);
        return;
    }
    addAuraBySpellInfo(spellInfo, auraScript);
}

SpellMechanic const* SpellMgr::getCrowdControlMechanicList([[maybe_unused]]bool includeSilence) const
{
#if VERSION_STRING >= Cata
    if (includeSilence)
    {
        static SpellMechanic mechanicListWithSilence[18] =
        {
            MECHANIC_CHARMED,
            MECHANIC_DISORIENTED,
            MECHANIC_FLEEING,
            MECHANIC_ROOTED,
            MECHANIC_SILENCED,
            MECHANIC_ASLEEP,
            MECHANIC_ENSNARED,
            MECHANIC_STUNNED,
            MECHANIC_FROZEN,
            MECHANIC_INCAPACIPATED,
            MECHANIC_POLYMORPHED,
            MECHANIC_BANISHED,
            MECHANIC_SHACKLED,
            MECHANIC_SEDUCED,
            MECHANIC_TURNED,
            MECHANIC_HORRIFIED,
            MECHANIC_SAPPED,
            MECHANIC_NONE
        };

        return mechanicListWithSilence;
    }
    else
#endif
    {
        static SpellMechanic mechanicListWithoutSilence[17] =
        {
            MECHANIC_CHARMED,
            MECHANIC_DISORIENTED,
            MECHANIC_FLEEING,
            MECHANIC_ROOTED,
            MECHANIC_ASLEEP,
            MECHANIC_ENSNARED,
            MECHANIC_STUNNED,
            MECHANIC_FROZEN,
            MECHANIC_INCAPACIPATED,
            MECHANIC_POLYMORPHED,
            MECHANIC_BANISHED,
            MECHANIC_SHACKLED,
            MECHANIC_SEDUCED,
            MECHANIC_TURNED,
            MECHANIC_HORRIFIED,
            MECHANIC_SAPPED,
            MECHANIC_NONE
        };

        return mechanicListWithoutSilence;
    }
}

SpellRequiredMapRange SpellMgr::getSpellsRequiredRangeForSpell(uint32_t spellId) const
{
    const auto [begin, end] = mSpellRequired.equal_range(spellId);
    return std::ranges::subrange(begin, end);
}

SpellsRequiringSpellMap SpellMgr::getSpellsRequiringSpell() const
{
    return mSpellsRequiringSpell;
}

SpellsRequiringSpellMapRange SpellMgr::getSpellsRequiringSpellRange(uint32_t spellId) const
{
    const auto [begin, end] = mSpellsRequiringSpell.equal_range(spellId);
    return std::ranges::subrange(begin, end);
}

bool SpellMgr::isSpellRequiringSpell(uint32_t spellId, uint32_t requiredSpellId) const
{
    auto spellsRequiringSpell = getSpellsRequiringSpellRange(requiredSpellId);
    for (const auto& itr : spellsRequiringSpell)
    {
        if (itr.second == spellId)
            return true;
    }

    return false;
}

uint32_t SpellMgr::getSpellRequired(uint32_t spellId) const
{
    auto itr = mSpellRequired.find(spellId);
    if (itr == mSpellRequired.end())
        return 0;

    return itr->second;
}

bool SpellMgr::isSpellDisabled(uint32_t spellId) const
{
    return mDisabledSpells.find(spellId) != mDisabledSpells.end();
}

void SpellMgr::reloadSpellDisabled()
{
    mDisabledSpells.clear();
    loadSpellDisabled();
}

SpellSkillAbilityMapRange SpellMgr::getSkillEntryRangeForSpell(uint32_t spellId) const
{
    const auto [begin, end] = mSpellSkillsMap.equal_range(spellId);
    return std::ranges::subrange(begin, end);
}

SkillSkillAbilityMapRange SpellMgr::getSkillEntryRangeForSkill(uint16_t skillId) const
{
    const auto [begin, end] = mSkillSpellsMap.equal_range(skillId);
    return std::ranges::subrange(begin, end);
}

WDB::Structures::SkillLineAbilityEntry const* SpellMgr::getFirstSkillEntryForSpell(uint32_t spellId, Player const* forPlayer/* = nullptr*/) const
{
    const auto spellSkillRange = getSkillEntryRangeForSpell(spellId);
    for (const auto& [_, skillEntry] : spellSkillRange)
    {
        if (forPlayer != nullptr)
        {
            if (skillEntry->race_mask != 0 && !(skillEntry->race_mask & forPlayer->getRaceMask()))
                continue;

            if (skillEntry->class_mask != 0 && !(skillEntry->class_mask & forPlayer->getClassMask()))
                continue;
        }

        return skillEntry;
    }

    return nullptr;
}

SpellTargetConstraint const* SpellMgr::getSpellTargetConstraintForSpell(uint32_t spellId) const
{
    const auto itr = mSpellTargetConstraintMap.find(spellId);
    if (itr == mSpellTargetConstraintMap.end())
        return nullptr;

    return itr->second.get();
}

SpellAreaMapBounds SpellMgr::getSpellAreaMapBounds(uint32_t spellId) const
{
    return SpellAreaMapBounds(mSpellAreaMap.lower_bound(spellId), mSpellAreaMap.upper_bound(spellId));
}

SpellAreaForAreaMapBounds SpellMgr::getSpellAreaForAreaMapBounds(uint32_t areaId) const
{
    return SpellAreaForAreaMapBounds(mSpellAreaForAreaMap.lower_bound(areaId), mSpellAreaForAreaMap.upper_bound(areaId));
}

SpellAreaForAuraMapBounds SpellMgr::getSpellAreaForAuraMapBounds(uint32_t spellId) const
{
    return SpellAreaForAuraMapBounds(mSpellAreaForAuraMap.lower_bound(spellId), mSpellAreaForAuraMap.upper_bound(spellId));
}

SpellAreaForQuestMapBounds SpellMgr::getSpellAreaForQuestMapBounds(uint32_t questId, bool active) const
{
    if (active)
        return SpellAreaForQuestMapBounds(mSpellAreaForActiveQuestMap.lower_bound(questId), mSpellAreaForQuestMap.upper_bound(questId));
    else
        return SpellAreaForQuestMapBounds(mSpellAreaForQuestMap.lower_bound(questId), mSpellAreaForQuestMap.upper_bound(questId));
}

SpellAreaForQuestMapBounds SpellMgr::getSpellAreaForQuestEndMapBounds(uint32_t questId) const
{
    return SpellAreaForQuestMapBounds(mSpellAreaForQuestEndMap.lower_bound(questId), mSpellAreaForQuestEndMap.upper_bound(questId));
}

bool SpellMgr::checkLocation(SpellInfo const* spellInfo, uint32_t zone_id, uint32_t area_id, Player* player) const
{
    if (spellInfo == nullptr)
        return false;

    // Normal case
    if (spellInfo->getRequiresAreaId() > 0)
    {
        const auto requireAreaId = static_cast<uint32_t>(spellInfo->getRequiresAreaId());
#if VERSION_STRING == TBC
        if (requireAreaId != zone_id && requireAreaId != area_id)
            return false;
#elif VERSION_STRING >= WotLK
        auto found = false;
        auto areaGroup = sAreaGroupStore.lookupEntry(requireAreaId);
        while (areaGroup != nullptr)
        {
            for (uint8_t i = 0; i < 6; ++i)
            {
                if (areaGroup->AreaId[i] == zone_id || areaGroup->AreaId[i] == area_id)
                    found = true;
            }

            if (found || areaGroup->next_group == 0)
                break;

            // Try next group
            areaGroup = sAreaGroupStore.lookupEntry(areaGroup->next_group);
        }

        if (!found)
            return false;
#endif
    }

    // Check for area requirements set in database
    const auto spellAreaMapBounds = getSpellAreaMapBounds(spellInfo->getId());
    if (spellAreaMapBounds.first != spellAreaMapBounds.second)
    {
        for (SpellAreaMap::const_iterator itr = spellAreaMapBounds.first; itr != spellAreaMapBounds.second; ++itr)
        {
            if (itr->second.fitsToRequirements(player, zone_id, area_id))
                return true;
        }
    }

    return false;
}

SpellInfo const* SpellMgr::getSpellInfo(const uint32_t spellId) const
{
    if (spellId == 0)
        return nullptr;

    const auto itr = mSpellInfoMapStore.find(spellId);
    if (itr != mSpellInfoMapStore.end())
        return itr->second.get();

    return nullptr;
}

SpellInfo const* SpellMgr::getSpellInfoByDifficulty([[maybe_unused]]const uint32_t spellDifficultyId, [[maybe_unused]] const uint8_t difficulty) const
{
    // Future request: create a new sql table to contain missing spell difficulties
    // it will also be useful for tbc since the dbc file does not exist there
    // Classic has no different difficulties so this will always return nullptr there
#if VERSION_STRING >= WotLK
    const auto spellDifficulty = sSpellDifficultyStore.lookupEntry(spellDifficultyId);
    if (spellDifficulty == nullptr)
        return nullptr;

    // difficulty 0 = normal 5 man dungeon or normal 10 man raid
    // difficulty 1 = heroic 5 man dungeon or normal 25 man raid
    // difficulty 2 = heroic 10 man raid
    // difficulty 3 = heroic 25 man raid
    if (spellDifficulty->SpellId[difficulty] <= 0)
        return nullptr;

    return getSpellInfo(static_cast<uint32_t>(spellDifficulty->SpellId[difficulty]));
#else
    return nullptr;
#endif
}

// Private methods

void SpellMgr::loadSpellInfoData()
{
    for (uint32_t i = 0; i < MAX_SPELL_ID; ++i)
    {
        const auto dbcSpellEntry = sSpellStore.lookupEntry(i);
        if (dbcSpellEntry == nullptr)
            continue;

        auto spell_id = dbcSpellEntry->Id;
        const auto [spellItr, _] = mSpellInfoMapStore.try_emplace(spell_id, Util::LazyInstanceCreator([] {
            return std::make_unique<SpellInfo>();
        }));
        const auto& spellInfo = spellItr->second;

#if VERSION_STRING == Mop

        spellInfo->setId(spell_id);
        spellInfo->setAttributes(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->Attributes : 0);
        spellInfo->setAttributesEx(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesEx : 0);
        spellInfo->setAttributesExB(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExB : 0);
        spellInfo->setAttributesExC(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExC : 0);
        spellInfo->setAttributesExD(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExD : 0);
        spellInfo->setAttributesExE(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExE : 0);
        spellInfo->setAttributesExF(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExF : 0);
        spellInfo->setAttributesExG(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExG : 0);
        spellInfo->setCastingTimeIndex(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->CastingTimeIndex : 0);
        spellInfo->setDurationIndex(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->DurationIndex : 0);
        spellInfo->setPowerType(static_cast<PowerType>(dbcSpellEntry->GetSpellPower() ? dbcSpellEntry->GetSpellPower()->powerType : 0));
        spellInfo->setRangeIndex(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->rangeIndex : 0);
        spellInfo->setSpeed(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->speed : 0);
        spellInfo->setSpellVisual(0, dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->SpellVisual : 0);
        spellInfo->setSpellVisual(1, dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->SpellVisual1 : 0);
        spellInfo->setSpellIconID(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->spellIconID : 0);
        spellInfo->setActiveIconID(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->activeIconID : 0);
        spellInfo->setSchoolMask(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->School : 0);
        spellInfo->setRuneCostID(dbcSpellEntry->RuneCostID);
        spellInfo->setSpellDifficultyID(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->SpellDifficultyId : 0);
        spellInfo->setAttributesExH(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExH : 0);
        spellInfo->setAttributesExI(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExI : 0);
        spellInfo->setAttributesExJ(dbcSpellEntry->GetSpellMisc() ? dbcSpellEntry->GetSpellMisc()->AttributesExJ : 0);

        spellInfo->setName(dbcSpellEntry->Name);
        spellInfo->setRank(dbcSpellEntry->Rank);

        // Initialize DBC links
        spellInfo->SpellScalingId = dbcSpellEntry->SpellScalingId;
        spellInfo->SpellAuraOptionsId = dbcSpellEntry->SpellAuraOptionsId;
        spellInfo->SpellAuraRestrictionsId = dbcSpellEntry->SpellAuraRestrictionsId;
        spellInfo->SpellCastingRequirementsId = dbcSpellEntry->SpellCastingRequirementsId;
        spellInfo->SpellCategoriesId = dbcSpellEntry->SpellCategoriesId;
        spellInfo->SpellClassOptionsId = dbcSpellEntry->SpellClassOptionsId;
        spellInfo->SpellCooldownsId = dbcSpellEntry->SpellCooldownsId;
        spellInfo->SpellEquippedItemsId = dbcSpellEntry->SpellEquippedItemsId;
        spellInfo->SpellInterruptsId = dbcSpellEntry->SpellInterruptsId;
        spellInfo->SpellLevelsId = dbcSpellEntry->SpellLevelsId;
        spellInfo->SpellPowerId = dbcSpellEntry->Id;
        spellInfo->SpellReagentsId = dbcSpellEntry->SpellReagentsId;
        spellInfo->SpellShapeshiftId = dbcSpellEntry->SpellShapeshiftId;
        spellInfo->SpellTargetRestrictionsId = dbcSpellEntry->SpellTargetRestrictionsId;
        spellInfo->SpellTotemsId = dbcSpellEntry->SpellTotemsId;

        // Data from SpellAuraOptions.dbc
        if (dbcSpellEntry->SpellAuraOptionsId && dbcSpellEntry->GetSpellAuraOptions() != nullptr)
        {
            spellInfo->setMaxstack(dbcSpellEntry->GetSpellAuraOptions()->MaxStackAmount);
            spellInfo->setProcChance(dbcSpellEntry->GetSpellAuraOptions()->procChance);
            spellInfo->setProcCharges(dbcSpellEntry->GetSpellAuraOptions()->procCharges);
            spellInfo->setProcFlags(dbcSpellEntry->GetSpellAuraOptions()->procFlags);
        }

        // Data from SpellAuraRestrictions.dbc
        if (dbcSpellEntry->SpellAuraRestrictionsId && dbcSpellEntry->GetSpellAuraRestrictions() != nullptr)
        {
            spellInfo->setCasterAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraState);
            spellInfo->setTargetAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraState);
            spellInfo->setCasterAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraStateNot);
            spellInfo->setTargetAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraStateNot);
            spellInfo->setCasterAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->casterAuraSpell);
            spellInfo->setTargetAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->targetAuraSpell);
            spellInfo->setCasterAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraSpellNot);
            spellInfo->setTargetAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraSpellNot);
        }

        // Data from SpellCastingRequirements.dbc
        if (dbcSpellEntry->SpellCastingRequirementsId && dbcSpellEntry->GetSpellCastingRequirements() != nullptr)
        {
            spellInfo->setFacingCasterFlags(dbcSpellEntry->GetSpellCastingRequirements()->FacingCasterFlags);
            spellInfo->setRequiresAreaId(dbcSpellEntry->GetSpellCastingRequirements()->AreaGroupId);
            spellInfo->setRequiresSpellFocus(dbcSpellEntry->GetSpellCastingRequirements()->RequiresSpellFocus);
        }

        // Data from SpellCategories.dbc
        if (dbcSpellEntry->SpellCategoriesId && dbcSpellEntry->GetSpellCategories() != nullptr)
        {
            spellInfo->setCategory(dbcSpellEntry->GetSpellCategories()->Category);
            spellInfo->setDispelType(dbcSpellEntry->GetSpellCategories()->DispelType);
            spellInfo->setDmgClass(dbcSpellEntry->GetSpellCategories()->DmgClass);
            spellInfo->setMechanicsType(dbcSpellEntry->GetSpellCategories()->MechanicsType);
            spellInfo->setPreventionType(dbcSpellEntry->GetSpellCategories()->PreventionType);
            spellInfo->setStartRecoveryCategory(dbcSpellEntry->GetSpellCategories()->StartRecoveryCategory);
        }

        // Data from SpellClassOptions.dbc
        if (dbcSpellEntry->SpellClassOptionsId && dbcSpellEntry->GetSpellClassOptions() != nullptr)
        {
            spellInfo->setSpellFamilyName(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyName);
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo->setSpellFamilyFlags(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyFlags[j], j);
        }

        // Data from SpellCooldowns.dbc
        if (dbcSpellEntry->SpellCooldownsId && dbcSpellEntry->GetSpellCooldowns() != nullptr)
        {
            spellInfo->setCategoryRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->CategoryRecoveryTime);
            spellInfo->setRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->RecoveryTime);
            spellInfo->setStartRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->StartRecoveryTime);
        }

        // Data from SpellEquippedItems.dbc
        if (dbcSpellEntry->SpellEquippedItemsId && dbcSpellEntry->GetSpellEquippedItems() != nullptr)
        {
            spellInfo->setEquippedItemClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemClass);
            spellInfo->setEquippedItemInventoryTypeMask(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemInventoryTypeMask);
            spellInfo->setEquippedItemSubClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemSubClassMask);
        }

        // Data from SpellInterrupts.dbc
        if (dbcSpellEntry->SpellInterruptsId && dbcSpellEntry->GetSpellInterrupts() != nullptr)
        {
            spellInfo->setAuraInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->AuraInterruptFlags);
            spellInfo->setChannelInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->ChannelInterruptFlags);
            spellInfo->setInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->InterruptFlags);
        }

        // Data from SpellLevels.dbc
        if (dbcSpellEntry->SpellLevelsId && dbcSpellEntry->GetSpellLevels() != nullptr)
        {
            spellInfo->setBaseLevel(dbcSpellEntry->GetSpellLevels()->baseLevel);
            spellInfo->setMaxLevel(dbcSpellEntry->GetSpellLevels()->maxLevel);
            spellInfo->setSpellLevel(dbcSpellEntry->GetSpellLevels()->spellLevel);
        }

        // Data from SpellPower.dbc
        if (dbcSpellEntry->GetSpellPower() != nullptr)
        {
            spellInfo->setManaCost(dbcSpellEntry->GetSpellPower()->manaCost);
            spellInfo->setManaCostPerlevel(dbcSpellEntry->GetSpellPower()->manaCostPerlevel);
            spellInfo->setManaCostPercentage(dbcSpellEntry->GetSpellPower()->ManaCostPercentageFloat);
            spellInfo->setManaPerSecond(dbcSpellEntry->GetSpellPower()->manaPerSecond);
            spellInfo->setManaPerSecondPerLevel(dbcSpellEntry->GetSpellPower()->manaPerSecondPerLevel);
        }

        // Data from SpellReagents.db2
        if (dbcSpellEntry->SpellReagentsId && sSpellReagentsStore.lookupEntry(dbcSpellEntry->SpellReagentsId) != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
            {
                const auto spellReagent = sSpellReagentsStore.lookupEntry(dbcSpellEntry->SpellReagentsId);
                spellInfo->setReagent(spellReagent->Reagent[j], j);
                spellInfo->setReagentCount(spellReagent->ReagentCount[j], j);
            }
        }

        // Data from SpellShapeshift.dbc
        if (dbcSpellEntry->SpellShapeshiftId && dbcSpellEntry->GetSpellShapeshift() != nullptr)
        {
            spellInfo->setRequiredShapeShift(dbcSpellEntry->GetSpellShapeshift()->Shapeshifts);
            spellInfo->setShapeshiftExclude(dbcSpellEntry->GetSpellShapeshift()->ShapeshiftsExcluded);
        }

        // Data from SpellTargetRestrictions.dbc
        if (dbcSpellEntry->SpellTargetRestrictionsId && dbcSpellEntry->GetSpellTargetRestrictions() != nullptr)
        {
            spellInfo->setMaxTargets(dbcSpellEntry->GetSpellTargetRestrictions()->MaxAffectedTargets);
            spellInfo->setMaxTargetLevel(dbcSpellEntry->GetSpellTargetRestrictions()->MaxTargetLevel);
            spellInfo->setTargetCreatureType(dbcSpellEntry->GetSpellTargetRestrictions()->TargetCreatureType);
            spellInfo->setTargets(dbcSpellEntry->GetSpellTargetRestrictions()->Targets);
        }

        // Data from SpellTotems.dbc
        if (dbcSpellEntry->SpellTotemsId && dbcSpellEntry->GetSpellTotems() != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
            {
                spellInfo->setTotemCategory(dbcSpellEntry->GetSpellTotems()->TotemCategory[j], j);
                spellInfo->setTotem(dbcSpellEntry->GetSpellTotems()->Totem[j], j);
            }
        }

        // Data from SpellEffect.dbc
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            const auto spell_effect_entry = GetSpellEffectEntry(spell_id, j);
            if (spell_effect_entry != nullptr)
            {
                spellInfo->setEffect(spell_effect_entry->Effect, j);
                spellInfo->setEffectMultipleValue(spell_effect_entry->EffectMultipleValue, j);
                spellInfo->setEffectApplyAuraName(spell_effect_entry->EffectApplyAuraName, j);
                spellInfo->setEffectAmplitude(spell_effect_entry->EffectAmplitude, j);
                spellInfo->setEffectBasePoints(spell_effect_entry->EffectBasePoints, j);
                spellInfo->setEffectBonusMultiplier(spell_effect_entry->EffectBonusMultiplier, j);
                spellInfo->setEffectDamageMultiplier(spell_effect_entry->EffectDamageMultiplier, j);
                spellInfo->setEffectChainTarget(spell_effect_entry->EffectChainTarget, j);
                spellInfo->setEffectDieSides(spell_effect_entry->EffectDieSides, j);
                spellInfo->setEffectItemType(spell_effect_entry->EffectItemType, j);
                spellInfo->setEffectMechanic(spell_effect_entry->EffectMechanic, j);
                spellInfo->setEffectMiscValue(spell_effect_entry->EffectMiscValue, j);
                spellInfo->setEffectMiscValueB(spell_effect_entry->EffectMiscValueB, j);
                spellInfo->setEffectPointsPerComboPoint(spell_effect_entry->EffectPointsPerComboPoint, j);
                spellInfo->setEffectRadiusIndex(spell_effect_entry->EffectRadiusIndex, j);
                spellInfo->setEffectRadiusMaxIndex(spell_effect_entry->EffectRadiusMaxIndex, j);
                spellInfo->setEffectRealPointsPerLevel(spell_effect_entry->EffectRealPointsPerLevel, j);
                for (uint8_t x = 0; x < 3; ++x)
                    spellInfo->setEffectSpellClassMask(spell_effect_entry->EffectSpellClassMask[x], j, x);
                spellInfo->setEffectTriggerSpell(spell_effect_entry->EffectTriggerSpell, j);
                spellInfo->setEffectImplicitTargetA(spell_effect_entry->EffectImplicitTargetA, j);
                spellInfo->setEffectImplicitTargetB(spell_effect_entry->EffectImplicitTargetB, j);
                spellInfo->setEffectSpellId(spell_effect_entry->EffectSpellId, j);
                spellInfo->setEffectIndex(spell_effect_entry->EffectIndex, j);
            }
        }
#else

        spellInfo->setId(spell_id);
        spellInfo->setAttributes(dbcSpellEntry->Attributes);
        spellInfo->setAttributesEx(dbcSpellEntry->AttributesEx);
        spellInfo->setAttributesExB(dbcSpellEntry->AttributesExB);
        spellInfo->setAttributesExC(dbcSpellEntry->AttributesExC);
        spellInfo->setAttributesExD(dbcSpellEntry->AttributesExD);
#if VERSION_STRING >= TBC
        spellInfo->setAttributesExE(dbcSpellEntry->AttributesExE);
        spellInfo->setAttributesExF(dbcSpellEntry->AttributesExF);
#endif
#if VERSION_STRING >= WotLK
        spellInfo->setAttributesExG(dbcSpellEntry->AttributesExG);
#endif
        spellInfo->setCastingTimeIndex(dbcSpellEntry->CastingTimeIndex);
        spellInfo->setDurationIndex(dbcSpellEntry->DurationIndex);
        spellInfo->setPowerType(static_cast<PowerType>(dbcSpellEntry->powerType));
        spellInfo->setRangeIndex(dbcSpellEntry->rangeIndex);
        spellInfo->setSpeed(dbcSpellEntry->speed);
        spellInfo->setSpellVisual(0, dbcSpellEntry->SpellVisual);
        spellInfo->setSpellVisual(1, dbcSpellEntry->SpellVisual1);
        spellInfo->setSpellIconID(dbcSpellEntry->spellIconID);
        spellInfo->setActiveIconID(dbcSpellEntry->activeIconID);
#if VERSION_STRING == Classic
        // Classic doesn't have schools bitwise in DBC
        spellInfo->setSchoolMask(1 << dbcSpellEntry->School);
#else
        spellInfo->setSchoolMask(dbcSpellEntry->School);
#endif
#if VERSION_STRING >= WotLK
        spellInfo->setRuneCostID(dbcSpellEntry->RuneCostID);
        spellInfo->setSpellDifficultyID(dbcSpellEntry->SpellDifficultyId);
#endif

#if VERSION_STRING < Cata
        spellInfo->setCategory(dbcSpellEntry->Category);
        spellInfo->setDispelType(dbcSpellEntry->DispelType);
        spellInfo->setMechanicsType(dbcSpellEntry->MechanicsType);
        spellInfo->setRequiredShapeShift(dbcSpellEntry->Shapeshifts);
        spellInfo->setShapeshiftExclude(dbcSpellEntry->ShapeshiftsExcluded);
        spellInfo->setTargets(dbcSpellEntry->Targets);
        spellInfo->setTargetCreatureType(dbcSpellEntry->TargetCreatureType);
        spellInfo->setRequiresSpellFocus(dbcSpellEntry->RequiresSpellFocus);
#if VERSION_STRING >= TBC
        spellInfo->setFacingCasterFlags(dbcSpellEntry->FacingCasterFlags);
#endif
        spellInfo->setCasterAuraState(dbcSpellEntry->CasterAuraState);
        spellInfo->setTargetAuraState(dbcSpellEntry->TargetAuraState);
#if VERSION_STRING >= TBC
        spellInfo->setCasterAuraStateNot(dbcSpellEntry->CasterAuraStateNot);
        spellInfo->setTargetAuraStateNot(dbcSpellEntry->TargetAuraStateNot);
#endif
#if VERSION_STRING == WotLK
        spellInfo->setCasterAuraSpell(dbcSpellEntry->casterAuraSpell);
        spellInfo->setTargetAuraSpell(dbcSpellEntry->targetAuraSpell);
        spellInfo->setCasterAuraSpellNot(dbcSpellEntry->casterAuraSpellNot);
        spellInfo->setTargetAuraSpellNot(dbcSpellEntry->targetAuraSpellNot);
#endif
        spellInfo->setRecoveryTime(dbcSpellEntry->RecoveryTime);
        spellInfo->setCategoryRecoveryTime(dbcSpellEntry->CategoryRecoveryTime);
        spellInfo->setInterruptFlags(dbcSpellEntry->InterruptFlags);
        spellInfo->setAuraInterruptFlags(dbcSpellEntry->AuraInterruptFlags);
        spellInfo->setChannelInterruptFlags(dbcSpellEntry->ChannelInterruptFlags);
        spellInfo->setProcFlags(dbcSpellEntry->procFlags);
        spellInfo->setProcChance(dbcSpellEntry->procChance);
        spellInfo->setProcCharges(dbcSpellEntry->procCharges);
        spellInfo->setMaxLevel(dbcSpellEntry->maxLevel);
        spellInfo->setBaseLevel(dbcSpellEntry->baseLevel);
        spellInfo->setSpellLevel(dbcSpellEntry->spellLevel);
        spellInfo->setManaCost(dbcSpellEntry->manaCost);
        spellInfo->setManaCostPerlevel(dbcSpellEntry->manaCostPerlevel);
        spellInfo->setManaPerSecond(dbcSpellEntry->manaPerSecond);
        spellInfo->setManaPerSecondPerLevel(dbcSpellEntry->manaPerSecondPerLevel);
        spellInfo->setMaxstack(dbcSpellEntry->MaxStackAmount);
        for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
            spellInfo->setTotem(dbcSpellEntry->Totem[j], j);
        for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
        {
            spellInfo->setReagent(dbcSpellEntry->Reagent[j], j);
            spellInfo->setReagentCount(dbcSpellEntry->ReagentCount[j], j);
        }
        spellInfo->setEquippedItemClass(dbcSpellEntry->EquippedItemClass);
        spellInfo->setEquippedItemSubClass(dbcSpellEntry->EquippedItemSubClass);
        spellInfo->setEquippedItemInventoryTypeMask(dbcSpellEntry->EquippedItemInventoryTypeMask);
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            spellInfo->setEffect(dbcSpellEntry->Effect[j], j);
            spellInfo->setEffectDieSides(dbcSpellEntry->EffectDieSides[j], j);
            spellInfo->setEffectRealPointsPerLevel(dbcSpellEntry->EffectRealPointsPerLevel[j], j);
            spellInfo->setEffectBasePoints(dbcSpellEntry->EffectBasePoints[j], j);
            spellInfo->setEffectMechanic(dbcSpellEntry->EffectMechanic[j], j);
            spellInfo->setEffectImplicitTargetA(dbcSpellEntry->EffectImplicitTargetA[j], j);
            spellInfo->setEffectImplicitTargetB(dbcSpellEntry->EffectImplicitTargetB[j], j);
            spellInfo->setEffectRadiusIndex(dbcSpellEntry->EffectRadiusIndex[j], j);
            spellInfo->setEffectApplyAuraName(dbcSpellEntry->EffectApplyAuraName[j], j);
            spellInfo->setEffectAmplitude(dbcSpellEntry->EffectAmplitude[j], j);
            spellInfo->setEffectMultipleValue(dbcSpellEntry->EffectMultipleValue[j], j);
            spellInfo->setEffectChainTarget(dbcSpellEntry->EffectChainTarget[j], j);
            spellInfo->setEffectItemType(dbcSpellEntry->EffectItemType[j], j);
            spellInfo->setEffectMiscValue(dbcSpellEntry->EffectMiscValue[j], j);
#if VERSION_STRING >= TBC
            spellInfo->setEffectMiscValueB(dbcSpellEntry->EffectMiscValueB[j], j);
#endif
            spellInfo->setEffectTriggerSpell(dbcSpellEntry->EffectTriggerSpell[j], j);
            spellInfo->setEffectPointsPerComboPoint(dbcSpellEntry->EffectPointsPerComboPoint[j], j);
#if VERSION_STRING == WotLK
            for (uint8_t x = 0; x < 3; ++x)
                spellInfo->setEffectSpellClassMask(dbcSpellEntry->EffectSpellClassMask[j][x], j, x);
#endif
        }
        spellInfo->setSpellPriority(dbcSpellEntry->spellPriority);
        spellInfo->setName(dbcSpellEntry->Name[sWorld.getDbcLocaleLanguageId()]);
        spellInfo->setRank(dbcSpellEntry->Rank[sWorld.getDbcLocaleLanguageId()]);
        spellInfo->setManaCostPercentage(dbcSpellEntry->ManaCostPercentage);
        spellInfo->setStartRecoveryCategory(dbcSpellEntry->StartRecoveryCategory);
        spellInfo->setStartRecoveryTime(dbcSpellEntry->StartRecoveryTime);
        spellInfo->setMaxTargetLevel(dbcSpellEntry->MaxTargetLevel);
        spellInfo->setSpellFamilyName(dbcSpellEntry->SpellFamilyName);
#if VERSION_STRING != WotLK
        for (uint8_t j = 0; j < 2; ++j)
            spellInfo->setSpellFamilyFlags(dbcSpellEntry->SpellFamilyFlags[j], j);
#else
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo->setSpellFamilyFlags(dbcSpellEntry->SpellFamilyFlags[j], j);
#endif
        spellInfo->setMaxTargets(dbcSpellEntry->MaxTargets);
        spellInfo->setDmgClass(dbcSpellEntry->DmgClass);
        spellInfo->setPreventionType(dbcSpellEntry->PreventionType);
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo->setEffectDamageMultiplier(dbcSpellEntry->EffectDamageMultiplier[j], j);
#if VERSION_STRING >= TBC
        for (uint8_t j = 0; j < MAX_SPELL_TOTEM_CATEGORIES; ++j)
            spellInfo->setTotemCategory(dbcSpellEntry->TotemCategory[j], j);
        spellInfo->setRequiresAreaId(dbcSpellEntry->AreaGroupId);
#endif
#if VERSION_STRING == WotLK
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo->setEffectBonusMultiplier(dbcSpellEntry->EffectBonusMultiplier[j], j);
#endif
        // Cataclysm and MoP begins
#else
        spellInfo->setAttributesExH(dbcSpellEntry->AttributesExH);
        spellInfo->setAttributesExI(dbcSpellEntry->AttributesExI);
        spellInfo->setAttributesExJ(dbcSpellEntry->AttributesExJ);

        spellInfo->setName(dbcSpellEntry->Name);
        spellInfo->setRank(dbcSpellEntry->Rank);

        // Initialize DBC links
        spellInfo->SpellScalingId = dbcSpellEntry->SpellScalingId;
        spellInfo->SpellAuraOptionsId = dbcSpellEntry->SpellAuraOptionsId;
        spellInfo->SpellAuraRestrictionsId = dbcSpellEntry->SpellAuraRestrictionsId;
        spellInfo->SpellCastingRequirementsId = dbcSpellEntry->SpellCastingRequirementsId;
        spellInfo->SpellCategoriesId = dbcSpellEntry->SpellCategoriesId;
        spellInfo->SpellClassOptionsId = dbcSpellEntry->SpellClassOptionsId;
        spellInfo->SpellCooldownsId = dbcSpellEntry->SpellCooldownsId;
        spellInfo->SpellEquippedItemsId = dbcSpellEntry->SpellEquippedItemsId;
        spellInfo->SpellInterruptsId = dbcSpellEntry->SpellInterruptsId;
        spellInfo->SpellLevelsId = dbcSpellEntry->SpellLevelsId;
        spellInfo->SpellPowerId = dbcSpellEntry->SpellPowerId;
        spellInfo->SpellReagentsId = dbcSpellEntry->SpellReagentsId;
        spellInfo->SpellShapeshiftId = dbcSpellEntry->SpellShapeshiftId;
        spellInfo->SpellTargetRestrictionsId = dbcSpellEntry->SpellTargetRestrictionsId;
        spellInfo->SpellTotemsId = dbcSpellEntry->SpellTotemsId;

        // Data from SpellAuraOptions.dbc
        auto spellAuraOption = dbcSpellEntry->GetSpellAuraOptions();
        if (dbcSpellEntry->SpellAuraOptionsId && spellAuraOption != nullptr)
        {
            spellInfo->setMaxstack(spellAuraOption->MaxStackAmount);
            spellInfo->setProcChance(spellAuraOption->procChance);
            spellInfo->setProcCharges(spellAuraOption->procCharges);
            spellInfo->setProcFlags(spellAuraOption->procFlags);
        }

        // Data from SpellAuraRestrictions.dbc
        if (dbcSpellEntry->SpellAuraRestrictionsId && dbcSpellEntry->GetSpellAuraRestrictions() != nullptr)
        {
            spellInfo->setCasterAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraState);
            spellInfo->setTargetAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraState);
            spellInfo->setCasterAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraStateNot);
            spellInfo->setTargetAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraStateNot);
            spellInfo->setCasterAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->casterAuraSpell);
            spellInfo->setTargetAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->targetAuraSpell);
            spellInfo->setCasterAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraSpellNot);
            spellInfo->setTargetAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraSpellNot);
        }

        // Data from SpellCastingRequirements.dbc
        if (dbcSpellEntry->SpellCastingRequirementsId && dbcSpellEntry->GetSpellCastingRequirements() != nullptr)
        {
            spellInfo->setFacingCasterFlags(dbcSpellEntry->GetSpellCastingRequirements()->FacingCasterFlags);
            spellInfo->setRequiresAreaId(dbcSpellEntry->GetSpellCastingRequirements()->AreaGroupId);
            spellInfo->setRequiresSpellFocus(dbcSpellEntry->GetSpellCastingRequirements()->RequiresSpellFocus);
        }

        // Data from SpellCategories.dbc
        auto spellCategories = dbcSpellEntry->GetSpellCategories();
        if (dbcSpellEntry->SpellCategoriesId && spellCategories != nullptr)
        {
            spellInfo->setCategory(spellCategories->Category);
            spellInfo->setDispelType(spellCategories->DispelType);
            spellInfo->setDmgClass(spellCategories->DmgClass);
            spellInfo->setMechanicsType(spellCategories->MechanicsType);
            spellInfo->setPreventionType(spellCategories->PreventionType);
            spellInfo->setStartRecoveryCategory(spellCategories->StartRecoveryCategory);
        }

        // Data from SpellClassOptions.dbc
        if (dbcSpellEntry->SpellClassOptionsId && dbcSpellEntry->GetSpellClassOptions() != nullptr)
        {
            spellInfo->setSpellFamilyName(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyName);
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo->setSpellFamilyFlags(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyFlags[j], j);
        }

        // Data from SpellCooldowns.dbc
        if (dbcSpellEntry->SpellCooldownsId && dbcSpellEntry->GetSpellCooldowns() != nullptr)
        {
            spellInfo->setCategoryRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->CategoryRecoveryTime);
            spellInfo->setRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->RecoveryTime);
            spellInfo->setStartRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->StartRecoveryTime);
        }

        // Data from SpellEquippedItems.dbc
        if (dbcSpellEntry->SpellEquippedItemsId && dbcSpellEntry->GetSpellEquippedItems() != nullptr)
        {
            spellInfo->setEquippedItemClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemClass);
            spellInfo->setEquippedItemInventoryTypeMask(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemInventoryTypeMask);
            spellInfo->setEquippedItemSubClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemSubClassMask);
        }

        // Data from SpellInterrupts.dbc
        if (dbcSpellEntry->SpellInterruptsId && dbcSpellEntry->GetSpellInterrupts() != nullptr)
        {
            spellInfo->setAuraInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->AuraInterruptFlags);
            spellInfo->setChannelInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->ChannelInterruptFlags);
            spellInfo->setInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->InterruptFlags);
        }

        // Data from SpellLevels.dbc
        if (dbcSpellEntry->SpellLevelsId && dbcSpellEntry->GetSpellLevels() != nullptr)
        {
            spellInfo->setBaseLevel(dbcSpellEntry->GetSpellLevels()->baseLevel);
            spellInfo->setMaxLevel(dbcSpellEntry->GetSpellLevels()->maxLevel);
            spellInfo->setSpellLevel(dbcSpellEntry->GetSpellLevels()->spellLevel);
        }

        // Data from SpellPower.dbc
        auto spellPower = dbcSpellEntry->GetSpellPower();
        if (dbcSpellEntry->SpellPowerId && spellPower != nullptr)
        {
            spellInfo->setManaCost(spellPower->manaCost);
            spellInfo->setManaCostPerlevel(spellPower->manaCostPerlevel);
            spellInfo->setManaCostPercentage(spellPower->ManaCostPercentage);
            spellInfo->setManaPerSecond(spellPower->manaPerSecond);
            spellInfo->setManaPerSecondPerLevel(spellPower->manaPerSecondPerLevel);
        }

        // Data from SpellReagents.dbc
        if (dbcSpellEntry->SpellReagentsId && dbcSpellEntry->GetSpellReagents() != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
            {
                spellInfo->setReagent(dbcSpellEntry->GetSpellReagents()->Reagent[j], j);
                spellInfo->setReagentCount(dbcSpellEntry->GetSpellReagents()->ReagentCount[j], j);
            }
        }

        // Data from SpellShapeshift.dbc
        if (dbcSpellEntry->SpellShapeshiftId && dbcSpellEntry->GetSpellShapeshift() != nullptr)
        {
            spellInfo->setRequiredShapeShift(dbcSpellEntry->GetSpellShapeshift()->Shapeshifts);
            spellInfo->setShapeshiftExclude(dbcSpellEntry->GetSpellShapeshift()->ShapeshiftsExcluded);
        }

        // Data from SpellTargetRestrictions.dbc
        if (dbcSpellEntry->SpellTargetRestrictionsId && dbcSpellEntry->GetSpellTargetRestrictions() != nullptr)
        {
            spellInfo->setMaxTargets(dbcSpellEntry->GetSpellTargetRestrictions()->MaxAffectedTargets);
            spellInfo->setMaxTargetLevel(dbcSpellEntry->GetSpellTargetRestrictions()->MaxTargetLevel);
            spellInfo->setTargetCreatureType(dbcSpellEntry->GetSpellTargetRestrictions()->TargetCreatureType);
            spellInfo->setTargets(dbcSpellEntry->GetSpellTargetRestrictions()->Targets);
        }

        // Data from SpellTotems.dbc
        if (dbcSpellEntry->SpellTotemsId && dbcSpellEntry->GetSpellTotems() != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
            {
                spellInfo->setTotemCategory(dbcSpellEntry->GetSpellTotems()->TotemCategory[j], j);
                spellInfo->setTotem(dbcSpellEntry->GetSpellTotems()->Totem[j], j);
            }
        }

        // Data from SpellEffect.dbc
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            const auto spell_effect_entry = GetSpellEffectEntry(spell_id, j);
            if (spell_effect_entry != nullptr)
            {
                spellInfo->setEffect(spell_effect_entry->Effect, j);
                spellInfo->setEffectMultipleValue(spell_effect_entry->EffectMultipleValue, j);
                spellInfo->setEffectApplyAuraName(spell_effect_entry->EffectApplyAuraName, j);
                spellInfo->setEffectAmplitude(spell_effect_entry->EffectAmplitude, j);
                spellInfo->setEffectBasePoints(spell_effect_entry->EffectBasePoints, j);
                spellInfo->setEffectBonusMultiplier(spell_effect_entry->EffectBonusMultiplier, j);
                spellInfo->setEffectDamageMultiplier(spell_effect_entry->EffectDamageMultiplier, j);
                spellInfo->setEffectChainTarget(spell_effect_entry->EffectChainTarget, j);
                spellInfo->setEffectDieSides(spell_effect_entry->EffectDieSides, j);
                spellInfo->setEffectItemType(spell_effect_entry->EffectItemType, j);
                spellInfo->setEffectMechanic(spell_effect_entry->EffectMechanic, j);
                spellInfo->setEffectMiscValue(spell_effect_entry->EffectMiscValue, j);
                spellInfo->setEffectMiscValueB(spell_effect_entry->EffectMiscValueB, j);
                spellInfo->setEffectPointsPerComboPoint(spell_effect_entry->EffectPointsPerComboPoint, j);
                spellInfo->setEffectRadiusIndex(spell_effect_entry->EffectRadiusIndex, j);
                spellInfo->setEffectRadiusMaxIndex(spell_effect_entry->EffectRadiusMaxIndex, j);
                spellInfo->setEffectRealPointsPerLevel(spell_effect_entry->EffectRealPointsPerLevel, j);
                for (uint8_t x = 0; x < 3; ++x)
                    spellInfo->setEffectSpellClassMask(spell_effect_entry->EffectSpellClassMask[x], j, x);
                spellInfo->setEffectTriggerSpell(spell_effect_entry->EffectTriggerSpell, j);
                spellInfo->setEffectImplicitTargetA(spell_effect_entry->EffectImplicitTargetA, j);
                spellInfo->setEffectImplicitTargetB(spell_effect_entry->EffectImplicitTargetB, j);
                spellInfo->setEffectSpellId(spell_effect_entry->EffectSpellId, j);
                spellInfo->setEffectIndex(spell_effect_entry->EffectIndex, j);
            }
        }
#endif
        
#endif
    }
}

void SpellMgr::loadSkillLineAbilityMap()
{
    const auto startTime = Util::TimeNow();
    mSkillSpellsMap.clear();
    mSpellSkillsMap.clear();

    uint32_t count = 0;
    for (uint32_t i = 0; i < sSkillLineAbilityStore.getNumRows(); ++i)
    {
        const auto skillAbilityEntry = sSkillLineAbilityStore.lookupEntry(i);
        if (skillAbilityEntry == nullptr)
            continue;

        // Should not happen but just in case
        if (skillAbilityEntry->skilline > 0)
            mSkillSpellsMap.insert({ static_cast<uint16_t>(skillAbilityEntry->skilline), skillAbilityEntry});

        mSpellSkillsMap.insert({ skillAbilityEntry->spell, skillAbilityEntry });
        ++count;
    }

    sLogger.info("SpellMgr : Loaded {} skill abilities in {} ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

#if VERSION_STRING < Mop
void SpellMgr::loadTalentRanks()
{
    const auto startTime = Util::TimeNow();

    uint32_t count = 0;
    for (uint32_t i = 0; i < sTalentStore.getNumRows(); ++i)
    {
        const auto talentEntry = sTalentStore.lookupEntry(i);
        if (talentEntry == nullptr)
            continue;

        const auto* const firstRank = getSpellInfo(talentEntry->RankID[0]);
        if (firstRank == nullptr)
        {
            sLogger.failure("SpellMgr::loadTalentRanks : Unknown spell id {} in sTalentStore, skipping this chain", talentEntry->RankID[0]);
            continue;
        }

#if VERSION_STRING >= WotLK
        const auto talentTree = sTalentTabStore.lookupEntry(talentEntry->TalentTree);
#endif

        // Get last rank
        SpellInfo const* lastRank = nullptr;
        for (uint8_t rank = 0; rank < 5; ++rank)
        {
            const auto spellId = talentEntry->RankID[rank];
            if (spellId == 0)
                break;

            auto* const rankInfo = getMutableSpellInfo(spellId);
            if (rankInfo == nullptr)
                break;

            // Also add some talent data to each rank's SpellInfo for faster access
            rankInfo->setIsTalent(true);
#if VERSION_STRING >= WotLK
            if (talentTree != nullptr && talentTree->PetTalentMask != 0)
                rankInfo->setIsPetTalent(true);
#endif

            lastRank = rankInfo;
        }

        // If talent has no ranks, skip it
        if (lastRank == nullptr || lastRank->getId() == firstRank->getId())
            continue;

        std::map<uint32_t/*spell id*/, SpellRankInfo> preparedSpellRanks{};
        auto areSpellsValid = true, areRanksValid = true, foundDuplicateChain = false;
        uint32_t invalidSpellId = 0;
        uint8_t invalidSpellRank = 0, expectedSpellRank = 0;

        uint32_t previousSpellId = 0;
        uint8_t previousRank = 0;
        for (uint8_t rank = 1; rank <= 5; ++rank)
        {
            const auto spellId = talentEntry->RankID[rank - 1];
            if (spellId == 0)
                break;

            if (rank != (previousRank + 1))
            {
                areRanksValid = false;
                invalidSpellRank = rank;
                expectedSpellRank = previousRank + 1;
                break;
            }

            const auto* const spellInfo = getSpellInfo(spellId);
            if (spellInfo == nullptr)
            {
                areSpellsValid = false;
                invalidSpellId = spellId;
                break;
            }

            if (spellInfo->hasSpellRanks())
            {
                foundDuplicateChain = true;
                invalidSpellId = spellId;
                break;
            }

            SpellRankInfo rankInfo{};
            rankInfo.previousSpell = previousSpellId != 0 ? getSpellInfo(previousSpellId) : nullptr;
            rankInfo.nextSpell = rank < 5 ? getSpellInfo(talentEntry->RankID[rank]) : nullptr;
            rankInfo.firstSpell = firstRank;
            rankInfo.lastSpell = lastRank;
            rankInfo.rank = rank;

            previousSpellId = spellId;
            previousRank = rank;

            // Spell rank data is ready but validate and prepare the entire chain before saving it
            preparedSpellRanks.insert({ spellId, rankInfo });
        }

        if (!areSpellsValid)
        {
            sLogger.failure("SpellMgr::loadTalentRanks : Talent rank chain contains invalid spell id {}, skipping this chain", invalidSpellId);
            continue;
        }

        if (!areRanksValid)
        {
            sLogger.failure("SpellMgr::loadTalentRanks : Talent rank chain contains invalid rank {} for spell id {} (expected {}), skipping this chain",
                invalidSpellRank, invalidSpellId, expectedSpellRank);
            continue;
        }

        if (foundDuplicateChain)
        {
            // Do not send error for talents because same spell can be found in multiple different talent trees
            continue;
        }

        if (preparedSpellRanks.size() == 1)
        {
            sLogger.failure("SpellMgr::loadTalentRanks : Talent rank chain contains only one spell for id {}, skipping", preparedSpellRanks.cbegin()->first);
            continue;
        }

        // Finally insert validated spell rank data to SpellInfo class
        for (const auto& [spellId, rankInfo] : preparedSpellRanks)
        {
            auto spellInfo = getMutableSpellInfo(spellId);
            spellInfo->setSpellRankData(rankInfo);
        }

        ++count;
    }

    sLogger.info("SpellMgr : Loaded {} talent rank chains in {} ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}
#endif

void SpellMgr::loadSpellCoefficientOverride()
{
    //                                            0           1                     2
    auto result = WorldDatabase.Query("SELECT spell_id, direct_coefficient, overtime_coefficient "
                                            "FROM spell_coefficient_override WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);

    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellCoefficientOverride : Your `spell_coefficient_override` table is empty!");
        return;
    }

    auto overridenCoeffs = 0;
    do
    {
        const auto fields = result->Fetch();
        auto spellInfo = getMutableSpellInfo(fields[0].asUint32());
        if (spellInfo == nullptr)
        {
            sLogger.failure("Table `spell_coefficient_override` has unknown spell entry {}, skipped", fields[0].asUint32());
            continue;
        }

        const auto direct_override = fields[1].asFloat();
        const auto overtime_override = fields[2].asFloat();
        // Coeff can be overridden to 0 when it won't receive any bonus from spell power (default value is -1)
        if (direct_override >= 0)
            spellInfo->spell_coeff_direct = direct_override;
        if (overtime_override >= 0)
            spellInfo->spell_coeff_overtime = overtime_override;
        ++overridenCoeffs;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} override values from `spell_coefficient_override` table", overridenCoeffs);
}

void SpellMgr::loadSpellCustomOverride()
{
    //                                                   0                1                         2                     3
    const auto result = WorldDatabase.Query("SELECT `spell_id`, `assign_on_target_flag`, `assign_self_cast_only`, `assign_c_is_flag` FROM spell_custom_override");
    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellCustomOverride : Your `spell_custom_override` table is empty!");
        return;
    }

    auto overridenSpells = 0;
    do
    {
        const auto fields = result->Fetch();

        auto spellInfo = getMutableSpellInfo(fields[0].asUint32());
        if (spellInfo == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Table `spell_custom_override` has unknown spell entry {}, skipped", fields[0].asUint32());
            continue;
        }

        // assign_on_target_flag
        if (fields[1].isSet())
            spellInfo->custom_BGR_one_buff_on_target = fields[1].asUint32();

        // assign_self_cast_only
        if (fields[2].isSet())
            spellInfo->custom_self_cast_only = fields[2].asBool();

        // assign_c_is_flag
        if (fields[3].isSet())
            spellInfo->custom_c_is_flags = fields[3].asUint32();

        // todo: following columns will be removed from db on spell proc rework -Appled

        //\ todo: remove this field
        //proc_flags
        //if (fields[5].isSet())
            //spellInfo->setProcFlags(fields[5].GetUInt32());

        //\ todo: remove this field
        //proc_target_selfs
        //if (fields[6].isSet() && fields[6].GetBool())
            //spellInfo->addProcFlags(PROC_TARGET_SELF);

        //\ todo: remove this field
        //proc_chance
        //if (fields[7].isSet())
            //spellInfo->setProcChance(fields[7].GetUInt32());

        //\ todo: remove this field
        //proc_charges
        //if (fields[8].isSet())
            //spellInfo->setProcCharges(fields[8].GetUInt32());

        //\ todo: remove this field
        //proc_interval
        //if (fields[9].isSet())
            //spellInfo->custom_proc_interval = fields[9].GetUInt32();

        //\ todo: remove this field
        //proc_effect_trigger_spell_0
        /*if (fields[10].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[10].GetUInt32(), 0);
            if (spellInfo->getEffectTriggerSpell(0) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        }*/

        //\ todo: remove this field
        //proc_effect_trigger_spell_1
        /*if (fields[11].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[11].GetUInt32(), 1);
            if (spellInfo->getEffectTriggerSpell(1) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        }*/

        //\ todo: remove this field
        //proc_effect_trigger_spell_2
        /*if (fields[12].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[12].GetUInt32(), 2);
            if (spellInfo->getEffectTriggerSpell(2) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 2);
        }*/

        ++overridenSpells;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} override values from `spell_custom_override` table", overridenSpells);
}

void SpellMgr::loadSpellAIThreat()
{
    const auto result = WorldDatabase.Query("SELECT * FROM ai_threattospellid");
    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellAIThreat : Your `ai_threattospellid` table is empty!");
        return;
    }

    auto threatCount = 0;
    do
    {
        const auto fields = result->Fetch();
        const auto spellId = fields[0].asUint32();

        auto spellInfo = getMutableSpellInfo(spellId);
        if (spellInfo == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Table `ai_threattospellid` has invalid spell entry {}, skipped", spellId);
            continue;
        }

        spellInfo->custom_ThreatForSpell = fields[1].asInt32();
        spellInfo->custom_ThreatForSpellCoef = fields[2].asFloat();

        ++threatCount;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} spell ai threat from `ai_threattospellid` table", threatCount);
}

void SpellMgr::loadSpellEffectOverride()
{
    const auto result = WorldDatabase.Query("SELECT * FROM spell_effects_override");
    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellEffectOverride : Your `spell_effects_override` table is empty!");
        return;
    }

    auto overridenEffects = 0;
    do
    {
        const auto fields = result->Fetch();
        uint32_t seo_SpellId = fields[0].asUint32();
        uint8_t seo_EffectId = fields[1].asUint8();
        uint32_t seo_Disable = fields[2].asUint32();
        uint32_t seo_Effect = fields[3].asUint32();
        uint32_t seo_BasePoints = fields[4].asUint32();
        uint32_t seo_ApplyAuraName = fields[5].asUint32();
        //uint32_t seo_SpellGroupRelation = fields[6].GetUInt32();
        uint32_t seo_MiscValue = fields[7].asUint32();
        uint32_t seo_TriggerSpell = fields[8].asUint32();
        uint32_t seo_ImplicitTargetA = fields[9].asUint32();
        uint32_t seo_ImplicitTargetB = fields[10].asUint32();
        uint32_t seo_EffectCustomFlag = fields[11].asUint32();

        auto spellInfo = getMutableSpellInfo(seo_SpellId);
        if (spellInfo == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_SPELL_EFF, "Table `spell_effects_override` has invalid spell entry {}, skipped", seo_SpellId);
            continue;
        }

        if (seo_Disable > 0)
            spellInfo->setEffect(SPELL_EFFECT_NULL, seo_EffectId);

        if (seo_Effect > 0)
            spellInfo->setEffect(seo_Effect, seo_EffectId);

        if (seo_BasePoints > 0)
            spellInfo->setEffectBasePoints(seo_BasePoints, seo_EffectId);

        if (seo_ApplyAuraName > 0)
            spellInfo->setEffectApplyAuraName(seo_ApplyAuraName, seo_EffectId);

        //if (seo_SpellGroupRelation > 0)
            //spellInfo->EffectSpellGroupRelation[seo_EffectId] = seo_SpellGroupRelation;

        if (seo_MiscValue > 0)
            spellInfo->setEffectMiscValue(seo_MiscValue, seo_EffectId);

        if (seo_TriggerSpell > 0)
            spellInfo->setEffectTriggerSpell(seo_TriggerSpell, seo_EffectId);

        if (seo_ImplicitTargetA > 0)
            spellInfo->setEffectImplicitTargetA(seo_ImplicitTargetA, seo_EffectId);

        if (seo_ImplicitTargetB > 0)
            spellInfo->setEffectImplicitTargetB(seo_ImplicitTargetB, seo_EffectId);

        if (seo_EffectCustomFlag != 0)
            spellInfo->EffectCustomFlag[seo_Effect] = seo_EffectCustomFlag;

        ++overridenEffects;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} spell effect overrides from `spell_effects_override` table", overridenEffects);
}

void SpellMgr::loadSpellAreas()
{
    mSpellAreaMap.clear();
    mSpellAreaForQuestMap.clear();
    mSpellAreaForActiveQuestMap.clear();
    mSpellAreaForQuestEndMap.clear();
    mSpellAreaForAuraMap.clear();

    //                                                0     1         2              3               4           5          6        7        8
    const auto result = WorldDatabase.Query("SELECT spell, area, quest_start, quest_start_active, quest_end, aura_spell, racemask, gender, autocast FROM spell_area");
    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellAreas : Your `spell_area` table is empty!");
        return;
    }

    auto areaCount = 0;
    do
    {
        const auto fields = result->Fetch();

        uint32_t spellId = fields[0].asUint32();

        SpellArea spellArea{};
        spellArea.spellId = spellId;
        spellArea.areaId = fields[1].asUint32();
        spellArea.questStart = fields[2].asUint32();
        spellArea.questStartCanActive = fields[3].asBool();
        spellArea.questEnd = fields[4].asUint32();
        spellArea.auraSpell = fields[5].asInt32();
        spellArea.raceMask = fields[6].asUint32();
        spellArea.gender = Gender(fields[7].asUint32());
        spellArea.autoCast = fields[8].asBool();

        // Search for a duplicate entry
        auto duplicate = false;
        const auto spellAreaMapBounds = getSpellAreaMapBounds(spellArea.spellId);
        for (SpellAreaMap::const_iterator itr = spellAreaMapBounds.first; itr != spellAreaMapBounds.second; ++itr)
        {
            if (spellArea.spellId != itr->second.spellId)
                continue;
            if (spellArea.areaId != itr->second.areaId)
                continue;
            if (spellArea.questStart != itr->second.questStart)
                continue;
            if (spellArea.auraSpell != itr->second.auraSpell)
                continue;
            if ((spellArea.raceMask & itr->second.raceMask) == 0)
                continue;
            if (spellArea.gender != itr->second.gender)
                continue;
            // Found a duplicate
            duplicate = true;
            break;
        }

        if (duplicate)
        {
            sLogger.failure("Table `spell_area` has duplicate spell entry ({}) with similiar requirements, skipped", spellId);
            continue;
        }

        // Check if the area id is a valid area id or zone id
        if (spellArea.areaId > 0)
        {
            const auto areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(spellArea.areaId);
            if (areaEntry == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_SPELL, "Table `spell_area` has invalid area id {} for spell entry {}, skipped", spellArea.areaId, spellId);
                continue;
            }
        }

        // Check if the start quest is a valid quest
        if (spellArea.questStart > 0)
        {
            const auto startQuest = sMySQLStore.getQuestProperties(spellArea.questStart);
            if (startQuest == nullptr)
            {
                sLogger.failure("Table `spell_area` has invalid quest id {} for spell entry {}, skipped", spellArea.questStart, spellId);
                continue;
            }
        }
        
        // Check if the end quest is a valid quest
        if (spellArea.questEnd > 0)
        {
            const auto endQuest = sMySQLStore.getQuestProperties(spellArea.questEnd);
            if (endQuest == nullptr)
            {
                sLogger.failure("Table `spell_area` has invalid quest id {} for spell entry {}, skipped", spellArea.questEnd, spellId);
                continue;
            }

            // Check if the end quest is same as start quest
            if (spellArea.questEnd == spellArea.questStart && !spellArea.questStartCanActive)
            {
                sLogger.failure("Table `spell_area` has quest (id {}) requirement for spell entry {} for start and end at the same time, skipped", spellArea.questEnd, spellId);
                continue;
            }
        }

        // Check for auraspell
        const auto auraId = static_cast<uint32_t>(std::abs(spellArea.auraSpell));
        if (spellArea.auraSpell != 0)
        {
            const auto spellInfo = getSpellInfo(auraId);
            if (spellInfo == nullptr)
            {
                sLogger.failure("Table `spell_area` has invalid aura spell entry {} for spell {}, skipped", auraId, spellId);
                continue;
            }

            if (auraId == spellArea.spellId)
            {
                sLogger.failure("Table `spell_area` has aura spell requirements for itself (id {}), skipped", spellId);
                continue;
            }

            if (spellArea.autoCast && spellArea.auraSpell > 0)
            {
                auto chain = false;
                const auto spellAreaForAuraMapBounds = getSpellAreaForAuraMapBounds(spellArea.spellId);
                for (SpellAreaForAuraMap::const_iterator itr = spellAreaForAuraMapBounds.first; itr != spellAreaForAuraMapBounds.second; ++itr)
                {
                    if (itr->second->autoCast && itr->second->auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLogger.failure("Table `spell_area` has aura spell (id {}) requirement that itself was autocasted from an aura, skipping", spellId);
                    continue;
                }

                const auto spellAreaMapBounds2 = getSpellAreaMapBounds(auraId);
                for (SpellAreaMap::const_iterator itr = spellAreaMapBounds2.first; itr != spellAreaMapBounds2.second; ++itr)
                {
                    if (itr->second.autoCast && itr->second.auraSpell > 0)
                    {
                        chain = true;
                        break;
                    }
                }

                if (chain)
                {
                    sLogger.failure("Table `spell_area` has aura spell (id {}) requirement that itself was autocasted from an aura, skipping", spellId);
                    continue;
                }
            }
        }

        if (spellArea.raceMask > 0 && !(spellArea.raceMask & RACEMASK_ALL_PLAYABLE))
        {
            sLogger.failure("Table `spell_area` has invalid racemask for spell entry {}, skipped", spellId);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_MALE && spellArea.gender != GENDER_FEMALE)
        {
            sLogger.failure("Table `spell_area` has invalid gender for spell entry {}, skipped", spellId);
            continue;
        }

        // Insert the spell area to correct map
        SpellArea const* spellArea2 = &mSpellAreaMap.insert(SpellAreaMap::value_type(spellId, spellArea))->second;

        if (spellArea.areaId > 0)
            mSpellAreaForAreaMap.insert(SpellAreaForAreaMap::value_type(spellArea.areaId, spellArea2));

        if (spellArea.questStart > 0)
        {
            if (spellArea.questStartCanActive)
                mSpellAreaForActiveQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, spellArea2));
            else
                mSpellAreaForQuestMap.insert(SpellAreaForQuestMap::value_type(spellArea.questStart, spellArea2));
        }

        if (spellArea.questEnd > 0)
            mSpellAreaForQuestEndMap.insert(SpellAreaForQuestMap::value_type(spellArea.questEnd, spellArea2));

        if (spellArea.auraSpell > 0)
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(auraId, spellArea2));

        ++areaCount;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} spell area requirements from `spell_area` table", areaCount);
}

void SpellMgr::loadSpellRequired()
{
    mSpellsRequiringSpell.clear();
    mSpellRequired.clear();

    //                                                   0         1
    const auto result = WorldDatabase.Query("SELECT spell_id, req_spell FROM spell_required");
    if (result == nullptr)
    {
        sLogger.debug("SpellMgr : Loaded 0 spell required records. DB table `spell_required` is empty.");
        return;
    }

    uint32_t count = 0;
    do
    {
        auto fields = result->Fetch();

        auto spell_id = fields[0].asUint32();
        auto spell_req = fields[1].asUint32();

        // Check for valid spells
        const auto spellInfo = getSpellInfo(spell_id);
        if (spellInfo == nullptr)
        {
            sLogger.debug("SpellMgr : spell_id {} in `spell_required` table is not found, skipped", spell_id);
            continue;
        }

        const auto requiredInfo = getSpellInfo(spell_req);
        if (requiredInfo == nullptr)
        {
            sLogger.debug("SpellMgr : req_spell {} in `spell_required` table is not found, skipped", spell_req);
            continue;
        }

        if (isSpellRequiringSpell(spell_id, spell_req))
        {
            sLogger.debug("SpellMgr : duplicated entry of req_spell {} and spell_id {} in `spell_required`, skipped", spell_req, spell_id);
            continue;
        }

        mSpellRequired.insert(std::pair<uint32_t, uint32_t>(spell_id, spell_req));
        mSpellsRequiringSpell.insert(std::pair<uint32_t, uint32_t>(spell_req, spell_id));
        ++count;
    } while (result->NextRow());

    sLogger.info("SpellMgr : Loaded {} spell required records from `spell_required` table", count);
}

void SpellMgr::loadSpellTargetConstraints()
{
    const auto result = WorldDatabase.Query("SELECT * FROM spelltargetconstraints WHERE SpellID > 0 ORDER BY SpellID");
    if (result != nullptr)
    {
        uint32_t oldspellId = 0;
        SpellTargetConstraint* stc = nullptr;

        do
        {
            auto fields = result->Fetch();

            if (fields != nullptr)
            {
                const auto spellId = fields[0].asUint32();
                if (oldspellId != spellId)
                {
                    const auto [stcItr, _] = mSpellTargetConstraintMap.try_emplace(spellId, Util::LazyInstanceCreator([] {
                        return std::make_unique<SpellTargetConstraint>();
                    }));

                    stc = stcItr->second.get();
                }

                const auto type = fields[1].asUint8();
                const auto value = fields[2].asUint32();

                if (type == SPELL_CONSTRAINT_EXPLICIT_CREATURE)
                {
                    if (stc != nullptr)
                    {
                        stc->addCreature(value);
                        stc->addExplicitTarget(value);
                    }
                }
                else if (type == SPELL_CONSTRAINT_EXPLICIT_GAMEOBJECT)
                {
                    if (stc != nullptr)
                    {
                        stc->addGameObject(value);
                        stc->addExplicitTarget(value);
                    }
                }
                else if (type == SPELL_CONSTRAINT_IMPLICIT_CREATURE)
                {
                    if (stc != nullptr)
                        stc->addCreature(value);
                }
                else if (type == SPELL_CONSTRAINT_IMPLICIT_GAMEOBJECT)
                {
                    if (stc != nullptr)
                        stc->addGameObject(value);
                }

                oldspellId = spellId;
            }
        } while (result->NextRow());
    }

    sLogger.info("SpellMgr : Loaded {} spell target constraints from `spelltargetconstraints` table", static_cast<uint32_t>(mSpellTargetConstraintMap.size()));
}

void SpellMgr::loadSpellDisabled()
{
    const auto result = WorldDatabase.Query("SELECT * FROM spell_disable");
    if (result != nullptr)
    {
        do
        {
            mDisabledSpells.insert(result->Fetch()[0].asUint32());
        } while (result->NextRow());
    }

    sLogger.info("SpellMgr : Loaded {} disabled spells from `spell_disable` table", static_cast<uint32_t>(mDisabledSpells.size()));
}

void SpellMgr::loadSpellRanks()
{
#if VERSION_STRING < Mop
    // Load talent ranks from DBC before spell ranks from database
    loadTalentRanks();
#endif

    //                                                  0             1          2
    const auto result = WorldDatabase.Query("SELECT `spell_id`, `first_spell`, `rank` "
        "FROM spell_ranks WHERE `min_build` <= %u AND `max_build` >= %u ORDER BY `first_spell`, `rank`", VERSION_STRING, VERSION_STRING);

    if (result == nullptr)
    {
        sLogger.debug("SpellMgr::loadSpellRanks : Your `spell_ranks` table is empty");
        return;
    }

    std::map<uint8_t/*rank*/, uint32_t/*spell id*/> spellRankChain{};
    uint32_t totalSpellRankChains = 0;

    const auto createSpellRankChain = [&spellRankChain, this, &totalSpellRankChains]() -> void
    {
        if (spellRankChain.empty())
            return;

        if (spellRankChain.size() < 2)
        {
            sLogger.failure("SpellMgr::loadSpellRanks : Spell rank chain for spell id {} must contain at least 2 spells, skipping", spellRankChain.cbegin()->second);
            spellRankChain.clear();
            return;
        }

        std::map<uint32_t/*spell id*/, SpellRankInfo> preparedSpellRanks{};
        auto areChainSpellsValid = true, areChainRanksValid = true, foundDuplicateChain = false;
        uint32_t invalidSpellId = 0;
        uint8_t invalidSpellRank = 0, expectedSpellRank = 0;

        uint32_t previousSpellId = 0;
        uint8_t previousRank = 0;
        for (auto itr = spellRankChain.cbegin(); itr != spellRankChain.cend();)
        {
            const auto spellRank = itr->first;
            const auto spellId = itr->second;

            if (spellRank == 0)
            {
                areChainRanksValid = false;
                invalidSpellRank = spellRank;
                invalidSpellId = spellId;
                expectedSpellRank = 1;
                break;
            }
            else if (spellRank > 1 && spellRank != (previousRank + 1))
            {
                areChainRanksValid = false;
                invalidSpellRank = spellRank;
                invalidSpellId = spellId;
                expectedSpellRank = previousRank + 1;
                break;
            }

            const auto* const spellInfo = getSpellInfo(spellId);
            if (spellInfo == nullptr)
            {
                areChainSpellsValid = false;
                invalidSpellId = spellId;
                break;
            }

            if (spellInfo->hasSpellRanks())
            {
                foundDuplicateChain = true;
                invalidSpellId = spellId;
                break;
            }

            SpellRankInfo rankInfo{};
            rankInfo.previousSpell = previousSpellId != 0 ? getSpellInfo(previousSpellId) : nullptr;
            rankInfo.firstSpell = getSpellInfo(spellRankChain.cbegin()->second);
            rankInfo.lastSpell = getSpellInfo(spellRankChain.crbegin()->second);
            rankInfo.rank = spellRank;

            previousSpellId = spellId;
            previousRank = spellRank;
            ++itr;

            rankInfo.nextSpell = itr != spellRankChain.cend() ? getSpellInfo(itr->second) : nullptr;

            // Spell rank data is ready but validate and prepare the entire chain before saving it
            preparedSpellRanks.insert({ spellId, rankInfo });
        }

        if (!areChainSpellsValid)
        {
            sLogger.failure("SpellMgr::loadSpellRanks : Spell rank chain contains invalid spell id {}, skipping this chain", invalidSpellId);
            spellRankChain.clear();
            return;
        }

        if (!areChainRanksValid)
        {
            sLogger.failure("SpellMgr::loadSpellRanks : Spell rank chain contains invalid spell rank {} for spell id {} (expected {}), skipping this chain",
                invalidSpellRank, invalidSpellId, expectedSpellRank);
            spellRankChain.clear();
            return;
        }

        if (foundDuplicateChain)
        {
            sLogger.failure("SpellMgr::loadSpellRanks : Spell id {} already contains a spell rank chain, skipping this chain", invalidSpellId);
            spellRankChain.clear();
            return;
        }

        // Finally insert validated spell rank data to SpellInfo class
        for (const auto& [spellId, rankInfo] : preparedSpellRanks)
        {
            auto spellInfo = getMutableSpellInfo(spellId);
            spellInfo->setSpellRankData(rankInfo);
        }

        spellRankChain.clear();
        ++totalSpellRankChains;
    };

    uint32_t lastFetchedSpellId = 0;
    do
    {
        const auto fields = result->Fetch();
        const auto spellId = fields[0].asUint32();
        const auto firstSpellId = fields[1].asUint32();
        const auto rank = fields[2].asUint8();

        if (lastFetchedSpellId == 0)
            lastFetchedSpellId = firstSpellId;

        // First spell id changes => create a rank chain for each rank before continuing
        if (lastFetchedSpellId != firstSpellId)
        {
            createSpellRankChain();
            lastFetchedSpellId = firstSpellId;
        }

        spellRankChain.insert({ rank, spellId });
    } while (result->NextRow());

    // Remember to create a rank chain for last chain as well
    createSpellRankChain();

    sLogger.info("SpellMgr : Loaded {} spell rank chains from `spell_ranks` table", totalSpellRankChains);
}

void SpellMgr::setSpellCoefficient(SpellInfo* sp)
{
    const auto baseDuration = float(GetDuration(sSpellDurationStore.lookupEntry(sp->getDurationIndex())));

    // Check if coefficient is overriden from database
    if (sp->spell_coeff_direct != -1)
    {
        // Store coeff value as per missile for channeled spells
        if (sp->isChanneled())
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
#if VERSION_STRING >= TBC
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
#endif
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                {
                    sp->spell_coeff_direct /= baseDuration / sp->getEffectAmplitude(i);
                    break;
                }
            }
        }
    }

    // If coefficients are already set, do not alter them
    // this can happen if this spell is triggered by another spell or coefficients are overriden in database
    if (sp->spell_coeff_direct != -1 || sp->spell_coeff_overtime != -1)
        return;

    // Skip non-magic spells
    if (sp->getDmgClass() != SPELL_DMG_TYPE_MAGIC)
        return;

    // Skip calculation if spell family name isn't any of the player classes
    // Note: some player spells may have generic spell family name but these spells can be added into the SQL table
    if (sp->getSpellFamilyName() != SPELLFAMILY_MAGE &&
        sp->getSpellFamilyName() != SPELLFAMILY_WARLOCK &&
        sp->getSpellFamilyName() != SPELLFAMILY_PRIEST &&
        sp->getSpellFamilyName() != SPELLFAMILY_DRUID &&
        sp->getSpellFamilyName() != SPELLFAMILY_PALADIN &&
        sp->getSpellFamilyName() != SPELLFAMILY_SHAMAN)
        return;

#if VERSION_STRING >= WotLK
    // Load spell coefficient values from DBC
    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        // If effect triggers another spell, ignore the coefficient value
        // The spell to be triggered has the coefficient already and later it will be handled in direct spell check
        if (sp->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL ||
            sp->getEffect(i) == SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE ||
            sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
            sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
            continue;

        if (sp->getEffectBonusMultiplier(i) > 0)
        {
            const auto coefficientValue = sp->getEffectBonusMultiplier(i);
            // For channeled and over-time spells, the coefficient is already stored as per tick in DBC
            if (sp->isChanneled() &&
                (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH))
                sp->spell_coeff_direct = coefficientValue;

            else if (sp->isDamagingEffect(i) || sp->isHealingEffect(i) || sp->getEffect(i) == SPELL_EFFECT_HEALTH_LEECH)
                sp->spell_coeff_direct = coefficientValue;

            else if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                sp->spell_coeff_overtime = coefficientValue;
        }
    }
#else
    const auto isOverTimeSpell = sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) || sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL);

    // Helper lambda for checking if spell has additional effects
    auto hasAdditionalEffects = [&](SpellInfo const* sp) -> bool
    {
        // TODO: is there more? -Appled
        if (sp->hasEffectApplyAuraName(SPELL_AURA_MOD_DECREASE_SPEED) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_MOD_ROOT) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_MOD_HIT_CHANCE) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_MOD_HASTE))
            return true;
        return false;
    };

    // Helper lambda for checking if spell is AoE
    auto isAoESpell = [&](SpellInfo const* sp) -> bool
    {
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffect(i) == 0)
                continue;
            // TODO: is there more? -Appled
            if (sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY_AROUND_CASTER ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_IN_FRONT_OF_CASTER ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_FRIENDLY_IN_AREA ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ALL_PARTY ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON ||
                // Dunno if these are needed (custom parameters)
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI ||
                sp->getEffectImplicitTargetA(i) == EFF_TARGET_CONE_IN_FRONT)
                return true;

            if (sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_INSTANT ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY_AROUND_CASTER ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMIES_AROUND_CASTER ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_IN_FRONT_OF_CASTER ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_ENEMY_IN_AREA_CHANNELED ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY_IN_AREA_CHANNELED ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_FRIENDLY_IN_AREA ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_TARGETABLE_AROUND_LOCATION_IN_RADIUS_OVER_TIME ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ALL_PARTY ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_ENEMIES_IN_AREA_CHANNELED_WITH_EXCEPTIONS ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_SELECTED_ENEMY_DEADLY_POISON ||
                // Dunno if these are needed (custom parameters)
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_CUSTOM_PARTY_INJURED_MULTI ||
                sp->getEffectImplicitTargetB(i) == EFF_TARGET_CONE_IN_FRONT)
                return true;
        }
        return false;
    };

    // Helper lambda for checking if spell is channeled
    auto hasChanneledEffect = [&](SpellInfo const* sp) -> bool
    {
        if (!sp->isChanneled())
            return false;

        if (sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL) ||
#if VERSION_STRING >= TBC
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) ||
#endif
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_LEECH))
            return true;

        return false;
    };

    // Calculate base spell coefficient
    auto spellCastTime = float(GetCastTime(sSpellCastTimesStore.lookupEntry(sp->getCastingTimeIndex())));
    if (spellCastTime < 1500)
        spellCastTime = 1500;
#if VERSION_STRING == Classic
    // Classic has 100% spell coefficient cap
    else if (spellCastTime > 3500)
        spellCastTime = 3500;
#else
    else if (spellCastTime > 7000)
        spellCastTime = 7000;
#endif

    // Leech spells (spells that deal damage and heal)
    const auto hasLeechEffect = sp->hasEffect(SPELL_EFFECT_HEALTH_LEECH);
    const auto hasLeechAura = sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_LEECH);
    const auto isLeechSpell = hasLeechEffect || hasLeechAura;

    // Channeled spells
    if (hasChanneledEffect(sp))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 3500)
            spellDuration = 3500;
#endif
        sp->spell_coeff_direct = spellDuration / 3500;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_direct *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_direct /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_direct /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() <= 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
        }

        // Store coeff value as per missile / tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
#if VERSION_STRING >= TBC
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
#endif
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                sp->spell_coeff_direct /= baseDuration / sp->getEffectAmplitude(i);
                // For channeled spells which trigger another spell on each "missile", set triggered spell's coeff to match master spell's coeff
#if VERSION_STRING == Classic
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
#else
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
#endif
                {
                    auto triggeredSpell = sSpellMgr.getMutableSpellInfo(sp->getEffectTriggerSpell(i));
                    // But do not alter its coefficient if it's already overridden in database
                    if (triggeredSpell != nullptr && triggeredSpell->spell_coeff_direct == -1)
                        triggeredSpell->spell_coeff_direct = sp->spell_coeff_direct;
                }
                break;
            }
        }
    }

    // Hybrid spells (spells with direct and over-time damage or direct and over-time healing effect)
    else if ((sp->hasDamagingEffect() && sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE)) ||
        (sp->hasHealingEffect() && sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL)))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 15000)
            spellDuration = 15000;
#endif
        const auto castTime = spellCastTime / 3500;
        spellDuration /= 15000;

        sp->spell_coeff_direct = (castTime * castTime) / (spellDuration + castTime);
        sp->spell_coeff_overtime = (spellDuration * spellDuration) / (castTime + spellDuration);

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            sp->spell_coeff_overtime *= 0.95f;
        }

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
        {
            sp->spell_coeff_direct /= 3;
            sp->spell_coeff_overtime /= 3;
        }
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            sp->spell_coeff_overtime *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
            {
                sp->spell_coeff_direct /= 3;
                sp->spell_coeff_overtime /= 3;
            }
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
            {
                sp->spell_coeff_direct /= 2;
                sp->spell_coeff_overtime /= 2;
            }
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
        {
            sp->spell_coeff_direct /= 2;
            sp->spell_coeff_overtime /= 2;
        }

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() <= 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
            sp->spell_coeff_overtime *= penalty;
        }

        // Store over-time coeff value as per tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                /*
                    If a spell has less than five ticks, each tick receives one-fifth of the bonus per tick.
                    If an over time spell has 5 or more ticks, the spell receives full benefit, divided equally between the number of ticks.
                */
                const auto ticks = baseDuration / sp->getEffectAmplitude(i);
                if (ticks < 5)
                    sp->spell_coeff_overtime /= 5;
                else
                    sp->spell_coeff_overtime /= ticks;
                break;
            }
        }
    }

    // Direct damage and healing spells
    else if (!isOverTimeSpell && ((sp->hasDamagingEffect() && !sp->hasEffect(SPELL_EFFECT_POWER_BURN)) || (sp->hasHealingEffect() && !sp->hasEffect(SPELL_EFFECT_HEAL_MAX_HEALTH)) || hasLeechEffect))
    {
        sp->spell_coeff_direct = spellCastTime / 3500;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_direct *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_direct /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_direct *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_direct /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_direct /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() <= 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
        }
    }

    // Damage and healing over-time spells
    else if ((isOverTimeSpell || hasLeechAura) && !sp->hasDamagingEffect() && !sp->hasHealingEffect() && !hasChanneledEffect(sp))
    {
        auto spellDuration = baseDuration;
#if VERSION_STRING == Classic
        // Classic has 100% spell coefficient cap
        if (spellDuration > 15000)
            spellDuration = 15000;
#endif
        sp->spell_coeff_overtime = spellDuration / 15000;

        // Spells with additional effects receive 5% penalty
#if VERSION_STRING == Classic
        if (hasAdditionalEffects(sp))
            sp->spell_coeff_overtime *= 0.95f;

        // In Classic all Area of Effect spells receive 33% of the coefficient
        if (isAoESpell(sp))
            sp->spell_coeff_overtime /= 3;
#else
        if (hasAdditionalEffects(sp))
        {
            sp->spell_coeff_overtime *= 0.95f;
            // In TBC, Area of Effect spells with additional effects receive 33% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_overtime /= 3;
        }
        else
        {
            // and normal Area of Effect spells receive 50% of the coefficient
            if (isAoESpell(sp))
                sp->spell_coeff_overtime /= 2;
        }
#endif

        // Leech spells receive 50% of the coefficient
        if (isLeechSpell)
            sp->spell_coeff_overtime /= 2;

        // Spells below level 20 receive a significant penalty
        if (sp->getBaseLevel() <= 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_overtime *= penalty;
        }

        // Store coeff value as per tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                /*
                    If a spell has less than five ticks, each tick receives one-fifth of the bonus per tick.
                    If an over time spell has 5 or more ticks, the spell receives full benefit, divided equally between the number of ticks.
                */
                const auto ticks = baseDuration / sp->getEffectAmplitude(i);
                if (ticks < 5)
                    sp->spell_coeff_overtime /= 5;
                else
                    sp->spell_coeff_overtime /= ticks;
                break;
            }
        }
    }
#endif
}

SpellInfo* SpellMgr::getMutableSpellInfo(const uint32_t spellId) const
{
    if (spellId == 0)
        return nullptr;

    const auto itr = mSpellInfoMapStore.find(spellId);
    if (itr != mSpellInfoMapStore.end())
        return itr->second.get();

    return nullptr;
}

void SpellMgr::addSpellBySpellInfo(SpellInfo* spellInfo, SpellScriptLinker spellScript)
{
    spellInfo->spellScriptLink = (void* (*)) spellScript;
}

void SpellMgr::addAuraBySpellInfo(SpellInfo* spellInfo, AuraScriptLinker auraScript)
{
    spellInfo->auraScriptLink = (void* (*)) auraScript;
}

void SpellMgr::setupSpellScripts()
{
    setupSpellClassScripts();
}
