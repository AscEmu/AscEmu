/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_ShadowfangKeep.h"

// Wotlk version
#if VERSION_STRING != Cata
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/PowerType.h"

 // Instance script for map 33 (Shadowfang Keep)
class ShadowfangKeepInstance : public InstanceScript
{
        // Gameobjects low guids
        uint32 go_leftCell_GUID;
        uint32 go_middleCell_GUID;
        uint32 go_rightCell_GUID;
        uint32 go_arugalsLair_GUID;
        uint32 go_sorcererGate_GUID;
        uint32 go_leftCellLever_GUID;
        uint32 go_middleCellLever_GUID;
        uint32 go_rightCellLever_GUID;
        uint32 go_courtyarDoor_GUID;

        // Creatures low guids
        uint32 npc_ashcrombe_GUID;
        uint32 npc_adamant_GUID;

        // Nandos event related
        std::list<uint32 /*guid*/> nandos_summons;

        // Encounters data
        uint32 m_encounterData[INDEX_MAX];

    public:

        ShadowfangKeepInstance(MapMgr* pMapMgr) : InstanceScript(pMapMgr),

            // Gameobjects low guids
            go_leftCell_GUID(0),
            go_middleCell_GUID(0),
            go_rightCell_GUID(0),
            go_arugalsLair_GUID(0),
            go_sorcererGate_GUID(0),
            go_leftCellLever_GUID(0),
            go_middleCellLever_GUID(0),
            go_rightCellLever_GUID(0),
            go_courtyarDoor_GUID(0),

            // Creatures low guids
            npc_ashcrombe_GUID(0),
            npc_adamant_GUID(0)
        {
            // NandosAI event related
            nandos_summons.clear();

            // Set encounters data to State_NotStarted
            memset(m_encounterData, NotStarted, sizeof(m_encounterData));
        }

        static InstanceScript* Create(MapMgr* pMapMgr) { return new ShadowfangKeepInstance(pMapMgr); }

        void SetLocaleInstanceData(uint32 /*pType*/, uint32 pIndex, uint32 pData)
        {
            if (pIndex >= INDEX_MAX)
                return;

            switch (pIndex)
            {
                case INDEX_VOIDWALKER:
                {
                    if (pData == InProgress)
                    {
                        if (Creature* ArugalSpawn = spawnCreature(CN_ARUGAL_BOSS, ArugalAtFenrusLoc.x, ArugalAtFenrusLoc.y, ArugalAtFenrusLoc.z, ArugalAtFenrusLoc.o))
                        {
                            ArugalSpawn->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                            ArugalSpawn->GetAIInterface()->SetAllowedToEnterCombat(false);
                            ArugalSpawn->GetAIInterface()->m_canMove = false;
                            if (ArugalSpawn->GetScript())
                            {
                                ArugalSpawn->GetScript()->RegisterAIUpdateEvent(500);
                            }
                        }
                    }
                }break;
                case INDEX_NANDOS:
                {
                    // Despawn all summons on fail or on boos death
                    if (pData == InvalidState || pData == Finished)
                    {
                        for (std::list<uint32>::iterator itr = nandos_summons.begin(); itr != nandos_summons.end();)
                        {
                            if (Creature* pCreature = GetInstance()->GetCreature(*itr))
                            {
                                pCreature->Despawn(1000, 0);
                            }
                            itr = nandos_summons.erase(itr);
                        }
                        // Despawn creatures
                    }

                    if (pData == Finished)
                    {
                        GameObject* pGate = GetGameObjectByGuid(go_arugalsLair_GUID);
                        if (pGate != nullptr && pGate->GetState() == GO_STATE_CLOSED)
                        {
                            pGate->SetState(GO_STATE_OPEN);
                        }
                    }
                }break;
                case INDEX_RETHILGORE:
                {
                    // Add gossip flag to prisoners
                    if (pData == Finished)
                    {
                        // Set gossip flags for both prisoners and push texts
                        if (Creature* pCreature = GetInstance()->GetCreature(npc_adamant_GUID))
                        {
                            if (pCreature->isAlive())
                            {
                                pCreature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                                pCreature->SendScriptTextChatMessage(SAY_ADAMANT_BOSS_DEATH);
                            }
                        }

                        if (Creature* pCreature = GetInstance()->GetCreature(npc_ashcrombe_GUID))
                        {
                            if (pCreature->isAlive())
                            {
                                pCreature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                                if (CreatureAIScript* pScript = pCreature->GetScript())
                                {
                                    // Say argument after 4 seconds
                                    pScript->RegisterAIUpdateEvent(4000);
                                }
                            }
                        }

                        // Make levers targetable
                        if (GameObject* pGO = GetGameObjectByGuid(go_leftCellLever_GUID))
                        {
                            pGO->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                        }

                        if (GameObject* pGO = GetGameObjectByGuid(go_middleCellLever_GUID))
                        {
                            pGO->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                        }

                        if (GameObject* pGO = GetGameObjectByGuid(go_rightCellLever_GUID))
                        {
                            pGO->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                        }
                    }
                }break;
                case INDEX_PRISONER_EVENT:
                {
                    // Open doors in any case
                    if (pData == Finished || pData == Performed)
                    {
                        if (GameObject* pGO = GetGameObjectByGuid(go_courtyarDoor_GUID))
                        {
                            if (pGO->GetState() != GO_STATE_OPEN)
                                pGO->SetState(GO_STATE_OPEN);
                        }
                    }
                }break;
                case INDEX_FENRUS:
                {
                    if (pData == Finished)
                    {
                        SetLocaleInstanceData(0, INDEX_VOIDWALKER, InProgress);
                        GameObject* pGate = GetGameObjectByGuid(go_sorcererGate_GUID);
                        if (pGate != nullptr && pGate->GetState() == GO_STATE_CLOSED)
                        {
                            pGate->SetState(GO_STATE_OPEN);
                        }
                    }
                }break;
                default:
                    break;
            }

            m_encounterData[pIndex] = pData;
        }

        uint32 GetInstanceData(uint32 /*pType*/, uint32 pIndex)
        {
            return pIndex >= INDEX_MAX ? 0 : m_encounterData[pIndex];
        }

