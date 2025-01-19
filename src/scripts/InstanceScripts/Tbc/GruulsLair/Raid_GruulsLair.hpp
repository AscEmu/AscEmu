/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"

enum EncounterData
{
    DATA_MAULGAR                    = 0,
    DATA_GRUUL                      = 1
};

enum CreatureEntry
{
    NPC_MAULGAR                     = 18831,
    NPC_KROSH_FIREHAND              = 18832,
    NPC_OLM_THE_SUMMONER            = 18834,
    NPC_KIGGLER_THE_CRAZED          = 18835,
    NPC_BLINDEYE_THE_SEER           = 18836,
    NPC_GRUUL_THE_DRAGONKILLER      = 19044,
    NPC_WILD_FEL_STALKER            = 18847,
    NPC_GRONN_PRIEST                = 21350,
    NPC_LAIR_BRUTE                  = 19389
};

enum GameObjectEntry
{
    GO_MAULGAR_DOOR                 = 184468,
    GO_GRUUL_DOOR                   = 184662
};

enum CreatureSpells
{
    // Wild Fel Stalker
    SPELL_WILD_BITE                 = 33086,

    // Gronn Priest
    SPELL_PSYCHICSCREAM             = 22884,
    SPELL_RENEW                     = 36679,
    SPELL_HEAL_GP                   = 36678,

    // Lair Brute
    SPELL_MORTALSTRIKE              = 39171,
    SPELL_CLEAVE                    = 39174,
    SPELL_CHARGE                    = 24193,

    // High King Maulgar
    SPELL_ARCING_SMASH              = 39144,
    SPELL_MIGHTY_BLOW               = 33230,
    SPELL_WHIRLWIND                 = 33238,
    SPELL_BERSERKER_C               = 26561,
    SPELL_ROAR                      = 16508,
    SPELL_FLURRY                    = 33232,
    SPELL_DUAL_WIELD                = 29651,

    // Olm the Summoner
    SPELL_DARK_DECAY                = 33129,
    SPELL_DEATH_COIL                = 33130,
    SPELL_SUMMON_WFH                = 33131,

    // Kiggler the Craed
    SPELL_GREATER_POLYMORPH         = 33173,
    SPELL_LIGHTNING_BOLT            = 36152,
    SPELL_ARCANE_SHOCK              = 33175,
    SPELL_ARCANE_EXPLOSION          = 33237,

    // Blindeye the Seer
    SPELL_GREATER_PW_SHIELD         = 33147,
    SPELL_HEAL                      = 33144,
    SPELL_PRAYER_OH                 = 33152,

    // Krosh Firehand
    SPELL_GREATER_FIREBALL          = 33051,
    SPELL_SPELLSHIELD               = 33054,
    SPELL_BLAST_WAVE                = 33061,

    // Gruul
    SPELL_GROWTH                    = 36300,
    SPELL_CAVE_IN                   = 36240,
    SPELL_GROUND_SLAM               = 33525, // AoE Ground Slam applying Ground Slam to everyone with a script effect (most likely the knock back, we can code it to a set knockback)
    SPELL_REVERBERATION             = 36297,
    SPELL_SHATTER                   = 33654,

    SPELL_SHATTER_EFFECT            = 33671,
    SPELL_HURTFUL_STRIKE            = 33813,
    SPELL_STONED                    = 33652, // Spell is self cast by target

    SPELL_MAGNETIC_PULL             = 28337,
    SPELL_KNOCK_BACK                = 24199, // Knockback spell until correct implementation is made
};

enum Actions
{
    ACTION_ENABLE                   = 1,
    ACTION_DISABLE                  = 2,
    ACTION_ADD_DEATH                = 3
};

enum DataTypes
{
    DATA_DOOR_MAULGAR               = 1,
    DATA_DOOR_GRUUL                 = 2
};

class GruulsLairInstanceScript : public InstanceScript
{
public:
    explicit GruulsLairInstanceScript(WorldMap* pMapMgr);
    static InstanceScript* Create(WorldMap* pMapMgr);

    void OnGameObjectPushToWorld(GameObject* pGameObject) override;
    void OnCreaturePushToWorld(Creature* pCreature) override;
    void OnEncounterStateChange(uint32_t entry, uint32_t state) override;
    void SetGameobjectStates();
    void setLocalData(uint32_t type, uint32_t data) override;

public:
    GruulsLairInstanceScript* Instance;
    GameObject* mDoorMaulgar;
    GameObject* mDoorGruul;
    Creature* mKrosh;
    Creature* mOlm;
    Creature* mKiggler;
    Creature* mBlindeye;
    Creature* mMaulgar;
    Creature* mGruul;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Creature: Lair Brute
class LairBruteAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LairBruteAI(Creature* pCreature);

    void OnCastSpell(uint32_t spellId) override;
};
