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

#include "Setup.h"

const uint32 CN_LAIR_BRUTE = 19389;
const uint32 LAIR_BRUTE_MORTALSTRIKE = 39171;
const uint32 LAIR_BRUTE_CLEAVE = 39174;
const uint32 LAIR_BRUTE_CHARGE = 24193;

void SpellFunc_LairBrute_Charge(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class LairBruteAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LairBruteAI);
        LairBruteAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(LAIR_BRUTE_CLEAVE, Target_Current, 20, 0, 15, 0, 7);
            AddSpell(LAIR_BRUTE_MORTALSTRIKE, Target_Current, 8, 0, 20, 0, 7);
            mCharge = AddSpell(LAIR_BRUTE_CHARGE, Target_Current, 0, 0, 0, 0, 40);
            AddSpellFunc(&SpellFunc_LairBrute_Charge, Target_Current, 7, 0, 35, 0, 40);
        }

        SpellDesc* mCharge;
};

void SpellFunc_LairBrute_Charge(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    LairBruteAI* pBruteAI = (pCreatureAI != NULL) ? static_cast< LairBruteAI* >(pCreatureAI) : NULL;
    if (pBruteAI != NULL)
    {
        Unit* pCurrentTarget = pBruteAI->getCreature()->GetAIInterface()->getNextTarget();
        if (pCurrentTarget != NULL && pCurrentTarget != pTarget)
        {
            pBruteAI->getCreature()->GetAIInterface()->AttackReaction(pTarget, 500);
            pBruteAI->getCreature()->GetAIInterface()->setNextTarget(pTarget);
            pBruteAI->getCreature()->GetAIInterface()->RemoveThreatByPtr(pCurrentTarget);
        }

        pBruteAI->CastSpell(pBruteAI->mCharge);
    }
}

const uint32 CN_GRONN_PRIEST = 21350;
const uint32 GRONN_PRIEST_PSYCHICSCREAM = 22884;
const uint32 GRONN_PRIEST_RENEW = 36679;
const uint32 GRONN_PRIEST_HEAL = 36678;

class GronnPriestAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GronnPriestAI);
        GronnPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(GRONN_PRIEST_PSYCHICSCREAM, Target_Self, 8, 0, 20);
            AddSpell(GRONN_PRIEST_RENEW, Target_WoundedFriendly, 6, 0, 25, 0, 100);
            AddSpell(GRONN_PRIEST_HEAL, Target_WoundedFriendly, 5, 2, 30, 0, 100);
        }
};

const uint32 CN_HIGH_KING_MAULGAR = 18831;
const uint32 HIGH_KING_MAULGAR_BERSERKER_CHARGE = 26561;
const uint32 HIGH_KING_MAULGAR_INTIMIDATING_ROAR = 16508;
const uint32 HIGH_KING_MAULGAR_MIGHTY_BLOW = 33230;
const uint32 HIGH_KING_MAULGAR_FLURRY = 33232;
const uint32 HIGH_KING_MAULGAR_ARCING_SMASH = 28168;
const uint32 HIGH_KING_MAULGAR_ARCING_SMASH2 = 39144;
const uint32 HIGH_KING_MAULGAR_WHIRLWIND = 33238;
const uint32 HIGH_KING_MAULGAR_WHIRLWIND2 = 33239;

