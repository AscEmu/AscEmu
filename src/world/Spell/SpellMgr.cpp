/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "SpellMgr.h"

#include "Spell/Definitions/AuraEffects.h"
#include "Spell/Definitions/ProcFlags.h"
#include "Spell/Definitions/SpellDamageType.h"
#include "Spell/Definitions/SpellEffects.h"
#include "Spell/Definitions/SpellEffectTarget.h"
#include "Spell/Definitions/SpellFamily.h"

#include "Map/Area/AreaStorage.hpp"
#include "shared/Log.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/DBC/DBCStores.h"

initialiseSingleton(SpellMgr);

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
        if (player == nullptr || !player->HasQuest(questStart))
            return false;
    }

    if (questEnd != 0)
    {
        if (player == nullptr || player->GetQuestRewardStatus(questEnd))
            return false;
    }

    if (auraSpell != 0)
    {
        if (player == nullptr || (auraSpell > 0 && !player->HasAura(auraSpell)) || (auraSpell < 0 && player->HasAura(-auraSpell)))
            return false;
    }

    // Misc Conditions
    switch (spellId)
    {
        case 58600: //Restricted Flight Zone (Dalaran)
        {
            if (player == nullptr)
                return false;

            if (!player->hasAuraWithAuraEffect(SPELL_AURA_ENABLE_FLIGHT2) && !player->hasAuraWithAuraEffect(SPELL_AURA_FLY))
                return false;
            break;
        }
    }

    return true;
}

SpellMgr::SpellMgr() {}

SpellMgr::~SpellMgr() {}

void SpellMgr::startSpellMgr()
{
    // Load spell data from DBC
    loadSpellInfoData();

    for (const auto itr : mSpellInfoMapStore)
    {
        auto spellInfo = itr.second;

        // Calculate spell coefficient
        setSpellCoefficient(&spellInfo);

        // Custom values
        // todo: if possible, get rid of these
        setSpellEffectAmplitude(&spellInfo);
        setSpellMeleeSpellBool(&spellInfo);
        setSpellRangedSpellBool(&spellInfo);
        setSpellMissingCIsFlags(&spellInfo);
        setSpellOnShapeshiftChange(&spellInfo);
    }

    // Hackfixes
    applyHackFixes();
}

void SpellMgr::loadSpellDataFromDatabase()
{
    // Load spell related SQL tables
    loadSpellCustomOverride();
    loadSpellCoefficientOverride();
    loadSpellAIThreat();
    loadSpellEffectOverride();
    loadSpellAreas();
}

void SpellMgr::loadSpellScripts()
{
    // Setup scripts
    setupSpellScripts();
}

Spell* SpellMgr::newSpell(Object* caster, SpellInfo const* info, bool triggered, Aura* aur)
{
    // For now we send non-const spellinfo to Spell class
    // This will of course be changed later
    auto spellInfo = info != nullptr ? getMutableSpellInfo(info->getId()) : nullptr;

    // Spells with a script
    if (info->spellScriptLink != nullptr)
        return (*SpellScriptLinker(info->spellScriptLink))(caster, spellInfo, triggered, aur);
    
    // Standard spells without a script
    return new Spell(caster, spellInfo, triggered, aur);
}

Aura* SpellMgr::newAura(SpellInfo const* proto, int32_t duration, Object* caster, Unit* target, bool temporary /*= false*/, Item* i_caster /*= nullptr*/)
{
    //\brief... nullptr when downgrading ae from wotlk to tbc (active auras from newer client versions should be removed before entering tbc)

    // For now we send non-const spellinfo to Aura class
    // This will of course be changed later
    auto spellInfo = proto != nullptr ? getMutableSpellInfo(proto->getId()) : nullptr;

    // Auras with a script
    if (proto->auraScriptLink != nullptr)
        return (*AuraScriptLinker(proto->auraScriptLink))(spellInfo, duration, caster, target, temporary, i_caster);
    // Auras with absorb effect
    else if (proto->hasEffectApplyAuraName(SPELL_AURA_SCHOOL_ABSORB))
        return (*AuraScriptLinker(&AbsorbAura::Create))(spellInfo, duration, caster, target, temporary, i_caster);
    
    // Standard auras without a script
    return new Aura(spellInfo, duration, caster, target, temporary, i_caster);
}

void SpellMgr::addSpellById(const uint32_t spellId, SpellScriptLinker spellScript)
{
    auto spellInfo = getMutableSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        LOG_ERROR("SpellMgr::addSpellById : Unknown spell id %u tried to register a spell script, skipped");
        return;
    }
    addSpellBySpellInfo(spellInfo, spellScript);
}

void SpellMgr::addAuraById(const uint32_t spellId, AuraScriptLinker auraScript)
{
    auto spellInfo = getMutableSpellInfo(spellId);
    if (spellInfo == nullptr)
    {
        LOG_ERROR("SpellMgr::addAuraById : Unknown spell id %u tried to register an aura script, skipped");
        return;
    }
    addAuraBySpellInfo(spellInfo, auraScript);
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
#if VERSION_STRING == TBC
        if (spellInfo->getRequiresAreaId() != zone_id && spellInfo->getRequiresAreaId() != area_id)
            return false;
#elif VERSION_STRING >= WotLK
        auto found = false;
        auto areaGroup = sAreaGroupStore.LookupEntry(spellInfo->getRequiresAreaId());
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
            areaGroup = sAreaGroupStore.LookupEntry(areaGroup->next_group);
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
    const auto itr = getSpellInfoMap()->find(spellId);
    if (itr != getSpellInfoMap()->end())
        return &itr->second;

    return nullptr;
}

