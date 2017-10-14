/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_ShadowfangKeep.h"
#include "Spell/SpellAuras.h"
#include "Spell/Definitions/PowerType.h"

 /// Instance script for map 33 (Shadowfang Keep)
class ShadowfangKeepInstance : public MoonInstanceScript
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
    uint64 npc_ashcrombe_GUID;
    uint64 npc_adamant_GUID;

    // Nandos event related
    std::list<uint64 /*guid*/> nandos_summons;

    // Encounters data
    uint32 m_encounterData[INDEX_MAX];
public:
    MOONSCRIPT_INSTANCE_FACTORY_FUNCTION(ShadowfangKeepInstance, MoonInstanceScript);
    ShadowfangKeepInstance(MapMgr* pMapMgr) :
        MoonInstanceScript(pMapMgr),

        // Gameobjects low guids
        go_leftCell_GUID(0),
        go_middleCell_GUID(0),
        go_rightCell_GUID(0),
        go_arugalsLair_GUID(0),
        go_sorcererGate_GUID(0),
        go_leftCellLever_GUID(0),
        go_middleCellLever_GUID(0),
        go_rightCellLever_GUID(0),

        // Creatures low guids
        npc_ashcrombe_GUID(0),
        npc_adamant_GUID(0)
    {
        // NandosAI event related
        nandos_summons.clear();

        // Set encounters data to State_NotStarted
        memset(m_encounterData, State_NotStarted, sizeof(m_encounterData));
    }

    void SetInstanceData(uint32 /*pType*/, uint32 pIndex, uint32 pData)
    {
        if (pIndex >= INDEX_MAX)
            return;

        switch (pIndex)
        {
            case INDEX_VOIDWALKER:
            {
                if (pData == State_InProgress)
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
                if (pData == State_InvalidState || pData == State_Finished)
                {
                    for (std::list<uint64>::iterator itr = nandos_summons.begin(); itr != nandos_summons.end();)
                    {
                        if (Creature* pCreature = GetInstance()->GetCreature(GET_LOWGUID_PART(*itr)))
                        {
                            pCreature->Despawn(1000, 0);
                        }
                        itr = nandos_summons.erase(itr);
                    }
                    // Despawn creatures
                }

                if (pData == State_Finished)
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
                if (pData == State_Finished)
                {
                    // Set gossip flags for both prisoners and push texts
                    if (Creature* pCreature = GetInstance()->GetCreature(GET_LOWGUID_PART(npc_adamant_GUID)))
                    {
                        if (pCreature->isAlive())
                        {
                            pCreature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                            pCreature->SendScriptTextChatMessage(SAY_ADAMANT_BOSS_DEATH);
                        }
                    }

                    if (Creature* pCreature = GetInstance()->GetCreature(GET_LOWGUID_PART(npc_ashcrombe_GUID)))
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
                if (pData == State_Finished || pData == State_Performed)
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
                if (pData == State_Finished)
                {
                    SetInstanceData(0, INDEX_VOIDWALKER, State_InProgress);
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
    void OnGameObjectPushToWorld(GameObject* pGameObject)
    {
        ParentClass::OnGameObjectPushToWorld(pGameObject);
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
                if (GetInstanceData(0, INDEX_NANDOS) == State_Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                {
                    pGameObject->SetState(GO_STATE_OPEN);
                }
            }break;
            case GO_SORCERER_GATE:
            {
                go_sorcererGate_GUID = pGameObject->GetLowGUID();
                if (GetInstanceData(0, INDEX_FENRUS) == State_Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                {
                    pGameObject->SetState(GO_STATE_OPEN);
                }
            }break;
            case GO_LEFT_LEVER:
            {
                go_leftCellLever_GUID = pGameObject->GetLowGUID();
                if (GetInstanceData(0, INDEX_RETHILGORE) != State_Finished)
                {
                    pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                }
            }break;
            case GO_RIGHT_LEVER:
            {
                go_rightCellLever_GUID = pGameObject->GetLowGUID();
                if (GetInstanceData(0, INDEX_RETHILGORE) != State_Finished)
                {
                    pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                }
            }break;
            case GO_MIDDLE_LEVER:
            {
                go_middleCellLever_GUID = pGameObject->GetLowGUID();
                if (GetInstanceData(0, INDEX_RETHILGORE) != State_Finished)
                {
                    pGameObject->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NONSELECTABLE);
                }
            }break;
            case GO_COURTYARD_DOOR:
            {
                go_courtyarDoor_GUID = pGameObject->GetLowGUID();
                if (GetInstanceData(0, INDEX_PRISONER_EVENT) == State_Finished && pGameObject->GetState() == GO_STATE_CLOSED)
                {
                    pGameObject->SetState(GO_STATE_OPEN);
                }
            }break;
            default:
                break;
        }
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* pPlayer)
    {
        ParentClass::OnGameObjectActivate(pGameObject, pPlayer);
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

    void OnCreatureDeath(Creature* pCreature, Unit* pKiller)
    {
        ParentClass::OnCreatureDeath(pCreature, pKiller);
        switch (pCreature->GetEntry())
        {
            case CN_NANDOS:
            {
                SetInstanceData(0, INDEX_NANDOS, State_Finished);
            }break;
            case CN_RETHILGORE:
            {
                SetInstanceData(0, INDEX_RETHILGORE, State_Finished);
            }break;
            case CN_FENRUS:
            {
                SetInstanceData(0, INDEX_FENRUS, State_Finished);
            }break;
            default:
                break;
        }
    }

    void OnCreaturePushToWorld(Creature* pCreature)
    {
        ParentClass::OnCreaturePushToWorld(pCreature);
        switch (pCreature->GetEntry())
        {
            case CN_ADAMANT:
            {
                npc_adamant_GUID = pCreature->GetGUID();
            }break;
            case CN_ASHCROMBE:
            {
                npc_ashcrombe_GUID = pCreature->GetGUID();
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
                if (GetInstanceData(0, INDEX_NANDOS) == State_InProgress)
                {
                    pCreature->Despawn(60 * 4 * 1000, 0);   // Despawn in 4 mins
                    nandos_summons.push_back(pCreature->GetGUID());
                }
            }break;
            case CN_LUPINE_DELUSION:
            {
                // Add to nandos summon lists only on his event is started
                if (GetInstanceData(0, INDEX_NANDOS) == State_InProgress)
                {
                    nandos_summons.push_back(pCreature->GetGUID());
                }
                pCreature->Despawn(60 * 4 * 1000, 0);   // Despawn in 4 mins
            }break;
            case CN_DEATHSTALKER_VINCENT:
            {
                if (GetInstanceData(0, INDEX_ARUGAL_INTRO) == State_Finished)
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
/// Special npcs events
/////////////////////////////////////////////////////

// Arugal intro event
// Creature entries: 10000, 4444

class ArugalAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(ArugalAI, MoonScriptCreatureAI);
    ArugalAI(Creature* pCreature) :
        MoonScriptCreatureAI(pCreature),
        stage(0)
    {
        SFK_Instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());
        if (SFK_Instance && SFK_Instance->GetInstanceData(0, INDEX_ARUGAL_INTRO) == State_NotStarted)
        {
            RegisterAIUpdateEvent(500);
            SFK_Instance->SetInstanceData(0, INDEX_ARUGAL_INTRO, State_InProgress);
        }
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (SFK_Instance && SFK_Instance->GetInstanceData(0, INDEX_ARUGAL_INTRO) == State_InProgress)
        {
            switch (stage)
            {
            case 0:
            {
                GetUnit()->SetInvisFlag(INVIS_FLAG_NORMAL);
                GetUnit()->CastSpell(GetUnit(), SPELL_ARUGAL_SPAWN, true);
                ModifyAIUpdateEvent(5500);  // call every step after 5.5 seconds
                if (Creature* pVincent = static_cast<Creature*>(ForceCreatureFind(CN_DEATHSTALKER_VINCENT, GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ())))
                {
                    pVincent->GetAIInterface()->AttackReaction(GetUnit(), 1);
                    pVincent->GetAIInterface()->disable_melee = true;
                }
            }break;
            case 1:
            {
                GetUnit()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO1);
            }break;
            case 2:
            {
                GetUnit()->Emote(EMOTE_ONESHOT_POINT);
            }break;
            case 3:
            {
                GetUnit()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO2);
            }break;
            case 4:
            {
                GetUnit()->Emote(EMOTE_ONESHOT_EXCLAMATION);
            }break;
            case 5:
            {
                GetUnit()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO3);
            }break;
            case 6:
            {
                GetUnit()->Emote(EMOTE_ONESHOT_LAUGH);
            }break;
            case 7:
            {
                if (Creature* pVincent = GetUnit()->GetMapMgr()->GetInterface()->GetCreatureNearestCoords(GetUnit()->GetPositionX(), GetUnit()->GetPositionY(), GetUnit()->GetPositionZ(), CN_DEATHSTALKER_VINCENT))
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
                GetUnit()->SendScriptTextChatMessage(SAY_ARUGAL_INTRO4);
            }break;
            case 9:
            {
                GetUnit()->CastSpell(GetUnit(), SPELL_ARUGAL_SPAWN, true);
                SFK_Instance->SetInstanceData(0, INDEX_ARUGAL_INTRO, State_Finished);
                GetUnit()->SetInvisFlag(INVIS_FLAG_TOTAL);
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

class AdamantAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(AdamantAI, MoonScriptCreatureAI);
    AdamantAI(Creature* pCreature) : 
        MoonScriptCreatureAI(pCreature),
        stage(0),
        eventStarted(false)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(GetUnit()->GetMapMgr()->GetScript());

        for (uint8 i = 0; i < adamantWpCount; ++i)
        {
            float waitTime = 0;
            float distanceX = 0;
            float distanceY = 0;
            float distance = 0;
            float walkSpeed = GetUnit()->GetCreatureProperties()->walk_speed;
            float runSpeed = GetUnit()->GetCreatureProperties()->run_speed;
            // first waypoint
            if (i == 0)
            {
                // (currentWP - perviousLocation) *(currentWP - perviousLocation)
                distanceX = (DeathstalkerAdamantWPS[i].wp_location.x - GetUnit()->GetPositionX())*(DeathstalkerAdamantWPS[i].wp_location.x - GetUnit()->GetPositionX());
                distanceY = (DeathstalkerAdamantWPS[i].wp_location.y - GetUnit()->GetPositionY())*(DeathstalkerAdamantWPS[i].wp_location.y - GetUnit()->GetPositionY());
                distance = sqrt(distanceX - distanceY);
            }
            else if (i != adamantWpCount - 1)
            {
                // (currentWP - perviousWP) *(currentWP - perviousWP)
                distanceX = (DeathstalkerAdamantWPS[i].wp_location.x - DeathstalkerAdamantWPS[i - 1].wp_location.x)*(DeathstalkerAdamantWPS[i].wp_location.x - DeathstalkerAdamantWPS[i - 1].wp_location.x);
                distanceY = (DeathstalkerAdamantWPS[i].wp_location.y - DeathstalkerAdamantWPS[i - 1].wp_location.y)*(DeathstalkerAdamantWPS[i].wp_location.y - DeathstalkerAdamantWPS[i - 1].wp_location.y);
                distance = sqrt(distanceX + distanceY);
            }
            waitTime = 1000 * std::abs(DeathstalkerAdamantWPS[i].wp_flag == Movement::WP_MOVE_TYPE_WALK ? distance / walkSpeed : distance / runSpeed);

            AddWaypoint(CreateWaypoint(i + 1, static_cast<uint32>(waitTime), DeathstalkerAdamantWPS[i].wp_flag, DeathstalkerAdamantWPS[i].wp_location));
        }

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);

        // Remove Gossip
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/)
    {
        switch (iWaypointId)
        {
            case 11:
            {
                RegisterAIUpdateEvent(2000);
            }break;
            case 30:
            {
                GetUnit()->Despawn(2000, 0);
            }break;
            default:
            {
                SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                SetWaypointToMove(++iWaypointId);
            }break;
        }
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (SFK_instance && (SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_InProgress 
            || SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_Performed))
        {
            if (eventStarted)
            {
                switch (stage)
                {
                    case 0:
                    {
                        GetUnit()->SetEmoteState(EMOTE_STATE_NONE);
                        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        SetWaypointToMove(1);
                        RemoveAIUpdateEvent();
                    }break;
                    case 1:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ADAMANT_BEFORE_OPEN);
                    }break;
                    case 2:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ADAMANT_OPENING);
                        GetUnit()->EventAddEmote(EMOTE_ONESHOT_USESTANDING, 8000);
                        ModifyAIUpdateEvent(8000);
                    }break;
                    case 3:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ADAMANT_AFTER_OPEN);
                        SFK_instance->SetInstanceData(0, INDEX_PRISONER_EVENT, State_Performed);
                        ModifyAIUpdateEvent(4000);
                    }break;
                    case 4:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ADAMANT_BYE);
                    }break;
                    case 5:
                    {
                        SFK_instance->SetInstanceData(0, INDEX_PRISONER_EVENT, State_Finished);
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
    void OnHello(Object* pObject, Player* plr)
    {
        //TODO: correct text id
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry()));

        ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript());
        if (pInstance != nullptr && pInstance->GetInstanceData(0, INDEX_RETHILGORE) == State_Finished && pInstance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_NotStarted)
        {
            //TODO: move this to database
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(prisonerGossipOptionID), 1);
        }
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code, uint32 gossipId)
    {
        if (Id == 1)
        {
            if (AdamantAI* pPrisoner = static_cast<AdamantAI*>(static_cast<Creature*>(pObject)->GetScript()))
            {
                pPrisoner->GetUnit()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                pPrisoner->GetUnit()->SendScriptTextChatMessage(SAY_ADAMANT_FOLLOW);
                pPrisoner->RegisterAIUpdateEvent(5000);
                pPrisoner->GetUnit()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                pPrisoner->GetUnit()->GetAIInterface()->SetAllowedToEnterCombat(false);
                pPrisoner->GetUnit()->EventAddEmote(EMOTE_ONESHOT_CHEER, 4000);
                pPrisoner->eventStarted = true;
                if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript()))
                    pInstance->SetInstanceData(0, INDEX_PRISONER_EVENT, State_InProgress);
            }
        }
        Arcemu::Gossip::Menu::Complete(plr);
    }
};

