/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org>
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


#ifndef _INSTANCE_BLACK_TEMPLE_H
#define _INSTANCE_BLACK_TEMPLE_H

enum CreatureEntry
{
    CN_SUPREMUS                     = 22898,

    //Beasts
    CN_DRAGON_TURTLE                = 22885,
    CN_LEVIATHAN                    = 22884,
    CN_MUTANT_WAR_HOUND             = 23232,
    CN_SHADOWMOON_RIDING_HOUND      = 23083,
    //Demons
    CN_SISTER_OF_PLEASURE           = 22964,
    CN_SISTER_OF_PAIN               = 22956,
    CN_PRIESTESS_OF_DEMENTIA        = 22957,
    CN_PRIESTESS_OF_DELIGHT         = 22962,
    CN_ILLIDARI_NIGHTLORD           = 22855,
    CN_ILLIDARI_HEARTSEEKER         = 23339,
    CN_ILLIDARI_FEARBRINGER         = 22954,
    CN_ILLIDARI_DEFILER             = 22853,
    CN_ILLIDARI_CENTURION           = 23337,
    CN_ILLIDARI_BONESLICER          = 22869,
    //Humanoids
    CN_ASHTONGUE_BATTLELORD         = 22844,
    CN_ASHTONGUE_DEFENDER           = 23216,
    CN_ASHTONGUE_ELEMENTALIST       = 23523,
    CN_ASHTONGUE_MYSTIC             = 22845,
    CN_ASHTONGUE_PRIMALIST          = 22847,
    CN_ASHTONGUE_ROGUE              = 23318,
    CN_ASHTONGUE_SPIRITBINDER       = 23524,
    CN_ASHTONGUE_STALKER            = 23374,
    CN_ASHTONGUE_STORMCALLER        = 22846,
    CN_BONECHEWER_BEHEMOTH          = 23196,
    CN_BONECHEWER_BLADE_FURY        = 23235,
    CN_BONECHEWER_BLOOD_PROPHET     = 23237,
    CN_BONECHEWER_BRAWLER           = 23222,
    CN_BONECHEWER_COMBATANT         = 23239,
    CN_BONECHEWER_SHIELD_DISCIPLE   = 23236,
    CN_BONECHEWER_SPECTATOR         = 23223,
    CN_BONECHEWER_TASKMASTER        = 23028,
    CN_BONECHEWER_WORKER            = 22963,
    CN_CHARMING_COURTESAN           = 22955,
    CN_COILSKAR_GENERAL             = 22873

};

