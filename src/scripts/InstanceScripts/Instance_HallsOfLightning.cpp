/*
 * ArcScripts for ArcEmu MMORPG Server
 * Copyright (C) 2009-2010 ArcEmu Team <http://www.arcemu.org/>
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
#include "Instance_HallsOfLightning.h"


/////////////////////////////////////////////////////////////////////////////////
/// Halls of Lightning Instance
class HallsOfLightningScript : public MoonInstanceScript
{
    public:
        uint32        mGeneralGUID;
        uint32        mVolkhanGUID;
        uint32        mLokenGUID;
        uint32        mIonarGUID;

        uint32        mGeneralDoorsGUID;
        uint32        mVolkhanDoorsGUID;
        uint32        mLokenDoorsGUID;
        uint32        mIonarDoors1GUID;
        uint32        mIonarDoors2GUID;

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(HallsOfLightningScript, MoonInstanceScript);
        HallsOfLightningScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            mGeneralGUID = 0;
            mVolkhanGUID = 0;
            mLokenGUID = 0;
            mIonarGUID = 0;

            mGeneralDoorsGUID = 0;
            mVolkhanDoorsGUID = 0;
            mLokenDoorsGUID = 0;
            mIonarDoors1GUID = 0;
            mIonarDoors2GUID = 0;
        };

        void OnCreaturePushToWorld(Creature* pCreature)
        {
            switch (pCreature->GetEntry())
            {
                case CN_GENERAL_BJARNGRIM:
                {
                    mGeneralGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_GENERAL_BJARNGRIM, BossData(0, mGeneralGUID)));
                }
                break;
                case CN_VOLKHAN:
                {
                    mVolkhanGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_VOLKHAN, BossData(0, mVolkhanGUID)));
                }
                break;
                case CN_LOKEN:
                {
                    mLokenGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_LOKEN, BossData(0, mLokenGUID)));
                }
                break;
                case CN_IONAR:
                {
                    mIonarGUID = pCreature->GetLowGUID();
                    mEncounters.insert(EncounterMap::value_type(CN_IONAR, BossData(0, mIonarGUID)));
                }
                break;
            };
        };

        void OnGameObjectPushToWorld(GameObject* pGameObject)
        {
            switch (pGameObject->GetEntry())
            {
                case GO_GENERAL_DOORS:
                    mGeneralDoorsGUID = pGameObject->GetLowGUID();
                    break;
                case GO_VOLKHAN_DOORS:
                    mVolkhanDoorsGUID = pGameObject->GetLowGUID();
                    break;
                case GO_LOKEN_DOORS:
                    mLokenDoorsGUID = pGameObject->GetLowGUID();
                    break;
                case GO_IONAR_DOORS1:
                    mIonarDoors1GUID = pGameObject->GetLowGUID();
                    break;
                case GO_IONAR_DOORS2:
                    mIonarDoors2GUID = pGameObject->GetLowGUID();
                    break;
            }

            ParentClass::OnGameObjectPushToWorld(pGameObject);
        };

        void SetInstanceData(uint32 pType, uint32 pIndex, uint32 pData)
        {
            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = (EncounterState)pData;
        };

        uint32 GetInstanceData(uint32 pType, uint32 pIndex)
        {
            if (pType != Data_EncounterState || pIndex == 0)
                return 0;

            EncounterMap::iterator Iter = mEncounters.find(pIndex);
            if (Iter == mEncounters.end())
                return 0;

            return (*Iter).second.mState;
        };

        void OnCreatureDeath(Creature* pVictim, Unit* pKiller)
        {
            EncounterMap::iterator Iter = mEncounters.find(pVictim->GetEntry());
            if (Iter == mEncounters.end())
                return;

            (*Iter).second.mState = State_Finished;

            GameObject* pDoors = NULL;
            switch (pVictim->GetEntry())
            {
                case CN_GENERAL_BJARNGRIM:
                {
                    SetInstanceData(Data_EncounterState, CN_GENERAL_BJARNGRIM, State_Finished);
                    pDoors = GetGameObjectByGuid(mGeneralDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_VOLKHAN:
                {
                    SetInstanceData(Data_EncounterState, CN_VOLKHAN, State_Finished);
                    pDoors = GetGameObjectByGuid(mVolkhanDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_LOKEN:
                {
                    SetInstanceData(Data_EncounterState, CN_LOKEN, State_Finished);
                    pDoors = GetGameObjectByGuid(mLokenDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_IONAR:
                {
                    SetInstanceData(Data_EncounterState, CN_IONAR, State_Finished);
                    pDoors = GetGameObjectByGuid(mIonarDoors1GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);

                    pDoors = GetGameObjectByGuid(mIonarDoors2GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
            };
        };
};

const uint32 TIMER_STANCE_CHANGE = 18000;

enum GENERAL_STANCES
{
    STANCE_BATTLE = 1,
    STANCE_BERSERKER = 2,
    STANCE_DEFENSIVE = 3,
};

/////////////////////////////////////////////////////////////////////////////////
/// General Bjarnrim Script
class GeneralBjarngrimAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(GeneralBjarngrimAI, MoonScriptBossAI);
    GeneralBjarngrimAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        mInstance = GetInstanceScript();
        // Battle Stance
        AddPhaseSpell(1, AddSpell(SPELL_MORTAL_STRIKE, Target_Current, 25, 0, 5));
        AddPhaseSpell(1, AddSpell(SPELL_WHIRLWIND, Target_Self, 90, 8, 30));

        // Berserker Stance
        AddPhaseSpell(2, AddSpell(SPELL_CLEAVE, Target_Current, 30, 0, 5));

        // Defensive Stance
        AddPhaseSpell(3, AddSpell(SPELL_SPELL_REFLECTION, Target_Self, 20, 0, 10));
        AddPhaseSpell(3, AddSpell(SPELL_INTERCEPT, Target_RandomPlayerNotCurrent, 40, 0, 6));
        AddPhaseSpell(3, AddSpell(SPELL_PUMMEL, Target_Current, 40, 0, 5));

        mStanceTimer = INVALIDATE_TIMER;
    };

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(758);      // I am the greatest of my father's sons! Your end has come!

        mStanceTimer = AddTimer(TIMER_STANCE_CHANGE + (RandomUInt(7) * 1000));
        switchStance(RandomUInt(2));

        ParentClass::OnCombatStart(pTarget);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
    };

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(1))
        {
           case 0:
              sendDBChatMessage(762);        // So ends your curse.
           break;
           case 1:
              sendDBChatMessage(763);        // Flesh... is... weak!
           break;
        }
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        ParentClass::OnCombatStop(pTarget);
    };

    void AIUpdate()
    {
        if (IsTimerFinished(mStanceTimer))
        {
            switch (GetPhase())
            {
                case STANCE_BATTLE:
                    switchStance(RandomUInt(1) + 2);
                    break;
                case STANCE_BERSERKER:
                    if (RandomUInt(1) == 1)
                        switchStance(STANCE_BATTLE);
                    else
                        switchStance(STANCE_DEFENSIVE);
                    break;
                case STANCE_DEFENSIVE:
                    switchStance(RandomUInt(1) + 1);
                    break;
            };


            ResetTimer(mStanceTimer, TIMER_STANCE_CHANGE + (RandomUInt(7) * 1000));
        };

        ParentClass::AIUpdate();
    };

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(765);      // How can it be...? Flesh is not... stronger!
    }

    void switchStance(int32 pStance)
    {
        switch (pStance)
        {
            case STANCE_BATTLE:
                ApplyAura(SPELL_BATTLE_AURA);
                ApplyAura(SPELL_BATTLE_STANCE);
                sendDBChatMessage(760);      // Defend yourself, for all the good it will do!
                Announce("General Bjarngrim switches to Battle Stance!");
                SetPhase(1);
                break;
            case STANCE_BERSERKER:
                ApplyAura(SPELL_BERSERKER_AURA);
                ApplyAura(SPELL_BERSERKER_STANCE);
                sendDBChatMessage(761);      // GRAAAAAH! Behold the fury of iron and steel!
                Announce("General Bjarngrim switches to Berserker Stance!");
                SetPhase(2);
                break;
            case STANCE_DEFENSIVE:
                ApplyAura(SPELL_DEFENSIVE_AURA);
                ApplyAura(SPELL_DEFENSIVE_STANCE);
                sendDBChatMessage(759);      // Give me your worst!
                Announce("General Bjarngrim switches to Defensive Stance!");
                SetPhase(3);
                break;
        }
    };

    private:

        int32 mStanceTimer;
        MoonInstanceScript* mInstance;
};


// Main Spells
const uint32 SPELL_TEMPER = 52238;

// Molten Golem Spells
const uint32 SPELL_BLAST_WAVE = 23113;
// 24 seconds + up to 6
const uint32 TIMER_STOMP = 24000;


/////////////////////////////////////////////////////////////////////////////////
/// Volkhan Script
class Volkhan : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(Volkhan, MoonScriptCreatureAI);
    Volkhan(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mInstance = GetInstanceScript();

        AddSpell(HeroicInt(52237, 59529), Target_WoundedFriendly, 15, 1.5f, 15);
        mStomp = AddSpell(HeroicInt(52237, 59529), Target_Self, 0, 3, 0);

        mStomp->AddEmote("I will crush you beneath my boots!", CHAT_MSG_MONSTER_YELL, 13963);
        mStomp->AddEmote("All my work... undone!", CHAT_MSG_MONSTER_YELL, 13964);

        m_cVolkhanWP.x = 1328.666870f;
        m_cVolkhanWP.y = -97.022758f;
        m_cVolkhanWP.z = 56.675297f;
        m_cVolkhanWP.o = 2.235341f;

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_RUN, m_cVolkhanWP));
        mStompTimer = INVALIDATE_TIMER;
        mPhase = 0;
        m_bStomp = false;
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(769);      // It is you who have destroyed my children? You... shall... pay!
        mStompTimer = AddTimer(TIMER_STOMP + (RandomUInt(6) * 1000));
        mPhase = 0;

        ParentClass::OnCombatStart(pTarget);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                sendDBChatMessage(774);      // The armies of iron will conquer all!
                break;
            case 1:
                sendDBChatMessage(775);      // Feh! Pathetic!
                break;
            case 2:
                sendDBChatMessage(776);      // You have cost me too much work!
                break;
        }
    }

    void OnCombatStop(Unit* pTarget)
    {
        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);

        ParentClass::OnCombatStop(pTarget);
    }

    void AIUpdate()
    {
        if (IsTimerFinished(mStompTimer))
        {
            if (m_bStomp == false)
            {
                m_bStomp = true;
                Announce("Volkhan prepares to shatter his Brittle Golems!");
                CastSpellNowNoScheduling(mStomp);
                ResetTimer(mStompTimer, 3000);
            }
            else
            {
                DoStomp();
                ResetTimer(mStompTimer, TIMER_STOMP + (RandomUInt(6) * 1000));
            }
        }

        if (_getHealthPercent() <= (100 - (20 * mPhase)))
        {
            ForceWaypointMove(1);
            Announce("Volkhan runs to his anvil!");
            ++mPhase;
        }

        ParentClass::AIUpdate();
    }

    void OnReachWP(uint32 iWaypointId, bool bForwards)
    {
        if (iWaypointId == 1)
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(770);      // Life from lifelessness... death for you.
                    break;
                case 1:
                    sendDBChatMessage(771);      // Nothing is wasted in the process. You will see....
                    break;
            }

            Creature* pAnvil = getNearestCreature(CN_VOLKHANS_ANVIL);
            if (pAnvil)
                _unit->CastSpell(pAnvil, SPELL_TEMPER, true);
            else
                _unit->CastSpell(GetUnit(), SPELL_TEMPER, true);

            setCanEnterCombat(true);
            _unit->GetAIInterface()->AttackReaction(getNearestPlayer(), 1);   // hackfix
        }
    }

    void DoStomp()
    {
        for (std::set< Object* >::iterator itr = _unit->GetInRangeSetBegin(); itr != _unit->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr) && (*itr)->IsCreature() && (*itr)->GetEntry() == CN_BRITTLE_GOLEM)
            {
                Creature* pCreature = static_cast< Creature* >((*itr));
                pCreature->CastSpell(pCreature, HeroicInt(52429, 59527), true);

                pCreature->Despawn(1000, 0);
            };
        };

        m_bStomp = false;
    }

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(777);      // The master was right... to be concerned.
    }

    SpellDesc* mStomp;
    Movement::Location m_cVolkhanWP;
    bool m_bStomp;
    int32 mStompTimer;
    int32 mPhase;
    MoonInstanceScript* mInstance;
};


class MoltenGolem : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MoltenGolem, MoonScriptCreatureAI);
    MoltenGolem(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_BLAST_WAVE, Target_Self, 25, 0, 20);
        AddSpell(HeroicInt(52433, 59530), Target_Current, 15, 0, 15);
    };

    void OnDied(Unit* pKiller)
    {
        SpawnCreature(CN_BRITTLE_GOLEM, _unit->GetPositionX(), _unit->GetPositionY(), _unit->GetPositionZ(), _unit->GetOrientation(), false);
        despawn();
    }
};


class BrittleGolem : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(BrittleGolem, MoonScriptCreatureAI);
    BrittleGolem(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        setCanEnterCombat(false);
        setRooted(true);
    };
};


class VolkhansAnvil : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(VolkhansAnvil, MoonScriptCreatureAI);
    VolkhansAnvil(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        _unit->GetAIInterface()->SetAllowedToEnterCombat(false);
        _unit->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
    };

};

const uint32 DISPRESE = 52770;;
const uint32 SPELL_SUMMON_SPARK = 52746;
/////////////////////////////////////////////////////////////////////////////////
/// Ionar
// Status: Basic script, missing spark phase
class IonarAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(IonarAI, MoonScriptBossAI);
    IonarAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        mInstance = GetInstanceScript();

        AddSpell(HeroicInt(52780, 59800), Target_RandomPlayerNotCurrent, 20, 1.5f, 5);
        AddSpell(HeroicInt(52658, 59795), Target_RandomPlayerNotCurrent, 15, 0, 12);
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(738);      // You wish to confront the master? You must first weather the storm!

        ParentClass::OnCombatStart(pTarget);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                sendDBChatMessage(741);      // Shocking, I know.
                break;
            case 1:
                sendDBChatMessage(742);      // You attempt the impossible.
                break;
            case 2:
                sendDBChatMessage(743);      // Your spark of life is... extinguished.
                break;
        }
    }

    void OnCombatStop(Unit* pTarget)
    {
        ParentClass::OnCombatStop(pTarget);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
    }

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(744);      // Master... you have guests.
    }

    MoonInstanceScript* mInstance;
};


const uint32 PULSING_SHOCKWAVE_AURA = 59414;
const uint32 ARC_LIGHTNING = 52921;
// 14 seconds + random up to 8
const uint32 TIMER_NOVA = 14000;
const uint32 TIMER_RESPOND = 18000;

/////////////////////////////////////////////////////////////////////////////////
/// Loken
class LokenAI : public MoonScriptCreatureAI
{
    MOONSCRIPT_FACTORY_FUNCTION(LokenAI, MoonScriptCreatureAI);
    LokenAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        mInstance = GetInstanceScript();
        mNova = AddSpell(HeroicInt(52960, 59835), Target_Self, 0, 4.0f, 0);
        AddSpell(ARC_LIGHTNING, Target_RandomPlayer, 25, 0, 6);

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 14160, "I have witnessed the rise and fall of empires. The birth and extinction of entire species. Over countless millennia the foolishness of mortals has remained the only constant. Your presence here confirms this.");

        mNovaTimer = INVALIDATE_TIMER;
        mRespondTimer = AddTimer(TIMER_RESPOND);
        RegisterAIUpdateEvent(1000);
        mSpeech = 1;
    }

    void OnCombatStart(Unit* pTarget)
    {
        sendDBChatMessage(801);      // What hope is there for you? None!

        ParentClass::OnCombatStart(pTarget);
        mSpeech = 1;
        ApplyAura(HeroicInt(52961, 59836));
        mNovaTimer = AddTimer(TIMER_NOVA);
        CastOnAllInrangePlayers(PULSING_SHOCKWAVE_AURA);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_InProgress);
    }

    void OnCombatStop(Unit* pTarget)
    {
        RemoveAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
        ParentClass::OnCombatStop(pTarget);

        if (mInstance)
            mInstance->SetInstanceData(Data_EncounterState, _unit->GetEntry(), State_Performed);
    }

    void OnTargetDied(Unit* pTarget)
    {
        switch (RandomUInt(2))
        {
            case 0:
                sendDBChatMessage(805);      // Only mortal...
                break;
            case 1:
                sendDBChatMessage(806);      // I... am... FOREVER!
                break;
            case 2:
                sendDBChatMessage(807);      // What little time you had, you wasted!
                break;
        }
    }

    void OnDied(Unit* pKiller)
    {
        sendDBChatMessage(811);      // My death... heralds the end of this world.

        RemoveAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
        ParentClass::OnDied(pKiller);
    }

    void AIUpdate()
    {
        if (IsTimerFinished(mNovaTimer))
        {
            switch (RandomUInt(2))
            {
                case 0:
                    sendDBChatMessage(802);      // You cannot hide from fate!
                    break;
                case 1:
                    sendDBChatMessage(803);      // Come closer. I will make it quick.
                    break;
                case 2:
                    sendDBChatMessage(804);      // Your flesh cannot hold out for long.
                    break;
            };

            Announce("Loken begins to cast Lightning Nova!");
            CastSpellNowNoScheduling(mNova);
            ResetTimer(mNovaTimer, TIMER_NOVA + (RandomUInt(8) * 1000));
        };

        if (mSpeech == 4)
            return;

        if (_getHealthPercent() <= (100 - (25 * mSpeech)))
        {
            switch (mSpeech) //rand() % 2
            {
                case 1:
                    sendDBChatMessage(808);      // You stare blindly into the abyss!
                    break;
                case 2:
                    sendDBChatMessage(809);      // Your ignorance is profound. Can you not see where this path leads?
                    break;
                case 3:
                    sendDBChatMessage(810);      // You cross the precipice of oblivion!
                    break;
            };

            ++mSpeech;
        };

        if (IsTimerFinished(mRespondTimer))
        {
            sendDBChatMessage(800);      // My master has shown me the future, and you have no place in it. Azeroth..
            RemoveTimer(mRespondTimer);
            RemoveAIUpdateEvent();
        };

        ParentClass::AIUpdate();
    };

    SpellDesc* mNova;
    MoonInstanceScript* mInstance;

    int32 mNovaTimer;
    int32 mRespondTimer;
    uint8 mSpeech;
};

void SetupHallsOfLightning(ScriptMgr* mgr)
{
#ifndef UseNewMapScriptsProject
    mgr->register_instance_script(MAP_HALLS_OF_LIGHTNING, &HallsOfLightningScript::Create);
#endif
    mgr->register_creature_script(CN_GENERAL_BJARNGRIM, &GeneralBjarngrimAI::Create);

    mgr->register_creature_script(CN_VOLKHAN, &Volkhan::Create);
    mgr->register_creature_script(CN_MOLTEN_GOLEM, &MoltenGolem::Create);
    mgr->register_creature_script(CN_BRITTLE_GOLEM, &BrittleGolem::Create);
    mgr->register_creature_script(CN_VOLKHANS_ANVIL, &VolkhansAnvil::Create);

    mgr->register_creature_script(CN_IONAR, &IonarAI::Create);
    mgr->register_creature_script(CN_LOKEN, &LokenAI::Create);
}
