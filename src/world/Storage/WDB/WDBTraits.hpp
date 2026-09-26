/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBStructures.hpp"
#include "WDBStructuresRaw.hpp"

namespace WDB
{
    template <std::size_t N>
    struct FixedString
    {
        char value[N]{};

        constexpr FixedString(char const (&str)[N])
        {
            std::copy_n(str, N, value);
        }

        constexpr std::string_view view() const noexcept { return {value, N - 1}; }
    };

    template <FixedString Stem>
    struct StandardDbcFile
    {
        static constexpr auto dbcName = []() {
            char buf[Stem.view().size() + 5]{};
            std::copy_n(Stem.value, Stem.view().size(), buf);
            std::copy_n(".dbc", 5, buf + Stem.view().size());
            return FixedString(buf);
        }();

        static constexpr auto db2Name = []() {
            char buf[Stem.view().size() + 5]{};
            std::copy_n(Stem.value, Stem.view().size(), buf);
            std::copy_n(".db2", 5, buf + Stem.view().size());
            return FixedString(buf);
        }();

        [[nodiscard]] static constexpr std::string_view getFilename(WoW::Expansion expansion) noexcept
        {
            if (expansion >= WoW::Expansion::_WoD)
                return db2Name.view();
            return dbcName.view();
        }
    };

    struct UnsupportedVersion {};

    template <typename T> struct DbcTraits;

    template <typename ClassicLayout, typename TbcLayout, typename WotlkLayout, typename CataLayout, typename MopLayout,
              typename WodLayout = UnsupportedVersion,
              typename LegionLayout = UnsupportedVersion>

    struct DbcVersionLayouts
    {
        using classic = ClassicLayout;
        using tbc = TbcLayout;
        using wotlk = WotlkLayout;
        using cata = CataLayout;
        using mop = MopLayout;
        using wod = WodLayout;
        using legion = LegionLayout;
    };

    template <>
    struct DbcTraits<Structures::AreaGroupEntry> :
        DbcVersionLayouts<
            UnsupportedVersion,
            UnsupportedVersion,
            Structures::Raw::AreaGroupEntryWotlkCataMop,
            Structures::Raw::AreaGroupEntryWotlkCataMop,
            Structures::Raw::AreaGroupEntryWotlkCataMop>,
        StandardDbcFile<"AreaGroup">
    {};

    template <>
    struct DbcTraits<Structures::AreaTableEntry> :
        DbcVersionLayouts<
            Structures::Raw::AreaTableEntryClassic,
            Structures::Raw::AreaTableEntryTbc,
            Structures::Raw::AreaTableEntryWotlk,
            Structures::Raw::AreaTableEntryCata,
            Structures::Raw::AreaTableEntryMop>,
        StandardDbcFile<"AreaTable">
    {};

    template <>
    struct DbcTraits<Structures::AreaTriggerEntry> :
        DbcVersionLayouts<
            Structures::Raw::AreaTriggerEntryClassic,
            Structures::Raw::AreaTriggerEntryTbc,
            Structures::Raw::AreaTriggerEntryWotlk,
            Structures::Raw::AreaTriggerEntryCata,
            Structures::Raw::AreaTriggerEntryMop>,
        StandardDbcFile<"AreaTrigger">
    {};

    template <>
    struct DbcTraits<Structures::AuctionHouseEntry> :
        DbcVersionLayouts<
            Structures::Raw::AuctionHouseEntryClassic,
            Structures::Raw::AuctionHouseEntryTbc,
            Structures::Raw::AuctionHouseEntryWotlk,
            Structures::Raw::AuctionHouseEntryCata,
            Structures::Raw::AuctionHouseEntryMop>,
        StandardDbcFile<"AuctionHouse">
    {};

    template <>
    struct DbcTraits<Structures::BankBagSlotPricesEntry> :
        DbcVersionLayouts<
            Structures::Raw::BankBagSlotPricesEntryClassic,
            Structures::Raw::BankBagSlotPricesEntryTbc,
            Structures::Raw::BankBagSlotPricesEntryWotlk,
            Structures::Raw::BankBagSlotPricesEntryCata,
            Structures::Raw::BankBagSlotPricesEntryMop>,
        StandardDbcFile<"BankBagSlotPrices">
    {};

