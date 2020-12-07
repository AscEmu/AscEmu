/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

enum CreatureEntry
{
    // Arcane ServantAI
    CN_ARCANE_SERVANT = 20478,

    // Bloodwarder CenturionAI
    CN_BLOODWARDER_CENTURION = 19510,

    // Bloodwarder PhysicianAI
    CN_BLOODWARDER_PHYSICIAN = 20990,

    // Bloodwarder SlayerAI
    CN_BLOODWARDER_SLAYER = 19167,

    // Mechanar CrusherAI
    CN_MECHANAR_CRUSHER = 19231,

    // Mechanar DrillerAI
    CN_MECHANAR_DRILLER = 19712,

    // Mechanar TinkererAI
    CN_MECHANAR_TINKERER = 19716,   // recheck and add some stuff

    // Mechanar WreckerAI
    CN_MECHANAR_WRECKER = 19713,

    // Raging FlamesAI
    CN_RAGING_FLAMES = 20481,

    // Sunseeker AstromageAI
    CN_SUNSEEKER_ASTROMAGE = 19168,

    // Sunseeker EngineerAI
    CN_SUNSEEKER_ENGINEER = 20988,

    // Sunseeker NetherbinderAI
    CN_SUNSEEKER_NETHERBINDER = 20059,

    // Tempest-Forge DestroyerAI
    CN_TEMPEST_FORGE_DESTROYER = 19735,

    // Tempest-Forge PatrollerAI
    CN_TEMPEST_FORGE_PATROLLER = 19166,

    // Boss AIs
    // Gatewatcher Gyro-Kill AI
    CN_GATEWATCHER_GYRO_KILL = 19218,

    // Gatewatcher Iron-Hand AI
    CN_GATEWATCHER_IRON_HAND = 19710,

    // Mechano-Lord Capacitus AI
    CN_MECHANO_LORD_CAPACITUS = 19219,

    // Nethermancer Sepethrea AI
    CN_NETHERMANCER_SEPETHREA = 19221,

    // Pathaleon the Calculator AI
    CN_PATHALEON_THE_CALCULATOR = 19220

};

enum CreatureSpells
{
    // Arcane ServantAI
    SP_ARCANE_VOLLEY                        = 35255,
    SP_ARCANE_EXPLOSION                     = 22271,
    //SPOTLIGHT = 35259 // SSS

    // Bloodwarder CenturionAI
    SP_CENTURION_SHIELD_BASH                = 35178,
    SP_CENTURION_UNSTABLE_AFFLICTION        = 35183,
    SP_CENTURION_MELT_ARMOR                 = 35185,
    SP_CENTURION_CHILLING_TOUCH             = 12531,
    SP_CENTURION_ETHEREAL_TELEPORT          = 34427,    // SSS
   //SEED_OF_CORRUPTION = 37826 // SSS

   // Bloodwarder PhysicianAI
    SP_PHYSICIAN_HOLY_SHOCK                 = 36340,
    SP_PHYSICIAN_ANESTHETIC                 = 36333,
    SP_PHYSICIAN_BANDAGE                    = 36348,    // DBC: 36348; mine choice 38919
    SP_PHYSICIAN_ETHEREAL_TELEPORT_PHYS     = 34427,    // SSS
    //SEED_OF_CORRUPTION = 37826 // SSS

    // Bloodwarder SlayerAI
    SP_SLAYER_WHIRLWIND                     = 13736,    // DBC: 13736, 15589;
    SP_SLAYER_SOLAR_STRIKE                  = 35189,
    SP_SLAYER_MELT_ARMOR                    = 35231,
    SP_SLAYER_CHILLING_TOUCH                = 12531,
    SP_SLAYER_MORTAL_STRIKE                 = 15708,

    // Mechanar CrusherAI
    SP_MECH_CRUSHER_DISARM                  = 31955,    // is that all?    and not sure to this one :)

    // Mechanar DrillerAI
    SP_MECH_DRILLER_GLOB_OF_MACHINE_FLUID   = 35056,
    SP_MECH_DRILLER_ARMOR                   = 35047,
    SP_MECH_DRILLER_CRIPPLING_POISON        = 30981,    // not sure if crippling poison and glob should be here (I mean if both should be)
    SP_MECH_DRILLER_POUND                   = 35049,

