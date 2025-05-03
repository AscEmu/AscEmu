/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Setup.h"
#include "Instance_ShadowfangKeep.h"

#include "Management/Gossip/GossipMenu.hpp"
#include "Management/Gossip/GossipScript.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Objects/GameObject.h"
#include "Server/Script/CreatureAIScript.hpp"
#include "Server/Script/InstanceScript.hpp"
#include "Spell/Spell.hpp"
#include "Spell/SpellAura.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Random.hpp"

// Instance script for map 33 (Shadowfang Keep)
class ShadowfangKeepInstance : public InstanceScript
{
    // Gameobjects low guids
    uint32_t go_leftCell_GUID;
    uint32_t go_middleCell_GUID;
    uint32_t go_rightCell_GUID;
    uint32_t go_arugalsLair_GUID;
    uint32_t go_sorcererGate_GUID;
    uint32_t go_leftCellLever_GUID;
    uint32_t go_middleCellLever_GUID;
    uint32_t go_rightCellLever_GUID;
    uint32_t go_courtyarDoor_GUID;

    // Creatures low guids
    uint32_t npc_ashcrombe_GUID;
    uint32_t npc_adamant_GUID;

    // Nandos event related
    std::list<uint32_t /*guid*/> nandos_summons;

    // Encounters data
    uint32_t m_encounterData[ShadowfangKeep::INDEX_MAX];

public:
    explicit ShadowfangKeepInstance(WorldMap* pMapMgr) : InstanceScript(pMapMgr),

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

    static InstanceScript* Create(WorldMap* pMapMgr) { return new ShadowfangKeepInstance(pMapMgr); }

    void SetLocaleInstanceData(uint32_t /*pType*/, uint32_t pIndex, uint32_t pData)
    {
        if (pIndex >= ShadowfangKeep::INDEX_MAX)
            return;

        switch (pIndex)
        {
            case ShadowfangKeep::INDEX_VOIDWALKER:
            {
                if (pData == InProgress)
                {
                    if (Creature* ArugalSpawn = spawnCreature(ShadowfangKeep::CN_ARUGAL_BOSS, ShadowfangKeep::ArugalAtFenrusLoc.x, ShadowfangKeep::ArugalAtFenrusLoc.y, ShadowfangKeep::ArugalAtFenrusLoc.z, ShadowfangKeep::ArugalAtFenrusLoc.o))
                    {
                        ArugalSpawn->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                        ArugalSpawn->getAIInterface()->setAllowedToEnterCombat(false);
                        ArugalSpawn->setControlled(true, UNIT_STATE_ROOTED);
                        if (ArugalSpawn->GetScript())
                        {
                            ArugalSpawn->GetScript()->RegisterAIUpdateEvent(500);
                        }
                    }
                }
            } break;
            case ShadowfangKeep::INDEX_NANDOS:
            {
                // Despawn all summons on fail or on boos death
                if (pData == InvalidState || pData == Performed)
                {
                    for (std::list<uint32_t>::iterator itr = nandos_summons.begin(); itr != nandos_summons.end();)
                    {
                        if (Creature* pCreature = getInstance()->getCreature(*itr))
                        {
                            pCreature->Despawn(1000, 0);
                        }
                        itr = nandos_summons.erase(itr);
                    }
                    // Despawn creatures
                }

                if (pData == Performed)
                {
                    GameObject* pGate = GetGameObjectByGuid(go_arugalsLair_GUID);
                    if (pGate != nullptr && pGate->getState() == GO_STATE_CLOSED)
                    {
                        pGate->setState(GO_STATE_OPEN);
                    }
                }
            }break;
            case ShadowfangKeep::INDEX_RETHILGORE:
            {
                // Add gossip flag to prisoners
                if (pData == Performed)
                {
                    // Set gossip flags for both prisoners and push texts
                    if (Creature* pCreature = getInstance()->getCreature(npc_adamant_GUID))
                    {
                        if (pCreature->isAlive())
                        {
                            pCreature->addNpcFlags(UNIT_NPC_FLAG_GOSSIP);
                            pCreature->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_BOSS_DEATH);
                        }
                    }

                    if (Creature* pCreature = getInstance()->getCreature(npc_ashcrombe_GUID))
                    {
                        if (pCreature->isAlive())
                        {
                            pCreature->addNpcFlags(UNIT_NPC_FLAG_GOSSIP);
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
                        pGO->removeFlags(GO_FLAG_NONSELECTABLE);
                    }

                    if (GameObject* pGO = GetGameObjectByGuid(go_middleCellLever_GUID))
                    {
                        pGO->removeFlags(GO_FLAG_NONSELECTABLE);
                    }

                    if (GameObject* pGO = GetGameObjectByGuid(go_rightCellLever_GUID))
                    {
                        pGO->removeFlags(GO_FLAG_NONSELECTABLE);
                    }
                }
            } break;
            case ShadowfangKeep::INDEX_PRISONER_EVENT:
            {
                // Open doors in any case
                if (pData == Performed)
                {
                    if (GameObject* pGO = GetGameObjectByGuid(go_courtyarDoor_GUID))
                    {
                        if (pGO->getState() != GO_STATE_OPEN)
                            pGO->setState(GO_STATE_OPEN);
                    }
                }
            }break;
            case ShadowfangKeep::INDEX_FENRUS:
            {
                if (pData == Performed)
                {
                    SetLocaleInstanceData(0, ShadowfangKeep::INDEX_VOIDWALKER, InProgress);
                    GameObject* pGate = GetGameObjectByGuid(go_sorcererGate_GUID);
                    if (pGate != nullptr && pGate->getState() == GO_STATE_CLOSED)
                    {
                        pGate->setState(GO_STATE_OPEN);
                    }
                }
            }break;
            default:
                break;
        }

        m_encounterData[pIndex] = pData;
    }