        // Objects handling
        void OnGameObjectPushToWorld(GameObject* pGameObject) override
        {
            switch (pGameObject->GetEntry())
            {
                case GO_LEFT_CELL:
                {
                    go_leftCell_GUID = pGameObject->GetLowGUID();
                }break;
                case GO_MIDDLE_CELL:
                {
                    go_middleCell_GUID = pGameObject->GetLowGUID();
                }break;
                case GO_RIGHT_CELL:
                {
                    go_rightCell_GUID = pGameObject->GetLowGUID();
                }break;
                case GO_ARUGALS_LAIR_GATE:
                {
                    go_arugalsLair_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_NANDOS) == Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                    {
                        pGameObject->SetState(GO_STATE_OPEN);
                    }
                }break;
                case GO_SORCERER_GATE:
                {
                    go_sorcererGate_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_FENRUS) == Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                    {
                        pGameObject->SetState(GO_STATE_OPEN);
                    }
                }break;
                case GO_LEFT_LEVER:
                {
                    go_leftCellLever_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_RETHILGORE) != Finished)
                    {
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                case GO_RIGHT_LEVER:
                {
                    go_rightCellLever_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_RETHILGORE) != Finished)
                    {
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                case GO_MIDDLE_LEVER:
                {
                    go_middleCellLever_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_RETHILGORE) != Finished)
                    {
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                case GO_COURTYARD_DOOR:
                {
                    go_courtyarDoor_GUID = pGameObject->GetLowGUID();
                    if (GetInstanceData(0, INDEX_PRISONER_EVENT) == Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                    {
                        pGameObject->SetState(GO_STATE_OPEN);
                    }
                }break;
                default:
                    break;
            }
        }

        void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
        {
            switch (pGameObject->GetEntry())
            {
                case GO_RIGHT_LEVER:
                {
                    if (GameObject* pGO = GetGameObjectByGuid(go_rightCell_GUID))
                    {
                        pGO->SetState(pGO->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                case GO_MIDDLE_LEVER:
                {
                    if (GameObject* pGO = GetGameObjectByGuid(go_middleCell_GUID))
                    {
                        pGO->SetState(pGO->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                case GO_LEFT_LEVER:
                {
                    if (GameObject* pGO = GetGameObjectByGuid(go_leftCell_GUID))
                    {
                        pGO->SetState(pGO->GetState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                        pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                    }
                }break;
                default:
                    break;
            }
        }

        void OnCreatureDeath(Creature* pCreature, Unit* /*pKiller*/) override
        {
            switch (pCreature->GetEntry())
            {
                case CN_NANDOS:
                {
                    SetLocaleInstanceData(0, INDEX_NANDOS, Finished);
                }break;
                case CN_RETHILGORE:
                {
                    SetLocaleInstanceData(0, INDEX_RETHILGORE, Finished);
                }break;
                case CN_FENRUS:
                {
                    SetLocaleInstanceData(0, INDEX_FENRUS, Finished);
                }break;
                default:
                    break;
            }
        }

        void OnCreaturePushToWorld(Creature* pCreature) override
        {
            switch (pCreature->GetEntry())
            {
                case CN_ADAMANT:
                {
                    npc_adamant_GUID = GET_LOWGUID_PART(pCreature->GetGUID());
                }break;
                case CN_ASHCROMBE:
                {
                    npc_ashcrombe_GUID = GET_LOWGUID_PART(pCreature->GetGUID());
                }break;
                // Make him hidden
                case CN_ARUGAL:
                {
                    pCreature->SetInvisFlag(INVIS_FLAG_TOTAL);
                }break;
                case CN_BLEAK_WORG:
                case CN_SLAVERING_WORG:
                case CN_LUPINE_HORROR:
                {
                    // Add to nandos summon lists only on his event is started
                    if (GetInstanceData(0, INDEX_NANDOS) == InProgress)
                    {
                        pCreature->Despawn(60 * 4 * 1000, 0);   // Despawn in 4 mins
                        nandos_summons.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    }
                }break;
                case CN_LUPINE_DELUSION:
                {
                    // Add to nandos summon lists only on his event is started
                    if (GetInstanceData(0, INDEX_NANDOS) == InProgress)
                    {
                        nandos_summons.push_back(GET_LOWGUID_PART(pCreature->GetGUID()));
                    }
                    pCreature->Despawn(60 * 4 * 1000, 0);   // Despawn in 4 mins
                }break;
                case CN_DEATHSTALKER_VINCENT:
                {
                    if (GetInstanceData(0, INDEX_ARUGAL_INTRO) == Finished)
                    {
                        // Make him look like dead
                        pCreature->SetStandState(STANDSTATE_DEAD);
                        pCreature->setDeathState(CORPSE);
                        pCreature->GetAIInterface()->m_canMove = false;
                        pCreature->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);
                        pCreature->SendScriptTextChatMessage(SAY_VINCENT_DEATH);
                    }
                }break;
                default:
                    break;
            }
        }
};

/////////////////////////////////////////////////////
// Special npcs events

// Arugal intro event
// Creature entries: 10000, 4444

class ArugalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ArugalAI);
        ArugalAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0)
        {
            SFK_Instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());
            if (SFK_Instance && SFK_Instance->GetInstanceData(0, INDEX_ARUGAL_INTRO) == NotStarted)
            {
                RegisterAIUpdateEvent(500);
                SFK_Instance->SetLocaleInstanceData(0, INDEX_ARUGAL_INTRO, InProgress);
            }
        }

        void AIUpdate() override
        {
            if (SFK_Instance && SFK_Instance->GetInstanceData(0, INDEX_ARUGAL_INTRO) == InProgress)
            {
                switch (stage)
                {
                    case 0:
                    {
                        getCreature()->SetInvisFlag(INVIS_FLAG_NORMAL);
                        getCreature()->CastSpell(getCreature(), SPELL_ARUGAL_SPAWN, true);
                        ModifyAIUpdateEvent(5500);  // call every step after 5.5 seconds
                        if (Creature* pVincent = getNearestCreature(CN_DEATHSTALKER_VINCENT))
                        {
                            pVincent->GetAIInterface()->AttackReaction(getCreature(), 1);
                            pVincent->GetAIInterface()->setMeleeDisabled(true);
                        }
                    }break;
                    case 1:
                    {
                        getCreature()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO1);
                    }break;
                    case 2:
                    {
                        getCreature()->Emote(EMOTE_ONESHOT_POINT);
                    }break;
                    case 3:
                    {
                        getCreature()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO2);
                    }break;
                    case 4:
                    {
                        getCreature()->Emote(EMOTE_ONESHOT_EXCLAMATION);
                    }break;
                    case 5:
                    {
                        getCreature()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO3);
                    }break;
                    case 6:
                    {
                        getCreature()->Emote(EMOTE_ONESHOT_LAUGH);
                    }break;
                    case 7:
                    {
                        if (Creature* pVincent = getCreature()->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), CN_DEATHSTALKER_VINCENT))
                        {
                            // Make him look like dead
                            pVincent->SendScriptTextChatMessage(SAY_VINCENT_DEATH);
                            pVincent->SetStandState(STANDSTATE_DEAD);
                            pVincent->setDeathState(CORPSE);
                            pVincent->GetAIInterface()->m_canMove = false;
                            pVincent->SetFlag(UNIT_DYNAMIC_FLAGS, U_DYN_FLAG_DEAD);
                        }
                    }break;
                    case 8:
                    {
                        getCreature()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO4);
                    }break;
                    case 9:
                    {
                        getCreature()->CastSpell(getCreature(), SPELL_ARUGAL_SPAWN, true);
                        SFK_Instance->SetLocaleInstanceData(0, INDEX_ARUGAL_INTRO, Finished);
                        getCreature()->SetInvisFlag(INVIS_FLAG_TOTAL);
                        RemoveAIUpdateEvent();
                    }break;
                }
                ++stage;
            }
        }

    protected:

        uint32 stage;
        ShadowfangKeepInstance* SFK_Instance;
};

