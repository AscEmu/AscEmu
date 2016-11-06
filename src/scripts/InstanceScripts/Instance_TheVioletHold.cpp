/*
Copyright (c) 2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_TheVioletHold.h"

enum DataIndex
{
    TVH_PHASE_1 = 0, // main event
    TVH_PHASE_2 = 1, // 1. portal
    TVH_PHASE_3 = 2, // 2. portal
    TVH_PHASE_4 = 3, // 3. portal
    TVH_PHASE_5 = 4, // 4. portal
    TVH_PHASE_6 = 5, // 5. portal
    TVH_PHASE_DONE = 6, // 6. portal

    TVH_END = 7
};

///////////////////////////////////////////////////////
//TheVioletHold Instance
class TheVioletHoldScript : public MoonInstanceScript
{
    friend class SinclariGossip; // Friendship forever ;-)

    private:

        uint32 m_phaseData[TVH_END];
        uint32 m_lastState = State_InvalidState;

        // NotStarted
        int32 S0_SpawnIntroMobsTimer = 0;   // Spawn mobs every 15s

        // PreProgress
        int32 S1_GuardFleeTimer = -1;       // Delay guards fleeing room for 2.5s (arbitrary)

    public:

        MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(TheVioletHoldScript, MoonInstanceScript);
        TheVioletHoldScript(MapMgr* pMapMgr) : MoonInstanceScript(pMapMgr)
        {
            for (uint8 i = 0; i < TVH_END; ++i)
                m_phaseData[i] = State_NotStarted;

            this->RegisterScriptUpdateEvent();
        }

        void UpdateEvent()
        {
            // Call original update function to elapse timers
            MoonInstanceScript::UpdateEvent();

            auto state = GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD);

            if (state != m_lastState)
            {
                OnStateChange(m_lastState, state);
                m_lastState = state;
            }

            switch (state)
            {
                case State_NotStarted:
                    S0_ReviveGuards();
                    S0_SpawnIntroMobs();
                    S0_RemoveDeadIntroMobs();
                    break;
                case State_InProgress:
                    S2_SpawnPortals();
                    break;
                case State_Finished: printf("State: %s\n", "State_Finished"); break;
                case State_Performed: printf("State: %s\n", "State_Performed"); break;
                case State_PreProgress:
                    S1_ActivateCrystalFleeRoom();
                    break;
            }
        }

        void S0_ReviveGuards()
        {
            auto guards = this->FindCreaturesOnMap(CN_VIOLET_HOLD_GUARD);
            for (auto guard : guards)
            {
                if (guard == nullptr || guard->isAlive())
                    continue;

                guard->Despawn(VH_TIMER_GUARD_DESPAWN_TIME, VH_TIMER_GUARD_RESPAWN_TIME);
            }
        }

        void S0_RemoveDeadIntroMobs()
        {
            auto introMobs = this->FindCreaturesOnMap(std::vector<uint32> { CN_INTRO_AZURE_BINDER_ARCANE, CN_INTRO_AZURE_INVADER_ARMS, CN_INTRO_AZURE_MAGE_SLAYER_MELEE, CN_INTRO_AZURE_SPELLBREAKER_ARCANE });
            for (auto mob : introMobs)
            {
                if (mob == nullptr || mob->isAlive())
                    continue;

                mob->Despawn(1500, 0);
            }
        }


        void S0_SpawnIntroMobs()
        {
            if (IsTimerFinished(S0_SpawnIntroMobsTimer))
            {
                S0_SpawnIntroMobsTimer = 0; // This forces a new timer to be started below
            
                SpawnCreature(GetRandomIntroMob(), IntroPortals[0]);
                SpawnCreature(GetRandomIntroMob(), IntroPortals[1]);
                SpawnCreature(GetRandomIntroMob(), IntroPortals[2]);
            }

            // Start another 15s timer
            if (GetTimer(S0_SpawnIntroMobsTimer) <= 0)
            {
                S0_SpawnIntroMobsTimer = AddTimer(VH_TIMER_SPAWN_INTRO_MOB);
            }
        }

        void S1_ActivateCrystalFleeRoom()
        {
            // TODO: activate crystal

            if (S1_GuardFleeTimer == -1)
            {
                S1_GuardFleeTimer = AddTimer(VH_TIMER_GUARD_FLEE_DELAY); // arbitrary time

            }

            if (GetTimer(S1_GuardFleeTimer) > 0)
            {
                return; // Wait for timer to finish
            }

            auto npcs = this->FindCreaturesOnMap(CN_VIOLET_HOLD_GUARD);
            for (auto guard : npcs)
            {
                if (!guard->IsInWorld())
                    continue;

                guard->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            }
        }

        void S2_SpawnPortals()
        {

        // TODO: Spawn any portals that are missing
        }

        void S2_SpawnMobs()
        {
            // TODO: Spawn any mobs that are missing
            // TODO: Move this logic to the portals
        }

        void S2_UpdateDoorDamage()
        {
            // TODO: Update damage done to the door
            // TODO: Move this logic to the mobs
        }

        void S3_UnlockDoorAndMoveNPCs()
        {
            // TODO: Open door
            // TODO: Move NPCs into room
        }

        void S3_SpawnGuards()
        {
            // TODO: Respawn guards that are missing
        }

        void ActivateCrystal()
        {
            // get all mobs
            // play GO anim
            // kill them after delay
            // TODO: Move this logic to the gameobjects
        }

        void DespawnIntroPortals()
        {
            auto portals = this->FindCreaturesOnMap(CN_PORTAL_INTRO);
            for (auto portal : portals)
            {
                portal->Despawn(VH_TIMER_INTRO_PORTAL_DESPAWN_TIME, 0);
            }
        }

        int GetRandomIntroMob()
        {
            auto rnd = RandomFloat(100.0f);
            if (rnd < 25.0f)
                return CN_INTRO_AZURE_BINDER_ARCANE;
            if (rnd < 50.f)
                return CN_INTRO_AZURE_INVADER_ARMS;
            if (rnd < 75.0f)
                return CN_INTRO_AZURE_MAGE_SLAYER_MELEE;

            return CN_INTRO_AZURE_SPELLBREAKER_ARCANE;
        }

        void OnStateChange(uint32 pLastState, uint32 pNewState)
        {
            switch (pNewState)
            {
                case State_PreProgress:
                    DespawnIntroPortals();
                    break;
            }
        }

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

        /*
        void SetData(uint32 pIndex, uint32 pData)
        {
            if (pIndex >= TVH_END)
                return;

            // If Data = MainEvent, set state "PreProgress". Gossip Sinclar 1 + 2
            if (pIndex == TVH_PHASE_1)
                mInstance->GetWorldStatesHandler().SetWorldStateForZone(0, AREA_VIOLET_HOLD, WORLDSTATE_VH, State_PreProgress);

            // If Data = second event, set state "InProgress". Gossip Sinclari Case 3
            if (pIndex == TVH_PHASE_2)
                mInstance->GetWorldStatesHandler().SetWorldStateForZone(0, AREA_VIOLET_HOLD, WORLDSTATE_VH, State_InProgress);

            m_phaseData[pIndex] = pData;
        }

        uint32 GetData(uint32 pIndex)
        {
            // If Phase = End/finishes, reset the Phases to 0
            if (pIndex >= TVH_END)
                return 0;

            return m_phaseData[pIndex];
        }
        */

        void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
        {

        }

        void OnPlayerEnter(Player* pPlayer)
        {
            TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();
            if (!pInstance)
                return;

            if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_NotStarted)
            {
                mEncounters.insert(EncounterMap::value_type(MAP_VIOLET_HOLD, State_NotStarted));
            }

        }
};