    uint32_t GetInstanceData(uint32_t /*pType*/, uint32_t pIndex)
    {
        return pIndex >= ShadowfangKeep::INDEX_MAX ? 0 : m_encounterData[pIndex];
    }

    // Objects handling
    void OnGameObjectPushToWorld(GameObject* pGameObject) override
    {
        switch (pGameObject->getEntry())
        {
            case ShadowfangKeep::GO_LEFT_CELL:
            {
                go_leftCell_GUID = pGameObject->getGuidLow();
            }break;
            case ShadowfangKeep::GO_MIDDLE_CELL:
            {
                go_middleCell_GUID = pGameObject->getGuidLow();
            }break;
            case ShadowfangKeep::GO_RIGHT_CELL:
            {
                go_rightCell_GUID = pGameObject->getGuidLow();
            }break;
            case ShadowfangKeep::GO_ARUGALS_LAIR_GATE:
            {
                go_arugalsLair_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_NANDOS) == Performed && pGameObject->getState() == GO_STATE_CLOSED)
                {
                    pGameObject->setState(GO_STATE_OPEN);
                }
            }break;
            case ShadowfangKeep::GO_SORCERER_GATE:
            {
                go_sorcererGate_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_FENRUS) == Performed && pGameObject->getState() == GO_STATE_CLOSED)
                {
                    pGameObject->setState(GO_STATE_OPEN);
                }
            }break;
            case ShadowfangKeep::GO_LEFT_LEVER:
            {
                go_leftCellLever_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE) != Performed)
                {
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            case ShadowfangKeep::GO_RIGHT_LEVER:
            {
                go_rightCellLever_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE) != Performed)
                {
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            case ShadowfangKeep::GO_MIDDLE_LEVER:
            {
                go_middleCellLever_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE) != Performed)
                {
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            case ShadowfangKeep::GO_COURTYARD_DOOR:
            {
                go_courtyarDoor_GUID = pGameObject->getGuidLow();
                if (GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == Performed && pGameObject->getState() == GO_STATE_CLOSED)
                {
                    pGameObject->setState(GO_STATE_OPEN);
                }
            }break;
            default:
                break;
        }
    }

    void OnGameObjectActivate(GameObject* pGameObject, Player* /*pPlayer*/) override
    {
        switch (pGameObject->getEntry())
        {
            case ShadowfangKeep::GO_RIGHT_LEVER:
            {
                if (GameObject* pGO = GetGameObjectByGuid(go_rightCell_GUID))
                {
                    pGO->setState(pGO->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            case ShadowfangKeep::GO_MIDDLE_LEVER:
            {
                if (GameObject* pGO = GetGameObjectByGuid(go_middleCell_GUID))
                {
                    pGO->setState(pGO->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            case ShadowfangKeep::GO_LEFT_LEVER:
            {
                if (GameObject* pGO = GetGameObjectByGuid(go_leftCell_GUID))
                {
                    pGO->setState(pGO->getState() == GO_STATE_CLOSED ? GO_STATE_OPEN : GO_STATE_CLOSED);
                    pGameObject->setFlags(GO_FLAG_NONSELECTABLE);
                }
            }break;
            default:
                break;
        }
    }

    void OnCreatureDeath(Creature* pCreature, Unit* /*pKiller*/) override
    {
        switch (pCreature->getEntry())
        {
            case ShadowfangKeep::CN_NANDOS:
            {
                SetLocaleInstanceData(0, ShadowfangKeep::INDEX_NANDOS, Performed);
            }break;
            case ShadowfangKeep::CN_RETHILGORE:
            {
                SetLocaleInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE, Performed);
            }break;
            case ShadowfangKeep::CN_FENRUS:
            {
                SetLocaleInstanceData(0, ShadowfangKeep::INDEX_FENRUS, Performed);
            }break;
            default:
                break;
        }
    }

    void OnCreaturePushToWorld(Creature* pCreature) override
    {
        WoWGuid wowGuid;
        wowGuid.Init(pCreature->getGuid());

        switch (pCreature->getEntry())
        {
            case ShadowfangKeep::CN_ADAMANT:
            {
                npc_adamant_GUID = wowGuid.getGuidLowPart();
            }break;
            case ShadowfangKeep::CN_ASHCROMBE:
            {
                npc_ashcrombe_GUID = wowGuid.getGuidLowPart();
            }break;
            // Make him hidden
            case ShadowfangKeep::CN_ARUGAL:
            {
                pCreature->setVisible(false);
            }break;
            case ShadowfangKeep::CN_BLEAK_WORG:
            case ShadowfangKeep::CN_SLAVERING_WORG:
            case ShadowfangKeep::CN_LUPINE_HORROR:
            {
                // Add to nandos summon lists only on his event is started
                if (GetInstanceData(0, ShadowfangKeep::INDEX_NANDOS) == InProgress)
                {
                    pCreature->Despawn(60 * 4 * 1000, 0);   // Despawn in 4 mins
                    nandos_summons.push_back(wowGuid.getGuidLowPart());
                }
            }break;
            case ShadowfangKeep::CN_LUPINE_DELUSION:
            {
                // Add to nandos summon lists only on his event is started
                if (GetInstanceData(0, ShadowfangKeep::INDEX_NANDOS) == InProgress)
                {
                    nandos_summons.push_back(wowGuid.getGuidLowPart());
                }
                pCreature->Despawn(60 * 4 * 1000, 0); // Despawn in 4 mins
            }break;
            case ShadowfangKeep::CN_DEATHSTALKER_VINCENT:
            {
                if (GetInstanceData(0, ShadowfangKeep::INDEX_ARUGAL_INTRO) == Performed)
                {
                    // Make him look like dead
                    pCreature->setStandState(STANDSTATE_DEAD);
                    pCreature->setDeathState(CORPSE);
                    pCreature->setControlled(true, UNIT_STATE_ROOTED);
                    pCreature->addDynamicFlags(U_DYN_FLAG_DEAD);
                    pCreature->SendScriptTextChatMessage(ShadowfangKeep::SAY_VINCENT_DEATH);
                }
            }break;
            default:
                break;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Special npcs events

// Arugal intro event
// Creature entries: 10000, 4444

class ArugalAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArugalAI(c); }
    ArugalAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0)
    {
        SFK_Instance = static_cast<ShadowfangKeepInstance*>(pCreature->getWorldMap()->getScript());
        if (SFK_Instance && SFK_Instance->GetInstanceData(0, ShadowfangKeep::INDEX_ARUGAL_INTRO) == NotStarted)
        {
            RegisterAIUpdateEvent(500);
            SFK_Instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_ARUGAL_INTRO, InProgress);
        }
    }

    void AIUpdate() override
    {
        if (SFK_Instance && SFK_Instance->GetInstanceData(0, ShadowfangKeep::INDEX_ARUGAL_INTRO) == InProgress)
        {
            switch (stage)
            {
                case 0:
                {
                    getCreature()->setVisible(true);
                    getCreature()->castSpell(getCreature(), ShadowfangKeep::SPELL_ARUGAL_SPAWN, true);
                    ModifyAIUpdateEvent(5500);  // call every step after 5.5 seconds
                    if (Creature* pVincent = getNearestCreature(ShadowfangKeep::CN_DEATHSTALKER_VINCENT))
                    {
                        pVincent->getAIInterface()->onHostileAction(getCreature());
                        pVincent->getAIInterface()->setMeleeDisabled(true);
                    }
                }break;
                case 1:
                {
                    getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ARUGAL_INTRO1);
                }break;
                case 2:
                {
                    getCreature()->emote(EMOTE_ONESHOT_POINT);
                }break;
                case 3:
                {
                    getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ARUGAL_INTRO2);
                }break;
                case 4:
                {
                    getCreature()->emote(EMOTE_ONESHOT_EXCLAMATION);
                }break;
                case 5:
                {
                    getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ARUGAL_INTRO3);
                }break;
                case 6:
                {
                    getCreature()->emote(EMOTE_ONESHOT_LAUGH);
                }break;
                case 7:
                {
                    if (Creature* pVincent = getCreature()->getWorldMap()->getInterface()->getCreatureNearestCoords(getCreature()->GetPositionX(), getCreature()->GetPositionY(), getCreature()->GetPositionZ(), ShadowfangKeep::CN_DEATHSTALKER_VINCENT))
                    {
                        // Make him look like dead
                        pVincent->SendScriptTextChatMessage(ShadowfangKeep::SAY_VINCENT_DEATH);
                        pVincent->setStandState(STANDSTATE_DEAD);
                        pVincent->setDeathState(CORPSE);
                        pVincent->setControlled(true, UNIT_STATE_ROOTED);
                        pVincent->addDynamicFlags(U_DYN_FLAG_DEAD);
                    }
                }break;
                case 8:
                {
                    getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ARUGAL_INTRO4);
                }break;
                case 9:
                {
                    getCreature()->castSpell(getCreature(), ShadowfangKeep::SPELL_ARUGAL_SPAWN, true);
                    SFK_Instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_ARUGAL_INTRO, Performed);
                    getCreature()->setVisible(false);
                    RemoveAIUpdateEvent();
                }break;
            }
            ++stage;
        }
    }