SpellInfo const* SpellMgr::getSpellInfoByDifficulty(const uint32_t spellDifficultyId, const uint8_t difficulty) const
{
    // Future request: create a new sql table to contain missing spell difficulties
    // it will also be useful for tbc since the dbc file does not exist there
    // Classic has no different difficulties so this will always return nullptr there
#if VERSION_STRING >= WotLK
    const auto spellDifficulty = sSpellDifficultyStore.LookupEntry(spellDifficultyId);
    if (spellDifficulty == nullptr)
        return nullptr;

    // difficulty 0 = normal 5 man dungeon or normal 10 man raid
    // difficulty 1 = heroic 5 man dungeon or normal 25 man raid
    // difficulty 2 = heroic 10 man raid
    // difficulty 3 = heroic 25 man raid
    if (spellDifficulty->SpellId[difficulty] <= 0)
        return nullptr;

    return getSpellInfo(spellDifficulty->SpellId[difficulty]);
#else
    return nullptr;
#endif
}

// Private methods

void SpellMgr::loadSpellInfoData()
{
    for (auto i = 0; i < MAX_SPELL_ID; ++i)
    {
        auto dbcSpellEntry = sSpellStore.LookupEntry(i);
        if (dbcSpellEntry == nullptr)
            continue;

        uint32_t spell_id = dbcSpellEntry->Id;
        SpellInfo& spellInfo = mSpellInfoMapStore[spell_id];

        spellInfo.setId(spell_id);
        spellInfo.setAttributes(dbcSpellEntry->Attributes);
        spellInfo.setAttributesEx(dbcSpellEntry->AttributesEx);
        spellInfo.setAttributesExB(dbcSpellEntry->AttributesExB);
        spellInfo.setAttributesExC(dbcSpellEntry->AttributesExC);
        spellInfo.setAttributesExD(dbcSpellEntry->AttributesExD);
#if VERSION_STRING >= TBC
        spellInfo.setAttributesExE(dbcSpellEntry->AttributesExE);
        spellInfo.setAttributesExF(dbcSpellEntry->AttributesExF);
#endif
#if VERSION_STRING >= WotLK
        spellInfo.setAttributesExG(dbcSpellEntry->AttributesExG);
#endif
        spellInfo.setCastingTimeIndex(dbcSpellEntry->CastingTimeIndex);
        spellInfo.setDurationIndex(dbcSpellEntry->DurationIndex);
        spellInfo.setPowerType(dbcSpellEntry->powerType);
        spellInfo.setRangeIndex(dbcSpellEntry->rangeIndex);
        spellInfo.setSpeed(dbcSpellEntry->speed);
        spellInfo.setSpellVisual(dbcSpellEntry->SpellVisual);
        spellInfo.setSpellIconID(dbcSpellEntry->spellIconID);
        spellInfo.setActiveIconID(dbcSpellEntry->activeIconID);
        spellInfo.setSchool(dbcSpellEntry->School);
#if VERSION_STRING >= WotLK
        spellInfo.setRuneCostID(dbcSpellEntry->RuneCostID);
        spellInfo.setSpellDifficultyID(dbcSpellEntry->SpellDifficultyId);
#endif

#if VERSION_STRING < Cata
        spellInfo.setRequiredShapeShift(dbcSpellEntry->Shapeshifts);
        spellInfo.setShapeshiftExclude(dbcSpellEntry->ShapeshiftsExcluded);
        spellInfo.setTargets(dbcSpellEntry->Targets);
        spellInfo.setTargetCreatureType(dbcSpellEntry->TargetCreatureType);
        spellInfo.setRequiresSpellFocus(dbcSpellEntry->RequiresSpellFocus);
#if VERSION_STRING >= TBC
        spellInfo.setFacingCasterFlags(dbcSpellEntry->FacingCasterFlags);
#endif
        spellInfo.setCasterAuraState(dbcSpellEntry->CasterAuraState);
        spellInfo.setTargetAuraState(dbcSpellEntry->TargetAuraState);
#if VERSION_STRING >= TBC
        spellInfo.setCasterAuraStateNot(dbcSpellEntry->CasterAuraStateNot);
        spellInfo.setTargetAuraStateNot(dbcSpellEntry->TargetAuraStateNot);
#endif
#if VERSION_STRING == WotLK
        spellInfo.setCasterAuraSpell(dbcSpellEntry->casterAuraSpell);
        spellInfo.setTargetAuraSpell(dbcSpellEntry->targetAuraSpell);
        spellInfo.setCasterAuraSpellNot(dbcSpellEntry->casterAuraSpellNot);
        spellInfo.setTargetAuraSpellNot(dbcSpellEntry->targetAuraSpellNot);
#endif
        spellInfo.setRecoveryTime(dbcSpellEntry->RecoveryTime);
        spellInfo.setCategoryRecoveryTime(dbcSpellEntry->CategoryRecoveryTime);
        spellInfo.setInterruptFlags(dbcSpellEntry->InterruptFlags);
        spellInfo.setAuraInterruptFlags(dbcSpellEntry->AuraInterruptFlags);
        spellInfo.setChannelInterruptFlags(dbcSpellEntry->ChannelInterruptFlags);
        spellInfo.setProcFlags(dbcSpellEntry->procFlags);
        spellInfo.setProcChance(dbcSpellEntry->procChance);
        spellInfo.setProcCharges(dbcSpellEntry->procCharges);
        spellInfo.setMaxLevel(dbcSpellEntry->maxLevel);
        spellInfo.setBaseLevel(dbcSpellEntry->baseLevel);
        spellInfo.setSpellLevel(dbcSpellEntry->spellLevel);
        spellInfo.setManaCost(dbcSpellEntry->manaCost);
        spellInfo.setManaCostPerlevel(dbcSpellEntry->manaCostPerlevel);
        spellInfo.setManaPerSecond(dbcSpellEntry->manaPerSecond);
        spellInfo.setManaPerSecondPerLevel(dbcSpellEntry->manaPerSecondPerLevel);
        spellInfo.setMaxstack(dbcSpellEntry->MaxStackAmount);
        for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
            spellInfo.setTotem(dbcSpellEntry->Totem[j], j);
        for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
        {
            spellInfo.setReagent(dbcSpellEntry->Reagent[j], j);
            spellInfo.setReagentCount(dbcSpellEntry->ReagentCount[j], j);
        }
        spellInfo.setEquippedItemClass(dbcSpellEntry->EquippedItemClass);
        spellInfo.setEquippedItemSubClass(dbcSpellEntry->EquippedItemSubClass);
        spellInfo.setEquippedItemInventoryTypeMask(dbcSpellEntry->EquippedItemInventoryTypeMask);
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            spellInfo.setEffect(dbcSpellEntry->Effect[j], j);
            spellInfo.setEffectDieSides(dbcSpellEntry->EffectDieSides[j], j);
            spellInfo.setEffectRealPointsPerLevel(dbcSpellEntry->EffectRealPointsPerLevel[j], j);
            spellInfo.setEffectBasePoints(dbcSpellEntry->EffectBasePoints[j], j);
            spellInfo.setEffectMechanic(dbcSpellEntry->EffectMechanic[j], j);
            spellInfo.setEffectImplicitTargetA(dbcSpellEntry->EffectImplicitTargetA[j], j);
            spellInfo.setEffectImplicitTargetB(dbcSpellEntry->EffectImplicitTargetB[j], j);
            spellInfo.setEffectRadiusIndex(dbcSpellEntry->EffectRadiusIndex[j], j);
            spellInfo.setEffectApplyAuraName(dbcSpellEntry->EffectApplyAuraName[j], j);
            spellInfo.setEffectAmplitude(dbcSpellEntry->EffectAmplitude[j], j);
            spellInfo.setEffectMultipleValue(dbcSpellEntry->EffectMultipleValue[j], j);
            spellInfo.setEffectChainTarget(dbcSpellEntry->EffectChainTarget[j], j);
            spellInfo.setEffectItemType(dbcSpellEntry->EffectItemType[j], j);
            spellInfo.setEffectMiscValue(dbcSpellEntry->EffectMiscValue[j], j);
#if VERSION_STRING >= TBC
            spellInfo.setEffectMiscValueB(dbcSpellEntry->EffectMiscValueB[j], j);
#endif
            spellInfo.setEffectTriggerSpell(dbcSpellEntry->EffectTriggerSpell[j], j);
            spellInfo.setEffectPointsPerComboPoint(dbcSpellEntry->EffectPointsPerComboPoint[j], j);
#if VERSION_STRING == WotLK
            for (uint8_t x = 0; x < 3; ++x)
                spellInfo.setEffectSpellClassMask(dbcSpellEntry->EffectSpellClassMask[j][x], j, x);
#endif
        }
        spellInfo.setSpellPriority(dbcSpellEntry->spellPriority);
        spellInfo.setName(dbcSpellEntry->Name[0]);
        spellInfo.setRank(dbcSpellEntry->Rank[0]);
        spellInfo.setManaCostPercentage(dbcSpellEntry->ManaCostPercentage);
        spellInfo.setStartRecoveryCategory(dbcSpellEntry->StartRecoveryCategory);
        spellInfo.setStartRecoveryTime(dbcSpellEntry->StartRecoveryTime);
        spellInfo.setMaxTargetLevel(dbcSpellEntry->MaxTargetLevel);
        spellInfo.setSpellFamilyName(dbcSpellEntry->SpellFamilyName);
#if VERSION_STRING != WotLK
        for (uint8_t j = 0; j < 2; ++j)
            spellInfo.setSpellFamilyFlags(dbcSpellEntry->SpellFamilyFlags[j], j);
#else
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo.setSpellFamilyFlags(dbcSpellEntry->SpellFamilyFlags[j], j);
#endif
        spellInfo.setMaxTargets(dbcSpellEntry->MaxTargets);
        spellInfo.setDmgClass(dbcSpellEntry->DmgClass);
        spellInfo.setPreventionType(dbcSpellEntry->PreventionType);
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo.setEffectDamageMultiplier(dbcSpellEntry->EffectDamageMultiplier[j], j);
#if VERSION_STRING >= TBC
        for (uint8_t j = 0; j < MAX_SPELL_TOTEM_CATEGORIES; ++j)
            spellInfo.setTotemCategory(dbcSpellEntry->TotemCategory[j], j);
        spellInfo.setRequiresAreaId(dbcSpellEntry->AreaGroupId);
#endif
#if VERSION_STRING == WotLK
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            spellInfo.setEffectBonusMultiplier(dbcSpellEntry->EffectBonusMultiplier[j], j);
#endif
        // Cataclysm begins
#else
        spellInfo.setAttributesExH(dbcSpellEntry->AttributesExH);
        spellInfo.setAttributesExI(dbcSpellEntry->AttributesExI);
        spellInfo.setAttributesExJ(dbcSpellEntry->AttributesExJ);

        spellInfo.setName(dbcSpellEntry->Name);
        spellInfo.setRank(dbcSpellEntry->Rank);

        // Initialize DBC links
        spellInfo.SpellScalingId = dbcSpellEntry->SpellScalingId;
        spellInfo.SpellAuraOptionsId = dbcSpellEntry->SpellAuraOptionsId;
        spellInfo.SpellAuraRestrictionsId = dbcSpellEntry->SpellAuraRestrictionsId;
        spellInfo.SpellCastingRequirementsId = dbcSpellEntry->SpellCastingRequirementsId;
        spellInfo.SpellCategoriesId = dbcSpellEntry->SpellCategoriesId;
        spellInfo.SpellClassOptionsId = dbcSpellEntry->SpellClassOptionsId;
        spellInfo.SpellCooldownsId = dbcSpellEntry->SpellCooldownsId;
        spellInfo.SpellEquippedItemsId = dbcSpellEntry->SpellEquippedItemsId;
        spellInfo.SpellInterruptsId = dbcSpellEntry->SpellInterruptsId;
        spellInfo.SpellLevelsId = dbcSpellEntry->SpellLevelsId;
        spellInfo.SpellPowerId = dbcSpellEntry->SpellPowerId;
        spellInfo.SpellReagentsId = dbcSpellEntry->SpellReagentsId;
        spellInfo.SpellShapeshiftId = dbcSpellEntry->SpellShapeshiftId;
        spellInfo.SpellTargetRestrictionsId = dbcSpellEntry->SpellTargetRestrictionsId;
        spellInfo.SpellTotemsId = dbcSpellEntry->SpellTotemsId;

        // Data from SpellAuraOptions.dbc
        if (dbcSpellEntry->SpellAuraOptionsId && dbcSpellEntry->GetSpellAuraOptions() != nullptr)
        {
            spellInfo.setMaxstack(dbcSpellEntry->GetSpellAuraOptions()->MaxStackAmount);
            spellInfo.setProcChance(dbcSpellEntry->GetSpellAuraOptions()->procChance);
            spellInfo.setProcCharges(dbcSpellEntry->GetSpellAuraOptions()->procCharges);
            spellInfo.setProcFlags(dbcSpellEntry->GetSpellAuraOptions()->procFlags);
        }

        // Data from SpellAuraRestrictions.dbc
        if (dbcSpellEntry->SpellAuraRestrictionsId && dbcSpellEntry->GetSpellAuraRestrictions() != nullptr)
        {
            spellInfo.setCasterAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraState);
            spellInfo.setTargetAuraState(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraState);
            spellInfo.setCasterAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraStateNot);
            spellInfo.setTargetAuraStateNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraStateNot);
            spellInfo.setCasterAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->casterAuraSpell);
            spellInfo.setTargetAuraSpell(dbcSpellEntry->GetSpellAuraRestrictions()->targetAuraSpell);
            spellInfo.setCasterAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->CasterAuraSpellNot);
            spellInfo.setTargetAuraSpellNot(dbcSpellEntry->GetSpellAuraRestrictions()->TargetAuraSpellNot);
        }

        // Data from SpellCastingRequirements.dbc
        if (dbcSpellEntry->SpellCastingRequirementsId && dbcSpellEntry->GetSpellCastingRequirements() != nullptr)
        {
            spellInfo.setFacingCasterFlags(dbcSpellEntry->GetSpellCastingRequirements()->FacingCasterFlags);
            spellInfo.setRequiresAreaId(dbcSpellEntry->GetSpellCastingRequirements()->AreaGroupId);
            spellInfo.setRequiresSpellFocus(dbcSpellEntry->GetSpellCastingRequirements()->RequiresSpellFocus);
        }

        // Data from SpellCategories.dbc
        if (dbcSpellEntry->SpellCategoriesId && dbcSpellEntry->GetSpellCategories() != nullptr)
        {
            spellInfo.setCategory(dbcSpellEntry->GetSpellCategories()->Category);
            spellInfo.setDispelType(dbcSpellEntry->GetSpellCategories()->DispelType);
            spellInfo.setDmgClass(dbcSpellEntry->GetSpellCategories()->DmgClass);
            spellInfo.setMechanicsType(dbcSpellEntry->GetSpellCategories()->MechanicsType);
            spellInfo.setPreventionType(dbcSpellEntry->GetSpellCategories()->PreventionType);
            spellInfo.setStartRecoveryCategory(dbcSpellEntry->GetSpellCategories()->StartRecoveryCategory);
        }

        // Data from SpellClassOptions.dbc
        if (dbcSpellEntry->SpellClassOptionsId && dbcSpellEntry->GetSpellClassOptions() != nullptr)
        {
            spellInfo.setSpellFamilyName(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyName);
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
                spellInfo.setSpellFamilyFlags(dbcSpellEntry->GetSpellClassOptions()->SpellFamilyFlags[j], j);
        }

        // Data from SpellCooldowns.dbc
        if (dbcSpellEntry->SpellCooldownsId && dbcSpellEntry->GetSpellCooldowns() != nullptr)
        {
            spellInfo.setCategoryRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->CategoryRecoveryTime);
            spellInfo.setRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->RecoveryTime);
            spellInfo.setStartRecoveryTime(dbcSpellEntry->GetSpellCooldowns()->StartRecoveryTime);
        }

        // Data from SpellEquippedItems.dbc
        if (dbcSpellEntry->SpellEquippedItemsId && dbcSpellEntry->GetSpellEquippedItems() != nullptr)
        {
            spellInfo.setEquippedItemClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemClass);
            spellInfo.setEquippedItemInventoryTypeMask(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemInventoryTypeMask);
            spellInfo.setEquippedItemSubClass(dbcSpellEntry->GetSpellEquippedItems()->EquippedItemSubClassMask);
        }

        // Data from SpellInterrupts.dbc
        if (dbcSpellEntry->SpellInterruptsId && dbcSpellEntry->GetSpellInterrupts() != nullptr)
        {
            spellInfo.setAuraInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->AuraInterruptFlags);
            spellInfo.setChannelInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->ChannelInterruptFlags);
            spellInfo.setInterruptFlags(dbcSpellEntry->GetSpellInterrupts()->InterruptFlags);
        }

        // Data from SpellLevels.dbc
        if (dbcSpellEntry->SpellLevelsId && dbcSpellEntry->GetSpellLevels() != nullptr)
        {
            spellInfo.setBaseLevel(dbcSpellEntry->GetSpellLevels()->baseLevel);
            spellInfo.setMaxLevel(dbcSpellEntry->GetSpellLevels()->maxLevel);
            spellInfo.setSpellLevel(dbcSpellEntry->GetSpellLevels()->spellLevel);
        }

        // Data from SpellPower.dbc
        if (dbcSpellEntry->SpellPowerId && dbcSpellEntry->GetSpellPower() != nullptr)
        {
            spellInfo.setManaCost(dbcSpellEntry->GetSpellPower()->manaCost);
            spellInfo.setManaCostPerlevel(dbcSpellEntry->GetSpellPower()->manaCostPerlevel);
            spellInfo.setManaCostPercentage(dbcSpellEntry->GetSpellPower()->ManaCostPercentage);
            spellInfo.setManaPerSecond(dbcSpellEntry->GetSpellPower()->manaPerSecond);
            spellInfo.setManaPerSecondPerLevel(dbcSpellEntry->GetSpellPower()->manaPerSecondPerLevel);
        }

        // Data from SpellReagents.dbc
        if (dbcSpellEntry->SpellReagentsId && dbcSpellEntry->GetSpellReagents() != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_REAGENTS; ++j)
            {
                spellInfo.setReagent(dbcSpellEntry->GetSpellReagents()->Reagent[j], j);
                spellInfo.setReagentCount(dbcSpellEntry->GetSpellReagents()->ReagentCount[j], j);
            }
        }

        // Data from SpellShapeshift.dbc
        if (dbcSpellEntry->SpellShapeshiftId && dbcSpellEntry->GetSpellShapeshift() != nullptr)
        {
            spellInfo.setRequiredShapeShift(dbcSpellEntry->GetSpellShapeshift()->Shapeshifts);
            spellInfo.setShapeshiftExclude(dbcSpellEntry->GetSpellShapeshift()->ShapeshiftsExcluded);
        }

        // Data from SpellTargetRestrictions.dbc
        if (dbcSpellEntry->SpellTargetRestrictionsId && dbcSpellEntry->GetSpellTargetRestrictions() != nullptr)
        {
            spellInfo.setMaxTargets(dbcSpellEntry->GetSpellTargetRestrictions()->MaxAffectedTargets);
            spellInfo.setMaxTargetLevel(dbcSpellEntry->GetSpellTargetRestrictions()->MaxTargetLevel);
            spellInfo.setTargetCreatureType(dbcSpellEntry->GetSpellTargetRestrictions()->TargetCreatureType);
            spellInfo.setTargets(dbcSpellEntry->GetSpellTargetRestrictions()->Targets);
        }

        // Data from SpellTotems.dbc
        if (dbcSpellEntry->SpellTotemsId && dbcSpellEntry->GetSpellTotems() != nullptr)
        {
            for (uint8_t j = 0; j < MAX_SPELL_TOTEMS; ++j)
            {
                spellInfo.setTotemCategory(dbcSpellEntry->GetSpellTotems()->TotemCategory[j], j);
                spellInfo.setTotem(dbcSpellEntry->GetSpellTotems()->Totem[j], j);
            }
        }

        // Data from SpellEffect.dbc
        for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
        {
            const auto spell_effect_entry = GetSpellEffectEntry(spell_id, j);
            if (spell_effect_entry != nullptr)
            {
                spellInfo.setEffect(spell_effect_entry->Effect, j);
                spellInfo.setEffectMultipleValue(spell_effect_entry->EffectMultipleValue, j);
                spellInfo.setEffectApplyAuraName(spell_effect_entry->EffectApplyAuraName, j);
                spellInfo.setEffectAmplitude(spell_effect_entry->EffectAmplitude, j);
                spellInfo.setEffectBasePoints(spell_effect_entry->EffectBasePoints, j);
                spellInfo.setEffectBonusMultiplier(spell_effect_entry->EffectBonusMultiplier, j);
                spellInfo.setEffectDamageMultiplier(spell_effect_entry->EffectDamageMultiplier, j);
                spellInfo.setEffectChainTarget(spell_effect_entry->EffectChainTarget, j);
                spellInfo.setEffectDieSides(spell_effect_entry->EffectDieSides, j);
                spellInfo.setEffectItemType(spell_effect_entry->EffectItemType, j);
                spellInfo.setEffectMechanic(spell_effect_entry->EffectMechanic, j);
                spellInfo.setEffectMiscValue(spell_effect_entry->EffectMiscValue, j);
                spellInfo.setEffectMiscValueB(spell_effect_entry->EffectMiscValueB, j);
                spellInfo.setEffectPointsPerComboPoint(spell_effect_entry->EffectPointsPerComboPoint, j);
                spellInfo.setEffectRadiusIndex(spell_effect_entry->EffectRadiusIndex, j);
                spellInfo.setEffectRadiusMaxIndex(spell_effect_entry->EffectRadiusMaxIndex, j);
                spellInfo.setEffectRealPointsPerLevel(spell_effect_entry->EffectRealPointsPerLevel, j);
                for (uint8_t x = 0; x < 3; ++x)
                    spellInfo.setEffectSpellClassMask(spell_effect_entry->EffectSpellClassMask[x], j, x);
                spellInfo.setEffectTriggerSpell(spell_effect_entry->EffectTriggerSpell, j);
                spellInfo.setEffectImplicitTargetA(spell_effect_entry->EffectImplicitTargetA, j);
                spellInfo.setEffectImplicitTargetB(spell_effect_entry->EffectImplicitTargetB, j);
                spellInfo.setEffectSpellId(spell_effect_entry->EffectSpellId, j);
                spellInfo.setEffectIndex(spell_effect_entry->EffectIndex, j);
            }
        }