// Prisoner Sorcerer Ashcrombe (entry: 3850) gossip, escort event

class AshcrombeAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(AshcrombeAI, MoonScriptCreatureAI);
    AshcrombeAI(Creature* pCreature) : 
        MoonScriptCreatureAI(pCreature),
        stage(0),
        argued(false),
        eventStarted(false)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(GetUnit()->GetMapMgr()->GetScript());

        for (uint8 i = 0; i < ashcrombeWpCount; ++i)
        {
            float waitTime = 0;
            float distanceX = 0;
            float distanceY = 0;
            float distance = 0;
            float walkSpeed = GetUnit()->GetCreatureProperties()->walk_speed;
            float runSpeed = GetUnit()->GetCreatureProperties()->run_speed;
            if (i == 0) // first waypoint
            {
                distanceX = (SorcererAshcrombeWPS[i].wp_location.x - GetUnit()->GetPositionX())*(SorcererAshcrombeWPS[i].wp_location.x - GetUnit()->GetPositionX());
                distanceY = (SorcererAshcrombeWPS[i].wp_location.y - GetUnit()->GetPositionY())*(SorcererAshcrombeWPS[i].wp_location.y - GetUnit()->GetPositionY());
                distance = std::sqrt(distanceX - distanceY);
            }
            else if (i != ashcrombeWpCount-1)
            {
                distanceX = (SorcererAshcrombeWPS[i].wp_location.x - SorcererAshcrombeWPS[i-1].wp_location.x)*(SorcererAshcrombeWPS[i].wp_location.x - SorcererAshcrombeWPS[i-1].wp_location.x);
                distanceY = (SorcererAshcrombeWPS[i].wp_location.y - SorcererAshcrombeWPS[i-1].wp_location.y)*(SorcererAshcrombeWPS[i].wp_location.y - SorcererAshcrombeWPS[i-1].wp_location.y);
                distance = std::sqrt(distanceX + distanceY);
            }
            waitTime = 300.0f + (1000 * std::abs(SorcererAshcrombeWPS[i].wp_flag == Movement::WP_MOVE_TYPE_WALK ? distance / walkSpeed : distance / runSpeed));
            AddWaypoint(CreateWaypoint(i + 1, static_cast<uint32>(waitTime), (uint32)SorcererAshcrombeWPS[i].wp_flag, SorcererAshcrombeWPS[i].wp_location));
        }

        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_NONE);
        stage = 0;

        // Remove Gossip
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void OnReachWP(uint32 iWaypointId, bool /*bForwards*/)
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

    void AIUpdate()
    {
        ParentClass::AIUpdate();

        if (SFK_instance 
            && (SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_InProgress
            || SFK_instance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_Performed))
        {
            if (eventStarted)
            {
                switch (stage)
                {
                    // Starting movement
                    case 0:
                    {
                        GetUnit()->SetEmoteState(EMOTE_STATE_NONE);
                        SetWaypointMoveType(Movement::WP_MOVEMENT_SCRIPT_WANTEDWP);
                        SetWaypointToMove(1);
                        RemoveAIUpdateEvent();
                    }break;
                    // Face him to doors
                    case 1:
                    {
                        GetUnit()->SetFacing(1.33f);
                    }break;
                    // Preparing to cast spell
                    case 2:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ASHCROMBE_OPEN_DOOR);
                    }break;
                    // Casting unlock spell and calling next events every 6 seconds
                    case 3:
                    {
                        ModifyAIUpdateEvent(6000);
                        GetUnit()->CastSpell(GetUnit(), SPELL_ASHCROMBE_UNLOCK, false);
                    }break;
                    // Setting instance data to finished
                    case 4:
                    {
                        GetUnit()->SendScriptTextChatMessage(SAY_ASHCROMBE_BYE);
                    }break;
                    // Final stage - casting spell which despawns Ashcrombe Sorcerer
                    case 5:
                    {
                        GetUnit()->CastSpell(GetUnit(), SPELL_ASHCROMBE_FIRE, true);
                        GetUnit()->SendScriptTextChatMessage(SAY_ASHCROMBE_VANISH);
                        SFK_instance->SetInstanceData(0, INDEX_PRISONER_EVENT, State_Finished);
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
            GetUnit()->SendScriptTextChatMessage(SAY_ASHCROMBE_BOSS_DEATH);
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
    void OnHello(Object* pObject, Player* plr)
    {
        Arcemu::Gossip::Menu menu(pObject->GetGUID(), sMySQLStore.getGossipTextIdForNpc(pObject->GetEntry()));

        ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript());
        if (pInstance != nullptr && pInstance->GetInstanceData(0, INDEX_RETHILGORE) == State_Finished && pInstance->GetInstanceData(0, INDEX_PRISONER_EVENT) == State_NotStarted)
        {
            menu.AddItem(GOSSIP_ICON_CHAT, plr->GetSession()->LocalizedGossipOption(prisonerGossipOptionID), 1);
        }
        menu.Send(plr);
    }

    void OnSelectOption(Object* pObject, Player* plr, uint32 Id, const char* Code, uint32 gossipId)
    {
        if (Id == 1)
        {
            if (AshcrombeAI* pPrisoner = static_cast<AshcrombeAI*>(static_cast<Creature*>(pObject)->GetScript()))
            {
                pPrisoner->GetUnit()->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                pPrisoner->GetUnit()->SendScriptTextChatMessage(SAY_ASHCROMBE_FOLLOW);
                pPrisoner->RegisterAIUpdateEvent(4000);
                pPrisoner->GetUnit()->setUInt64Value(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                pPrisoner->GetUnit()->GetAIInterface()->SetAllowedToEnterCombat(false);
                pPrisoner->GetUnit()->Emote(EMOTE_ONESHOT_POINT);
                pPrisoner->eventStarted = true;
                if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->GetMapMgr()->GetScript()))
                    pInstance->SetInstanceData(0, INDEX_PRISONER_EVENT, State_InProgress);
            }
        }
        Arcemu::Gossip::Menu::Complete(plr);
    }
};

