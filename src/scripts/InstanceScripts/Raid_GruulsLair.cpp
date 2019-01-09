/*
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
 * Copyright (c) 2008-2015 Sun++ Team <http://www.sunplusplus.info>
 * Copyright (c) 2007-2015 Moon++ Team <http://www.moonplusplus.info>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
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

class LairBruteAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LairBruteAI);
    explicit LairBruteAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(LAIR_BRUTE_CLEAVE, 20.0f, TARGET_ATTACKING, 0, 15);
        addAISpell(LAIR_BRUTE_MORTALSTRIKE, 8.0f, TARGET_ATTACKING, 0, 20);
        addAISpell(LAIR_BRUTE_CHARGE, 7.0f, TARGET_ATTACKING, 0, 35);
    }

    void OnCastSpell(uint32 spellId) override
    {
        if (spellId == LAIR_BRUTE_CHARGE)
        {
            Unit* pCurrentTarget = getCreature()->GetAIInterface()->getNextTarget();
            if (pCurrentTarget != nullptr)
            {
                getCreature()->GetAIInterface()->AttackReaction(pCurrentTarget, 500);
                getCreature()->GetAIInterface()->setNextTarget(pCurrentTarget);
                getCreature()->GetAIInterface()->RemoveThreatByPtr(pCurrentTarget);
            }
        }
    }
};


const uint32 CN_GRONN_PRIEST = 21350;
const uint32 GRONN_PRIEST_PSYCHICSCREAM = 22884;
const uint32 GRONN_PRIEST_RENEW = 36679;
const uint32 GRONN_PRIEST_HEAL = 36678;

class GronnPriestAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GronnPriestAI);
    explicit GronnPriestAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(GRONN_PRIEST_PSYCHICSCREAM, 8.0f, TARGET_SELF, 0, 20);

        auto renew = addAISpell(GRONN_PRIEST_RENEW, 8.0f, TARGET_RANDOM_FRIEND, 0, 25);
        renew->setMinMaxDistance(0.0f, 100.0f);

        auto heal = addAISpell(GRONN_PRIEST_HEAL, 8.0f, TARGET_RANDOM_FRIEND, 2, 30);
        heal->setMinMaxDistance(0.0f, 100.0f);
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

// 4th unit sometimes cannot be found - blame cell system
uint32 Adds[4] = { 18832, 18834, 18836, 18835 };

class HighKingMaulgarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(HighKingMaulgarAI);
    explicit HighKingMaulgarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        auto charge = addAISpell(HIGH_KING_MAULGAR_BERSERKER_CHARGE, 10.0f, TARGET_RANDOM_SINGLE, 0, 25);
        charge->setAvailableForScriptPhase({ 2 });
        charge->setMinMaxDistance(0.0f, 40.0f);

        auto roar = addAISpell(HIGH_KING_MAULGAR_INTIMIDATING_ROAR, 7.0f, TARGET_ATTACKING, 0, 20);
        roar->setAvailableForScriptPhase({ 2 });

        addAISpell(HIGH_KING_MAULGAR_ARCING_SMASH, 8.0f, TARGET_ATTACKING,  0, 15);
        addAISpell(HIGH_KING_MAULGAR_WHIRLWIND, 7.0f, TARGET_SELF, 15, 25);                    // SpellFunc for range check?
        addAISpell(HIGH_KING_MAULGAR_MIGHTY_BLOW, 7.0f, TARGET_ATTACKING, 0, 20);

        mEnrage = addAISpell(HIGH_KING_MAULGAR_FLURRY, 2.0f, TARGET_SELF, 0, 60);
        mEnrage->addEmote("You will not defeat the hand of Gruul!", CHAT_MSG_MONSTER_YELL, 11368);

        addEmoteForEvent(Event_OnCombatStart, 8806);
        addEmoteForEvent(Event_OnTargetDied, 8807);
        addEmoteForEvent(Event_OnTargetDied, 8808);
        addEmoteForEvent(Event_OnTargetDied, 8809);
        addEmoteForEvent(Event_OnDied, 8810);

        mLastYell = -1;
        mAliveAdds = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _setDisplayWeapon(true, true);
        
        mAliveAdds = 0;
        mLastYell = -1;
        for (uint8 i = 0; i < 4; ++i)
        {
            Unit* pAdd = getNearestCreature(Adds[i]);
            if (pAdd != NULL && pAdd->isAlive())
            {
                Unit* pTarget = getBestPlayerTarget();
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

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        setCanEnterCombat(true);
    }

    void OnDied(Unit* mKiller) override
    {
        GameObject* pDoor = mKiller->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(95.26f, 251.836f, 0.47f, 183817);
        if (pDoor != NULL)
        {
            pDoor->setState(GO_STATE_OPEN);
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
                _castAISpell(mEnrage);
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
                uint32 RandomText = Util::getRandomUInt(1);
                while((int)RandomText == mLastYell)
                {
                    RandomText = Util::getRandomUInt(1);
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
    CreatureAISpells* mEnrage;
};


const uint32 CN_KIGGLER_THE_CRAZED = 18835;
const uint32 KIGGLER_THE_CRAZED_LIGHTNING_BOLT = 36152;
const uint32 KIGGLER_THE_CRAZED_GREATER_POLYMORPH = 33173;
const uint32 KIGGLER_THE_CRAZED_ARCANE_EXPLOSION = 33237;
const uint32 KIGGLER_THE_CRAZED_ARCANE_SHOCK = 33175;

class KigglerTheCrazedAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(KigglerTheCrazedAI);
    explicit KigglerTheCrazedAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(KIGGLER_THE_CRAZED_LIGHTNING_BOLT, 70.0f, TARGET_ATTACKING, 2, 0);
        addAISpell(KIGGLER_THE_CRAZED_GREATER_POLYMORPH, 8.0f, TARGET_RANDOM_SINGLE, 0, 15);
        addAISpell(KIGGLER_THE_CRAZED_ARCANE_EXPLOSION, 8.0f, TARGET_SELF, 0, 20);
        addAISpell(KIGGLER_THE_CRAZED_ARCANE_SHOCK, 10.0f, TARGET_RANDOM_SINGLE, 0, 15);
    }

    void OnCombatStart(Unit* pTarget) override
    {
        if (getRangeToObject(pTarget) <= 40.0f)
        {
            setAIAgent(AGENT_SPELL);
            setRooted(true);
        }
    }

    void OnDied(Unit* /*mKiller*/) override
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
    explicit BlindeyeTheSeerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(BLINDEYE_THE_SEER_PRAYER_OF_HEALING, 5.0f, TARGET_SELF, 4, 30);
        addAISpell(BLINDEYE_THE_SEER_GREAT_POWER_WORD_SHIELD, 8.0f, TARGET_SELF, 0, 30);
        addAISpell(BLINDEYE_THE_SEER_HEAL, 8.0f, TARGET_RANDOM_FRIEND,2, 25);
    }

    void OnDied(Unit* /*mKiller*/) override
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
    explicit OlmTheSummonerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(OLM_THE_SUMMONER_DEATH_COIL, 7.0f, TARGET_RANDOM_SINGLE, 0, 10);
        addAISpell(OLM_THE_SUMMONER_SUMMON_WILD_FELHUNTER, 7.0f, TARGET_SELF, 3, 15);
        addAISpell(OLM_THE_SUMMONER_DARK_DECAY, 10.0f, TARGET_RANDOM_SINGLE, 0, 6);
    }

    void OnDied(Unit* /*mKiller*/) override
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
    explicit WildFelStalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(WILD_FEL_STALKER_WILD_BITE, 10.0f, TARGET_ATTACKING, 0, 10);
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
    explicit KroshFirehandAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        //spells
        mBlastWave = addAISpell(BALST_WAVE, 0.0f, TARGET_SELF, 0, 15);
        addAISpell(GREAT_FIREBALL, 100.0f, TARGET_ATTACKING, 3, 0);
        mSpellShield = addAISpell(SPELLSHIELD, 0.0f, TARGET_SELF, 0, 0);

        mEventTimer = _addTimer(30000);
        mBlastWaveTimer = -1;
        SetAIUpdateFreq(250);
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        _castAISpell(mSpellShield);
    }

    void AIUpdate() override
    {
        if (!_isCasting())
        {
            if (mBlastWaveTimer == -1 || _isTimerFinished(mBlastWaveTimer))
            {
                Unit* unit = getBestUnitTarget(TargetFilter_Closest);
                if (unit && getRangeToObject(unit) < 15.0f)
                {
                    _castAISpell(mBlastWave);
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
                _castAISpell(mSpellShield);
            }
        }
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        Creature* pMaulgar = getNearestCreature(143.048996f, 192.725998f, -11.114700f, CN_HIGH_KING_MAULGAR);
        if (pMaulgar != NULL && pMaulgar->isAlive() && pMaulgar->GetScript())
        {
            HighKingMaulgarAI* pMaulgarAI = static_cast< HighKingMaulgarAI* >(pMaulgar->GetScript());
            pMaulgarAI->OnAddDied();
        }
    }

    CreatureAISpells* mSpellShield;
    CreatureAISpells* mBlastWave;
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

//void SpellFunc_Gruul_GroundSlam(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
//void SpellFunc_Gruul_Stoned(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);
//void SpellFunc_Gruul_Shatter(SpellDesc* pThis, CreatureAIScript* pCreatureAI, Unit* pTarget, TargetType pType);

class GruulTheDragonkillerAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GruulTheDragonkillerAI);
    explicit GruulTheDragonkillerAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        mHurtfulStrike = addAISpell(GRUUL_THE_DRAGONKILLER_HURTFUL_STRIKE, 0.0f, TARGET_ATTACKING, 0, 8);

        mGroundSlam = addAISpell(GRUUL_THE_DRAGONKILLER_GROUND_SLAM, 6.0f, TARGET_SELF, 1, 35);
        mGroundSlam->addEmote("Scurry.", CHAT_MSG_MONSTER_YELL, 11356);
        mGroundSlam->addEmote("No escape.", CHAT_MSG_MONSTER_YELL, 11357);

        mGroundSlam2 = addAISpell(GRUUL_THE_DRAGONKILLER_GROUND_SLAM2, 0.0f, TARGET_SELF, 1, 0);

        mShatter2 = addAISpell(GRUUL_THE_DRAGONKILLER_SHATTER, 0.0f, TARGET_SELF, 0, 1, 0);
        mShatter2->addEmote("Stay...", CHAT_MSG_MONSTER_YELL, 11358);
        mShatter2->addEmote("Beg for life.", CHAT_MSG_MONSTER_YELL, 11359);

        addAISpell(GRUUL_THE_DRAGONKILLER_REVERBERATION, TARGET_SELF, 4, 0, 30);
        addAISpell(GRUUL_THE_DRAGONKILLER_CAVE_IN, TARGET_RANDOM_DESTINATION, 7, 0, 25);

        //addAISpell(&SpellFunc_Gruul_GroundSlam, TARGET_SELF, 6, 1, 35);

        addEmoteForEvent(Event_OnCombatStart, 8811);
        addEmoteForEvent(Event_OnTargetDied, 8812);
        addEmoteForEvent(Event_OnTargetDied, 8813);
        addEmoteForEvent(Event_OnTargetDied, 8814);
        addEmoteForEvent(Event_OnDied, 8815);

        mGrowthTimer = mHurtfulTimer = -1;
        mGrowthStacks = 0;
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mGrowthTimer = _addTimer(30000);
        mHurtfulTimer = _addTimer(8000);
        mGrowthStacks = 0;

        GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
        if (pGate != NULL)
            pGate->setState(GO_STATE_CLOSED);
    }

    void OnCastSpell(uint32 spellId) override
    {
        if (spellId == GRUUL_THE_DRAGONKILLER_GROUND_SLAM)
        {
            _castAISpell(mGroundSlam);
            _castAISpell(mGroundSlam2);
            //_castAISpell(mStoned);
        }
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
        if (pGate != NULL)
            pGate->setState(GO_STATE_OPEN);
    }

    void OnDied(Unit* /*mKiller*/) override
    {
        GameObject* pGate = getNearestGameObject(166.897f, 368.226f, 16.9209f, 184662);
        if (pGate != NULL)
            pGate->setState(GO_STATE_OPEN);
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
                if (pCurrentTarget != nullptr)
                {
                    Unit* pTarget = pCurrentTarget;
                    for (const auto& itr : getCreature()->getInRangePlayersSet())
                    {
                        Player* pPlayer = static_cast<Player*>(itr);
                        if (!pPlayer || !pPlayer->isAlive())
                            continue;
                        if (pPlayer->hasUnitFlags(UNIT_FLAG_FEIGN_DEATH))
                            continue;
                        if (getRangeToObject(pPlayer) > 8.0f)
                            continue;
                        if (getCreature()->GetAIInterface()->getThreatByPtr(pPlayer) >= getCreature()->GetAIInterface()->getThreatByPtr(pCurrentTarget))
                            continue;

                        pTarget = static_cast<Unit*>(pPlayer);
                    }

                    if (pTarget == pCurrentTarget)
                        _castAISpell(mHurtfulStrike);
                    else
                        getCreature()->castSpell(pTarget, GRUUL_THE_DRAGONKILLER_HURTFUL_STRIKE, true);
                }

                _resetTimer(mHurtfulTimer, 8000);
            }
        }
    }

    uint32 mGrowthStacks;        // temporary way to store it
    int32 mHurtfulTimer;
    int32 mGrowthTimer;

    CreatureAISpells* mHurtfulStrike;
    CreatureAISpells* mGroundSlam;
    CreatureAISpells* mGroundSlam2;
    //CreatureAISpells* mStoned;
    //CreatureAISpells* mShatter;
    CreatureAISpells* mShatter2;
};

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

