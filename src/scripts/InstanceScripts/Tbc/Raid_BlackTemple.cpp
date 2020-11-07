/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Raid_BlackTemple.h"
#include "Objects/Faction.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Illidan Encounter Event Options

#define USE_SHADOW_PRISON // aggroes a lot of mobs/NPCs if they are not friendly to Illidan

// 
//////////////////////////////////////////////////////////////////////////////////////////

enum
{
    CN_NAJENTUS = 22887,

    CRASHING_WAVE = 40100, // Crashing Wave (2313-2687 Nature damage)
    NEEDLE_SPINE = 39835, // Needle Spine (3188-4312 Damage, AoE of 2375-2625 Aura): Fires a needle spine at an enemy target.
    //NEEDLE_SPINE_EXPLOSION = 39968,
    TIDAL_SHIELD = 39872, // Tidal Shield : Impervious to normal attack and spells. Regenerating health. Will deal 8500 frost damage to the raid when broken.
    IMPALING_SPINE = 39837,
    //NAJENTUS_SPINE_GO = 3264, // Campfire so it wont crash Real go is 185584 //Did not find it in any database (including drake's go's), tested with a chair, and it works

    //Abilities - Phase 1
    MOLTEN_FLAME = 40253, // Molten Flame: Blue-flamed fire sent along the ground in a straight line toward random players that deals 3325-3675 fire damage every 1 second while stood on. Ability used every 20 seconds.
    HURTFUL_STRIKE = 40126, // 33813 //Hurtful Strike: A massive melee ability applied to the second highest on his aggro list. If there are no other melee targets in range, it will be performed on the main tank instead.

    //Abilities - Phase 2
    //Gaze: Supremus will target a random player and follow that target around for 10 seconds at 90% of normal movement speed. Switches targets after 10 seconds.
    MOLTEN_PUNCH = 40126, // Molten Punch: Once a targeted player is within a 40 yard range, Supremus will do a knockback ability for 5250 damage.
    VOLCANIC_GAZER = 42055, // Volcanic Geyser: Emoting "the ground begins to crack open", Supremus will summon small volcanoes that spit out blue flames for 4163-4837 fire damage in a 15 yard radius.

    CN_SHADE_OF_AKAMA = 22841,
    CN_ASHTONGUE_CHANNELER = 23421,
    CN_ASHTONGUE_SORCERER = 23215,

    DEBILITATING_STRIKE = 41178,

    //RAIN_OF_FIRE = 42023,

    DEBILITATING_POISON = 41978,

    SPIRIT_MEND = 42025,
    CHAIN_HEAL = 42027,

    CN_GURTOGG_BLOODBOIL = 22948,

    // Phase 1
    BLOODBOIL = 42005,
    ACIDIC_WOUND = 40481,
    DISORIENT = 32752,
    KNOCKBACK = 38576,

    // Phase 2
    FEL_RAGE1 = 40604,
    FEL_RAGE2 = 40594, // 40616 - doesn't work at all
    ACID_GEYSER = 40629,

    // Spells common for both phases
    ARCING_SMASH = 40599,
    FEL_ACID_BREATH = 40595,

    // Essence of Suffering
    CN_ESSENCEOFSUFFERING = 23418,

    EOS_FIXATE = 40893, // 40414
    EOS_FRENZY = 41305,
    EOS_SOUL_DRAIN = 41303,
    EOS_AURA_OF_SUFFERING = 41292,

    // Essence Of Desire

    CN_ESSENCEOFDESIRE = 23419,

    EOD_RUNE_SHIELD = 41431,
    EOD_DEADEN = 41410,
    EOD_SPIRIT_SHOCK = 41426,
    EOD_AURA_OF_DESIRE = 41350,

    // EssenceOfAnger

    CN_ESSENCEOFANGER = 23420,

    EOA_SEETHE = 41520,
    EOA_SOUL_SCREAM = 41545,
    EOA_SPITE = 41377,
    EOA_AURA_OF_ANGER = 41337,

    CN_RELIQUARY_OF_SOULS = 22856,
    ROS_SUMMON_SUFFERING = 41488,
    ROS_SUMMON_DESIRE = 41493,
    ROS_SUMMON_ANGER = 41496,

    CN_MOTHER_SHAHRAZ = 22947,

    SINFUL_BEAM = 40827,
    SINISTER_BEAM = 40859,
    VILE_BEAM = 40860,
    WICKED_BEAM = 40861,

    SABER_LASH = 40810, // Should be 40816 but 40816 is not supported by core
    FATAL_ATTRACTION = 41001, // 40869 - needs more scripting
    MS_ENRAGE = 40743, // 40683
    CN_GATHIOS_THE_SHATTERER = 22949,

    HAMMER_OF_JUSTICE = 41468,
    SEAL_OF_COMMAND = 41469,
    SEAL_OF_BLOOD = 41459,
    CONSECRATION = 41541,
    BLESSING_OF_SPELL_WARDING = 41451,
    BLESSING_OF_PROTECTION = 41450,
    //CHROMATIC_RESISTANCE_AURA = 41453,
    DEVOTION_AURA = 41452,

    CN_VERAS_DARKSHADOW = 22952,

    DEADLY_POISON = 41485,
    ENVENOM = 41487,
    VANISH = 41476,

    CN_HIGH_NETHERMANCER_ZEREVOR = 22950,

    ARCANE_BOLT = 41483,
    BLIZZARD = 41482,
    FLAMESTRIKE = 41481,
    ARCANE_EXPLOSION = 29973,
    DAMPEN_MAGIC = 41478,

    CN_LADY_MALANDE = 22951,

    DIVINE_WRATH = 41472,
    REFLECTIVE_SHIELD = 41475,
    EMPOWERED_SMITE = 41471,
    CIRCLE_OF_HEALING = 41455,

    CN_TERON_GOREFIEND = 22871,

    DOOM_BLOSSOM = 40188, // needs additional creature in DB
    CRUSHING_SHADOWS = 40243,
    INCINERATE = 40239,
    SHADOW_OF_DEATH = 40251, // need further scripting?

    //\todo all these texts seems to be incorrect... is the creature entry correct?
    //SINFUL_BEAM0 = 00000,

    // Unselectable Trigger AI
    CN_DOOR_EVENT_TRIGGER = 60001,
    CN_FACE_TRIGGER = 60002,

    // Demon Fire settings
    CN_DEMON_FIRE = 23069,
    DEMON_FIRE = 40029,
    DEMON_FIRE_DESPAWN = 60000, // time in ms

    // Blaze Effect settings
    CN_BLAZE_EFFECT = 23259,
    BLAZE_EFFECT = 40610,
    BLAZE_EFFECT_DESPAWN = 60000, // time in ms

    // Flame Crash Effect settings
    CN_FLAME_CRASH = 23336,
    FLAME_CRASH_EFFECT = 40836,
    FLAME_CRASH_EFFECT_DESPAWN = 120000, // time in ms

    // Flame Burst settings
    CN_FLAME_BURST = 30003,
    FLAME_BURST = 41131,
    FLAME_BURST_DESPAWN = 1000, // time in ms

    // Eye Beam Trigger AI
    CN_EYE_BEAM_TRIGGER = 30000,

    EYE_BLAST = 40017,

    // Shadow Demon AI
    CN_SHADOW_DEMON = 23375,

    SHADOW_DEMON_PARALYZE = 41083,
    SHADOW_DEMON_CONSUME_SOUL = 41080,
    SHADOW_DEMON_PURPLE_BEAM = 39123, // temporary spell
    SHADOW_DEMON_PASSIVE = 41079,

    // Parasitic Shadowfiend AI - This AI is one huge hack :P
    CN_PARASITIC_SHADOWFIEND = 23498,

    PARASITIC_SHADOWFIEND_PASSIVE = 41913,
    PARASITIC_SHADOWFIEND_WITH_DAMAGE = 41917,

    CN_AKAMA = 22990,

    AKAMA_HEALING_POTION = 40535,
    AKAMA_BLESSING_OF_KINGS = 20217,
    AKAMA_DESPAWN = 41242,

    // Door Event Spells
    AKAMA_DOOR_FAIL = 41271,
    AKAMA_DOOR_OPEN = 41268,
    DEATHSWORN_DOOR_OPEN = 41269,
    GATE_FAILURE = 10390,

    AKAMA_WAYPOINT_SIZE = 20,

    CN_MAIEV = 23197,

    //    TELEPORT_MAIEV  = 41221,
    MAIEV_TELEPORT = 34673,
    MAIEV_SHADOW_STRIKE = 40685,
    MAIEV_THROW_DAGGER = 41152,
    MAIEV_CAGE_TRAP_SUMMON = 40694,

    //\todo rewritten Illidan Script
    CN_ILLIDAN_STORMRAGE = 22917,

    // Normal Form Spells
    ILLIDAN_SHEAR = 41032, // +
    ILLIDAN_DRAW_SOUL = 40904, // +
    ILLIDAN_FLAME_CRASH = 40832,  // +

    // Phase 2 Spells
    ILLIDAN_FIREBALL = 40598, // +
    ILLIDAN_DARK_BARRAGE = 40585, // ? Haven't Tried
    //ILLIDAN_SUMMON_TEAR_OF_AZZINOTH = 39855, // +
    ILLIDAN_EYE_BLAST1 = 39908,
    //ILLIDAN_EYE_BLAST2 = 40017,

    // Illidan's Glaive spells
    //ILLIDAN_SUMMON_GLAIVE = 41466,
    ILLIDAN_GLAIVE_RETURNS = 39873,
    ILLIDAN_THROW_GLAIVE1 = 39635,
    ILLIDAN_THROW_GLAIVE2 = 39849,

    TEAR_OF_AZZINOTH_CHANNEL = 39857,

    // Phase 3 Spells
    ILLIDAN_AGONIZING_FLAMES = 40932, // +

    // Phase 4 Spells
    ILLIDAN_DEMON_FORM = 40506,
    //ILLIDAN_AURA_OF_DREAD = 41142,
    ILLIDAN_SHADOW_BLAST = 41078, // +
    ILLIDAN_SUMMON_DEMON = 41117, // - Core Support
    ILLIDAN_FLAME_BURST1 = 41131, // ? Frost-nova-like effect dealing damage
    ILLIDAN_FLAME_BURST2 = 41126, // ? Cast Effect

    // Demon Forms
    //ILLIDAN_DEMON_FORM1 = 40511, // +
    //ILLIDAN_DEMON_FORM2 = 40398, // +
    //ILLIDAN_DEMON_FORM3 = 40510, // +

    // Phase 5 Spells
    ILLIDAN_ENRAGE = 40683, // +

    // Other spells
    ILLIDAN_SHADOW_PRISON = 40647, // +
    ILLIDAN_SKULL_INTRO = 39656, // + Works with RemoveAura
    //ILLIDAN_SUMMON_PARASITIC_SHADOWFIENDS = 41915, // ? Haven't Tried
    //ILLIDAN_PARASITIC_SHADOWFIEND_WITH_SE = 41914,
    ILLIDAN_BERSERK = 45078,
    ILLIDAN_DEATH1 = 41218,
    ILLIDAN_DEATH2 = 41220,
    ILLIDAN_RAKE = 1822, // + Used to force animation use

    // Other macros
    CN_BLADE_OF_AZZINOTH = 22996,
    ILLIDAN_WAYPOINT_SIZE = 5,

    // Cage Trap Trigger AI
    CN_CAGE_TRAP_DISTURB_TRIGGER = 23304, // GO data taken from UDB
    CN_CAGE_TRAP_TRIGGER = 23292, // 23292 - 9
    GO_CAGE_TRAP = 185916,

    CAGE_TRAP = 40760,
    CAGED1 = 40695,
    CAGED2 = 40713, // must find proper spell for it

    // Udalo and Olum AI
    CN_UDALO = 23410,
    CN_OLUM = 23411,

    CN_FLAME_OF_AZZINOTH = 22997,

    //FLAME_OF_AZZINOTH_BLAZE = 40637, // this one just summons mob that will trigger that fire cloud - currently not used
    FLAME_OF_AZZINOTH_FLAME_BLAST = 40631,
    FLAME_OF_AZZINOTH_ENRAGE = 40683, // 40683 or 40743
    FLAME_OF_AZZINOTH_CHARGE = 40602, // CHAOS_CHARGE 40497 CHARGE 40602

};

class BlackTempleScript : public InstanceScript
{
public:

    explicit BlackTempleScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
    {
        if (getData(CN_SUPREMUS) == Finished)
            setGameObjectStateForEntry(185882, GO_STATE_OPEN); // Gate to Black Temple behind Supremus
    }

    static InstanceScript* Create(MapMgr* pMapMgr) { return new BlackTempleScript(pMapMgr); }

    void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
    {
        // You don't have to use additional scripts to open any gates / doors
        switch (pVictim->getEntry())
        {
            case CN_SUPREMUS:
                setGameObjectStateForEntry(185882, GO_STATE_OPEN); // Gate to Black Temple behind Supremus
                break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Beast AIs

class DragonTurtleAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DragonTurtleAI)
    explicit DragonTurtleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGON_TRUTLE_SHELL_SHIELD, 10.0f, TARGET_SELF, 0, 25);
        addAISpell(DRAGON_TURTLE_WATER_SPIT, 8.0f, TARGET_ATTACKING, 3, 20);
    }
};

class LeviathanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LeviathanAI)
    explicit LeviathanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LEVIATHAN_DEBILITATING_SPRAY, 8.0f, TARGET_RANDOM_SINGLE, 3, 20);
        addAISpell(LEVIATHAN_POISON_SPIT, 8.0f, TARGET_RANDOM_SINGLE, 3, 25);
        addAISpell(LEVIATHAN_TAIL_SWEEP, 7.0f, TARGET_SELF, 0, 30);
    }
};

class MutantWarHoundAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MutantWarHoundAI)
    explicit MutantWarHoundAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*pKiller*/) override
    {
        Aura* pAura = sSpellMgr.newAura(sSpellMgr.getSpellInfo(MUTANT_WAR_HOUND_CLOUD_OF_DISEASE), (uint32_t)20000, getCreature(), getCreature());
        getCreature()->addAura(pAura);
    }
};

class ShadowmoonRidingHoundAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonRidingHoundAI)
    explicit ShadowmoonRidingHoundAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_RIDING_HOUND_CARNIVOROUS_BITE, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(SHADOWMOON_RIDING_HOUND_CHARGE, 8.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(SHADOWMOON_RIDING_HOUND_ENRAGE, 6.0f, TARGET_SELF, 0, 40);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Demon AIs

class SisterOfPleasureAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SisterOfPleasureAI)
    explicit SisterOfPleasureAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SISTER_OF_PLEASURE_GREATER_HEAL, 10.0f, TARGET_RANDOM_FRIEND, 2, 45);
        addAISpell(SISTER_OF_PLEASURE_HOLY_NOVA, 10.0f, TARGET_SELF, 0, 20);
        addAISpell(SISTER_OF_PLEASURE_SHARED_BONDS, 5.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(SISTER_OF_PLEASURE_SHELL_OF_LIFE, 7.0f, TARGET_SELF, 1, 45);
    }
};

class SisterOfPainAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SisterOfPainAI)
    explicit SisterOfPainAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SISTER_OF_PAIN_LASH_OF_PAIN, 10.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(SISTER_OF_PAIN_PAINFUL_RAGE, 5.0f, TARGET_SELF, 0, 60);
        addAISpell(SISTER_OF_PAIN_SHADOW_WORD_PAIN, 10.0f, TARGET_RANDOM_SINGLE, 0, 25);
        addAISpell(SISTER_OF_PAIN_SHARED_BONDS, 5.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(SISTER_OF_PAIN_SHELL_OF_PAIN, 7.0f, TARGET_SELF, 1, 45);
    }
};


class PriestessOfDementiaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PriestessOfDementiaAI)
    explicit PriestessOfDementiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PRIESTESS_OF_DEMENTIA_CONFUSION, 8.0f, TARGET_SELF, 0, 30);
        addAISpell(PRIESTESS_OF_DEMENTIA_DEMENTIA, 5.0f, TARGET_SELF, 0, 60);
    }
};

class PriestessOfDelightAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PriestessOfDelightAI)
    explicit PriestessOfDelightAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PRIESTESS_OF_DELIGHT_CURSE_OF_VITALITY, 10.0f, TARGET_RANDOM_SINGLE, 0, 30);
    }
};

class IllidariNightlordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariNightlordAI)
    explicit IllidariNightlordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_NIGHTLORD_SUMMON_SHADOWFIENDS, 7.0f, TARGET_SELF, 0, 45);            // does it work?
        addAISpell(ILLIDARI_NIGHTLORD_SHADOW_INFERNO, 10.0f, TARGET_SELF, 0, 25);
        addAISpell(ILLIDARI_NIGHTLORD_FEAR, 7.0f, TARGET_SELF, 1, 30);
        addAISpell(ILLIDARI_NIGHTLORD_CURSE_OF_MENDING, 8.0f, TARGET_RANDOM_SINGLE, 2, 35);
    }
};

class IllidariHeartseekerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariHeartseekerAI)
    explicit IllidariHeartseekerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto bleakheart = addAISpell(ILLIDARI_HEARTSEEKER_CURSE_OF_THE_BLEAKHEART, 8.0f, TARGET_RANDOM_SINGLE, 0, 40);
        bleakheart->setMinMaxDistance(0.0f, 30.0f);

        mRapidShot = addAISpell(ILLIDARI_HEARTSEEKER_RAPID_SHOT, 7.0f, TARGET_SELF, 8, 0);

        auto shoot = addAISpell(ILLIDARI_HEARTSEEKER_SHOOT, 80.0f, TARGET_ATTACKING, 0, 2);
        shoot->setMinMaxDistance(0.0f, 30.0f);

        auto shot = addAISpell(ILLIDARI_HEARTSEEKER_SKELETON_SHOT, 7.0f, TARGET_RANDOM_SINGLE, 0, 25);
        shot->setMinMaxDistance(0.0f, 30.0f);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 30.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 30.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }

    CreatureAISpells* mRapidShot;
};

class IllidariFearbringerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariFearbringerAI)
    explicit IllidariFearbringerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_FEARBRINGER_ILLIDARI_FLAMES, 8.0f, TARGET_ATTACKING, 2, 25);
        addAISpell(ILLIDARI_FEARBRINGER_RAIN_OF_CHAOS, 7.0f, TARGET_RANDOM_DESTINATION, 0, 35);
        addAISpell(ILLIDARI_FEARBRINGER_WAR_STOMP, 8.0f, TARGET_SELF, 0, 35);
    }
};

class IllidariDefilerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariDefilerAI)
    explicit IllidariDefilerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_DEFILER_BANISH, 8.0f, TARGET_RANDOM_SINGLE, 1, 30);
        addAISpell(ILLIDARI_DEFILER_CURSE_OF_AGONY, 7.0f, TARGET_RANDOM_SINGLE, 0, 45);
        addAISpell(ILLIDARI_DEFILER_FEL_IMMOLATE, 8.0f, TARGET_RANDOM_SINGLE, 2, 25);
        addAISpell(ILLIDARI_DEFILER_RAIN_OF_CHAOS, 7.0f, TARGET_RANDOM_DESTINATION, 6, 35);
    }
};

class IllidariCenturionAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariCenturionAI)
    explicit IllidariCenturionAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_CENTURION_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(ILLIDARI_CENTURION_SONIC_STRIKE, 8.0f, TARGET_SELF, 0, 35);
    }
};

class IllidariBoneslicerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariBoneslicerAI)
    explicit IllidariBoneslicerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_BONESLICER_CLOAK_OF_SHADOWS, 8.0f, TARGET_SELF, 0, 25);
        addAISpell(ILLIDARI_BONESLICER_GOUGE, 7.0f, TARGET_ATTACKING);
        addAISpell(ILLIDARI_BONESLICER_SHADOWSTEP, 7.0f, TARGET_ATTACKING, 0, 30);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Humanoid AIs
class AshtongueBattlelordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueBattlelordAI)
    explicit AshtongueBattlelordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_BATTLELORD_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(ASHTONGUE_BATTLELORD_CONCUSSION_BLOW, 8.0f, TARGET_ATTACKING, 0, 30);
        addAISpell(ASHTONGUE_BATTLELORD_CONCUSSIVE_THROW, 8.0f, TARGET_RANDOM_SINGLE, 0, 25);
        addAISpell(ASHTONGUE_BATTLELORD_ENRAGE, 4.0f, TARGET_SELF, 0, 50);
    }
};

class AshtongueDefenderAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueDefenderAI)
    explicit AshtongueDefenderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_DEFENDER_DEBILITATING_STRIKE, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(ASHTONGUE_DEFENDER_SHIELD_BASH, 7.0f, TARGET_ATTACKING, 0, 25);
    }
};

class AshtongueElementalistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueElementalistAI)
    explicit AshtongueElementalistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_ELEMENTALIST_LIGHTNING_BOLT, 8.0f, TARGET_ATTACKING, 1, 20);
        addAISpell(ASHTONGUE_ELEMENTALIST_RAID_OF_FIRE, 6.0f, TARGET_RANDOM_DESTINATION, 0, 25);
    }
};

//\ todo Add Totem AIs
class AshtongueMysticAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueMysticAI)
    explicit AshtongueMysticAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_MYSTIC_BLOODLUST, 5.0f, TARGET_SELF, 0, 45);
        addAISpell(ASHTONGUE_MYSTIC_CHAIN_HEAL, 6.0f, TARGET_RANDOM_FRIEND, 3, 35);
        addAISpell(ASHTONGUE_MYSTIC_CYCLONE_TOTEM, 7.0f, TARGET_SELF, 0, 35);
        addAISpell(ASHTONGUE_MYSTIC_FLAME_SHOCK, 8.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(ASHTONGUE_MYSTIC_FROST_SHOCK, 8.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(ASHTONGUE_MYSTIC_SEARING_TOTEM, 7.0f, TARGET_SELF, 0, 35);
        addAISpell(ASHTONGUE_MYSTIC_SUMMON_WINDFURY_TOTEM, 7.0f, TARGET_SELF, 0, 35);
    }
};

class AshtonguePrimalistAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtonguePrimalistAI)
    explicit AshtonguePrimalistAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_PRIMALIST_MULTISHOT, 8.0f, TARGET_ATTACKING, 0, 40);
        addAISpell(ASHTONGUE_PRIMALIST_SHOOT, 80.0f, TARGET_ATTACKING, 0, 1);
        addAISpell(ASHTONGUE_PRIMALIST_SWEEPING_WING_CLIP, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        addAISpell(ASHTONGUE_PRIMALIST_WYVERN_STRING, 7.0f, TARGET_RANDOM_SINGLE, 0, 25);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 30.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 30.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }
};

class AshtongueRogueAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueRogueAI)
    explicit AshtongueRogueAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_ROGUE_DEBILITATING_POISON, 7.0f, TARGET_ATTACKING, 0, 30);
        addAISpell(ASHTONGUE_ROGUE_EVISCERATE, 10.0f, TARGET_ATTACKING, 0, 15);
    }
};

class AshtongueSpiritbinderAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueSpiritbinderAI)
    explicit AshtongueSpiritbinderAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_SPIRITBINDER_CHAIN_HEAL, 7.0f, TARGET_RANDOM_FRIEND, 1, 25);

        if (!_isHeroic())    // Guessed
            addAISpell(ASHTONGUE_SPIRITBINDER_SPIRIT_HEAL, 7.0f, TARGET_SELF, 0, 25);
        else
            addAISpell(ASHTONGUE_SPIRITBINDER_SPIRIT_HEAL2, 7.0f, TARGET_SELF, 0, 25);

        addAISpell(ASHTONGUE_SPIRITBINDER_SPIRIT_MEND, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
    }
};

//\brief Completely guessed mechanics
class AshtongueStalkerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueStalkerAI)
    explicit AshtongueStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_STALKER_BLIND, 7.0f, TARGET_RANDOM_SINGLE, 1, 25);
        addAISpell(ASHTONGUE_STALKER_INSTANT_POISON, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(ASHTONGUE_STALKER_MINDNUMBING_POISON, 8.0f, TARGET_RANDOM_SINGLE, 0, 20);
        _applyAura(ASHTONGUE_STALKER_STEATH);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isAlive())
        {
            _applyAura(ASHTONGUE_STALKER_STEATH);
        }
    }
};

class AshtongueStormcallerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AshtongueStormcallerAI)
    explicit AshtongueStormcallerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ASHTONGUE_STORMCALLER_CHAIN_LIGHTNING, 7.0f, TARGET_ATTACKING, 3, 35);
        addAISpell(ASHTONGUE_STORMCALLER_LIGHTNING_BOLT, 10.0f, TARGET_ATTACKING, 2, 20);
        addAISpell(ASHTONGUE_STORMCALLER_LIGHTNING_SHIELD, 10.0f, TARGET_SELF, 0, 50);
    }
};

class BonechewerBehemothAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerBehemothAI)
    explicit BonechewerBehemothAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCharge = addAISpell(BONECHEWER_BEHEMOTH_BEHEMOTH_CHARGE, 0.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(BONECHEWER_BEHEMOTH_ENRAGE, 5.0f, TARGET_SELF, 0, 45);
        addAISpell(BONECHEWER_BEHEMOTH_FEL_STOMP, 7.0f, TARGET_SELF, 0, 30);
        addAISpell(BONECHEWER_BEHEMOTH_FIERY_COMET, 6.0f, TARGET_RANDOM_DESTINATION, 1, 30);
        addAISpell(BONECHEWER_BEHEMOTH_METEOR, 5.0f, TARGET_RANDOM_DESTINATION, 2, 40);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (pCharge != nullptr)
        {
            _castAISpell(pCharge);
        }
    }

    CreatureAISpells* pCharge;
};

class BonechewerBladeFuryAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerBladeFuryAI)
    explicit BonechewerBladeFuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_BLADE_FURY_WHIRLWIND, 7.0f, TARGET_ATTACKING, 8, 40);
    }
};

class BonechewerBloodProphetAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerBloodProphetAI)
    explicit BonechewerBloodProphetAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_BLOOD_PROPHET_BLOOD_DRAIN, 6.0f, TARGET_ATTACKING, 5, 30);
        addAISpell(BONECHEWER_BLOOD_PROPHET_BLOODBOLT, 7.0f, TARGET_RANDOM_SINGLE, 2, 25);
        addAISpell(BONECHEWER_BLOOD_PROPHET_ENRAGE, 5.0f, TARGET_SELF, 0, 50);

        if (_isHeroic())
            addAISpell(BONECHEWER_BLOOD_PROPHET_PROPHECY_OF_BLOOD, 10.0f, TARGET_RANDOM_SINGLE, 0, 30);
        else
            addAISpell(BONECHEWER_BLOOD_PROPHET_PROPHECY_OF_BLOOD2, 10.0f, TARGET_RANDOM_SINGLE, 0, 30);
    }
};

class BonechewerBrawlerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerBrawlerAI)
    explicit BonechewerBrawlerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_BRAWLER_FRENZY, 7.0f, TARGET_SELF, 0, 30);
    }
};

class BonechewerCombatantAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerCombatantAI)
    explicit BonechewerCombatantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_COMBATANT_FRENZY, 7.0f, TARGET_SELF, 0, 45);
    }
};

class BonechewerShieldDiscipleAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerShieldDiscipleAI)
    explicit BonechewerShieldDiscipleAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pIntervene = addAISpell(BONECHEWER_SHIELD_DISCIPLE_INTERVENE, 0.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(BONECHEWER_SHIELD_DISCIPLE_SHIELD_BASH, 8.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(BONECHEWER_SHIELD_DISCIPLE_SHIELD_WALL, 8.0f, TARGET_SELF, 0, 35);
        addAISpell(BONECHEWER_SHIELD_DISCIPLE_THROW_SHIELD, 7.0f, TARGET_RANDOM_SINGLE, 0, 30);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (pIntervene != nullptr)
        {
            _castAISpell(pIntervene);
        }
    }

    CreatureAISpells* pIntervene;
};

class BonechewerSpectatorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerSpectatorAI)
    explicit BonechewerSpectatorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pCharge = addAISpell(BONECHEWER_SPECTATOR_CHARGE, 0.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(BONECHEWER_SPECTATOR_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(BONECHEWER_SPECTATOR_MORTAL_WOUND, 7.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(BONECHEWER_SPECTATOR_STRIKE, 10.0f, TARGET_ATTACKING, 0, 10);
        addAISpell(BONECHEWER_SPECTATOR_SUNDER_ARMOR, 7.0f, TARGET_ATTACKING, 0, 20);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (pCharge != nullptr)
        {
            _castAISpell(pCharge);
        }
    }

    CreatureAISpells* pCharge;
};

class BonechewerTaskmasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerTaskmasterAI)
    explicit BonechewerTaskmasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_TASKMASTER_DISGRUNTLED, 5.0f, TARGET_SELF, 0, 45);
        addAISpell(BONECHEWER_TASKMASTER_FURY, 8.0f, TARGET_SELF, 0, 15);
    }
};

class BonechewerWorkerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BonechewerWorkerAI)
    explicit BonechewerWorkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BONECHEWER_WORKER_THROW_PICK, 8.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }
};

class CharmingCourtesanAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CharmingCourtesanAI)
    explicit CharmingCourtesanAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(CHARMING_COURTESAN_INFATUATION, 7.0f, TARGET_RANDOM_SINGLE, 20, 40);
        addAISpell(CHARMING_COURTESAN_POISONOUS_THROW, 8.0f, TARGET_RANDOM_SINGLE, 0, 20);
    }
};

class CoilskarGeneralAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarGeneralAI)
    explicit CoilskarGeneralAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COILSKAR_GENERAL_BOOMING_VOICE, 7.0f, TARGET_SELF, 0, 35);
        addAISpell(COILSKAR_GENERAL_FREE_FRIEND, 7.0f, TARGET_RANDOM_FRIEND, 0, 20);
    }
};

//\todo Is it Ranged unit throwing spears all the time?
class CoilskarHarpoonerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarHarpoonerAI)
    explicit CoilskarHarpoonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        pMark = addAISpell(COILSKAR_HARPOONER_HARPOONERS_MARK, 7.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(COILSKAR_HARPOONER_HOOKED_NET, 7.0f, TARGET_RANDOM_SINGLE, 0, 25);
        addAISpell(COILSKAR_HARPOONER_SPEAR_THROW, 8.0f, TARGET_ATTACKING, 0, 15);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        if (pMark != nullptr)
        {
            _castAISpell(pMark);
        }
    }

    CreatureAISpells* pMark;
};

class CoilskarSeacallerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarSeacallerAI)
    explicit CoilskarSeacallerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COILSKAR_SEACALLER_FORKED_LIGHTNING, 8.0f, TARGET_SELF, 2, 20);
        addAISpell(COILSKAR_SEACALLER_HURRICANE, 8.0f, TARGET_RANDOM_DESTINATION, 20, 35);
        addAISpell(COILSKAR_SEACALLER_SUMMON_GEYSER, 7.0f, TARGET_RANDOM_SINGLE, 2, 25);
    }
};

class CoilskarSoothsayerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarSoothsayerAI)
    explicit CoilskarSoothsayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COILSKAR_SOOTHSAYER_HOLY_NOVA, 10.0f, TARGET_SELF, 0, 20);
        addAISpell(COILSKAR_SOOTHSAYER_RESTORATION, 8.0f, TARGET_RANDOM_FRIEND, 2, 35);
    }
};

class CoilskarWranglerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CoilskarWranglerAI)
    explicit CoilskarWranglerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(COILSKAR_WRANGLER_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(COILSKAR_WRANGLER_ELECTRIC_SPUR, 8.0f, TARGET_SELF, 3, 45);
        addAISpell(COILSKAR_WRANGLER_LIGHTNING_PROD, 8.0f, TARGET_RANDOM_SINGLE, 3, 25);
    }
};

class DragonmawSkyStalkerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DragonmawSkyStalkerAI)
    explicit DragonmawSkyStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONMAW_SKY_STALKER_IMMOLATION_ARROW, 8.0f, TARGET_ATTACKING, 2, 15);
        addAISpell(DRAGONMAW_SKY_STALKER_SHOOT, 75.0f, TARGET_ATTACKING, 0, 1);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 40.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 40.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }
};

//\todo Should it run away from tank when he's close?
class DragonmawWindReaverAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DragonmawWindReaverAI)
    explicit DragonmawWindReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONMAW_WIND_REAVER_DOOM_BOLT, 10.0f, TARGET_ATTACKING, 2, 15);
        addAISpell(DRAGONMAW_WIND_REAVER_FIREBALL, 75.0f, TARGET_ATTACKING, 2, 0);
        addAISpell(DRAGONMAW_WIND_REAVER_FREEZE, 10.0f, TARGET_RANDOM_SINGLE, 2, 15);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 40.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 40.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }
};

class DragonmawWyrmcallerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DragonmawWyrmcallerAI)
    explicit DragonmawWyrmcallerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DRAGONMAW_WYRMCALLER_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(DRAGONMAW_WYRMCALLER_FIXATE, 7.0f, TARGET_RANDOM_SINGLE, 0, 20);
        addAISpell(DRAGONMAW_WYRMCALLER_JAB, 8.0f, TARGET_ATTACKING, 0, 25);
    }
};

class EnslavedServantAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EnslavedServantAI)
    explicit EnslavedServantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ENSLAVED_SERVANT_KIDNEY_SHOT, 7.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(ENSLAVED_SERVANT_UPPERCUT, 8.0f, TARGET_RANDOM_SINGLE, 0, 20);

        mHealthResetTimer = -1;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mHealthResetTimer = _addTimer(45000);    // to check
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mHealthResetTimer))
        {
            getCreature()->setHealth(getCreature()->getMaxHealth());    // Found such note about this mob
            _resetTimer(mHealthResetTimer, 45000);
        }
    }

    int32_t mHealthResetTimer;
};

class HandOfGorefiendAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HandOfGorefiendAI)
    explicit HandOfGorefiendAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(HAND_OF_GOREFIEND_FRENZY, 6.0f, TARGET_SELF, 0, 45);
    }
};

//\todo Mechanics are guessed. I'm also not sure if it's not typical caster unit
class IllidariArchonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariArchonAI)
    explicit IllidariArchonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                addAISpell(ILLIDARI_ARCHON_HEAL, 5.0f, TARGET_RANDOM_FRIEND, 2, 30);
                addAISpell(ILLIDARI_ARCHON_HOLY_SMITE, 8.0f, TARGET_ATTACKING, 3, 25);
                addAISpell(ILLIDARI_ARCHON_POWER_WORD_SHIELD, 7.0f, TARGET_SELF, 0, 35);
                break;
            case 1:
                addAISpell(ILLIDARI_ARCHON_MIND_BLAST, 8.0f, TARGET_ATTACKING, 2, 25);
                _applyAura(ILLIDARI_ARCHON_SHADOWFORM);
                break;
        }
    }
};

//\todo Couldn't find mechanics nowhere around the net, so kept it simple
class IllidariAssassinAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariAssassinAI)
    explicit IllidariAssassinAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_ASSASSIN_PARALYZING_POISON, 8.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(ILLIDARI_ASSASSIN_VANISH, 7.0f, TARGET_SELF, 1, 30);
    }

    void AIUpdate() override
    {
        if (getCreature()->HasAura(ILLIDARI_ASSASSIN_VANISH))
        {
            _delayNextAttack(1500);
        }            
    }
};

//\todo I've parted it on frost and fire mage - correct me if it's wrong (also slap me if it's typical caster)
class IllidariBattlemageAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariBattlemageAI)
    explicit IllidariBattlemageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        switch (Util::getRandomUInt(1))
        {
            case 0:
                addAISpell(ILLIDARI_BATTLEMAGE_BLIZZARD, 8.0f, TARGET_RANDOM_DESTINATION, 8, 35);
                addAISpell(ILLIDARI_BATTLEMAGE_FROSTBOLT, 15.0f, TARGET_ATTACKING, 0, 10);
                break;
            case 1:
                addAISpell(ILLIDARI_BATTLEMAGE_FIREBALL, 15.0f, TARGET_ATTACKING, 0, 10);
                addAISpell(ILLIDARI_BATTLEMAGE_FLAMESTRIKE, 8.0f, TARGET_RANDOM_DESTINATION, 0, 40);
                break;
        }
    }
};

class IllidariBloodLordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidariBloodLordAI)
    explicit IllidariBloodLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ILLIDARI_BLOOD_LORD_DIVINE_SHIELD, 8.0f, TARGET_SELF, 0, 30);
        addAISpell(ILLIDARI_BLOOD_LORD_HAMMER_OF_JUSTICE, 9.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(ILLIDARI_BLOOD_LORD_JUDGEMENT_OF_COMMAND, 8.0f, TARGET_ATTACKING, 0, 25);
    }
};

//\todo Should be summoned by Priestess of Dementia
class ImageOfDementiaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ImageOfDementiaAI)
    explicit ImageOfDementiaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(IMAGE_OF_DEMENTIA_WHRILWIND, 15.0f, TARGET_SELF, 15, 30);
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == IMAGE_OF_DEMENTIA_WHRILWIND)
            despawn(25000);
    }
};

class ShadowmoonBloodMageAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonBloodMageAI)
    explicit ShadowmoonBloodMageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_BLOOD_MAGE_BLOOD_SIPHON, 8.0f, TARGET_SELF, 0, 35);
        addAISpell(SHADOWMOON_BLOOD_MAGE_BLOOD_BOLT, 9.0f, TARGET_RANDOM_SINGLE, 3, 25);
    }
};

class ShadowmoonChampionAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonChampionAI)
    explicit ShadowmoonChampionAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_CHAMPION_CHAOTIC_LIGHT, 8.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(SHADOWMOON_CHAMPION_WHIRLING_BLADE, 8.0f, TARGET_RANDOM_SINGLE, 1, 30);    // I must check its mechanics
    }
};

class ShadowmoonDeathshaperAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonDeathshaperAI)
    explicit ShadowmoonDeathshaperAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_DEATHSHAPER_DEATH_COIL, 7.0f, TARGET_RANDOM_SINGLE, 0, 30);
        addAISpell(SHADOWMOON_DEATHSHAPER_DEMON_ARMOR, 8.0f, TARGET_SELF, 0, 60);
        addAISpell(SHADOWMOON_DEATHSHAPER_RAISE_DEAD, 12.0f, TARGET_SELF, 2, 30);
        addAISpell(SHADOWMOON_DEATHSHAPER_SHADOW_BOLT, 75.0f, TARGET_ATTACKING, 3, 0);

        getCreature()->setPower(POWER_TYPE_MANA, 100000);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 40.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SHADOWMOON_DEATHSHAPER_RAISE_DEAD)
        {
            Creature* pAI = spawnCreature(23371, getCreature()->GetPosition());
            if (pAI != NULL)
            {
                pAI->GetAIInterface()->StopMovement(2500);
                pAI->setAttackTimer(MELEE, 2500);
            }

            getCreature()->Despawn(3000, 0);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 40.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }
};

//\todo No entry in my DB, but AI may stay :)?
class ShadowmoonFallenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonFallenAI)
    explicit ShadowmoonFallenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    { }
};

class ShadowmoonHoundmasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonHoundmasterAI)
    explicit ShadowmoonHoundmasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_HOUNDMASTER_FLARE, 5.0f, TARGET_RANDOM_DESTINATION, 0, 30);
        addAISpell(SHADOWMOON_HOUNDMASTER_FREEZING_TRAP, 5.0f, TARGET_SELF, 0, 30);
        addAISpell(SHADOWMOON_HOUNDMASTER_SHOOT, 70.0f, TARGET_ATTACKING, 0, 1);
        addAISpell(SHADOWMOON_HOUNDMASTER_SILENCING_SHOT, 6.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(SHADOWMOON_HOUNDMASTER_SUMMON_RIDING_WARHOUND, 5.0f, TARGET_SELF, 0, 45);
        addAISpell(SHADOWMOON_HOUNDMASTER_VOLLEY, 5.0f, TARGET_RANDOM_DESTINATION, 1, 25);
        addAISpell(SHADOWMOON_HOUNDMASTER_WING_CLIP, 5.0f, TARGET_ATTACKING, 0, 20, false, true);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 30.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void AIUpdate() override
    {
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL)
        {
            if (getRangeToObject(pTarget) <= 30.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }
    }
};

class ShadowmoonReaverAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonReaverAI)
    explicit ShadowmoonReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_REAVER_SPELL_ABSORPTION, 8.0f, TARGET_SELF, 0, 35);
    }
};

class ShadowmoonSoldierAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonSoldierAI)
    explicit ShadowmoonSoldierAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWMOON_SOLDIER_STRIKE, 10.0f, TARGET_ATTACKING, 0, 15);
    }
};

//\todo Haven't found informations about Shield Wall ability
class ShadowmoonWeaponMasterAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowmoonWeaponMasterAI)
    explicit ShadowmoonWeaponMasterAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto knockAway = addAISpell(SHADOWMOON_WEAPON_MASTER_KNOCK_AWAY, 9.0f, TARGET_ATTACKING, 0, 25);
        knockAway->setAvailableForScriptPhase({ 1, 2, 3 });

        auto mutilate = addAISpell(SHADOWMOON_WEAPON_MASTER_MUTILATE, 8.0f, TARGET_ATTACKING, 0, 30);
        mutilate->setAvailableForScriptPhase({ 1, 3 });

        auto shielWall = addAISpell(SHADOWMOON_WEAPON_MASTER_SHIELD_WALL, 10.0f, TARGET_SELF, 0, 35);
        shielWall->setAvailableForScriptPhase({ 2 });

        auto whirlwind = addAISpell(SHADOWMOON_WEAPON_MASTER_WHIRLWIND, 10.0f, TARGET_SELF, 15, 35);
        whirlwind->setAvailableForScriptPhase({ 3 });

        _applyAura(SHADOWMOON_WEAPON_MASTER_BATTLE_STANCE);
        _applyAura(SHADOWMOON_WEAPON_MASTER_BATTLE_AURA);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isAlive())
        {
            _removeAura(SHADOWMOON_WEAPON_MASTER_DEFENSIVE_AURA);
            _removeAura(SHADOWMOON_WEAPON_MASTER_BERSEKER_AURA);
            //SetDisplayWeaponIds(0, 0)    // Sword
            _applyAura(SHADOWMOON_WEAPON_MASTER_BATTLE_STANCE);
            _applyAura(SHADOWMOON_WEAPON_MASTER_BATTLE_AURA);
        }
    }

    void AIUpdate() override
    {
        if (isScriptPhase(1) && _getHealthPercent() <= 85)
        {
            setScriptPhase(2);
            return;
        }
        if (isScriptPhase(2) && _getHealthPercent() <= 35)
        {
            setScriptPhase(3);
            return;
        }
    }

    void OnScriptPhaseChange(uint32_t phaseId) override
    {
        switch (phaseId)
        {
            case 2:
            {
                _removeAura(SHADOWMOON_WEAPON_MASTER_BATTLE_AURA);
                _applyAura(SHADOWMOON_WEAPON_MASTER_DEFENSIVE_STANCE);
                _applyAura(SHADOWMOON_WEAPON_MASTER_DEFENSIVE_AURA);
            } break;
            case 3:
            {
                _removeAura(SHADOWMOON_WEAPON_MASTER_DEFENSIVE_AURA);
                _applyAura(SHADOWMOON_WEAPON_MASTER_BERSERKER_STANCE);
                _applyAura(SHADOWMOON_WEAPON_MASTER_BERSEKER_AURA);
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Berserker stance! Attack them recklessly!");
            } break;
            default:
                break;
        }
    }
};

class SpellboundAttendantAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SpellboundAttendantAI)
    explicit SpellboundAttendantAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELLBOUND_ATTENDANT_KICK, 9.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(SPELLBOUND_ATTENDANT_SLEEP, 8.0f, TARGET_RANDOM_SINGLE, 1, 25);
    }
};

class TempleConcubineAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TempleConcubineAI)
    explicit TempleConcubineAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(TEMPLE_CONCUBINE_LOVE_TAP, 9.0f, TARGET_ATTACKING, 0, 25);
        addAISpell(TEMPLE_CONCUBINE_POLYMORPH, 7.0f, TARGET_RANDOM_SINGLE, 1, 25);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Elemental AIs
class StormFuryAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(StormFuryAI)
    explicit StormFuryAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mStormBlink = addAISpell(STORM_FURY_STORM_BLINK, 8.0f, TARGET_SELF, 1, 0);    // Mechanics was guessed
        mStormBlink->setAttackStopTimer(500);
    }

    void AIUpdate() override
    {
        if (getCreature()->HasAura(STORM_FURY_STORM_BLINK))
        {
            _delayNextAttack(2000);
        }
    }

    CreatureAISpells* mStormBlink;
};

class AqueousSurgerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AqueousSurgerAI)
    explicit AqueousSurgerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(AQUEOUS_SURGER_POISON_BOLT_VOLLEY, 7.0f, TARGET_SELF, 0, 20);
    }
};

class AqueousSpawnAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AqueousSpawnAI)
    explicit AqueousSpawnAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(AQUEOUS_SPAWN_MERGE, 7.0f, TARGET_SELF, 11, 45);
        addAISpell(AQUEOUS_SPAWN_SLUDGE_NOVA, 8.0f, TARGET_RANDOM_SINGLE, 3, 20);
    }
};

class AqueousLordAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AqueousLordAI)
    explicit AqueousLordAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(AQUEOUS_LORD_CRASHING_WAVE, 8.0f, TARGET_SELF, 0, 35);
        addAISpell(AQUEOUS_LORD_VILE_SLIME, 10.0f,  TARGET_RANDOM_SINGLE, 0, 25);

        mAqueousTimerId = 0;
    }

    void OnCombatStart(Unit*) override
    {
        mAqueousTimerId = _addTimer(30000);
    }

    void AIUpdate() override
    {
        /*if (Util::getRandomUInt(0, 100) < 10 && _isTimerFinished(mAqueousTimerId))
        {
            CreatureAIScript* pSpawnAI = spawnCreatureAndGetAIScript(CN_AQUEOUS_SPAWN, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
        }*/
    }

    uint32_t mAqueousTimerId;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Mechanical AIs
class PromenadeSentinelAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(PromenadeSentinelAI)
    explicit PromenadeSentinelAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PROMENADE_SENTINEL_L5_ARCANE_CHARGE, 8.0f, TARGET_RANDOM_SINGLE, 3, 35);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Uncategorized AIs
class AngeredSoulFragmentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AngeredSoulFragmentAI)
    explicit AngeredSoulFragmentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ANGERED_SOUL_FRAGMENT_ANGER, 10.0f, TARGET_SELF, 3, 25);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Undead AIs
class EnslavedSoulAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EnslavedSoulAI)
    explicit EnslavedSoulAI(Creature* pCreature) : CreatureAIScript(pCreature) {}

    void OnDied(Unit* /*mKiller*/) override
    {
        _applyAura(ENSLAVED_SOUL_SOUL_RELEASE);            // beg core to support OnDied casts
        despawn(120000, 0);
    }
};

class HungeringSoulFragmentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HungeringSoulFragmentAI)
    explicit HungeringSoulFragmentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        _applyAura(HUNGERING_SOUL_FRAGMENT_CONSUMING_STRIKES);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        if (isAlive())
        {
            _applyAura(HUNGERING_SOUL_FRAGMENT_CONSUMING_STRIKES);
        }
    }
};

class ShadowyConstructAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowyConstructAI)
    explicit ShadowyConstructAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SHADOWY_CONSTRUCT_ATROPHY, 10.0f, TARGET_ATTACKING, 0, 45);
    }
};

class SufferingSoulFragmentAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SufferingSoulFragmentAI)
    explicit SufferingSoulFragmentAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SUFFERING_SOUL_FRAGMENT_SOUL_BLAST, 8.0f, TARGET_SELF, 2, 30);
    }
};

//\todo Teron Gorefiend transforms random player into it with those abilities, but AI might be handy too (too overpowered?) - cannot check (no npc in DB)
class VangefulSpiritAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VangefulSpiritAI)
    explicit VangefulSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(VANGEFUL_SPIRIT_SPIRIT_CHAINS, 8.0f, TARGET_SELF, 0, 25);
        addAISpell(VANGEFUL_SPIRIT_SPIRIT_LANCE, 6.0f, TARGET_RANDOM_SINGLE, 0, 35);
        addAISpell(VANGEFUL_SPIRIT_SPIRIT_SHIELD, 6.0f, TARGET_SELF, 0, 50);
        addAISpell(VANGEFUL_SPIRIT_SPIRIT_STRIKE, 9.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(VANGEFUL_SPIRIT_SPIRIT_VOLLEY, 4.0f, TARGET_SELF, 0, 40);
    }
};

class WrathboneFlayerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(WrathboneFlayerAI)
    explicit WrathboneFlayerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(WRATHBONE_FLAYER_CLEAVE, 10.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(WRATHBONE_FLAYER_IGNORED, 7.0f, TARGET_ATTACKING, 0, 25);
    }
};


// Bosses


// There are also other sounds, but Idk where they should go (mostly specials and enrage - which erange spell is that O_O)
class NajentusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(NajentusAI)
    explicit NajentusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto crashingWave = addAISpell(CRASHING_WAVE, 10.0f, TARGET_VARIOUS, 0, 15);
        crashingWave->setAttackStopTimer(1000);

        auto needleSpine = addAISpell(NEEDLE_SPINE, 10.0f, TARGET_RANDOM_SINGLE, 0, 10);
        needleSpine->setAttackStopTimer(2000);
        needleSpine->setMinMaxDistance(0.0f, 60.0f);

        auto tidalShield = addAISpell(TIDAL_SHIELD, 5.0f, TARGET_SELF, 0, 105, false, true);
        tidalShield->setAttackStopTimer(2000);

        auto impalingSpine = addAISpell(IMPALING_SPINE, 0.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
        impalingSpine->setAttackStopTimer(2000);
        impalingSpine->setMinMaxDistance(0.0f, 60.0f);
        impalingSpine->addDBEmote(4702);      // Stick around!
        impalingSpine->addDBEmote(4703);      // I'll deal with you later.

        addEmoteForEvent(Event_OnCombatStart, 4720);     // You will die in the name of Lady Vashj!
        addEmoteForEvent(Event_OnTargetDied, 4705);     // Time for you to go.
        addEmoteForEvent(Event_OnTargetDied, 4704); // Your success was short-lived!
        addEmoteForEvent(Event_OnDied, 4710);     // Lord Illidan will... crush you!
    }
};

class SupremusAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(SupremusAI)
    explicit SupremusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        m_MoltenFlame = m_HurtfulStrike = m_MoltenPunch = m_VolcanicGazer = true;

        infoMoltenFlame = sSpellMgr.getSpellInfo(MOLTEN_FLAME);
        infoHurtfulStrike = sSpellMgr.getSpellInfo(HURTFUL_STRIKE);
        infoMoltenPunch =  sSpellMgr.getSpellInfo(MOLTEN_PUNCH);
        infoVolcanicGazer =  sSpellMgr.getSpellInfo(VOLCANIC_GAZER);

        timer = 0;
        m_phase = 0;

        addEmoteForEvent(Event_OnCombatStart, 5034);    // Bear witness to the agent of your demise! used when he kills Warden Mellichar
        addEmoteForEvent(Event_OnTargetDied, 5035);     // Your fate is written.
        addEmoteForEvent(Event_OnTargetDied, 5036);     // The chaos I have sown here is but a taste....
        addEmoteForEvent(Event_OnDied, 5042);           // I am merely one of... infinite multitudes.
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
        timer = 0;
    }

    void AIUpdate() override
    {
        switch (m_phase)
        {
            case 1:
                {
                    PhaseOne();
                }
                break;
            case 2:
                {
                    PhaseTwo();
                }
                break;
            default:
                {
                    m_phase = 1;
                }
        }
    }

    void PhaseOne()
    {
        timer++;

        uint32_t val = Util::getRandomUInt(1000);

        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            if (m_MoltenFlame)
            {
                getCreature()->castSpell(getCreature(), infoMoltenFlame, false);
                m_MoltenFlame = false;
                return;
            }

            else if (m_HurtfulStrike)
            {
                getCreature()->castSpell(getCreature(), infoHurtfulStrike, false);
                m_HurtfulStrike = false;
                return;
            }

            if (val <= 500)
            {
                getCreature()->setAttackTimer(MELEE, 6000);//6000
                m_MoltenFlame = true;
            }

            else
            {
                getCreature()->setAttackTimer(MELEE, 4000);//2000
                m_HurtfulStrike = true;
            }
        }

        if (timer >= 45)
        {
            sendDBChatMessage(5041);     // We span the universe, as countless as the stars!
            timer = 0;
            m_phase = 2;
        }
    }

    void PhaseTwo()
    {
        timer++;

        uint32_t val = Util::getRandomUInt(1000);

        if (!getCreature()->isCastingSpell() && getCreature()->GetAIInterface()->getNextTarget())//_unit->getAttackTarget())
        {
            if (m_MoltenPunch)
            {
                getCreature()->castSpell(getCreature(), infoMoltenPunch, false);
                m_MoltenPunch = false;
                return;
            }

            else if (m_VolcanicGazer)
            {
                sendDBChatMessage(4690);     // The ground begins to crack open"
                getCreature()->castSpell(getCreature(), infoVolcanicGazer, false);
                m_VolcanicGazer = false;
                return;

            }

            if (val <= 500)
            {
                getCreature()->setAttackTimer(MELEE, 6000);//6000
                m_MoltenPunch = true;
            }

            else
            {
                getCreature()->setAttackTimer(MELEE, 4000);//2000
                m_VolcanicGazer = true;
            }
        }

        if (timer >= 45)
        {
            sendDBChatMessage(5041);     // We span the universe, as countless as the stars!
            timer = 0;
            m_phase = 1;

        }
    }

protected:
    uint32_t timer;
    uint32_t m_phase;
    bool m_MoltenFlame, m_HurtfulStrike, m_MoltenPunch, m_VolcanicGazer;
    SpellInfo const* infoMoltenFlame, *infoHurtfulStrike, *infoMoltenPunch, *infoVolcanicGazer;
};

class GurtoggAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GurtoggAI)
    explicit GurtoggAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto arcingSmash = addAISpell(ARCING_SMASH, 5.0f, TARGET_ATTACKING, 0, 10, false, true);
        arcingSmash->setAttackStopTimer(1000);

        auto acidBreath = addAISpell(FEL_ACID_BREATH, 10.0f, TARGET_ATTACKING, 0, 15, false, true);
        acidBreath->setAttackStopTimer(2000);

        //Phase 1
        auto bloodBoil = addAISpell(BLOODBOIL, 5.0f, TARGET_VARIOUS, 0, 10, false, true);
        bloodBoil->setAttackStopTimer(2000);
        bloodBoil->setAvailableForScriptPhase({ 1 });

        auto acidicWound = addAISpell(ACIDIC_WOUND, 10.0f, TARGET_ATTACKING, 0, 15, false, true);
        acidicWound->setAttackStopTimer(2000);
        acidicWound->setAvailableForScriptPhase({ 1 });

        auto disoriente = addAISpell(DISORIENT, 7.0f, TARGET_ATTACKING, 0, 10, false, true);
        disoriente->setAttackStopTimer(2000);
        disoriente->setAvailableForScriptPhase({ 1 });

        auto knockBack = addAISpell(KNOCKBACK, 5.0f, TARGET_ATTACKING, 0, 15, false, true);
        knockBack->setAttackStopTimer(2000);
        knockBack->setAvailableForScriptPhase({ 1 });

        //Phase2
        auto acidGeyser = addAISpell(ACID_GEYSER, 7.0f, TARGET_DESTINATION, 0, 60, false, true);
        acidGeyser->setAttackStopTimer(2000);
        acidGeyser->setAvailableForScriptPhase({ 2 });

        auto delRage1 = addAISpell(FEL_RAGE1, 5.0f, TARGET_RANDOM_SINGLE, 0, 90, false, true);
        delRage1->setAttackStopTimer(2000);
        delRage1->setMinMaxDistance(0.0f, 60.0f);
        delRage1->setAvailableForScriptPhase({ 2 });

        auto delRage2 = addAISpell(FEL_RAGE2, 5.0f, TARGET_SELF, 0, 90, false, true);
        delRage2->setAttackStopTimer(2000);
        delRage2->setAvailableForScriptPhase({ 2 });

        PhaseTimerId = 0;

        addEmoteForEvent(Event_OnCombatStart, 4642);    // Horde will crush you!"
        addEmoteForEvent(Event_OnTargetDied, 4644);     // "More! I want more!"
        addEmoteForEvent(Event_OnTargetDied, 4643);     // Time to feast!"
        addEmoteForEvent(Event_OnDied, 4649);           // Aaaahrg...
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        PhaseTimerId = _addTimer(60000);
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        if (isAlive())
            sendDBChatMessage(4648);     //I'll rip the meat from your bones!

        _removeTimer(PhaseTimerId);
    }

    void AIUpdate() override
    {
        if (getScriptPhase() == 1 && _isTimerFinished(PhaseTimerId))
        {
            _removeTimer(PhaseTimerId);
            setScriptPhase(2);
        }
    }

protected:

    uint32_t PhaseTimerId;
};

class EssenceOfSufferingAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EssenceOfSufferingAI)
    explicit EssenceOfSufferingAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(EOS_FIXATE, 10.0f, TARGET_ATTACKING, 0, 5);
        addAISpell(EOS_FRENZY, 100.0f, TARGET_SELF, 3, 60);
        addAISpell(EOS_SOUL_DRAIN, 7.0f, TARGET_RANDOM_SINGLE, 1, 15);
        mAuraOfSuffering = addAISpell(EOS_AURA_OF_SUFFERING, 0.0f, TARGET_RANDOM_SINGLE);

        addEmoteForEvent(Event_OnTargetDied, 8890);
        addEmoteForEvent(Event_OnTargetDied, 8891);
        addEmoteForEvent(Event_OnTargetDied, 8892);

        // Freed
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11415, "Pain and suffering are all that await you.");
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        _castAISpell(mAuraOfSuffering);
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 1)
        {
            getCreature()->SetHealthPct(1);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            _setCastDisabled(true);
            getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            _removeAllAuras();
            _removeAuraOnPlayers(EOS_AURA_OF_SUFFERING);

            CreatureAIScript* mRoS = getNearestCreatureAI(22856);
            if (mRoS != nullptr && mRoS->isAlive())
                moveToUnit(mRoS->getCreature());
        }
    }

    CreatureAISpells* mAuraOfSuffering;
};

class EssenceOfDesireAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EssenceOfDesireAI)
    explicit EssenceOfDesireAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(EOD_RUNE_SHIELD, 6.0f, TARGET_SELF, 0, 15);
        addAISpell(EOD_DEADEN, 6.0f, TARGET_ATTACKING, 1, 15);
        addAISpell(EOD_SPIRIT_SHOCK, 100.0f, TARGET_ATTACKING, 1, 15);
        mAuraOfDesire = addAISpell(EOD_AURA_OF_DESIRE, 0.0f, TARGET_RANDOM_SINGLE);

        addEmoteForEvent(Event_OnTargetDied, 8893);
        addEmoteForEvent(Event_OnTargetDied, 8894);
        addEmoteForEvent(Event_OnTargetDied, 8895);

        // Freed
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11408, "You can have anything you desire... for a price.");
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        _castAISpell(mAuraOfDesire);
    }

    void AIUpdate() override
    {
        if (_getHealthPercent() <= 1)
        {
            getCreature()->SetHealthPct(1);

            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            _setCastDisabled(true);
            getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            _removeAllAuras();
            _removeAuraOnPlayers(EOD_AURA_OF_DESIRE);

            CreatureAIScript* mRoS = getNearestCreatureAI(22856);
            if (mRoS != nullptr && mRoS->isAlive())
                moveToUnit(mRoS->getCreature());
        }
    }

    CreatureAISpells* mAuraOfDesire;
};

class EssenceOfAngerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EssenceOfAngerAI)
    explicit EssenceOfAngerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mTaunt = addAISpell(EOA_SEETHE, 0.0f, TARGET_ATTACKING); // on Taunt

        auto mSoulScream = addAISpell(EOA_SOUL_SCREAM, 6.0f, TARGET_ATTACKING, 1, 15);
        mSoulScream->addEmote("So foolish!", CHAT_MSG_MONSTER_YELL, 11400);

        auto mSpite = addAISpell(EOA_SPITE, 6.0f, TARGET_ATTACKING, 0, 15);
        mSpite->addEmote("On your knees!", CHAT_MSG_MONSTER_YELL, 11403);

        mAuraOfAnger = addAISpell(EOA_AURA_OF_ANGER, 0.0f, TARGET_RANDOM_SINGLE);

        addEmoteForEvent(Event_OnTargetDied, 8896);
        addEmoteForEvent(Event_OnTargetDied, 8897);
        addEmoteForEvent(Event_OnDied, 8898);
        addEmoteForEvent(Event_OnDied, 8899);

        sendDBChatMessage(8900);
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        _castAISpell(mAuraOfAnger);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        _removeAuraOnPlayers(EOD_AURA_OF_DESIRE);
    }

    void AIUpdate() override
    {
        if (getCreature()->GetAIInterface()->GetIsTaunted())
        {
            _castAISpell(mTaunt);
        }
    }

    CreatureAISpells* mAuraOfAnger;
    CreatureAISpells* mTaunt;
};

class ReliquaryOfSoulsAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ReliquaryOfSoulsAI)
    explicit ReliquaryOfSoulsAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mSummonSuffering = addAISpell(ROS_SUMMON_SUFFERING, 0.0f, TARGET_SELF, 0, 0);
        mSummonDesire = addAISpell(ROS_SUMMON_DESIRE, 0.0f, TARGET_SELF, 0, 0);
        mSummonAnger = addAISpell(ROS_SUMMON_ANGER, 0.0f, TARGET_SELF, 0, 0);
        setCanEnterCombat(true);
        _setMeleeDisabled(true);
        _setRangedDisabled(false);
        setRooted(true);
        Phase = 0;
        mEnslavedSoulTimer = 0;
        SpawnedEnsalvedSoul = false;
        DeadSoulCount = 0;
        mEoS = NULL;
        mEoD = NULL;
        mEoA = NULL;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
        _setMeleeDisabled(false);
        _setRangedDisabled(true);

        if (Phase == 0)
        {
            getCreature()->Emote(EMOTE_ONESHOT_SUBMERGE);
            _castAISpell(mSummonSuffering);
            Phase = 1;
        }
        RegisterAIUpdateEvent(1000);
    }

    void AIUpdate() override
    {
        switch (Phase)
        {
            case 1:
                {
                    getCreature()->Emote(EMOTE_STATE_SUBMERGED_NEW);
                    mEoS = getNearestCreatureAI(CN_ESSENCEOFSUFFERING);
                    if (mEoS && mEoS->getCreature() && mEoS->isAlive())
                    {
                        Creature* pEoS = mEoS->getCreature();
                        if (pEoS->getHealthPct() <= 1 && pEoS->CalcDistance(getCreature()) <= 3)
                        {
                            getCreature()->Emote(EMOTE_STATE_STAND);
                            mEoS->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11414, "Now what do I do?!");
                            pEoS->Emote(EMOTE_ONESHOT_SUBMERGE);
                            pEoS->Despawn(100, 0);
                            Phase = 2;
                            mEnslavedSoulTimer = _addTimer(5000);
                        }
                    }
                }
                break;
            case 3:
                {
                    mEoD = getNearestCreatureAI(CN_ESSENCEOFDESIRE);
                    if (!mEoD || !mEoD->getCreature())
                    {
                        getCreature()->Emote(EMOTE_ONESHOT_SUBMERGE);
                        _castAISpell(mSummonDesire);
                        Phase = 4;
                    }
                }
                break;
            case 4:
                {
                    mEoD = getNearestCreatureAI(CN_ESSENCEOFDESIRE);
                    if (mEoD && mEoD->getCreature() && mEoD->isAlive())
                    {
                        Creature* pEoD = mEoD->getCreature();
                        if (pEoD->getHealthPct() <= 1 && pEoD->CalcDistance(getCreature()) <= 3)
                        {
                            getCreature()->Emote(EMOTE_STATE_STAND);
                            mEoD->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11413, "I'll be waiting.");
                            pEoD->Emote(EMOTE_ONESHOT_SUBMERGE);
                            pEoD->Despawn(100, 0);
                            Phase = 5;
                            mEnslavedSoulTimer = _addTimer(5000);
                        }
                        else
                        {
                            getCreature()->Emote(EMOTE_STATE_SUBMERGED_NEW);
                        }
                    }
                }
                break;
            case 6:
                {
                    mEoA = getNearestCreatureAI(CN_ESSENCEOFANGER);
                    if (!mEoA || !mEoA->getCreature())
                    {
                        _castAISpell(mSummonAnger);
                        getCreature()->Emote(EMOTE_ONESHOT_SUBMERGE);
                        Phase = 7;
                    }
                }
                break;
            case 7:
                {
                    getCreature()->Emote(EMOTE_STATE_SUBMERGED_NEW);
                    mEoA = getNearestCreatureAI(CN_ESSENCEOFANGER);
                    if (mEoA && mEoA->getCreature() && !mEoA->getCreature()->isAlive())
                    {
                        despawn(100, 0);
                        Phase = 8;
                    }
                }
                break;
            case 2:
            case 5:
                {
                    getCreature()->Emote(EMOTE_STATE_STAND);
                    if (_isTimerFinished(mEnslavedSoulTimer) && !SpawnedEnsalvedSoul)
                    {
                        _removeTimer(mEnslavedSoulTimer);
                        SpawnedEnsalvedSoul = true;

                        _removeTimer(mEnslavedSoulTimer);
                    }
                    if (SpawnedEnsalvedSoul)
                    {
                        for (const auto& itr : getCreature()->getInRangeObjectsSet())
                        {
                            if (itr && itr->isCreature())
                            {
                                Creature* creature = static_cast<Creature*>(itr);
                                if (creature->GetCreatureProperties()->Id == CN_ENSLAVED_SOUL && !creature->isAlive())
                                    DeadSoulCount++;
                            }
                        }
                        if (DeadSoulCount == 10)
                        {
                            Phase++;
                            SpawnedEnsalvedSoul = false;
                        }
                    }
                }
                break;
        }
    }

    bool SpawnedEnsalvedSoul;
    int Phase;                  // do we have negative phase?
    int DeadSoulCount;          // negative count?
    uint32_t mEnslavedSoulTimer;  // negative timer?
    CreatureAIScript* mEoS;
    CreatureAIScript* mEoD;
    CreatureAIScript* mEoA;
    CreatureAISpells* mSummonAnger;
    CreatureAISpells* mSummonDesire;
    CreatureAISpells* mSummonSuffering;
};