// Prisoner Adamant (entry: 3849) gossip, escort event

class AdamantAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(AdamantAI);
        AdamantAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0), eventStarted(false)
        {
            SFK_instance = static_cast<ShadowfangKeepInstance*>(getCreature()->GetMapMgr()->GetScript());

            for (uint8 i = 0; i < adamantWpCount; ++i)
            {
                float waitTime = 0;
                float distanceX = 0;
                float distanceY = 0;
                float distance = 0;
                float walkSpeed = getCreature()->GetCreatureProperties()->walk_speed;
                float runSpeed = getCreature()->GetCreatureProperties()->run_speed;
                // first waypoint
                if (i == 0)
                {
                    // (currentWP - perviousLocation) *(currentWP - perviousLocation)
                    distanceX = (DeathstalkerAdamantWPS[i].wp_location.x - getCreature()->GetPositionX()) * (DeathstalkerAdamantWPS[i].wp_location.x - getCreature()->GetPositionX());
                    distanceY = (DeathstalkerAdamantWPS[i].wp_location.y - getCreature()->GetPositionY()) * (DeathstalkerAdamantWPS[i].wp_location.y - getCreature()->GetPositionY());
                    distance = sqrt(distanceX - distanceY);
                }
                else if (i != adamantWpCount - 1)
                {
                    // (currentWP - perviousWP) *(currentWP - perviousWP)
                    distanceX = (DeathstalkerAdamantWPS[i].wp_location.x - DeathstalkerAdamantWPS[i - 1].wp_location.x) * (DeathstalkerAdamantWPS[i].wp_location.x - DeathstalkerAdamantWPS[i - 1].wp_location.x);
                    distanceY = (DeathstalkerAdamantWPS[i].wp_location.y - DeathstalkerAdamantWPS[i - 1].wp_location.y) * (DeathstalkerAdamantWPS[i].wp_location.y - DeathstalkerAdamantWPS[i - 1].wp_location.y);
                    distance = sqrt(distanceX + distanceY);
                }
                waitTime = 1000 * std::abs(DeathstalkerAdamantWPS[i].wp_flag == Movement::WP_MOVE_TYPE_WALK ? distance / walkSpeed : distance / runSpeed);

                AddWaypoint(CreateWaypoint(i + 1, static_cast<uint32>(waitTime), DeathstalkerAdamantWPS[i].wp_flag, DeathstalkerAdamantWPS[i].wp_location));
            }

            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

            // Remove Gossip
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            switch (iWaypointId)
            {
                case 11:
                {
                    RegisterAIUpdateEvent(2000);
                }break;
                case 30:
                {
                    getCreature()->Despawn(2000, 0);
                }break;
                default:
                {
                    SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                    SetWaypointToMove(++iWaypointId);
                }break;
            }
        }

        void AIUpdate() override
        {
            if (SFK_instance && (SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == InProgress 
                || SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == Performed))
            {
                if (eventStarted)
                {
                    switch (stage)
                    {
                        case 0:
                        {
                            getCreature()->SetEmoteState(EMOTE_STATE_NONE);
                            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            SetWaypointToMove(1);
                            RemoveAIUpdateEvent();
                        }break;
                        case 1:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ADAMANT_BEFORE_OPEN);
                        }break;
                        case 2:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ADAMANT_OPENING);
                            getCreature()->EventAddEmote(EMOTE_ONESHOT_USESTANDING, 8000);
                            ModifyAIUpdateEvent(8000);
                        }break;
                        case 3:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ADAMANT_AFTER_OPEN);
                            SFK_instance->SetLocaleInstanceData(0, INDEX_PRISONER_EVENT, Performed);
                            ModifyAIUpdateEvent(4000);
                        }break;
                        case 4:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ADAMANT_BYE);
                        }break;
                        case 5:
                        {
                            SFK_instance->SetLocaleInstanceData(0, INDEX_PRISONER_EVENT, Finished);
                            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            SetWaypointToMove(12);  // Lets run
                            RemoveAIUpdateEvent();
                        }break;
                        default:
                            break;
                    }
                    ++stage;
                }
            }
        }

    protected:

        uint32 stage;
        ShadowfangKeepInstance* SFK_instance;

    public:

        // This variable will be set to true on OnSelectOption gossip event
        bool eventStarted;
};

class AdamantGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            //TODO: correct text id
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry()));

            ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript());
            if (pInstance != nullptr && pInstance->GetInstanceData(0, INDEX_RETHILGORE) == Finished && pInstance->GetInstanceData(0, INDEX_PRISONER_EVENT) == NotStarted)
            {
                //TODO: move this to database
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(prisonerGossipOptionID), 1);
            }
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            if (Id == 1)
            {
                if (AdamantAI* pPrisoner = static_cast<AdamantAI*>(static_cast<Creature*>(pObject)->GetScript()))
                {
                    pPrisoner->getCreature()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    pPrisoner->getCreature()->SendScriptTextChatMessage(SAY_ADAMANT_FOLLOW);
                    pPrisoner->RegisterAIUpdateEvent(5000);
                    pPrisoner->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                    pPrisoner->getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                    pPrisoner->getCreature()->EventAddEmote(EMOTE_ONESHOT_CHEER, 4000);
                    pPrisoner->eventStarted = true;
                    if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript()))
                        pInstance->SetLocaleInstanceData(0, INDEX_PRISONER_EVENT, InProgress);
                }
            }
            Arcemu::Gossip::Menu::Complete(plr);
        }
};

// Prisoner Sorcerer Ashcrombe (entry: 3850) gossip, escort event
class AshcrombeAI : public CreatureAIScript
{
    public:

        ADD_CREATURE_FACTORY_FUNCTION(AshcrombeAI);
        AshcrombeAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0), argued(false), eventStarted(false)
        {
            SFK_instance = static_cast<ShadowfangKeepInstance*>(getCreature()->GetMapMgr()->GetScript());

            for (uint8 i = 0; i < ashcrombeWpCount; ++i)
            {
                float waitTime = 0;
                float distanceX = 0;
                float distanceY = 0;
                float distance = 0;
                float walkSpeed = getCreature()->GetCreatureProperties()->walk_speed;
                //float runSpeed = getCreature()->GetCreatureProperties()->run_speed;
                if (i == 0) // first waypoint
                {
                    distanceX = (SorcererAshcrombeWPS[i].x - getCreature()->GetPositionX())*(SorcererAshcrombeWPS[i].x - getCreature()->GetPositionX());
                    distanceY = (SorcererAshcrombeWPS[i].y - getCreature()->GetPositionY())*(SorcererAshcrombeWPS[i].y - getCreature()->GetPositionY());
                    distance = std::sqrt(distanceX - distanceY);
                }
                else if (i != ashcrombeWpCount-1)
                {
                    distanceX = (SorcererAshcrombeWPS[i].x - SorcererAshcrombeWPS[i-1].x)*(SorcererAshcrombeWPS[i].x - SorcererAshcrombeWPS[i-1].x);
                    distanceY = (SorcererAshcrombeWPS[i].y - SorcererAshcrombeWPS[i-1].y)*(SorcererAshcrombeWPS[i].y - SorcererAshcrombeWPS[i-1].y);
                    distance = std::sqrt(distanceX + distanceY);
                }
                waitTime = 300.0f + (1000 * std::abs(distance / walkSpeed));
                AddWaypoint(CreateWaypoint(i + 1, static_cast<uint32>(waitTime), Movement::WP_MOVE_TYPE_WALK, SorcererAshcrombeWPS[i]));
            }

            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
            stage = 0;

            // Remove Gossip
            pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }

        void OnReachWP(uint32 iWaypointId, bool /*bForwards*/) override
        {
            if (iWaypointId == 10)
            {
                // Do script update every 2s
                RegisterAIUpdateEvent(2000);
            }

            if (iWaypointId > 0 && iWaypointId < 11)
            {
                SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                SetWaypointToMove(++iWaypointId);
            }
        }

        void AIUpdate() override
        {
            if (SFK_instance 
                && (SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == InProgress
                || SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == Performed))
            {
                if (eventStarted)
                {
                    switch (stage)
                    {
                        // Starting movement
                        case 0:
                        {
                            getCreature()->SetEmoteState(EMOTE_STATE_NONE);
                            SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                            SetWaypointToMove(1);
                            RemoveAIUpdateEvent();
                        }break;
                        // Face him to doors
                        case 1:
                        {
                            getCreature()->SetFacing(1.33f);
                        }break;
                        // Preparing to cast spell
                        case 2:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ASHCROMBE_OPEN_DOOR);
                        }break;
                        // Casting unlock spell and calling next events every 6 seconds
                        case 3:
                        {
                            ModifyAIUpdateEvent(6000);
                            getCreature()->CastSpell(getCreature(), SPELL_ASHCROMBE_UNLOCK, false);
                        }break;
                        // Setting instance data to finished
                        case 4:
                        {
                            getCreature()->SendScriptTextChatMessage(SAY_ASHCROMBE_BYE);
                        }break;
                        // Final stage - casting spell which despawns Ashcrombe Sorcerer
                        case 5:
                        {
                            getCreature()->CastSpell(getCreature(), SPELL_ASHCROMBE_FIRE, true);
                            getCreature()->SendScriptTextChatMessage(SAY_ASHCROMBE_VANISH);
                            SFK_instance->SetLocaleInstanceData(0, INDEX_PRISONER_EVENT, Finished);
                            RemoveAIUpdateEvent();
                        }break;
                        default:
                            break;
                    }
                    ++stage;
                }
            }

            if (!argued)
            {
                getCreature()->SendScriptTextChatMessage(SAY_ASHCROMBE_BOSS_DEATH);
                RemoveAIUpdateEvent();
                argued = true;
            }
        }

    protected:

        uint32 stage;
        ShadowfangKeepInstance* SFK_instance;

        // Used to say text after Adamant, after boss kill
        bool argued;

    public:

        // This variable will be set to true on OnSelectOption gossip event
        bool eventStarted;
};

class AshcrombeGossip : public Arcemu::Gossip::Script
{
    public:

        void OnHello(Object* pObject, Player* plr) override
        {
            Arcemu::Gossip::Menu menu(pObject->GetGUID(), sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry()));

            ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript());
            if (pInstance != nullptr && pInstance->GetInstanceData(0, INDEX_RETHILGORE) == Finished && pInstance->GetInstanceData(0, INDEX_PRISONER_EVENT) == NotStarted)
            {
                menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(prisonerGossipOptionID), 1);
            }
            menu.Send(plr);
        }

        void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* /*Code*/, uint32 /*gossipId*/) override
        {
            if (Id == 1)
            {
                if (AshcrombeAI* pPrisoner = static_cast<AshcrombeAI*>(static_cast<Creature*>(pObject)->GetScript()))
                {
                    pPrisoner->getCreature()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    pPrisoner->getCreature()->SendScriptTextChatMessage(SAY_ASHCROMBE_FOLLOW);
                    pPrisoner->RegisterAIUpdateEvent(4000);
                    pPrisoner->getCreature()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                    pPrisoner->getCreature()->GetAIInterface()->SetAllowedToEnterCombat(false);
                    pPrisoner->getCreature()->Emote(EMOTE_ONESHOT_POINT);
                    pPrisoner->eventStarted = true;
                    if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript()))
                        pInstance->SetLocaleInstanceData(0, INDEX_PRISONER_EVENT, InProgress);
                }
            }
            Arcemu::Gossip::Menu::Complete(plr);
        }
};

