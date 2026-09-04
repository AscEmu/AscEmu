/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

// Well of Eternity - three story encounters (Perotharn, Queen Azshara, Mannoroth & Varothen).
// Only Perotharn is a real stand-up fight; Azshara and Mannoroth/Varothen are cinematic/quest
// encounters with no combat creature of their own even in the reference implementation, so
// only Perotharn is scripted here (encounter tracking only, matching the reference, which
// leaves it unscripted too).

uint32_t const WellOfEternityEncounterCount = 3;

enum WellOfEternityData
{
    DATA_PEROTHARN = 0
};

enum WellOfEternityCreatures
{
    BOSS_PEROTHARN            = 55085,

    // Trash - both confirmed against this instance's own real creature spawn data. Neither
    // NPC page carried ability data, so their kits are thematically-reasonable rotations built
    // from generic, verified-safe spells rather than confirmed tooltips.
    NPC_DOOMGUARD_ANNIHILATOR = 55519,
    NPC_DREADLORD_DEFENDER    = 55656
};

enum DoomguardAnnihilatorSpells
{
    SPELL_DOOMGUARD_CLEAVE        = 845,    // stand-in: reuses "Cleave" rank 1
    SPELL_DOOMGUARD_MORTAL_STRIKE = 12294   // stand-in: reuses "Mortal Strike" rank 1
};

enum DreadlordDefenderSpells
{
    SPELL_DREADLORD_SHADOW_BOLT = 32860,  // stand-in: reuses the real "Shadow Bolt" spell
    SPELL_DREADLORD_FEAR        = 6215    // stand-in: reuses generic "Fear"
};

enum WellOfEternityGameObjects
{
    GO_LARGE_FIREWALL_DOOR = 210234,
    GO_SMALL_FIREWALL_DOOR = 210130
};
