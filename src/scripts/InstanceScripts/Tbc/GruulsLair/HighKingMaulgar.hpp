/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/Script/CreatureAIScript.hpp"

enum MaulgarPhases
{
    PHASE_1                         = 1,
    PHASE_2                         = 2
};

enum MaulgarEvents
{
    EVENT_BLASTWAVE                 = 1
};

enum MaulgarYells
{
    MAUL_SAY_AGGRO                  = 4830,
    MAUL_SAY_ENRAGE                 = 4831,
    MAUL_SAY_OGRE_DEATH_01          = 4832,
    MAUL_SAY_OGRE_DEATH_02          = 4833,
    MAUL_SAY_OGRE_DEATH_03          = 4834,
    MAUL_SAY_OGRE_DEATH_04          = 4835,
    MAUL_SAY_SLAY_01                = 4836,
    MAUL_SAY_SLAY_02                = 4837,
    MAUL_SAY_SLAY_03                = 4838,
    MAUL_SAY_DEATH                  = 4839,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Maulgar
class HighKingMaulgarAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit HighKingMaulgarAI(Creature* pCreature);

    void OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/) override;
    void OnScriptPhaseChange(uint32_t phaseId) override;
    void DoAction(int32_t actionId) override;

protected:
    std::vector<uint32_t> emoteVector;
    CreatureAISpells* mArcingSmash;
    CreatureAISpells* mMightyBlow;
    CreatureAISpells* mWhrilwind;
    CreatureAISpells* mBerserker;
    CreatureAISpells* mRoar;
    CreatureAISpells* mFlurry;
    CreatureAISpells* mDualWield;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Kiggler
class KigglerTheCrazedAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit KigglerTheCrazedAI(Creature* pCreature);

    void OnDied(Unit* mKiller) override;
    void AIUpdate(unsigned long time_passed) override;

protected:
    CreatureAISpells* mGreaterPolymorph;
    CreatureAISpells* mLightningBolt;
    CreatureAISpells* mArcaneShock;
    CreatureAISpells* mArcaneExplosion;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Blind eye
class BlindeyeTheSeerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit BlindeyeTheSeerAI(Creature* pCreature);

    void OnDied(Unit* mKiller) override;

protected:
    CreatureAISpells* mPrayerOfHealing;
    CreatureAISpells* mGreaterPowerWordShield;
    CreatureAISpells* mHeal;
};

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Olm
class OlmTheSummonerAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit OlmTheSummonerAI(Creature* pCreature);

    void OnDied(Unit* mKiller) override;

protected:
    CreatureAISpells* mDeathCoil;
    CreatureAISpells* mSummon;
    CreatureAISpells* mDarkDecay;
};

/* He will first spellshield on himself, and recast every 30 sec,
   then spam great fireball to the target, also if there is any unit
   close to him (15yr) he'll cast blast wave
*/

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Krosh
class KroshFirehandAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* pCreature);
    explicit KroshFirehandAI(Creature* pCreature);

    void OnCombatStart(Unit* pTarget) override;
    void AIUpdate(unsigned long time_passed) override;
    void OnDied(Unit* mKiller) override;

protected:
    CreatureAISpells* mSpellShield;
    CreatureAISpells* mGreaterFireball;
    CreatureAISpells* mBlastWave;
};
