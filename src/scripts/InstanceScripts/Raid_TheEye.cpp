/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2008-2015 Sun++ Team <http://www.sunplusplus.info/>
 * Copyright (C) 2005-2007 Ascent Team
 * Copyright (C) 2007-2015 Moon++ Team <http://www.moonplusplus.info/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// \todo move most defines to enum, text to db (use SendScriptTextChatMessage(ID))
#include "Setup.h"
#include "Objects/Faction.h"


//////////////////////////////////////////////////////////////////////////////////////////
//Bosses

const uint32 CN_VOID_REAVER = 19516;

const uint32 VOID_REAVER_POUNDING = 34164;
const uint32 VOID_REAVER_ARCANE_ORB = 34190;
const uint32 VOID_REAVER_ARCANE_ORB_TRIGGER = 34172;
const uint32 VOID_REAVER_KNOCK_AWAY = 25778;
const uint32 VOID_REAVER_ENRAGE = 27680; // Needs checking (as it can be wrong [or maybe IS wrong])

class VoidReaverAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VoidReaverAI);
        VoidReaverAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto pPounding = addAISpell(VOID_REAVER_POUNDING, 100.0f, TARGET_SELF, 0, 12);
            if (pPounding != nullptr)
            {
                pPounding->addEmote("Alternative measure commencing...", CHAT_MSG_MONSTER_YELL, 11218);
                pPounding->addEmote("Calculating force parameters...", CHAT_MSG_MONSTER_YELL, 11219);
            }

            mArcaneOrb = addAISpell(VOID_REAVER_ARCANE_ORB_TRIGGER, 0.0f, TARGET_RANDOM_DESTINATION, 0, 3);
            addAISpell(VOID_REAVER_KNOCK_AWAY, 100.0f, TARGET_ATTACKING, 0, 20);

            mLocaleEnrageSpell = addAISpell(VOID_REAVER_ENRAGE, 0.0f, TARGET_SELF, 0, 600);

            addEmoteForEvent(Event_OnCombatStart, 8867);
            addEmoteForEvent(Event_OnTargetDied, 8868);
            addEmoteForEvent(Event_OnTargetDied, 8869);
            addEmoteForEvent(Event_OnTargetDied, 8870);
            addEmoteForEvent(Event_OnDied, 8871);

            mArcaneOrbTimer = 0;
            mLocaleEnrageTimerId = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            mLocaleEnrageTimerId = _addTimer(600000);

            if (mArcaneOrb != nullptr)
            {
                mArcaneOrbTimer = _addTimer(10000);
            }
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(mArcaneOrbTimer))
            {
                _castAISpell(mArcaneOrb);
                _removeTimer(mArcaneOrbTimer);
            }  

            if (_isTimerFinished(mLocaleEnrageTimerId))
            {
                _castAISpell(mLocaleEnrageSpell);
                _removeTimer(mLocaleEnrageTimerId);
            }
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            _removeTimer(mLocaleEnrageTimerId);
        }

        CreatureAISpells* mLocaleEnrageSpell;
        uint32_t mLocaleEnrageTimerId;

        uint32 mArcaneOrbTimer;
        CreatureAISpells* mArcaneOrb;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//High Astromancer Solarian AI Script
//
// Phase timers based on boss mods:
// - Split every 90sec (except first split happens 50sec after engage)
// - 3x4 Solarium Agents spawns 6sec after split
// - Solarian comes back with two Solarium Priest 22sec after split (end of phase 2)
// - Once phase 2 is finished, phase 1 starts again
// - At 20% health, Solarian enter phase 3 until she dies
//
const uint32 CN_SOLARIAN = 18805;
const uint32 CN_SOLARIUMAGENT = 18925;
const uint32 CN_SOLARIUMPRIEST = 18806;
const uint32 CN_SOLARIUM_SPOT_LIGHT = 15631;
const uint32 SOLARIAN_WRATH_OF_THE_ASTROMANCER = 42783;    //Infuses an enemy with Arcane power, causing them to harm nearby allies for 5400 to 6600. Arcane damage after 6 sec.
const uint32 SOLARIAN_WRATH_OF_THE_ASTROMANCER_BOMB = 42787;    //The actual spell that triggers the explosion with arcane damage and slow fall
const uint32 SOLARIAN_ARCANE_MISSILES = 33031;   //Launches magical missiles at an enemy, inflicting Arcane damage each second for 3 sec. Trigger spell (3000 arcane damage)
const uint32 SOLARIAN_BLINDING_LIGHT = 33009;    //Hits everyone in the raid for 2280 to 2520 arcane damage. 20sec cooldown.
const uint32 SOLARIAN_SOLARIANS_TRANSFORM = 39117;    //Transforms into void walker.
const uint32 SOLARIAN_VOID_BOLT = 39329;    //The Void Walker casts this every 10 seconds. It deals 4394 to 5106 shadow damage to the target with the highest aggro.
const uint32 SOLARIAN_PSYCHIC_SCREAM = 34322;    //Fears up to 5 targets in melee range.
const uint32 SOLARIUMPRIEST_GREATER_HEAL = 38580;    //Heals 23125 to 26875 any friendly target
const uint32 SOLARIUMPRIEST_HOLY_SMITE = 31740;   //Deals 553 to 747 holy damage

class HighAstromancerSolarianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighAstromancerSolarianAI);
        HighAstromancerSolarianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //Initialize timers
            mSplitTimer = mAgentsTimer = mSolarianTimer = 0;

            //Phase 1 spells
            auto arcanMissile = addAISpell(SOLARIAN_ARCANE_MISSILES, 60.0f, TARGET_RANDOM_SINGLE, 3, 0);
            arcanMissile->setMinMaxDistance(0.0f, 45.0f);
            arcanMissile->setAvailableForScriptPhase({ 1 });

            auto wrath = addAISpell(SOLARIAN_WRATH_OF_THE_ASTROMANCER, 20.0f, TARGET_RANDOM_SINGLE, 0, 6);
            wrath->setMinMaxDistance(0.0f, 50000.0f);
            wrath->setAvailableForScriptPhase({ 1 });

            auto blindingLight = addAISpell(SOLARIAN_BLINDING_LIGHT, 20.0f, TARGET_SELF, 0, 20);
            blindingLight->setAvailableForScriptPhase({ 1 });

            /* mDisappear = AddSpellFunc(&SpellFunc_Solarian_Disappear, TARGET_SELF, 0, 22, 0);
            mDisappear->addEmote("You are hopelessly outmatched!", CHAT_MSG_MONSTER_YELL, 11139);
            mDisappear->addEmote("I will crush your delusions of grandeur!", CHAT_MSG_MONSTER_YELL, 11140);*/

            //Phase 2 spells
            /*mReappear = AddSpellFunc(&SpellFunc_Solarian_Reappear, TARGET_SELF, 0, 0, 0);*/

            //Phase 3 spells
            auto bolt = addAISpell(SOLARIAN_VOID_BOLT, 100.0f, TARGET_ATTACKING, 3, 10);
            bolt->setMinMaxDistance(0.0f, 100.0f);
            bolt->setAvailableForScriptPhase({ 3 });

            auto psychicScream = addAISpell(SOLARIAN_PSYCHIC_SCREAM, 10.0f, TARGET_SELF);
            psychicScream->setAvailableForScriptPhase({ 3 });

            mVoidForm = addAISpell(SOLARIAN_SOLARIANS_TRANSFORM, 0.0f, TARGET_SELF);
            mVoidForm->addEmote("Enough of this! Now I call upon the fury of the cosmos itself.", CHAT_MSG_MONSTER_YELL);
            mVoidForm->addEmote("I become ONE... with the VOID!", CHAT_MSG_MONSTER_YELL);

            //Emotes
            addEmoteForEvent(Event_OnCombatStart, 8872);
            addEmoteForEvent(Event_OnDied, 8873);
            addEmoteForEvent(Event_OnTargetDied, 8874);
            addEmoteForEvent(Event_OnTargetDied, 8875);
            addEmoteForEvent(Event_OnTargetDied, 8876);

            isNotInitialPhase = false;
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            mSplitTimer = _addTimer(50000);    //First split after 50sec
        }

        void AIUpdate() override
        {
            if (isScriptPhase(1))
            {
                if (_getHealthPercent() <= 20 && !_isCasting())
                {
                    setScriptPhase(3);
                    _cancelAllTimers();
                }
                else if (_isTimerFinished(mSplitTimer) && !_isCasting())
                {
                    setScriptPhase(2);
                    _resetTimer(mSplitTimer, 90000);        //Next split in 90sec
                    mAgentsTimer = _addTimer(6000);        //Agents spawns 6sec after the split
                    mSolarianTimer = _addTimer(22000);    //Solarian with 2 priests spawns 22sec after split
                }
            }
            else if (isScriptPhase(2))
            {
                if (_isTimerFinished(mSolarianTimer) && !_isCasting())
                {
                    isNotInitialPhase = true;
                    setScriptPhase(1);
                    _removeTimer(mSolarianTimer);
                }
                else if (_isTimerFinished(mAgentsTimer) && !_isCasting())
                {
                    for (uint8 SpawnIter = 0; SpawnIter < 4; SpawnIter++)
                    {
                        spawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[0][0], mSpawnPositions[0][1], 17, 0, getCreature()->GetFaction());
                        spawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[1][0], mSpawnPositions[1][1], 17, 0, getCreature()->GetFaction());
                        spawnCreature(CN_SOLARIUMAGENT, mSpawnPositions[2][0], mSpawnPositions[2][1], 17, 0, getCreature()->GetFaction());
                    }
                    _removeTimer(mAgentsTimer);
                }
            }
        }

        void OnScriptPhaseChange(uint32_t phaseId) override
        {
            switch (phaseId)
            {
                /*case 1:
                    if (isNotInitialPhase)
                        CastSpellNowNoScheduling(mReappear);
                    break;
                case 2:
                    CastSpellNowNoScheduling(mDisappear);
                    break;*/
                case 3:
                    _castAISpell(mVoidForm);
                    break;
                default:
                    break;
            }
        }

        CreatureAISpells* mVoidForm;
        int32 mSplitTimer;
        uint32 mAgentsTimer;
        uint32 mSolarianTimer;
        float mSpawnPositions[3][2];
        bool isNotInitialPhase;
};