///////////////////////////////////////////////////////////////////
// Main bosses events

// Creature entry: 4278
class SpringvaleAI : public CreatureAIScript
{
        enum SpringvaleSpells
        {
            SPELL_DEVO_AURA            = 643,
            SPELL_HOLY_LIGHT           = 1026,
            SPELL_HAMMER_OF_JUSTICE    = 5588,
            SPELL_DIVINE_PROT          = 13007
        };

        ADD_CREATURE_FACTORY_FUNCTION(SpringvaleAI);
        SpringvaleAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            

            addAISpell(SPELL_HOLY_LIGHT, 10.0f, TARGET_RANDOM_FRIEND, 3, 0);

            DevoAura = addAISpell(SPELL_DEVO_AURA, 0.0f, TARGET_SELF, 0, 0);
            DivineProt = addAISpell(SPELL_DIVINE_PROT, 0.0f, TARGET_SELF, 0, 0);
            mEnableDivineProt = true;

            addAISpell(SPELL_HAMMER_OF_JUSTICE, 12.0f, TARGET_ATTACKING, 0, 60);
        }

        void OnCombatStart(Unit* /*pTarget*/) override
        {
            // Turn aura ON!
            if (!getCreature()->HasAura(SPELL_DEVO_AURA))
                _castAISpell(DevoAura);
        }

        void OnCombatStop(Unit* /*pTarget*/) override
        {
            // Turn aura OFF!
            if (getCreature()->HasAura(SPELL_DEVO_AURA))
                _removeAura(SPELL_DEVO_AURA);
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 20 && mEnableDivineProt)
            {
                _castAISpell(DivineProt);
                mEnableDivineProt = false;
            }
        }

    protected:

        CreatureAISpells* DevoAura;
        CreatureAISpells* DivineProt;
        bool mEnableDivineProt;
};

// Creature entry: 3914
class RethilgoreAI : public CreatureAIScript
{
    const uint32 SPELL_SOUL_DRAIN = 7295;

        ADD_CREATURE_FACTORY_FUNCTION(RethilgoreAI);
        RethilgoreAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_SOUL_DRAIN, 8.0f, TARGET_RANDOM_SINGLE, 2, 10);
        }
};

// Creature entry: 3927
class NandosAI : public CreatureAIScript
{
        enum NandosAISpells : uint32
        {
            SPELL_CALL_BLEAK_WORG        = 7487,
            SPELL_CALL_SLAVERING_WORG    = 7488,
            SPELL_CALLLUPINE_HORROR      = 7489
        };

        ADD_CREATURE_FACTORY_FUNCTION(NandosAI);
        NandosAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());

            sCallBleakWord = addAISpell(SPELL_CALL_BLEAK_WORG, 0.0f, TARGET_SELF);
            sCallSlaveringWorg = addAISpell(SPELL_CALL_SLAVERING_WORG, 0.0f, TARGET_SELF);
            sCallLupineHorror = addAISpell(SPELL_CALLLUPINE_HORROR, 0.0f, TARGET_SELF);
            Reset();
        }

        void Reset()
        {
            sCallBleakWorg_Timer        = 0;
            sCallSlaveringWorg_Timer    = 0;
            sCallLupineHorror_Timer     = 0;

            mEnableCallBleakWord = true;
            mEnableCallSlaveringWorg = true;
            mEnableCallLupineHorror = true;
        }

        void OnCombatStart(Unit* /*mEnemy*/) override
        {
            
            sCallBleakWorg_Timer = _addTimer(Util::getRandomUInt(1) ? 33700 : 48800);
            sCallSlaveringWorg_Timer = _addTimer(Util::getRandomUInt(1) ? 45400 : 51700);
            sCallLupineHorror_Timer = _addTimer(69500);
            if (SFK_instance)
                SFK_instance->SetLocaleInstanceData(0, INDEX_NANDOS, InProgress);
        }

        void OnCombatStop(Unit* /*mEnemy*/) override
        {
            // Battle has failed
            if (SFK_instance)
                SFK_instance->SetLocaleInstanceData(0, INDEX_NANDOS, InvalidState);

            Reset();
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 80)
            {
                if (_isTimerFinished(sCallBleakWorg_Timer) && mEnableCallBleakWord)
                {
                    _castAISpell(sCallBleakWord);
                    _removeTimer(sCallBleakWorg_Timer);
                    mEnableCallBleakWord = false;
                }

                if (_isTimerFinished(sCallSlaveringWorg_Timer) && mEnableCallSlaveringWorg)
                {
                    _castAISpell(sCallSlaveringWorg);
                    _removeTimer(sCallSlaveringWorg_Timer);
                    mEnableCallSlaveringWorg = false;
                }

                if (_isTimerFinished(sCallLupineHorror_Timer) && mEnableCallLupineHorror)
                {
                    _castAISpell(sCallLupineHorror);
                    _removeTimer(sCallLupineHorror_Timer);
                    mEnableCallLupineHorror = false;
                }
            }
        }

    protected:

        ShadowfangKeepInstance* SFK_instance;
        uint32 sCallBleakWorg_Timer;
        uint32 sCallSlaveringWorg_Timer;
        uint32 sCallLupineHorror_Timer;

        CreatureAISpells* sCallBleakWord;
        CreatureAISpells* sCallSlaveringWorg;
        CreatureAISpells* sCallLupineHorror;

        bool mEnableCallBleakWord;
        bool mEnableCallSlaveringWorg;
        bool mEnableCallLupineHorror;
};

// Creature entry: 3887
class BaronSilverlaineAI : public CreatureAIScript
{
        const uint32 SPELL_VEIL_OF_SHADOW = 7068;

        ADD_CREATURE_FACTORY_FUNCTION(BaronSilverlaineAI);
        BaronSilverlaineAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_VEIL_OF_SHADOW, 10.0f, TARGET_ATTACKING, 0, 2);
        }
};