///////////////////////////////////////////////////////////////////
/// Main bosses events
///////////////////////////////////////////////////////////////////

// Creature entry: 4278
class SpringvaleAI : public MoonScriptCreatureAI
{
    enum SpringvaleSpells
    {
        SPELL_DEVO_AURA            = 643,
        SPELL_HOLY_LIGHT           = 1026,
        SPELL_HAMMER_OF_JUSTICE    = 5588,
        SPELL_DIVINE_PROT          = 13007
    };
public:
    MOONSCRIPT_FACTORY_FUNCTION(SpringvaleAI, MoonScriptCreatureAI);
    SpringvaleAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_HOLY_LIGHT, Target_WoundedFriendly, 10, 2.5f, 0);
        DevoAura = AddSpell(SPELL_DEVO_AURA, Target_Self, 0, 0, 0);
        DivineProt = AddSpell(SPELL_DIVINE_PROT, Target_Self, 0, 0, 0);
        HammerOfJustice = AddSpell(SPELL_HAMMER_OF_JUSTICE, Target_Current, 12, 0, 60);
    }

    void OnCombatStart(Unit* pTarget)
    {
        ParentClass::OnCombatStart(pTarget);
        // Turn aura ON!
        if (!GetUnit()->HasAura(SPELL_DEVO_AURA))
            CastSpellNowNoScheduling(DevoAura);
    }

    void OnCombatStop(Unit* pTarget)
    {
        ParentClass::OnCombatStop(pTarget);
        // Turn aura OFF!
        if (GetUnit()->HasAura(SPELL_DEVO_AURA))
            RemoveAura(SPELL_DEVO_AURA);
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (GetHealthPercent() <= 20 && DivineProt->mEnabled)
        {
            CastSpellNowNoScheduling(DivineProt);
            DivineProt->mEnabled = false;
        }
    }

