/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

namespace MoltenCore
{
    enum CreatureEntry
    {
        CN_CORERAGER = 11672,
        CN_SULFURON_HARBRINGER = 12098,
        CN_RAGNAROS = 11502,
        CN_ANCIENTCOREHOUND = 11673,
        CN_GARR = 12057,
        CN_FIRESWORN = 12099
    };

    enum CreatureSpells
    {
        MANGLE = 19820,
        DEMORALIZING_SHOUT = 19778,
        INSPIRE = 19779,
        FLAME_SPEAR = 19781,
        ELEMENTAL_FIRE = 20563,         // 1 target
        MAGMA_BLAST = 20565,            // various targets on not attacked. -> sound -> 8048 ?
        WRATH_OF_RAGNAROS = 20566,      // Fly bitches fly! quote -> "Taste the Flames of Sulfuron!" -> sound 8047
        HAMMER_OF_RAGNAROS = 19780,     // quote -> "By fire be purged!" -> sound 8046
        MELT_WEAPON = 21387,            // 1 target
        SUMMON_SONS_OF_FLAMES = 21108,
        ANCIENTCOREHOUND_GROUND_STOMP = 19364,
        ANCIENTCOREHOUND_ANCIENT_DREAD = 19365,
        ANCIENTCOREHOUND_ANCIENT_DESPAIR = 19369,
        ANCIENTCOREHOUND_CAUTERIZING_FLAMES = 19366,
        ANCIENTCOREHOUND_WITHERING_HEAT = 19367,
        ANCIENTCOREHOUND_ANCIENT_HYSTERIA = 19372,
        FIRESWORN_SEPARATION_ANXIETY = 23492,
        SHAZZRAH_ARCANE_EXPLOSION = 19712,
        SHAZZRAH_BLINK = 29883
    };
}