// Creature entry: 4279
class BlindWatcherAI : public CreatureAIScript
{
        enum ODO_THE_BLINDWATCHER_SPELLS : uint32
        {
            ODO_HOWLING_RAGE1 = 7481,
            ODO_HOWLING_RAGE2 = 7483,
            ODO_HOWLING_RAGE3 = 7484
        };

        ADD_CREATURE_FACTORY_FUNCTION(BlindWatcherAI);
        BlindWatcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            HowlingRage1 = addAISpell(ODO_HOWLING_RAGE1, 0.0f, TARGET_SELF);
            HowlingRage2 = addAISpell(ODO_HOWLING_RAGE2, 0.0f, TARGET_SELF);
            HowlingRage3 = addAISpell(ODO_HOWLING_RAGE3, 0.0f, TARGET_SELF);
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 75 && getScriptPhase() == 1)
            {
                setScriptPhase(2);
            }
            else if (_getHealthPercent() <= 45 && getScriptPhase() == 2)
            {
                setScriptPhase(3);
            }
            else if (_getHealthPercent() <= 20 && getScriptPhase() == 3)
            {
                setScriptPhase(4);
            }
        }

        void OnScriptPhaseChange(uint32_t scriptPhase) override
        {
            switch (scriptPhase)
            {
                case 2:
                    _castAISpell(HowlingRage1);
                    break;
                case 3:
                    _castAISpell(HowlingRage2);
                    break;
                case 4:
                    _castAISpell(HowlingRage3);
                    break;
            }
        }

    protected:

        CreatureAISpells* HowlingRage1;
        CreatureAISpells* HowlingRage2;
        CreatureAISpells* HowlingRage3;
};

// Creature entry: 4274
class FenrusAI : public CreatureAIScript
{
        const uint32 SPELL_TOXIC_SALIVA = 7125;

        ADD_CREATURE_FACTORY_FUNCTION(FenrusAI);
        FenrusAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_TOXIC_SALIVA, 12.0f, TARGET_ATTACKING, 2, 60);
        }
};

// Creature entry: 4275
class ArugalBossAI : public CreatureAIScript
{
        enum Arugal_Boss_Spells : uint32
        {
            SPELL_VOID_BOLT                 = 7588,
            SPELL_SHADOW_PORT_UPPER_LEDGE   = 7587,
            SPELL_SHADOW_PORT_SPAWN_LEDGE   = 7586,
            SPELL_SHADOW_PORT_STAIRS        = 7136,
            SPELL_THUNDER_SHOCK             = 7803,
            SPELL_ARUGALS_CURSE             = 7621
        };

        enum ArugalLoc : uint8
        {
            ARUGAL_LOC_LEDGE        = 0,    // Arugal's base spawn location
            ARUGAL_LOC_UPPER_LEDGE  = 1,    // Room corner
            ARUGAL_LOC_STAIRS       = 2     // Stairs
        };


        ADD_CREATURE_FACTORY_FUNCTION(ArugalBossAI);
        ArugalBossAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0), arugalPosition(ARUGAL_LOC_LEDGE)
        {
            SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());

            sVoidBolt = addAISpell(SPELL_VOID_BOLT, 0.0f, TARGET_ATTACKING);

            addAISpell(SPELL_THUNDER_SHOCK, 10.0f, TARGET_SELF);
            addAISpell(SPELL_ARUGALS_CURSE, 5.0f, TARGET_RANDOM_SINGLE);

            addEmoteForEvent(Event_OnCombatStart, YELL_ARUGAL_AGROO);
            addEmoteForEvent(Event_OnTargetDied, YELL_ARUGAL_ENEMY_DEATH);
            addEmoteForEvent(Event_OnTaunt, YELL_ARUGAL_COMBAT);
            setAIAgent(AGENT_SPELL);

            aiUpdateOriginal = GetAIUpdateFreq();
            originalRegen = getCreature()->PctPowerRegenModifier[POWER_TYPE_MANA];
        }

        void OnCastSpell(uint32 spellId) override
        {
            if (spellId == SPELL_ARUGALS_CURSE)
            {
                getCreature()->SendScriptTextChatMessage(YELL_ARUGAL_COMBAT);
            }
        }

        void Reset()
        {
            setAIAgent(AGENT_SPELL);
            getCreature()->GetAIInterface()->setMeleeDisabled(true);
            getCreature()->PctPowerRegenModifier[POWER_TYPE_MANA] = originalRegen;
        }

        void OnCombatStart(Unit* /*pEnemy*/) override
        {
            // do not regen mana
            getCreature()->PctPowerRegenModifier[POWER_TYPE_MANA] = 0.3f;
            aiUpdateOriginal = GetAIUpdateFreq();
            originalRegen = getCreature()->PctPowerRegenModifier[POWER_TYPE_MANA];

            // Do not do melee attacks
            getCreature()->GetAIInterface()->setMeleeDisabled(true);
        }

        void OnCombatStop(Unit* /*pEnemy*/) override
        {
            Reset();
        }

        void FenrusEvent(uint32 pStage)
        {
            switch (pStage)
            {
                case 0:
                {
                    ModifyAIUpdateEvent(6000);
                    _applyAura(SPELL_ARUGAL_SPAWN);
                    getCreature()->SendScriptTextChatMessage(YELL_ARUGAL_FENRUS);
                }break;
                case 1:
                {
                    if (GameObject* pGO = getNearestGameObject(GO_ARUGAL_FOCUS))
                    {
                        pGO->SetState(GO_STATE_OPEN);
                    }

                    // Spawn Arugal's Voidwalkers
                    for (uint8 x = 0; x < ArugalVoidCount; x++)
                    {
                        if (CreatureAIScript* voidwalker = spawnCreatureAndGetAIScript(CN_VOIDWALKER, voidwalkerSpawns[x].x, voidwalkerSpawns[x].y, voidwalkerSpawns[x].z, voidwalkerSpawns[x].o))
                        {
                            voidwalker->despawn(4 * 60 * 1000); // Despawn in 4 mins
                        }
                    }
                    getCreature()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                    getCreature()->GetAIInterface()->SetAllowedToEnterCombat(true);
                    getCreature()->GetAIInterface()->m_canMove = true;

                    // sanctum32: not sure if it is correct spell id
                    getCreature()->CastSpell(getCreature(), SPELL_ASHCROMBE_FIRE, true);
                    SFK_instance->SetLocaleInstanceData(0, INDEX_VOIDWALKER, Finished);
                    RemoveAIUpdateEvent();
                }break;
            }
        }

        void AIUpdate() override
        {
            if (SFK_instance && SFK_instance->GetInstanceData(0, INDEX_VOIDWALKER) == InProgress)
            {
                FenrusEvent(stage);
                ++stage;
            }

            if (_isInCombat())
            {
                // if mana is out - do melee attacks
                if (getCreature()->GetManaPct() <= 10 && getAIAgent() == AGENT_SPELL)
                {
                    setAIAgent(AGENT_MELEE);
                    getCreature()->GetAIInterface()->setMeleeDisabled(false);
                }
                // boss got mana regenerated
                else
                {
                    setAIAgent(AGENT_SPELL);
                    getCreature()->GetAIInterface()->setMeleeDisabled(true);
                }

                // Cast void bolt non stop
                if (getAIAgent() == AGENT_SPELL)
                {
                    _castAISpell(sVoidBolt);
                }

                if (_getHealthPercent() <= 25)
                {
                    if (arugalPosition == ARUGAL_LOC_UPPER_LEDGE)
                    {
                        ModifyAIUpdateEvent(aiUpdateOriginal);
                        getCreature()->CastSpell(getCreature(), SPELL_SHADOW_PORT_STAIRS, true);
                        arugalPosition = ARUGAL_LOC_STAIRS;
                        setRooted(false);
                    }

                    if (arugalPosition == ARUGAL_LOC_LEDGE)
                    {
                        aiUpdateOriginal = GetAIUpdateFreq();
                        ModifyAIUpdateEvent(3000);
                        getCreature()->CastSpell(getCreature(), SPELL_SHADOW_PORT_UPPER_LEDGE, true);
                        arugalPosition = ARUGAL_LOC_UPPER_LEDGE;
                        setRooted(true);
                    }
                }
            }
        }

    protected:

        uint8 stage;
        uint8 arugalPosition;
        ShadowfangKeepInstance* SFK_instance;
        CreatureAISpells* sVoidBolt;

        uint32 aiUpdateOriginal;
        float originalRegen;
};