protected:
    uint32_t stage;
    ShadowfangKeepInstance* SFK_Instance;
};

// Prisoner Adamant (entry: 3849) gossip, escort event

class AdamantAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AdamantAI(c); }
    AdamantAI(Creature* pCreature) : CreatureAIScript(pCreature), eventStarted(false)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(getCreature()->getWorldMap()->getScript());

        loadCustomWaypoins(1);

        // Remove Gossip
        pCreature->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        switch (iWaypointId)
        {
            case 30:
            {
                getCreature()->Despawn(2000, 0);
            }break;
        default:
            {
                setWaypointToMove(1, ++iWaypointId);
            }break;
        }
    }

    void DoAction(int32_t const action) override
    {
        switch (action)
        {
            case 0:
            {
                getCreature()->setEmoteState(EMOTE_STATE_NONE);
                setWaypointToMove(1, 1);
            }
            break;
            case 1:
            {
                scriptEvents.addEvent(1, 2000);
                scriptEvents.addEvent(2, 3000);
                scriptEvents.addEvent(3, 4000);
                scriptEvents.addEvent(4, 5000);
                scriptEvents.addEvent(5, 6000);
            }
            break;
        }
    }

    void AIUpdate() override
    {
        if (SFK_instance && (SFK_instance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == InProgress 
            || SFK_instance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == Performed))
        {
            if (eventStarted)
            {
                scriptEvents.updateEvents(GetAIUpdateFreq(), getScriptPhase());

                while (uint32_t eventId = scriptEvents.getFinishedEvent())
                {
                    switch (eventId)
                    {
                    case 1:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_BEFORE_OPEN);
                    }
                        break;
                    case 2:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_OPENING);
                        getCreature()->eventAddEmote(EMOTE_ONESHOT_USESTANDING, 8000);
                    }
                        break;
                    case 3:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_AFTER_OPEN);
                        SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT, Performed);
                    }
                        break;
                    case 4:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_BYE);
                    }
                        break;
                    case 5:
                    {
                        SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT, Performed);
                        setWaypointToMove(1, 12);  // Lets run
                    }
                        break;
                    } 
                }
            }
        }
    }