// Dunno where "spell" sounds/texts should go
class ShahrazAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShahrazAI)
    explicit ShahrazAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto saberLash = addAISpell(SABER_LASH, 8.0f, TARGET_ATTACKING, 0, 15, false, true);
        saberLash->setAttackStopTimer(1000);

        auto fataAttraction = addAISpell(FATAL_ATTRACTION, 10.0f, TARGET_RANDOM_SINGLE, 0, 15, false, true);
        fataAttraction->setAttackStopTimer(2000);
        fataAttraction->setMinMaxDistance(10.0f, 60.0f);

        auto sinfulBeam = addAISpell(SINFUL_BEAM, 10.0f, TARGET_RANDOM_SINGLE, 0, 9, false, true);
        sinfulBeam->setAttackStopTimer(2000);
        sinfulBeam->setMinMaxDistance(0.0f, 80.0f);

        auto wickedBeam = addAISpell(WICKED_BEAM, 10.0f, TARGET_RANDOM_SINGLE, 0, 9, false, true);
        wickedBeam->setAttackStopTimer(2000);
        wickedBeam->setMinMaxDistance(0.0f, 80.0f);

        auto vileBeam = addAISpell(VILE_BEAM, 10.0f, TARGET_RANDOM_SINGLE, 0, 9, false, true);
        vileBeam->setAttackStopTimer(2000);
        vileBeam->setMinMaxDistance(0.0f, 80.0f);

        auto sinisterBeam = addAISpell(SINISTER_BEAM, 10.0f, TARGET_RANDOM_SINGLE, 0, 9, false, true);
        sinisterBeam->setAttackStopTimer(2000);
        sinisterBeam->setMinMaxDistance(0.0f, 80.0f);

        Enraged = false;
        AuraChange = 0;
        SoundTimer = 0;

        addEmoteForEvent(Event_OnCombatStart, 4653);    //So, business... or pleasure?"
        addEmoteForEvent(Event_OnTargetDied, 4658);     // So much for a happy ending.
        addEmoteForEvent(Event_OnTargetDied, 4657);     // Easy come, easy go.
        addEmoteForEvent(Event_OnDied, 4660);           // I wasn't finished.
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        AuraChange = (uint32_t)time(NULL) + 15;
        Enraged = false;
        SoundTimer = 5;
    }

    void AIUpdate() override
    {
        SoundTimer++;
        if (getCreature()->GetAIInterface()->GetIsTaunted() && SoundTimer > 10)
        {
            switch (Util::getRandomUInt(3))
            {
                case 1:
                    sendDBChatMessage(4651);     // I'm not impressed.
                    break;
                case 2:
                    sendDBChatMessage(4652);     // Enjoying yourselves?
                    break;
                default:
                    sendDBChatMessage(4650);     // You play, you pay.
                    break;
            }

            SoundTimer = 0;
        }

        if (!Enraged && getCreature()->getHealthPct() <= 20)
        {
            sendDBChatMessage(4659);     // Stop toying with my emotions!
            getCreature()->castSpell(getCreature(), MS_ENRAGE, true);

            Enraged = true;
        }

        uint32_t t = (uint32_t)time(NULL);

        // In normal way it is applied to players all around enemy caster =/
        if (t > AuraChange)
        {
            uint32_t SpellId = 0;
            switch (Util::getRandomUInt(6))
            {
                case 1:
                    SpellId = 40891;    // Arcane
                    break;
                case 2:
                    SpellId = 40882;    // Fire
                    break;
                case 3:
                    SpellId = 40896;    // Frost
                    break;
                case 4:
                    SpellId = 40897;    // Holy
                    break;
                case 5:
                    SpellId = 40883;    // Nature
                    break;
                case 6:
                    SpellId = 40880;    // Shadow
                    break;
                default:
                    SpellId = 40880;    // Shadow
                    break;
            }

            //_unit->castSpell(_unit, SpellId, true);
            Aura* aura = sSpellMgr.newAura(sSpellMgr.getSpellInfo(SpellId), (uint32_t)15000, getCreature(), getCreature());
            getCreature()->addAura(aura);

            AuraChange = t + 15;
        }
    }

protected:

    uint32_t SoundTimer;
    uint32_t AuraChange;
    bool Enraged;
};

typedef std::vector<Creature*> EncounterVector;

class GathiosAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GathiosAI)
    explicit GathiosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(HAMMER_OF_JUSTICE, 15.0f, TARGET_RANDOM_SINGLE, 0, 14);
        addAISpell(SEAL_OF_COMMAND, 15.0f, TARGET_SELF, 0, 30);
        addAISpell(SEAL_OF_BLOOD, 15.0f, TARGET_SELF, 0, 30);
        addAISpell(CONSECRATION, 15.0f, TARGET_SELF, 0, 30);
        addAISpell(BLESSING_OF_SPELL_WARDING, 15.0f, TARGET_RANDOM_FRIEND, 0, 60);
        addAISpell(BLESSING_OF_PROTECTION, 15.0f, TARGET_RANDOM_FRIEND, 0, 60);

        addEmoteForEvent(Event_OnTargetDied, 8799);
        addEmoteForEvent(Event_OnCombatStart, 8800);
        addEmoteForEvent(Event_OnDied, 8801);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _applyAura(DEVOTION_AURA);
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        DealDamageToFriends(fAmount, getCreature()->getEntry());
    }

    void AddEncounterCreature(Creature* pCreature)
    {
        mEncounterVector.push_back(pCreature);
    }

    void DealDamageToFriends(uint32_t val, uint32_t pCreatureEntry)
    {
        for (std::vector<Creature*>::iterator itr = mEncounterVector.begin(); itr != mEncounterVector.end(); ++itr)
        {
            if (*itr && (*itr)->isAlive() && (*itr)->getEntry() != pCreatureEntry)
            {
                (*itr)->dealDamage(*itr, val, 0);
            }
        }

        if (isAlive() && getCreature()->getEntry() != pCreatureEntry)
            getCreature()->dealDamage(getCreature(), val, 0);
    }

    void Destroy() override
    {
        mEncounterVector.clear();
        delete this;
    }

private:
    EncounterVector mEncounterVector;
};

class VerasAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VerasAI)
    explicit VerasAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DEADLY_POISON, 15.0f, TARGET_RANDOM_SINGLE, 0, 20);
        addAISpell(ENVENOM, 15.0f, TARGET_RANDOM_SINGLE, 0, 20);

        addEmoteForEvent(Event_OnTargetDied, 8901);
        addEmoteForEvent(Event_OnDied, 8902);
        addEmoteForEvent(Event_OnCombatStart, 8903);

        pGethois = static_cast< GathiosAI* >(getNearestCreatureAI(CN_GATHIOS_THE_SHATTERER));
        if (pGethois != NULL)
            pGethois->AddEncounterCreature(getCreature());
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        pGethois->DealDamageToFriends(fAmount, getCreature()->getEntry());
    }

    GathiosAI* pGethois;
};

class ZerevorAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ZerevorAI)
    explicit ZerevorAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(ARCANE_BOLT, 65.0f, TARGET_ATTACKING, 2, 6);
        addAISpell(BLIZZARD, 25.0f, TARGET_RANDOM_DESTINATION, 0, 16);
        addAISpell(FLAMESTRIKE, 10.0f, TARGET_RANDOM_DESTINATION, 2, 25);
        addAISpell(DAMPEN_MAGIC, 10.0f, TARGET_SELF, 0, 60);
        addAISpell(ARCANE_EXPLOSION, 15.0f, TARGET_RANDOM_SINGLE, 0, 6);

        addEmoteForEvent(Event_OnCombatStart, 8904);
        addEmoteForEvent(Event_OnTargetDied, 8905);

        pGethois = static_cast< GathiosAI* >(getNearestCreatureAI(CN_GATHIOS_THE_SHATTERER));
        if (pGethois != NULL)
            pGethois->AddEncounterCreature(getCreature());
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        pGethois->DealDamageToFriends(fAmount, getCreature()->getEntry());
    }

    GathiosAI* pGethois;
};

class MalandeAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MalandeAI)
    explicit MalandeAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(DIVINE_WRATH, 15.0f, TARGET_RANDOM_SINGLE, 0, 14);
        addAISpell(REFLECTIVE_SHIELD, 10.0f, TARGET_SELF, 0, 30);
        addAISpell(EMPOWERED_SMITE, 30.0f, TARGET_RANDOM_SINGLE, 2, 6);
        addAISpell(CIRCLE_OF_HEALING, 30.0f, TARGET_RANDOM_FRIEND, 2, 18);

        addEmoteForEvent(Event_OnCombatStart, 8906);
        addEmoteForEvent(Event_OnDied, 8907);
        addEmoteForEvent(Event_OnTargetDied, 8908);

        pGethois = static_cast< GathiosAI* >(getNearestCreatureAI(CN_GATHIOS_THE_SHATTERER));
        if (pGethois != NULL)
            pGethois->AddEncounterCreature(getCreature());
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t fAmount) override
    {
        if (pGethois != NULL)
            pGethois->DealDamageToFriends(fAmount, getCreature()->getEntry());
    }

    GathiosAI* pGethois;
};

// Dunno where other sounds should go
class TeronGorefiendAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(TeronGorefiendAI)
    explicit TeronGorefiendAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto doomBlossom = addAISpell(DOOM_BLOSSOM, 8.0f, TARGET_SELF, 0, 25, false, true);
        doomBlossom->setAttackStopTimer(1000);

        auto crushingShadows = addAISpell(CRUSHING_SHADOWS, 10.0f, TARGET_VARIOUS, 0, 20, false, true);
        crushingShadows->setAttackStopTimer(1000);

        auto incinerate = addAISpell(INCINERATE, 10.0f, TARGET_RANDOM_SINGLE, 0, 20, false, true);
        incinerate->setAttackStopTimer(1000);
        incinerate->setMinMaxDistance(0.0f, 45.0f);

        auto shadowOfDeath = addAISpell(SHADOW_OF_DEATH, 10.0f, TARGET_RANDOM_SINGLE, 0, 30, false, true);
        shadowOfDeath->setAttackStopTimer(1000);
        shadowOfDeath->setMinMaxDistance(0.0f, 60.0f);

        addEmoteForEvent(Event_OnCombatStart, 4692); // Vengeance is mine!
        addEmoteForEvent(Event_OnTargetDied, 4694); // It gets worse...
        addEmoteForEvent(Event_OnTargetDied, 4693); // I have use for you!
        addEmoteForEvent(Event_OnDied, 4700); // The wheel...spins...again....
    }
};

class ShadeofakamaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadeofakamaAI)
    explicit ShadeofakamaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        hm = 0;
    }

    void OnCombatStart(Unit* /*mTarget*/) override
    {
        hm = 100;
        RegisterAIUpdateEvent(getCreature()->getBaseAttackTime(MELEE));
    }

    void OnTargetDied(Unit* /*mTarget*/) override
    {
        if (getCreature()->getHealthPct() > 0)
        {
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I will not last much longer...");
                    getCreature()->PlaySoundToSet(11385);
                    break;
                case 1:
                    getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "It is time to free your soul from Illidan's grasp!");
                    //_unit->PlaySoundToSet(11510);
                    break;
                default:
                    break;

            }
        }
    }

    void OnCombatStop(Unit* /*mTarget*/) override
    {
        hm = 100;
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        Creature* cre = NULL;
        getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "I don't want to go back!");
        getCreature()->PlaySoundToSet(11420);
        cre = spawnCreature(22990, 643.741f, 305.852f, 271.689f, 0.00628f);
    }

    void AIUpdate() override
    {
        if (getCreature()->getHealthPct() <= 85 && hm == 100)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 2; i++)
            {
                cre = spawnCreature(23421, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 85;
        }
        else if (getCreature()->getHealthPct() <= 70 && hm == 85)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 2; i++)
            {
                cre = spawnCreature(23215, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 70;
        }
        else if (getCreature()->getHealthPct() <= 55 && hm == 70)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 2; i++)
            {
                cre = spawnCreature(23216, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 55;
        }
        else if (getCreature()->getHealthPct() <= 40 && hm == 55)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 2; i++)
            {
                cre = spawnCreature(23523, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 40;
        }
        else if (getCreature()->getHealthPct() <= 25 && hm == 40)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 5; i++)
            {
                cre = spawnCreature(23318, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 25;
        }
        else if (getCreature()->getHealthPct() <= 10 && hm == 25)
        {
            Creature* cre = NULL;
            for (uint8_t i = 0; i < 5; i++)
            {
                cre = spawnCreature(23524, getCreature()->GetPosition());
                if (cre)
                    cre->GetAIInterface()->setOutOfCombatRange(30000);
            }
            getCreature()->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, "Abandon all hope! The legion has returned to finish what was begun so many years ago. This time there will be no escape!");
            getCreature()->PlaySoundToSet(10999);
            hm = 10;
        }
    }

protected:

    int hm;
};


// Illidan Stormrage Encounter
// Percent done: 85% - 90+%
// Scripted by: M4ksiu
// Script is totally private and none of outside forces were involved in its creation
// It will remain like that until author decides what to do to not give people opportunity to take credits for it
// For more informations ask script author

/*
 * TO DO List:
 * 1) Fix Maiev positioning during Demon Form encounter                                            *DONE*                +
 * 2) Fix Maiev weapon wielding during dialogs and fight                                           *PARTLY DONE*         *
 * 3) Fix Illidan weapon appearing during Shadow Demons summoning                                  *DONE*                +
 * 4) Fix not casting spells by Illidan in phases 1, 3                                             *DONE*                +
 * 5) Add demon cursing after Akama runs away                                                      *DONE*                +
 * 6) Fix trigger channeling during trap execution                                                                       *
 * 7) Maiev yelling when Illidan is caged                                                          *DONE*                +
 * 8) Move Illidan yelling from Maiev class to Illidan's                                           *DONE*                +
 * 9) Add aura for Shadow Demons                                                                   *DONE*                +
 * 10) Recheck texts and their purposes/palces                                                     *PARTLY DONE*         *
 * 11) Adjust Eye Beam movement timers                                                             *DONE*                *
 * 12) Adjust Database data - about speed, damage, health etc.                                     *PARTLY DONE*         +
 * 13) Prevent Maiev from taking aggro of Illidan                                                  *PARTLY*              +
 * 14) Add Immunities                                                                              *HANDLED BY CORE*     +
 * 15) Adjust Enrage timer with trap timer to prevent dealing massive damage                       *DONE*                +
 * 16) Adjust Trap Cage behavior                                                                   *DONE W/O ANIM PROB**
 * 17) Adjust Flame Burst spell and its mechanism                                                  *DONE*                +
 * 18) Adjust Flame Burst timer                                                                    *SHOULD BE DONE*      +
 * 19) Fix Maiev facing during Demon Form encounter                                                *DONE*                *
 * 20) Find Maiev proper knife                                                                                           *
 * 21) Add Flame of Azzinoth check to charge target only by one FoA???                             *UNKNOWN SITUATION*   *
 * 22) Add 'snow' that appears in Demon phase                                                                            *
 * 23) Fix eventual animation problems                                                                                   *
 * 24) Fix problem when sometimes trap catches Illidan too far and rest of it can't be exe.        *SHOULD BE FIXED*     +
 * 25) Check if Parasitic Shadowfiends should attack random target after appearing                 *DONE*                +
 * 26) Inspect why Cage Trap crashes server    so often (mostly on spawning or taking Gameobject)  *FIXED?*              +
 * 27) Fix trigger aggroing - they stop in place                                                   *INVALID?*            +
 * 28) Fix Akama not appearing in death scene                                                      *DONE*                +
 * 29) Change mechanics of Blaze effect (should be spawned after using that Flame Blast spell      *DONE*                +
 * 30) Check core mess with multi-npc spawning by spell
 * 31) Fix triggers falling through floor or not being spawned correctly                           *DONE FOR EYE BLAST**
 * 32) Check what happens with Akama when he runs away and when he should exactly come back
 * 33) Check timer of Shadow Demon spawning (according to WoWWiki.com)                             *COULDN'T FIND INFO**
 */

// Structures

struct Transformation
{
    uint32_t mTimer;
    uint32_t mEmote;
    uint32_t mEmoteType;
    const char* mText;
    uint32_t mSoundId;
    uint32_t mAura;
    uint32_t mUnAura;
    bool mEquipWeapons;
};

// Transformations
// Emote Types - 0 (Oneshot), 1 (Emotestate)

static Transformation Ascend[] =
{
    { 250 * 5, 403, 1, "", 0, 0, 0, false },
    { 250 * 8, 404, 1, "Behold the power... of the demon within!", 11475, 0, 0, false },
    { 250 * 20, 404, 1, "", 0, 40506, 0, false },
    { 250 * 10, 405, 1, "", 0, 0, 0, false },
    { 250 * 10, 405, 1, "", 0, 0, 0, false },                 // additional code inside function
    { 250 * 8, 0, 1, "", 0, 0, 0, false },                    // additional code inside function
};

static Transformation Descend[] =
{
    { 250 * 7, 403, 1, "", 0, 0, 0, false },
    { 250 * 3, 404, 1, "", 0, 0, 0, false },
    { 250 * 24, 404, 1, "", 0, 0, 40506, false },            // additional code inside function
    { 250 * 6, 405, 1, "", 0, 0, 0, false },
    { 250 * 4, 405, 1, "", 0, 0, 0, true },
    { 250 * 8, 0, 1, "", 0, 0, 0, true },                    // additional code inside function
};

// Paths
// Eye Beam stuff
static Movement::Location EyeBeamPaths[] =
{
    { 642.240601f, 297.297180f, 354.423401f },
    { 641.197449f, 314.330963f, 354.300262f },
    { 657.239807f, 256.925568f, 353.996094f },
    { 657.913330f, 353.562775f, 353.996185f },
    { 706.751343f, 298.312683f, 354.653809f },
    { 706.593262f, 310.011475f, 354.693848f },
    { 706.592407f, 343.425568f, 353.996124f },
    { 707.019043f, 270.441772f, 353.996063f }
};

// Akama stuff
static Movement::Location ToIllidan[] =
{
    {  },
    { 660.248596f, 330.695679f, 271.688110f, 1.243284f },
    { 671.172241f, 353.273193f, 271.689453f, 1.022600f },
    { 694.227783f, 379.461365f, 271.792145f, 6.232135f },
    { 724.159973f, 373.263275f, 282.614349f, 5.324218f },
    { 747.034973f, 335.668274f, 307.047150f, 4.972365f },
    { 755.477234f, 304.194580f, 312.167328f, 6.276120f },
    { 771.809753f, 303.744873f, 313.563507f, 6.265894f },
    // After door event waypoints
    { 778.550232f, 304.515198f, 318.818542f, 0.002354f },
    { 789.564697f, 304.493652f, 319.759583f, 6.248631f },
    { 799.598389f, 295.776642f, 319.760040f, 4.687257f },
    { 799.054016f, 288.826660f, 320.030334f, 4.563174f },
    { 794.595459f, 262.302856f, 341.463715f, 4.500343f },
    { 794.396973f, 256.420471f, 341.463715f, 4.557680f },
    { 783.355957f, 250.460892f, 341.463776f, 3.746361f },
    { 764.988098f, 238.561462f, 353.646484f, 3.324606f },
    { 749.337463f, 236.288681f, 352.997223f, 1.633631f },
    { 751.941528f, 304.626221f, 352.996124f, 3.128243f },
    { 747.236511f, 304.922913f, 352.996155f, 6.278462f },
    { 747.834106f, 362.513977f, 352.996155f, 1.604554f }
};

// Illidan stuff
static Movement::Location ForIllidan[] =
{
    {  },
    { 709.783203f, 305.073883f, 362.988770f, 1.546307f }, // Middle pos
    { 678.703735f, 337.249664f, 362.988770f, 4.741172f }, // Eastern pos
    { 679.968384f, 284.618011f, 362.988770f, 1.603503f }, // Western pos
    { 680.798157f, 305.648743f, 353.192200f, 3.196716f }  // Land
};

static Movement::Location UnitPos[] =
{
    { 676.717346f, 322.445251f, 354.153320f, 5.732623f }, // Blade 1
    { 677.368286f, 285.374725f, 354.242157f, 5.645614f }  // Blade 2
};

uint32_t DoorEventTimers[] =
{
    1000 * 2,
    1000 * 4,
    1000 * 4,
    1000 * 9,
    1000 * 1,
    1000 * 6,
    1000 * 1,
    1000 * 6,
    1000 * 5,
    1000 * 1,
    1000 * 9,
    1000 * 1,
    1000 * 3,
    1000 * 3,
    1000 * 3,
    // After breaking gate
    1000 * 2,
    1000 * 15
};

uint32_t IllidanDialog[] =
{
    1000 * 1,
    1000 * 1,
    1000 * 1,
    1000 * 4,
    1000 * 5,
    1000 * 3,
    1000 * 3,
    1000 * 4,
    1000 * 3,
    1000 * 3,
    1000 * 4,
    1000 * 2,
    1000 * 2,
    1000 * 2,
    1000 * 2
};

uint32_t AkamaEscapeTimers[] =
{
    1000 * 1,
    1000 * 7,
    1000 * 1,
    1000 * 1,
    1000 * 1,
    1000 * 1,
    1000 * 2,
    1000 * 2
};

uint32_t BladeEvent[] =
{
    500 * 8,
    500 * 2,
    500 * 2,
    500 * 4,
    500 * 4,
    // After killing both Azzinoth elementals
    500 * 4,
    500 * 1,
    500 * 4,
    500 * 6
};

uint32_t MaievTimers[] =
{
    1000 * 3,
    1000 * 3,
    1000 * 3,
    1000 * 1,
    1000 * 1,
    1000 * 1,
    1000 * 1,
    1000 * 5,
    1000 * 3,
    1000 * 3,
    1000 * 3,
    1000 * 1,
    1000 * 2//3
};

uint32_t DeathSceneTimers[] =
{
    // Maiev
    1000 * 1,
    1000 * 1,
    1000 * 3,
    1000 * 20,
    1000 * 5,
    1000 * 9,
    1000 * 1,
    1000 * 1,
    // Akama
    1000 * 2,
    1000 * 4,
    1000 * 4
};

class UnselectableTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(UnselectableTriggerAI)
    explicit UnselectableTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
    }
};

// Generic Trigger AI

class GenericTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GenericTriggerAI)
    explicit GenericTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setRooted(true);
        getCreature()->m_noRespawn = true;

        mSpellId = mDespawnTimer = 0;
        uint32_t AIUpdate = 2000;
        bool OnSpawn = false;
        switch (getCreature()->getEntry())
        {
            case CN_DEMON_FIRE:
                OnSpawn = true;
                mSpellId = DEMON_FIRE;
                mDespawnTimer = DEMON_FIRE_DESPAWN;
                break;
            case CN_BLAZE_EFFECT:
                AIUpdate = 4000;                        // to recheck
                mSpellId = BLAZE_EFFECT;
                mDespawnTimer = BLAZE_EFFECT_DESPAWN;
                break;
            case CN_FLAME_CRASH:
                mSpellId = FLAME_CRASH_EFFECT;
                mDespawnTimer = FLAME_CRASH_EFFECT_DESPAWN;
                break;
            case CN_FLAME_BURST:
                mSpellId = FLAME_BURST;
                mDespawnTimer = FLAME_BURST_DESPAWN;
                break;
        }

        if (mSpellId == 0)
            return;

        if (OnSpawn)
        {
            getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
            _applyAura(mSpellId);
            _setMeleeDisabled(false);
            despawn(mDespawnTimer, 0);
        }
        else
        {
            getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
            RegisterAIUpdateEvent(AIUpdate);
        }
    }

    void AIUpdate() override
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        _applyAura(mSpellId);
        _setMeleeDisabled(false);
        despawn(mDespawnTimer, 0);
        RemoveAIUpdateEvent();

        if (getCreature()->getEntry() == CN_FLAME_BURST)
        {
            getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        }
    }

    uint32_t mDespawnTimer;
    uint32_t mSpellId;
};

class EyeBeamTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(EyeBeamTriggerAI)
    explicit EyeBeamTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        getCreature()->GetAIInterface()->setAiState(AI_STATE_SCRIPTMOVE);
        getCreature()->m_noRespawn = true;

        _setMeleeDisabled(false);
        RegisterAIUpdateEvent(1000);

        mPosition = -1;
    }

    void AIUpdate() override
    {
        if (mPosition == -1)
        {
            despawn(10000, 0);

            mPosition = -2;
        }
        else if (mPosition != -2)
        {
            moveTo(EyeBeamPaths[7 - mPosition].x, EyeBeamPaths[7 - mPosition].y, EyeBeamPaths[7 - mPosition].z, false);

            mPosition = -2;
        }

        _applyAura(EYE_BLAST);
    }

    int32_t    mPosition;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Other creature AIs

class ShadowDemonAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ShadowDemonAI)
    explicit ShadowDemonAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mParalyze = addAISpell(SHADOW_DEMON_PARALYZE, 0.0f, TARGET_ATTACKING);
        mConsumeSoul = addAISpell(SHADOW_DEMON_CONSUME_SOUL, 0.0f, TARGET_ATTACKING);
        getCreature()->m_noRespawn = true;

        Unit* pTarget = getBestPlayerTarget();
        if (pTarget != NULL)
        {
            getCreature()->GetAIInterface()->AttackReaction(pTarget, 200000);
            _setTargetToChannel(pTarget, SHADOW_DEMON_PURPLE_BEAM);
            _castAISpell(mParalyze);
            _applyAura(SHADOW_DEMON_PASSIVE);
            RegisterAIUpdateEvent(1000);
        }
        else
        {
            despawn(1);
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        if (_getTargetToChannel() != NULL)
        {
            Unit* pUnit = _getTargetToChannel();
            pUnit->RemoveAura(SHADOW_DEMON_PARALYZE);
        }
    }

    void OnTargetDied(Unit* /*pTarget*/) override
    {
        despawn(2000, 0);
    }

    void AIUpdate() override
    {
        // Ugly code :P
        Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
        if (pTarget != NULL && _getTargetToChannel() != NULL && pTarget == _getTargetToChannel())
        {
            if (getRangeToObject(pTarget) <= 8.0f)
            {
                pTarget->RemoveAura(SHADOW_DEMON_PARALYZE);
                _castAISpell(mConsumeSoul);

                RemoveAIUpdateEvent();
                despawn(500, 0);
                return;
            }
        }
        else
        {
            despawn(500, 0);
        }
    }

    CreatureAISpells* mParalyze;
    CreatureAISpells* mConsumeSoul;
};

class ParasiticShadowfiendAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(ParasiticShadowfiendAI)
    explicit ParasiticShadowfiendAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(PARASITIC_SHADOWFIEND_WITH_DAMAGE, 0.0f, TARGET_ATTACKING);

        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        _applyAura(PARASITIC_SHADOWFIEND_PASSIVE);
        getCreature()->m_noRespawn = true;
        _setScale(0.0f);

        Unit* pTarget = getBestPlayerTarget(TargetFilter_Closest);
        if (pTarget != NULL)
        {
            getCreature()->GetAIInterface()->SetUnitToFollow(pTarget);
            getCreature()->GetAIInterface()->SetUnitToFollowAngle(2.0f);
            RegisterAIUpdateEvent(11000);
        }
        else
        {
            despawn(1);
        }
    }
};

class SCRIPT_DECL AkamaGossip : public GossipScript
{
public:

    void onHello(Object* pObject, Player* pPlayer) override
    {
        Creature* pAIOwner = static_cast<Creature*>(pObject);
        if (pAIOwner->GetScript() == NULL)
            return;

        CreatureAIScript* pAI = static_cast< CreatureAIScript* >(pAIOwner->GetScript());

        if (pAI->GetCurrentWaypoint() >= 10)
        {
            GossipMenu menu(pObject->getGuid(), 229902);
            menu.addItem(GOSSIP_ICON_CHAT, 444, 2);     // We're ready to face Illidan.
            menu.sendGossipPacket(pPlayer);
        }
        else
        {
            GossipMenu menu(pObject->getGuid(), 229901);
            menu.addItem(GOSSIP_ICON_CHAT, 445, 1);     // I'm ready, Akama.
            menu.sendGossipPacket(pPlayer);
        }
    }

    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*EnteredCode*/, uint32_t /*gossipId*/) override
    {
        Creature* pAIOwner = static_cast<Creature*>(pObject);
        if (pAIOwner->GetScript() == NULL)
                return;

        CreatureAIScript* pAI = static_cast< CreatureAIScript* >(pAIOwner->GetScript());
        switch (Id)
        {
            case 1:
                pAIOwner->setNpcFlags(UNIT_NPC_FLAG_NONE);
                pAI->ForceWaypointMove(1);
                break;
            case 2:
                pAIOwner->setNpcFlags(UNIT_NPC_FLAG_NONE);
                pAI->ForceWaypointMove(17);
                pAI->_setWieldWeapon(false);
                break;
        }
        GossipMenu::senGossipComplete(pPlayer);
    }
};

class AkamaAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(AkamaAI)
    explicit AkamaAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto blessingOfKings = addAISpell(AKAMA_BLESSING_OF_KINGS, 15.0f, TARGET_SELF, 0, 60);
        blessingOfKings->setAvailableForScriptPhase({ 2 });

        mHealingPotion = addAISpell(AKAMA_HEALING_POTION, 0.0f, TARGET_SELF);
        mHealingPotion->addEmote("I will not last much longer!", CHAT_MSG_MONSTER_YELL, 11385);
        mHealingPotion->addEmote("No! Not yet!", CHAT_MSG_MONSTER_YELL, 11386);

        mDespawn = addAISpell(AKAMA_DESPAWN, 0.0f, TARGET_SELF, 0, 0);

        addEmoteForEvent(Event_OnTargetDied, 8909);
        addEmoteForEvent(Event_OnTargetDied, 8910);
        addEmoteForEvent(Event_OnDied, 8911);

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setCanEnterCombat(false);
        setScriptPhase(1);

        for (uint8_t i = 1; i < AKAMA_WAYPOINT_SIZE; ++i)
        {
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_RUN, ToIllidan[i]));
        }

        getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
        getCreature()->setDualWield(true);

        mUdaloAI = mOlumAI = NULL;
        mIllidanAI = NULL;
        mTimeLeft = mScenePart = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _setWieldWeapon(true);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _setWieldWeapon(false);
    }

    void AIUpdate() override
    {
        switch (getScriptPhase())
        {
            case 1:
                WayToIllidan();
                break;
            case 2:
                FightWithIllidan();
                break;
            case 3:
                EncounterEnd();
                break;
        }
    }

    void ToIllidanEvent()
    {
        if (mScenePart > 6 && mScenePart < 15 && (mUdaloAI == NULL || mOlumAI == NULL))
        {
            mScenePart = -1;
        }

        GameObject* pGate = getNearestGameObject(774.7f, 304.6f, 314.85f, 185905);
        Unit* pDoorTrigger = getNearestCreature(771.5f, 304.7f, 319.0f, CN_DOOR_EVENT_TRIGGER);
        if (mScenePart <= 15 && pGate == NULL || mScenePart == -1)
        {
            sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "It's strange that Illidan doesn't protect himself against intruders.");
            _unsetTargetToChannel();
            ForceWaypointMove(7);

            if (mUdaloAI != NULL)
                _unsetTargetToChannel();

            if (mOlumAI != NULL)
                _unsetTargetToChannel();

            RemoveAIUpdateEvent();
            mUdaloAI = mOlumAI = NULL;
            mTimeLeft = DoorEventTimers[15];
            mScenePart = 16;
            return;
        }

        mTimeLeft -= GetAIUpdateFreq();
        if (mTimeLeft > 0)
            return;

        switch (mScenePart)
        {
            case 1:
                getCreature()->SetFacing(6.248631f);
                break;
            case 2:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "The door is all that stands between us and the Betrayer. Stand aside, friends.");
                getCreature()->Emote(EMOTE_ONESHOT_TALK);
                break;
            case 3:
                if (pDoorTrigger != NULL)
                {
                    _applyAura(AKAMA_DOOR_FAIL);
                    _setTargetToChannel(pDoorTrigger, AKAMA_DOOR_FAIL);
                }
                break;
            case 4:
                _unsetTargetToChannel();
                break;
            case 5:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "I cannot do this alone...");
                getCreature()->Emote(EMOTE_ONESHOT_NO);
                break;
            case 6:        // summoning two spirits to help Akama with breaking doors
                mUdaloAI = spawnCreatureAndGetAIScript(23410, 751.884705f, 311.270050f, 312.121185f, 0.047113f);
                mOlumAI  = spawnCreatureAndGetAIScript(23411, 751.687744f, 297.408600f, 312.124817f, 0.054958f);
                if (mUdaloAI == nullptr || mOlumAI == nullptr)
                {
                    pGate->setState(GO_STATE_OPEN);
                    break;
                }
                break;
            case 7:
                mUdaloAI->sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "You are not alone, Akama.");
                break;
            case 8:
                mOlumAI->sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Your people will always be with you.");
                break;
            case 9:
                if (pDoorTrigger != NULL)
                {
                    _applyAura(AKAMA_DOOR_OPEN);
                    _setTargetToChannel(pDoorTrigger, AKAMA_DOOR_OPEN);
                }
                break;
            case 10:
                if (pDoorTrigger != NULL)
                {
                    mUdaloAI->_applyAura(DEATHSWORN_DOOR_OPEN);
                    mUdaloAI->_setTargetToChannel(pDoorTrigger, DEATHSWORN_DOOR_OPEN);
                    mOlumAI->_applyAura(DEATHSWORN_DOOR_OPEN);
                    mOlumAI->_setTargetToChannel(pDoorTrigger, DEATHSWORN_DOOR_OPEN);
                }
                break;
            case 11:
                pGate->setState(GO_STATE_OPEN);
                if (pDoorTrigger != NULL)
                {
                    pDoorTrigger->castSpell(pDoorTrigger, sSpellMgr.getSpellInfo(GATE_FAILURE), true);
                }
                break;
            case 12:
                _unsetTargetToChannel();
                mUdaloAI->_unsetTargetToChannel();
                mOlumAI->_unsetTargetToChannel();
                break;
            case 13:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "I thank you for your aid, my brothers. Our people will be redeemed!");
                getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                break;
            case 14:
                mUdaloAI->getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                mOlumAI->getCreature()->Emote(EMOTE_ONESHOT_SALUTE);

                mUdaloAI = NULL;
                mOlumAI = NULL;
                break;
            case 15:
                ForceWaypointMove(7);
                RemoveAIUpdateEvent();
                break;
            case 16:
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 11388, "");
                getCreature()->SetFacing(2.113512f);
                break;
            case 17:
                getCreature()->setNpcFlags(UNIT_NPC_FLAG_GOSSIP);
                RemoveAIUpdateEvent();

                mScenePart = 0;
                return;
        }

        mTimeLeft = DoorEventTimers[mScenePart];
        ++mScenePart;
    }

    void IllidanDialogEvent()
    {
        if (mIllidanAI == NULL)
        {
            Creature* pIllidan = getNearestCreature(704.539001f, 305.282013f, 353.919006f, 22917);
            if (pIllidan != NULL && pIllidan->GetScript() != NULL)
            {
                mIllidanAI = static_cast< CreatureAIScript* >(pIllidan->GetScript());
            }
        }

        if (mIllidanAI == NULL || !mIllidanAI->isAlive())
        {
            sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Not this time my friends.");
            _castAISpell(mDespawn);
            despawn(0);
            return;
        }

        mTimeLeft -= GetAIUpdateFreq();
        if (mTimeLeft > 0)
            return;

        switch (mScenePart)
        {
            case 1:
                getCreature()->SetFacing(3.126680f);
                break;
            case 2:
                mIllidanAI->_removeAura(39656);
                break;
            case 3:
                mIllidanAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11463, "Akama... your duplicity is hardly surprising. I should have slaughtered you and your malformed brethren long ago.");
                break;
            case 4:
                mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                break;
            case 5:
                mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                break;
            case 6:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11389, "We've come to end your reign, Illidan. My people and all of Outland shall be free!");
                getCreature()->Emote(EMOTE_ONESHOT_POINT);
                break;
            case 7:
                getCreature()->Emote(EMOTE_ONESHOT_TALK);
                break;
            case 8:
                getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                break;
            case 9:
                mIllidanAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11464, "Boldly said. But I remain unconvinced.");
                mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                break;
            case 10:
                mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                break;
            case 11:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11380, "The time has come! The moment is at hand!");
                getCreature()->Emote(EMOTE_ONESHOT_SHOUT);
                break;
            case 12:
                _setWieldWeapon(true);
                getCreature()->Emote(EMOTE_ONESHOT_ROAR);
                break;
            case 13:
                mIllidanAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11466, "You are not prepared!");
                mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL05);

                getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                break;
            case 14:
                mIllidanAI->_setWieldWeapon(true);
                break;
            case 15:
                GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
                GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
                if (pRightGate != NULL)
                {
                    pRightGate->setState(GO_STATE_CLOSED);
                }
                if (pLeftGate != NULL)
                {
                    pLeftGate->setState(GO_STATE_CLOSED);
                }

                setRooted(false);
                setCanEnterCombat(true);
                setAIAgent(AGENT_NULL);
                SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                SetWaypointToMove(0);
                RemoveAIUpdateEvent();
                getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);

                getCreature()->GetAIInterface()->AttackReaction(mIllidanAI->getCreature(), 1, 0);

                mIllidanAI->setCanEnterCombat(true);
                mIllidanAI->setRooted(false);

                Unit* pTarget = FindClosestTargetToUnit(mIllidanAI->getCreature());
                if (pTarget == NULL)
                {
                    pTarget = getCreature();
                }

                mIllidanAI->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500, 0);

                mScenePart = 0;
                setScriptPhase(2);
                return;
        }

        mTimeLeft = IllidanDialog[mScenePart];
        ++mScenePart;
    }

    void WayToIllidan()
    {
        if (mScenePart != 0)
        {
            if (GetCurrentWaypoint() == 6 || GetCurrentWaypoint() == 16)
            {
                ToIllidanEvent();
            }
            else if (GetCurrentWaypoint() == 17)
            {
                IllidanDialogEvent();
            }
        }
    }

    void FightWithIllidan()
    {
        if (_getHealthPercent() <= 15 && !_isCasting())
        {
            _castAISpell(mHealingPotion);
        }
        if (mIllidanAI != NULL && !mIllidanAI->isAlive())
        {
            mIllidanAI = NULL;
        }
        if (mScenePart <= 2)
        {
            if (mIllidanAI == NULL)
            {
                sendChatMessage(CHAT_MSG_MONSTER_SAY, 0, "Not this time my friends.");
                if (!_isCasting())
                {
                    _castAISpell(mDespawn);
                }

                despawn(0);
                return;
            }
            else if (mIllidanAI->getCreature()->GetAIInterface()->isFlying())
            {
                setCanEnterCombat(false);
                getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
                // azolex to prevent compile error
                uint32_t nullfix = 0;
                getCreature()->GetAIInterface()->setNextTarget(nullfix);
                getCreature()->GetAIInterface()->WipeTargetList();
                getCreature()->GetAIInterface()->WipeHateList();
                _setMeleeDisabled(false);
                setRooted(true);
                mIllidanAI->getCreature()->GetAIInterface()->RemoveThreatByPtr(getCreature());
            }
            else
            {
                getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                
            }
        }
        if (mScenePart == 0 && mIllidanAI != NULL && mIllidanAI->_getHealthPercent() <= 85)
        {
            mTimeLeft = AkamaEscapeTimers[0];
            mScenePart = 1;
        }
        if (GetCurrentWaypoint() < 18 && mScenePart != 0)
        {
            mTimeLeft -= GetAIUpdateFreq();
            if (mTimeLeft > 0)
                return;

            switch (mScenePart)
            {
                case 1:
                    mIllidanAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11465, "Come, my minions. Deal with this traitor as he deserves!");
                    break;
                case 2:
                    if (canEnterCombat())
                    {
                        setCanEnterCombat(false);
                        getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
                        // ugly code, trows compile error if left just null, this should do it ~ azolex
                        uint32_t nullfix = 0;
                        getCreature()->GetAIInterface()->setNextTarget(nullfix);
                        getCreature()->GetAIInterface()->WipeTargetList();
                        getCreature()->GetAIInterface()->WipeHateList();
                        _setMeleeDisabled(false);
                        setRooted(true);

                        mIllidanAI->getCreature()->GetAIInterface()->RemoveThreatByPtr(getCreature());
                    }

                    mIllidanAI = NULL;
                    break;
                case 3:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11390, "I will deal with these mongrels! Strike now, friends! Strike at the Betrayer!");
                    getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                    break;
                case 4:
                    getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                    break;
                case 5:
                    getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                    break;
                case 6:
                    _setWieldWeapon(false);
                    getCreature()->Emote(EMOTE_ONESHOT_EXCLAMATION);
                    break;
                case 7:
                    getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                    _setWieldWeapon(true);
                    break;
                case 8:
                    setRooted(false);
                    ForceWaypointMove(18);
                    RemoveAIUpdateEvent();
                    //_unit->setEmoteState(EMOTE_ONESHOT_NONE);

                    mScenePart = 0;
                    return;
                    break;
            }

            mTimeLeft = AkamaEscapeTimers[mScenePart];
            ++mScenePart;
        }
    }

    void EncounterEnd()
    {
        mTimeLeft -= GetAIUpdateFreq();
        if (mTimeLeft > 0)
            return;

        switch (mScenePart)
        {
            case 1:
                {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11387, "The Light will fill these dismal halls once again. I swear it.");

                    Unit* pIllidan = getNearestCreature(22917);
                    if (pIllidan != NULL)
                    {
                        getCreature()->GetAIInterface()->setNextTarget(pIllidan);
                    }

                    getCreature()->Emote(EMOTE_ONESHOT_TALK);
                }
                break;
            case 2:
                getCreature()->Emote(EMOTE_ONESHOT_SALUTE);
                break;
            case 3:
                RemoveAIUpdateEvent();
                _castAISpell(mDespawn);
                despawn(0);
                return;
        }

        mTimeLeft = DeathSceneTimers[mScenePart + 8];
        ++mScenePart;
    }

    void OnReachWP(uint32_t iWaypointId, bool /*bForwards*/) override
    {
        switch (iWaypointId)
        {
            case 6:
                RegisterAIUpdateEvent(1000);
                mTimeLeft = DoorEventTimers[0];
                mScenePart = 1;
                break;
            case 16:
                RegisterAIUpdateEvent(1000);
                break;
            case 17:
                RegisterAIUpdateEvent(1000);
                mTimeLeft = IllidanDialog[0];
                mScenePart = 1;
                break;
            case 19:
                //_unit->m_auracount[SPELL_AURA_MOD_INVISIBILITY] = true;                        // Arc's
                getCreature()->UpdateVisibility();
                break;
            case 20:
                RegisterAIUpdateEvent(1000);
                mTimeLeft = DeathSceneTimers[8];
                mScenePart = 1;
                setScriptPhase(3);
                break;
            default:
                {
                    getCreature()->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    getCreature()->GetAIInterface()->setWayPointToMove(iWaypointId + 1);
                }
        }
    }

    // A bit rewritten FindTarget function
    Unit* FindClosestTargetToUnit(Unit* pSeeker)
    {
        if (pSeeker == nullptr)
            return nullptr;

        Unit* pTarget = nullptr;
        float distance = 70.0f;
        float z_diff;

        for (const auto& itr : getCreature()->getInRangePlayersSet())
        {
            Unit* pUnit = static_cast<Unit*>(itr);

            if (!pUnit || pUnit->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                continue;

            z_diff = fabs(getCreature()->GetPositionZ() - pUnit->GetPositionZ());
            if (z_diff > 5.0f)
                continue;

            if (!pUnit->isAlive())
                continue;

            float dist = pSeeker->GetDistance2dSq(pUnit);

            if (dist > distance * distance)
                continue;

            distance = dist;
            pTarget = pUnit;
        }

        return pTarget;
    }

    // SPells
    CreatureAISpells* mHealingPotion;
    CreatureAISpells* mDespawn;

    // AIs
    CreatureAIScript* mIllidanAI;
    CreatureAIScript* mUdaloAI;
    CreatureAIScript* mOlumAI;

    // Other variables
    int32_t mScenePart;
    uint32_t mTimeLeft;
};

class MaievAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MaievAI)
    explicit MaievAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto strike = addAISpell(MAIEV_SHADOW_STRIKE, 10.0f, TARGET_ATTACKING, 0, 30);
        strike->setAvailableForScriptPhase({ 1 });

        auto throwDagger = addAISpell(MAIEV_THROW_DAGGER, 50.0f, TARGET_ATTACKING, 1, 0);
        throwDagger->setAvailableForScriptPhase({ 2 });

        mTeleport = addAISpell(MAIEV_TELEPORT, 0.0f, TARGET_SELF);
        mTrapSummon = addAISpell(MAIEV_CAGE_TRAP_SUMMON, 0.0f, TARGET_SELF);

        // HACK!
        //\todo to set flags will override all values from db. To add/remove flags use SetFlag(/RemoveFlag(
        getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
        getCreature()->setMaxHealth(1000000);
        getCreature()->setHealth(1000000);
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setScriptPhase(1);

        mYellTimer = mScenePart = mTrapTimer = 0;
        mSummonTrap = false;
        mIllidanAI = NULL;
        mTimeLeft = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _setWieldWeapon(true);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _setWieldWeapon(false);
        if (mIllidanAI != NULL)
        {
            setScriptPhase(3);
            mScenePart = 1;
            mTimeLeft = DeathSceneTimers[0];
            RegisterAIUpdateEvent(1000);
            DeathScene();
        }
        else
        {
            _castAISpell(mTeleport);
            despawn(500, 0);
        }
    }

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override
    {
        getCreature()->setHealth(getCreature()->getMaxHealth());
    }

    void AIUpdate() override
    {
        switch (getScriptPhase())
        {
            case 1:
                FightWithIllidan();
                break;
            case 2:
                FightWithDemon();
                break;
            case 3:
                DeathScene();
                break;
        }
    }

    void FightWithIllidan()
    {
        if (mIllidanAI == NULL)
        {
            _castAISpell(mTeleport);
            RemoveAIUpdateEvent();
            despawn(500, 0);
            return;
        }
        if (mIllidanAI->isAlive())
        {
            if (mIllidanAI->getCreature()->HasAura(40506))
            {
                setScriptPhase(2);
                FightWithDemon();
                return;
            }
            else if (mSummonTrap)
            {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11495, "There shall be no prison for you this time!");
                _castAISpell(mTrapSummon);
                _resetTimer(mYellTimer, _getTimeForTimer(mYellTimer) + (5 + Util::getRandomUInt(10)) * 1000);
                mSummonTrap = false;
                return;
            }
            else if (_isTimerFinished(mTrapTimer))
            {
                Unit* pTarget = getBestPlayerTarget();
                if (pTarget != NULL)
                {
                    _castAISpell(mTeleport);
                    getCreature()->SetPosition(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());        // does it really work?
                    getCreature()->GetAIInterface()->StopMovement(2500);
                }

                _resetTimer(mTrapTimer, (Util::getRandomUInt(5) + 25) * 1000);
                mSummonTrap = true;
                return;
            }
            else if (_isTimerFinished(mYellTimer))
            {
                switch (Util::getRandomUInt(2))
                {
                    case 0:
                        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11494, "Bleed as I have bled!");
                        break;
                    case 1:
                        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11493, "That is for Naisha!");
                        break;
                    case 2:                                                            // Not sure if this one should be here
                        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11500, "Meet your end, demon!");
                        break;
                }

                _resetTimer(mYellTimer, (Util::getRandomUInt(20) + 20) * 1000);
            }
        }
        else
        {
            mTimeLeft = DeathSceneTimers[0];
            mScenePart = 1;
            setScriptPhase(3);
            DeathScene();
        }
    }

    void FightWithDemon()
    {
        if (mIllidanAI == NULL)
        {
            _castAISpell(mTeleport);
            RemoveAIUpdateEvent();
            despawn(500, 0);
            return;
        }
        if (!mIllidanAI->getCreature()->HasAura(40506))
        {
            if (mIllidanAI->isAlive())
            {
                setAIAgent(AGENT_NULL);
                _setDisplayWeapon(true, false);
                _setWieldWeapon(true);
                _resetTimer(mTrapTimer, (Util::getRandomUInt(5) + 20) * 1000);
                setScriptPhase(1);
                FightWithIllidan();
            }
            else
            {
                mTimeLeft = DeathSceneTimers[0];
                mScenePart = 1;
                setScriptPhase(3);
                DeathScene();
            }

            return;
        }
        else if (getRangeToObject(mIllidanAI->getCreature()) < 15.0f)
        {
            getCreature()->GetAIInterface()->_CalcDestinationAndMove(mIllidanAI->getCreature(), 20.0f);
            setAIAgent(AGENT_SPELL);
            return;
        }
        else if (getRangeToObject(mIllidanAI->getCreature()) > 35.0f)
        {
            getCreature()->GetAIInterface()->_CalcDestinationAndMove(mIllidanAI->getCreature(), 30.0f);
            setAIAgent(AGENT_SPELL);
            return;
        }

        _setDisplayWeapon(false, false);
        if (mIllidanAI->getCreature()->getEmoteState() == 0)        // dunno if it's really needed
        {
            
        }

        // Ugly -.-'
        float Facing = getCreature()->calcRadAngle(getCreature()->GetPositionX(), getCreature()->GetPositionY(), mIllidanAI->getCreature()->GetPositionX(), mIllidanAI->getCreature()->GetPositionY());
        if (getCreature()->GetOrientation() != Facing)
        {
            getCreature()->SetFacing(Facing);
        }

        setAIAgent(AGENT_SPELL);
    }

    void DeathScene()
    {
        if (mIllidanAI == NULL && mScenePart <= 6)
        {
            _castAISpell(mTeleport);
            RemoveAIUpdateEvent();
            despawn(500, 0);
            return;
        }
        if (mIllidanAI != NULL && mScenePart <= 6 || mScenePart > 6)
        {
            mTimeLeft -= GetAIUpdateFreq();
            if (mTimeLeft > 0)
                return;

            switch (mScenePart)
            {
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11496, "Ah, it is finished. You are beaten.");

                    mIllidanAI->getCreature()->Emote(EMOTE_ONESHOT_CUSTOMSPELL06);
                    if (mIllidanAI->getCreature()->isCastingSpell())
                        mIllidanAI->getCreature()->interruptSpell();
                    break;
                case 2:
                    mIllidanAI->getCreature()->setEmoteState(EMOTE_ONESHOT_CUSTOMSPELL07);

                    getCreature()->GetAIInterface()->setNextTarget(mIllidanAI->getCreature());
                    break;
                case 3:
                    mIllidanAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11478, "You have won... Maiev. But the huntress... is nothing without the hunt. You... are nothing... without me.");
                    getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                    _setWieldWeapon(false);
                    break;
                case 4:
                    {
                        GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
                        GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
                        if (pRightGate != NULL)
                        {
                            pRightGate->setState(GO_STATE_OPEN);
                        }
                        if (pLeftGate != NULL)
                        {
                            pLeftGate->setState(GO_STATE_OPEN);
                        }

                        mIllidanAI->getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
                        mIllidanAI->getCreature()->setHealth(0);
                        mIllidanAI->getCreature()->setDeathState(JUST_DIED);
                    }
                    break;
                case 5:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11497, "He's right. I feel nothing. I am nothing.");
                    break;
                case 6:
                    {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11498, "Farewell, champions.");
                        Creature* pAkama = getNearestCreature(ToIllidan[19].x, ToIllidan[19].y, ToIllidan[19].z, CN_AKAMA);
                        if (pAkama != NULL)
                        {
                            AkamaAI* pAkamaAI = static_cast< AkamaAI* >(pAkama->GetScript());
                            //pAkama->m_auracount[SPELL_AURA_MOD_INVISIBILITY] = true;                        // Arc's
                            pAkama->UpdateVisibility();
                            if (pAkamaAI->isRooted())
                            {
                                pAkamaAI->setRooted(false);
                            }

                            float IllidanX = mIllidanAI->getCreature()->GetPositionX();
                            float IllidanY = mIllidanAI->getCreature()->GetPositionY();
                            float IllidanZ = mIllidanAI->getCreature()->GetPositionZ();
                            float IllidanO = mIllidanAI->getCreature()->GetOrientation();

                            float Angle = getCreature()->calcAngle(pAkama->GetPositionX(), pAkama->GetPositionY(), IllidanX, IllidanY) * M_PI_FLOAT / 180.0f;
                            float X = 12.0f * cosf(Angle);
                            float Y = 12.0f * sinf(Angle);

                            X -= cosf(IllidanO);
                            Y -= sinf(IllidanO);

                            Movement::Location pCoords;
                            pCoords.x = IllidanX - X;
                            pCoords.y = IllidanY - Y;
                            pCoords.z = IllidanZ;
                            pCoords.o = 0.0f;

                            pAkamaAI->AddWaypoint(CreateWaypoint(20, 0, Movement::WP_MOVE_TYPE_RUN, pCoords));
                            pAkamaAI->ForceWaypointMove(20);
                        }
                    }
                    break;
                case 7:
                    _castAISpell(mTeleport);
                    break;
                case 8:
                    RemoveAIUpdateEvent();
                    despawn(0);
                    return;
                    break;
            }

            mTimeLeft = DeathSceneTimers[mScenePart];
            ++mScenePart;
        }

        else
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11498, "Farewell, champions.");
            despawn(0);
        }
    }

    // AI class pointers
    CreatureAIScript* mIllidanAI;

    // Spell Description pointers
    CreatureAISpells* mTrapSummon;
    CreatureAISpells* mTeleport;

    // Other variables
    bool mSummonTrap;
    int32_t mTrapTimer;
    int32_t mYellTimer;
    uint32_t mScenePart;
    uint32_t mTimeLeft;

};

class IllidanStormrageAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IllidanStormrageAI)
    explicit IllidanStormrageAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //Phase 1 spells
        //AddPhaseSpell(1, AddSpell(ILLIDAN_SHEAR, TARGET_ATTACKING, 12, 1.5f, 15));
        //AddPhaseSpell(1, AddSpell(ILLIDAN_DRAW_SOUL, TARGET_SELF, 6, 1.5f, 40));    // Self-kills are bad =/
        //AddPhaseSpell(1, AddSpell(ILLIDAN_FLAME_CRASH, TARGET_RANDOM_DESTINATION, 12, 1.3f, 25));
        //mParasiticDmg = AddSpell(PARASITIC_SHADOWFIEND_WITH_DAMAGE, TARGET_RANDOM_SINGLE, 0, 0, 0);
        //mParasitic = AddSpellFunc(&SpellFunc_Illidan_Parasitic, TARGET_RANDOM_SINGLE, 0, 0, 0);
        ////Phase 2 spells
        //AddPhaseSpell(2, AddSpell(ILLIDAN_THROW_GLAIVE1, TARGET_RANDOM_DESTINATION, 0, 0.5f, 0));
        //mGlaiveThrow = AddPhaseSpell(2, AddSpell(ILLIDAN_THROW_GLAIVE2, TARGET_SELF, 0, 0.5f, 0));
        //AddPhaseSpell(2, AddSpell(ILLIDAN_FIREBALL, TARGET_RANDOM_SINGLE, 60, 2, 0));
        //AddPhaseSpell(2, AddSpell(ILLIDAN_DARK_BARRAGE, TARGET_RANDOM_SINGLE, 40, 10, 40));
        //mGlaiveReturns = AddPhaseSpell(2, AddSpell(ILLIDAN_GLAIVE_RETURNS, TARGET_SELF, 0, 0.5f, 0));
        ////Phase 3 spells
        //AddPhaseSpell(3, AddSpell(ILLIDAN_SHEAR, TARGET_ATTACKING, 12, 1.5f, 15));
        //AddPhaseSpell(3, AddSpell(ILLIDAN_DRAW_SOUL, TARGET_SELF, 6, 1.5f, 40));    // Self-kills are bad =/
        //AddPhaseSpell(3, AddSpell(ILLIDAN_FLAME_CRASH, TARGET_RANDOM_DESTINATION, 12, 1.3f, 25));
        //AddPhaseSpell(3, AddSpell(ILLIDAN_AGONIZING_FLAMES, TARGET_RANDOM_DESTINATION, 7, 0, 25, 0, 35, true));
        //mShadowPrison = AddPhaseSpell(3, AddSpell(ILLIDAN_SHADOW_PRISON, TARGET_SELF, 0, 0, 0));
        ////Phase 4 spells
        //mShadowBlast = AddPhaseSpell(4, AddSpell(ILLIDAN_SHADOW_BLAST, TARGET_RANDOM_DESTINATION, 0, 2, 0));
        //mFlameBurst = AddPhaseSpell(4, AddSpell(ILLIDAN_FLAME_BURST1, TARGET_SELF, 0, 0, 20));
        //mShadowDemons = AddPhaseSpell(4, AddSpell(ILLIDAN_SUMMON_DEMON, TARGET_SELF, 0, 1.3f, 60));
        ////Phase 5 spells
        //AddPhaseSpell(5, AddSpell(ILLIDAN_SHEAR, TARGET_ATTACKING, 12, 1.5f, 15));
        //AddPhaseSpell(5, AddSpell(ILLIDAN_DRAW_SOUL, TARGET_SELF, 6, 1.5f, 40));    // Self-kills are bad =/
        //AddPhaseSpell(5, AddSpell(ILLIDAN_FLAME_CRASH, TARGET_RANDOM_DESTINATION, 12, 1.3f, 25));
        //AddPhaseSpell(5, AddSpell(ILLIDAN_AGONIZING_FLAMES, TARGET_RANDOM_DESTINATION, 7, 0, 25, 0, 35, true));
        //mEnrage = AddPhaseSpell(5, AddSpell(ILLIDAN_ENRAGE, TARGET_SELF, 0, 1.3f, 0, 0, 0, true, "You've wasted too much time mortals, now you shall fall!", CHAT_MSG_MONSTER_YELL, 11474));

        addEmoteForEvent(Event_OnTargetDied, 8912);
        addEmoteForEvent(Event_OnTargetDied, 8913);

        //mLocaleEnrageSpell = addAISpell(ILLIDAN_BERSERK, 0.0f, TARGET_SELF, 0, 60);

        _applyAura(ILLIDAN_SKULL_INTRO);
        setCanEnterCombat(false);
        _setWieldWeapon(false);
        setRooted(true);
        setFlyMode(false);
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        setScriptPhase(1);

        for (uint8_t i = 1; i < ILLIDAN_WAYPOINT_SIZE; ++i)
        {
            AddWaypoint(CreateWaypoint(i, 0, Movement::WP_MOVE_TYPE_FLY, ForIllidan[i]));
        }

        getCreature()->setBaseAttackTime(RANGED, 1800);
        getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
        getCreature()->setDualWield(true);

        mFoA1 = mFoA2 = NULL;
        mAllow = true;

        mPhaseBackup = 0;
        mScenePart = 0;
        mTimeLeft = 0;
        mParasiticTimer = 0;
        mMovementTimer = 0;
        mFireWallTimer = 0;
        mLastFireWall = 0;
        mMiscEventPart = 0;
        mShadowDemonsTimer = 0;
        mFlameBurstTimer = 0;
        mPlaySound = 0;
        mDemonTimer = 0;
        mYellTimer = 0;
        mEnrageTimer = 0;
        mCurrentWaypoint = 0;
        mLocaleEnrageTimerId = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mLocaleEnrageTimerId = _addTimer(1500000);

        GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
        GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
        if (pRightGate != NULL)
        {
            pRightGate->setState(GO_STATE_CLOSED);
        }
        if (pLeftGate != NULL)
        {
            pLeftGate->setState(GO_STATE_CLOSED);
        }

        getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);

        //mParasitic->mEnabled = false;
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        // General
        getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        _unsetTargetToChannel();
        setCanEnterCombat(true);
        _setWieldWeapon(true);
        _setMeleeDisabled(true);
        setFlyMode(false);
        setRooted(false);

        SetAIUpdateFreq(1000);
        mAllow = true;

        // Phase 2
        if (mFoA1 != NULL)
        {
            mFoA1->despawn(0);
            mFoA1 = NULL;
        }
        if (mFoA2 != NULL)
        {
            mFoA2->despawn(0);
            mFoA2 = NULL;
        }

        Creature* pTrigger = getNearestCreature(677.399963f, 305.545044f, 353.192169f, CN_FACE_TRIGGER);
        if (pTrigger != NULL)
        {
            pTrigger->Despawn(0, 0);
        }
        for (uint8_t i = 0; i < 2; ++i)
        {
            Creature* pBlade = getNearestCreature(UnitPos[i].x, UnitPos[i].y, UnitPos[i].z, CN_BLADE_OF_AZZINOTH);
            if (pBlade != NULL)
            {
                pBlade->setChannelObjectGuid(0);
                pBlade->setChannelSpellId(0);
                pBlade->Despawn(0, 0);
            }
        }

        // Phase 4
        _setDisplayWeapon(true, true);
        _removeAura(ILLIDAN_DEMON_FORM);

        if (isAlive())
        {
            GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
            GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
            if (pRightGate != NULL)
            {
                pRightGate->setState(GO_STATE_OPEN);
            }
            if (pLeftGate != NULL)
            {
                pLeftGate->setState(GO_STATE_OPEN);
            }

            Creature* pMaiev = getNearestCreature(CN_MAIEV);
            if (pMaiev != NULL)
            {
                pMaiev->Despawn(0, 0);
            }
        }
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        Creature* pMaiev = getNearestCreature(CN_MAIEV);
        if (pMaiev != NULL && pMaiev->isAlive())
        {
            getCreature()->setHealth(1);
            setCanEnterCombat(false);
            _applyAura(ILLIDAN_DEATH1);
            _applyAura(ILLIDAN_DEATH2);

            pMaiev->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
            pMaiev->GetAIInterface()->setAiState(AI_STATE_IDLE);
            pMaiev->GetAIInterface()->WipeTargetList();
            pMaiev->GetAIInterface()->WipeHateList();
        }
        else
        {
            GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
            GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
            if (pRightGate != NULL)
            {
                pRightGate->setState(GO_STATE_OPEN);
            }
            if (pLeftGate != NULL)
            {
                pLeftGate->setState(GO_STATE_OPEN);
            }
        }
    }

    // Does not work until it's hooked
    void OnHit(Unit* mVictim, float fAmount) override
    {
        sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "ON HIT1!");
        if (mVictim->isCreature() && (mVictim->getEntry() == CN_MAIEV || mVictim->getEntry() == CN_AKAMA))
        {
            sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "ON HIT2!");
            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget == NULL || !pTarget->isPlayer())
            {
                pTarget = getCreature()->GetAIInterface()->GetSecondHated();
                if (pTarget == NULL || !pTarget->isPlayer())
                {
                    pTarget = getBestPlayerTarget(TargetFilter_Closest);
                    if (pTarget == NULL)
                        return;
                }
            }

            getCreature()->GetAIInterface()->setNextTarget(pTarget);
            getCreature()->GetAIInterface()->modThreatByPtr(pTarget, (int32_t)(fAmount * 2));
            getCreature()->GetAIInterface()->RemoveThreatByPtr(mVictim);
        }
    }

    void OnDamageTaken(Unit* mAttacker, uint32_t fAmount) override
    {
        if (mAttacker->isCreature() && (mAttacker->getEntry() == CN_MAIEV || mAttacker->getEntry() == CN_AKAMA))
        {
            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget == NULL || !pTarget->isPlayer())
            {
                pTarget = getCreature()->GetAIInterface()->GetSecondHated();
            }
            if (pTarget != NULL && pTarget->isPlayer())
            {
                getCreature()->GetAIInterface()->setNextTarget(pTarget);
                getCreature()->GetAIInterface()->modThreatByPtr(pTarget, fAmount * 3);
            }
            else
                return;

            getCreature()->GetAIInterface()->RemoveThreatByPtr(mAttacker);
        }
    }

    void AIUpdate() override
    {
        if (!mAllow)
            return;

        /*if (_isTimerFinished(mLocaleEnrageTimerId))
        {
            castSpell(mLocaleEnrageSpell);
            _removeTimer(mLocaleEnrageTimerId);
        }*/

        switch (getScriptPhase())
        {
            case 1:
                PhaseOne();
                break;
            case 2:
                PhaseTwo();
                break;
            case 3:
                PhaseThree();
                break;
            case 4:
                PhaseFour();
                break;
            case 5:
                PhaseFive();
                break;
            default:
                return;
        }
    }

    void PhaseOne()
    {
        if (_getHealthPercent() <= 85)
        {
            /*if (mParasitic->mEnabled == false)
            {
                mParasitic->mEnabled = true;
                mParasiticTimer = 30000;
            }

            mParasiticTimer -= GetAIUpdateFreq();
            if (mParasiticTimer <= 0)
            {
                CastSpellNowNoScheduling(mParasitic);
                mParasiticTimer = 30000;
                return;
            }*/
        }
        if (getCreature()->GetAIInterface()->isFlying())
        {
            Creature* pAI = spawnCreature(CN_FACE_TRIGGER, 677.399963f, 305.545044f, 353.192169f, 0.0f);
            if (pAI != NULL)
            {
                pAI->m_noRespawn = true;

                getCreature()->GetAIInterface()->setNextTarget(pAI);
            }

            setScriptPhase(2);
            ForceWaypointMove(1);
            SetAIUpdateFreq(500);

            mTimeLeft = BladeEvent[0];
            mScenePart = 1;
            mAllow = false;
            return;
        }
        else if (_getHealthPercent() <= 65 && !_isCasting())
        {
            Creature* pAkama = getNearestCreature(CN_AKAMA);
            if (pAkama != NULL && pAkama->GetScript() != NULL)
            {
                AkamaAI* pAkamaAI = static_cast< AkamaAI* >(pAkama->GetScript());
                if (pAkamaAI->mScenePart <= 2 && pAkamaAI->canEnterCombat())
                {
                    pAkamaAI->setCanEnterCombat(false);
                    pAkamaAI->getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
                    // ugly code, trows compile error if left just null, this should do it ~ azolex
                    uint32_t nullfix = 0;
                    pAkamaAI->getCreature()->GetAIInterface()->setNextTarget(nullfix);
                    pAkamaAI->getCreature()->GetAIInterface()->WipeTargetList();
                    pAkamaAI->getCreature()->GetAIInterface()->WipeHateList();
                    pAkamaAI->_setMeleeDisabled(false);
                    pAkamaAI->setRooted(true);

                    getCreature()->GetAIInterface()->RemoveThreatByPtr(pAkamaAI->getCreature());
                }
            }

            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11479, "I will not be touched by rabble such as you!");
            moveTo(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ() + 10.0f, false);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            setFlyMode(true);

            getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);
            getCreature()->Emote(EMOTE_ONESHOT_LIFTOFF);

            mFireWallTimer = 30000;
            mMovementTimer = 40000;
            mMiscEventPart = 0;
            mLastFireWall = -1;
            return;
        }

        getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
        
    }

    void PhaseTwo()
    {
        if (mScenePart != 0)
        {
            mTimeLeft -= GetAIUpdateFreq();
            if (mTimeLeft != 0)
                return;

            switch (mScenePart - 1)
            {
                case 0:
                    {
                    /*sendChatMessage(CHAT_MSG_MONSTER_YELL, 11480, "Behold the flames of Azzinoth!");
                        if (mGlaiveThrow != NULL)
                        {
                            CastSpellNowNoScheduling(mGlaiveThrow);
                        }*/
                    }
                    break;
                case 1:
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        getCreature()->castSpellLoc(LocationVector(UnitPos[i].x, UnitPos[i].y, UnitPos[i].z), sSpellMgr.getSpellInfo(ILLIDAN_THROW_GLAIVE1), false);
                    }
                    _setWieldWeapon(false);
                    break;
                case 2:
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        Creature* pBlade = spawnCreature(CN_BLADE_OF_AZZINOTH, UnitPos[i].x, UnitPos[i].y, UnitPos[i].z, UnitPos[i].o);
                        if (pBlade != NULL)
                        {
                            pBlade->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                            pBlade->m_noRespawn = true;
                        }
                    }
                    break;
                case 3:
                    mFoA1 = spawnCreatureAndGetAIScript(22997, 672.039246f, 326.748322f, 354.206390f, 0.207343f);
                    mFoA2 = spawnCreatureAndGetAIScript(22997, 673.008667f, 283.813660f, 354.267548f, 6.203853f);
                    if (mFoA1 != nullptr)
                    {
                        Unit* pBlade = getNearestCreature(UnitPos[0].x, UnitPos[0].y, UnitPos[0].z, CN_BLADE_OF_AZZINOTH);
                        if (pBlade != nullptr)
                        {
                            pBlade->setChannelObjectGuid(mFoA1->getCreature()->getGuid());
                            pBlade->setChannelSpellId(TEAR_OF_AZZINOTH_CHANNEL);
                        }
                    }
                    if (mFoA2 != nullptr)
                    {
                        Unit* pBlade = getNearestCreature(UnitPos[1].x, UnitPos[1].y, UnitPos[1].z, CN_BLADE_OF_AZZINOTH);
                        if (pBlade != nullptr)
                        {
                            pBlade->setChannelObjectGuid(mFoA2->getCreature()->getGuid());
                            pBlade->setChannelSpellId(TEAR_OF_AZZINOTH_CHANNEL);
                        }
                    }
                    getCreature()->addUnitFlags(UNIT_FLAG_NON_ATTACKABLE);
                    break;
                case 4:
                    {
                        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                        SetAIUpdateFreq(1000);
                        mScenePart = 0;

                        getCreature()->GetAIInterface()->setNextTarget(getBestPlayerTarget(TargetFilter_Closest));
                        return;
                    }
                    break;
                case 5:
                    for (uint8_t i = 0; i < 2; ++i)
                    {
                        Creature* Blade = getNearestCreature(UnitPos[i].x, UnitPos[i].y, UnitPos[i].z, CN_BLADE_OF_AZZINOTH);
                        if (Blade != NULL)
                        {
                            Blade->Despawn(0, 0);
                        }
                    }
                    break;
                case 6:
                    {
                        /*if (mGlaiveReturns != NULL)
                            CastSpellNowNoScheduling(mGlaiveReturns);*/

                        _applyAura(ILLIDAN_RAKE);
                        _removeAura(ILLIDAN_RAKE);
                        _setWieldWeapon(true);
                    }
                    break;
                case 7:
                    setFlyMode(false);
                    getCreature()->Emote(EMOTE_ONESHOT_LAND);
                    break;
                case 8:
                    {
                        Creature* pTrigger = getNearestCreature(677.399963f, 305.545044f, 353.192169f, CN_FACE_TRIGGER);
                        if (pTrigger != NULL)
                        {
                            pTrigger->Despawn(0, 0);
                        }

                        getCreature()->GetAIInterface()->setNextTarget(getBestPlayerTarget(TargetFilter_Closest));
                        getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                        setCanEnterCombat(true);
                        _setMeleeDisabled(true);
                        setRooted(false);
                        _clearHateList();
                        setScriptPhase(3);
                        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);

                        SetAIUpdateFreq(1000);

                        mParasiticTimer = 30000;
                        mShadowDemonsTimer = (Util::getRandomUInt(25) + 15) * 1000;
                        mDemonTimer = 40000;
                        mPhaseBackup = 3;
                        mScenePart = 0;
                        mPlaySound = false;
                        return;
                    }
                    break;
            }

            mTimeLeft = BladeEvent[mScenePart];
            ++mScenePart;
            return;
        }

        mMovementTimer -= GetAIUpdateFreq();
        mFireWallTimer -= GetAIUpdateFreq();
        if (mFoA1 != NULL)
        {
            if (!mFoA1->isAlive())
            {
                Unit* pBlade = getNearestCreature(UnitPos[0].x, UnitPos[0].y, UnitPos[0].z, CN_BLADE_OF_AZZINOTH);
                if (pBlade != NULL)
                {
                    pBlade->setChannelObjectGuid(0);
                    pBlade->setChannelSpellId(0);
                }

                mFoA1 = NULL;
            }
        }
        if (mFoA2 != NULL)
        {
            if (!mFoA2->isAlive())
            {
                Unit* pBlade = getNearestCreature(UnitPos[1].x, UnitPos[1].y, UnitPos[1].z, CN_BLADE_OF_AZZINOTH);
                if (pBlade != NULL)
                {
                    pBlade->setChannelObjectGuid(0);
                    pBlade->setChannelSpellId(0);
                }

                mFoA2 = NULL;
            }
        }
        if (getCreature()->GetAIInterface()->getWaypointScriptType() != Movement::WP_MOVEMENT_SCRIPT_WANTEDWP && !_isCasting())
        {
            if (getCreature()->getChannelSpellId() == 0)
            {
                if (mFoA1 == NULL && mFoA2 == NULL)
                {
                    Unit* pTrigger = getNearestCreature(677.399963f, 305.545044f, 353.192169f, CN_FACE_TRIGGER);
                    if (pTrigger != NULL)
                    {
                        getCreature()->GetAIInterface()->setNextTarget(pTrigger);
                    }

                    ForceWaypointMove(4);
                    SetAIUpdateFreq(500);

                    mTimeLeft = BladeEvent[5];
                    mScenePart = 6;
                    mAllow = false;
                    return;
                }
                else if (mMovementTimer <= 0)
                {
                    if (Util::getRandomUInt(1) == 1)    // Move right
                    {
                        ++mCurrentWaypoint;
                        if (mCurrentWaypoint > 3)
                            mCurrentWaypoint = 1;
                    }
                    else                    // Move left
                    {
                        --mCurrentWaypoint;
                        if (mCurrentWaypoint < 1)
                            mCurrentWaypoint = 3;
                    }

                    ForceWaypointMove(mCurrentWaypoint);

                    mMovementTimer = 50000;
                    mAllow = false;
                    return;
                }
            }
            if (mMiscEventPart != 0)
            {
                if (mMiscEventPart == 1)
                {
                    uint32_t FireWall = Util::getRandomUInt(7);
                    while((int)FireWall == mLastFireWall || (int)FireWall == 7 - mLastFireWall)
                    {
                        FireWall = Util::getRandomUInt(7);
                    }

                    Creature* pTrigger = spawnCreature(CN_EYE_BEAM_TRIGGER, EyeBeamPaths[FireWall].x, EyeBeamPaths[FireWall].y, EyeBeamPaths[FireWall].z, EyeBeamPaths[FireWall].o);
                    if (pTrigger != NULL && pTrigger->GetScript() != NULL)
                    {
                        sendChatMessage(CHAT_MSG_MONSTER_YELL, 11481, "Stare into the eyes of the Betrayer!");
                        _setTargetToChannel(pTrigger, ILLIDAN_EYE_BLAST1);
                        getCreature()->castSpell(pTrigger, ILLIDAN_EYE_BLAST1, true);
                        getCreature()->GetAIInterface()->setNextTarget(pTrigger);

                        float Distance = pTrigger->CalcDistance(EyeBeamPaths[7 - FireWall].x, EyeBeamPaths[7 - FireWall].y, EyeBeamPaths[7 - FireWall].z);
                        uint32_t TimeToReach = (uint32_t)(Distance * 1000 / pTrigger->getSpeedRate(TYPE_WALK, true));
                        EyeBeamTriggerAI* pEyeBeamTriggerAI = static_cast< EyeBeamTriggerAI* >(pTrigger->GetScript());
                        pEyeBeamTriggerAI->mPosition = FireWall;
                        pEyeBeamTriggerAI->despawn(TimeToReach + 1500, 0);
                        mFireWallTimer = TimeToReach + 1000;
                        mLastFireWall = FireWall;
                        mMiscEventPart = 2;
                    }
                    else
                    {
                        mFireWallTimer = 30000;
                        mMiscEventPart = 0;
                        mLastFireWall = -1;
                    }

                    return;
                }
                else if (mMiscEventPart == 2 && mFireWallTimer <= 0)
                {
                    _unsetTargetToChannel();
                    _removeAura(ILLIDAN_EYE_BLAST1);
                    getCreature()->GetAIInterface()->setNextTarget(getBestPlayerTarget(TargetFilter_Closest));

                    mFireWallTimer = 30000;
                    mMiscEventPart = 0;
                    return;
                }
            }
            else if (mFireWallTimer <= 0)
            {
                mMiscEventPart = 1;
                return;
            }

        }
    }

    void Transform(Transformation* pTransformation, uint32_t pMaxPart)
    {
        mTimeLeft -= GetAIUpdateFreq();
        if (mTimeLeft > 0)
            return;

        // Ugly ass code
        if (pTransformation[mMiscEventPart - 1].mEmoteType == 0)
            getCreature()->Emote((EmoteType)pTransformation[mMiscEventPart - 1].mEmote);
        else
            getCreature()->setEmoteState(pTransformation[mMiscEventPart - 1].mEmote);
        sendChatMessage(CHAT_MSG_MONSTER_YELL, pTransformation[mMiscEventPart - 1].mSoundId, pTransformation[mMiscEventPart - 1].mText);
        _applyAura(pTransformation[mMiscEventPart - 1].mAura);
        _removeAura(pTransformation[mMiscEventPart - 1].mUnAura);
        _setWieldWeapon(pTransformation[mMiscEventPart - 1].mEquipWeapons);

        if (pTransformation == Ascend)
        {
            if (mMiscEventPart == 5)
            {
                _clearHateList();
                Unit* pTarget = getBestPlayerTarget();
                if (pTarget != NULL)
                {
                    getCreature()->GetAIInterface()->AttackReaction(pTarget, 5000);
                }
                setAIAgent(AGENT_SPELL);
                stopMovement();                    // readding settings after target switch
                setScriptPhase(mPhaseBackup);            // without it he gets back to phase 1 and then immediatly to 2

                if (mPlaySound)
                {
                    switch (Util::getRandomUInt(2))
                    {
                        case 0:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11469, "You know nothing of power!");
                            mPlaySound = false;
                            break;
                        case 1:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11471, "Such arrogance!");
                            mPlaySound = false;
                            break;
                    }
                }
            }
            else if (mMiscEventPart == 6)
            {
                if (mPlaySound)
                {
                    switch (Util::getRandomUInt(2))
                    {
                        case 0:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11469, "You know nothing of power!");
                            mPlaySound = false;
                            break;
                        case 1:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11471, "Such arrogance!");
                            mPlaySound = false;
                            break;
                    }
                }

                _setDisplayWeapon(false, false);
                SetAIUpdateFreq(1000);

                mShadowDemonsTimer = (Util::getRandomUInt(25) + 15) * 1000;
                mFlameBurstTimer = (Util::getRandomUInt(7) + 8) * 1000;
                mDemonTimer = 60000;
                mMiscEventPart = 0;

                setScriptPhase(4);
                PhaseFour();
                return;
            }
        }
        else if (pTransformation == Descend)
        {
            if (mMiscEventPart == 3)
            {
                _clearHateList();
                setScriptPhase(4);            // without it he gets back to phase 1 and then immediatly to 2
            }
            else if (mMiscEventPart == 6)
            {
                setAIAgent(AGENT_NULL);
                setScriptPhase(mPhaseBackup);
                if (mPhaseBackup == 5)
                {
                    mEnrageTimer = Util::getRandomUInt(5) + 25;
                }

                SetAIUpdateFreq(1000);

                mParasiticTimer = 30000;
                mEnrageTimer = 25000;
                mDemonTimer = 60000;
                mYellTimer = 25000;
                mMiscEventPart = 0;
                return;
            }
        }

        ++mMiscEventPart;
        if (mMiscEventPart > pMaxPart)
            return;

        mTimeLeft = pTransformation[mMiscEventPart - 1].mTimer;
    }

    bool SpawnMaiev()    // this doesn't have collision checks! so keep in mind that Maiev can be spawned behind walls!
    {
        float xchange  = Util::getRandomFloat(15.0f);
        float distance = 15.0f;

        float ychange = std::sqrt(distance * distance - xchange * xchange);

        if (Util::getRandomUInt(1) == 1)
            xchange *= -1;
        if (Util::getRandomUInt(1) == 1)
            ychange *= -1;

        float newposx = getCreature()->GetPositionX() + xchange;
        float newposy = getCreature()->GetPositionY() + ychange;

        CreatureAIScript* pMaievAI = spawnCreatureAndGetAIScript(CN_MAIEV, newposx, newposy, getCreature()->GetPositionZ() + 0.5f, 2.177125f);
        if (pMaievAI == nullptr)
        {
            UnstuckFromShadowPrison();
            return false;
        }

        pMaievAI->getCreature()->GetAIInterface()->setNextTarget(getCreature());
        return true;
    }

    void UnstuckFromShadowPrison()
    {
        _removeAuraOnPlayers(ILLIDAN_SHADOW_PRISON);
        setCanEnterCombat(true);
        _setMeleeDisabled(true);
        setRooted(false);

        getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);

        mScenePart = 0;
    }

    void MaievScene()    // thinking about converting Maiev's functions to CreatureAIScript functions
    {
        mTimeLeft -= GetAIUpdateFreq();
        if (mTimeLeft > 0)
            return;

        MaievAI* pMaievAI = NULL;
        if (mScenePart > 3)
        {
            Creature* pMaiev = getNearestCreature(CN_MAIEV);
            if (pMaiev != NULL && pMaiev->GetScript() != NULL)
            {
                pMaievAI = static_cast< MaievAI* >(pMaiev->GetScript());
            }
            else
            {
                UnstuckFromShadowPrison();
                return;
            }
        }

        switch (mScenePart)
        {
            case 1:
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11476, "Is this it, mortals? Is this all the fury you can muster?");
                break;
            case 2:
                getCreature()->Emote(EMOTE_ONESHOT_QUESTION);
                break;
            case 3:
                if (!SpawnMaiev())
                    return;
                break;
            case 4:
                pMaievAI->_applyAura(MAIEV_TELEPORT);
                getCreature()->GetAIInterface()->setNextTarget(pMaievAI->getCreature());
                break;
            case 5:
                pMaievAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11491, "Their fury pales before mine, Illidan. We have some unsettled business between us.");
                break;
            case 6:
                pMaievAI->_setWieldWeapon(true);
                break;
            case 7:
                pMaievAI->getCreature()->Emote(EMOTE_ONESHOT_EXCLAMATION);
                pMaievAI->_setDisplayWeapon(false, false);
                break;
            case 8:
                {
                sendChatMessage(CHAT_MSG_MONSTER_YELL, 11477, "Maiev... How is it even possible?");
                    pMaievAI->_setDisplayWeapon(true, true);
                }
                break;
            case 9:
                {
                    float Facing = pMaievAI->getCreature()->calcRadAngle(getCreature()->GetPositionX(), getCreature()->GetPositionY(), pMaievAI->getCreature()->GetPositionX(), pMaievAI->getCreature()->GetPositionY());
                    getCreature()->SetFacing(Facing);
                    getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                }
                break;
            case 10:
                pMaievAI->sendChatMessage(CHAT_MSG_MONSTER_YELL, 11492, "Ah, my long hunt is finally over. Today, Justice will be done!");
                pMaievAI->getCreature()->Emote(EMOTE_ONESHOT_EXCLAMATION);
                pMaievAI->_setDisplayWeapon(false, false);
                break;
            case 11:
                pMaievAI->getCreature()->Emote(EMOTE_ONESHOT_YES);
                pMaievAI->_setDisplayWeapon(true, true);
                break;
            case 12:
                pMaievAI->getCreature()->Emote(EMOTE_ONESHOT_ROAR);
                break;
            case 13:
                setCanEnterCombat(true);
                _setMeleeDisabled(true);
                setRooted(false);
                setScriptPhase(5);

                getCreature()->removeUnitFlags(UNIT_FLAG_NON_ATTACKABLE);

                pMaievAI->RegisterAIUpdateEvent(1000);
                pMaievAI->mYellTimer = pMaievAI->_addTimer((Util::getRandomUInt(20) + 20) * 1000);
                pMaievAI->mTrapTimer = pMaievAI->_addTimer((Util::getRandomUInt(5) + 18) * 1000);
                pMaievAI->getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                pMaievAI->setCanEnterCombat(true);
                pMaievAI->getCreature()->GetAIInterface()->setCurrentAgent(AGENT_NULL);
                pMaievAI->getCreature()->GetAIInterface()->setAiState(AI_STATE_IDLE);
                pMaievAI->setRooted(false);
                pMaievAI->getCreature()->GetAIInterface()->AttackReaction(getCreature(), 1, 0);
                pMaievAI->getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
                pMaievAI->_setWieldWeapon(true);
                pMaievAI->mIllidanAI = this;

                _clearHateList();
                getCreature()->GetAIInterface()->AttackReaction(pMaievAI->getCreature(), 200);

                mParasiticTimer = 30000;
                mDemonTimer = 60000;
                mPhaseBackup = 5;
                mEnrageTimer = 25000;
                mYellTimer = 25000;
                break;
        }

        ++mScenePart;
        if (mScenePart > 13)
        {
            mScenePart = 0;
            return;
        }

        mTimeLeft = MaievTimers[mScenePart - 1];
    }

    void PhaseThree()
    {
        if (mMiscEventPart != 0)
        {
            Transform(Ascend, 6);
            return;
        }
        else if (mScenePart != 0)
        {
            MaievScene();
            return;
        }
        else if (getCreature()->getHealthPct() <= 30 && !_isCasting())
        {
#ifdef USE_SHADOW_PRISON
            stopMovement();
            //CastSpellNowNoScheduling(mShadowPrison);
#endif

            setCanEnterCombat(false);
            setAIAgent(AGENT_NULL);
            _setMeleeDisabled(false);
            setRooted(true);

            getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

            mTimeLeft = MaievTimers[0];
            mScenePart = 1;
            return;
        }

        mDemonTimer -= GetAIUpdateFreq();
        mParasiticTimer -= GetAIUpdateFreq();
        if (!_isCasting())
        {
            if (mDemonTimer <= 0)
            {
                stopMovement();
                setAIAgent(AGENT_SPELL);
                SetAIUpdateFreq(250);

                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

                mMiscEventPart = 1;
                mTimeLeft = Ascend[0].mTimer;
                return;
            }
            /*if (mParasiticTimer <= 0)
            {
                CastSpellNowNoScheduling(mParasitic);
                mParasiticTimer = 30000;
                return;
            }*/

            getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
            
        }
    }

    void PhaseFour()
    {
        if (mPlaySound)
        {
            switch (Util::getRandomUInt(2))
            {
                case 0:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11469, "You know nothing of power!");
                    mPlaySound = false;
                    break;
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11471, "Such arrogance!");
                    mPlaySound = false;
                    break;
            }
        }

        mDemonTimer -= GetAIUpdateFreq();
        mFlameBurstTimer -= GetAIUpdateFreq();
        mShadowDemonsTimer -= GetAIUpdateFreq();
        if (_isCasting())
        {
            mPlaySound = false;
            return;
        }

        if (mMiscEventPart != 0)
        {
            Transform(Descend, 6);
            return;
        }

        if (mDemonTimer <= 0 || _getHealthPercent() <= 30 && mPhaseBackup == 3)
        {
            _setDisplayWeapon(true, true);
            stopMovement();
            setAIAgent(AGENT_SPELL);
            SetAIUpdateFreq(250);

            getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

            mMiscEventPart = 1;
            mTimeLeft = Descend[0].mTimer;
        }
        else if (getCreature()->GetAIInterface()->getNextTarget() != NULL)
        {
            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (getRangeToObject(pTarget) <= 80.0f)
            {
                setAIAgent(AGENT_SPELL);
                stopMovement();

                if (mShadowDemonsTimer <= 0)
                {
                    //CastSpellNowNoScheduling(mShadowDemons);
                    spawnCreature(CN_SHADOW_DEMON, getCreature()->GetPositionX() + Util::getRandomFloat(5), getCreature()->GetPositionY() + Util::getRandomFloat(5), getCreature()->GetPositionZ() + 2.0f, 0);
                    spawnCreature(CN_SHADOW_DEMON, getCreature()->GetPositionX() - Util::getRandomFloat(5), getCreature()->GetPositionY() + Util::getRandomFloat(5), getCreature()->GetPositionZ() + 2.0f, 0);
                    spawnCreature(CN_SHADOW_DEMON, getCreature()->GetPositionX() + Util::getRandomFloat(5), getCreature()->GetPositionY() - Util::getRandomFloat(5), getCreature()->GetPositionZ() + 2.0f, 0);

                    mShadowDemonsTimer = 120000;
                    return;
                }
                if (mFlameBurstTimer <= 0)
                {
                    //CastSpellNowNoScheduling(mFlameBurst);
                    for (const auto& itr : getCreature()->getInRangePlayersSet())
                    {
                        Unit* pUnit = static_cast<Unit*>(itr);
                        if (pUnit)
                        {
                            CreatureAIScript* pAI = spawnCreatureAndGetAIScript(CN_FLAME_BURST, itr->GetPositionX(), itr->GetPositionY(), itr->GetPositionZ(), 0, getCreature()->getFactionTemplate());
                            getCreature()->castSpell(pUnit, ILLIDAN_FLAME_BURST2, true);
                            if (pAI != nullptr)
                            {
                                float Distance = getRangeToObject(pUnit);
                                if (Distance == 0.0f)
                                {
                                    pAI->RegisterAIUpdateEvent(300);        // o'rly?
                                }
                                else
                                {
                                    pAI->RegisterAIUpdateEvent((uint32_t)(Distance * 1000 / 32.796));
                                }
                            }
                        }
                    }

                    mFlameBurstTimer += 20000;
                    return;
                }

                /*uint32_t Spell = Util::getRandomUInt() % 100;
                if (Spell <= 80)
                    CastSpellNowNoScheduling(mShadowBlast);*/
            }
            else
            {
                setAIAgent(AGENT_NULL);
            }
        }
    }

    void PhaseFive()
    {
        if (mMiscEventPart != 0)
        {
            Transform(Ascend, 6);
            return;
        }

        mYellTimer -= GetAIUpdateFreq();
        mDemonTimer -= GetAIUpdateFreq();
        mEnrageTimer -= GetAIUpdateFreq();
        mParasiticTimer -= GetAIUpdateFreq();
        if (getCreature()->HasAura(40760) || getCreature()->HasAura(40695))
        {
            mEnrageTimer = 120000;
        }
        else if (!_isCasting())
        {
            if (mDemonTimer <= 0)
            {
                stopMovement();
                setAIAgent(AGENT_SPELL);
                SetAIUpdateFreq(250);

                getCreature()->setEmoteState(EMOTE_ONESHOT_NONE);

                mMiscEventPart = 1;
                mTimeLeft = Ascend[0].mTimer;
                mPlaySound = true;
                return;
            }
            /*if (mEnrageTimer <= 0)
            {
                CastSpellNowNoScheduling(mEnrage);
                mEnrageTimer = 120000;
                mYellTimer += 7000;
                return;
            }*/
            /*if (mParasiticTimer <= 0)
            {
                CastSpellNowNoScheduling(mParasitic);
                mParasiticTimer = 30000;
                return;
            }*/
            if (mYellTimer <= 0 && getCreature()->GetAIInterface()->getNextTarget() != NULL)
            {
                Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
                if (pTarget->isCreature() && pTarget->getEntry() == CN_MAIEV)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11470, "Feel the hatred of ten thousand years!");
                    mYellTimer = 25000;
                }
            }

            getCreature()->setEmoteState(EMOTE_ONESHOT_READY1H);
            
        }
    }

    void OnReachWP(uint32_t pWaypointId, bool /*pForwards*/) override
    {
        if (pWaypointId == 1)
        {
            _clearHateList();
            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget != NULL && (!pTarget->isCreature() || pTarget->getEntry() != CN_FACE_TRIGGER))
            {
                Creature* pTrigger = getNearestCreature(677.399963f, 305.545044f, 353.192169f, CN_FACE_TRIGGER);
                if (pTrigger != NULL)
                {
                    getCreature()->GetAIInterface()->setNextTarget(pTrigger);
                }
            }
        }

        StopWaypointMovement();

        mCurrentWaypoint = pWaypointId;
        mAllow = true;
    }

    void Destroy() override
    {
        Creature* pAkama = getNearestCreature(CN_AKAMA);
        if (pAkama != NULL && pAkama->GetScript() != NULL)
        {
            AkamaAI* pAI = static_cast< AkamaAI* >(pAkama->GetScript());
            if (pAI != NULL)
            {
                if (pAI->mIllidanAI == this)
                    pAI->mIllidanAI = NULL;
            }
        }

        Creature* pMaiev = getNearestCreature(CN_MAIEV);
        if (pMaiev != NULL && pMaiev->GetScript() != NULL)
        {
            MaievAI* pAI = static_cast< MaievAI* >(pMaiev->GetScript());
            if (pAI != NULL)
            {
                if (pAI->mIllidanAI == this)
                    pAI->mIllidanAI = NULL;
            }
        }

        // commented - due to the creature not being in world when this is called, mapmgr == null -> access violation
        /*        GameObject* pRightGate = getNearestGameObject(745.07f, 241.802f, 354.292f, 200000);
                GameObject* pLeftGate  = getNearestGameObject(744.829f, 369.276f, 354.324f, 200001);
                if (pRightGate != NULL)
                {
                    pRightGate->setState(GO_STATE_OPEN);
                }
                if (pLeftGate != NULL)
                {
                    pLeftGate->setState(GO_STATE_OPEN);
                }*/

        delete this;
    }

    // Global variables
    uint32_t mPhaseBackup;
    uint32_t mScenePart;
    int32_t mTimeLeft;
    bool mAllow;

    // Phase 1 variables
    int32_t mParasiticTimer;
    /*SpellDesc* mParasiticDmg;
    SpellDesc* mParasitic;*/

    // Phase 2 variables
    CreatureAIScript* mFoA1;
    CreatureAIScript* mFoA2;
    int32_t mMovementTimer;
    int32_t mFireWallTimer;
    int32_t mLastFireWall;
    uint32_t mMiscEventPart;
    //SpellDesc* mGlaiveThrow;
    //SpellDesc* mGlaiveReturns;

    //// Phase 3 variables
    //SpellDesc* mShadowPrison;

    // Phase 4 variables
    int32_t mShadowDemonsTimer;
    int32_t mFlameBurstTimer;
    /*SpellDesc* mFlameBurst;
    SpellDesc* mShadowDemons;
    SpellDesc* mShadowBlast;
    SpellDesc* mLocaleEnrageSpell;*/
    uint32_t mLocaleEnrageTimerId;
    bool mPlaySound;

    // Phase 3 & 4 variables
    int32_t mDemonTimer;

    // Phase 5 variables
    int32_t mYellTimer;
    int32_t mEnrageTimer;

    // Temporary variables
    uint32_t mCurrentWaypoint;
};