// Creature entry: 3886
class RazorclawTheButcherAI : public CreatureAIScript
{
        const uint32 SPELL_BUTCHER_DRAIN = 7485;

        ADD_CREATURE_FACTORY_FUNCTION(RazorclawTheButcherAI);
        RazorclawTheButcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_BUTCHER_DRAIN, 5.0f, TARGET_RANDOM_SINGLE);
        }
};

///////////////////////////////////////////////////////////////////
// Trash npcs

// Creature entry: 3866
class VileBatAI : public CreatureAIScript
{
        enum VileBatSpells : uint32
        {
            SPELL_DIVING_SWEEP  = 7145,
            SPELL_DISARM        = 6713
        };

        ADD_CREATURE_FACTORY_FUNCTION(VileBatAI);
        VileBatAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_DIVING_SWEEP, 8.0f, TARGET_ATTACKING);
            addAISpell(SPELL_DISARM, 5.0f, TARGET_ATTACKING, 0, 6);
        }
};

// Creature entry: 3868
class BloodSeekerAI : public CreatureAIScript
{
        const uint32 SPELL_EXPOSE_WEAKNESS = 7140;

        ADD_CREATURE_FACTORY_FUNCTION(BloodSeekerAI);
        BloodSeekerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            addAISpell(SPELL_EXPOSE_WEAKNESS, 5.0f, TARGET_ATTACKING, 0, 5);
        }
};

// Creature entry: 4627
class VoidWalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(VoidWalkerAI);
        VoidWalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Dark Offering
            addAISpell(7154, 5.0f, TARGET_RANDOM_FRIEND, 0, 7);
        }
};

// Creature entry: 3861
class BleakWorgAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(BleakWorgAI);
        BleakWorgAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Wavering Will
            addAISpell(7127, 5.0f, TARGET_RANDOM_SINGLE, 0, 60);
        }
};

// Creature entry: 3863
class LupineHorrorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(LupineHorrorAI);
        LupineHorrorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Summon Lupine Delusions
            addAISpell(7132, 5.0f, TARGET_SELF, 0, 240);
        }
};

// Creature entry: 2529
class SonOfArugalAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(SonOfArugalAI);
        SonOfArugalAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Arugal's Gift
            addAISpell(7124, 5.0f, TARGET_ATTACKING, 3, 0);
        }
};

// Creature entry: 3853
class ShadowfangMoonwalkerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowfangMoonwalkerAI);
        ShadowfangMoonwalkerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Anti-Magic Shield
            addAISpell(7121, 5.0f, TARGET_SELF, 2, 10);
        }
};

// Creature entry: 3855
class ShadowfangDarksoulAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowfangDarksoulAI);
        ShadowfangDarksoulAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Befuddlement
            addAISpell(8140, 8.0f, TARGET_RANDOM_SINGLE, 0, 15);

            // Shadow Word : Pain
            addAISpell(970, 5.0f, TARGET_RANDOM_SINGLE, 0, 18);
        }
};

// Creature entry: 3857
class ShadowfangGluttonAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(ShadowfangGluttonAI);
        ShadowfangGluttonAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {

            // Blood Tap
            addAISpell(7122, 5.0f, TARGET_ATTACKING);
        }
};

// Creature entry: 3859
class ShadowfangRagetoothAI : public CreatureAIScript
{
        const uint32 SPELL_WILD_RAGE = 7072;

        ADD_CREATURE_FACTORY_FUNCTION(ShadowfangRagetoothAI);
        ShadowfangRagetoothAI(Creature* pCreature) : CreatureAIScript(pCreature), sWildRageCasted(false)
        {
        }

        void AIUpdate() override
        {
            // Cast Wild rage at 30% health
            if (_getHealthPercent() <= 30 && !getCreature()->HasAura(SPELL_WILD_RAGE) && !sWildRageCasted)
            {
                getCreature()->CastSpell(getCreature(), SPELL_WILD_RAGE, true);
                sWildRageCasted = true;
            }
        }

    protected:

        bool sWildRageCasted;
};