protected:
    ShadowfangKeepInstance* SFK_instance;

public:
    // This variable will be set to true on OnSelectOption gossip event
    bool eventStarted;
};

class AdamantGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        //TODO: correct text id
        GossipMenu menu(pObject->getGuid(), sMySQLStore.getGossipTextIdForNpc(pObject->getEntry()));

        ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->getWorldMap()->getScript());
        if (pInstance != nullptr && pInstance->GetInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE) == Performed && pInstance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == NotStarted)
        {
            //TODO: move this to database
            menu.addItem(GOSSIP_ICON_CHAT, ShadowfangKeep::prisonerGossipOptionID, 1);
        }
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        if (Id == 1)
        {
            if (AdamantAI* pPrisoner = static_cast<AdamantAI*>(static_cast<Creature*>(pObject)->GetScript()))
            {
                pPrisoner->getCreature()->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
                pPrisoner->getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ADAMANT_FOLLOW);
                pPrisoner->DoAction(0);
                pPrisoner->getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                pPrisoner->getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
                pPrisoner->getCreature()->eventAddEmote(EMOTE_ONESHOT_CHEER, 4000);
                pPrisoner->eventStarted = true;
                if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->getWorldMap()->getScript()))
                    pInstance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT, InProgress);
            }
        }
        GossipMenu::senGossipComplete(plr);
    }
};