bool Dummy_Solarian_WrathOfTheAstromancer(uint32 /*pEffectIndex*/, Spell* pSpell)
{
    Unit* Caster = pSpell->u_caster;
    if (!Caster) return true;

    Unit* Target = Caster->GetAIInterface()->getNextTarget();
    if (!Target) return true;

    SpellInfo* SpellInfo = sSpellCustomizations.GetSpellInfo(SOLARIAN_WRATH_OF_THE_ASTROMANCER_BOMB);
    if (!SpellInfo) return true;

    //Explode bomb after 6sec
    sEventMgr.AddEvent(Target, &Unit::EventCastSpell, Target, SpellInfo, EVENT_UNK, 6000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    return true;
}

//void SpellFunc_Solarian_Disappear(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
//{
//    HighAstromancerSolarianAI* Solarian = (pCreatureAI) ? static_cast< HighAstromancerSolarianAI* >(pCreatureAI) : nullptr;
//    if (Solarian)
//    {
//        if (pCreatureAI != nullptr)
//        {
//            pCreatureAI->_clearHateList();
//            pCreatureAI->setRooted(true);
//            pCreatureAI->setCanEnterCombat(false);
//            pCreatureAI->_applyAura(24699);     // vanish
//        }
//
//        //Spawn spot lights, and despawn them after 26sec X(400,460) Y(-340,-400)
//        Solarian->mSpawnPositions[0][0] = 400 + Util::getRandomFloat(60);
//        Solarian->mSpawnPositions[0][1] = -400 + Util::getRandomFloat(60);
//        Solarian->spawnCreatureAndGetAIScript(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[0][0], Solarian->mSpawnPositions[0][1], 17, 0)->despawn(26000);
//        Solarian->mSpawnPositions[1][0] = 400 + Util::getRandomFloat(60);
//        Solarian->mSpawnPositions[1][1] = -400 + Util::getRandomFloat(60);
//        Solarian->spawnCreatureAndGetAIScript(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[1][0], Solarian->mSpawnPositions[1][1], 17, 0)->despawn(26000);
//        Solarian->mSpawnPositions[2][0] = 400 + Util::getRandomFloat(60);
//        Solarian->mSpawnPositions[2][1] = -400 + Util::getRandomFloat(60);
//        Solarian->spawnCreatureAndGetAIScript(CN_SOLARIUM_SPOT_LIGHT, Solarian->mSpawnPositions[2][0], Solarian->mSpawnPositions[2][1], 17, 0)->despawn(26000);
//    }
//}

//void SpellFunc_Solarian_Reappear(SpellDesc* /*pThis*/, CreatureAIScript* pCreatureAI, Unit* /*pTarget*/, TargetType /*pType*/)
//{
//    HighAstromancerSolarianAI* Solarian = (pCreatureAI) ? static_cast< HighAstromancerSolarianAI* >(pCreatureAI) : nullptr;
//    if (Solarian)
//    {
//        //Spawn two priest friend to help Solarian
//        Solarian->spawnCreatureAndGetAIScript(CN_SOLARIUMPRIEST, Solarian->mSpawnPositions[0][0], Solarian->mSpawnPositions[0][1], 17, 0);
//        Solarian->spawnCreatureAndGetAIScript(CN_SOLARIUMPRIEST, Solarian->mSpawnPositions[1][0], Solarian->mSpawnPositions[1][1], 17, 0);
//        //Solarian->MoveTo(Solarian->mSpawnPositions[2][0], Solarian->mSpawnPositions[2][1], 17);    //Doesn't work quite right yet
//
//        if (pCreatureAI != nullptr)
//        {
//            pCreatureAI->setRooted(false);
//            pCreatureAI->setCanEnterCombat(true);
//            pCreatureAI->_removeAura(24699);    // vanish
//        }
//    }
//}

class SolariumAgentAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SolariumAgentAI);
        SolariumAgentAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
        }
};

class SolariumPriestAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SolariumPriestAI);
        SolariumPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto greaterHeal = addAISpell(SOLARIUMPRIEST_GREATER_HEAL, 20.0f, TARGET_RANDOM_FRIEND, 2, 0);
            greaterHeal->setMinMaxDistance(0.0f, 40.0f);

            addAISpell(SOLARIUMPRIEST_HOLY_SMITE, 80.0f, TARGET_ATTACKING, 3, 0);
        }
};

class SolariumSpotLight : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SolariumSpotLight);
        SolariumSpotLight(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            setCanEnterCombat(false);
        }
};

// Al'ar AI

class AlarAuxClass: public Object
{
    public:
        AlarAuxClass(CreatureAIScript*);
        ~AlarAuxClass();
        void Rebirth();

    protected:
        CreatureAIScript* alar;
};


// Thaladred the Darkener AI(1st advisor)
const uint32 CN_DARKENER = 20064;
const uint32 DARKENER_PSYCHIC_BLOW = 36966;
const uint32 DARKENER_SILENCE = 29943;

class DarkenerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DarkenerAI);
        DarkenerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(DARKENER_PSYCHIC_BLOW, 10.0f, TARGET_ATTACKING, 0, 20);
            addAISpell(DARKENER_SILENCE, 10.0f, TARGET_ATTACKING, 0, 15);

            addEmoteForEvent(Event_OnCombatStart, 8877);
            addEmoteForEvent(Event_OnDied, 8878);
            setCanEnterCombat(false);
            mCurrentTarget = NULL;
            mGazeSwitchTimer = 0;
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            setCanEnterCombat(true);
            SwitchTarget();

            mGazeSwitchTimer = _addTimer((Util::getRandomUInt(4) + 8) * 1000);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            mCurrentTarget = NULL;

            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            SwitchTarget();
        }

        void AIUpdate() override
        {
            if (_isTimerFinished(mGazeSwitchTimer))
            {
                _resetTimer(mGazeSwitchTimer, (Util::getRandomUInt(4) + 8) * 1000);
                if (!SwitchTarget())
                    return;
            }
        }

        bool SwitchTarget()
        {
            mCurrentTarget = getBestPlayerTarget();
            if (mCurrentTarget == getCreature()->GetAIInterface()->getNextTarget())
                return true;

            if (mCurrentTarget != NULL)
            {
                getCreature()->GetAIInterface()->modThreatByPtr(mCurrentTarget, 1000000);
                Player* pPlayer = static_cast<Player*>(mCurrentTarget);
                char msg[256];
                snprintf((char*)msg, 256, "%s sets eyes on %s", getCreature()->GetCreatureProperties()->Name.c_str(), pPlayer->GetName());
                getCreature()->SendChatMessageAlternateEntry(CN_DARKENER, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, msg);
                return true;
            }

            _wipeHateList();
            return false;
        }

        int32 mGazeSwitchTimer;
        Unit* mCurrentTarget;
};

const uint32 CN_SANGUINAR = 20060;
const uint32 SANGUINAR_BELLOWING = 36922;

class SanguinarAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SanguinarAI);
        SanguinarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SANGUINAR_BELLOWING, 100.0f, TARGET_SELF, 0, 30);

            addEmoteForEvent(Event_OnCombatStart, 8879);
            addEmoteForEvent(Event_OnDied, 8880);
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            setCanEnterCombat(true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }
};

// Grand Astromancer Capernian AI (3rd advisor)
const uint32 CN_CAPERNIAN = 20062;
const uint32 CAPERNIAN_CONFLAGRATION = 37018;
const uint32 CAPERNIAN_FIREBALL = 36971;
const uint32 CAPERNIAN_ARCANE_BURST = 36970;

class CapernianAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(CapernianAI);
        CapernianAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto conflagration = addAISpell(CAPERNIAN_CONFLAGRATION, 7.0f, TARGET_RANDOM_SINGLE, 0, 10, false, true);
            conflagration->setMinMaxDistance(0.0f, 30.0f);

            addAISpell(CAPERNIAN_FIREBALL, 73.0f, TARGET_ATTACKING, 2, 0);
            mArcaneBurst = addAISpell(CAPERNIAN_ARCANE_BURST, 0.0f, TARGET_SELF, 1, 15);

            addEmoteForEvent(Event_OnCombatStart, 8881);
            addEmoteForEvent(Event_OnDied, 8882);
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* mTarget) override
        {
            setCanEnterCombat(true);

            if (getRangeToObject(mTarget) <= 30.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }

        void AIUpdate() override
        {
            setAIAgent(AGENT_NULL);
            setRooted(false);
            Unit* pClosestTarget = getBestPlayerTarget(TargetFilter_Closest);
            if (pClosestTarget != NULL && getRangeToObject(pClosestTarget) <= 6.0f)
            {
                _castAISpell(mArcaneBurst);
            }

            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget != NULL && getRangeToObject(pTarget) <= 30.0f)
            {
                
                if (getAIAgent() != AGENT_SPELL)
                {
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }
            }
        }

        CreatureAISpells* mArcaneBurst;
};

const uint32 CN_TELONICUS = 20063;
const uint32 TELONICUS_BOMB = 37036;
const uint32 TELONICUS_REMOTE_TOY = 37027;    // doesn't seems to work like it should

class TelonicusAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TelonicusAI);
        TelonicusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            auto bomb = addAISpell(TELONICUS_BOMB, 10.0f, TARGET_RANDOM_DESTINATION, 2, 15);
            bomb->setMinMaxDistance(0.0f, 30.0f);

            auto remoteToy = addAISpell(TELONICUS_REMOTE_TOY, 10.0f, TARGET_RANDOM_SINGLE, 0, 15);
            remoteToy->setMinMaxDistance(0.0f, 30.0f);

            addEmoteForEvent(Event_OnCombatStart, 8883);
            addEmoteForEvent(Event_OnDied, 8884);            // not sure
            setCanEnterCombat(false);
        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            setCanEnterCombat(true);
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            if (isAlive())
            {
                setCanEnterCombat(false);
            }
        }
};

// Flame Strike AI
const uint32 CN_FLAME_STRIKE_TRIGGER = 21369;
const uint32 FLAME_STRIKE_TRIGGER_FLAME_STRIKE = 36731;
const uint32 FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT = 36730;

class FlameStrikeAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FlameStrikeAI);
        FlameStrikeAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            _applyAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT);
            RegisterAIUpdateEvent(5000);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            setRooted(true);
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            _removeAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE_EFFECT);
            despawn(500);
        }

        void AIUpdate() override
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _applyAura(FLAME_STRIKE_TRIGGER_FLAME_STRIKE);
            RemoveAIUpdateEvent();
            despawn(8500);
        }
};

// Phoenix AI
const uint32 CN_PHOENIX = 21362;
const uint32 PHOENIX_BURN = 36721;
const uint32 PHOENIX_REBIRTH = 35369;        // used as instant cast - but it does not show animation now (maybe it would be good to move it to trigger?)

class PhoenixAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PhoenixAI);
        PhoenixAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(1000);
            Unit* pTarget = getBestPlayerTarget();
            if (pTarget != NULL)
            {
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 500, 0);
            }

            mBurnTimer = _addTimer(3000);
        }

        void OnTargetDied(Unit* /*mTarget*/) override
        {
            Unit* pTarget = getBestPlayerTarget(TargetFilter_Closest);
            if (pTarget != NULL)
            {
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
            }
            else
            {
                despawn(1, 0);
            }
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            _applyAura(PHOENIX_REBIRTH);
            spawnCreature(21364, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
            despawn(500);
        }

        void AIUpdate() override
        {
            double CurrentHP = (double)getCreature()->getUInt32Value(UNIT_FIELD_HEALTH);
            double PercMaxHP = (double)getCreature()->getUInt32Value(UNIT_FIELD_MAXHEALTH) * 0.05;
            if (CurrentHP > PercMaxHP && _isTimerFinished(mBurnTimer))
            {
                getCreature()->SetHealth((uint32)(CurrentHP - PercMaxHP));
                _resetTimer(mBurnTimer, 3000);
                _applyAura(PHOENIX_BURN);
            }
            else if (CurrentHP <= PercMaxHP)
            {
                spawnCreature(21364, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
                despawn(500);
            }            
        }

        int32    mBurnTimer;
};

//Phoenix Egg AI
const uint32 CN_PHOENIX_EGG = 21364;

class PhoenixEggAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(PhoenixEggAI);
        PhoenixEggAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            RegisterAIUpdateEvent(15000);
            setCanEnterCombat(false);
            _setMeleeDisabled(false);
            setRooted(true);
        }

        void OnDied(Unit* /*mKiller*/) override
        {
            despawn(500);
        }

        void AIUpdate() override
        {
            spawnCreature(CN_PHOENIX, getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), getCreature()->GetOrientation());
            despawn(0);
        }
};



const uint32 NETHERSTRAND_LONGBOW = 21268;
const uint32 DEVASTATION = 21269;
const uint32 COSMIC_INFUSER = 21270;
const uint32 INFINITY_BLADE = 21271;
const uint32 WARP_SLICER = 21272;
const uint32 PHASESHIFT_BULWARK = 21273;
const uint32 STAFF_OF_DISINTEGRATION = 21274;

class WeaponsAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WeaponsAI);
        WeaponsAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            getCreature()->m_noRespawn = true;

            Unit* pTarget = getBestPlayerTarget();
            if (pTarget != NULL)
            {
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 200, 0);
            }
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            Unit* pTarget = getBestPlayerTarget();
            if (pTarget != NULL)
            {
                getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
            }
            else
            {
                despawn(1);
            }
        }
};

//\todo Add weapon summon effect
// Check why some features block (like melee, movement and so on) if there's only 1 target and spell req. NotCurrent one

//Prince Kael'Thas
const uint32 CN_KAELTHAS = 19622;

// Common spells
const uint32 KAELTHAS_FIREBALL = 36805;    // prolly wrong id
const uint32 KAELTHAS_ARCANE_DISRUPTION = 36834;
const uint32 KAELTHAS_SHOCK_BARRIER = 36815;    // timed

// Phase 4 spells
const uint32 KAELTHAS_FLAME_STRIKE_SUMMON = 36735;
const uint32 KAELTHAS_PHOENIX = 36723;
const uint32 KAELTHAS_PYROBLAST = 36819;    // timed
const uint32 KAELTHAS_MIND_CONTROL = 36797;    // timed

// Phase 5 spells
const uint32 KAELTHAS_GRAVITY_LAPSE = 35966;    // timed
const uint32 KAELTHAS_NETHER_VAPOR = 35859;
const uint32 KAELTHAS_NETHER_BEAM = 35873;    // timed along with lapse

//const uint32 KAELTHAS_GRAVITY1 = 34480;    // knockback + aura
//const uint32 KAELTHAS_GRAVITY2 = 35941;    // explosion effect

const uint32 KAELTHAS_SUMMON_WEAPONS = 36976;    //casting effect

const uint32 REMOVE_INFUSER = 39498;
const uint32 REMOVE_DEVASTATION = 39499;
const uint32 REMOVE_INFINITY = 39500;
const uint32 REMOVE_LONGBOW = 39501;
const uint32 REMOVE_BULWARK = 39502;
const uint32 REMOVE_STAFF = 39503;
const uint32 REMOVE_SLICER = 39504;

const Movement::Location Triggers[] =
{
    { 789.719543f, 24.627499f, 52.728550f },
    { 791.931152f, -24.925735f, 52.728550f },
};

const LocationExtra Advisors[] =
{
    { 785.807007f,  19.486200f, 48.911800f, 3.979350f, 20064 },
    { 785.781982f, -20.399500f, 48.911800f, 2.303830f, 20060 },
    { 792.408020f, -13.241500f, 48.911800f, 2.687810f, 20062 },
    { 792.724976f,  12.775400f, 48.911800f, 3.595380f, 20063 }
};

const LocationExtra Gates[] =
{
    { 676.777283f, -44.468628f, 46.780785f, 0.932028f, 184325 },
    { 676.812500f,  43.073757f, 46.781292f, 5.312979f, 184324 }
};

const Movement::LocationWithFlag Waypoints[] =
{
    {  },
    { 794.072998f,  0.214634f, 48.728500f, 0.0f, Movement::WP_MOVE_TYPE_RUN },
    { 794.052998f,  0.214634f, 75.728500f, 0.0f, Movement::WP_MOVE_TYPE_FLY },
    { 794.032998f,  0.214634f, 48.728500f, 0.0f, Movement::WP_MOVE_TYPE_FLY }
};

