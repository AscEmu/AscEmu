/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
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
#include "Instance_PitOfSaron.h"

class InstancePitOfSaronScript : public MoonInstanceScript
{
    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(InstancePitOfSaronScript, MoonInstanceScript);
        InstancePitOfSaronScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            // Way to select bosses
            BuildEncounterMap();
            if (mEncounters.size() == 0)
                return;

            for (EncounterMap::iterator Iter = mEncounters.begin(); Iter != mEncounters.end(); ++Iter)
            {
                if ((*Iter).second.mState != State_Finished)
                    continue;
            }
        }

        void OnGameObjectPushToWorld(GameObject* pGameObject) { }

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        }

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        }

        void OnCreatureDeath(Creature* pCreature, Unit* pUnit)
        {
            EncounterMap::iterator Iter = mEncounters.find(pCreature->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;
        }

        void OnPlayerEnter(Player* player)
        {
            if (!mSpawnsCreated)
            {
                if (player->GetTeam() == TEAM_ALLIANCE)
                {
                    spawnCreature(CN_JAINA_PROUDMOORE, 441.39f, 213.32f, 528.71f, 0.10f, 35);
                    spawnCreature(CN_ARCHMAGE_ELANDRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
                    spawnCreature(CN_ARCHMAGE_KORELN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
                }
                else // TEAM_HORDE
                {
                    spawnCreature(CN_SYLVANAS_WINDRUNNER, 441.39f, 213.32f, 528.71f, 0.10f, 35);
                    spawnCreature(CN_DARK_RANGER_LORALEN, 440.35f, 211.154f, 528.71f, 6.15f, 35);
                    spawnCreature(CN_DARK_RANGER_KALIRA, 439.26f, 215.89f, 528.71f, 0.02f, 35);
                }
                mSpawnsCreated = true;
            }
        }
};

// BOSSES
// Forgemaster Garfrost

class ForgemasterGarfrostAI : MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(ForgemasterGarfrostAI, MoonScriptBossAI);
    ForgemasterGarfrostAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Instance Script
        mInstance = GetInstanceScript();

        // Normal Spells
        mSaronite = AddSpell(SPELL_THROWSARONITE, Target_RandomPlayerDestination, 20, 2, 15);
        mPermafrost = AddSpell(SPELL_PERMAFROST, Target_Self, 0, 0, 0);

        if (IsHeroic())
        {
            // Phased Spells
            mChllingWave = AddPhaseSpell(2, AddSpell(H_SPELL_CHILLING_WAVE, Target_Current, 25, 0, 14));
            mDeepFreeze = AddPhaseSpell(3, AddSpell(H_SPELL_DEEP_FREEZE, Target_RandomPlayer, 15, 2, 20));
        }
        else
        {
            // Phased Spells
            mChllingWave = AddPhaseSpell(2, AddSpell(SPELL_CHILLINGWAVE, Target_Current, 25, 0, 14));
            mDeepFreeze = AddPhaseSpell(3, AddSpell(SPELL_DEEPFREEZE, Target_RandomPlayer, 15, 2, 20));
        }

        // Timers
        mSaroniteTimer = INVALIDATE_TIMER;
        mPermafrostTimer = INVALIDATE_TIMER;
        mChllingWaveTimer = INVALIDATE_TIMER;
        mDeepFreezeTimer = INVALIDATE_TIMER;

        // Emotes
        AddEmote(Event_OnCombatStart, 8761);
        AddEmote(Event_OnTargetDied, 8762);
        AddEmote(Event_OnTargetDied, 8763);
        AddEmote(Event_OnDied, 8764);
    }

    void OnCombatStart(Unit* pTarget)
    {
        // Seting up Timers
        mSaroniteTimer = AddTimer(45000);
        mPermafrostTimer = AddTimer(2000);
        mChllingWaveTimer = AddTimer(10000);
        mDeepFreezeTimer = AddTimer(10000);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);

        MoonScriptBossAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* mTarget)
    {
        // Clear Agent and Ai State
        _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
        _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        MoonScriptBossAI::OnCombatStop(mTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        MoonScriptBossAI::OnTargetDied(pTarget);
    }

    void OnDied(Unit* mKiller)
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        CastSpells();

        if (GetPhase() == 1 && GetHealthPercent() <= 66)
        {
            sendDBChatMessage(8765);
            _unit->CastSpell(_unit, SPELL_STOMP, false);
            _unit->GetAIInterface()->WipeHateList();
            _unit->GetAIInterface()->splineMoveJump(JumpCords[0].x, JumpCords[0].y, JumpCords[0].z);
            
            if (GameObject * pObject = getNearestGameObject(401006))	//forgemaster's anvil (TEMP)
                _unit->SetFacing(_unit->calcRadAngle(_unit->GetPositionX(), _unit->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));

            if (IsHeroic())
                _unit->CastSpell(_unit, H_SPELL_FORGE_BLADE, false);
            else
                _unit->CastSpell(_unit, SPELL_FROZEBLADE, false);

            _unit->SetEquippedItem(MELEE, EQUIP_ID_SWORD);
            _unit->SetEquippedItem(OFFHAND, 0);
            SetPhase(2);
        }

        if (GetPhase() == 2 && GetHealthPercent() <= 33)
        {
            sendDBChatMessage(8766);
            _unit->CastSpell(_unit, SPELL_STOMP, false);
            _unit->GetAIInterface()->WipeHateList();
            _unit->GetAIInterface()->splineMoveJump(JumpCords[1].x, JumpCords[1].y, JumpCords[1].z);

            if (GameObject * pObject = getNearestGameObject(401006))	//forgemaster's anvil (TEMP)
                _unit->SetFacing(_unit->calcRadAngle(_unit->GetPositionX(), _unit->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));
            
            if (IsHeroic())
                _unit->CastSpell(_unit, H_SPELL_FORGE_MACE, false);
            else
                _unit->CastSpell(_unit, SPELL_FROZEMACE, false);
            
            _unit->SetEquippedItem(MELEE, EQUIP_ID_MACE);
            SetPhase(3);
        }

        ParentClass::AIUpdate();
    }

    void CastSpells()
    {
        if (IsTimerFinished(mSaroniteTimer))
        {
            // Casting Saronite Boulder every 45 secs.
            Unit* unit = GetBestUnitTarget(TargetFilter_None);
            CastSpellOnTarget(unit, TargetGen_RandomPlayer, mSaronite->mInfo, true);
            ResetTimer(mSaroniteTimer, 45000);
        }

        if (IsTimerFinished(mPermafrostTimer))
        {
            // Cast Permafrost every 2 secs.
            CastSpellNowNoScheduling(mPermafrost);
            ResetTimer(mPermafrostTimer, 2000);
        }

        if (IsTimerFinished(mChllingWaveTimer) && GetPhase() == 2)
        {
            // Cast Chilling Wave every 10 secs.
            CastSpell(mChllingWave);
            ResetTimer(mChllingWaveTimer, 10000);
        }

        if (IsTimerFinished(mDeepFreezeTimer) && GetPhase() == 3)
        {
            // Cast Deep Freeze every 10 secs.
            CastSpell(mDeepFreeze);
            ResetTimer(mDeepFreezeTimer, 10000);
        }
    }

    SpellDesc* mPermafrost;
    SpellDesc* mSaronite;
    SpellDesc* mChllingWave;
    SpellDesc* mDeepFreeze;
    int32_t mSaroniteTimer;
    int32_t mPermafrostTimer;
    int32_t mChllingWaveTimer;
    int32_t mDeepFreezeTimer;
    MoonInstanceScript* mInstance;
};