    template <>
    struct DbcTraits<Structures::BannedAddOnsEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            UnsupportedVersion, // TBC
            UnsupportedVersion, // WotLK
            Structures::Raw::BannedAddOnsEntryCata,
            Structures::Raw::BannedAddOnsEntryMop>,
        StandardDbcFile<"BannedAddOns">
    {};

    template <>
    struct DbcTraits<Structures::BarberShopStyleEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            UnsupportedVersion, // TBC
            Structures::Raw::BarberShopStyleEntryWotlkCataMop,
            Structures::Raw::BarberShopStyleEntryWotlkCataMop,
            Structures::Raw::BarberShopStyleEntryWotlkCataMop>,
        StandardDbcFile<"BarberShopStyle">
    {};

    template <>
    struct DbcTraits<Structures::ChatChannelsEntry>
        : DbcVersionLayouts<
              Structures::Raw::ChatChannelsEntryClassic,
              Structures::Raw::ChatChannelsEntryTbc,
              Structures::Raw::ChatChannelsEntryWotlk,
              Structures::Raw::ChatChannelsEntryCata,
              Structures::Raw::ChatChannelsEntryMop>,
          StandardDbcFile<"ChatChannels">
    {};

    template <>
    struct DbcTraits<Structures::CharStartOutfitEntry> :
        DbcVersionLayouts<
            Structures::Raw::CharStartOutfitEntryClassic,
            Structures::Raw::CharStartOutfitEntryTbc,
            Structures::Raw::CharStartOutfitEntryWotlk,
            Structures::Raw::CharStartOutfitEntryCata,
            Structures::Raw::CharStartOutfitEntryMop>,
        StandardDbcFile<"CharStartOutfit">
    {};

    template <>
    struct DbcTraits<Structures::CharTitlesEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            Structures::Raw::CharTitlesEntryTbcWotlk,
            Structures::Raw::CharTitlesEntryTbcWotlk,
            Structures::Raw::CharTitlesEntryCataMop,
            Structures::Raw::CharTitlesEntryCataMop>,
        StandardDbcFile<"CharTitles">
    {};

    template <>
    struct DbcTraits<Structures::ChrClassesEntry> :
        DbcVersionLayouts<
            Structures::Raw::ChrClassesEntryClassic,
            Structures::Raw::ChrClassesEntryTbc,
            Structures::Raw::ChrClassesEntryWotlk,
            Structures::Raw::ChrClassesEntryCata,
            Structures::Raw::ChrClassesEntryMop>,
        StandardDbcFile<"ChrClasses">
    {};

    template <>
    struct DbcTraits<Structures::ChrPowerTypesEntry> :
        DbcVersionLayouts<
            UnsupportedVersion,
            UnsupportedVersion,
            UnsupportedVersion,
            Structures::Raw::ChrPowerTypesEntryCata,
            Structures::Raw::ChrPowerTypesEntryMop>,
        StandardDbcFile<"ChrClassesXPowerTypes">
    {};

    template <>
    struct DbcTraits<Structures::ChrRacesEntry> :
        DbcVersionLayouts<
            Structures::Raw::ChrRacesEntryClassic,
            Structures::Raw::ChrRacesEntryTbc,
            Structures::Raw::ChrRacesEntryWotlk,
            Structures::Raw::ChrRacesEntryCata,
            Structures::Raw::ChrRacesEntryMop>,
        StandardDbcFile<"ChrRaces">
    {};

    template <>
    struct DbcTraits<Structures::CreatureDisplayInfoEntry> :
        DbcVersionLayouts<
            Structures::Raw::CreatureDisplayInfoEntryClassic,
            Structures::Raw::CreatureDisplayInfoEntryTbc,
            Structures::Raw::CreatureDisplayInfoEntryWotlk,
            Structures::Raw::CreatureDisplayInfoEntryCata,
            Structures::Raw::CreatureDisplayInfoEntryMop>,
        StandardDbcFile<"CreatureDisplayInfo">
    {};

