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
class HallsOfLightningScript : public InstanceScript
{
    public:
        uint32        mGeneralDoorsGUID;
        uint32        mVolkhanDoorsGUID;
        uint32        mLokenDoorsGUID;
        uint32        mIonarDoors1GUID;
        uint32        mIonarDoors2GUID;

        HallsOfLightningScript(MapMgr* pMapMgr) : InstanceScript(pMapMgr)
        {
            mGeneralDoorsGUID = 0;
            mVolkhanDoorsGUID = 0;
            mLokenDoorsGUID = 0;
            mIonarDoors1GUID = 0;
            mIonarDoors2GUID = 0;
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new HallsOfLightningScript(pMapMgr); }

        void OnGameObjectPushToWorld(GameObject* pGameObject) override
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
        }

        void OnCreatureDeath(Creature* pVictim, Unit* /*pKiller*/) override
        {
            GameObject* pDoors = NULL;
            switch (pVictim->GetEntry())
            {
                case CN_GENERAL_BJARNGRIM:
                {
                    pDoors = GetGameObjectByGuid(mGeneralDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_VOLKHAN:
                {
                    pDoors = GetGameObjectByGuid(mVolkhanDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_LOKEN:
                {
                    pDoors = GetGameObjectByGuid(mLokenDoorsGUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
                case CN_IONAR:
                {
                    pDoors = GetGameObjectByGuid(mIonarDoors1GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);

                    pDoors = GetGameObjectByGuid(mIonarDoors2GUID);
                    if (pDoors)
                        pDoors->SetState(GO_STATE_OPEN);
                }
                break;
            }
        }
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
class GeneralBjarngrimAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(GeneralBjarngrimAI);
    GeneralBjarngrimAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
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

        // new
        addEmoteForEvent(Event_OnCombatStart, 758);      // I am the greatest of my father's sons! Your end has come!
        addEmoteForEvent(Event_OnTargetDied, 762);        // So ends your curse.
        addEmoteForEvent(Event_OnTargetDied, 763);        // Flesh... is... weak!
        addEmoteForEvent(Event_OnDied, 765);      // How can it be...? Flesh is not... stronger!
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStanceTimer = _addTimer(TIMER_STANCE_CHANGE + (RandomUInt(7) * 1000));
        switchStance(RandomUInt(2));
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mStanceTimer))
        {
            switch (getScriptPhase())
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
            }
            _resetTimer(mStanceTimer, TIMER_STANCE_CHANGE + (RandomUInt(7) * 1000));
        }
    }

    // case for scriptPhase and spell emotes
    void switchStance(int32 pStance)
    {
        switch (pStance)
        {
            case STANCE_BATTLE:
                _applyAura(SPELL_BATTLE_AURA);
                _applyAura(SPELL_BATTLE_STANCE);
                sendDBChatMessage(760);      // Defend yourself, for all the good it will do!
                sendAnnouncement("General Bjarngrim switches to Battle Stance!");
                setScriptPhase(1);
                break;
            case STANCE_BERSERKER:
                _applyAura(SPELL_BERSERKER_AURA);
                _applyAura(SPELL_BERSERKER_STANCE);
                sendDBChatMessage(761);      // GRAAAAAH! Behold the fury of iron and steel!
                sendAnnouncement("General Bjarngrim switches to Berserker Stance!");
                setScriptPhase(2);
                break;
            case STANCE_DEFENSIVE:
                _applyAura(SPELL_DEFENSIVE_AURA);
                _applyAura(SPELL_DEFENSIVE_STANCE);
                sendDBChatMessage(759);      // Give me your worst!
                sendAnnouncement("General Bjarngrim switches to Defensive Stance!");
                setScriptPhase(3);
                break;
        }
    }

    private:

        int32 mStanceTimer;
};


// Main Spells
const uint32 SPELL_TEMPER = 52238;

// Molten Golem Spells
const uint32 SPELL_BLAST_WAVE = 23113;
// 24 seconds + up to 6
const uint32 TIMER_STOMP = 24000;


/////////////////////////////////////////////////////////////////////////////////
/// Volkhan Script
class Volkhan : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(Volkhan);
    Volkhan(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            AddSpell(59529, Target_WoundedFriendly, 15, 1.5f, 15);
            mStomp = AddSpell(59529, Target_Self, 0, 3, 0);
        }
        else
        {
            AddSpell(52237, Target_WoundedFriendly, 15, 1.5f, 15);
            mStomp = AddSpell(52237, Target_Self, 0, 3, 0);
        }

        mStomp->addEmote("I will crush you beneath my boots!", CHAT_MSG_MONSTER_YELL, 13963);
        mStomp->addEmote("All my work... undone!", CHAT_MSG_MONSTER_YELL, 13964);

        m_cVolkhanWP.x = 1328.666870f;
        m_cVolkhanWP.y = -97.022758f;
        m_cVolkhanWP.z = 56.675297f;
        m_cVolkhanWP.o = 2.235341f;

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        AddWaypoint(CreateWaypoint(1, 0, Movement::WP_MOVE_TYPE_RUN, m_cVolkhanWP));
        mStompTimer = INVALIDATE_TIMER;
        mPhase = 0;
        m_bStomp = false;

        // new
        addEmoteForEvent(Event_OnCombatStart, 769);      // It is you who have destroyed my children? You... shall... pay!
        addEmoteForEvent(Event_OnTargetDied, 774);      // The armies of iron will conquer all!
        addEmoteForEvent(Event_OnTargetDied, 775);      // Feh! Pathetic!
        addEmoteForEvent(Event_OnTargetDied, 776);      // You have cost me too much work!
        addEmoteForEvent(Event_OnDied, 777);      // The master was right... to be concerned.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mStompTimer = _addTimer(TIMER_STOMP + (RandomUInt(6) * 1000));
        mPhase = 0;
    }

    void AIUpdate() override
    {
        if (_isTimerFinished(mStompTimer))
        {
            if (m_bStomp == false)
            {
                m_bStomp = true;
                sendAnnouncement("Volkhan prepares to shatter his Brittle Golems!");
                CastSpellNowNoScheduling(mStomp);
                _resetTimer(mStompTimer, 3000);
            }
            else
            {
                DoStomp();
                _resetTimer(mStompTimer, TIMER_STOMP + (RandomUInt(6) * 1000));
            }
        }

        if (_getHealthPercent() <= (100 - (20 * mPhase)))
        {
            ForceWaypointMove(1);
            sendAnnouncement("Volkhan runs to his anvil!");
            ++mPhase;
        }

        
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
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
                getCreature()->CastSpell(pAnvil, SPELL_TEMPER, true);
            else
                getCreature()->CastSpell(getCreature(), SPELL_TEMPER, true);

            setCanEnterCombat(true);
            getCreature()->GetAIInterface()->AttackReaction(getNearestPlayer(), 1);   // hackfix
        }
    }

    void DoStomp()
    {
        for (std::set< Object* >::iterator itr = getCreature()->GetInRangeSetBegin(); itr != getCreature()->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr) && (*itr)->IsCreature() && (*itr)->GetEntry() == CN_BRITTLE_GOLEM)
            {
                Creature* pCreature = static_cast< Creature* >((*itr));
                if (_isHeroic())
                    pCreature->CastSpell(pCreature, 59527, true);
                else
                    pCreature->CastSpell(pCreature, 52429, true);

                pCreature->Despawn(1000, 0);
            }
        }

        m_bStomp = false;
    }

    SpellDesc* mStomp;
    Movement::Location m_cVolkhanWP;
    bool m_bStomp;
    int32 mStompTimer;
    int32 mPhase;
};