protected:
    SpellDesc* DevoAura;
    SpellDesc* DivineProt;
    SpellDesc* HolyLight;
    SpellDesc* HammerOfJustice;
};

// Creature entry: 3914

class RethilgoreAI : public MoonScriptCreatureAI
{
    const uint32 SPELL_SOUL_DRAIN = 7295;
public:
    MOONSCRIPT_FACTORY_FUNCTION(RethilgoreAI, MoonScriptCreatureAI);
    RethilgoreAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_SOUL_DRAIN, Target_RandomPlayer, 8.0f, 2, 10);
    }
};

// Creature entry: 3927

class NandosAI : public MoonScriptCreatureAI
{
    enum NandosAISpells : uint32
    {
        SPELL_CALL_BLEAK_WORG        = 7487,
        SPELL_CALL_SLAVERING_WORG    = 7488,
        SPELL_CALLLUPINE_HORROR      = 7489
    };
public:
    MOONSCRIPT_FACTORY_FUNCTION(NandosAI, MoonScriptCreatureAI);
    NandosAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());
        sCallBleakWord = AddSpell(SPELL_CALL_BLEAK_WORG, Target_Self, 0, 1.3f, 0);
        sCallSlaveringWorg = AddSpell(SPELL_CALL_SLAVERING_WORG, Target_Self, 0, 1.3f, 0);
        sCallLupineHorror = AddSpell(SPELL_CALLLUPINE_HORROR, Target_Self, 0, 1.3f, 0);
        Reset();
    }

    void Reset()
    {
        sCallBleakWorg_Timer        = -1;
        sCallSlaveringWorg_Timer    = -1;
        sCallLupineHorror_Timer     = -1;
    }

    void OnCombatStart(Unit* mEnemy)
    {
        ParentClass::OnCombatStart(mEnemy);
        sCallBleakWorg_Timer = AddTimer(RandomUInt(1) ? 33700 : 48800);
        sCallSlaveringWorg_Timer = AddTimer(RandomUInt(1) ? 45400 : 51700);
        sCallLupineHorror_Timer = AddTimer(69500);
        if (SFK_instance)
            SFK_instance->SetInstanceData(0, INDEX_NANDOS, State_InProgress);
    }

    void OnCombatStop(Unit* mEnemy)
    {
        ParentClass::OnCombatStop(mEnemy);
        // Battle has failed
        if (SFK_instance)
            SFK_instance->SetInstanceData(0, INDEX_NANDOS, State_InvalidState);

        Reset();
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (GetHealthPercent() <= 80)
        {
            if (IsTimerFinished(sCallBleakWorg_Timer) && sCallBleakWord->mEnabled)
            {
                CastSpell(sCallBleakWord);
                RemoveTimer(sCallBleakWorg_Timer);
                sCallBleakWord->mEnabled = false;
            }

            if (IsTimerFinished(sCallSlaveringWorg_Timer) && sCallSlaveringWorg->mEnabled)
            {
                CastSpell(sCallSlaveringWorg);
                RemoveTimer(sCallSlaveringWorg_Timer);
                sCallSlaveringWorg->mEnabled = false;
            }

            if (IsTimerFinished(sCallLupineHorror_Timer) && sCallLupineHorror->mEnabled)
            {
                CastSpell(sCallLupineHorror);
                RemoveTimer(sCallLupineHorror_Timer);
                sCallLupineHorror->mEnabled = false;
            }
        }
    }