// Ick and Krick

class IckAI : MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(IckAI, MoonScriptBossAI);
    IckAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Instance Script
        mInstance = GetInstanceScript();

        // Spells
        mMightyKick = AddSpell(SPELL_MIGHTY_KICK, Target_Current, 25, 0, 0);
        mPursue = AddSpell(SPELL_PURSUED, Target_RandomPlayer, 0, 5000, 0);
        mExplosionBarage = AddSpell(SPELL_EXPLOSIVE_BARRAGE, Target_Self, 0, 0, 0);
        mShadowBolt = AddSpell(SPELL_SHADOW_BOLT, Target_Current, 0, 3000, 0);
        mConfusion = AddSpell(SPELL_CONFUSION, Target_Self, 0, 0, 0);
        mExplosionBarageKrick = AddSpell(SPELL_EXPLOSIVE_BARRAGE_KRICK, Target_Self, 0, 0, 0);

        if (IsHeroic())
        {
            mToxicWaste = AddSpell(H_SPELL_TOXIC_WASTE, Target_Current, 0, 0, 0);
            mPoisonNova = AddSpell(H_SPELL_POISON_NOVA, Target_Self, 0, 5000, 0);
        }
        else
        {
            mToxicWaste = AddSpell(SPELL_TOXIC_WASTE, Target_Current, 0, 0, 0);
            mPoisonNova = AddSpell(SPELL_POISON_NOVA, Target_Self, 0, 5000, 0);
        }

        // Timers
        mMightyKickTimer = INVALIDATE_TIMER;
        mPursueTimer = INVALIDATE_TIMER;
        mPoisonNovaTimer = INVALIDATE_TIMER;
        mExplosionBarageTimer = INVALIDATE_TIMER;
        mToxicWasteTimer = INVALIDATE_TIMER;
        mShadowBoltTimer = INVALIDATE_TIMER;
        mExplosionBarageEndTimer = INVALIDATE_TIMER;
        mSpecialAttackTimer = INVALIDATE_TIMER;

        // Emotes
        // Krick
        mKrickAI = SpawnCreature(CN_KRICK, false);

        mKrickAI->AddEmote(Event_OnCombatStart, 8767);
        mKrickAI->AddEmote(Event_OnTargetDied, 8768);
        mKrickAI->AddEmote(Event_OnTargetDied, 8769);

        // Ick Spell Announcements
        mPursue->AddAnnouncement("Ick is chasing you!");
        mPoisonNova->AddAnnouncement("Ick begins to unleash a toxic poison cloud!");

        Phase = BATTLE;
    }

    void OnCombatStart(Unit* pTarget)
    {
        Phase = BATTLE;

        // Setip Timers
        mMightyKickTimer = AddTimer(20000);
        mSpecialAttackTimer = AddTimer(35000);
        mToxicWasteTimer = AddTimer(5000);
        mShadowBoltTimer = AddTimer(15000);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);

        ParentClass::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        Phase = OUTRO;

        ParentClass::OnCombatStop(pTarget);
    }

    void OnTargetDied(Unit* pTarget)
    {
        MoonScriptBossAI::OnTargetDied(pTarget);
    }

    void OnDied(Unit* mKiller)
    {
        RemoveAIUpdateEvent();
    }

    void AIUpdate()
    {
        if (Phase == BATTLE)
            if (!_unit->IsCasting())
                CastSpells();

        ParentClass::AIUpdate();
    }

    void CastSpells()
    {
        // Mighty Kick
        if (IsTimerFinished(mMightyKickTimer))
        {
            CastSpell(mMightyKick);
            ResetTimer(mMightyKickTimer, 25000);
        }

        // Toxic Waste
        if (IsTimerFinished(mToxicWasteTimer))
        {
            CastSpell(mToxicWaste);
            ResetTimer(mToxicWasteTimer, 5000);
        }

        // Shadow Bolt
        if (IsTimerFinished(mShadowBoltTimer))
        {
            CastSpell(mShadowBolt);
            ResetTimer(mShadowBoltTimer, 15000);
        }

        // Special Attack
        if (IsTimerFinished(mSpecialAttackTimer))
        {
            switch (RandomUInt(2))
            {
                case 0:
                    mPursueTimer = AddTimer(1000);
                    break;
                case 1:
                    mPoisonNovaTimer = AddTimer(1000);
                    break;
                case 2:
                    mExplosionBarageTimer = AddTimer(1000);
                    break;
            }
            ResetTimer(mSpecialAttackTimer, 28000);
        }

        // Poison Nova
        if (IsTimerFinished(mPoisonNovaTimer))
        {
            if (mKrickAI)
                mKrickAI->sendDBChatMessage(8770);

            CastSpell(mPoisonNova);
            RemoveTimer(mPoisonNovaTimer);
        }

        // Pursue
        if (IsTimerFinished(mPursueTimer))
        {
            if (mKrickAI)
            {
                switch (RandomUInt(2))
                {
                    case 0:
                        mKrickAI->sendDBChatMessage(8771);//Chase 1
                        break;
                    case 1:
                        mKrickAI->sendDBChatMessage(8772);//Chase 2
                        break;
                    case 2:
                        mKrickAI->sendDBChatMessage(8773);//Chase 3
                        break;
                }
            }

            Unit* pTarget = GetBestPlayerTarget(TargetFilter_NotCurrent);
            if (pTarget != NULL)
            {
                ClearHateList();
                _unit->GetAIInterface()->setNextTarget(pTarget);
                _unit->GetAIInterface()->modThreatByPtr(pTarget, 1000);
                CastSpellOnTarget(pTarget, TargetGen_Current, mPursue->mInfo, true);
            }

            CastSpell(mConfusion);
            RemoveTimer(mPursueTimer);
        }

        // Explosive Barage
        if (IsTimerFinished(mExplosionBarageTimer))
        {
            if (mKrickAI)
            {
                mKrickAI->sendDBChatMessage(8774);
                mKrickAI->Announce("Krick begins rapidly conjuring explosive mines!");
                mKrickAI->CastSpell(mExplosionBarageKrick);
            }
            
            _unit->setMoveRoot(true);
            CastSpell(mExplosionBarage);

            mExplosionBarageEndTimer = AddTimer(20000);

            ResetTimer(mSpecialAttackTimer, GetTimer(mSpecialAttackTimer) + 20000);
            ResetTimer(mMightyKickTimer, GetTimer(mMightyKickTimer) + 20000);
            ResetTimer(mToxicWasteTimer, GetTimer(mToxicWasteTimer) + 20000);
            ResetTimer(mShadowBoltTimer, GetTimer(mShadowBoltTimer) + 20000);
            RemoveTimer(mExplosionBarageTimer);
        }

        // Explosive Barage End
        if (IsTimerFinished(mExplosionBarageEndTimer))
        {
            _unit->setMoveRoot(false);
            RemoveTimer(mExplosionBarageEndTimer);
        }
    }

    MoonInstanceScript* mInstance;
    MoonScriptCreatureAI* mKrickAI;
    int32_t mMightyKickTimer;
    int32_t mPursueTimer;
    int32_t mPoisonNovaTimer;
    int32_t mExplosionBarageTimer;
    int32_t mExplosionBarageEndTimer;
    int32_t mToxicWasteTimer;
    int32_t mShadowBoltTimer;
    int32_t mSpecialAttackTimer;
    SpellDesc* mMightyKick;
    SpellDesc* mPursue;
    SpellDesc* mPoisonNova;
    SpellDesc* mExplosionBarage;
    SpellDesc* mToxicWaste;
    SpellDesc* mShadowBolt;
    SpellDesc* mConfusion;
    SpellDesc* mExplosionBarageKrick;
    BattlePhases Phase;
};

