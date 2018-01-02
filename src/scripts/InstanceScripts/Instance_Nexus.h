/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum NexusEncounterList
{
    NEXUS_ANOMALUS = 0,
    NEXUS_TELESTRA = 1,
    NEXUS_ORMOROK = 2,
    NEXUS_KERISTRASZA = 3,

    NEXUS_END = 4
};

enum CreatureEntry
{
    // Anomalus
    CN_ANOMALUS             = 26763,
    CN_CHAOTIC_RIFT         = 26918,
    CN_CRAZED_MANA_WRAITH   = 26746,

    // Grand Magus Telestra
    CN_TELESTRA             = 26731,
    CN_TELESTRA_FROST       = 26930,
    CN_TELESTRA_FIRE        = 26928,
    CN_TELESTRA_ARCANE      = 26929,

    // Ormorok the Tree-Shaper
    CN_ORMOROK              = 26794,
    CN_CRYSTAL_SPIKE        = 27099,

    // Keristrasza
    CN_KERISTRASZA          = 26723,

    // Hall of Statis
    CN_ALLIANCE_COMMANDER   = 27949,
    H_CN_ALLIANCE_COMMANDER = 26796,
    CN_HORDE_COMMANDER      = 27947,
    H_CN_HORDE_COMMANDER    = 26798,
};

enum CreatureSpells
{
    // Anomalus
    SPARK                   = 47751,
    SPARK_HC                = 57062,    //Heroic
    CHAOTIC_ENERGY_BURST    = 47688,
    CHAOTIC_RIFT_AURA       = 47687,
    SUMMON_MANA_WRAITH      = 47692,

    // Grand Magus Telestra
    GRAVITY_WELL            = 47756,

    // Normal mode spells
    ICE_NOVA            = 47772,
    FIREBOMB            = 47773,
    
    // Heroic mode spells
    ICE_NOVA_HC         = 56935,
    FIREBOMB_HC         = 56934,

    // Arcane spells
    CRITTER             = 47731,
    TIME_STOP           = 47736,

    // Fire
    FIRE_BLAST          = 47721,
    FIRE_BLAST_HC       = 56939,
    SCORCH              = 47723,
    SCORCH_HC           = 56938,

    // Frost
    BLIZZARD            = 47727,
    BLIZZARD_HC         = 56936,
    ICE_BARB            = 47729,
    ICE_BARB_HC         = 56937,

    // Ormorok the Tree-Shaper
    SPELL_REFLECTION        = 47981,
    FRENZY                  = 48017,

    // normal mode spells
    TRAMPLE                     = 48016,
    CRYSTAL_SPIKES              = 47958,

    // heroic mode spells
    TRAMPLE_H                   = 57066,
    CRYSTAL_SPIKES_H            = 57082,

    // Crystal Spike spells
    SPELL_CRYSTAL_SPIKE_VISUAL  = 50442,
    SPELL_CRYSTAL_SPIKE         = 47944,
    SPELL_CRYSTAL_SPIKE_H       = 57067,

    // Keristrasza
    TAIL_SWEEP              = 50155,
    INTENSE_COLD            = 48094,
    CRYSTAL_CHAINS          = 50997,
    CRYSTALLIZE             = 48179,
    ENRAGE                  = 8599,
    CRYSTALFIRE_BREATH      = 48096,
    CRYSTALFIRE_BREATH_HC   = 57091
};

enum GameObject_Entry
{
    ANOMALUS_CS             = 188527,
    TELESTRA_CS             = 188526,
    ORMOROK_CS              = 188528
};

struct NexusSpawns
{
    uint32 entry;
    float x;
    float y;
    float z;
    float o;
    uint32 faction;
};

const NexusSpawns TrashAllySpawns[18] = {
    // Hall of Stasis
    { 26805, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26802, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26800, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26805, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26802, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26800, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26805, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26802, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26800, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26805, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26802, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26800, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26805, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26802, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26800, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26805, 462.00f, 146.856f, -35.01f, -1.25f, 14 },
    { 26802, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26800, 460.63f, 164.358f, -35.01f, -0.14f, 14 }
};

const NexusSpawns TrashHordeSpawns[18] = {
    // Hall of Stasis
    { 26803, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26801, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26799, 388.61f, 149.039f, -35.01f, 1.55f, 14 },
    { 26803, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26801, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26799, 389.67f, 168.528f, -35.01f, -2.2f, 14 },
    { 26803, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26801, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26799, 402.82f, 184.085f, -35.01f, -1.64f, 14 },
    { 26803, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26801, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26799, 442.156f, 175.61f, -35.01f, 2.1f, 14 },
    { 26803, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26801, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26799, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26803, 462.00f, 146.856f, -35.01f, -1.25f, 14 },
    { 26801, 460.63f, 164.358f, -35.01f, -0.14f, 14 },
    { 26799, 460.63f, 164.358f, -35.01f, -0.14f, 14 }
};


enum CreatureSay
{

};