const LocationExtra KaelthasWeapons[] =
{
    { 794.38f, 15.00f, 48.72f, 2.9f, 21270 },        // [Cosmic Infuser]
    { 785.47f, 12.12f, 48.72f, 3.14f, 21269 },        // [Devastation]
    { 781.25f, 4.39f, 48.72f, 3.14f, 21271 },        // [Infinity Blade]
    { 777.38f, -0.81f, 48.72f, 3.06f, 21273 },        // [Phaseshift Bulwark]
    { 781.48f, -6.08f, 48.72f, 3.9f, 21274 },        // [Staff of Disintegration]
    { 785.42f, -13.59f, 48.72f, 3.4f, 21272 },        // [Warp Slicer]
    { 793.06f, -16.61f, 48.72f, 3.10f, 21268 }        // [Netherstrand Longbow]
};

enum AdvisorPhase
{
    PHASE_SPEECH,
    PHASE_ATTACK_COMMAND,
    PHASE_ADV_FIGHT,
};

class KaelThasAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KaelThasAI);
        KaelThasAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            for (uint8 i = 1; i < 4; ++i)
            {
                AddWaypoint(CreateWaypoint(1, 0, Waypoints[i].wp_flag, Waypoints[i].wp_location));
            }

            setCanEnterCombat(true);
            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            setRooted(false);

            // Other spells
            mSummonWeapons = addAISpell(KAELTHAS_SUMMON_WEAPONS, 0.0f, TARGET_SELF, 3, 0);
            mSummonWeapons->addEmote("As you see, I have many weapons in my arsenal...", CHAT_MSG_MONSTER_YELL, 11261);

            // Common spells
            mArcaneDisruption = addAISpell(KAELTHAS_ARCANE_DISRUPTION, 0.0f, TARGET_SELF);

            //mArcaneDisruptionFunc = AddSpellFunc(&SpellFunc_KaelThasArcaneDisruption, TARGET_RANDOM_SINGLE, 0, 0, 0);

            mShockBarrier = addAISpell(KAELTHAS_SHOCK_BARRIER, 0.0f, TARGET_SELF, 0, 60);

            auto fireball = addAISpell(KAELTHAS_FIREBALL, 10.0f, TARGET_ATTACKING, 2, 15);
            fireball->setAvailableForScriptPhase({ 7, 8 });

            // 1st phase
            mPyroblast = addAISpell(KAELTHAS_PYROBLAST, 0.0f, TARGET_ATTACKING, 4, 0);

            auto mMindControl = addAISpell(KAELTHAS_MIND_CONTROL, 100.0f, TARGET_SELF, 0, 30);
            mMindControl->setAvailableForScriptPhase({ 7 });
            mMindControl->addEmote("Obey me.", CHAT_MSG_MONSTER_YELL, 11268);
            mMindControl->addEmote("Bow to my will.", CHAT_MSG_MONSTER_YELL, 11269);

            mFlameStrike = addAISpell(KAELTHAS_FLAME_STRIKE_SUMMON, 0.0f, TARGET_RANDOM_SINGLE);

            //mFlameStrikeFunc = AddSpellFunc(&SpellFunc_KaelThasFlameStrike, TARGET_RANDOM_SINGLE, 0, 0, 0);

            mPhoenix = addAISpell(KAELTHAS_PHOENIX, 0.0f, TARGET_SELF);
            mPhoenix->addEmote("Anara'nel belore!", CHAT_MSG_MONSTER_YELL, 11267);
            mPhoenix->addEmote("By the power of the sun!", CHAT_MSG_MONSTER_YELL, 11266);

            // After powering up + Nether Vapor + Additional spells
            mNetherBeam = addAISpell(KAELTHAS_NETHER_BEAM, 0.0f, TARGET_RANDOM_SINGLE);
            mNetherBeam->setAvailableForScriptPhase({ 8 });

            addEmoteForEvent(Event_OnCombatStart, 8885);
            addEmoteForEvent(Event_OnTargetDied, 8886);
            addEmoteForEvent(Event_OnTargetDied, 8887);
            addEmoteForEvent(Event_OnTargetDied, 8888);
            addEmoteForEvent(Event_OnDied, 8889);

            mArcaneDisruptionTimer = 0;
            mShockBarrierTimer = 0;
            mFlameStrikeTimer = 0;
            mPhoenixTimer = 0;
            mEventTimer = 0;
            mAdvisorPhase = PHASE_SPEECH;

            for (uint8 i = 0; i < 4; ++i)
            {
                Creature* creature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                if (creature != NULL)
                {
                    creature->Despawn(0, 0);
                }

                spawnCreature(Advisors[i].addition, Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].o);
            }

        }

        void OnCombatStart(Unit* /*mTarget*/) override
        {
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            SetAIUpdateFreq(24000);
            
            setAIAgent(AGENT_SPELL);
            setRooted(true);

            for (uint8 i = 0; i < 2; ++i)
            {
                GameObject* pGameobject = getNearestGameObject(Gates[i].x, Gates[i].y, Gates[i].z, Gates[i].addition);
                if (pGameobject != NULL && pGameobject->GetState() == 0)
                {
                    pGameobject->SetState(GO_STATE_CLOSED);
                }
            }

            mEventTimer = mArcaneDisruptionTimer = mShockBarrierTimer = mFlameStrikeTimer = mPhoenixTimer = 0;
            mAdvisorPhase = PHASE_SPEECH;
            mAdvCoords.clear();
        }

        void OnCombatStop(Unit* /*mTarget*/) override
        {
            setRooted(false);
            getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            if (isAlive())
            {
                for (uint8 i = 0; i < 4; ++i)
                {
                    Creature* pCreature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                    if (pCreature != NULL)
                    {
                        pCreature->Despawn(0, 0);
                    }

                    spawnCreature(Advisors[i].addition, Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].o);
                }
            }

            for (uint8 i = 0; i < 2; ++i)
            {
                GameObject* pGameobject = getNearestGameObject(Gates[i].x, Gates[i].y, Gates[i].z, Gates[i].addition);
                if (pGameobject != NULL && pGameobject->GetState() == 1)
                {
                    pGameobject->SetState(GO_STATE_OPEN);
                }
            }
        }

        void SendAdvisorEmote()
        {
            switch (getScriptPhase())
            {
                case 1:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11259, "Let us see how your nerves hold up against the Darkener, Thaladred.");
                    SetAIUpdateFreq(5000);
                    break;
                case 2:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11260, "You have persevered against some of my best advisors. But none can withstand the might of the Bloodhammer. Behold, Lord Sanguinar.");
                    SetAIUpdateFreq(12000);
                    break;
                case 3:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11257, "Capernian will see to it that your stay here is a short one.");
                    SetAIUpdateFreq(5000);
                    break;
                case 4:
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11258, "Well done. You have proven worthy to test your skills against my Master Engineer, Telonicus.");
                    SetAIUpdateFreq(8000);
                    break;
            }

            mAdvisorPhase = PHASE_ATTACK_COMMAND;
        }

        void SendAdvisorToFight(Creature* pCreature)
        {
            pCreature->GetAIInterface()->SetAllowedToEnterCombat(true);
            pCreature->setUInt64Value(UNIT_FIELD_FLAGS, 0);

            Unit* pTarget = getBestPlayerTarget();
            if (pTarget != NULL)
            {
                pCreature->GetAIInterface()->AttackReaction(pTarget, 200, 0);
            }

            SetAIUpdateFreq(1000);
            mAdvisorPhase = PHASE_ADV_FIGHT;
        }

        void CheckAdvisorState(Creature* pCreature)
        {
            if (!pCreature->isAlive())
            {
                LocationExtra pCoords;
                pCoords.x = pCreature->GetPositionX();
                pCoords.y = pCreature->GetPositionY();
                pCoords.z = pCreature->GetPositionZ();
                pCoords.o = pCreature->GetOrientation();
                pCoords.addition = pCreature->GetEntry();

                SetAIUpdateFreq(5000);
                setScriptPhase(getScriptPhase() + 1);
                mAdvCoords.push_back(pCoords);
                mAdvisorPhase = PHASE_SPEECH;
            }
        }

        void AIUpdate() override
        {
            if (getScriptPhase() < 5)
            {
                uint32 i = getScriptPhase() > 0 ? getScriptPhase() - 1 : 0;
                Creature* pCreature = getNearestCreature(Advisors[i].x, Advisors[i].y, Advisors[i].z, Advisors[i].addition);
                if (pCreature == NULL || (!pCreature->isAlive() && mAdvisorPhase != PHASE_ADV_FIGHT))
                {
                    LocationExtra pCoords;
                    pCoords.x = Advisors[i].x;
                    pCoords.y = Advisors[i].y;
                    pCoords.z = Advisors[i].z;
                    pCoords.o = Advisors[i].o;
                    pCoords.addition = Advisors[i].addition;
                    mAdvCoords.push_back(pCoords);

                    setScriptPhase(getScriptPhase() + 1);
                    mAdvisorPhase = PHASE_SPEECH;
                    return;
                }
                switch (mAdvisorPhase)
                {
                    case PHASE_SPEECH:
                        SendAdvisorEmote();
                        break;
                    case PHASE_ATTACK_COMMAND:
                        SendAdvisorToFight(pCreature);
                        break;
                    case PHASE_ADV_FIGHT:
                        CheckAdvisorState(pCreature);
                        break;
                }
            }
            if (isScriptPhase(5))
            {
                if (mEventTimer == -1)
                {
                    _castAISpell(mSummonWeapons);
                    SetAIUpdateFreq(3000);
                    mEventTimer = -2;
                    return;
                }
                else if (mEventTimer == -2)
                {
                    for (uint8 i = 0; i < 7; ++i)
                    {
                        spawnCreature(KaelthasWeapons[i].addition, KaelthasWeapons[i].x, KaelthasWeapons[i].y, KaelthasWeapons[i].z, KaelthasWeapons[i].o);
                    }

                    SetAIUpdateFreq(1000);
                    mEventTimer = _addTimer(125000);
                    return;
                }
                else if (_isTimerFinished(mEventTimer))
                {
                    for (uint8 i = 0; i < 4; ++i)
                    {
                        if (mAdvCoords.size() <= (size_t)i)
                            break;

                        Creature* pCreature = getNearestCreature(mAdvCoords[i].x, mAdvCoords[i].y, mAdvCoords[i].z, mAdvCoords[i].addition);
                        if (pCreature != NULL && !pCreature->isAlive())
                        {
                            pCreature->Despawn(0, 0);
                        }

                        CreatureAIScript* pAI = spawnCreatureAndGetAIScript(mAdvCoords[i].addition, mAdvCoords[i].x, mAdvCoords[i].y, mAdvCoords[i].z, 0);
                        if (pAI != nullptr)
                        {
                            pCreature = pAI->getCreature();
                        }
                        else
                            continue;

                        if (pCreature != nullptr)
                        {
                            pCreature->GetAIInterface()->SetAllowedToEnterCombat(true);
                            pCreature->setUInt64Value(UNIT_FIELD_FLAGS, 0);
                        }
                    }

                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 11262, "Perhaps I underestimated you. It would be unfair to make you fight all four Advisors at once, but...fair treatment was never shown to my people. I'm just returning the favor.");
                    _resetTimer(mEventTimer, 180000);
                    setScriptPhase(6);
                    mAdvCoords.clear();
                }

                
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
            if (isScriptPhase(6))
            {
                
                if (_isTimerFinished(mEventTimer))
                {
                    mArcaneDisruptionTimer = _addTimer(20000);
                    mShockBarrierTimer = _addTimer(60000);
                    mFlameStrikeTimer = _addTimer(40000);
                    setCanEnterCombat(true);
                    setAIAgent(AGENT_NULL);
                    setRooted(false);
                    setScriptPhase(7);
                }
                else
                {
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }

                return;
            }
            if (isScriptPhase(7))
            {
                if (!_isCasting())
                {
                    if (getAIAgent() == AGENT_SPELL)
                    {
                        setAIAgent(AGENT_NULL);
                        setRooted(false);
                    }
                    if (_isTimerFinished(mShockBarrierTimer))
                    {
                        _castAISpell(mShockBarrier);
                        _resetTimer(mShockBarrierTimer, 70000);
                    }
                    /*else if (_isTimerFinished(mArcaneDisruptionTimer))
                    {
                        CastSpellNowNoScheduling(mArcaneDisruptionFunc);
                        _resetTimer(mArcaneDisruptionTimer, 30000);
                    }
                    else if (_isTimerFinished(mFlameStrikeTimer))
                    {
                        CastSpellNowNoScheduling(mFlameStrikeFunc);
                    }*/
                }
                if (getCreature()->HasAura(KAELTHAS_SHOCK_BARRIER))
                {
                    _castAISpell(mPyroblast);
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }
                else if (_isTimerFinished(mPhoenixTimer))        // it spawns on caster's place, but should in 20y from him
                {
                    // also it seems to not work (not always)
                    _castAISpell(mPhoenix);
                    _removeTimer(mPhoenixTimer);
                }

                
            }
        }

        Unit* GetRandomPlayer()
        {
            return getBestPlayerTarget(TargetFilter_NotCurrent);
        }

        CreatureAISpells* mSummonWeapons;
        CreatureAISpells* mArcaneDisruption;
        CreatureAISpells* mShockBarrier;
        CreatureAISpells* mPyroblast;
        CreatureAISpells* mFlameStrike;
        CreatureAISpells* mPhoenix;
        CreatureAISpells* mNetherBeam;

        AdvisorPhase mAdvisorPhase;
        int32 mArcaneDisruptionTimer;
        int32 mShockBarrierTimer;
        int32 mFlameStrikeTimer;
        uint32 mPhoenixTimer;
        int32 mEventTimer;

        std::vector<LocationExtra> mAdvCoords;
};