protected:
    ShadowfangKeepInstance* SFK_instance;
    int32 sCallBleakWorg_Timer;
    int32 sCallSlaveringWorg_Timer;
    int32 sCallLupineHorror_Timer;
    SpellDesc* sCallBleakWord;
    SpellDesc* sCallSlaveringWorg;
    SpellDesc* sCallLupineHorror;
};

// Creature entry: 3887

class BaronSilverlaineAI : public MoonScriptBossAI
{
    const uint32 SPELL_VEIL_OF_SHADOW = 7068;
public:
    MOONSCRIPT_FACTORY_FUNCTION(BaronSilverlaineAI, MoonScriptBossAI);
    BaronSilverlaineAI(Creature* pCreature) : MoonScriptBossAI(pCreature)
    {
        AddSpell(SPELL_VEIL_OF_SHADOW, Target_Current, 10.0f, 0, 2);
    }
};

// Creature entry: 4279

class BlindWatcherAI : public MoonScriptCreatureAI
{
    enum ODO_THE_BLINDWATCHER_SPELLS : uint32
    {
        ODO_HOWLING_RAGE1 = 7481,
        ODO_HOWLING_RAGE2 = 7483,
        ODO_HOWLING_RAGE3 = 7484
    };
public:
    MOONSCRIPT_FACTORY_FUNCTION(BlindWatcherAI, MoonScriptCreatureAI);
    BlindWatcherAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature), mPhase(0)
    {
        // Howling Rage 1
        HowlingRage1 = AddSpell(ODO_HOWLING_RAGE1, Target_Self, 0, 5, 0);
        // Howling Rage 2
        HowlingRage2 = AddSpell(ODO_HOWLING_RAGE2, Target_Self, 0, 1.5f, 0);
        // Howling Rage 3
        HowlingRage3 = AddSpell(ODO_HOWLING_RAGE3, Target_Self, 0, 1.5f, 0);
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (GetHealthPercent() <= 75 && !GetUnit()->HasAura(ODO_HOWLING_RAGE1) && mPhase == 0)
        {
            CastSpell(HowlingRage1);
            ++mPhase;
        }
        else if (GetHealthPercent() <= 45 && !GetUnit()->HasAura(ODO_HOWLING_RAGE2) && mPhase == 1)
        {
            CastSpell(HowlingRage2);
            ++mPhase;
        }
        else if (GetHealthPercent() <= 20 && !GetUnit()->HasAura(ODO_HOWLING_RAGE2) && mPhase == 2)
        {
            CastSpell(HowlingRage3);
            ++mPhase;
        }
    }

