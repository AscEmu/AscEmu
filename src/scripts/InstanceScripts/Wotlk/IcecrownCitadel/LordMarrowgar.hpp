/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Spell/SpellScript.hpp"

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

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm
class BoneStorm : public SpellScript
{
public:
    void onAuraCreate(Aura* aur) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Storm Damage
class BoneStormDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Spike Graveyard
class BoneSpikeGraveyard : public SpellScript
{
public:
    void onAuraApply(Aura* aur) override;
    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override;
    Unit* GetRandomTargetNotMainTank(Creature* caster);
    bool CheckTarget(Unit* target, CreatureAIScript* creatureAI);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame
class Coldflame : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override;
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Bone Storm
class ColdflameBonestorm : public SpellScript
{
public:
    SpellScriptExecuteState beforeSpellEffect(Spell* spell, uint8_t effectIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Coldflame Damage
class ColdflameDamage : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override;
    bool CanBeAppliedOn(Unit* target, Spell* spell);
    SpellScriptExecuteState beforeSpellEffect(Spell* /*spell*/, uint8_t effectIndex) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Bone Slice
class BoneSlice : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override;
    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override;
    void filterEffectTargets(Spell* /*spell*/, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override;
    void afterSpellEffect(Spell* spell, uint8_t effIndex) override;

    uint32_t targetCount;
};