    template <>
    struct DbcTraits<Structures::CreatureDisplayInfoExtraEntry> :
        DbcVersionLayouts<
            Structures::Raw::CreatureDisplayInfoExtraEntryClassic,
            Structures::Raw::CreatureDisplayInfoExtraEntryTbc,
            Structures::Raw::CreatureDisplayInfoExtraEntryWotlk,
            Structures::Raw::CreatureDisplayInfoExtraEntryCata,
            Structures::Raw::CreatureDisplayInfoExtraEntryMop>,
        StandardDbcFile<"CreatureDisplayInfoExtra">
    {};

    template <>
    struct DbcTraits<Structures::CreatureFamilyEntry> :
        DbcVersionLayouts<
            Structures::Raw::CreatureFamilyEntryClassic,
            Structures::Raw::CreatureFamilyEntryTbc,
            Structures::Raw::CreatureFamilyEntryWotlk,
            Structures::Raw::CreatureFamilyEntryCata,
            Structures::Raw::CreatureFamilyEntryMop>,
        StandardDbcFile<"CreatureFamily">
    {};

    template <>
    struct DbcTraits<Structures::CreatureModelDataEntry> :
        DbcVersionLayouts<
            Structures::Raw::CreatureModelDataEntryClassic,
            Structures::Raw::CreatureModelDataEntryTbc,
            Structures::Raw::CreatureModelDataEntryWotlk,
            Structures::Raw::CreatureModelDataEntryCata,
            Structures::Raw::CreatureModelDataEntryMop>,
        StandardDbcFile<"CreatureModelData">
    {};

    template <>
    struct DbcTraits<Structures::CreatureSpellDataEntry> :
        DbcVersionLayouts<
            Structures::Raw::CreatureSpellDataEntryClassic,
            Structures::Raw::CreatureSpellDataEntryTbc,
            Structures::Raw::CreatureSpellDataEntryWotlk,
            Structures::Raw::CreatureSpellDataEntryCata,
            Structures::Raw::CreatureSpellDataEntryMop>,
        StandardDbcFile<"CreatureSpellData">
    {};

    template <>
    struct DbcTraits<Structures::FactionEntry> :
        DbcVersionLayouts<
            Structures::Raw::FactionEntryClassic,
            Structures::Raw::FactionEntryTbc,
            Structures::Raw::FactionEntryWotlk,
            Structures::Raw::FactionEntryCata,
            Structures::Raw::FactionEntryMop>,
        StandardDbcFile<"Faction">
    {};

    template <>
    struct DbcTraits<Structures::DurabilityCostsEntry> :
        DbcVersionLayouts<
            Structures::Raw::DurabilityCostsEntryClassic,
            Structures::Raw::DurabilityCostsEntryTbc,
            Structures::Raw::DurabilityCostsEntryWotlk,
            Structures::Raw::DurabilityCostsEntryCata,
            Structures::Raw::DurabilityCostsEntryMop>,
        StandardDbcFile<"DurabilityCosts">
    {};

    template <>
    struct DbcTraits<Structures::DurabilityQualityEntry> :
        DbcVersionLayouts<
            Structures::Raw::DurabilityQualityEntryClassic,
            Structures::Raw::DurabilityQualityEntryTbc,
            Structures::Raw::DurabilityQualityEntryWotlk,
            Structures::Raw::DurabilityQualityEntryCata,
            Structures::Raw::DurabilityQualityEntryMop>,
        StandardDbcFile<"DurabilityQuality">
    {};

    template <>
    struct DbcTraits<Structures::EmotesTextEntry> :
        DbcVersionLayouts<
            Structures::Raw::EmotesTextEntryClassic,
            Structures::Raw::EmotesTextEntryTbc,
            Structures::Raw::EmotesTextEntryWotlk,
            Structures::Raw::EmotesTextEntryCata,
            Structures::Raw::EmotesTextEntryMop>,
        StandardDbcFile<"EmotesText">
    {};