#endif
    }
}

void SpellMgr::loadSpellCoefficientOverride()
{
    //                                                  0           1                     2
    const auto result = WorldDatabase.Query("SELECT spell_id, direct_coefficient, overtime_coefficient "
                                            "FROM spell_coefficient_override WHERE min_build <= %u AND max_build >= %u", VERSION_STRING, VERSION_STRING);

    if (result == nullptr)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::loadSpellCoefficientOverride : Your `spell_coefficient_override` table is empty!");
        return;
    }

    auto overridenCoeffs = 0;
    do
    {
        const auto fields = result->Fetch();
        auto spellInfo = getMutableSpellInfo(fields[0].GetUInt32());
        if (spellInfo == nullptr)
        {
            LOG_ERROR("Table `spell_coefficient_override` has unknown spell entry %u, skipped", fields[0].GetUInt32());
            continue;
        }

        const auto direct_override = fields[1].GetFloat();
        const auto overtime_override = fields[2].GetFloat();
        // Coeff can be overridden to 0 when it won't receive any bonus from spell power (default value is -1)
        if (direct_override >= 0)
            spellInfo->spell_coeff_direct_override = direct_override;
        if (overtime_override >= 0)
            spellInfo->spell_coeff_overtime_override = overtime_override;
        ++overridenCoeffs;
    } while (result->NextRow());
    delete result;

    LOG_DETAIL("Loaded %u override values from `spell_coefficient_override` table", overridenCoeffs);
}