#define SINCLARI_SAY_1 "Prison guards, we are leaving! These adventurers are taking over! Go go go!"
#define SINCLARY_SAY_2 "I'm locking the door. Good luck, and thank you for doing this."

///////////////////////////////////////////////////////
//Lieutnant Sinclari StartEvent
class SinclariAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(SinclariAI, MoonScriptCreatureAI);
        SinclariAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _unit->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 2:
                {
                    this->OnRescuePrisonGuards();
                }break;
                case 4:
                {
                    _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARY_SAY_2);
                    _unit->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }break;
                case 5:
                {
                    TheVioletHoldScript* pInstance = (TheVioletHoldScript*)_unit->GetMapMgr()->GetScript();
                    pInstance->SetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD, State_InProgress);
                    GameObject* pVioletHoldDoor = pInstance->FindClosestGameObjectOnMap(GO_PRISON_SEAL, 1822.59f, 803.93f, 44.36f);
                    if (pVioletHoldDoor != NULL)
                        pVioletHoldDoor->SetState(GO_STATE_CLOSED);
                }break;
            }
        }

        void OnRescuePrisonGuards()
        {
            _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, SINCLARI_SAY_1);
            _unit->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);

        }
};


///////////////////////////////////////////////////////
//Lieutnant Sinclari Gossip and init events
//Sinclari Gossip
class SinclariGossip : public GossipScript
{
    public:

        void GossipHello(Object* pObject, Player* pPlayer)
        {
            TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();
            if (!pInstance)
                return;

            GossipMenu* menu;

            //Page 1: Textid and first menu item
            if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_NotStarted)
            {
                objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_ON_HELLO, pPlayer);
                menu->AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_ACTIVATE), 1);

                menu->SendTo(pPlayer);
            }

            //If VioletHold is started, Sinclari has this item for people who aould join.
            if (pInstance->GetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD) == State_InProgress)
            {
                objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_OUTSIDE, pPlayer);
                menu->AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_SEND_ME_IN), 3);

                menu->SendTo(pPlayer);
            }
        }

        void GossipSelectOption(Object* pObject, Player* pPlayer, uint32 Id, uint32 IntId, const char* Code)
        {
            TheVioletHoldScript* pInstance = (TheVioletHoldScript*)pPlayer->GetMapMgr()->GetScript();

            if (!pInstance)
                return;

            if (!pObject->IsCreature())
                return;

            Creature* pCreature = static_cast<Creature*>(pObject);

            switch (IntId)
            {
                case 0:
                    GossipHello(pObject, pPlayer);
                    break;

                case 1:
                {
                    GossipMenu* menu;
                    objmgr.CreateGossipMenuForPlayer(&menu, pObject->GetGUID(), SINCLARI_ON_FINISH, pPlayer);
                    menu->AddItem(GOSSIP_ICON_CHAT, pPlayer->GetSession()->LocalizedGossipOption(SINCLARI_GET_SAFETY), 2);
                    menu->SendTo(pPlayer);

                }break;

                case 2:
                {
                    static_cast<Creature*>(pObject)->SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
                    pCreature->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);
                    //pCreature->MoveToWaypoint(1);
                    pCreature->GetAIInterface()->StopMovement(10);

                    // New Encounter State included
                    pInstance->SetInstanceData(Data_EncounterState, MAP_VIOLET_HOLD, State_PreProgress);

                }break;

                case 3:
                {
                    Arcemu::Gossip::Menu::Complete(pPlayer);
                    pPlayer->SafeTeleport(pPlayer->GetInstanceID(), MAP_VIOLET_HOLD, 1830.531006f, 803.939758f, 44.340508f, 6.281611f);
                }break;
            }
        }
};

///////////////////////////////////////////////////////
//VH Guards
class VHGuardsAI : public MoonScriptCreatureAI
{
    public:

        MOONSCRIPT_FACTORY_FUNCTION(VHGuardsAI, MoonScriptCreatureAI);
        VHGuardsAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            _unit->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
        }

        //WPs inserted in db.

};

//\TODO: Replace spell casting logic for all instances, this is temp
class VHCreatureAI : public MoonScriptCreatureAI
{
    protected:

        bool m_isIntroMob = false;
        int m_spellCount = 0;
    
        /* Warning
         * Using vectors here is theoretically dangerous as they don't guarantee order
         *   when elements are erased or moved, however it doesn't matter here as we
         *   aren't modifying the number of elements in each vector
         * 
         * TODO: Write a proper spell manager to handle this stuff */
        std::vector<bool> m_spellsEnabled;
        std::vector<SP_AI_Spell> m_spells;

    public:

        MOONSCRIPT_FACTORY_FUNCTION(VHCreatureAI, MoonScriptCreatureAI);
        VHCreatureAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
        {
            //this->CreateWaypoint(1, 0, 0, VH_DOOR_ATTACK_POSITION);
            //_unit->GetAIInterface()->setMoveType(MOVEMENTTYPE_FORWARDTHENSTOP);
            //this->SetWaypointToMove(1);
            //this->MoveTo(VH_DOOR_ATTACK_POSITION.x, VH_DOOR_ATTACK_POSITION.y, VH_DOOR_ATTACK_POSITION.z, true);
            //_unit->GetAIInterface()->UpdateMove();
            for (int i = 1; i < 3; ++i)
            {
                AddWaypoint(CreateWaypoint(i, 0, AttackerWP[i]));
            }
            _unit->GetAIInterface()->SetWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP);
            _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "I am alive!");
            _unit->GetAIInterface()->UpdateMove();
        }

        void OnReachWP(uint32 iWaypointId, bool bForwards)
        {
            switch (iWaypointId)
            {
                case 1:
                    _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Reached wp 1!");
                    SetWaypointToMove(2);
                    break;
                case 2:
                {
                    if (m_isIntroMob)
                    {
                        _unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, LANG_UNIVERSAL, "Reached wp 2!");
                        _unit->Despawn(500, 0);
                    }
                    else
                    {
                        // TODO: Door attack code
                    }
                }break;
            }
        }


        void OnCombatStart(Unit* mTarget)
        {
            PutAllSpellsOnCooldown();
            RegisterAIUpdateEvent(_unit->GetBaseAttackTime(MELEE));
        }

        void PutAllSpellsOnCooldown()
        {
            for (int i = 0; i < m_spellCount; i++)
                m_spells[i].casttime = m_spells[i].cooldown;
        }

        void OnTargetDied(Unit* mTarget)
        {
        }

        void OnCombatStop(Unit* mTarget)
        {
            PutAllSpellsOnCooldown();
            _unit->GetAIInterface()->setCurrentAgent(AGENT_NULL);
            _unit->GetAIInterface()->SetAIState(STATE_IDLE);
            RemoveAIUpdateEvent();
        }

        void OnDied(Unit* mKiller)
        {
            PutAllSpellsOnCooldown();
            RemoveAIUpdateEvent();
        }

        void AIUpdate()
        {
            auto randomValue = RandomFloat(100.0f);
            SpellCast(randomValue);
        }
    
        void SpellCast(float randomValue)
        {
            if (_unit->GetCurrentSpell() == NULL && _unit->GetAIInterface()->getNextTarget())
            {
                float comulativeperc = 0;
                Unit* target = NULL;
                for (int i = 0; i < m_spellCount; i++)
                {
                    m_spells[i].casttime--;

                    if (m_spellsEnabled[i])
                    {
                        if (!m_spells[i].instant)
                        {
                            this->StopMovement();
                        }

                        m_spells[i].casttime = m_spells[i].cooldown;
                        target = _unit->GetAIInterface()->getNextTarget();
                        switch (m_spells[i].targettype)
                        {
                        case TARGET_SELF:
                        case TARGET_VARIOUS:
                            _unit->CastSpell(_unit, m_spells[i].info, m_spells[i].instant);
                            break;
                        case TARGET_ATTACKING:
                            _unit->CastSpell(target, m_spells[i].info, m_spells[i].instant);
                            break;
                        case TARGET_DESTINATION:
                            _unit->CastSpellAoF(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), m_spells[i].info, m_spells[i].instant);
                            break;
                        }

                        if (m_spells[i].speech != "")
                        {
                            _unit->SendChatMessage(CHAT_MSG_MONSTER_YELL, LANG_UNIVERSAL, m_spells[i].speech.c_str());
                            _unit->PlaySoundToSet(m_spells[i].soundid);
                        }

                        m_spellsEnabled[i] = false;
                        return;
                    }

                    if ((randomValue > comulativeperc && randomValue <= (comulativeperc + m_spells[i].perctrigger)) || !m_spells[i].casttime)
                    {
                        _unit->setAttackTimer(m_spells[i].attackstoptimer, false);
                        m_spellsEnabled[i] = true;
                    }
                    comulativeperc += m_spells[i].perctrigger;
                }
            }
        }
};

class VHIntroAzureBinder : VHCreatureAI
{
    private:

        const int SPELL_ARCANE_BARRAGE = 58456;
        const int SPELL_ARCANE_EXPLOSION = 58455;
    public:

        ADD_CREATURE_FACTORY_FUNCTION(VHIntroAzureBinder);
        VHIntroAzureBinder(Creature* pCreature) : VHCreatureAI(pCreature)
        {
            m_isIntroMob = true;
            m_spellCount = 2;
            for (int i = 0; i < m_spellCount; i++)
            {
                m_spellsEnabled.push_back(false);
            }

            auto spellArcaneBarrage = SP_AI_Spell();
            spellArcaneBarrage.info = sSpellCustomizations.GetSpellInfo(SPELL_ARCANE_BARRAGE);
            spellArcaneBarrage.cooldown = 6;
            spellArcaneBarrage.targettype = TARGET_ATTACKING;
            spellArcaneBarrage.instant = true;
            spellArcaneBarrage.perctrigger = 50.0f;
            spellArcaneBarrage.attackstoptimer = 1000;
            m_spells.push_back(spellArcaneBarrage);
            m_spellsEnabled[0] = true;

            auto spellArcaneExplosion = SP_AI_Spell();
            spellArcaneExplosion.info = sSpellCustomizations.GetSpellInfo(SPELL_ARCANE_EXPLOSION);
            spellArcaneExplosion.cooldown = 4;
            spellArcaneExplosion.targettype = TARGET_VARIOUS;
            spellArcaneExplosion.instant = true;
            spellArcaneExplosion.perctrigger = 50.0f;
            spellArcaneExplosion.attackstoptimer = 1000;
            m_spells.push_back(spellArcaneExplosion);
            m_spellsEnabled[1] = true;
        }
};

class VHIntroAzureInvader : VHCreatureAI
{
    private:

        const int SPELL_CLEAVE = 15496;
        const int SPELL_IMPALE = 58459;

    public:

        ADD_CREATURE_FACTORY_FUNCTION(VHIntroAzureInvader);
        VHIntroAzureInvader(Creature* pCreature) : VHCreatureAI(pCreature)
        {
            m_isIntroMob = true;
            m_spellCount = 2;
            for (int i = 0; i < m_spellCount; i++)
            {
                m_spellsEnabled.push_back(false);
            }

            auto spellCleave = SP_AI_Spell();
            spellCleave.info = sSpellCustomizations.GetSpellInfo(SPELL_CLEAVE);
            spellCleave.cooldown = 6;
            spellCleave.targettype = TARGET_ATTACKING;
            spellCleave.instant = true;
            spellCleave.perctrigger = 50.0f;
            spellCleave.attackstoptimer = 1000;
            m_spells.push_back(spellCleave);
            m_spellsEnabled[0] = true;

            auto spellImpale = SP_AI_Spell();
            spellImpale.info = sSpellCustomizations.GetSpellInfo(SPELL_IMPALE);
            spellImpale.cooldown = 8;
            spellImpale.targettype = TARGET_ATTACKING;
            spellImpale.instant = true;
            spellImpale.perctrigger = 20.0f;
            spellImpale.attackstoptimer = 1000;
            m_spells.push_back(spellImpale);
            m_spellsEnabled[1] = true;
        }
};

class VHIntroAzureMageSlayer : VHCreatureAI
{
    private:

        const int SPELL_ARCANE_EMPOWERMENT = 58469;

    public:

        ADD_CREATURE_FACTORY_FUNCTION(VHIntroAzureMageSlayer);
        VHIntroAzureMageSlayer(Creature* pCreature) : VHCreatureAI(pCreature)
        {
            m_isIntroMob = true;
            m_spellCount = 1;
            for (int i = 0; i < m_spellCount; i++)
            {
                m_spellsEnabled.push_back(false);
            }

            auto spellArcaneEmpowerment = SP_AI_Spell();
            spellArcaneEmpowerment.info = sSpellCustomizations.GetSpellInfo(SPELL_ARCANE_EMPOWERMENT);
            spellArcaneEmpowerment.cooldown = 8;
            spellArcaneEmpowerment.targettype = TARGET_SELF;
            spellArcaneEmpowerment.instant = true;
            spellArcaneEmpowerment.perctrigger = 50.0f;
            spellArcaneEmpowerment.attackstoptimer = 1000;
            m_spells.push_back(spellArcaneEmpowerment);
            m_spellsEnabled[0] = true;
        }
};

class VHIntroAzureSpellBreaker : VHCreatureAI
{
    private:

        const int SPELL_ARCANE_BLAST = 58462;
        const int SPELL_SLOW = 25603;

    public:

        ADD_CREATURE_FACTORY_FUNCTION(VHIntroAzureSpellBreaker);
        VHIntroAzureSpellBreaker(Creature* pCreature) : VHCreatureAI(pCreature)
        {
            m_isIntroMob = true;
            m_spellCount = 2;
            for (int i = 0; i < m_spellCount; i++)
            {
                m_spellsEnabled.push_back(false);
            }

            auto spellArcaneBlast = SP_AI_Spell();
            spellArcaneBlast.info = sSpellCustomizations.GetSpellInfo(SPELL_ARCANE_BLAST);
            spellArcaneBlast.cooldown = 3;
            spellArcaneBlast.targettype = TARGET_ATTACKING;
            spellArcaneBlast.instant = false;
            spellArcaneBlast.casttime = 2500;
            spellArcaneBlast.perctrigger = 60.0f;
            spellArcaneBlast.attackstoptimer = 1000;
            m_spells.push_back(spellArcaneBlast);
            m_spellsEnabled[0] = true;

            auto spellSlow = SP_AI_Spell();
            spellSlow.info = sSpellCustomizations.GetSpellInfo(SPELL_SLOW);
            spellSlow.cooldown = 7;
            spellSlow.targettype = TARGET_ATTACKING;
            spellSlow.instant = true;
            spellSlow.perctrigger = 40.0f;
            spellSlow.attackstoptimer = 1000;
            m_spells.push_back(spellSlow);
            m_spellsEnabled[0] = true;        
        }
};

///////////////////////////////////////////////////////
//Boss: Erekem
//class ErekemAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Moragg
//class MoraggAI : public CreatureAIScript

class MoraggAI : public MoonScriptBossAI
{
    MOONSCRIPT_FACTORY_FUNCTION(MoraggAI, MoonScriptBossAI);
    MoraggAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        // Spells
        if (IsHeroic())
        {
            AddSpell(MORAGG_SPELL_RAY_OF_SUFFERING_H, Target_Current, 100, 0, 0, 0, 45);
            AddSpell(MORAGG_SPELL_RAY_OF_PAIN_H, Target_Current, 100, 0, 0, 0 , 45);
        }
        else
        {
            AddSpell(MORAGG_SPELL_RAY_OF_SUFFERING, Target_Self, 100, 0, 0, 0, 45);
            AddSpell(MORAGG_SPELL_RAY_OF_PAIN, Target_Current, 100, 0, 0, 0, 45);
        }

        AddSpell(MORAGG_SPELL_CORROSIVE_SALIVA, Target_Current, 100, 0, 10, 0 , 5);
        AddSpell(MORAGG_SPELL_OPTIC_LINK, Target_Current, 100, 0, 15, 0, 50);

    }
};

///////////////////////////////////////////////////////
//Boss: Ichoron
//class IchoronAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Xevozz
//class XevozzAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Lavanthos
//class LavanthosAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Boss: Zuramat the Obliterator
//class ZuramatTheObliteratorAI : public CreatureAIScript

///////////////////////////////////////////////////////
//Final Boss: Cyanigosa
//class CyanigosaAI : public CreatureAIScript


void SetupTheVioletHold(ScriptMgr* mgr)
{
    //Instance
    mgr->register_instance_script(MAP_VIOLET_HOLD, &TheVioletHoldScript::Create);

    //Sinclari and Guards
    mgr->register_creature_script(CN_LIEUTNANT_SINCLARI, &SinclariAI::Create);
    mgr->register_creature_script(CN_VIOLET_HOLD_GUARD, &VHGuardsAI::Create);

    // Intro trash
    mgr->register_creature_script(CN_INTRO_AZURE_BINDER_ARCANE, &VHIntroAzureBinder::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_INVADER_ARMS, &VHIntroAzureInvader::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_MAGE_SLAYER_MELEE, &VHIntroAzureMageSlayer::Create);
    mgr->register_creature_script(CN_INTRO_AZURE_SPELLBREAKER_ARCANE, &VHIntroAzureSpellBreaker::Create);

    //Bosses
    //mgr->register_creature_script(CN_EREKEM, &ErekemAI::Create);
    mgr->register_creature_script(CN_MORAGG, &MoraggAI::Create);
    //mgr->register_creature_script(CN_ICHORON, &IchoronAI::Create);
    //mgr->register_creature_script(CN_XEVOZZ, &XevozzAI::Create);
    //mgr->register_creature_script(CN_LAVANTHOR, &LavanthorAI::Create);
    //mgr->register_creature_script(CN_TURAMAT_THE_OBLITERATOR, &ZuramatTheObliteratorAI::Create);
    //mgr->register_creature_script(CN_CYANIGOSA, &CyanigosaAI::Create);

    GossipScript* GSinclari = new SinclariGossip;
    mgr->register_gossip_script(CN_LIEUTNANT_SINCLARI, GSinclari);


}