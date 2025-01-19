/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"

LocationVector const FactionChampionLoc[] =
{
    { 514.231f, 105.569f, 418.234f, 0 },               //  0 - Horde Initial Pos 0
    { 508.334f, 115.377f, 418.234f, 0 },               //  1 - Horde Initial Pos 1
    { 506.454f, 126.291f, 418.234f, 0 },               //  2 - Horde Initial Pos 2
    { 506.243f, 106.596f, 421.592f, 0 },               //  3 - Horde Initial Pos 3
    { 499.885f, 117.717f, 421.557f, 0 },               //  4 - Horde Initial Pos 4

    { 613.127f, 100.443f, 419.74f, 0 },                //  5 - Ally Initial Pos 0
    { 621.126f, 128.042f, 418.231f, 0 },               //  6 - Ally Initial Pos 1
    { 618.829f, 113.606f, 418.232f, 0 },               //  7 - Ally Initial Pos 2
    { 625.845f, 112.914f, 421.575f, 0 },               //  8 - Ally Initial Pos 3
    { 615.566f, 109.653f, 418.234f, 0 },               //  9 - Ally Initial Pos 4

    { 535.469f, 113.012f, 394.66f, 0 },                // 10 - Horde Final Pos 0
    { 526.417f, 137.465f, 394.749f, 0 },               // 11 - Horde Final Pos 1
    { 528.108f, 111.057f, 395.289f, 0 },               // 12 - Horde Final Pos 2
    { 519.92f, 134.285f, 395.289f, 0 },                // 13 - Horde Final Pos 3
    { 533.648f, 119.148f, 394.646f, 0 },               // 14 - Horde Final Pos 4
    { 531.399f, 125.63f, 394.708f, 0 },                // 15 - Horde Final Pos 5
    { 528.958f, 131.47f, 394.73f, 0 },                 // 16 - Horde Final Pos 6
    { 526.309f, 116.667f, 394.833f, 0 },               // 17 - Horde Final Pos 7
    { 524.238f, 122.411f, 394.819f, 0 },               // 18 - Horde Final Pos 8
    { 521.901f, 128.488f, 394.832f, 0 }                // 19 - Horde Final Pos 9
};

namespace champions
{
    enum AI
    {
        AI_MELEE                        = 0,
        AI_RANGED                       = 1,
        AI_HEALER                       = 2,
        AI_PET                          = 3
    };

    enum Spells
    {
        // generic
        SPELL_ANTI_AOE                  = 68595,
        SPELL_PVP_TRINKET               = 65547,

        // druid healer
        SPELL_LIFEBLOOM                 = 66093,
        SPELL_NOURISH                   = 66066,
        SPELL_REGROWTH                  = 66067,
        SPELL_REJUVENATION              = 66065,
        SPELL_TRANQUILITY               = 66086,
        SPELL_BARKSKIN                  = 65860,
        SPELL_THORNS                    = 66068,
        SPELL_NATURE_GRASP              = 66071,

        // shaman healer
        SPELL_HEALING_WAVE              = 66055,
        SPELL_RIPTIDE                   = 66053,
        SPELL_SPIRIT_CLEANSE            = 66056, //friendly only
        SPELL_HEROISM                   = 65983,
        SPELL_BLOODLUST                 = 65980,
        SPELL_HEX                       = 66054,
        SPELL_EARTH_SHIELD              = 66063,
        SPELL_EARTH_SHOCK               = 65973,
        AURA_EXHAUSTION                 = 57723,
        AURA_SATED                      = 57724,

        // paladin healer
        SPELL_HAND_OF_FREEDOM           = 68757,
        SPELL_DIVINE_SHIELD             = 66010,
        SPELL_CLEANSE                   = 66116,
        SPELL_FLASH_OF_LIGHT            = 66113,
        SPELL_HOLY_LIGHT                = 66112,
        SPELL_HOLY_SHOCK                = 66114,
        SPELL_HAND_OF_PROTECTION        = 66009,
        SPELL_HAMMER_OF_JUSTICE         = 66613,
        SPELL_FORBEARANCE               = 25771,