void SpellMgr::loadSpellCustomOverride()
{
    //                                             0     1            2                         3                  4               5              6              7            8
    const auto result = WorldDatabase.Query("SELECT spell_id, rank, assign_on_target_flag, assign_self_cast_only, assign_c_is_flag, proc_flags, proc_target_selfs, proc_chance, proc_charges, "
    //                                      9                       10                           11                         12
                                      "proc_interval, proc_effect_trigger_spell_0, proc_effect_trigger_spell_1, proc_effect_trigger_spell_2 FROM spell_custom_override");
    
    if (result == nullptr)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::loadSpellCustomOverride : Your `spell_custom_override` table is empty!");
        return;
    }

    auto overridenSpells = 0;
    do
    {
        const auto fields = result->Fetch();

        auto spellInfo = getMutableSpellInfo(fields[0].GetUInt32());
        if (spellInfo == nullptr)
        {
            LOG_ERROR("Table `spell_custom_override` has unknown spell entry %u, skipped", fields[0].GetUInt32());
            continue;
        }

        // rank
        if (fields[1].isSet())
            spellInfo->custom_RankNumber = fields[1].GetUInt32();

        // assign_on_target_flag
        if (fields[2].isSet())
            spellInfo->custom_BGR_one_buff_on_target = fields[2].GetUInt32();

        // assign_self_cast_only
        if (fields[3].isSet())
            spellInfo->custom_self_cast_only = fields[3].GetBool();

        // assign_c_is_flag
        if (fields[4].isSet())
            spellInfo->custom_c_is_flags = fields[4].GetUInt32();

        //proc_flags
        if (fields[5].isSet())
            spellInfo->setProcFlags(fields[5].GetUInt32());

        //proc_target_selfs
        if (fields[6].isSet() && fields[6].GetBool())
            spellInfo->addProcFlags(PROC_TARGET_SELF);

        //proc_chance
        if (fields[7].isSet())
            spellInfo->setProcChance(fields[7].GetUInt32());

        //proc_charges
        if (fields[8].isSet())
            spellInfo->setProcCharges(fields[8].GetUInt32());

        //proc_interval
        if (fields[9].isSet())
            spellInfo->custom_proc_interval = fields[9].GetUInt32();

        //proc_effect_trigger_spell_0
        if (fields[10].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[10].GetUInt32(), 0);
            if (spellInfo->getEffectTriggerSpell(0) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 0);
        }

        //proc_effect_trigger_spell_1
        if (fields[11].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[11].GetUInt32(), 1);
            if (spellInfo->getEffectTriggerSpell(1) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 1);
        }

        //proc_effect_trigger_spell_2
        if (fields[12].isSet())
        {
            spellInfo->setEffectTriggerSpell(fields[12].GetUInt32(), 2);
            if (spellInfo->getEffectTriggerSpell(2) > 0)
                spellInfo->setEffectApplyAuraName(SPELL_AURA_PROC_TRIGGER_SPELL, 2);
        }

        ++overridenSpells;
    } while (result->NextRow());
    delete result;
   
    LOG_DETAIL("Loaded %u override values from `spell_custom_override` table", overridenSpells);
}