// Prisoner Sorcerer Ashcrombe (entry: 3850) gossip, escort event
class AshcrombeAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new AshcrombeAI(c); }
    AshcrombeAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0), argued(false), eventStarted(false)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(getCreature()->getWorldMap()->getScript());

        for (uint8_t i = 0; i < ShadowfangKeep::ashcrombeWpCount; ++i)
        {
            float waitTime = 0;
            float distanceX = 0;
            float distanceY = 0;
            float distance = 0;
            float walkSpeed = getCreature()->GetCreatureProperties()->walk_speed;
            //float runSpeed = getCreature()->GetCreatureProperties()->run_speed;
            if (i == 0) // first waypoint
            {
                distanceX = (ShadowfangKeep::SorcererAshcrombeWPS[i].x - getCreature()->GetPositionX())*(ShadowfangKeep::SorcererAshcrombeWPS[i].x - getCreature()->GetPositionX());
                distanceY = (ShadowfangKeep::SorcererAshcrombeWPS[i].y - getCreature()->GetPositionY())*(ShadowfangKeep::SorcererAshcrombeWPS[i].y - getCreature()->GetPositionY());
                distance = std::sqrt(distanceX - distanceY);
            }
            else if (i != ShadowfangKeep::ashcrombeWpCount-1)
            {
                distanceX = (ShadowfangKeep::SorcererAshcrombeWPS[i].x - ShadowfangKeep::SorcererAshcrombeWPS[i-1].x)*(ShadowfangKeep::SorcererAshcrombeWPS[i].x - ShadowfangKeep::SorcererAshcrombeWPS[i-1].x);
                distanceY = (ShadowfangKeep::SorcererAshcrombeWPS[i].y - ShadowfangKeep::SorcererAshcrombeWPS[i-1].y)*(ShadowfangKeep::SorcererAshcrombeWPS[i].y - ShadowfangKeep::SorcererAshcrombeWPS[i-1].y);
                distance = std::sqrt(distanceX + distanceY);
            }
            waitTime = 300.0f + (1000 * std::abs(distance / walkSpeed));
            addWaypoint(1, createWaypoint(i + 1, static_cast<uint32_t>(waitTime), WAYPOINT_MOVE_TYPE_WALK, ShadowfangKeep::SorcererAshcrombeWPS[i]));
        }

        stage = 0;

        // Remove Gossip
        pCreature->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
    }

    void OnReachWP(uint32_t type, uint32_t iWaypointId) override
    {
        if (type != WAYPOINT_MOTION_TYPE)
            return;

        if (iWaypointId == 10)
        {
            // Do script update every 2s
            RegisterAIUpdateEvent(2000);
        }

        if (iWaypointId > 0 && iWaypointId < 11)
        {
            setWaypointToMove(1, ++iWaypointId);
        }
    }

    void AIUpdate() override
    {
        if (SFK_instance 
            && (SFK_instance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == InProgress
            || SFK_instance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == Performed))
        {
            if (eventStarted)
            {
                switch (stage)
                {
                    // Starting movement
                    case 0:
                    {
                        getCreature()->setEmoteState(EMOTE_STATE_NONE);
                        setWaypointToMove(1, 1);
                        RemoveAIUpdateEvent();
                    }break;
                    // Face him to doors
                    case 1:
                    {
                        getCreature()->setFacing(1.33f);
                    }break;
                    // Preparing to cast spell
                    case 2:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ASHCROMBE_OPEN_DOOR);
                    }break;
                    // Casting unlock spell and calling next events every 6 seconds
                    case 3:
                    {
                        ModifyAIUpdateEvent(6000);
                        getCreature()->castSpell(getCreature(), ShadowfangKeep::SPELL_ASHCROMBE_UNLOCK, false);
                    }break;
                    // Setting instance data to finished
                    case 4:
                    {
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ASHCROMBE_BYE);
                    }break;
                    // Final stage - casting spell which despawns Ashcrombe Sorcerer
                    case 5:
                    {
                        getCreature()->castSpell(getCreature(), ShadowfangKeep::SPELL_ASHCROMBE_FIRE, true);
                        getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ASHCROMBE_VANISH);
                        SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT, Performed);
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
            getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ASHCROMBE_BOSS_DEATH);
            RemoveAIUpdateEvent();
            argued = true;
        }
    }

protected:
    uint32_t stage;
    ShadowfangKeepInstance* SFK_instance;

    // Used to say text after Adamant, after boss kill
    bool argued;

public:
    // This variable will be set to true on OnSelectOption gossip event
    bool eventStarted;
};