void SpellFunc_Maulgar_Enrage(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
// 4th unit sometimes cannot be found - blame cell system
uint32 Adds[4] = { 18832, 18834, 18836, 18835 };

class HighKingMaulgarAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HighKingMaulgarAI);
        HighKingMaulgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddPhaseSpell(2, AddSpell(HIGH_KING_MAULGAR_BERSERKER_CHARGE, Target_RandomPlayer, 10, 0, 25, 0, 40));
            AddPhaseSpell(2, AddSpell(HIGH_KING_MAULGAR_INTIMIDATING_ROAR, Target_Current, 7, 0, 20, 0, 5));
            AddSpell(HIGH_KING_MAULGAR_ARCING_SMASH, Target_Current, 8, 0, 15, 0, 10);
            AddSpell(HIGH_KING_MAULGAR_WHIRLWIND, Target_Self, 7, 15, 25);                    // SpellFunc for range check?
            AddSpell(HIGH_KING_MAULGAR_MIGHTY_BLOW, Target_Current, 7, 0, 20, 0, 5);
            mEnrage = AddSpellFunc(&SpellFunc_Maulgar_Enrage, Target_Self, 0, 0, 0);
            mEnrage->addEmote("You will not defeat the hand of Gruul!", CHAT_MSG_MONSTER_YELL, 11368);

            addEmoteForEvent(Event_OnCombatStart, 8806);
            addEmoteForEvent(Event_OnTargetDied, 8807);
            addEmoteForEvent(Event_OnTargetDied, 8808);
            addEmoteForEvent(Event_OnTargetDied, 8809);
            addEmoteForEvent(Event_OnDied, 8810);

            mLastYell = -1;
            mAliveAdds = 0;
        }

        void OnCombatStart(Unit* pTarget) override
        {
            _setDisplayWeapon(true, true);
            
            mAliveAdds = 0;
            mLastYell = -1;
            for (uint8 i = 0; i < 4; ++i)
            {
                Unit* pAdd = getNearestCreature(Adds[i]);
                if (pAdd != NULL && pAdd->isAlive())
                {
                    Unit* pTarget = GetBestPlayerTarget();
                    if (pTarget != NULL)
                    {
                        pAdd->GetAIInterface()->AttackReaction(pTarget, 200);
                    }

                    ++mAliveAdds;
                }
            }
            if (mAliveAdds > 1)
            {
                setCanEnterCombat(false);
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }

        void OnCombatStop(Unit* pTarget) override
        {
            setCanEnterCombat(true);
        }

        void OnDied(Unit* mKiller) override
        {
            GameObject* pDoor = mKiller->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(95.26f, 251.836f, 0.47f, 183817);
            if (pDoor != NULL)
            {
                pDoor->SetState(GO_STATE_OPEN);
            }
        }

        void AIUpdate() override
        {
            if (mAliveAdds > 1)
                return;

            if (isScriptPhase(1) && _getHealthPercent() <= 50)
                setScriptPhase(2);
        }

        void OnScriptPhaseChange(uint32_t phaseId) override
        {
            switch (phaseId)
            {
                case 2:
                    CastSpellNowNoScheduling(mEnrage);
                    break;
                default:
                    break;
            }
        }

        void OnAddDied()
        {
            if (mAliveAdds > 0)
            {
                --mAliveAdds;
                if (mAliveAdds > 1)
                {
                    uint32 RandomText = RandomUInt(1);
                    while((int)RandomText == mLastYell)
                    {
                        RandomText = RandomUInt(1);
                    }

                    switch (RandomText)
                    {
                        case 0:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11369, "You not kill next one so easy!");
                            break;
                        case 1:
                            sendChatMessage(CHAT_MSG_MONSTER_YELL, 11370, "Does not prove anything!");
                            break;
                    }

                    mLastYell = RandomText;
                }
                else if (mAliveAdds == 1)
                {
                    sendChatMessage(CHAT_MSG_MONSTER_YELL, 0, "Good, now you fight me!");
                    setCanEnterCombat(true);
                    setAIAgent(AGENT_NULL);
                    setRooted(false);
                }
            }
        }

        uint32 mAliveAdds;
        int32 mLastYell;
        SpellDesc* mEnrage;
};

void SpellFunc_Maulgar_Enrage(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    HighKingMaulgarAI* pMaulgarAI = (pCreatureAI != NULL) ? static_cast< HighKingMaulgarAI* >(pCreatureAI) : NULL;
    if (pMaulgarAI != NULL)
    {
        pMaulgarAI->_applyAura(HIGH_KING_MAULGAR_FLURRY);
        pMaulgarAI->_setDisplayWeapon(false, false);
    }
}

const uint32 CN_KIGGLER_THE_CRAZED = 18835;
const uint32 KIGGLER_THE_CRAZED_LIGHTNING_BOLT = 36152;
const uint32 KIGGLER_THE_CRAZED_GREATER_POLYMORPH = 33173;
const uint32 KIGGLER_THE_CRAZED_ARCANE_EXPLOSION = 33237;
const uint32 KIGGLER_THE_CRAZED_ARCANE_SHOCK = 33175;

class KigglerTheCrazedAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KigglerTheCrazedAI);
        KigglerTheCrazedAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(KIGGLER_THE_CRAZED_LIGHTNING_BOLT, Target_Current, 70, 2, 0, 0, 40);
            AddSpell(KIGGLER_THE_CRAZED_GREATER_POLYMORPH, Target_RandomPlayer, 8, 0, 15, 0, 30);    // Additional SpellFunc for removing target from target list if there are different targets?
            AddSpell(KIGGLER_THE_CRAZED_ARCANE_EXPLOSION, Target_Self, 8, 0, 20);
            AddSpell(KIGGLER_THE_CRAZED_ARCANE_SHOCK, Target_RandomPlayer, 10, 0, 15, 0, 30);
        }

        void OnCombatStart(Unit* pTarget) override
        {
            if (getRangeToObject(pTarget) <= 40.0f)
            {
                setAIAgent(AGENT_SPELL);
                setRooted(true);
            }
        }

        void OnDied(Unit* mKiller) override
        {
            Creature* pMaulgar = getNearestCreature(143.048996f, 192.725998f, -11.114700f, CN_HIGH_KING_MAULGAR);
            if (pMaulgar != NULL && pMaulgar->isAlive() && pMaulgar->GetScript())
            {
                HighKingMaulgarAI* pMaulgarAI = static_cast< HighKingMaulgarAI* >(pMaulgar->GetScript());
                pMaulgarAI->OnAddDied();
            }
        }

        void AIUpdate() override
        {
            Unit* pTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pTarget != NULL)
            {
                if (getRangeToObject(pTarget) <= 40.0f)
                {
                    setAIAgent(AGENT_SPELL);
                    setRooted(true);
                }
            }
        }
};

const uint32 CN_BLINDEYE_THE_SEER = 18836;
const uint32 BLINDEYE_THE_SEER_PRAYER_OF_HEALING = 33152;
const uint32 BLINDEYE_THE_SEER_GREAT_POWER_WORD_SHIELD = 33147;
const uint32 BLINDEYE_THE_SEER_HEAL = 33144;

class BlindeyeTheSeerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BlindeyeTheSeerAI);
        BlindeyeTheSeerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(BLINDEYE_THE_SEER_PRAYER_OF_HEALING, Target_Self, 5, 4, 30);                // Affects players? Core bugzor?
            AddSpell(BLINDEYE_THE_SEER_GREAT_POWER_WORD_SHIELD, Target_Self, 8, 0, 30);            // Strategies don't say anything about buffing friends
            AddSpell(BLINDEYE_THE_SEER_HEAL, Target_WoundedFriendly, 8, 1.5, 25);
        }

        void OnDied(Unit* mKiller) override
        {
            Creature* pMaulgar = getNearestCreature(143.048996f, 192.725998f, -11.114700f, CN_HIGH_KING_MAULGAR);
            if (pMaulgar != NULL && pMaulgar->isAlive() && pMaulgar->GetScript())
            {
                HighKingMaulgarAI* pMaulgarAI = static_cast< HighKingMaulgarAI* >(pMaulgar->GetScript());
                pMaulgarAI->OnAddDied();
            }
        }
};

const uint32 CN_OLM_THE_SUMMONER = 18834;
const uint32 OLM_THE_SUMMONER_DEATH_COIL = 33130;
const uint32 OLM_THE_SUMMONER_SUMMON_WILD_FELHUNTER = 33131;
const uint32 OLM_THE_SUMMONER_DARK_DECAY = 33129;

class OlmTheSummonerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(OlmTheSummonerAI);
        OlmTheSummonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(OLM_THE_SUMMONER_DEATH_COIL, Target_RandomPlayer, 7, 0, 10, 0, 30);
            AddSpell(OLM_THE_SUMMONER_SUMMON_WILD_FELHUNTER, Target_Self, 7, 3, 15);
            AddSpell(OLM_THE_SUMMONER_DARK_DECAY, Target_RandomPlayer, 10, 0, 6);
        }

        void OnDied(Unit* mKiller) override
        {
            Creature* pMaulgar = getNearestCreature(143.048996f, 192.725998f, -11.114700f, CN_HIGH_KING_MAULGAR);
            if (pMaulgar != NULL && pMaulgar->isAlive() && pMaulgar->GetScript())
            {
                HighKingMaulgarAI* pMaulgarAI = static_cast< HighKingMaulgarAI* >(pMaulgar->GetScript());
                pMaulgarAI->OnAddDied();
            }
        }
};