protected:
    SpellDesc* HowlingRage1;
    SpellDesc* HowlingRage2;
    SpellDesc* HowlingRage3;
    uint8 mPhase;
};

// Creature entry: 4274

class FenrusAI : public MoonScriptCreatureAI
{
    const uint32 SPELL_TOXIC_SALIVA = 7125;
public:
    MOONSCRIPT_FACTORY_FUNCTION(FenrusAI, MoonScriptCreatureAI);
    FenrusAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_TOXIC_SALIVA, Target_Current, 12, 1.5f, 60);
    }
};

// Creature entry: 4275

class ArugalBossAI : public MoonScriptCreatureAI
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
public:
    MOONSCRIPT_FACTORY_FUNCTION(ArugalBossAI, MoonScriptCreatureAI);
    ArugalBossAI(Creature* pCreature) :
        MoonScriptCreatureAI(pCreature),
        stage(0),
        arugalPosition(ARUGAL_LOC_LEDGE)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->GetMapMgr()->GetScript());

        sVoidBolt = AddSpell(SPELL_VOID_BOLT, Target_Current, 0, 3, 1);
        AddSpell(SPELL_THUNDER_SHOCK, Target_Self, 10.0f, 0, 0, 0, 5.0f);
        AddSpell(SPELL_ARUGALS_CURSE, Target_RandomPlayer, 5.0f, 0, 0);

        AddEmote(Event_OnCombatStart, YELL_ARUGAL_AGROO);
        AddEmote(Event_OnTargetDied, YELL_ARUGAL_ENEMY_DEATH);
        AddEmote(Event_OnTaunt, YELL_ARUGAL_COMBAT);
        SetBehavior(Behavior_Spell);

        aiUpdateOriginal = GetAIUpdateFreq();
        originalRegen = GetUnit()->PctPowerRegenModifier[POWER_TYPE_MANA];
    }

    void OnCastSpell(uint32 spellId)
    {
        if (spellId == SPELL_ARUGALS_CURSE)
        {
            GetUnit()->SendScriptTextChatMessage(YELL_ARUGAL_COMBAT);
        }
    }

    void Reset()
    {
        SetBehavior(Behavior_Spell);
        GetUnit()->GetAIInterface()->disable_melee = true;
        GetUnit()->PctPowerRegenModifier[POWER_TYPE_MANA] = originalRegen;
    }

    void OnCombatStart(Unit* pEnemy)
    {
        ParentClass::OnCombatStart(pEnemy);

        // do not regen mana
        GetUnit()->PctPowerRegenModifier[POWER_TYPE_MANA] = 0.3f;
        aiUpdateOriginal = GetAIUpdateFreq();
        originalRegen = GetUnit()->PctPowerRegenModifier[POWER_TYPE_MANA];

        // Do not do melee attacks
        GetUnit()->GetAIInterface()->disable_melee = true;
    }

    void OnCombatStop(Unit* pEnemy)
    {
        ParentClass::OnCombatStop(pEnemy);
        Reset();
    }

    void FenrusEvent(uint32 pStage)
    {
        switch (pStage)
        {
            case 0:
            {
                ModifyAIUpdateEvent(6000);
                ApplyAura(SPELL_ARUGAL_SPAWN);
                GetUnit()->SendScriptTextChatMessage(YELL_ARUGAL_FENRUS);
            }break;
            case 1:
            {
                if (GameObject* pGO = GetNearestGameObject(GO_ARUGAL_FOCUS))
                {
                    pGO->SetState(GO_STATE_OPEN);
                }

                // Spawn Arugal's Voidwalkers
                for (uint8 x = 0; x < ArugalVoidCount; x++)
                {
                    if (MoonScriptCreatureAI* voidwalker = SpawnCreature(CN_VOIDWALKER, voidwalkerSpawns[x].x, voidwalkerSpawns[x].y, voidwalkerSpawns[x].z, voidwalkerSpawns[x].o))
                    {
                        voidwalker->Despawn(4 * 60 * 1000); // Despawn in 4 mins
                        voidwalker->AggroNearestPlayer();
                    }
                }
                GetUnit()->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9);
                GetUnit()->GetAIInterface()->SetAllowedToEnterCombat(true);
                GetUnit()->GetAIInterface()->m_canMove = true;

                // sanctum32: not sure if it is correct spell id
                GetUnit()->CastSpell(GetUnit(), SPELL_ASHCROMBE_FIRE, true);
                SFK_instance->SetInstanceData(0, INDEX_VOIDWALKER, State_Finished);
                RemoveAIUpdateEvent();
            }break;
        }
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (SFK_instance && SFK_instance->GetInstanceData(0, INDEX_VOIDWALKER) == State_InProgress)
        {
            FenrusEvent(stage);
            ++stage;
        }

        if (IsInCombat())
        {
            // if mana is out - do melee attacks
            if (GetUnit()->GetManaPct() <= 10 && GetBehavior() == Behavior_Spell)
            {
                SetBehavior(Behavior_Melee);
                GetUnit()->GetAIInterface()->disable_melee = false;
            }
            // boss got mana regenerated
            else
            {
                SetBehavior(Behavior_Spell);
                GetUnit()->GetAIInterface()->disable_melee = true;
            }

            // Cast void bolt non stop
            if (GetBehavior() == Behavior_Spell)
            {
                CastSpellNowNoScheduling(sVoidBolt);
            }

            if (GetHealthPercent() <= 25)
            {
                if (arugalPosition == ARUGAL_LOC_UPPER_LEDGE)
                {
                    ModifyAIUpdateEvent(aiUpdateOriginal);
                    GetUnit()->CastSpell(GetUnit(), SPELL_SHADOW_PORT_STAIRS, true);
                    arugalPosition = ARUGAL_LOC_STAIRS;
                    SetCanMove(true);
                }

                if (arugalPosition == ARUGAL_LOC_LEDGE)
                {
                    aiUpdateOriginal = GetAIUpdateFreq();
                    ModifyAIUpdateEvent(3000);
                    GetUnit()->CastSpell(GetUnit(), SPELL_SHADOW_PORT_UPPER_LEDGE, true);
                    arugalPosition = ARUGAL_LOC_UPPER_LEDGE;
                    SetCanMove(false);
                }
            }
        }
    }