    // Mechanar TinkererAI
    SP_MECH_TINKERER_NETHERBOMB             = 35057,
    SP_MECH_TINKERER_PRAYER_OF_MENDING      = 35092,    // DBC: 35092, 33110, 35094    // SSS
    SP_MECH_TINKERER_MANIACAL_CHARGE        = 35062,    // those two are connected with each other and Idk if those shouldn't be scripted separately
    SP_MECH_TINKERER_NETHER_EXPLOSION       = 35058,    // additional coding to add health percent check, chance to cast and killing caster =/

    // Mechanar WreckerAI
    SP_MECH_WRECKER_POUND                   = 35049,
    SP_MECH_WRECKER_GLOB_OF_MACHINE_FLUID   = 35056,
    SP_MECH_WRECKER_PRAYER_OF_MENDING       = 33280,    // DBC: 33280, 33110, 35093; SSS (hmm... not sure if it uses it =S) should also affect allies, not enemies

    // Raging FlamesAI
    SP_RAGING_FLAMES                        = 35278,    // DBC: 35278;
    SP_RAGING_FLAMES_INFERNO                = 35268,    // DBC: 35268, 35283;
    // I think skills needs additional core support

    // Sunseeker AstromageAI
    SP_SS_ASTROMAGE_SCORCH                  = 17195,
    SP_SS_ASTROMAGE_SOLARBURN               = 35267,
    SP_SS_ASTROMAGE_FIRE_SHIELD             = 35265,    //  DBC: 35266, 35265
    SP_SS_ASTROMAGE_ETHEREAL_TELEPORT       = 34427,

    // Sunseeker EngineerAI
    SP_SS_ENGINEER_SUPER_SHRINK_RAY         = 36341,
    SP_SS_ENGINEER_DEATH_RAY                = 36345,
    SP_SS_ENGINEER_GROWTH_RAY               = 36346,    // doesn't work, even when used by player =(
    SP_SS_ENGINEER_ETHEREAL_TELEPORT        = 34427,

    // Sunseeker NetherbinderAI
    SP_SS_NETHERBINDER_ARCANE_NOVA          = 35261,
    SP_SS_NETHERBINDER_STARFIRE             = 35243,
    SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM1 = 35251,    // DBC: 35251, 35260; Guardian, so won't work now
    SP_SS_NETHERBINDER_SUMMON_ARCANE_GOLEM2 = 35260,
    SP_SS_NETHERBINDER_DISPEL_MAGIC         = 23859,    // no idea about this one
    //SEED_OF_CORRUPTION = 37826

    // Tempest-Forge DestroyerAI
    SP_TEMPEST_DESTROYER_KNOCKDOWN          = 35783,
    SP_TEMPEST_DESTROYER_CHARGED_FIST       = 36582,    // DBC: 36582, 36583;    doesn't work anyway at all (lack of core support?)

    // Tempest-Forge PatrollerAI
    SP_TEMPEST_PAT_CHARGED_ARCANE_MISSILE   = 35012,
    SP_TEMPEST_PAT_KNOCKDOWN                = 35011,

    // Boss AIs
    // Gatewatcher Gyro-Kill AI
    SP_GW_GYRO_KILL_SAW_BLADE               = 35318,
    SP_GW_GYRO_KILL_SHADOW_POWER            = 35322,
    SP_GW_GYRO_KILL_STEAM_OF_MACHINE_FLUID  = 35311,

    // Gatewatcher Iron-Hand AI
    SP_GW_IRON_HAND_JACK_HAMMER             = 35327,    // DBC: 35327, 35330
    SP_GW_IRON_HAND_HAMMER_PUNCH            = 35326,
    SP_GW_IRON_HAND_STREAM_OF_MACHINE_FLUID = 35311,
    SP_GW_IRON_HAND_SHADOW_POWER            = 35322,

    // Mechano-Lord Capacitus AI
    SP_MECH_LORD_HEAD_CRACK                 = 35161,
    SP_MECH_LORD_REFLECTIVE_DAMAGE_SHIELD   = 35159,
    SP_MECH_LORD_REFLECTIVE_MAGIC_SHIELD    = 35158,
    SP_MECH_LORD_SEED_OF_CORRUPTION         = 37826,    // SSS (server side script) (is it really used?)
    /*NETHER_CHARGE 34303
    NETHER_CHARGE_PASSIVE 35150 // SSS
    NETHER_CHARGE_PULSE 35151    // SSS
    NETHER_CHARGE_TIMER 37670
    NETHER_DETONATION 35152        // Spell from Timer
    // Note: All for bombs :O*/