        // priest healer
        SPELL_RENEW                     = 66177,
        SPELL_SHIELD                    = 66099,
        SPELL_FLASH_HEAL                = 66104,
        SPELL_DISPEL                    = 65546,
        SPELL_PSYCHIC_SCREAM            = 65543,
        SPELL_MANA_BURN                 = 66100,
        SPELL_PENANCE                   = 66097,

        // priest dps
        SPELL_SILENCE                   = 65542,
        SPELL_VAMPIRIC_TOUCH            = 65490,
        SPELL_SW_PAIN                   = 65541,
        SPELL_MIND_FLAY                 = 65488,
        SPELL_MIND_BLAST                = 65492,
        SPELL_HORROR                    = 65545,
        SPELL_DISPERSION                = 65544,
        SPELL_SHADOWFORM                = 16592,

        // warlock
        SPELL_HELLFIRE                  = 65816,
        SPELL_CORRUPTION                = 65810,
        SPELL_CURSE_OF_AGONY            = 65814,
        SPELL_CURSE_OF_EXHAUSTION       = 65815,
        SPELL_FEAR                      = 65809,
        SPELL_SEARING_PAIN              = 65819,
        SPELL_SHADOW_BOLT               = 65821,
        SPELL_UNSTABLE_AFFLICTION       = 65812,
        SPELL_UNSTABLE_AFFLICTION_DISPEL = 65813,
        SPELL_SUMMON_FELHUNTER          = 67514,

        // mage
        SPELL_ARCANE_BARRAGE            = 65799,
        SPELL_ARCANE_BLAST              = 65791,
        SPELL_ARCANE_EXPLOSION          = 65800,
        SPELL_BLINK                     = 65793,
        SPELL_COUNTERSPELL              = 65790,
        SPELL_FROST_NOVA                = 65792,
        SPELL_FROSTBOLT                 = 65807,
        SPELL_ICE_BLOCK                 = 65802,
        SPELL_POLYMORPH                 = 65801,

        // hunter
        SPELL_AIMED_SHOT                = 65883,
        SPELL_DETERRENCE                = 65871,
        SPELL_DISENGAGE                 = 65869,
        SPELL_EXPLOSIVE_SHOT            = 65866,
        SPELL_FROST_TRAP                = 65880,
        SPELL_SHOOT                     = 65868,
        SPELL_STEADY_SHOT               = 65867,
        SPELL_WING_CLIP                 = 66207,
        SPELL_WYVERN_STING              = 65877,
        SPELL_CALL_PET                  = 67777,

        // druid dps
        SPELL_CYCLONE                   = 65859,
        SPELL_ENTANGLING_ROOTS          = 65857,
        SPELL_FAERIE_FIRE               = 65863,
        SPELL_FORCE_OF_NATURE           = 65861,
        SPELL_INSECT_SWARM              = 65855,
        SPELL_MOONFIRE                  = 65856,
        SPELL_STARFIRE                  = 65854,
        SPELL_WRATH                     = 65862,

        // warrior
        SPELL_BLADESTORM                = 65947,
        SPELL_INTIMIDATING_SHOUT        = 65930,
        SPELL_MORTAL_STRIKE             = 65926,
        SPELL_CHARGE                    = 68764,
        SPELL_DISARM                    = 65935,
        SPELL_OVERPOWER                 = 65924,
        SPELL_SUNDER_ARMOR              = 65936,
        SPELL_SHATTERING_THROW          = 65940,
        SPELL_RETALIATION               = 65932,

        // death knight
        SPELL_CHAINS_OF_ICE             = 66020,
        SPELL_DEATH_COIL                = 66019,
        SPELL_DEATH_GRIP                = 66017,
        SPELL_FROST_STRIKE              = 66047,
        SPELL_ICEBOUND_FORTITUDE        = 66023,
        SPELL_ICY_TOUCH                 = 66021,
        SPELL_STRANGULATE               = 66018,
        SPELL_DEATH_GRIP_PULL           = 64431,    // used at spellscript

