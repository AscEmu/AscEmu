/*
Copyright (c) 2014-2022 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Raid_IceCrownCitadel.h"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lord Marrowgar
class LordMarrowgarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LordMarrowgarAI(Creature* pCreature);


    void IntroStart();

    void OnCombatStart(Unit* pTarget) override;
    void OnCombatStop(Unit* pTarget) override;
    void Reset();
    void AIUpdate(unsigned long time_passed) override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;

    LocationVector const* GetLastColdflamePosition() const;
    void SetLastColdflamePosition(LocationVector pos);

    void SetCreatureData64(uint32_t Type, uint64_t Data) override;
    uint64_t GetCreatureData64(uint32_t Type) const override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    InstanceScript* mInstance;
    float baseSpeed;
    bool introDone;
    bool boneSlice;

    Unit* boneStormtarget;
    LocationVector coldflameLastPos;
    uint64_t coldflameTarget;
    std::vector<uint64_t> boneSpikeImmune;

    // Spells
    CreatureAISpells* boneSliceSpell;
    CreatureAISpells* boneStormSpell;
    CreatureAISpells* boneSpikeGraveyardSpell;
    CreatureAISpells* coldflameNormalSpell;
    CreatureAISpells* coldflameBoneStormSpell;
    CreatureAISpells* berserkSpell;

    uint32_t boneStormDuration;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cold Flame
class ColdflameAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit ColdflameAI(Creature* pCreature);

    void OnLoad() override;
    void OnSummon(Unit* summoner) override;
    void AIUpdate(unsigned long time_passed) override;

protected:
    // Common
    InstanceScript* mInstance;

    //Spells
    CreatureAISpells* coldflameTriggerSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Bone Spike
class BoneSpikeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BoneSpikeAI(Creature* pCreature);
    
    void OnSummon(Unit* summoner) override;
    void OnTargetDied(Unit* pTarget) override;
    void OnDied(Unit* /*pTarget*/) override;
    void AIUpdate(unsigned long time_passed) override;

protected:
    // Common
    InstanceScript* mInstance;

    // Summon
    Unit* summon;

    bool hasTrappedUnit;
};