class AshcrombeGossip : public GossipScript
{
public:
    void onHello(Object* pObject, Player* plr) override
    {
        GossipMenu menu(pObject->getGuid(), sMySQLStore.getGossipTextIdForNpc(pObject->getEntry()));

        ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->getWorldMap()->getScript());
        if (pInstance != nullptr && pInstance->GetInstanceData(0, ShadowfangKeep::INDEX_RETHILGORE) == Performed && pInstance->GetInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT) == NotStarted)
        {
            menu.addItem(GOSSIP_ICON_CHAT, ShadowfangKeep::prisonerGossipOptionID, 1);
        }
        menu.sendGossipPacket(plr);
    }

    void onSelectOption(Object* pObject, Player* plr, uint32_t Id, const char* /*Code*/, uint32_t /*gossipId*/) override
    {
        if (Id == 1)
        {
            if (AshcrombeAI* pPrisoner = static_cast<AshcrombeAI*>(static_cast<Creature*>(pObject)->GetScript()))
            {
                pPrisoner->getCreature()->removeNpcFlags(UNIT_NPC_FLAG_GOSSIP);
                pPrisoner->getCreature()->SendScriptTextChatMessage(ShadowfangKeep::SAY_ASHCROMBE_FOLLOW);
                pPrisoner->RegisterAIUpdateEvent(4000);
                pPrisoner->getCreature()->addUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                pPrisoner->getCreature()->getAIInterface()->setAllowedToEnterCombat(false);
                pPrisoner->getCreature()->emote(EMOTE_ONESHOT_POINT);
                pPrisoner->eventStarted = true;
                if (ShadowfangKeepInstance* pInstance = static_cast<ShadowfangKeepInstance*>(pObject->getWorldMap()->getScript()))
                    pInstance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_PRISONER_EVENT, InProgress);
            }
        }
        GossipMenu::senGossipComplete(plr);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Main bosses events

// Creature entry: 4278
class SpringvaleAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new SpringvaleAI(c); }

    enum SpringvaleSpells
    {
        SPELL_DEVO_AURA            = 643,
        SPELL_HOLY_LIGHT           = 1026,
        SPELL_HAMMER_OF_JUSTICE    = 5588,
        SPELL_DIVINE_PROT          = 13007
    };

    explicit SpringvaleAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
        if (!getCreature()->hasAurasWithId(SPELL_DEVO_AURA))
            _castAISpell(DevoAura);
    }

    void OnCombatStop(Unit* /*pTarget*/) override
    {
        // Turn aura OFF!
        if (getCreature()->hasAurasWithId(SPELL_DEVO_AURA))
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
public:
    static CreatureAIScript* Create(Creature* c) { return new RethilgoreAI(c); }
    explicit RethilgoreAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        const uint32_t SPELL_SOUL_DRAIN = 7295;
        addAISpell(SPELL_SOUL_DRAIN, 8.0f, TARGET_RANDOM_SINGLE, 2, 10);
    }
};

// Creature entry: 3927
class NandosAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new NandosAI(c); }

    enum NandosAISpells : uint32_t
    {
        SPELL_CALL_BLEAK_WORG        = 7487,
        SPELL_CALL_SLAVERING_WORG    = 7488,
        SPELL_CALLLUPINE_HORROR      = 7489
    };

    explicit NandosAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->getWorldMap()->getScript());

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
            SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_NANDOS, InProgress);
    }

    void OnCombatStop(Unit* /*mEnemy*/) override
    {
        // Battle has failed
        if (SFK_instance)
            SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_NANDOS, InvalidState);

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
    uint32_t sCallBleakWorg_Timer;
    uint32_t sCallSlaveringWorg_Timer;
    uint32_t sCallLupineHorror_Timer;

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
public:
    static CreatureAIScript* Create(Creature* c) { return new BaronSilverlaineAI(c); }

    const uint32_t SPELL_VEIL_OF_SHADOW = 7068;

    explicit BaronSilverlaineAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_VEIL_OF_SHADOW, 10.0f, TARGET_ATTACKING, 0, 2);
    }
};

// Creature entry: 4279
class BlindWatcherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new BlindWatcherAI(c); }

    enum ODO_THE_BLINDWATCHER_SPELLS : uint32_t
    {
        ODO_HOWLING_RAGE1 = 7481,
        ODO_HOWLING_RAGE2 = 7483,
        ODO_HOWLING_RAGE3 = 7484
    };

    explicit BlindWatcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
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
public:
    static CreatureAIScript* Create(Creature* c) { return new FenrusAI(c); }

    const uint32_t SPELL_TOXIC_SALIVA = 7125;

    explicit FenrusAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        addAISpell(SPELL_TOXIC_SALIVA, 12.0f, TARGET_ATTACKING, 2, 60);
    }
};

