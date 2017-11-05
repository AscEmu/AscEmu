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

class InstancePitOfSaronScript : public InstanceScript
{
    public:

        InstancePitOfSaronScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {}

        static InstanceScript* Create(MapMgr* pMapMgr) { return new InstancePitOfSaronScript(pMapMgr); }

        void OnPlayerEnter(Player* player)
        {
            if (!spawnsCreated())
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

                setSpawnsCreated();
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
        mInstance = getInstanceScript();

        // Normal Spells
        mSaronite = AddSpell(SPELL_THROWSARONITE, Target_RandomPlayerDestination, 20, 2, 15);
        mPermafrost = AddSpell(SPELL_PERMAFROST, Target_Self, 0, 0, 0);

        if (_isHeroic())
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
        mSaroniteTimer = _addTimer(45000);
        mPermafrostTimer = _addTimer(2000);
        mChllingWaveTimer = _addTimer(10000);
        mDeepFreezeTimer = _addTimer(10000);

        if (mInstance)
            mInstance->setData(_unit->GetEntry(), InProgress);

        MoonScriptBossAI::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* mTarget)
    {
        // Clear Agent and Ai State
        setAIAgent(AGENT_NULL);
        _unit->GetAIInterface()->setAiState(AI_STATE_IDLE);

        if (mInstance)
            mInstance->setData(_unit->GetEntry(), Performed);

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

        if (GetPhase() == 1 && _getHealthPercent() <= 66)
        {
            sendDBChatMessage(8765);
            _unit->CastSpell(_unit, SPELL_STOMP, false);
            _unit->GetAIInterface()->WipeHateList();
            _unit->GetAIInterface()->splineMoveJump(JumpCords[0].x, JumpCords[0].y, JumpCords[0].z);
            
            if (GameObject * pObject = getNearestGameObject(401006))	//forgemaster's anvil (TEMP)
                _unit->SetFacing(_unit->calcRadAngle(_unit->GetPositionX(), _unit->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));

            if (_isHeroic())
                _unit->CastSpell(_unit, H_SPELL_FORGE_BLADE, false);
            else
                _unit->CastSpell(_unit, SPELL_FROZEBLADE, false);

            _unit->SetEquippedItem(MELEE, EQUIP_ID_SWORD);
            _unit->SetEquippedItem(OFFHAND, 0);
            SetPhase(2);
        }

        if (GetPhase() == 2 && _getHealthPercent() <= 33)
        {
            sendDBChatMessage(8766);
            _unit->CastSpell(_unit, SPELL_STOMP, false);
            _unit->GetAIInterface()->WipeHateList();
            _unit->GetAIInterface()->splineMoveJump(JumpCords[1].x, JumpCords[1].y, JumpCords[1].z);

            if (GameObject * pObject = getNearestGameObject(401006))	//forgemaster's anvil (TEMP)
                _unit->SetFacing(_unit->calcRadAngle(_unit->GetPositionX(), _unit->GetPositionY(), pObject->GetPositionX(), pObject->GetPositionY()));
            
            if (_isHeroic())
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
        if (_isTimerFinished(mSaroniteTimer))
        {
            // Casting Saronite Boulder every 45 secs.
            Unit* unit = GetBestUnitTarget(TargetFilter_None);
            CastSpellOnTarget(unit, TargetGen_RandomPlayer, mSaronite->mInfo, true);
            _resetTimer(mSaroniteTimer, 45000);
        }

        if (_isTimerFinished(mPermafrostTimer))
        {
            // Cast Permafrost every 2 secs.
            CastSpellNowNoScheduling(mPermafrost);
            _resetTimer(mPermafrostTimer, 2000);
        }

        if (_isTimerFinished(mChllingWaveTimer) && GetPhase() == 2)
        {
            // Cast Chilling Wave every 10 secs.
            CastSpell(mChllingWave);
            _resetTimer(mChllingWaveTimer, 10000);
        }

        if (_isTimerFinished(mDeepFreezeTimer) && GetPhase() == 3)
        {
            // Cast Deep Freeze every 10 secs.
            CastSpell(mDeepFreeze);
            _resetTimer(mDeepFreezeTimer, 10000);
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
    InstanceScript* mInstance;
};

// Ick and Krick

class IckAI : MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(IckAI, MoonScriptBossAI);
    IckAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Instance Script
        mInstance = getInstanceScript();

        // Spells
        mMightyKick = AddSpell(SPELL_MIGHTY_KICK, Target_Current, 25, 0, 0);
        mPursue = AddSpell(SPELL_PURSUED, Target_RandomPlayer, 0, 5000, 0);
        mExplosionBarage = AddSpell(SPELL_EXPLOSIVE_BARRAGE, Target_Self, 0, 0, 0);
        mShadowBolt = AddSpell(SPELL_SHADOW_BOLT, Target_Current, 0, 3000, 0);
        mConfusion = AddSpell(SPELL_CONFUSION, Target_Self, 0, 0, 0);
        mExplosionBarageKrick = AddSpell(SPELL_EXPLOSIVE_BARRAGE_KRICK, Target_Self, 0, 0, 0);

        if (_isHeroic())
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
        mKrickAI = SpawnCreature(CN_KRICK, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);

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
        mMightyKickTimer = _addTimer(20000);
        mSpecialAttackTimer = _addTimer(35000);
        mToxicWasteTimer = _addTimer(5000);
        mShadowBoltTimer = _addTimer(15000);

        if (mInstance)
            mInstance->setData(_unit->GetEntry(), InProgress);

        ParentClass::OnCombatStart(pTarget);
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->setData(_unit->GetEntry(), Performed);

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
        if (_isTimerFinished(mMightyKickTimer))
        {
            CastSpell(mMightyKick);
            _resetTimer(mMightyKickTimer, 25000);
        }

        // Toxic Waste
        if (_isTimerFinished(mToxicWasteTimer))
        {
            CastSpell(mToxicWaste);
            _resetTimer(mToxicWasteTimer, 5000);
        }

        // Shadow Bolt
        if (_isTimerFinished(mShadowBoltTimer))
        {
            CastSpell(mShadowBolt);
            _resetTimer(mShadowBoltTimer, 15000);
        }

        // Special Attack
        if (_isTimerFinished(mSpecialAttackTimer))
        {
            switch (RandomUInt(2))
            {
                case 0:
                    mPursueTimer = _addTimer(1000);
                    break;
                case 1:
                    mPoisonNovaTimer = _addTimer(1000);
                    break;
                case 2:
                    mExplosionBarageTimer = _addTimer(1000);
                    break;
            }
            _resetTimer(mSpecialAttackTimer, 28000);
        }

        // Poison Nova
        if (_isTimerFinished(mPoisonNovaTimer))
        {
            if (mKrickAI)
                mKrickAI->sendDBChatMessage(8770);

            CastSpell(mPoisonNova);
            _removeTimer(mPoisonNovaTimer);
        }

        // Pursue
        if (_isTimerFinished(mPursueTimer))
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
                _clearHateList();
                _unit->GetAIInterface()->setNextTarget(pTarget);
                _unit->GetAIInterface()->modThreatByPtr(pTarget, 1000);
                CastSpellOnTarget(pTarget, TargetGen_Current, mPursue->mInfo, true);
            }

            CastSpell(mConfusion);
            _removeTimer(mPursueTimer);
        }

