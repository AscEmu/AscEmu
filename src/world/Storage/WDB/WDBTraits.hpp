/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WDBStructures.hpp"
#include "WDBStructuresRaw.hpp"

namespace WDB
{
    struct UnsupportedVersion {};

    template <typename T> struct DbcTraits;

    template <typename ClassicLayout, typename TbcLayout, typename WotlkLayout, typename CataLayout, typename MopLayout>
    struct DbcVersionLayouts
    {
        using classic = ClassicLayout;
        using tbc = TbcLayout;
        using wotlk = WotlkLayout;
        using cata = CataLayout;
        using mop = MopLayout;
    };

    template <>
    struct DbcTraits<Structures::AreaGroupEntry> : DbcVersionLayouts<
            UnsupportedVersion,
            UnsupportedVersion,
            Structures::Raw::AreaGroupEntryWotlkCataMop,
            Structures::Raw::AreaGroupEntryWotlkCataMop,
            Structures::Raw::AreaGroupEntryWotlkCataMop>
    {
        static constexpr const char* filename = "AreaGroup.dbc";
    };

    template <>
    struct DbcTraits<Structures::AreaTableEntry> : DbcVersionLayouts<
            Structures::Raw::AreaTableEntryClassic,
            Structures::Raw::AreaTableEntryTbc,
            Structures::Raw::AreaTableEntryWotlk,
            Structures::Raw::AreaTableEntryCata,
            Structures::Raw::AreaTableEntryMop>
    {
        static constexpr const char* filename = "AreaTable.dbc";
    };

    template <>
    struct DbcTraits<Structures::BarberShopStyleEntry> : DbcVersionLayouts<
            UnsupportedVersion, // Classic
            UnsupportedVersion, // TBC
            Structures::Raw::BarberShopStyleEntryWotlkCataMop,
            Structures::Raw::BarberShopStyleEntryWotlkCataMop,
            Structures::Raw::BarberShopStyleEntryWotlkCataMop>
    {
        static constexpr const char* filename = "BarberShopStyle.dbc";
    };

    template <>
    struct DbcTraits<Structures::CharTitlesEntry> : DbcVersionLayouts<
            UnsupportedVersion, // Classic
            Structures::Raw::CharTitlesEntryTbcWotlk, // TBC
            Structures::Raw::CharTitlesEntryTbcWotlk, // WotLK
            Structures::Raw::CharTitlesEntryCataMop, // Cata
            Structures::Raw::CharTitlesEntryCataMop> // MoP
    {
        static constexpr const char* filename = "CharTitles.dbc";
    };

    template <>
    struct DbcTraits<Structures::ChrClassesEntry> : DbcVersionLayouts<
            Structures::Raw::ChrClassesEntryClassic,
            Structures::Raw::ChrClassesEntryTbc,
            Structures::Raw::ChrClassesEntryWotlk,
            Structures::Raw::ChrClassesEntryCata,
            Structures::Raw::ChrClassesEntryMop>
    {
        static constexpr const char* filename = "ChrClasses.dbc";
    };

    template <>
    struct DbcTraits<Structures::ChrRacesEntry> : DbcVersionLayouts<
            Structures::Raw::ChrRacesEntryClassic,
            Structures::Raw::ChrRacesEntryTbc,
            Structures::Raw::ChrRacesEntryWotlk,
            Structures::Raw::ChrRacesEntryCata,
            Structures::Raw::ChrRacesEntryMop>
    {
        static constexpr const char* filename = "ChrRaces.dbc";
    };

    template <>
    struct DbcTraits<Structures::FactionEntry> : DbcVersionLayouts<
            Structures::Raw::FactionEntryClassic,
            Structures::Raw::FactionEntryTbc,
            Structures::Raw::FactionEntryWotlk,
            Structures::Raw::FactionEntryCata,
            Structures::Raw::FactionEntryMop>
    {
        static constexpr const char* filename = "Faction.dbc";
    };

    template <>
    struct DbcTraits<Structures::FactionTemplateEntry> : DbcVersionLayouts<
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll,
            Structures::Raw::FactionTemplateEntryAll>
    {
        static constexpr const char* filename = "FactionTemplate.dbc";
    };
}