const uint32 CN_WILD_FEL_STALKER = 18847;
const uint32 WILD_FEL_STALKER_WILD_BITE = 33086;

class WildFelStalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WildFelStalkerAI);
        WildFelStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            AddSpell(WILD_FEL_STALKER_WILD_BITE, Target_Current, 10, 0, 10, 0, 5);
            AggroRandomPlayer(200);
        }
};

const uint32 CN_KROSH_FIREHAND = 18832;

/* He will first spellshield on himself, and recast every 30 sec,
   then spam great fireball to the target, also if there is any unit
   close to him (15yr) he'll cast blast wave
*/

const uint32 GREAT_FIREBALL = 33051;
const uint32 BALST_WAVE = 33061;
const uint32 SPELLSHIELD = 33054;

class KroshFirehandAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(KroshFirehandAI);
        KroshFirehandAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            //spells
            mBlastWave = AddSpell(BALST_WAVE, Target_Self, 0, 0, 0, 0, 15);
            AddSpell(GREAT_FIREBALL, Target_Current, 100, 3, 0, 0, 100);
            mSpellShield = AddSpell(SPELLSHIELD, Target_Self, 0, 0, 0);

            mEventTimer = _addTimer(30000);
            mBlastWaveTimer = -1;
            SetAIUpdateFreq(250);
        }

        void OnCombatStart(Unit* pTarget) override
        {
            CastSpellNowNoScheduling(mSpellShield); 
        }

        void AIUpdate() override
        {
            if (!_isCasting())
            {
                if (mBlastWaveTimer == -1 || _isTimerFinished(mBlastWaveTimer))
                {
                    Unit* unit = GetBestUnitTarget(TargetFilter_Closest);
                    if (unit && getRangeToObject(unit) < 15.0f)
                    {
                        CastSpellNowNoScheduling(mBlastWave);
                        if (mBlastWaveTimer == -1)
                            mBlastWaveTimer = _addTimer(6000);
                        else
                            _resetTimer(mBlastWaveTimer, 6000);
                        
                        return;
                    }
                }

                if (_isTimerFinished(mEventTimer))
                {
                    _resetTimer(mEventTimer, 30000);
                    CastSpellNowNoScheduling(mSpellShield);
                }
            }
        }

        void OnDied(Unit* mKiller) override
        {
            Creature* pMaulgar = getNearestCreature(143.048996f, 192.725998f, -11.114700f, CN_HIGH_KING_MAULGAR);
            if (pMaulgar != NULL && pMaulgar->isAlive() && pMaulgar->GetScript())
            {
                HighKingMaulgarAI* pMaulgarAI = static_cast< HighKingMaulgarAI* >(pMaulgar->GetScript());
                pMaulgarAI->OnAddDied();
            }
        }

        SpellDesc* mSpellShield;
        SpellDesc* mBlastWave;
        int32 mEventTimer;
        int32 mBlastWaveTimer;
};

const uint32 CN_GRUUL_THE_DRAGONKILLER = 19044;
const uint32 GRUUL_THE_DRAGONKILLER_GROWTH = 36300;   // +
const uint32 GRUUL_THE_DRAGONKILLER_CAVE_IN = 36240;   // + 
const uint32 GRUUL_THE_DRAGONKILLER_GROUND_SLAM = 33525;    // +
const uint32 GRUUL_THE_DRAGONKILLER_GROUND_SLAM2 = 39187;    // +
const uint32 GRUUL_THE_DRAGONKILLER_SHATTER = 33671;    // does not make dmg - to script
const uint32 GRUUL_THE_DRAGONKILLER_HURTFUL_STRIKE = 33813;   // +
const uint32 GRUUL_THE_DRAGONKILLER_REVERBERATION = 36297;    // +
const uint32 GRUUL_THE_DRAGONKILLER_STONED = 33652;    // +
const uint32 GRUUL_THE_DRAGONKILLER_GRONN_LORDS_GRASP = 33572;    // Should be used only after Ground Slam