        // Explosive Barage
        if (_isTimerFinished(mExplosionBarageTimer))
        {
            if (mKrickAI)
            {
                mKrickAI->sendDBChatMessage(8774);
                mKrickAI->Announce("Krick begins rapidly conjuring explosive mines!");
                mKrickAI->CastSpell(mExplosionBarageKrick);
            }
            
            _unit->setMoveRoot(true);
            CastSpell(mExplosionBarage);

            mExplosionBarageEndTimer = _addTimer(20000);

            _resetTimer(mSpecialAttackTimer, _getTimeForTimer(mSpecialAttackTimer) + 20000);
            _resetTimer(mMightyKickTimer, _getTimeForTimer(mMightyKickTimer) + 20000);
            _resetTimer(mToxicWasteTimer, _getTimeForTimer(mToxicWasteTimer) + 20000);
            _resetTimer(mShadowBoltTimer, _getTimeForTimer(mShadowBoltTimer) + 20000);
            _removeTimer(mExplosionBarageTimer);
        }

        // Explosive Barage End
        if (_isTimerFinished(mExplosionBarageEndTimer))
        {
            _unit->setMoveRoot(false);
            _removeTimer(mExplosionBarageEndTimer);
        }
    }

    InstanceScript* mInstance;
    MoonScriptCreatureAI* mKrickAI;
    int32_t mMightyKickTimer;
    uint32_t mPursueTimer;
    uint32_t mPoisonNovaTimer;
    uint32_t mExplosionBarageTimer;
    uint32_t mExplosionBarageEndTimer;
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
        mInstance = getInstanceScript();

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
        mBarrageTimer = _addTimer(2500); // Timer Quessed
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
                if (_isTimerFinished(mBarrageTimer))
                {
                    _castOnInrangePlayersWithinDist(0, 60.0f, SPELL_EXPLOSIVE_BARRAGE_SUMMON, true);
                    _resetTimer(mBarrageTimer, 2500);
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

        if (_isTimerFinished(mOutroTimer))
            ++sequence;

        if (!mOutroTimerStarted)
        {
            GetUnit()->SetPosition(833.19f, 115.79f, 510.0f, 3.42673f, false);
            _unit->CastSpell(_unit, SPELL_STRANGULATE, true);
            _unit->setMoveRoot(true);
            _clearHateList();

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
            mOutroTimer = _addTimer(2000);
        }

        if (_isTimerFinished(mOutroTimer))
        {
            switch (sequence)
            {
                case 1:              
                    sendDBChatMessage(8775);
                    _resetTimer(mOutroTimer, 14000);
                    break;
                case 2:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8776); // SAY_JAYNA_OUTRO_2
                        else
                            JainaOrSylvanas->sendDBChatMessage(8777); // SAY_SYLVANAS_OUTRO_2
                    }
                    _resetTimer(mOutroTimer, 8500);
                    break;
                case 3:
                    sendDBChatMessage(8778); // SAY_KRICK_OUTRO_3
                    _resetTimer(mOutroTimer, 12000);
                    break;
                case 4:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance())
                            JainaOrSylvanas->sendDBChatMessage(8779); // SAY_JAYNA_OUTRO_4
                        else
                            JainaOrSylvanas->sendDBChatMessage(8780); // SAY_SYLVANAS_OUTRO_4
                    }
                    _resetTimer(mOutroTimer, 8000);
                    break;
                case 5:
                    sendDBChatMessage(8781); // SAY_KRICK_OUTRO_5
                    _resetTimer(mOutroTimer, 4000);
                    break;
                case 6:
                    // TODO spawn Tyrannus at some distance and MovePoint near-by (flying on rimefang)
                    // Adjust timer so tyrannus has time to come
                    _resetTimer(mOutroTimer, 1);
                    break;
                case 7:
                    sendDBChatMessage(8782); // SAY_TYRANNUS_OUTRO_7
                    _resetTimer(mOutroTimer, 7000);
                    break;
                case 8:
                    sendDBChatMessage(8783); // SAY_KRICK_OUTRO_8
                    _resetTimer(mOutroTimer, 6000);
                    break;
                case 9:
                    // tyrannus kills krick
                    _unit->SetStandState(STANDSTATE_DEAD);
                    _unit->SetHealth(1);
                    sendDBChatMessage(8784); // SAY_TYRANNUS_OUTRO_9
                    _resetTimer(mOutroTimer, 12000);
                    break;
                case 10:
                    if (JainaOrSylvanas)
                    {
                        if (pTarget->IsTeamAlliance() && JainaOrSylvanas)
                            JainaOrSylvanas->sendDBChatMessage(8785); // SAY_JAYNA_OUTRO_10
                        else
                            JainaOrSylvanas->sendDBChatMessage(8786); // SAY_SYLVANAS_OUTRO_10
                    }
                    _resetTimer(mOutroTimer, 8000);
                    break;
                case 11:
                    _unit->Despawn(1, 0);
                    JainaOrSylvanas->despawn(1, 0);
                    _removeTimer(mOutroTimer);
                    break;
            }
        }
    }

    InstanceScript* mInstance;
    Creature* mIckAI;
    MoonScriptCreatureAI* JainaOrSylvanas;
    SpellDesc* mBarrageSummon;
    uint8_t sequence;
    uint32_t mOutroTimer;
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
