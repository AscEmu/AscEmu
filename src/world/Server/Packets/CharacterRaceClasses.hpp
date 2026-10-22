/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::Packets
{
    // The races a character can be created with on the 12.x clients and the classes of each race, zero ends a list.
    // The lists follow the race and class combinations of the client; the auth response and the character list send
    // them so the client offers the races and classes.
    struct RaceClasses
    {
        int8_t race;
        int8_t classes[11];
    };

    inline constexpr RaceClasses midnightRaceClasses[] = {
        { 1, { 1, 2, 3, 4, 5, 6, 8, 9, 10, 0, 0 } },       // human
        { 2, { 1, 3, 4, 6, 7, 8, 9, 10, 0, 0, 0 } },       // orc
        { 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0 } },       // dwarf
        { 4, { 1, 3, 4, 5, 6, 8, 10, 11, 12, 0, 0 } },     // night elf
        { 5, { 1, 3, 4, 5, 6, 8, 9, 10, 0, 0, 0 } },       // undead
        { 6, { 1, 2, 3, 5, 6, 7, 10, 11, 0, 0, 0 } },      // tauren
        { 7, { 1, 3, 4, 5, 6, 8, 9, 10, 0, 0, 0 } },       // gnome
        { 8, { 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0 } },      // troll
        { 9, { 1, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0 } },        // goblin
        { 10, { 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 0 } },     // blood elf
        { 11, { 1, 2, 3, 5, 6, 7, 8, 10, 0, 0, 0 } },      // draenei
        { 22, { 1, 3, 4, 5, 6, 8, 9, 11, 0, 0, 0 } },      // worgen
        { 24, { 1, 3, 4, 5, 6, 7, 8, 10, 0, 0, 0 } },      // pandaren
        { 25, { 1, 3, 4, 5, 6, 7, 8, 10, 0, 0, 0 } },      // pandaren alliance
        { 26, { 1, 3, 4, 5, 6, 7, 8, 10, 0, 0, 0 } },      // pandaren horde
        { 27, { 1, 3, 4, 5, 6, 8, 9, 10, 0, 0, 0 } },      // nightborne
        { 28, { 1, 3, 6, 7, 10, 11, 0, 0, 0, 0, 0 } },     // highmountain tauren
        { 29, { 1, 3, 4, 5, 6, 8, 9, 10, 0, 0, 0 } },      // void elf
        { 30, { 1, 2, 3, 5, 6, 8, 0, 0, 0, 0, 0 } },       // lightforged draenei
        { 31, { 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 0 } },     // zandalari troll
        { 32, { 1, 3, 4, 5, 6, 7, 8, 10, 11, 0, 0 } },     // kul tiran
        { 34, { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0 } },      // dark iron dwarf
        { 35, { 1, 3, 4, 5, 6, 7, 8, 9, 10, 0, 0 } },      // vulpera
        { 36, { 1, 3, 4, 5, 6, 7, 8, 10, 0, 0, 0 } },      // mag'har orc
        { 37, { 1, 3, 4, 5, 6, 8, 9, 10, 0, 0, 0 } },      // mechagnome
        { 52, { 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },      // dracthyr alliance
        { 70, { 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },      // dracthyr horde
        { 84, { 1, 2, 3, 4, 5, 7, 8, 9, 10, 0, 0 } },      // earthen alliance
        { 85, { 1, 2, 3, 4, 5, 7, 8, 9, 10, 0, 0 } },      // earthen horde
        { 86, { 1, 3, 4, 5, 7, 8, 9, 10, 11, 0, 0 } },     // haranir alliance
        { 91, { 1, 3, 4, 5, 7, 8, 9, 10, 11, 0, 0 } },     // haranir horde
    };

    inline constexpr uint32_t classCountOf(RaceClasses const& entry)
    {
        uint32_t count = 0;
        while (count < 11 && entry.classes[count] != 0)
            ++count;
        return count;
    }

    // the expansion a class needs
    inline constexpr uint8_t expansionOfClass(int8_t playerClass)
    {
        switch (playerClass)
        {
            case 6: return 2;       // death knight
            case 10: return 4;      // monk
            case 12: return 6;      // demon hunter
            case 13: return 9;      // evoker
            default: return 0;
        }
    }
}