class KrickAI : MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(KrickAI, MoonScriptBossAI);
    KrickAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Get Instance Script
        mInstance = GetInstanceScript();

        // Timer
        mOutroTimer = INVALIDATE_TIMER;
        mBarrageTimer = INVALIDATE_TIMER;

        mBarrageSummon = nullptr;

        // Ick
        mIckAI = nullptr;
        JainaOrSylvanas = nullptr;

        //Outro - set 0 on combat start
        sequence = 0;

        // Outro Execute only 1 time
        mOutroTimerStarted = false;

        // Set Battle
        Phase = BATTLE;
    }

    void OnCombatStart(Unit* pTarget)
    {
        // Set Battle
        Phase = BATTLE;

        // Get Ick
        mIckAI = getNearestCreature(CN_ICK);

        // Spell Timers
        mBarrageTimer = AddTimer(2500); // Timer Quessed
        ParentClass::OnCombatStart(pTarget);
    }

    void AIUpdate()
    {
        if (!mIckAI->isAlive())
            Phase = OUTRO;

        if (Phase == BATTLE)
        {
            if (_unit->HasAura(SPELL_EXPLOSIVE_BARRAGE_KRICK))
            {
                if (IsTimerFinished(mBarrageTimer))
                {
                    CastOnInrangePlayers(0, 60.0f, SPELL_EXPLOSIVE_BARRAGE_SUMMON, true);
                    ResetTimer(mBarrageTimer, 2500);
                }
            }
        }
        else if (Phase == OUTRO)
        {
            Outro();
        }

        ParentClass::AIUpdate();
    }

    void Outro()
    {
        Player* pTarget = getNearestPlayer();
        if (pTarget == nullptr)
            return;

        if (IsTimerFinished(mOutroTimer))
            ++sequence;

        if (!mOutroTimerStarted)
        {
            GetUnit()->SetPosition(833.19f, 115.79f, 510.0f, 3.42673f, false);
            _unit->CastSpell(_unit, SPELL_STRANGULATE, true);
            _unit->setMoveRoot(true);
            ClearHateList();

            setCanEnterCombat(false);

            // Clear Hatelist dont allow Combat and root the Unit
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
            _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);
            _unit->GetAIInterface()->WipeTargetList();
            _unit->GetAIInterface()->WipeHateList();

            if (pTarget->IsTeamHorde())
                JainaOrSylvanas = SpawnCreature(CN_SYLVANAS_WINDRUNNER, 816.58f, 111.53f, 510.0f, 0.3825f, false);
            else
                JainaOrSylvanas = SpawnCreature(CN_JAINA_PROUDMOORE, 816.58f, 111.53f, 510.0f, 0.3825f, false);

            mOutroTimerStarted = true;
            mOutroTimer = AddTimer(2000);
        }

        if (IsTimerFinished(mOutroTimer))
        {
            switch (sequence)
            {
                case 1:              
                    sendDBChatMessage(8775);
                    ResetTimer(mOutroTimer, 14000);
                    break;
                case 2:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8776); // SAY_JAYNA_OUTRO_2
                        else
                            JainaOrSylvanas->sendDBChatMessage(8777); // SAY_SYLVANAS_OUTRO_2
                    }
                    ResetTimer(mOutroTimer, 8500);
                    break;
                case 3:
                    sendDBChatMessage(8778); // SAY_KRICK_OUTRO_3
                    ResetTimer(mOutroTimer, 12000);
                    break;
                case 4:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8779); // SAY_JAYNA_OUTRO_4
                        else
                            JainaOrSylvanas->sendDBChatMessage(8780); // SAY_SYLVANAS_OUTRO_4
                    }
                    ResetTimer(mOutroTimer, 8000);
                    break;
                case 5:
                    sendDBChatMessage(8781); // SAY_KRICK_OUTRO_5
                    ResetTimer(mOutroTimer, 4000);
                    break;
                case 6:
                    // TODO spawn Tyrannus at some distance and MovePoint near-by (flying on rimefang)
                    // Adjust timer so tyrannus has time to come
                    ResetTimer(mOutroTimer, 1);
                    break;
                case 7:
                    sendDBChatMessage(8782); // SAY_TYRANNUS_OUTRO_7
                    ResetTimer(mOutroTimer, 7000);
                    break;
                case 8:
                    sendDBChatMessage(8783); // SAY_KRICK_OUTRO_8
                    ResetTimer(mOutroTimer, 6000);
                    break;
                case 9:
                    // tyrannus kills krick
                    _unit->SetStandState(STANDSTATE_DEAD);
                    _unit->SetHealth(1);
                    sendDBChatMessage(8784); // SAY_TYRANNUS_OUTRO_9
                    ResetTimer(mOutroTimer, 12000);
                    break;
                case 10:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance() && JainaOrSylvanas)
                            JainaOrSylvanas->sendDBChatMessage(8785); // SAY_JAYNA_OUTRO_10
                        else
                            JainaOrSylvanas->sendDBChatMessage(8786); // SAY_SYLVANAS_OUTRO_10
                    }
                    ResetTimer(mOutroTimer, 8000);
                    break;
                case 11:
                    _unit->Despawn(1, 0);
                    JainaOrSylvanas->despawn(1, 0);
                    RemoveTimer(mOutroTimer);
                    break;
            }
        }
    }

    MoonInstanceScript* mInstance;
    Creature* mIckAI;
    MoonScriptCreatureAI* JainaOrSylvanas;
    SpellDesc* mBarrageSummon;
    uint8_t sequence;
    int32_t mOutroTimer;
    int32_t mBarrageTimer;
    bool mOutroTimerStarted;
    BattlePhases Phase;
};