void SpellFunc_Gruul_GroundSlam(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_Gruul_Stoned(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
void SpellFunc_Gruul_Shatter(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class GruulTheDragonkillerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(GruulTheDragonkillerAI);
        GruulTheDragonkillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            mHurtfulStrike = AddSpell(GRUUL_THE_DRAGONKILLER_HURTFUL_STRIKE, Target_Current, 0, 0, 0, 0, 8);
            mGroundSlam = AddSpell(GRUUL_THE_DRAGONKILLER_GROUND_SLAM, Target_Self, 0, 1, 0);
            mGroundSlam->addEmote("Scurry.", CHAT_MSG_MONSTER_YELL, 11356);
            mGroundSlam->addEmote("No escape.", CHAT_MSG_MONSTER_YELL, 11357);
            mGroundSlam2 = AddSpell(GRUUL_THE_DRAGONKILLER_GROUND_SLAM2, Target_Self, 0, 1, 0);
            mStoned = AddSpellFunc(&SpellFunc_Gruul_Stoned, Target_Self, 0, 2, 0);
            mShatter = AddSpellFunc(&SpellFunc_Gruul_Shatter, Target_Self, 0, 3, 0);
            mShatter2 = AddSpell(GRUUL_THE_DRAGONKILLER_SHATTER, Target_Self, 0, 1, 0);
            mShatter2->addEmote("Stay...", CHAT_MSG_MONSTER_YELL, 11358);
            mShatter2->addEmote("Beg for life.", CHAT_MSG_MONSTER_YELL, 11359);
            AddSpell(GRUUL_THE_DRAGONKILLER_REVERBERATION, Target_Self, 4, 0, 30);
            AddSpell(GRUUL_THE_DRAGONKILLER_CAVE_IN, Target_RandomPlayerDestination, 7, 0, 25);
            AddSpellFunc(&SpellFunc_Gruul_GroundSlam, Target_Self, 6, 1, 35);

            addEmoteForEvent(Event_OnCombatStart, 8811);
            addEmoteForEvent(Event_OnTargetDied, 8812);
            addEmoteForEvent(Event_OnTargetDied, 8813);
            addEmoteForEvent(Event_OnTargetDied, 8814);
            addEmoteForEvent(Event_OnDied, 8815);

            mGrowthTimer = mHurtfulTimer = -1;
            mGrowthStacks = 0;
        }

        void OnCombatStart(Unit* pTarget) override
        {
            mGrowthTimer = _addTimer(30000);
            mHurtfulTimer = _addTimer(8000);
            mGrowthStacks = 0;

            GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
            if (pGate != NULL)
                pGate->SetState(GO_STATE_CLOSED);
        }

        void OnCombatStop(Unit* pTarget) override
        {
            GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
            if (pGate != NULL)
                pGate->SetState(GO_STATE_OPEN);
        }

        void OnDied(Unit* mKiller) override
        {
            GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
            if (pGate != NULL)
                pGate->SetState(GO_STATE_OPEN);
        }

        void AIUpdate() override
        {
            if (!_isCasting())
            {
                if (_isTimerFinished(mGrowthTimer))
                {
                    if (mGrowthStacks == 30)
                    {
                        _removeAura(GRUUL_THE_DRAGONKILLER_GROWTH);
                        mGrowthStacks = 0;
                    }
                    if (mGrowthStacks != 29)
                    {
                        _resetTimer(mGrowthTimer, 30000);
                    }
                    else if (mGrowthStacks == 29)
                    {
                        _resetTimer(mGrowthTimer, 300000);
                    }

                    _applyAura(GRUUL_THE_DRAGONKILLER_GROWTH);
                    ++mGrowthStacks;
                }
                else if (_isTimerFinished(mHurtfulTimer))
                {
                    Unit* pCurrentTarget = getCreature()->GetAIInterface()->getNextTarget();
                    if (pCurrentTarget != NULL)
                    {
                        Unit* pTarget = pCurrentTarget;
                        for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
                        {
                            Player* pPlayer = static_cast< Player* >(*itr);
                            if (!pPlayer->isAlive())
                                continue;
//                        if (pPlayer->m_auracount[SPELL_AURA_MOD_INVISIBILITY])
//                            continue;
                            if (pPlayer->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                                continue;
                            if (getRangeToObject(pPlayer) > 8.0f)
                                continue;
                            if (getCreature()->GetAIInterface()->getThreatByPtr(pPlayer) >= getCreature()->GetAIInterface()->getThreatByPtr(pCurrentTarget))
                                continue;

                            pTarget = static_cast<Unit*>(pPlayer);
                        }

                        if (pTarget == pCurrentTarget)
                            CastSpellNowNoScheduling(mHurtfulStrike);
                        else
                            getCreature()->CastSpell(pTarget, GRUUL_THE_DRAGONKILLER_HURTFUL_STRIKE, true);
                    }

                    _resetTimer(mHurtfulTimer, 8000);
                }
            }
        }

        UnitArray GetInRangePlayers()
        {
            UnitArray TargetArray;
            for (std::set< Object* >::iterator itr = getCreature()->GetInRangePlayerSetBegin(); itr != getCreature()->GetInRangePlayerSetEnd(); ++itr)
            {
                if (IsValidUnitTarget(*itr, TargetFilter_None))
                {
                    TargetArray.push_back(static_cast< Unit* >(*itr));
                }
            }

            return TargetArray;
        }

        uint32 mGrowthStacks;        // temporary way to store it
        int32 mHurtfulTimer;
        int32 mGrowthTimer;

        SpellDesc* mHurtfulStrike;
        SpellDesc* mGroundSlam;
        SpellDesc* mGroundSlam2;
        SpellDesc* mStoned;
        SpellDesc* mShatter;
        SpellDesc*  mShatter2;
};

void SpellFunc_Gruul_GroundSlam(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    GruulTheDragonkillerAI* pGruul = (pCreatureAI != NULL) ? static_cast< GruulTheDragonkillerAI* >(pCreatureAI) : NULL;
    if (pGruul != NULL)
    {
        pGruul->CastSpellNowNoScheduling(pGruul->mGroundSlam);
        pGruul->CastSpell(pGruul->mGroundSlam2);
        pGruul->CastSpell(pGruul->mStoned);
    }
}

void SpellFunc_Gruul_Stoned(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    GruulTheDragonkillerAI* pGruul = (pCreatureAI != NULL) ? static_cast< GruulTheDragonkillerAI* >(pCreatureAI) : NULL;
    if (pGruul != NULL)
    {
        UnitArray TargetArray = pGruul->GetInRangePlayers();
        if (TargetArray.size() > 0)
        {
            for (size_t i = 0; i < TargetArray.size(); ++i)
            {
                Unit* pTarget = TargetArray[i];
                pTarget->CastSpell(pTarget, GRUUL_THE_DRAGONKILLER_STONED, true);
            }
        }

        pGruul->CastSpell(pGruul->mShatter);
    }
}

void SpellFunc_Gruul_Shatter(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType)
{
    GruulTheDragonkillerAI* pGruul = (pCreatureAI != NULL) ? static_cast< GruulTheDragonkillerAI* >(pCreatureAI) : NULL;
    if (pGruul != NULL)
    {
        pGruul->CastSpell(pGruul->mShatter2);    // Spell to script
        UnitArray TargetArray = pGruul->GetInRangePlayers();
        if (TargetArray.size() > 0)
        {
            for (size_t i = 0; i < TargetArray.size(); ++i)
            {
                Unit* pTarget = TargetArray[i];
                pTarget->RemoveAura(GRUUL_THE_DRAGONKILLER_STONED);
            }
        }
    }
}

void SetupGruulsLair(ScriptMgr* mgr)
{
    mgr->register_creature_script(CN_LAIR_BRUTE, &LairBruteAI::Create);
    mgr->register_creature_script(CN_GRONN_PRIEST, &GronnPriestAI::Create);
    mgr->register_creature_script(CN_KIGGLER_THE_CRAZED, &KigglerTheCrazedAI::Create);
    mgr->register_creature_script(CN_BLINDEYE_THE_SEER, &BlindeyeTheSeerAI::Create);
    mgr->register_creature_script(CN_OLM_THE_SUMMONER, &OlmTheSummonerAI::Create);
    mgr->register_creature_script(CN_WILD_FEL_STALKER, &WildFelStalkerAI::Create);
    mgr->register_creature_script(CN_KROSH_FIREHAND, &KroshFirehandAI::Create);
    mgr->register_creature_script(CN_HIGH_KING_MAULGAR,    &HighKingMaulgarAI::Create);
    mgr->register_creature_script(CN_GRUUL_THE_DRAGONKILLER, &GruulTheDragonkillerAI::Create);
}