// Creature entry: 4275
class ArugalBossAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new ArugalBossAI(c); }

    enum Arugal_Boss_Spells : uint32_t
    {
        SPELL_VOID_BOLT                 = 7588,
        SPELL_SHADOW_PORT_UPPER_LEDGE   = 7587,
        //SPELL_SHADOW_PORT_SPAWN_LEDGE   = 7586,
        SPELL_SHADOW_PORT_STAIRS        = 7136,
        SPELL_THUNDER_SHOCK             = 7803,
        SPELL_ARUGALS_CURSE             = 7621
    };

    enum ArugalLoc : uint8_t
    {
        ARUGAL_LOC_LEDGE        = 0,    // Arugal's base spawn location
        ARUGAL_LOC_UPPER_LEDGE  = 1,    // Room corner
        ARUGAL_LOC_STAIRS       = 2     // Stairs
    };

    ArugalBossAI(Creature* pCreature) : CreatureAIScript(pCreature), stage(0), arugalPosition(ARUGAL_LOC_LEDGE)
    {
        SFK_instance = static_cast<ShadowfangKeepInstance*>(pCreature->getWorldMap()->getScript());

        sVoidBolt = addAISpell(SPELL_VOID_BOLT, 0.0f, TARGET_ATTACKING);

        addAISpell(SPELL_THUNDER_SHOCK, 10.0f, TARGET_SELF);
        addAISpell(SPELL_ARUGALS_CURSE, 5.0f, TARGET_RANDOM_SINGLE);

        addEmoteForEvent(Event_OnCombatStart, ShadowfangKeep::YELL_ARUGAL_AGROO);
        addEmoteForEvent(Event_OnTargetDied, ShadowfangKeep::YELL_ARUGAL_ENEMY_DEATH);
        addEmoteForEvent(Event_OnTaunt, ShadowfangKeep::YELL_ARUGAL_COMBAT);
        setAIAgent(AGENT_SPELL);

        aiUpdateOriginal = GetAIUpdateFreq();
    }

    void OnCastSpell(uint32_t spellId) override
    {
        if (spellId == SPELL_ARUGALS_CURSE)
        {
            getCreature()->SendScriptTextChatMessage(ShadowfangKeep::YELL_ARUGAL_COMBAT);
        }
    }

    void Reset()
    {
        setAIAgent(AGENT_SPELL);
        getCreature()->getAIInterface()->setMeleeDisabled(true);
        getCreature()->removeNpcFlags(UNIT_NPC_FLAG_DISABLE_PWREGEN);
    }

    void OnCombatStart(Unit* /*pEnemy*/) override
    {
        // do not regen mana
        getCreature()->addNpcFlags(UNIT_NPC_FLAG_DISABLE_PWREGEN);
        aiUpdateOriginal = GetAIUpdateFreq();

        // Do not do melee attacks
        getCreature()->getAIInterface()->setMeleeDisabled(true);
    }

    void OnCombatStop(Unit* /*pEnemy*/) override
    {
        Reset();
    }

    void FenrusEvent(uint32_t pStage)
    {
        switch (pStage)
        {
            case 0:
            {
                ModifyAIUpdateEvent(6000);
                _applyAura(ShadowfangKeep::SPELL_ARUGAL_SPAWN);
                getCreature()->SendScriptTextChatMessage(ShadowfangKeep::YELL_ARUGAL_FENRUS);
            }break;
            case 1:
            {
                if (GameObject* pGO = getNearestGameObject(ShadowfangKeep::GO_ARUGAL_FOCUS))
                {
                    pGO->setState(GO_STATE_OPEN);
                }

                // Spawn Arugal's Voidwalkers
                for (uint8_t x = 0; x < ShadowfangKeep::ArugalVoidCount; x++)
                {
                    if (CreatureAIScript* voidwalker = spawnCreatureAndGetAIScript(ShadowfangKeep::CN_VOIDWALKER, ShadowfangKeep::voidwalkerSpawns[x].x, ShadowfangKeep::voidwalkerSpawns[x].y, ShadowfangKeep::voidwalkerSpawns[x].z, ShadowfangKeep::voidwalkerSpawns[x].o))
                    {
                        voidwalker->despawn(4 * 60 * 1000); // Despawn in 4 mins
                    }
                }
                getCreature()->removeUnitFlags(UNIT_FLAG_IGNORE_PLAYER_COMBAT);
                getCreature()->getAIInterface()->setAllowedToEnterCombat(true);
                getCreature()->setControlled(false, UNIT_STATE_ROOTED);

                // sanctum32: not sure if it is correct spell id
                getCreature()->castSpell(getCreature(), ShadowfangKeep::SPELL_ASHCROMBE_FIRE, true);
                SFK_instance->SetLocaleInstanceData(0, ShadowfangKeep::INDEX_VOIDWALKER, Performed);
                RemoveAIUpdateEvent();
            }break;
        }
    }

    void AIUpdate() override
    {
        if (SFK_instance && SFK_instance->GetInstanceData(0, ShadowfangKeep::INDEX_VOIDWALKER) == InProgress)
        {
            FenrusEvent(stage);
            ++stage;
        }

        if (_isInCombat())
        {
            // if mana is out - do melee attacks
            if (getCreature()->getPowerPct(POWER_TYPE_MANA) <= 10 && getAIAgent() == AGENT_SPELL)
            {
                setAIAgent(AGENT_MELEE);
                getCreature()->getAIInterface()->setMeleeDisabled(false);
            }
            // boss got mana regenerated
            else
            {
                setAIAgent(AGENT_SPELL);
                getCreature()->getAIInterface()->setMeleeDisabled(true);
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
                    getCreature()->castSpell(getCreature(), SPELL_SHADOW_PORT_STAIRS, true);
                    arugalPosition = ARUGAL_LOC_STAIRS;
                    setRooted(false);
                }

                if (arugalPosition == ARUGAL_LOC_LEDGE)
                {
                    aiUpdateOriginal = GetAIUpdateFreq();
                    ModifyAIUpdateEvent(3000);
                    getCreature()->castSpell(getCreature(), SPELL_SHADOW_PORT_UPPER_LEDGE, true);
                    arugalPosition = ARUGAL_LOC_UPPER_LEDGE;
                    setRooted(true);
                }
            }
        }
    }