// Barrage Spell Creature
class BarrageAI : public MoonScriptBossAI
{
    public:
        MOONSCRIPT_FACTORY_FUNCTION(BarrageAI, MoonScriptBossAI);
        BarrageAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
        {
            _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_2);
            _unit->CastSpell(_unit, SPELL_EXPLODING_ORB, false);
            _unit->CastSpell(_unit, SPELL_AUTO_GROW, false);

            // Invisibility Hack
            _unit->SetDisplayId(11686);

            // AIUpdate
            RegisterAIUpdateEvent(500);
        }

        void AIUpdate()
        {
            if (_unit->HasAura(SPELL_HASTY_GROW))
            {
                if (_unit->GetAuraStackCount(SPELL_HASTY_GROW) >= 15)
                {
                    _unit->CastSpell(_unit, SPELL_EXPLOSIVE_BARRAGE_DAMAGE, true);
                    _unit->Despawn(100, 0);
                }
            }
        }
};

class SylvanasAI : public MoonScriptBossAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(SylvanasAI, MoonScriptBossAI);
    SylvanasAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
    }
};

class JainaAI : public MoonScriptBossAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(JainaAI, MoonScriptBossAI);
    JainaAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
    }
};

// Scourgelord Tyrannus and Rimefang

void SetupPitOfSaron(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_PIT_OF_SARON, &InstancePitOfSaronScript::Create);
#endif
    mgr->register_creature_script(CN_FORGEMASTER_GARFROST, &ForgemasterGarfrostAI::Create);
    mgr->register_creature_script(CN_ICK, &IckAI::Create);
    mgr->register_creature_script(CN_KRICK, &KrickAI::Create);
    mgr->register_creature_script(CREATURE_EXPLODING_ORB, &BarrageAI::Create);
    mgr->register_creature_script(CN_JAINA_PROUDMOORE, &JainaAI::Create);
    mgr->register_creature_script(CN_SYLVANAS_WINDRUNNER, &SylvanasAI::Create);
}