void SetupTheEye(ScriptMgr* mgr)
{
    //Void Reaver event
    mgr->register_creature_script(CN_VOID_REAVER, &VoidReaverAI::Create);

    //High Astromancer Solarian
    mgr->register_creature_script(CN_SOLARIAN, &HighAstromancerSolarianAI::Create);
    mgr->register_creature_script(CN_SOLARIUMAGENT, &SolariumAgentAI::Create);
    mgr->register_creature_script(CN_SOLARIUMPRIEST, &SolariumPriestAI::Create);
    mgr->register_creature_script(CN_SOLARIUM_SPOT_LIGHT, &SolariumSpotLight::Create);

    //Kael'Thas Encounter
    mgr->register_creature_script(CN_PHOENIX, &PhoenixAI::Create);
    mgr->register_creature_script(CN_PHOENIX_EGG, &PhoenixEggAI::Create);
    mgr->register_creature_script(CN_FLAME_STRIKE_TRIGGER, &FlameStrikeAI::Create);
    mgr->register_creature_script(CN_DARKENER, &DarkenerAI::Create);
    mgr->register_creature_script(CN_SANGUINAR, &SanguinarAI::Create);
    mgr->register_creature_script(CN_CAPERNIAN, &CapernianAI::Create);
    mgr->register_creature_script(CN_TELONICUS, &TelonicusAI::Create);
    mgr->register_creature_script(CN_KAELTHAS, &KaelThasAI::Create);

    // Kael'thas Weapons
    mgr->register_creature_script(NETHERSTRAND_LONGBOW, &WeaponsAI::Create);
    mgr->register_creature_script(DEVASTATION, &WeaponsAI::Create);
    mgr->register_creature_script(COSMIC_INFUSER, &WeaponsAI::Create);
    mgr->register_creature_script(INFINITY_BLADE, &WeaponsAI::Create);
    mgr->register_creature_script(WARP_SLICER, &WeaponsAI::Create);
    mgr->register_creature_script(PHASESHIFT_BULWARK, &WeaponsAI::Create);
    mgr->register_creature_script(STAFF_OF_DISINTEGRATION, &WeaponsAI::Create);
}
