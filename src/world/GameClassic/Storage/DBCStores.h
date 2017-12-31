/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#if VERSION_STRING == Classic
#include "../world/Storage/DBC/DBCGlobals.hpp"
#include "Server/Definitions.h"

inline float GetRadius(DBC::Structures::SpellRadiusEntry const* radius)
{
    if (radius == nullptr)
        return 0;

    return radius->radius_min;
}
inline uint32_t GetCastTime(DBC::Structures::SpellCastTimesEntry const* time)
{
    if (time == nullptr)
        return 0;

    return time->CastTime;
}
inline float GetMaxRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->maxRange;
}
inline float GetMinRange(DBC::Structures::SpellRangeEntry const* range)
{
    if (range == nullptr)
        return 0;

    return range->minRange;
}
inline uint32_t GetDuration(DBC::Structures::SpellDurationEntry const* dur)
{
    if (dur == nullptr)
        return 0;
    return dur->Duration1;
}

extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WorldMapOverlayEntry> sWorldMapOverlayStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CharTitlesEntry> sCharTitlesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GemPropertiesEntry> sGemPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemEntry> sItemStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemSetEntry> sItemSetStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LockEntry> sLockStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellEntry> sSpellStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellDurationEntry> sSpellDurationStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRangeEntry> sSpellRangeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellShapeshiftFormEntry> sSpellShapeshiftFormStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::EmotesTextEntry> sEmotesTextStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellRadiusEntry> sSpellRadiusStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellCastTimesEntry> sSpellCastTimesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTableEntry> sAreaStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionTemplateEntry> sFactionTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::FactionEntry> sFactionStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SpellItemEnchantmentEntry> sSpellItemEnchantmentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomPropertiesEntry> sItemRandomPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineAbilityEntry> sSkillLineAbilityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SkillLineEntry> sSkillLineStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiNodesEntry> sTaxiNodesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathEntry> sTaxiPathStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TaxiPathNodeEntry> sTaxiPathNodeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AuctionHouseEntry> sAuctionHouseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentEntry> sTalentStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::TalentTabEntry> sTalentTabStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureDisplayInfoEntry> sCreatureDisplayInfoStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureSpellDataEntry> sCreatureSpellDataStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::CreatureFamilyEntry> sCreatureFamilyStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrClassesEntry> sChrClassesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChrRacesEntry> sChrRacesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MapEntry> sMapStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemExtendedCostEntry> sItemExtendedCostStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ItemRandomSuffixEntry> sItemRandomSuffixStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtCombatRatingsEntry> sGtCombatRatingsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::ChatChannelsEntry> sChatChannelsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityCostsEntry> sDurabilityCostsStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::DurabilityQualityEntry> sDurabilityQualityStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::BankBagSlotPrices> sBankBagSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::StableSlotPrices> sStableSlotPricesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritEntry> sGtChanceToMeleeCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritEntry> sGtChanceToSpellCritStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenMPEntry> sGtOCTRegenMPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenMPPerSptEntry> sGtRegenMPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtOCTRegenHPEntry> sGtOCTRegenHPStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::GtRegenHPPerSptEntry> sGtRegenHPPerSptStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::AreaTriggerEntry> sAreaTriggerStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::MailTemplateEntry> sMailTemplateStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::WMOAreaTableEntry> sWMOAreaTableStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::SummonPropertiesEntry> sSummonPropertiesStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::NameGenEntry> sNameGenStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LFGDungeonEntry> sLFGDungeonStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::LiquidTypeEntry> sLiquidTypeStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleEntry> sVehicleStore;
extern SERVER_DECL DBC::DBCStorage<DBC::Structures::VehicleSeatEntry> sVehicleSeatStore;

DBC::Structures::WMOAreaTableEntry const* GetWMOAreaTableEntryByTriple(int32 root_id, int32 adt_id, int32 group_id);

std::string generateName(uint32_t type = 0);

uint32_t const* getTalentTabPages(uint8_t playerClass);

bool LoadDBCs();
#endif