enum CreatureSpells
{
    //Beasts
    DRAGON_TRUTLE_SHELL_SHIELD                          = 40087,
    DRAGON_TURTLE_WATER_SPIT                            = 40086,
    LEVIATHAN_DEBILITATING_SPRAY                        = 40079,
    LEVIATHAN_POISON_SPIT                               = 40078,
    LEVIATHAN_TAIL_SWEEP                                = 40077,
    MUTANT_WAR_HOUND_CLOUD_OF_DISEASE                   = 41193,
    SHADOWMOON_RIDING_HOUND_CARNIVOROUS_BITE            = 41092,
    SHADOWMOON_RIDING_HOUND_CHARGE                      = 25821,
    SHADOWMOON_RIDING_HOUND_ENRAGE                      = 8599,
    //Demons
    SISTER_OF_PLEASURE_GREATER_HEAL                     = 41378,
    SISTER_OF_PLEASURE_HOLY_NOVA                        = 41380,
    SISTER_OF_PLEASURE_SHARED_BONDS                     = 41362,
    SISTER_OF_PLEASURE_SHELL_OF_LIFE                    = 41381,
    SISTER_OF_PAIN_LASH_OF_PAIN                         = 41353,
    SISTER_OF_PAIN_PAINFUL_RAGE                         = 41369,
    SISTER_OF_PAIN_SHADOW_WORD_PAIN                     = 41355,
    SISTER_OF_PAIN_SHARED_BONDS                         = 41362,
    SISTER_OF_PAIN_SHELL_OF_PAIN                        = 41371,
    PRIESTESS_OF_DEMENTIA_CONFUSION                     = 41397,
    PRIESTESS_OF_DEMENTIA_DEMENTIA                      = 41404,
    PRIESTESS_OF_DELIGHT_CURSE_OF_VITALITY              = 41351,
    ILLIDARI_NIGHTLORD_SUMMON_SHADOWFIENDS              = 39649,
    ILLIDARI_NIGHTLORD_SHADOW_INFERNO                   = 39645,
    ILLIDARI_NIGHTLORD_FEAR                             = 41150,
    ILLIDARI_NIGHTLORD_CURSE_OF_MENDING                 = 39647,
    ILLIDARI_HEARTSEEKER_CURSE_OF_THE_BLEAKHEART        = 41170,
    ILLIDARI_HEARTSEEKER_RAPID_SHOT                     = 41173,
    ILLIDARI_HEARTSEEKER_SHOOT                          = 41169,
    ILLIDARI_HEARTSEEKER_SKELETON_SHOT                  = 41171,
    ILLIDARI_FEARBRINGER_ILLIDARI_FLAMES                = 40938,
    ILLIDARI_FEARBRINGER_RAIN_OF_CHAOS                  = 40946,
    ILLIDARI_FEARBRINGER_WAR_STOMP                      = 40936,
    ILLIDARI_DEFILER_BANISH                             = 39674,
    ILLIDARI_DEFILER_CURSE_OF_AGONY                     = 39672,
    ILLIDARI_DEFILER_FEL_IMMOLATE                       = 39670,
    ILLIDARI_DEFILER_RAIN_OF_CHAOS                      = 39671,
    ILLIDARI_CENTURION_CLEAVE                           = 15284,
    ILLIDARI_CENTURION_SONIC_STRIKE                     = 41168,
    ILLIDARI_BONESLICER_CLOAK_OF_SHADOWS                = 39666,
    ILLIDARI_BONESLICER_GOUGE                           = 24698,
    ILLIDARI_BONESLICER_SHADOWSTEP                      = 41176,