void SpellMgr::loadSpellAIThreat()
{
    const auto result = WorldDatabase.Query("SELECT * FROM ai_threattospellid");
    if (result == nullptr)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::loadSpellAIThreat : Your `ai_threattospellid` table is empty!");
        return;
    }

    auto threatCount = 0;
    do
    {
        const auto fields = result->Fetch();
        const auto spellId = fields[0].GetUInt32();

        auto spellInfo = getMutableSpellInfo(spellId);
        if (spellInfo == nullptr)
        {
            LOG_ERROR("Table `ai_threattospellid` has invalid spell entry %u, skipped", spellId);
            continue;
        }

        spellInfo->custom_ThreatForSpell = fields[1].GetInt32();
        spellInfo->custom_ThreatForSpellCoef = fields[2].GetFloat();

        ++threatCount;
    } while (result->NextRow());
    delete result;

    LOG_DETAIL("Loaded %u spell ai threat", threatCount);
}

void SpellMgr::loadSpellEffectOverride()
{
    const auto result = WorldDatabase.Query("SELECT * FROM spell_effects_override");
    if (result == nullptr)
    {
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::loadSpellEffectOverride : Your `spell_effects_override` table is empty!");
        return;
    }

    auto overridenEffects = 0;
    do
    {
        const auto fields = result->Fetch();
        uint32_t seo_SpellId = fields[0].GetUInt32();
        uint8_t seo_EffectId = fields[1].GetUInt8();
        uint32_t seo_Disable = fields[2].GetUInt32();
        uint32_t seo_Effect = fields[3].GetUInt32();
        uint32_t seo_BasePoints = fields[4].GetUInt32();
        uint32_t seo_ApplyAuraName = fields[5].GetUInt32();
        //uint32_t seo_SpellGroupRelation = fields[6].GetUInt32();
        uint32_t seo_MiscValue = fields[7].GetUInt32();
        uint32_t seo_TriggerSpell = fields[8].GetUInt32();
        uint32_t seo_ImplicitTargetA = fields[9].GetUInt32();
        uint32_t seo_ImplicitTargetB = fields[10].GetUInt32();
        uint32_t seo_EffectCustomFlag = fields[11].GetUInt32();

        auto spellInfo = getMutableSpellInfo(seo_SpellId);
        if (spellInfo == nullptr)
        {
            LOG_ERROR("Table `spell_effects_override` has invalid spell entry %u, skipped", seo_SpellId);
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
    delete result;

    LOG_DETAIL("Loaded %u spell effect overrides", overridenEffects);
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
        LogDebugFlag(LF_DB_TABLES, "SpellMgr::loadSpellAreas : Your `spell_area` table is empty!");
        return;
    }

    auto areaCount = 0;
    do
    {
        const auto fields = result->Fetch();

        uint32_t spellId = fields[0].GetUInt32();

        SpellArea spellArea;
        spellArea.spellId = spellId;
        spellArea.areaId = fields[1].GetUInt32();
        spellArea.questStart = fields[2].GetUInt32();
        spellArea.questStartCanActive = fields[3].GetBool();
        spellArea.questEnd = fields[4].GetUInt32();
        spellArea.auraSpell = fields[5].GetUInt32();
        spellArea.raceMask = fields[6].GetUInt32();
        spellArea.gender = Gender(fields[7].GetUInt32());
        spellArea.autoCast = fields[8].GetBool();

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
            LOG_ERROR("Table `spell_area` has duplicate spell entry (%u) with similiar requirements, skipped", spellId);
            continue;
        }

        // Check if the area id is a valid area id or zone id
        if (spellArea.areaId > 0)
        {
            const auto areaEntry = MapManagement::AreaManagement::AreaStorage::GetAreaById(spellArea.areaId);
            if (areaEntry == nullptr)
            {
                LOG_ERROR("Table `spell_area` has invalid area id %u for spell entry %u, skipped", spellArea.areaId, spellId);
                continue;
            }
        }

        // Check if the start quest is a valid quest
        if (spellArea.questStart > 0)
        {
            const auto startQuest = sMySQLStore.getQuestProperties(spellArea.questStart);
            if (startQuest == nullptr)
            {
                LOG_ERROR("Table `spell_area` has invalid quest id %u for spell entry %u, skipped", spellArea.questStart, spellId);
                continue;
            }
        }
        
        // Check if the end quest is a valid quest
        if (spellArea.questEnd > 0)
        {
            const auto endQuest = sMySQLStore.getQuestProperties(spellArea.questEnd);
            if (endQuest == nullptr)
            {
                LOG_ERROR("Table `spell_area` has invalid quest id %u for spell entry %u, skipped", spellArea.questEnd, spellId);
                continue;
            }

            // Check if the end quest is same as start quest
            if (spellArea.questEnd == spellArea.questStart && !spellArea.questStartCanActive)
            {
                LOG_ERROR("Table `spell_area` has quest (id %u) requirement for spell entry %u for start and end at the same time, skipped", spellArea.questEnd, spellId);
                continue;
            }
        }

        // Check for auraspell
        if (spellArea.auraSpell != 0)
        {
            const auto spellInfo = getSpellInfo(abs(spellArea.auraSpell));
            if (spellInfo == nullptr)
            {
                LOG_ERROR("Table `spell_area` has invalid aura spell entry %u for spell %u, skipped", abs(spellArea.auraSpell), spellId);
                continue;
            }

            if (uint32_t(abs(spellArea.auraSpell)) == spellArea.spellId)
            {
                LOG_ERROR("Table `spell_area` has aura spell requirements for itself (id %u), skipped", spellId);
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
                    LOG_ERROR("Table `spell_area` has aura spell (id %u) requirement that itself was autocasted from an aura, skipping", spellId);
                    continue;
                }

                const auto spellAreaMapBounds2 = getSpellAreaMapBounds(spellArea.auraSpell);
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
                    LOG_ERROR("Table `spell_area` has aura spell (id %u) requirement that itself was autocasted from an aura, skipping", spellId);
                    continue;
                }
            }
        }

        if (spellArea.raceMask > 0 && !(spellArea.raceMask & RACEMASK_ALL_PLAYABLE))
        {
            LOG_ERROR("Table `spell_area` has invalid racemask for spell entry %u, skipped", spellId);
            continue;
        }

        if (spellArea.gender != GENDER_NONE && spellArea.gender != GENDER_MALE && spellArea.gender != GENDER_FEMALE)
        {
            LOG_ERROR("Table `spell_area` has invalid gender for spell entry %u, skipped", spellId);
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
            mSpellAreaForAuraMap.insert(SpellAreaForAuraMap::value_type(uint32_t(abs(spellArea.auraSpell)), spellArea2));

        ++areaCount;
    } while (result->NextRow());
    delete result;

    LOG_DETAIL("Loaded %u spell area requirements", areaCount);
}