class MoltenGolem : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(MoltenGolem);
    MoltenGolem(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        AddSpell(SPELL_BLAST_WAVE, Target_Self, 25, 0, 20);

        if (_isHeroic())
            AddSpell(59530, Target_Current, 15, 0, 15);
        else
            AddSpell(52433, Target_Current, 15, 0, 15);
    }

    void OnDied(Unit* /*pKiller*/) override
    {
        spawnCreature(CN_BRITTLE_GOLEM, getCreature()->GetPosition());
        despawn();
    }
};


class BrittleGolem : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(BrittleGolem);
    BrittleGolem(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        setCanEnterCombat(false);
        setRooted(true);
    }
};


class VolkhansAnvil : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(VolkhansAnvil);
    VolkhansAnvil(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
        getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        setRooted(true);
    }

};

const uint32 DISPRESE = 52770;;
const uint32 SPELL_SUMMON_SPARK = 52746;
/////////////////////////////////////////////////////////////////////////////////
/// Ionar
// Status: Basic script, missing spark phase
class IonarAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(IonarAI);
    IonarAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
        {
            AddSpell(59800, Target_RandomPlayerNotCurrent, 20, 1.5f, 5);
            AddSpell(59795, Target_RandomPlayerNotCurrent, 15, 0, 12);
        }
        else
        {
            AddSpell(52780, Target_RandomPlayerNotCurrent, 20, 1.5f, 5);
            AddSpell(52658, Target_RandomPlayerNotCurrent, 15, 0, 12);
        }

        // new
        addEmoteForEvent(Event_OnCombatStart, 738);      // You wish to confront the master? You must first weather the storm!
        addEmoteForEvent(Event_OnTargetDied, 741);      // Shocking, I know.
        addEmoteForEvent(Event_OnTargetDied, 742);      // You attempt the impossible.
        addEmoteForEvent(Event_OnTargetDied, 743);      // Your spark of life is... extinguished.
        addEmoteForEvent(Event_OnDied, 744);      // Master... you have guests
    }
};


