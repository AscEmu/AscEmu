/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "HighKingMaulgar.hpp"
#include "Raid_GruulsLair.hpp"

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Maulgar
HighKingMaulgarAI::HighKingMaulgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mArcingSmash = addAISpell(SPELL_ARCING_SMASH, 8.0f, TARGET_ATTACKING, 0, 15);

    mMightyBlow = addAISpell(SPELL_MIGHTY_BLOW, 7.0f, TARGET_ATTACKING, 0, 30);

    mWhrilwind = addAISpell(SPELL_WHIRLWIND, 7.0f, TARGET_ATTACKING, 0, 55);

    mFlurry = addAISpell(SPELL_FLURRY, 2.0f, TARGET_ATTACKING, 0, 60);
    mFlurry->addDBEmote(MAUL_SAY_ENRAGE);

    mBerserker = addAISpell(SPELL_BERSERKER_C, 10.0f, TARGET_RANDOM_SINGLE, 0, 20);
    mBerserker->setAvailableForScriptPhase({ 2 });
    mBerserker->setMinMaxDistance(0.0f, 40.0f);

    mRoar = addAISpell(SPELL_ROAR, 8.0f, TARGET_SELF, 0, 40);
    mRoar->setAvailableForScriptPhase({ 2 });

    mDualWield = addAISpell(SPELL_DUAL_WIELD, 0.0f, TARGET_SELF);

    emoteVector.clear();
    emoteVector.push_back(MAUL_SAY_OGRE_DEATH_01);
    emoteVector.push_back(MAUL_SAY_OGRE_DEATH_02);
    emoteVector.push_back(MAUL_SAY_OGRE_DEATH_03);

    addEmoteForEvent(Event_OnCombatStart, MAUL_SAY_AGGRO);
    addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_01);
    addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_02);
    addEmoteForEvent(Event_OnTargetDied, MAUL_SAY_SLAY_03);
    addEmoteForEvent(Event_OnDied, MAUL_SAY_DEATH);
}

CreatureAIScript* HighKingMaulgarAI::Create(Creature* pCreature) { return new HighKingMaulgarAI(pCreature); }

void HighKingMaulgarAI::OnDamageTaken(Unit* /*mAttacker*/, uint32_t /*fAmount*/)
{
    if (isScriptPhase(PHASE_1) && _getHealthPercent() <= 50)
        setScriptPhase(PHASE_2);
}

void HighKingMaulgarAI::OnScriptPhaseChange(uint32_t phaseId)
{
    switch (phaseId)
    {
    case PHASE_2:
        _castAISpell(mFlurry);
        break;
    default:
        break;
    }
}

void HighKingMaulgarAI::DoAction(int32_t actionId)
{
    if (actionId == ACTION_ADD_DEATH)
        sendRandomDBChatMessage(emoteVector, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Kiggler
KigglerTheCrazedAI::KigglerTheCrazedAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mGreaterPolymorph = addAISpell(SPELL_GREATER_POLYMORPH, 8.0f, TARGET_RANDOM_SINGLE, 0, 15);
    mLightningBolt = addAISpell(SPELL_LIGHTNING_BOLT, 70.0f, TARGET_ATTACKING, 0, 15);
    mArcaneShock = addAISpell(SPELL_ARCANE_SHOCK, 10.0f, TARGET_RANDOM_SINGLE, 0, 20);
    mArcaneExplosion = addAISpell(SPELL_ARCANE_EXPLOSION, 10.0f, TARGET_SELF, 0, 30);
}

CreatureAIScript* KigglerTheCrazedAI::Create(Creature* pCreature) { return new KigglerTheCrazedAI(pCreature); }

void KigglerTheCrazedAI::OnDied(Unit* /*mKiller*/)
{
    getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
}

void KigglerTheCrazedAI::AIUpdate(unsigned long /*time_passed*/)
{
    Unit* pTarget = getCreature()->getAIInterface()->getCurrentTarget();
    if (pTarget != NULL)
    {
        if (getRangeToObject(pTarget) <= 40.0f)
        {
            //setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
        else
            setRooted(false);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Blind eye
BlindeyeTheSeerAI::BlindeyeTheSeerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mPrayerOfHealing = addAISpell(SPELL_PRAYER_OH, 5.0f, TARGET_SELF, 0, 35);
    mGreaterPowerWordShield = addAISpell(SPELL_GREATER_PW_SHIELD, 8.0f, TARGET_SELF, 0, 40);
    mHeal = addAISpell(SPELL_HEAL, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
    mHeal->setMinMaxPercentHp(0.0f, 70.0f);
}

CreatureAIScript* BlindeyeTheSeerAI::Create(Creature* pCreature) { return new BlindeyeTheSeerAI(pCreature); }

void BlindeyeTheSeerAI::OnDied(Unit* /*mKiller*/)
{
    getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Olm
OlmTheSummonerAI::OlmTheSummonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mDeathCoil = addAISpell(SPELL_DEATH_COIL, 7.0f, TARGET_RANDOM_SINGLE, 0, 10);
    mSummon = addAISpell(SPELL_SUMMON_WFH, 7.0f, TARGET_SELF, 3, 20);
    mDarkDecay = addAISpell(SPELL_DARK_DECAY, 10.0f, TARGET_RANDOM_SINGLE, 0, 15);
}

CreatureAIScript* OlmTheSummonerAI::Create(Creature* pCreature) { return new OlmTheSummonerAI(pCreature); }

void OlmTheSummonerAI::OnDied(Unit* /*mKiller*/)
{
    getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
}

/* He will first spellshield on himself, and recast every 30 sec,
   then spam great fireball to the target, also if there is any unit
   close to him (15yr) he'll cast blast wave
*/

//////////////////////////////////////////////////////////////////////////////////////////
/// Boss: Krosh
KroshFirehandAI::KroshFirehandAI(Creature* pCreature) : CreatureAIScript(pCreature)
{
    mBlastWave = addAISpell(SPELL_BLAST_WAVE, 0.0f, TARGET_SELF, 0, 15);
    mGreaterFireball = addAISpell(SPELL_GREATER_FIREBALL, 100.0f, TARGET_ATTACKING, 3, 0);
    mSpellShield = addAISpell(SPELL_SPELLSHIELD, 100.0f, TARGET_SELF, 0, 30);
}

CreatureAIScript* KroshFirehandAI::Create(Creature* pCreature) { return new KroshFirehandAI(pCreature); }

void KroshFirehandAI::OnCombatStart(Unit* /*pTarget*/)
{
    _castAISpell(mSpellShield);
    scriptEvents.addEvent(EVENT_BLASTWAVE, 20000);
}

void KroshFirehandAI::AIUpdate(unsigned long time_passed)
{
    if (_isCasting())
        return;

    scriptEvents.updateEvents(time_passed, getScriptPhase());

    while (uint32_t eventId = scriptEvents.getFinishedEvent())
    {
        switch (eventId)
        {
            case EVENT_BLASTWAVE:
            {
                Unit* unit = getBestUnitTarget(TargetFilter_Closest);
                if (unit && getRangeToObject(unit) < 15.0f)
                    _castAISpell(mBlastWave);

                scriptEvents.addEvent(EVENT_BLASTWAVE, 20000);
            }
                break;
            default:
                break;
        }
    }
}

void KroshFirehandAI::OnDied(Unit* /*mKiller*/)
{
    getLinkedCreatureAIScript()->DoAction(ACTION_ADD_DEATH);
}