void SpellMgr::setSpellCoefficient(SpellInfo* sp)
{
    const auto baseDuration = float(GetDuration(sSpellDurationStore.LookupEntry(sp->getDurationIndex())));
    const auto isOverTimeSpell = sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) || sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL);

    // Get coefficient override from database if any is set
    if (sp->spell_coeff_direct_override != -1)
    {
        sp->spell_coeff_direct = sp->spell_coeff_direct_override;

        // Store coeff value as per missile for channeled spells
        if (sp->isChanneled())
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
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
    if (sp->spell_coeff_overtime_override != -1)
    {
        sp->spell_coeff_overtime = sp->spell_coeff_overtime_override;

        // Store coeff value as per tick for over-time spells
        if (isOverTimeSpell)
        {
            for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
            {
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
                {
                    sp->spell_coeff_overtime /= baseDuration / sp->getEffectAmplitude(i);
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
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_DAMAGE) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_HEAL) ||
            sp->hasEffectApplyAuraName(SPELL_AURA_PERIODIC_LEECH))
            return true;

        return false;
    };

    // Calculate base spell coefficient
    auto spellCastTime = float(GetCastTime(sSpellCastTimesStore.LookupEntry(sp->getCastingTimeIndex())));
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
        if (sp->getBaseLevel() < 20)
        {
            const auto penalty = 1.0f - ((20.0f - float(sp->getBaseLevel())) * 0.0375f);
            sp->spell_coeff_direct *= penalty;
        }

        // Store coeff value as per missile / tick
        for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_DAMAGE ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_HEAL ||
                sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_LEECH)
            {
                sp->spell_coeff_direct /= baseDuration / sp->getEffectAmplitude(i);
                // For channeled spells which trigger another spell on each "missile", set triggered spell's coeff to match master spell's coeff
                if (sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL ||
                    sp->getEffectApplyAuraName(i) == SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE)
                {
                    auto triggeredSpell = sSpellMgr.getMutableSpellInfo(sp->getEffectTriggerSpell(i));
                    // But do not alter its coefficient if it's already overridden in database
                    if (triggeredSpell != nullptr && triggeredSpell->spell_coeff_direct_override == -1)
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
        if (sp->getBaseLevel() < 20)
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
        if (sp->getBaseLevel() < 20)
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
        if (sp->getBaseLevel() < 20)
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

SpellInfo* SpellMgr::getMutableSpellInfo(const uint32_t spellId)
{
    const auto itr = getSpellInfoMap()->find(spellId);
    if (itr != getSpellInfoMap()->end())
        return const_cast<SpellInfo*>(&itr->second);

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