protected:
    uint8 stage;
    uint8 arugalPosition;
    ShadowfangKeepInstance* SFK_instance;
    SpellDesc* sVoidBolt;
    uint32 aiUpdateOriginal;
    float originalRegen;
};

// Creature entry: 3886

class RazorclawTheButcherAI : public MoonScriptCreatureAI
{
    const uint32 SPELL_BUTCHER_DRAIN = 7485;
public:
    MOONSCRIPT_FACTORY_FUNCTION(RazorclawTheButcherAI, MoonScriptCreatureAI);
    RazorclawTheButcherAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_BUTCHER_DRAIN, Target_RandomPlayer, 5.0f, 0, 0);
    }
};

///////////////////////////////////////////////////////////////////
/// Trash npcs
///////////////////////////////////////////////////////////////////

// Creature entry: 3866

class VileBatAI : public MoonScriptCreatureAI
{
    enum VileBatSpells : uint32
    {
        SPELL_DIVING_SWEEP  = 7145,
        SPELL_DISARM        = 6713
    };
public:
    MOONSCRIPT_FACTORY_FUNCTION(VileBatAI, MoonScriptCreatureAI);
    VileBatAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        AddSpell(SPELL_DIVING_SWEEP, Target_Current, 8.0f, 0, 0);
        AddSpell(SPELL_DISARM, Target_Current, 5.0f, 0, 6);
    }
};

// Creature entry: 3868

class BloodSeekerAI : public MoonScriptCreatureAI
{
    const uint32 SPELL_EXPOSE_WEAKNESS = 7140;
public:
    MOONSCRIPT_FACTORY_FUNCTION(BloodSeekerAI, MoonScriptCreatureAI);
    BloodSeekerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Expose Weakness
        AddSpell(SPELL_EXPOSE_WEAKNESS, Target_Current, 5.0f, 0, 5);
    }
};

// Creature entry: 4627

class VoidWalkerAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(VoidWalkerAI, MoonScriptCreatureAI);
    VoidWalkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Dark Offering
        AddSpell(7154, Target_WoundedFriendly, 5.0f, 0, 7);
    }
};

// Creature entry: 3861

class BleakWorgAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(BleakWorgAI, MoonScriptCreatureAI);
    BleakWorgAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Wavering Will
        AddSpell(7127, Target_RandomPlayer, 5.0f, 0, 60);
    }
};

// Creature entry: 3863

class LupineHorrorAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(LupineHorrorAI, MoonScriptCreatureAI);
    LupineHorrorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Summon Lupine Delusions
        AddSpell(7132, Target_Self, 5.0f, 0, 4 * 60);
    }
};

// Creature entry: 2529

class SonOfArugalAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(SonOfArugalAI, MoonScriptCreatureAI);
    SonOfArugalAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Arugal's Gift
        AddSpell(7124, Target_Current, 5.0f, 2.5f, 0);
    }
};

// Creature entry: 3853

class ShadowfangMoonwalkerAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(ShadowfangMoonwalkerAI, MoonScriptCreatureAI);
    ShadowfangMoonwalkerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Anti-Magic Shield
        AddSpell(7121, Target_Self, 5.0f, 2.0f, 10);
    }
};

// Creature entry: 3855

class ShadowfangDarksoulAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(ShadowfangDarksoulAI, MoonScriptCreatureAI);
    ShadowfangDarksoulAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Befuddlement
        AddSpell(8140, Target_RandomPlayer, 8.0f, 0, 15);

        // Shadow Word : Pain
        AddSpell(970, Target_RandomPlayer, 5.0f, 0, 18);
    }
};

// Creature entry: 3857

class ShadowfangGluttonAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(ShadowfangGluttonAI, MoonScriptCreatureAI);
    ShadowfangGluttonAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Blood Tap
        AddSpell(7122, Target_Current, 5.0f, 0, 0);
    }
};

// Creature entry: 3859

class ShadowfangRagetoothAI : public MoonScriptCreatureAI
{
    const uint32 SPELL_WILD_RAGE = 7072;
public:
    MOONSCRIPT_FACTORY_FUNCTION(ShadowfangRagetoothAI, MoonScriptCreatureAI);
    ShadowfangRagetoothAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature), sWildRageCasted(false)
    {
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        // Cast Wild rage at 30% health
        if (GetHealthPercent() <= 30 && !GetUnit()->HasAura(SPELL_WILD_RAGE) && !sWildRageCasted)
        {
            GetUnit()->CastSpell(GetUnit(), SPELL_WILD_RAGE, true);
            sWildRageCasted = true;
        }
    }

protected:
    SpellDesc* sWildRage;
    bool sWildRageCasted;
};

// Creature entry: 3864

class FelSteedAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(FelSteedAI, MoonScriptCreatureAI);
    FelSteedAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Fel Stomp
        AddSpell(7139, Target_Current, 5.0f, 0, 3);
    }
};

// Creature entry: 3872

class DeathswornCaptainAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(DeathswornCaptainAI, MoonScriptCreatureAI);
    DeathswornCaptainAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Hamstring
        AddSpell(9080, Target_Current, 5.0f, 0, 10);

        // Cleave
        AddSpell(40505, Target_Current, 8.0f, 0, 10);
    }
};

// Creature entry: 3873

class TormentedOfficerAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(TormentedOfficerAI, MoonScriptCreatureAI);
    TormentedOfficerAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Forsaken Skills (TODO: implement dummy aura of this spell)
        AddSpell(7054, Target_Current, 5.0f, 2.0f, 300);
    }
};

// Creature entry: 3875

class HauntedServitorAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(HauntedServitorAI, MoonScriptCreatureAI);
    HauntedServitorAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Haunting Spirits (TODO: implement dummy aura of this spell)
        AddSpell(7057, Target_Current, 5.0f, 2.0f, 300);
    }
};

// Creature entry: 3877

class WaillingGuardsmanAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(WaillingGuardsmanAI, MoonScriptCreatureAI);
    WaillingGuardsmanAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
        // Screams of the Past
        AddSpell(7074, Target_Self, 5.0f, 0, 5);
    }
};

// Creature entry: 3877
class WorlfguardWorgAI : public MoonScriptCreatureAI
{
public:
    MOONSCRIPT_FACTORY_FUNCTION(WorlfguardWorgAI, MoonScriptCreatureAI);
    WorlfguardWorgAI(Creature* pCreature) : MoonScriptCreatureAI(pCreature)
    {
    }

    void AIUpdate()
    {
        ParentClass::AIUpdate();
        if (GetHealthPercent() <= 15 && GetBehavior() != Behavior_Flee)
        {
            SetBehavior(Behavior_Flee);
        }
    }
};

///////////////////////////////////////////////////////////////////
/// Spells used by creatures in Shadowfang keep dungeon
///////////////////////////////////////////////////////////////////

// Spell entry: 6421
bool ashrombeUnlockDummySpell(uint32 i, Spell* pSpell)
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
bool ashrombeTeleportDummyAura(uint32 /*i*/, Aura* pAura, bool /*apply*/)
{
    Unit* target = pAura->GetUnitCaster();
    if (!target || !target->IsCreature())
    {
        return false;
    }

    if (Creature* creatureCaster = static_cast<Creature*>(target))
    {
        creatureCaster->Despawn(3000, 0);
        return true;
    }
    return false;
}

void SetupShadowfangKeep(ScriptMgr* mgr)
{
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
}