    // Nethermancer Sepethrea AI
    SP_NETH_SEPETHREA_SUMMON_RAGIN_FLAMES   = 35275,    // must add despawning after death!
    SP_NETH_SEPETHREA_FROST_ATTACK          = 35263,
    SP_NETH_SEPETHREA_ARCANE_BLAST          = 35314,
    SP_NETH_SEPETHREA_DRAGONS_BREATH        = 35250,
    //KNOCKBACK 37317    // not sure to this one!

    // Pathaleon the Calculator AI
    SP_PATHALEON_MANA_TRAP                  = 36021,    // I am not sure to any of those ids =(
    SP_PATHALEON_DOMINATION                 = 36866,
    SP_PATHALEON_SILENCE                    = 38491,
    SP_PATHALEON_SUMMON_NETHER_WRAITH1      = 35285,    // not the best way, but blizzlike :) (but they don't work for now =()
    SP_PATHALEON_SUMMON_NETHER_WRAITH2      = 35286,
    SP_PATHALEON_SUMMON_NETHER_WRAITH3      = 35287,
    SP_PATHALEON_SUMMON_NETHER_WRAITH4      = 35288,

};

enum CreatureSay
{
    // Gatewatcher Gyro-Kill AI
    SAY_GW_GYRO_KILL_01      = 6153,    // An unforeseen... contingency.
    SAY_GW_GYRO_KILL_02      = 6155,    // Measure twice; cut once.
    SAY_GW_GYRO_KILL_03      = 6156,    // If my division is correct... you should be quite dead.
    SAY_GW_GYRO_KILL_04      = 6157,    // Yes, the only logical outcome.
    SAY_GW_GYRO_KILL_05      = 6158,    // I predict a painful death.
    SAY_GW_GYRO_KILL_06      = 6159,    // Your strategy was flawed.

    // Gatewatcher Iron-Hand AI
    SAY_GW_IRON_HAND_01     = 5098,    // You have approximately five seconds to live.
    SAY_GW_IRON_HAND_02     = 5099,    // With the precise angle and velocity...
    SAY_GW_IRON_HAND_03     = 5100,    // Low-tech, yet quite effective.
    SAY_GW_IRON_HAND_04     = 5101,    // A foregone conclusion.
    SAY_GW_IRON_HAND_05     = 5102,    // The processing will continue as scheduled!
    SAY_GW_IRON_HAND_06     = 5103,    // My calculations did not...

    BROADCAST_IRON_HAND_01     = 5104,  /// \todo unused %s raises his hammer menacingly...

    // Mechano-Lord Capacitus AI
    SAY_MECH_LORD_01        = 5783,    // Bully!
    SAY_MECH_LORD_02        = 5804,    // Damn, I'm good!
    SAY_MECH_LORD_03        = 5805,    // Can't say I didn't warn you....
    SAY_MECH_LORD_04        = 6151,    /// \todo currently not from db Go ahead, gimme your best shot.  I can take it!
    SAY_MECH_LORD_05        = 6152,    /// \todo currently not from db Think you can hurt me, huh?  Think I'm afraid a' you?
    SAY_MECH_LORD_06        = 6154,    // You should split while you can.

    // Nethermancer Sepethrea AI
    SAY_NETH_SEPETHREA_01   = 5105,    // Don't value your life very much, do you?
    SAY_NETH_SEPETHREA_02   = 5106,    // I am not alone.
    SAY_NETH_SEPETHREA_03   = 5107,    // Think you can take the heat?
    SAY_NETH_SEPETHREA_04   = 5108,    // Anar'endal dracon!
    SAY_NETH_SEPETHREA_05   = 5109,    // And don't come back!
    SAY_NETH_SEPETHREA_06   = 5110,    // Endala finel endal!
    SAY_NETH_SEPETHREA_07   = 5111,    // Anu... bala belore...alon.

    // Pathaleon the Calculator AI
    SAY_PATHALEON_01        = 5112,    // We are on a strict timetable. You will not interfere!
    SAY_PATHALEON_02        = 5113,    // I'm looking for a team player...
    SAY_PATHALEON_03        = 5114,    // You work for me now!
    SAY_PATHALEON_04        = 5115,    // Time to supplement my work force.
    SAY_PATHALEON_05        = 5116,    /// \todo unused I prefer to be hands-on...
    SAY_PATHALEON_06        = 5117,    // A minor inconvenience.
    SAY_PATHALEON_07        = 5118,    // Looks like you lose.
    SAY_PATHALEON_08        = 5119,    // The project will... continue.

};
