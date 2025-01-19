/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"
#include "Spell/SpellScript.hpp"

enum EventsGruul
{
    EVENT_SHATTER                   = 1,
    EVENT_HURTFUL_STRIKE            = 2
};

enum YellsGruul
{
    GRUUL_SAY_AGGRO                 = 4820,
    GRUUL_SAY_SLAM_01               = 4821,
    GRUUL_SAY_SLAM_02               = 4822,
    GRUUL_SAY_SHATTER_01            = 4823,
    GRUUL_SAY_SHATTER_02            = 4824,
    GRUUL_SAY_SLAY_01               = 4825,
    GRUUL_SAY_SLAY_02               = 4826,
    GRUUL_SAY_SLAY_03               = 4827,
    GRUUL_SAY_DEATH                 = 4828,
    GRUUL_EMOTE_GROW                = 4829,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Gruul the Dragonkiller
class GruulTheDragonkillerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit GruulTheDragonkillerAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget) override;
    void OnCastSpell(uint32_t spellId) override;
    void AIUpdate(unsigned long time_passed) override;

protected:
    CreatureAISpells* mGrowth;
    CreatureAISpells* mHurtfulStrike;
    CreatureAISpells* mGroundSlam;
    CreatureAISpells* mCaveIn;
    CreatureAISpells* mShatter;
    CreatureAISpells* mReverberation;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Ground Slam
bool GroundSlamEffect(uint8_t /*effectIndex*/, Spell* pSpell);

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell Effect: Shatter
bool ShatterEffect(uint8_t /*effectIndex*/, Spell* pSpell);

//////////////////////////////////////////////////////////////////////////////////////////
/// Spell: Shatter Damage
class ShatterDamage : public SpellScript
{
public:
    SpellScriptEffectDamage doCalculateEffect(Spell* spell, uint8_t effIndex, int32_t* dmg) override;
};
