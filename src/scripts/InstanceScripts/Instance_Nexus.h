/*
 * AscScripts for AscEmu Framework
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INSTANCE_NEXUS_H
#define _INSTANCE_NEXUS_H

#define ANOMALUS_CS        188527
#define TELESTRA_CS        188526
#define ORMOROK_CS        188528

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

enum CreatureSay
{

};

#endif // _INSTANCE_NEXUS_H