    //Humanoids
    ASHTONGUE_BATTLELORD_CLEAVE                         = 15284,
    ASHTONGUE_BATTLELORD_CONCUSSION_BLOW                = 32588,
    ASHTONGUE_BATTLELORD_CONCUSSIVE_THROW               = 41182,
    ASHTONGUE_BATTLELORD_ENRAGE                         = 34970,
    ASHTONGUE_DEFENDER_DEBILITATING_STRIKE              = 41178,
    ASHTONGUE_DEFENDER_SHIELD_BASH                      = 41180,
    ASHTONGUE_ELEMENTALIST_LIGHTNING_BOLT               = 42024,
    ASHTONGUE_ELEMENTALIST_RAID_OF_FIRE                 = 42023,
    ASHTONGUE_MYSTIC_BLOODLUST                          = 41185,
    ASHTONGUE_MYSTIC_CHAIN_HEAL                         = 41114,
    ASHTONGUE_MYSTIC_CYCLONE_TOTEM                      = 39589,
    ASHTONGUE_MYSTIC_FLAME_SHOCK                        = 41115,
    ASHTONGUE_MYSTIC_FROST_SHOCK                        = 41116,
    ASHTONGUE_MYSTIC_SEARING_TOTEM                      = 39588,
    ASHTONGUE_MYSTIC_SUMMON_WINDFURY_TOTEM              = 39586,
    ASHTONGUE_PRIMALIST_MULTISHOT                       = 41187,
    ASHTONGUE_PRIMALIST_SHOOT                           = 41188,
    ASHTONGUE_PRIMALIST_SWEEPING_WING_CLIP              = 39584,
    ASHTONGUE_PRIMALIST_WYVERN_STRING                   = 41186,
    ASHTONGUE_ROGUE_DEBILITATING_POISON                 = 41978,
    ASHTONGUE_ROGUE_EVISCERATE                          = 41177,
    ASHTONGUE_SPIRITBINDER_CHAIN_HEAL                   = 42027,
    ASHTONGUE_SPIRITBINDER_SPIRIT_HEAL                  = 42317,
    ASHTONGUE_SPIRITBINDER_SPIRIT_HEAL2                 = 42318,
    ASHTONGUE_SPIRITBINDER_SPIRIT_MEND                  = 42025,
    ASHTONGUE_STALKER_BLIND                             = 34654,
    ASHTONGUE_STALKER_INSTANT_POISON                    = 41189,
    ASHTONGUE_STALKER_MINDNUMBING_POISON                = 41190,
    ASHTONGUE_STALKER_STEATH                            = 34189,
    ASHTONGUE_STORMCALLER_CHAIN_LIGHTNING               = 41183,
    ASHTONGUE_STORMCALLER_LIGHTNING_BOLT                = 41184,
    ASHTONGUE_STORMCALLER_LIGHTNING_SHIELD              = 41151,
    BONECHEWER_BEHEMOTH_BEHEMOTH_CHARGE                 = 41272,
    BONECHEWER_BEHEMOTH_ENRAGE                          = 8269,
    BONECHEWER_BEHEMOTH_FEL_STOMP                       = 41274,
    BONECHEWER_BEHEMOTH_FIERY_COMET                     = 41277,
    BONECHEWER_BEHEMOTH_METEOR                          = 41276,
    BONECHEWER_BLADE_FURY_WHIRLWIND                     = 41194,
    BONECHEWER_BLADE_FURY_WHIRLWIND2                    = 41195,
    BONECHEWER_BLOOD_PROPHET_BLOOD_DRAIN                = 41238,
    BONECHEWER_BLOOD_PROPHET_BLOODBOLT                  = 41229,
    BONECHEWER_BLOOD_PROPHET_ENRAGE                     = 8269,
    BONECHEWER_BLOOD_PROPHET_PROPHECY_OF_BLOOD          = 41230,
    BONECHEWER_BLOOD_PROPHET_PROPHECY_OF_BLOOD2         = 41231,
    BONECHEWER_BRAWLER_FRENZY                           = 41254,
    BONECHEWER_COMBATANT_FRENZY                         = 8269,
    BONECHEWER_SHIELD_DISCIPLE_INTERVENE                = 41198,
    BONECHEWER_SHIELD_DISCIPLE_SHIELD_BASH              = 41197,
    BONECHEWER_SHIELD_DISCIPLE_SHIELD_WALL              = 41196,
    BONECHEWER_SHIELD_DISCIPLE_THROW_SHIELD             = 41213,
    BONECHEWER_SPECTATOR_CHARGE                         = 36140,
    BONECHEWER_SPECTATOR_CLEAVE                         = 40505,
    BONECHEWER_SPECTATOR_MORTAL_WOUND                   = 25646,
    BONECHEWER_SPECTATOR_STRIKE                         = 13446,
    BONECHEWER_SPECTATOR_SUNDER_ARMOR                   = 13444,
    BONECHEWER_TASKMASTER_DISGRUNTLED                   = 40851,
    BONECHEWER_TASKMASTER_FURY                          = 40845,
    BONECHEWER_WORKER_THROW_PICK                        = 40844,
    CHARMING_COURTESAN_INFATUATION                      = 41345,
    CHARMING_COURTESAN_POISONOUS_THROW                  = 41346,
    COILSKAR_GENERAL_BOOMING_VOICE                      = 40080,
    COILSKAR_GENERAL_FREE_FRIEND                        = 40081
};

#endif  // _INSTANCE_BLACK_TEMPLE_H
