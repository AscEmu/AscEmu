/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/SpellScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Lady Deathwhisper
class LadyDeathwhisperAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit LadyDeathwhisperAI(Creature* pCreature);

    void IntroStart();
    void OnCombatStart(Unit* /*pTarget*/) override;
    void OnCombatStop(Unit* /*_target*/) override;
    void DoAction(int32_t const action) override;
    void Reset();
    void AIUpdate(unsigned long time_passed) override;
    // summoning function for first phase
    void SummonWavePhaseOne();
    // summoning function for second phase
    void SummonWavePhaseTwo();
    void Summon(uint32_t entry, const LocationVector& pos);
    void DeleteSummons();
    void ReanimateCultist();
    void EmpowerCultist();
    void SetCreatureData64(uint32_t Type, uint64_t Data) override;

protected:
    // Common
    InstanceScript* mInstance;
    uint64_t nextVengefulShadeTargetGUID;
    std::deque<uint64_t> reanimationQueue;
    uint32_t waveCounter;
    uint8_t dominateMindCount;
    bool introDone;
    std::list<Creature*> summons;

    // Spells
    CreatureAISpells* shadowChannelingSpell;
    CreatureAISpells* manaBarrierSpell;
    CreatureAISpells* deathAndDecaySpell;
    CreatureAISpells* dominateMindHeroSpell;
    CreatureAISpells* shadowBoltSpell;
    CreatureAISpells* frostBoltSpell;
    CreatureAISpells* frostBoltVolleySpell;
    CreatureAISpells* touchOfInsignifcanceSpell;
    CreatureAISpells* summonShadeSpell;
    CreatureAISpells* berserkSpell;
    CreatureAISpells* darkMartydromSpell;
    CreatureAISpells* darkTransformationSpell;
    CreatureAISpells* darkEmpowermentSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Mana Barrier
class ManaBarrier : public SpellScript
{
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/) override;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Adherent
class CultAdherentAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit CultAdherentAI(Creature* pCreature);

    void OnLoad() override;

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* frostFeverSpell;
    CreatureAISpells* deathchillSpell;
    CreatureAISpells* curseOfTorporSpell;
    CreatureAISpells* shroudOfTheOccultSpell;
    CreatureAISpells* cultistDarkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Misc: Cult Fanatic
class CultFanaticAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit CultFanaticAI(Creature* pCreature);

    void OnLoad() override;

protected:
    // Common
    InstanceScript* mInstance;

    // Spells
    CreatureAISpells* temporalVisualSpell;
    CreatureAISpells* necroticStrikeSpell;
    CreatureAISpells* shadowCleaveSpell;
    CreatureAISpells* vampireMightSpell;
    CreatureAISpells* darkMartyrdomSpell;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Cultist Dark Martyrdom
class DarkMartyrdom : public SpellScript
{
public:
    void afterSpellEffect(Spell* spell, uint8_t effIndex) override;
};