// Creature entry: 3864
class FelSteedAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(FelSteedAI);
        FelSteedAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Fel Stomp
            addAISpell(7139, 5.0f, TARGET_ATTACKING, 0, 3);
        }
};

// Creature entry: 3872
class DeathswornCaptainAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(DeathswornCaptainAI);
        DeathswornCaptainAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Hamstring
            addAISpell(9080, 5.0f, TARGET_ATTACKING, 0, 10);

            // Cleave
            addAISpell(40505, 8.0f, TARGET_ATTACKING, 0, 10);
        }
};

// Creature entry: 3873
class TormentedOfficerAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(TormentedOfficerAI);
        TormentedOfficerAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Forsaken Skills (TODO: implement dummy aura of this spell)
            addAISpell(7054, 5.0f, TARGET_ATTACKING, 2, 300);
        }
};

// Creature entry: 3875
class HauntedServitorAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(HauntedServitorAI);
        HauntedServitorAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Haunting Spirits (TODO: implement dummy aura of this spell)
            addAISpell(7057, 5.0f, TARGET_ATTACKING, 2, 300);
        }
};

// Creature entry: 3877
class WaillingGuardsmanAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WaillingGuardsmanAI);
        WaillingGuardsmanAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
            // Screams of the Past
            addAISpell(7074, 5.0f, TARGET_SELF, 0, 5);
        }
};

// Creature entry: 3877
class WorlfguardWorgAI : public CreatureAIScript
{
        ADD_CREATURE_FACTORY_FUNCTION(WorlfguardWorgAI);
        WorlfguardWorgAI(Creature* pCreature) : CreatureAIScript(pCreature)
        {
        }

        void AIUpdate() override
        {
            if (_getHealthPercent() <= 15 && getAIAgent() != AGENT_FLEE)
            {
                setAIAgent(AGENT_FLEE);
            }
        }
};

///////////////////////////////////////////////////////////////////
// Spells used by creatures in Shadowfang keep dungeon

// Spell entry: 6421
bool ashrombeUnlockDummySpell(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->u_caster;
    if (!target)
    {
        return false;
    }

    if (GameObject* pGameObject = target->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), GO_COURTYARD_DOOR))
    {
        pGameObject->SetState(GO_STATE_OPEN);
        return true;
    }

    return false;
}

// Spell entry: 6422
bool ashrombeTeleportDummyAura(uint8_t /*effectIndex*/, Aura* pAura, bool /*apply*/)
{
    Unit* target = pAura->GetUnitCaster();
    if (!target || !target->IsCreature())
    {
        return false;
    }
    else
    {
        Creature* creatureCaster = static_cast<Creature*>(target);
        creatureCaster->Despawn(3000, 0);
        return true;
    }
}

#endif //VERSION_STRING != Cata

void SetupShadowfangKeep(ScriptMgr* mgr)
{
#if VERSION_STRING != Cata
    // Map
    mgr->register_instance_script(SHADOWFANG_KEEP_MAP, &ShadowfangKeepInstance::Create);

    // Arugal (intro event)
    mgr->register_creature_script(CN_ARUGAL, &ArugalAI::Create);

    // Prisoners
    mgr->register_creature_gossip(CN_ADAMANT, new AdamantGossip());
    mgr->register_creature_script(CN_ADAMANT, &AdamantAI::Create);
    mgr->register_creature_gossip(CN_ASHCROMBE, new AshcrombeGossip());
    mgr->register_creature_script(CN_ASHCROMBE, &AshcrombeAI::Create);

    // Bosses
    mgr->register_creature_script(CN_RETHILGORE, &RethilgoreAI::Create);
    mgr->register_creature_script(CN_NANDOS, &NandosAI::Create);
    mgr->register_creature_script(CN_BARON_SILVERLAINE, &BaronSilverlaineAI::Create);
    mgr->register_creature_script(CN_FENRUS, &FenrusAI::Create);
    mgr->register_creature_script(CN_ARUGAL_BOSS, &ArugalBossAI::Create);
    mgr->register_creature_script(CN_SPRINGVALE, &SpringvaleAI::Create);
    mgr->register_creature_script(CN_BLINDWATCHER, &BlindWatcherAI::Create);
    mgr->register_creature_script(CN_RAZORCLAW_THE_BUTCHER, &RazorclawTheButcherAI::Create);

    // Trash mobs
    mgr->register_creature_script(CN_VILE_BAT, &VileBatAI::Create);
    mgr->register_creature_script(CN_BLOOD_SEEKER, &BloodSeekerAI::Create);
    mgr->register_creature_script(CN_BLEAK_WORG, &BleakWorgAI::Create);
    mgr->register_creature_script(CN_LUPINE_HORROR, &LupineHorrorAI::Create);
    mgr->register_creature_script(CN_VOIDWALKER, &VoidWalkerAI::Create);
    mgr->register_creature_script(CN_SON_OF_ARUGAL, &SonOfArugalAI::Create);
    mgr->register_creature_script(CN_SHADOWFANG_MOONWALKER, &ShadowfangMoonwalkerAI::Create);
    mgr->register_creature_script(CN_SHADOWFANG_DARKSOUL, &ShadowfangDarksoulAI::Create);
    mgr->register_creature_script(CN_SHADOWFANG_GLUTTON, &ShadowfangGluttonAI::Create);
    mgr->register_creature_script(CN_SHADOWFANG_RAGETOOTH, &ShadowfangRagetoothAI::Create);
    mgr->register_creature_script(CN_FEL_STEED, &FelSteedAI::Create);
    mgr->register_creature_script(CN_DEATHSWORN_CAPTAIN, &DeathswornCaptainAI::Create);
    mgr->register_creature_script(CN_TORMENTED_OFFICER, &TormentedOfficerAI::Create);
    mgr->register_creature_script(CN_HAUNTED_SERVITOR, &HauntedServitorAI::Create);
    mgr->register_creature_script(CN_WAILLING_GUARDSMAN, &WaillingGuardsmanAI::Create);

    // Spells
    mgr->register_dummy_spell(SPELL_ASHCROMBE_UNLOCK, &ashrombeUnlockDummySpell);
    mgr->register_dummy_aura(SPELL_ASHCROMBE_FIRE, &ashrombeTeleportDummyAura);
#else
    if (mgr != nullptr) { return; }
#endif //VERSION_STRING != Cata
}