        // rogue
        SPELL_FAN_OF_KNIVES             = 65955,
        SPELL_BLIND                     = 65960,
        SPELL_CLOAK                     = 65961,
        SPELL_BLADE_FLURRY              = 65956,
        SPELL_SHADOWSTEP                = 66178,
        SPELL_HEMORRHAGE                = 65954,
        SPELL_EVISCERATE                = 65957,
        SPELL_WOUND_POISON              = 65962,

        // shaman dps (some spells taken from shaman healer)
        SPELL_LAVA_LASH                 = 65974,
        SPELL_STORMSTRIKE               = 65970,
        SPELL_WINDFURY                  = 65976,

        // paladin dps
        SPELL_AVENGING_WRATH            = 66011,
        SPELL_CRUSADER_STRIKE           = 66003,
        SPELL_DIVINE_STORM              = 66006,
        SPELL_HAMMER_OF_JUSTICE_RET     = 66007,
        SPELL_JUDGEMENT_OF_COMMAND      = 66005,
        SPELL_REPENTANCE                = 66008,
        SPELL_SEAL_OF_COMMAND           = 66004,

        // warlock pet
        SPELL_DEVOUR_MAGIC              = 67518,
        SPELL_SPELL_LOCK                = 67519,

        // hunter pet
        SPELL_CLAW                      = 67793
    };

    enum Actions
    {
        ACTION_SUMMON,
        ACTION_START,
        ACTION_INPROGRESS,
        ACTION_FAILED,
        ACTION_PERFORMED
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Northrend Beasts Combat Stalker
class ChampionControllerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ChampionControllerAI(Creature* pCreature);

    void InitOrReset() override;

    void DoAction(int32_t action) override;

    void summonChampions();
    std::vector<uint32_t> selectChampions(PlayerTeam playerTeam);

protected:
    uint32_t mChampionsNotStarted = 0;
    uint32_t mChampionsFailed = 0;
    uint32_t mChampionsKilled = 0;
    bool mInProgress = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Main Structure for Champions AI
class FactionChampionsAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FactionChampionsAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
    bool onAttackStart(Unit* target) override;
    void OnCombatStart(Unit* /*_target*/) override;
    void OnTargetDied(Unit* /*_target*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void justReachedSpawn() override;

    float calculateThreat(float distance, uint32_t armor, uint32_t health) const;

    uint32_t enemiesInRange(float range);

protected:
    void update(CreatureAIFunc pThis);
    void removeCC(CreatureAIFunc pThis);

    uint8_t mAIType;
    uint8_t mTeamInInstance;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Healers Champions AI
class DruidAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit DruidAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class ShamanAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ShamanAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class PaladinAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit PaladinAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class PriestAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit PriestAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Ranged Champions AI
class ShadowPriestAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ShadowPriestAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class WarlockAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit WarlockAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;

    void OnCombatStart(Unit* _target) override;
};

class MageAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MageAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class HunterAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit HunterAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;

    void OnCombatStart(Unit* _target) override;
};

class BoomkinAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BoomkinAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class WarriorAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit WarriorAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class DKAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit DKAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class RogueAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit RogueAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class EnhancerAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit EnhancerAI(Creature* pCreature, uint8_t aitype);

    void initialize();
    void InitOrReset() override;

    void OnDied(Unit* _killer) override;

    void onSummonedCreature(Creature* summon) override;   // We summoned a Creature
    void OnSummonDespawn(Creature* summon) override;   // Summoned Creature got UnSummoned

    void deployTotems();

protected:
    uint8_t mTotemCount;
    float mTotemOldCenterX;
    float mTotemOldCenterY;
};

class RetriAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit RetriAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;

    void OnCombatStart(Unit* _target) override;
};

class PetWarlockAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit PetWarlockAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};

class PetHunterAI : public FactionChampionsAI
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit PetHunterAI(Creature* pCreature, uint8_t aitype);

    void InitOrReset() override;
};