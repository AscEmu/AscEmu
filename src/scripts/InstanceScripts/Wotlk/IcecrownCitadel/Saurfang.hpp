/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Raid_IceCrownCitadel.hpp"
#include "Server/Script/AchievementScript.hpp"
#include "Spell/SpellScript.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Deathbringer Suarfang
class MuradinSeGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;

protected:
    IceCrownCitadelScript* pInstance;
};

class MuradinSaurfangEvent : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit MuradinSaurfangEvent(Creature* pCreature);

    void OnCombatStop(Unit* /*_target*/) override;
    void Reset();
    void AIUpdate(unsigned long time_passed) override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;
    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    std::list<Creature*> _guardList;
};

class OverlordSeGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override;
    void onSelectOption(Object* pObject, Player* pPlayer, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override;
};

class OverlordSaurfangEvent : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit OverlordSaurfangEvent(Creature* pCreature);

    void OnCombatStop(Unit* /*_target*/) override;
    void Reset();
    void AIUpdate(unsigned long time_passed) override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;
    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    std::list<Creature*> _guardList;
};

class DeathbringerSaurfangAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit DeathbringerSaurfangAI(Creature* pCreature);

    void clearMarksFromTargets();
    void OnCombatStop(Unit* /*_target*/) override;
    void Reset();
    void AIUpdate(unsigned long time_passed) override;
    void OnScriptPhaseChange(uint32_t _phaseId) override;
    void DamageTaken(Unit* _attacker, uint32_t* damage)  override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;
    void DoAction(int32_t const action) override;
    void onSummonedCreature(Creature* summon) override;
    void OnSummonDies(Creature* summon, Unit* /*killer*/) override;
    void OnCastSpell(uint32_t _spellId) override;
    void OnSpellHitTarget(Object* target, SpellInfo const* info) override;
    uint32_t GetCreatureData(uint32_t type) const override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    bool _introDone;
    bool _frenzied;
    bool _dead;
    uint32_t FightWonValue;

    std::vector<uint64_t> _markedTargetGuids;

    // Spells
    CreatureAISpells* GripOfAgonySpell;
    CreatureAISpells* BerserkSpell;
    CreatureAISpells* SummonBloodBeast;
    CreatureAISpells* SummonBloodBeast25;
    CreatureAISpells* BoilingBloodSpell;
    CreatureAISpells* BloodNovaSpell;
    CreatureAISpells* RuneOfBloodSpell;
    CreatureAISpells* ScentOfBloodSpell;
    CreatureAISpells* ZeroPowerSpell;
    CreatureAISpells* BloodLinkSpell;
    CreatureAISpells* BloodPowerSpell;
    CreatureAISpells* MarkOfTheFallenSpell;
    CreatureAISpells* MarkOfTheFallenSpell_Self;
    CreatureAISpells* RuneOfBloodSSpell;
    CreatureAISpells* FrenzySpell;
    CreatureAISpells* RemoveMarksSpell;
    CreatureAISpells* AchievementSpell;
    CreatureAISpells* ReputationBossSpell;
    CreatureAISpells* PermanentFeignSpell;
};

class NpcSaurfangEventAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit NpcSaurfangEventAI(Creature* pCreature);

    void SetCreatureData(uint32_t type, uint32_t data) override;
    void OnHitBySpell(uint32_t _spellId, Unit* /*_caster*/) override;
    void OnReachWP(uint32_t type, uint32_t iWaypointId) override;
    void DoAction(int32_t const action) override;

protected:
    // Common
    IceCrownCitadelScript* mInstance;
    uint32_t _index;
};

class GripOfAgony : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effectIndex, std::vector<uint64_t>* effectTargets) override;
};

// Blood Nova and Rune of Blood should cast Blood Link dummy on Saurfang
class GenericBloodLinkTrigger : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override;
};

class BoilingBlood : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t /*effectIndex*/, std::vector<uint64_t>* effectTargets) override;
    SpellScriptExecuteState onAuraPeriodicTick(Aura* aur, AuraEffectModifier* /*aurEff*/, float_t* /*damage*/) override;
};

class BloodNova : public SpellScript
{
public:
    void filterEffectTargets(Spell* spell, uint8_t effIndex, std::vector<uint64_t>* effectTargets) override;
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override;

private:
    uint64_t randomTargetGuid = 0;
};

class BloodLink : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* /*aurEff*/, bool /*apply*/) override;
};

class BloodLinkDummy : public SpellScript
{
public:
    SpellScriptExecuteState onDoProcEffect(SpellProc* spellProc, Unit* /*victim*/, SpellInfo const* /*castingSpell*/, DamageInfo /*damageInfo*/) override;
    SpellCastResult onCanCast(Spell* spell, uint32_t* /*parameter1*/, uint32_t* /*parameter2*/) override;
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override;
};

class BloodLinkEnergize : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t /*effIndex*/) override;
};

class RemoveMarksOfTheFallen : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex) override;
};

class achievement_ive_gone_and_made_a_mess : public AchievementCriteriaScript
{
public:
    bool canCompleteCriteria(uint32_t criteriaID, Player* /*pPlayer*/, Object* target) override;
};