protected:
    uint8_t stage;
    uint8_t arugalPosition;
    ShadowfangKeepInstance* SFK_instance;
    CreatureAISpells* sVoidBolt;

    uint32_t aiUpdateOriginal;
};

// Creature entry: 3886
class RazorclawTheButcherAI : public CreatureAIScript
{
public:
    static CreatureAIScript* Create(Creature* c) { return new RazorclawTheButcherAI(c); }
    explicit RazorclawTheButcherAI(Creature* pCreature) : CreatureAIScript(pCreature)
    {
        const uint32_t SPELL_BUTCHER_DRAIN = 7485;
        addAISpell(SPELL_BUTCHER_DRAIN, 5.0f, TARGET_RANDOM_SINGLE);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////
// Spells used by creatures in Shadowfang keep dungeon

// Spell entry: 6421
bool ashrombeUnlockDummySpell(uint8_t /*effectIndex*/, Spell* pSpell)
{
    Unit* target = pSpell->getUnitCaster();
    if (!target)
    {
        return false;
    }

    if (GameObject* pGameObject = target->getWorldMap()->getInterface()->getGameObjectNearestCoords(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), ShadowfangKeep::GO_COURTYARD_DOOR))
    {
        pGameObject->setState(GO_STATE_OPEN);
        return true;
    }

    return false;
}

// Spell entry: 6422
bool ashrombeTeleportDummyAura(uint8_t /*effectIndex*/, Aura* pAura, bool /*apply*/)
{
    Unit* target = pAura->GetUnitCaster();
    if (!target || !target->isCreature())
        return false;

    Creature* creatureCaster = static_cast<Creature*>(target);
    creatureCaster->Despawn(3000, 0);
    return true;
}

void SetupShadowfangKeep(ScriptMgr* mgr)
{
    // Map
    mgr->register_instance_script(MAP_SHADOWFANG_KEEP, &ShadowfangKeepInstance::Create);

    // Arugal (intro event)
    mgr->register_creature_script(ShadowfangKeep::CN_ARUGAL, &ArugalAI::Create);

    // Prisoners
    mgr->register_creature_gossip(ShadowfangKeep::CN_ADAMANT, new AdamantGossip());
    mgr->register_creature_script(ShadowfangKeep::CN_ADAMANT, &AdamantAI::Create);
    mgr->register_creature_gossip(ShadowfangKeep::CN_ASHCROMBE, new AshcrombeGossip());
    mgr->register_creature_script(ShadowfangKeep::CN_ASHCROMBE, &AshcrombeAI::Create);

    // Bosses
    mgr->register_creature_script(ShadowfangKeep::CN_RETHILGORE, &RethilgoreAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_NANDOS, &NandosAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_BARON_SILVERLAINE, &BaronSilverlaineAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_FENRUS, &FenrusAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_ARUGAL_BOSS, &ArugalBossAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_SPRINGVALE, &SpringvaleAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_BLINDWATCHER, &BlindWatcherAI::Create);
    mgr->register_creature_script(ShadowfangKeep::CN_RAZORCLAW_THE_BUTCHER, &RazorclawTheButcherAI::Create);

    // Spells
    mgr->register_dummy_spell(ShadowfangKeep::SPELL_ASHCROMBE_UNLOCK, &ashrombeUnlockDummySpell);
    mgr->register_dummy_aura(ShadowfangKeep::SPELL_ASHCROMBE_FIRE, &ashrombeTeleportDummyAura);
}