//void SpellFunc_Illidan_Parasitic(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType /*pType*/)
//{
//    IllidanStormrageAI* Illidan = (pCreatureAI != NULL) ? static_cast< IllidanStormrageAI* >(pCreatureAI) : NULL;
//    if (Illidan != NULL)
//    {
//        Illidan->castSpell(Illidan->mParasiticDmg);
//        if (pTarget != NULL)                        // not sure if target is really added here
//        {
//            // Workaround - we will spawn 2 Parasitic Shadowfiends on that player place
//            Illidan->spawnCreature(CN_PARASITIC_SHADOWFIEND, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
//            Illidan->spawnCreature(CN_PARASITIC_SHADOWFIEND, pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), pTarget->GetOrientation());
//        }
//    }
//}

/*END .:Rewritten Illidan Script by M4ksiu:. END*/

float PositionAdds[8][2] =
{
    { 0, 1 },
    { 0.5, 0.5 },
    { 1, 0 },
    { 0.5, -0.5 },
    { 0, 1 },
    { -0.5, 0.5 },
    { -1, 0 },
    { -0.5, 0.5 }
};

class CageTrapTriggerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(CageTrapTriggerAI)
    explicit CageTrapTriggerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
        getCreature()->GetAIInterface()->m_canMove = false;
        getCreature()->m_noRespawn = true;

        RegisterAIUpdateEvent(1000);
        _setMeleeDisabled(false);
        mIsActivated = false;
        mHasTrapped = false;
    }

    void AIUpdate() override
    {
        Creature* pIllidan = getNearestCreature(22917);
        if (pIllidan != NULL)
        {
            IllidanStormrageAI* pAI = static_cast< IllidanStormrageAI* >(pIllidan->GetScript());
            if (pAI->mMiscEventPart != 0 && mTriggerAIList.size() == 0)
            {
                GameObject* pGameObject = getNearestGameObject(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), GO_CAGE_TRAP);
                if (pGameObject != NULL)
                {
                    pGameObject->Despawn(0, 0);
                }

                despawn(0);
                return;
            }

            if (!mIsActivated)
                return;

            if (mTriggerAIList.size() == 0 && pIllidan->isAlive() && getRangeToObject(pIllidan) <= 5.0f && !pAI->_isCasting())
            {
                for (uint8_t i = 0; i < 8; ++i)
                {
                    CreatureAIScript* pTriggerAI = spawnCreatureAndGetAIScript(CN_CAGE_TRAP_TRIGGER, getCreature()->GetPositionX() + PositionAdds[i][0], getCreature()->GetPositionY() + PositionAdds[i][1], getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                    if (pTriggerAI != nullptr)
                    {
                        pTriggerAI->getCreature()->addUnitFlags(UNIT_FLAG_NOT_SELECTABLE);
                        pTriggerAI->getCreature()->GetAIInterface()->m_canMove = false;
                        pTriggerAI->getCreature()->m_noRespawn = true;
                        mTriggerAIList.push_back(pTriggerAI);
                    }
                }

                pAI->_applyAura(CAGE_TRAP);
                pAI->setRooted(true);
                pAI->_setMeleeDisabled(false);
                pAI->stopMovement();

                SetAIUpdateFreq(2500);
                return;
            }
            else if (pIllidan->isAlive() && !mHasTrapped && mTriggerAIList.size() != 0)
            {
                pAI->_removeAura(ILLIDAN_ENRAGE);
                pAI->_applyAura(CAGED1);
                SetAIUpdateFreq(14000);

                for (size_t i = 0; i < mTriggerAIList.size() / 2; ++i)
                {
                    CreatureAIScript* pTriggerAI1 = mTriggerAIList[i];
                    CreatureAIScript* pTriggerAI2 = mTriggerAIList[mTriggerAIList.size() - i - 1];
                    pTriggerAI1->_setTargetToChannel(pTriggerAI2->getCreature(), 40708);//CAGED2);
                    pTriggerAI2->_setTargetToChannel(pTriggerAI1->getCreature(), 40709);//CAGED2);
                }

                mHasTrapped = true;
                return;
            }
            else if (mHasTrapped || !pIllidan->isAlive())
            {
                GameObject* pGameObject = getNearestGameObject(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), GO_CAGE_TRAP);
                if (pGameObject != NULL)
                {
                    pGameObject->Despawn(0, 0);
                }

                // Immediatly Illidan, after trap expires, becomes Demon
                if (pIllidan->isAlive())
                {
                    pAI->_removeAura(CAGED1);
                    pAI->setRooted(false);
                    pAI->_setMeleeDisabled(true);
                    pAI->stopMovement();
                    pAI->setAIAgent(AGENT_SPELL);

                    pIllidan->setEmoteState(EMOTE_ONESHOT_NONE);

                    pAI->SetAIUpdateFreq(250);
                    pAI->mMiscEventPart = 1;
                    pAI->mTimeLeft = 250;
                }
                for (size_t i = 0; i < mTriggerAIList.size(); ++i)
                {
                    CreatureAIScript* pTriggerAI = mTriggerAIList[i];
                    pTriggerAI->_setTargetToChannel(NULL, 0);
                    pTriggerAI->despawn(0);
                }

                mTriggerAIList.clear();
                RemoveAIUpdateEvent();
                despawn(0);
                return;
            }
        }
        else
        {
            GameObject* pGameObject = getNearestGameObject(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), GO_CAGE_TRAP);
            if (pGameObject != NULL)
            {
                pGameObject->Despawn(0, 0);
            }

            for (size_t i = 0; i < mTriggerAIList.size(); ++i)
            {
                CreatureAIScript* pTriggerAI = mTriggerAIList[i];
                pTriggerAI->despawn(0);
            }

            mTriggerAIList.clear();
            RemoveAIUpdateEvent();
            despawn(0);
        }
    }

    std::vector<CreatureAIScript*> mTriggerAIList;
    bool mIsActivated;
    bool mHasTrapped;
};

// Animation progress is wrong - it should be closed and OnActive - open

class CageTrapGO : public GameObjectAIScript
{
public:

    explicit CageTrapGO(GameObject* pGameObject) : GameObjectAIScript(pGameObject)
    {
        _gameobject->setScale(3.0f);
    }

    void OnActivate(Player* /*pPlayer*/) override
    {
        _gameobject->setFlags(GO_FLAG_NONSELECTABLE);
        Creature* pTrigger = _gameobject->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(_gameobject->GetPositionX(), _gameobject->GetPositionY(), _gameobject->GetPositionZ(), CN_CAGE_TRAP_DISTURB_TRIGGER);
        if (pTrigger != NULL && pTrigger->GetScript() != NULL)
        {
            CageTrapTriggerAI* pTriggerAI = static_cast< CageTrapTriggerAI* >(pTrigger->GetScript());
            pTriggerAI->mIsActivated = true;
        }
    }

    static GameObjectAIScript* Create(GameObject* pGameObject) { return new CageTrapGO(pGameObject); }
};

class DranaeiSpiritAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(DranaeiSpiritAI)
    explicit DranaeiSpiritAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        despawn(45000);
        setCanEnterCombat(false);
        getCreature()->m_noRespawn = true;
    }

    void Destroy() override
    {
        Creature* pAkama = getNearestCreature(22990);
        if (pAkama != NULL && pAkama->GetScript() != NULL)
        {
            AkamaAI* pAI = static_cast< AkamaAI* >(pAkama->GetScript());
            if (pAI != NULL)
            {
                if (pAI->mUdaloAI == this)
                    pAI->mUdaloAI = NULL;
                else if (pAI->mOlumAI == this)
                    pAI->mOlumAI = NULL;
            }
        }

        delete this;
    }
};

class FlameOfAzzinothAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(FlameOfAzzinothAI)
    explicit FlameOfAzzinothAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mFlameBlast = addAISpell(FLAME_OF_AZZINOTH_FLAME_BLAST, 15.0f, TARGET_ATTACKING, 0, 25);

        mCharge = addAISpell(FLAME_OF_AZZINOTH_CHARGE, 0.0f, TARGET_ATTACKING, 0, 60);
        mEnrage = addAISpell(FLAME_OF_AZZINOTH_ENRAGE, 0.0f, TARGET_SELF);

        getCreature()->m_noRespawn = true;
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == FLAME_OF_AZZINOTH_FLAME_BLAST)
        {
            spawnCreature(CN_BLAZE_EFFECT, getCreature()->GetPosition());
        }
    }

    void Destroy() override
    {
        Creature* pIllidan = getNearestCreature(22917);
        if (pIllidan != NULL && pIllidan->GetScript() != NULL)
        {
            IllidanStormrageAI* pAI = static_cast< IllidanStormrageAI* >(pIllidan->GetScript());
            if (pAI != NULL)
            {
                if (pAI->mFoA1 == this)
                    pAI->mFoA1 = NULL;
                else if (pAI->mFoA2 == this)
                    pAI->mFoA2 = NULL;
            }
        }

        delete this;
    }

    // Flame Blast Spell Function
    CreatureAISpells* mFlameBlast;

    // Charge Spell Function
    CreatureAISpells* mCharge;
    CreatureAISpells* mEnrage;
};

void SetupBlackTemple(ScriptMgr* mgr)
{
    mgr->register_instance_script(MAP_BLACK_TEMPLE, &BlackTempleScript::Create);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Mobs
    mgr->register_creature_script(CN_DRAGON_TURTLE, &DragonTurtleAI::Create);
    mgr->register_creature_script(CN_LEVIATHAN, &LeviathanAI::Create);
    mgr->register_creature_script(CN_MUTANT_WAR_HOUND, &MutantWarHoundAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_RIDING_HOUND, &ShadowmoonRidingHoundAI::Create);
    mgr->register_creature_script(CN_SISTER_OF_PLEASURE, &SisterOfPleasureAI::Create);
    mgr->register_creature_script(CN_SISTER_OF_PAIN, &SisterOfPainAI::Create);
    mgr->register_creature_script(CN_PRIESTESS_OF_DEMENTIA, &PriestessOfDementiaAI::Create);
    mgr->register_creature_script(CN_PRIESTESS_OF_DELIGHT, &PriestessOfDelightAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_NIGHTLORD, &IllidariNightlordAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_HEARTSEEKER, &IllidariHeartseekerAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_FEARBRINGER, &IllidariFearbringerAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_DEFILER, &IllidariDefilerAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_CENTURION, &IllidariCenturionAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_BONESLICER, &IllidariBoneslicerAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_BATTLELORD, &AshtongueBattlelordAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_DEFENDER, &AshtongueDefenderAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_ELEMENTALIST, &AshtongueElementalistAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_MYSTIC, &AshtongueMysticAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_PRIMALIST, &AshtonguePrimalistAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_ROGUE, &AshtongueRogueAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_SPIRITBINDER, &AshtongueSpiritbinderAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_STALKER, &AshtongueStalkerAI::Create);
    mgr->register_creature_script(CN_ASHTONGUE_STORMCALLER, &AshtongueStormcallerAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_BEHEMOTH, &BonechewerBehemothAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_BLADE_FURY, &BonechewerBladeFuryAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_BLOOD_PROPHET, &BonechewerBloodProphetAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_BRAWLER, &BonechewerBrawlerAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_COMBATANT, &BonechewerCombatantAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_SHIELD_DISCIPLE, &BonechewerShieldDiscipleAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_SPECTATOR, &BonechewerSpectatorAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_TASKMASTER, &BonechewerTaskmasterAI::Create);
    mgr->register_creature_script(CN_BONECHEWER_WORKER, &BonechewerWorkerAI::Create);
    mgr->register_creature_script(CN_CHARMING_COURTESAN, &CharmingCourtesanAI::Create);
    mgr->register_creature_script(CN_COILSKAR_GENERAL, &CoilskarGeneralAI::Create);
    mgr->register_creature_script(CN_COILSKAR_HARPOONER, &CoilskarHarpoonerAI::Create);
    mgr->register_creature_script(CN_COILSKAR_SEACALLER, &CoilskarSeacallerAI::Create);
    mgr->register_creature_script(CN_COILSKAR_SOOTHSAYER, &CoilskarSoothsayerAI::Create);
    mgr->register_creature_script(CN_COILSKAR_WRANGLER, &CoilskarWranglerAI::Create);
    mgr->register_creature_script(CN_DRAGONMAW_SKY_STALKER, &DragonmawSkyStalkerAI::Create);
    mgr->register_creature_script(CN_DRAGONMAW_WIND_REAVER, &DragonmawWindReaverAI::Create);
    mgr->register_creature_script(CN_DRAGONMAW_WYRMCALLER, &DragonmawWyrmcallerAI::Create);
    mgr->register_creature_script(CN_ENSLAVED_SERVANT, &EnslavedServantAI::Create);
    mgr->register_creature_script(CN_HAND_OF_GOREFIEND, &HandOfGorefiendAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_ARCHON, &IllidariArchonAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_ASSASSIN, &IllidariAssassinAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_BATTLEMAGE, &IllidariBattlemageAI::Create);
    mgr->register_creature_script(CN_ILLIDARI_BLOOD_LORD, &IllidariBloodLordAI::Create);
    mgr->register_creature_script(CN_IMAGE_OF_DEMENTIA, &ImageOfDementiaAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_BLOOD_MAGE, &ShadowmoonBloodMageAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_CHAMPION, &ShadowmoonChampionAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_DEATHSHAPER, &ShadowmoonDeathshaperAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_FALLEN, &ShadowmoonFallenAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_HOUNDMASTER, &ShadowmoonHoundmasterAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_REAVER, &ShadowmoonReaverAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_SOLDIER, &ShadowmoonSoldierAI::Create);
    mgr->register_creature_script(CN_SHADOWMOON_WEAPON_MASTER, &ShadowmoonWeaponMasterAI::Create);
    mgr->register_creature_script(CN_SPELLBOUND_ATTENDANT, &SpellboundAttendantAI::Create);
    mgr->register_creature_script(CN_TEMPLE_CONCUBINE, &TempleConcubineAI::Create);
    mgr->register_creature_script(CN_STORM_FURY, &StormFuryAI::Create);
    mgr->register_creature_script(CN_AQUEOUS_SURGER, &AqueousSurgerAI::Create);
    mgr->register_creature_script(CN_AQUEOUS_SPAWN, &AqueousSpawnAI::Create);
    mgr->register_creature_script(CN_AQUEOUS_LORD, &AqueousLordAI::Create);
    mgr->register_creature_script(CN_PROMENADE_SENTINEL, &PromenadeSentinelAI::Create);
    mgr->register_creature_script(CN_ANGERED_SOUL_FRAGMENT, &AngeredSoulFragmentAI::Create);
    mgr->register_creature_script(CN_ENSLAVED_SOUL, &EnslavedSoulAI::Create);
    mgr->register_creature_script(CN_HUNGERING_SOUL_FRAGMENT, &HungeringSoulFragmentAI::Create);
    mgr->register_creature_script(CN_SHADOWY_CONSTRUCT, &ShadowyConstructAI::Create);
    mgr->register_creature_script(CN_SUFFERING_SOUL_FRAGMENT, &SufferingSoulFragmentAI::Create);
    mgr->register_creature_script(CN_VANGEFUL_SPIRIT, &VangefulSpiritAI::Create);
    mgr->register_creature_script(CN_WRATHBONE_FLAYER, &WrathboneFlayerAI::Create);

    //Bosses
    //mgr->register_creature_script(CN_SUPREMUS, &SupremusAI::Create);
    mgr->register_creature_script(CN_NAJENTUS, &NajentusAI::Create);
    mgr->register_creature_script(CN_GURTOGG_BLOODBOIL, &GurtoggAI::Create);

    mgr->register_creature_script(CN_RELIQUARY_OF_SOULS, &ReliquaryOfSoulsAI::Create);
    mgr->register_creature_script(CN_ESSENCEOFSUFFERING, &EssenceOfSufferingAI::Create);
    mgr->register_creature_script(CN_ESSENCEOFDESIRE, &EssenceOfDesireAI::Create);
    mgr->register_creature_script(CN_ESSENCEOFANGER, &EssenceOfAngerAI::Create);

    mgr->register_creature_script(CN_MOTHER_SHAHRAZ, &ShahrazAI::Create);
    mgr->register_creature_script(CN_LADY_MALANDE, &MalandeAI::Create);
    mgr->register_creature_script(CN_GATHIOS_THE_SHATTERER, &GathiosAI::Create);
    mgr->register_creature_script(CN_HIGH_NETHERMANCER_ZEREVOR, &ZerevorAI::Create);
    mgr->register_creature_script(CN_VERAS_DARKSHADOW, &VerasAI::Create);
    mgr->register_creature_script(CN_TERON_GOREFIEND, &TeronGorefiendAI::Create);
    //mgr->register_creature_script(CN_SHADE_OF_AKAMA, &ShadeofakamaAI::Create); //test

    //Illidan Stormrage related
    GossipScript* AG = new AkamaGossip();
    mgr->register_creature_gossip(CN_AKAMA, AG);

    mgr->register_creature_script(CN_DOOR_EVENT_TRIGGER, &UnselectableTriggerAI::Create);
    mgr->register_creature_script(CN_FACE_TRIGGER, &UnselectableTriggerAI::Create);
    mgr->register_creature_script(CN_CAGE_TRAP_TRIGGER, &UnselectableTriggerAI::Create);
    mgr->register_creature_script(CN_CAGE_TRAP_DISTURB_TRIGGER, &CageTrapTriggerAI::Create);
    mgr->register_creature_script(CN_DEMON_FIRE, &GenericTriggerAI::Create);
    mgr->register_creature_script(CN_EYE_BEAM_TRIGGER, &EyeBeamTriggerAI::Create);
    mgr->register_creature_script(CN_FLAME_CRASH, &GenericTriggerAI::Create);
    mgr->register_creature_script(CN_BLAZE_EFFECT, &GenericTriggerAI::Create);
    mgr->register_creature_script(CN_FLAME_BURST, &GenericTriggerAI::Create);
    mgr->register_creature_script(CN_FLAME_OF_AZZINOTH, &FlameOfAzzinothAI::Create);
    mgr->register_creature_script(CN_SHADOW_DEMON, &ShadowDemonAI::Create);
    mgr->register_creature_script(CN_PARASITIC_SHADOWFIEND, &ParasiticShadowfiendAI::Create);
    mgr->register_creature_script(CN_AKAMA, &AkamaAI::Create);
    mgr->register_creature_script(CN_MAIEV, &MaievAI::Create);
    mgr->register_creature_script(CN_ILLIDAN_STORMRAGE, &IllidanStormrageAI::Create);
    mgr->register_creature_script(CN_UDALO, &DranaeiSpiritAI::Create);
    mgr->register_creature_script(CN_OLUM, &DranaeiSpiritAI::Create);
    mgr->register_gameobject_script(GO_CAGE_TRAP, &CageTrapGO::Create);
}