const uint32 PULSING_SHOCKWAVE_AURA = 59414;
const uint32 ARC_LIGHTNING = 52921;
// 14 seconds + random up to 8
const uint32 TIMER_NOVA = 14000;
const uint32 TIMER_RESPOND = 18000;

/////////////////////////////////////////////////////////////////////////////////
/// Loken
class LokenAI : public CreatureAIScript
{
    ADD_CREATURE_FACTORY_FUNCTION(LokenAI);
    LokenAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        if (_isHeroic())
            mNova = AddSpell(59835, Target_Self, 0, 4.0f, 0);
        else
            mNova = AddSpell(52960, Target_Self, 0, 4.0f, 0);
        
        AddSpell(ARC_LIGHTNING, Target_RandomPlayer, 25, 0, 6);

        sendChatMessage(CHAT_MSG_MONSTER_YELL, 14160, "I have witnessed the rise and fall of empires. The birth and extinction of entire species. Over countless millennia the foolishness of mortals has remained the only constant. Your presence here confirms this.");

        mNovaTimer = INVALIDATE_TIMER;
        mRespondTimer = _addTimer(TIMER_RESPOND);
        RegisterAIUpdateEvent(1000);
        mSpeech = 1;

        // new
        addEmoteForEvent(Event_OnCombatStart, 801);      // What hope is there for you? None!
        addEmoteForEvent(Event_OnTargetDied, 805);      // Only mortal...
        addEmoteForEvent(Event_OnTargetDied, 806);      // I... am... FOREVER!
        addEmoteForEvent(Event_OnTargetDied, 807);      // What little time you had, you wasted!
        addEmoteForEvent(Event_OnDied, 811);      // My death... heralds the end of this world.
    }

    void OnCombatStart(Unit* /*pTarget*/) override
    {
        mSpeech = 1;

        if (_isHeroic())
            _applyAura(59836);
        else
            _applyAura(52961);

        mNovaTimer = _addTimer(TIMER_NOVA);
        _castOnInrangePlayers(PULSING_SHOCKWAVE_AURA);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        _removeAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
    }


    void OnDied(Unit* /*pKiller*/) override
    {
        _removeAuraOnPlayers(PULSING_SHOCKWAVE_AURA);
    }

    void AIUpdate() override
    {
        // spell emote
        if (_isTimerFinished(mNovaTimer))
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
            }

            sendAnnouncement("Loken begins to cast Lightning Nova!");
            CastSpellNowNoScheduling(mNova);
            _resetTimer(mNovaTimer, TIMER_NOVA + (RandomUInt(8) * 1000));
        }

        if (mSpeech == 4)
            return;

        // scriptPhase
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
            }

            ++mSpeech;
        }

        if (_isTimerFinished(mRespondTimer))
        {
            sendDBChatMessage(800);      // My master has shown me the future, and you have no place in it. Azeroth..
            _removeTimer(mRespondTimer);
            RemoveAIUpdateEvent();
        }

        
    }

    SpellDesc* mNova;

    int32 mNovaTimer;
    uint32 mRespondTimer;
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
