/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Instance_TheVioletHold.hpp"
#include "Server/Script/AchievementScript.hpp"

namespace Ichron
{
    enum Spells
    {
        SPELL_WATER_BLAST                   = 54237,
        SPELL_WATER_BOLT_VOLLEY             = 54241,
        SPELL_SPLATTER                      = 54259,
        SPELL_PROTECTIVE_BUBBLE             = 54306,
        SPELL_FRENZY                        = 54312,
        SPELL_BURST                         = 54379,
        SPELL_DRAINED                       = 59820,
        SPELL_THREAT_PROC                   = 61732,
        SPELL_SHRINK                        = 54297,

        SPELL_WATER_GLOBULE_SUMMON_1        = 54258,
        SPELL_WATER_GLOBULE_SUMMON_2        = 54264,
        SPELL_WATER_GLOBULE_SUMMON_3        = 54265,
        SPELL_WATER_GLOBULE_SUMMON_4        = 54266,
        SPELL_WATER_GLOBULE_SUMMON_5        = 54267,
        SPELL_WATER_GLOBULE_TRANSFORM       = 54268,
        SPELL_WATER_GLOBULE_VISUAL          = 54260,

        SPELL_MERGE                         = 54269,
        SPELL_SPLASH                        = 59516
    };

    enum Yells
    {
        SAY_AGGRO                           = 4532,
        SAY_SLAY1                           = 4533,
        SAY_SLAY2                           = 4534,
        SAY_SLAY3                           = 4535,
        SAY_DEATH                           = 4536,
        SAY_SPAWN                           = 4537,
        SAY_ENRAGE                          = 4538,
        SAY_SHATTER                         = 4539,
        SAY_BUBBLE                          = 4540
    };

    enum Actions
    {
        ACTION_WATER_GLOBULE_HIT            = 1,
        ACTION_PROTECTIVE_BUBBLE_SHATTERED  = 2,
        ACTION_DRAINED                      = 3
    };

    enum Misc
    {
        DATA_DEHYDRATION                    = 1
    };
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Ichron AI
class IchronAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit IchronAI(Creature* pCreature);

    void OnLoad() override;
    void OnCombatStop(Unit* /*_target*/) override;
    void OnDied(Unit* /*_killer*/) override;
    void DoAction(int32_t /*action*/) override;
    void justReachedSpawn() override;

    void AIUpdate(unsigned long /*time_passed*/) override;

    void onSummonedCreature(Creature* /*summon*/) override;
    void OnSummonDespawn(Creature* /*summon*/) override;

    uint32_t GetCreatureData(uint32_t /*type*/) const override;

    void initialize();

protected:
    InstanceScript* mInstance;

    bool mIsFrenzy = false;
    bool mDehydration = true;
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Ichron Globule AI
class IchronGlobuleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit IchronGlobuleAI(Creature* pCreature);

    void DamageTaken(Unit* /*_attacker*/, uint32_t* /*damage*/) override;
    void OnHitBySpell(uint32_t /*_spellId*/, Unit* /*_caster*/) override;
    void OnReachWP(uint32_t /*type*/, uint32_t /*pointId*/) override;

protected:
    InstanceScript* mInstance;
    bool mSplashTriggered = false;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 54269 - Merge
class IchronMerge : public SpellScript
{
public:
    SpellScriptCheckDummy onDummyOrScriptedEffect(Spell* spell, uint8_t effIndex);
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: 54306 - Protective Bubble
class IchronBubble : public SpellScript
{
public:
    SpellScriptCheckDummy onAuraDummyEffect(Aura* aur, AuraEffectModifier* aurEff, bool apply);
};

//////////////////////////////////////////////////////////////////////////////////////////
//  Dehydration Achievement
class achievement_Dehydration : public AchievementCriteriaScript
{
public:
    bool canCompleteCriteria(uint32_t criteriaID, Player* /*pPlayer*/, Object* target) override;
};
