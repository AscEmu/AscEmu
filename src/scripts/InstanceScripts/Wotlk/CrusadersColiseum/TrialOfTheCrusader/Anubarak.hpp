/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"
#include "CommonTime.hpp"

namespace anubarak
{
    enum Yells
    {
        SAY_INTRO                   = 0,
        SAY_AGGRO                   = 1,
        EMOTE_SUBMERGE              = 2,
        EMOTE_BURROWER              = 3,
        EMOTE_EMERGE                = 4,
        SAY_LEECHING_SWARM          = 5,
        EMOTE_LEECHING_SWARM        = 6,
        SAY_KILL_PLAYER             = 7,
        SAY_DEATH                   = 8,

        EMOTE_SPIKE                 = 0
    };

    enum Summons
    {
        NPC_FROST_SPHERE            = 34606,
        NPC_BURROW                  = 34862,
        NPC_BURROWER                = 34607,
        NPC_SCARAB                  = 34605,
        NPC_SPIKE                   = 34660
    };

    enum BossSpells
    {
        SPELL_FREEZE_SLASH          = 66012,
        SPELL_PENETRATING_COLD      = 66013,
        SPELL_LEECHING_SWARM        = 66118,
        SPELL_LEECHING_SWARM_HEAL   = 66125,
        SPELL_LEECHING_SWARM_DMG    = 66240,
        SPELL_MARK                  = 67574,
        SPELL_SPIKE_CALL            = 66169,
        SPELL_SUBMERGE_ANUBARAK     = 65981,
        SPELL_CLEAR_ALL_DEBUFFS     = 34098,
        SPELL_EMERGE_ANUBARAK       = 65982,
        SPELL_SUMMON_BEATLES        = 66339,
        SPELL_SUMMON_BURROWER       = 66332,

        // Burrow
        SPELL_CHURNING_GROUND       = 66969,

        // Scarab
        SPELL_DETERMINATION         = 66092,
        SPELL_ACID_MANDIBLE         = 65774, //Passive - Triggered

        // Burrower
        SPELL_SPIDER_FRENZY         = 66128,
        SPELL_EXPOSE_WEAKNESS       = 67720, //Passive - Triggered
        SPELL_SHADOW_STRIKE         = 66134,
        SPELL_SUBMERGE_EFFECT       = 68394,
        SPELL_AWAKENED              = 66311,
        SPELL_EMERGE_EFFECT         = 65982,

        SPELL_PERSISTENT_DIRT       = 68048,

        SUMMON_SCARAB               = NPC_SCARAB,
        SUMMON_FROSTSPHERE          = NPC_FROST_SPHERE,
        SPELL_BERSERK               = 26662,

        //Frost Sphere
        SPELL_FROST_SPHERE          = 67539,
        SPELL_PERMAFROST            = 66193,
        SPELL_PERMAFROST_DUMMY      = 65872,
        SPELL_PERMAFROST_VISUAL     = 65882,
        SPELL_PERMAFROST_MODEL      = 66185,

        //Spike
        SPELL_SUMMON_SPIKE          = 66169,
        SPELL_SPIKE_SPEED1          = 65920,
        SPELL_SPIKE_TRAIL           = 65921,
        SPELL_SPIKE_SPEED2          = 65922,
        SPELL_SPIKE_SPEED3          = 65923,
        SPELL_SPIKE_FAIL            = 66181,
        SPELL_SPIKE_TELE            = 66170
    };

#define SPELL_PERMAFROST_HELPER getRaidModeValue(66193, 67855, 67856, 67857)

    enum SummonActions
    {
        ACTION_SHADOW_STRIKE        = 0,
        ACTION_SCARAB_SUBMERGE      = 1
    };

    const LocationVector SphereSpawn[6] =
    {
        {779.8038f, 150.6580f, 158.1426f, 0},
        {736.0243f, 113.4201f, 158.0226f, 0},
        {712.5712f, 160.9948f, 158.4368f, 0},
        {701.4271f, 126.4740f, 158.0205f, 0},
        {747.9202f, 155.0920f, 158.0613f, 0},
        {769.6285f, 121.1024f, 158.0504f, 0},
    };

    enum MovementPoints
    {
        POINT_FALL_GROUND           = 1
    };

    enum PursuingSpikesPhases
    {
        PHASE_NO_MOVEMENT           = 0,
        PHASE_IMPALE_NORMAL         = 1,
        PHASE_IMPALE_MIDDLE         = 2,
        PHASE_IMPALE_FAST           = 3
    };

    enum Phases
    {
        // Anub'arak
        PHASE_MELEE                 = 1,
        PHASE_SUBMERGED             = 2
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Anubarak
class AnubarakAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit AnubarakAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* /*_target*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void justReachedSpawn() override;
    void DoAction(int32_t action) override;
    void onSummonedCreature(Creature* summon) override;

protected:
    void submerge(CreatureAIFunc pThis);
    void emerge(CreatureAIFunc pThis);
    void summonFrostSphere(CreatureAIFunc pThis);
    void summonScarabs(CreatureAIFunc pThis);

    std::vector<uint64_t> burrowGuids;
    std::vector<uint64_t> sphereGuids;

    bool introDone;
    bool reachedPhase3;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Swarm Scrab
class SwarmScrabAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SwarmScrabAI(Creature* pCreature);

    void InitOrReset() override;
    void OnDied(Unit* _killer) override;

    void DoAction(int32_t action) override;

    void AIUpdate(unsigned long time_passed) override;
protected:
    int32_t determinationTimer = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Burrower
class BurrowerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BurrowerAI(Creature* pCreature);

    void InitOrReset() override;

    void DoAction(int32_t action) override;

    void AIUpdate(unsigned long time_passed) override;

protected:
    int32_t submergeTimer = 30 * TimeVarsMs::Second;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Frost Sphere
class FrostSphereAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit FrostSphereAI(Creature* pCreature);

    void InitOrReset() override;

    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override;
    void OnReachWP(uint32_t type, uint32_t id) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spikes
class SpikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit SpikeAI(Creature* pCreature);

    void InitOrReset() override;
    void OnCombatStart(Unit* target) override;
    void AIUpdate(unsigned long time_passed) override;
    bool canAttackTarget(Unit* target) override;
    void DamageTaken(Unit* /*_attacker*/, uint32_t* damage) override;
    void startChase(Unit* target);

    void handlePermafrostHit(Creature* pCreature);

protected:
    int32_t phaseTimer = 1;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Permafrost
bool PermafrostDummySpell(uint8_t /*effectIndex*/, Spell* pSpell);