    template <>
    struct DbcTraits<Structures::FactionTemplateEntry> :
        DbcVersionLayouts<
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll>,
        StandardDbcFile<"FactionTemplate">
    {};

    template <>
    struct DbcTraits<Structures::GameObjectDisplayInfoEntry> :
        DbcVersionLayouts<
            Structures::Raw::GameObjectDisplayInfoEntryClassic,
            Structures::Raw::GameObjectDisplayInfoEntryTbc,
            Structures::Raw::GameObjectDisplayInfoEntryWotlk,
            Structures::Raw::GameObjectDisplayInfoEntryCata,
            Structures::Raw::GameObjectDisplayInfoEntryMop>,
        StandardDbcFile<"GameObjectDisplayInfo">
    {};

    template <>
    struct DbcTraits<Structures::GemPropertiesEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            Structures::Raw::GemPropertiesEntryTbcWotlkCataMop,
            Structures::Raw::GemPropertiesEntryTbcWotlkCataMop,
            Structures::Raw::GemPropertiesEntryTbcWotlkCataMop,
            Structures::Raw::GemPropertiesEntryTbcWotlkCataMop>,
        StandardDbcFile<"GemProperties">
    {};

    template <>
    struct DbcTraits<Structures::ItemSetEntry> :
        DbcVersionLayouts<
            Structures::Raw::ItemSetEntryClassic,
            Structures::Raw::ItemSetEntryTbc,
            Structures::Raw::ItemSetEntryWotlk,
            Structures::Raw::ItemSetEntryCata,
            Structures::Raw::ItemSetEntryMop>,
        StandardDbcFile<"ItemSet">
    {};

    template <>
    struct DbcTraits<Structures::MapDifficultyEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            UnsupportedVersion, // TBC
            Structures::Raw::MapDifficultyEntryWotlkCataMop,
            Structures::Raw::MapDifficultyEntryWotlkCataMop,
            Structures::Raw::MapDifficultyEntryWotlkCataMop>,
        StandardDbcFile<"MapDifficulty">
    {};

    template <>
    struct DbcTraits<Structures::MapEntry> :
        DbcVersionLayouts<
            Structures::Raw::MapEntryClassic,
            Structures::Raw::MapEntryTbc,
            Structures::Raw::MapEntryWotlk,
            Structures::Raw::MapEntryCataMop,
            Structures::Raw::MapEntryCataMop>,
        StandardDbcFile<"Map">
    {};

    template <>
    struct DbcTraits<Structures::StableSlotPricesEntry> :
        DbcVersionLayouts<
            Structures::Raw::StableSlotPricesEntryClassicTbcWotlk,
            Structures::Raw::StableSlotPricesEntryClassicTbcWotlk,
            Structures::Raw::StableSlotPricesEntryClassicTbcWotlk,
            UnsupportedVersion, // Cata
            UnsupportedVersion>, // MoP
        StandardDbcFile<"StableSlotPrices">
    {};

    template <>
    struct DbcTraits<Structures::TotemCategoryEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            Structures::Raw::TotemCategoryEntryTbcWotlkCataMop,
            Structures::Raw::TotemCategoryEntryTbcWotlkCataMop,
            Structures::Raw::TotemCategoryEntryTbcWotlkCataMop,
            Structures::Raw::TotemCategoryEntryTbcWotlkCataMop>,
        StandardDbcFile<"TotemCategory">
    {};

    template <>
    struct DbcTraits<Structures::WorldMapAreaEntry> :
        DbcVersionLayouts<
            UnsupportedVersion, // Classic
            Structures::Raw::WorldMapAreaEntryTbcWotlkCataMop,
            Structures::Raw::WorldMapAreaEntryTbcWotlkCataMop,
            Structures::Raw::WorldMapAreaEntryTbcWotlkCataMop,
            Structures::Raw::WorldMapAreaEntryTbcWotlkCataMop>,
        StandardDbcFile<"WorldMapArea">
    {};
}
