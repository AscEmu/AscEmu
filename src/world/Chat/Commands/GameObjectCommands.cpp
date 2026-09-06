/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Management/SpawnManager.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <cctype>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

namespace
{
    SpawnManager* getSpawnManager(GameObject* gameObject)
    {
        if (!gameObject)
            return nullptr;

        WorldMap* map = gameObject->getWorldMap();
        return map ? &map->getSpawnManager() : nullptr;
    }

    bool isPersistentGameObject(GameObject* gameObject)
    {
        if (SpawnManager* spawnManager = getSpawnManager(gameObject))
            return spawnManager->isPersistentSpawn(gameObject->getGuid());
        return false;
    }

    void syncGameObjectSpawn(GameObject* gameObject, bool updateHomePosition = false)
    {
        if (SpawnManager* spawnManager = getSpawnManager(gameObject))
            spawnManager->syncGameObjectSpawn(gameObject, updateHomePosition);
    }

    bool prepareGameObjectClientRecreate(GameObject* gameObject, Player* player, WorldMap*& map, SpawnManager*& spawnManager)
    {
        if (!gameObject || !player || !gameObject->IsInWorld())
            return false;

        map = gameObject->getWorldMap();
        if (!map)
            return false;

        spawnManager = &map->getSpawnManager();
        const uint64_t oldGuid = gameObject->getGuid();

        gameObject->RemoveFromWorld(false);

        if (!spawnManager->regenerateGameObjectGuid(gameObject))
        {
            gameObject->PushToWorld(map);
            player->setSelectedGo(oldGuid);
            return false;
        }

        player->setSelectedGo(gameObject->getGuid());
        return true;
    }
}

//.gobject damage
bool ChatCommandHandler::HandleGODamageCommand(const char* args, WorldSession* session)
{
    uint32_t damage = 0;
    uint32_t spellid = 0;

    if (sscanf(args, "%u %u", &damage, &spellid) < 1)
    {
        if (damage == 0)
        {
            redSystemMessage(session, "You need to specify how much you want to damage the selected GO!");
            return true;
        }
    }

    auto gameobject = session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(session, "You need to select a GO first!");
        return true;
    }

    if (gameobject->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        redSystemMessage(session, "The selected GO must be a destructible building!");
        return true;
    }

    if (spellid == 0)
        spellid = 57609;

    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameobject);
    if (dgo->GetHP() == 0)
    {
        redSystemMessage(session, "Cannot further damage a destroyed GameObject");
        return true;
    }

    uint64_t guid = session->GetPlayer()->getGuid();
    dgo->Damage(damage, guid, 0, spellid);

    greenSystemMessage(session, "GameObject has been damaged for {} hitpoints", damage);
    greenSystemMessage(session, "New hitpoints {}", dgo->GetHP());

    return true;
}

//.gobject delete
bool ChatCommandHandler::HandleGODeleteCommand(const char* /*args*/, WorldSession* m_session)
{
    GameObject* selected_gobject = m_session->GetPlayer()->getSelectedGo();
    if (selected_gobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    if (selected_gobject->IsInBg())
    {
        redSystemMessage(m_session, "GameObjects can't be deleted in Battlegrounds");
        return true;
    }

    WorldMap* map = selected_gobject->getWorldMap();
    if (!map)
        return true;

    SpawnManager& spawnManager = map->getSpawnManager();
    const uint32_t spawnId = selected_gobject->getSpawnId();
    const bool persistent = spawnManager.isPersistentSpawn(selected_gobject->getGuid());

    sGMLog.writefromsession(
        m_session,
        "Deleted game object entry {} spawnId {} persistent {} on map {} at X:{} Y:{} Z:{} (name: {}).",
        selected_gobject->getEntry(),
        spawnId,
        persistent ? 1u : 0u,
        selected_gobject->GetMapId(),
        selected_gobject->GetPositionX(),
        selected_gobject->GetPositionY(),
        selected_gobject->GetPositionZ(),
        sMySQLStore.getGameObjectProperties(selected_gobject->getEntry())->name);

    // Only persistent spawns have a DB row.
    if (persistent)
        selected_gobject->deleteFromDB();

    spawnManager.eraseGameObjectSpawnBySpawnID(spawnId);
    spawnManager.despawn(selected_gobject, 0);

    m_session->GetPlayer()->setSelectedGo(0);
    return true;
}

//.gobject enable
bool ChatCommandHandler::HandleGOEnableCommand(const char* /*args*/, WorldSession* m_session)
{
    GameObject* gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    if (gameobject->isActive())
    {
        // Deactivate
        gameobject->setDynamicFlags(GO_DYN_FLAG_NONE);
        blueSystemMessage(m_session, "Gameobject deactivated.");
    }
    else
    {
        // Activate
        gameobject->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        blueSystemMessage(m_session, "Gameobject activated.");
    }

    sGMLog.writefromsession(m_session, "Activated/deactivated gameobject {}, entry {}.", sMySQLStore.getGameObjectProperties(gameobject->getEntry())->name, gameobject->getEntry());

    return true;
}

//.gobject export
bool ChatCommandHandler::HandleGOExportCommand(const char* args, WorldSession* m_session)
{
    if (!m_session->GetPlayer()->getSelectedGo())
        return false;

    GameObject* gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
        return false;

    std::stringstream name;
    if (*args)
    {
        name << "GO_" << args << ".sql";
    }
    else
    {
        name << "GO_" << gameobject->getEntry() << ".sql";
    }

    gameobject->SaveToFile(name);

    blueSystemMessage(m_session, "Go saved to: {}", name.str());

    return true;
}

//.gobject info
bool ChatCommandHandler::HandleGOInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (!gameobject)
    {
        redSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    systemMessage(m_session, "======== {} Information =======", MSG_COLOR_SUBWHITE);
    const bool persistent = isPersistentGameObject(gameobject);
    systemMessage(m_session, "{} SpawnID:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getSpawnId());
    systemMessage(m_session, "{} Spawn type:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, persistent ? "persistent" : "ephemeral");
    systemMessage(m_session, "{} Entry:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getEntry());
    systemMessage(m_session, "{} GUID:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getGuidLow());
    systemMessage(m_session, "{} Model:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getDisplayId());
    systemMessage(m_session, "{} State:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getState());
    systemMessage(m_session, "{} flags:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getFlags());
    systemMessage(m_session, "{} dynflags:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getDynamicFlags());
    systemMessage(m_session, "{} faction:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getFactionTemplate());
    systemMessage(m_session, "{} phase:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetPhase());

    std::string gotypetxt;
    switch (gameobject->getGoType())
    {
        case GAMEOBJECT_TYPE_DOOR:
            gotypetxt = "Door";
            break;
        case GAMEOBJECT_TYPE_BUTTON:
            gotypetxt = "Button";
            break;
        case GAMEOBJECT_TYPE_QUESTGIVER:
            gotypetxt = "Quest Giver";
            break;
        case GAMEOBJECT_TYPE_CHEST:
            gotypetxt = "Chest";
            break;
        case GAMEOBJECT_TYPE_BINDER:
            gotypetxt = "Binder";
            break;
        case GAMEOBJECT_TYPE_GENERIC:
            gotypetxt = "Generic";
            break;
        case GAMEOBJECT_TYPE_TRAP:
            gotypetxt = "Trap";
            break;
        case GAMEOBJECT_TYPE_CHAIR:
            gotypetxt = "Chair";
            break;
        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            gotypetxt = "Spell Focus";
            break;
        case GAMEOBJECT_TYPE_TEXT:
            gotypetxt = "Text";
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            gotypetxt = "Goober";
            break;
        case GAMEOBJECT_TYPE_TRANSPORT:
            gotypetxt = "Transport";
            break;
        case GAMEOBJECT_TYPE_AREADAMAGE:
            gotypetxt = "Area Damage";
            break;
        case GAMEOBJECT_TYPE_CAMERA:
            gotypetxt = "Camera";
            break;
        case GAMEOBJECT_TYPE_MAP_OBJECT:
            gotypetxt = "Map Object";
            break;
        case GAMEOBJECT_TYPE_MO_TRANSPORT:
            gotypetxt = "Mo Transport";
            break;
        case GAMEOBJECT_TYPE_DUEL_ARBITER:
            gotypetxt = "Duel Arbiter";
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            gotypetxt = "Fishing Node";
            break;
        case GAMEOBJECT_TYPE_RITUAL:
            gotypetxt = "Ritual";
            break;
        case GAMEOBJECT_TYPE_MAILBOX:
            gotypetxt = "Mailbox";
            break;
        case GAMEOBJECT_TYPE_AUCTIONHOUSE:
            gotypetxt = "Auction House";
            break;
        case GAMEOBJECT_TYPE_GUARDPOST:
            gotypetxt = "Guard Post";
            break;
        case GAMEOBJECT_TYPE_SPELLCASTER:
            gotypetxt = "Spell Caster";
            break;
        case GAMEOBJECT_TYPE_MEETINGSTONE:
            gotypetxt = "Meeting Stone";
            break;
        case GAMEOBJECT_TYPE_FLAGSTAND:
            gotypetxt = "Flag Stand";
            break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
            gotypetxt = "Fishing Hole";
            break;
        case GAMEOBJECT_TYPE_FLAGDROP:
            gotypetxt = "Flag Drop";
            break;
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            gotypetxt = "Destructible Building";
            break;
        default:
            gotypetxt = "Unknown.";
            break;
    }
    systemMessage(m_session, "{} Type:{}{} -- {}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getGoType(), gotypetxt);

    systemMessage(m_session, "{} Distance:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->CalcDistance(m_session->GetPlayer()));

    GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(gameobject->getEntry());
    if (!gameobject_info)
    {
        redSystemMessage(m_session, "This GameObject doesn't have template, you won't be able to get some information nor to spawn a GO with this entry.");
        return true;
    }


    systemMessage(m_session, "{} Name:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject_info->name);

    systemMessage(m_session, "{} Size:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getScale());
    systemMessage(m_session, "{} X:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetPositionX());
    systemMessage(m_session, "{} Y:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetPositionY());
    systemMessage(m_session, "{} Z:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetPositionZ());
    systemMessage(m_session, "{} Orientation:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->GetOrientation());
    systemMessage(m_session, "{} Rotation 0:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getParentRotation(0));
    systemMessage(m_session, "{} Rotation 1:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getParentRotation(1));
    systemMessage(m_session, "{} Rotation 2:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getParentRotation(2));
    systemMessage(m_session, "{} Rotation 3:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->getParentRotation(3));

    GameObject_Destructible* dgo = dynamic_cast<GameObject_Destructible*>(gameobject);
    if (gameobject_info->type == GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        systemMessage(m_session, "{} HP:{}{}/{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, dgo->GetHP(), dgo->GetMaxHP());
    }

    systemMessage(m_session, "=================================");

    if (persistent)
        systemMessage(m_session, "Persistent DB-backed spawn.");
    else
        systemMessage(m_session, "Ephemeral runtime spawn.");

    return true;
}

//.gobject movehere
bool ChatCommandHandler::HandleGOMoveHereCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* player = m_session->GetPlayer();
    GameObject* gameobject = player->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    const float position_x = player->GetPositionX();
    const float position_y = player->GetPositionY();
    const float position_z = player->GetPositionZ();
    const float position_o = gameobject->GetOrientation();

    WorldMap* map = nullptr;
    SpawnManager* spawnManager = nullptr;
    if (!prepareGameObjectClientRecreate(gameobject, player, map, spawnManager))
    {
        redSystemMessage(m_session, "Failed to recreate selected GameObject.");
        return true;
    }

    gameobject->SetPosition(LocationVector(position_x, position_y, position_z, position_o));
    spawnManager->syncGameObjectSpawn(gameobject, true);
    gameobject->PushToWorld(map);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = spawnManager->isPersistentSpawn(gameobject->getGuid());

    if (persistent)
    {
        gameobject->saveToDB();
        greenSystemMessage(m_session, "GameObject spawn ID {} moved and saved.", spawnId);
    }
    else
    {
        greenSystemMessage(m_session, "Ephemeral GameObject spawn ID {} moved.", spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject position of spawn ID: %u persistent %u.",
        spawnId,
        persistent ? 1u : 0u);

    return true;
}

//.gobject open
bool ChatCommandHandler::HandleGOOpenCommand(const char* /*args*/, WorldSession* m_session)
{
    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    if (gameobject->getState() != GO_STATE_OPEN)
    {
        gameobject->setState(GO_STATE_OPEN);
        blueSystemMessage(m_session, "Gameobject opened.");
    }
    else
    {
        gameobject->setState(GO_STATE_CLOSED);
        blueSystemMessage(m_session, "Gameobject closed.");
    }

    return true;
}

//.gobject rebuild
bool ChatCommandHandler::HandleGORebuildCommand(const char* /*args*/, WorldSession* session)
{
    auto gameobject = session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(session, "You need to select a GO first!");
        return true;
    }

    if (gameobject->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        redSystemMessage(session, "The selected GO must be a destructible building!");
        return true;
    }

    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameobject);

    uint32_t oldHitPoints = dgo->GetHP();

    dgo->Rebuild();

    blueSystemMessage(session, "GameObject has been rebuilt.");
    greenSystemMessage(session, "Old hitpoints: {} New hitpoints {}", oldHitPoints, dgo->GetHP());

    return true;
}

//.gobject rotate
bool ChatCommandHandler::HandleGORotateCommand(const char* args, WorldSession* m_session)
{
    char Axis;
    float deg;
    if (sscanf(args, "%c %f", &Axis, &deg) < 1)
        return false;

    Player* player = m_session->GetPlayer();
    GameObject* go = player->getSelectedGo();
    if (!go)
    {
        redSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    const float rotation_x = player->m_goLastXRotation;
    const float rotation_y = player->m_goLastYRotation;
    const float orientation = go->GetOrientation();

    if (tolower(Axis) != 'x' && tolower(Axis) != 'y' && tolower(Axis) != 'o')
    {
        redSystemMessage(m_session, "Invalid Axis, Please use x, y, or o.");
        return true;
    }

    WorldMap* map = nullptr;
    SpawnManager* spawnManager = nullptr;
    if (!prepareGameObjectClientRecreate(go, player, map, spawnManager))
    {
        redSystemMessage(m_session, "Failed to recreate selected GameObject.");
        return true;
    }

    switch (tolower(Axis))
    {
        case 'x':
            go->setLocalRotationAngles(orientation, rotation_y, deg);
            player->m_goLastXRotation = deg;
            break;
        case 'y':
            go->setLocalRotationAngles(orientation, deg, rotation_x);
            player->m_goLastYRotation = deg;
            break;
        case 'o':
            go->SetOrientation(player->GetOrientation());
            go->setLocalRotationAngles(go->GetOrientation(), rotation_y, rotation_x);
            break;
    }

    spawnManager->syncGameObjectSpawn(go);
    go->PushToWorld(map);

    if (spawnManager->isPersistentSpawn(go->getGuid()))
    {
        go->saveToDB();
        greenSystemMessage(m_session, "GameObject spawn ID {} rotated and saved.", go->getSpawnId());
    }
    else
    {
        greenSystemMessage(m_session, "Ephemeral GameObject spawn ID {} rotated.", go->getSpawnId());
    }

    return true;
}

//.gobject select
bool ChatCommandHandler::HandleGOSelectCommand(const char* args, WorldSession* m_session)
{
    GameObject* GObj = nullptr;
    GameObject* GObjs = m_session->GetPlayer()->getSelectedGo();

    float cDist = 9999.0f;
    float nDist = 0.0f;
    bool bUseNext = false;

    if (args)
    {
        if (args[0] == '1')
        {
            if (GObjs == nullptr)
                bUseNext = true;

            for (const auto& Itr : m_session->GetPlayer()->getInRangeObjectsSet())
            {
                if (Itr && Itr->isGameObject() && Itr->GetPhase() == m_session->GetPlayer()->GetPhase())
                {
                    // Find the current go, move to the next one
                    if (bUseNext)
                    {
                        // Select the first.
                        GObj = static_cast<GameObject*>(Itr);
                        break;
                    }

                    if (Itr == GObjs)
                    {
                        // Found him. Move to the next one, or beginning if we're at the end
                        bUseNext = true;
                    }
                }
            }
        }
    }
    if (!GObj)
    {
        for (const auto& Itr : m_session->GetPlayer()->getInRangeObjectsSet())
        {
            if (Itr && Itr->isGameObject() && Itr->GetPhase() == m_session->GetPlayer()->GetPhase())
            {
                if ((nDist = m_session->GetPlayer()->CalcDistance(Itr)) < cDist)
                {
                    cDist = nDist;
                    nDist = 0.0f;
                    GObj = static_cast<GameObject*>(Itr);
                }
            }
        }
    }


    if (GObj == nullptr)
    {
        redSystemMessage(m_session, "No inrange GameObject found.");
        return true;
    }

    m_session->GetPlayer()->setSelectedGo(GObj->getGuid());

    //reset last rotation values on selecting a new go.
    m_session->GetPlayer()->m_goLastXRotation = 0.0f;
    m_session->GetPlayer()->m_goLastYRotation = 0.0f;

    auto* props = sMySQLStore.getGameObjectProperties(GObj->getEntry());
    std::string goName = props ? props->name : "Unknown GameObject";

    greenSystemMessage(m_session, "Selected GameObject [ {} ] which is {} meters away from you.",
                       goName, m_session->GetPlayer()->CalcDistance(GObj));

    return true;
}

//.gobject selectguid
bool ChatCommandHandler::HandleGOSelectGuidCommand(const char* args, WorldSession* m_session)
{
    uint32_t guid = 0;
    auto [ptr, ec] = std::from_chars(args, args + std::strlen(args), guid);

    if (ec != std::errc{})
        return false;

    Player* player = m_session->GetPlayer();
    auto gameobject = player->getWorldMapGameObject(guid);
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No GameObject found with guid {}", guid);
        return true;
    }

    player->setSelectedGo(gameobject->getGuid());

    auto* props = gameobject->GetGameObjectProperties();
    std::string goName = props ? props->name : "Unknown GameObject";

    greenSystemMessage(
        m_session,
        "GameObject [ {} ] with distance {} to your position selected.",
        goName,
        player->CalcDistance(gameobject));

    return true;
}

//.gobject spawn
bool ChatCommandHandler::HandleGOSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32_t entry = 0;
    uint32_t persistent = 0;
    if (!args || sscanf(args, "%u %u", &entry, &persistent) < 1 || entry == 0 || persistent > 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject spawn <entry> [persistent 0|1]");
        return true;
    }

    auto const* gameObjectProperties = sMySQLStore.getGameObjectProperties(entry);
    if (gameObjectProperties == nullptr)
    {
        redSystemMessage(m_session, "GameObject entry {} is an invalid entry!", entry);
        return true;
    }

    Player* player = m_session->GetPlayer();
    if (!player || !player->getWorldMap())
        return true;

    GameObject* gameObject = player->getWorldMap()->getSpawnManager().spawnGameObject(
        entry,
        player->GetPosition());

    if (gameObject == nullptr)
    {
        redSystemMessage(m_session, "Failed to spawn GameObject with entry {}.", entry);
        return true;
    }

    const uint32_t ephemeralSpawnId = gameObject->getSpawnId();

    if (persistent != 0)
        gameObject->saveToDB(true);

    greenSystemMessage(
        m_session,
        "Spawned GameObject `{}` entry {} spawnId {}{}.",
        gameObjectProperties->name,
        entry,
        gameObject->getSpawnId(),
        persistent != 0 ? " (persistent)" : " (ephemeral)");

    if (persistent != 0 && ephemeralSpawnId != gameObject->getSpawnId())
    {
        systemMessage(
            m_session,
            "Ephemeral spawnId {} migrated to DB spawnId {}.",
            ephemeralSpawnId,
            gameObject->getSpawnId());
    }

    sGMLog.writefromsession(
        m_session,
        "spawned gameobject %s entry %u spawnId %u persistent %u at map %u %f %f %f",
        gameObjectProperties->name.c_str(),
        gameObject->getEntry(),
        gameObject->getSpawnId(),
        persistent,
        player->GetMapId(),
        gameObject->GetPositionX(),
        gameObject->GetPositionY(),
        gameObject->GetPositionZ());

    player->setSelectedGo(gameObject->getGuid());

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .gobject set commands
//.gobject set animprogress
bool ChatCommandHandler::HandleGOSetAnimProgressCommand(const char* args, WorldSession* m_session)
{
    uint32_t animprogress;

    if (sscanf(args, "%u", &animprogress) != 1)
    {
        redSystemMessage(m_session, "You need to define the animprogress value!");
        redSystemMessage(m_session, ".gobject setanimprogress <animprogress>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    gameobject->setAnimationProgress(static_cast<uint8_t>(animprogress));
    greenSystemMessage(m_session, "Gameobject animprogress set to {}", animprogress);

    return true;
}

//.gobject set faction
bool ChatCommandHandler::HandleGOSetFactionCommand(const char* args, WorldSession* m_session)
{
    uint32_t go_faction = 0;
    if (sscanf(args, "%u", &go_faction) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject setfaction <faction>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No GameObject is selected.");
        return true;
    }

    auto faction_template = sFactionTemplateStore.lookupEntry(go_faction);
    if (faction_template == nullptr)
    {
        redSystemMessage(m_session, "The entered faction is invalid! Use a valid faction id.");
        return false;
    }

    gameobject->SetFaction(go_faction);
    syncGameObjectSpawn(gameobject);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = isPersistentGameObject(gameobject);

    if (persistent)
    {
        WorldDatabase.execute(
            "REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)",
            spawnId,
            VERSION_STRING,
            VERSION_STRING,
            gameobject->getScale(),
            go_faction,
            gameobject->getFlags());

        greenSystemMessage(
            m_session,
            "Faction changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "Faction changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject faction of spawn ID: %u persistent %u.",
        spawnId,
        persistent ? 1u : 0u);

    return true;
}

//.gobject set flags
bool ChatCommandHandler::HandleGOSetFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32_t go_flags;

    if (sscanf(args, "%u", &go_flags) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject setflags <flags>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No GameObject selected!");
        return true;
    }

    gameobject->setFlags(go_flags);
    syncGameObjectSpawn(gameobject);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = isPersistentGameObject(gameobject);

    if (persistent)
    {
        WorldDatabase.execute(
            "REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)",
            spawnId,
            VERSION_STRING,
            VERSION_STRING,
            gameobject->getScale(),
            gameobject->getFactionTemplate(),
            go_flags);

        greenSystemMessage(
            m_session,
            "Flags changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "Flags changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject flags of spawn ID: %u persistent %u.",
        spawnId,
        persistent ? 1u : 0u);

    return true;
}

//.gobject set overrides
bool ChatCommandHandler::HandleGOSetOverridesCommand(const char* args, WorldSession* m_session)
{
    uint32_t go_override;
    if (sscanf(args, "%u", &go_override) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject setoverride <value>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    Player* player = m_session->GetPlayer();
    WorldMap* map = nullptr;
    SpawnManager* spawnManager = nullptr;
    if (!prepareGameObjectClientRecreate(gameobject, player, map, spawnManager))
    {
        redSystemMessage(m_session, "Failed to recreate selected GameObject.");
        return true;
    }

    gameobject->SetOverrides(go_override);
    spawnManager->syncGameObjectSpawn(gameobject);
    gameobject->PushToWorld(map);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = spawnManager->isPersistentSpawn(gameobject->getGuid());

    if (persistent)
    {
        WorldDatabase.execute(
            "UPDATE gameobject_spawns SET overrides = %u WHERE id = %u AND min_build <= %u AND max_build >= %u",
            go_override,
            spawnId,
            VERSION_STRING,
            VERSION_STRING);

        greenSystemMessage(
            m_session,
            "Overrides changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "Overrides changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject overrides of spawn ID: %u to %u persistent %u",
        spawnId,
        go_override,
        persistent ? 1u : 0u);

    return true;
}

//.gobject set phase
bool ChatCommandHandler::HandleGOSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32_t phase;

    if (sscanf(args, "%u", &phase) < 1)
    {
        redSystemMessage(m_session, "You need to define the phase value!");
        redSystemMessage(m_session, ".gobject setphase <phase>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    Player* player = m_session->GetPlayer();
    WorldMap* map = nullptr;
    SpawnManager* spawnManager = nullptr;
    if (!prepareGameObjectClientRecreate(gameobject, player, map, spawnManager))
    {
        redSystemMessage(m_session, "Failed to recreate selected GameObject.");
        return true;
    }

    gameobject->Phase(PHASE_SET, phase);
    spawnManager->syncGameObjectSpawn(gameobject);
    gameobject->PushToWorld(map);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = spawnManager->isPersistentSpawn(gameobject->getGuid());

    if (persistent)
    {
        WorldDatabase.execute(
            "UPDATE gameobject_spawns SET phase = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u",
            phase,
            spawnId,
            VERSION_STRING,
            VERSION_STRING);

        greenSystemMessage(
            m_session,
            "Phase changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "Phase changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject phase of spawn ID: %u to %u persistent %u",
        spawnId,
        phase,
        persistent ? 1u : 0u);

    return true;
}

//.gobject set scale
bool ChatCommandHandler::HandleGOSetScaleCommand(const char* args, WorldSession* m_session)
{
    float scale = 0.0f;
    if (sscanf(args, "%f", &scale) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject setscale <scale>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    Player* player = m_session->GetPlayer();
    WorldMap* map = nullptr;
    SpawnManager* spawnManager = nullptr;
    if (!prepareGameObjectClientRecreate(gameobject, player, map, spawnManager))
    {
        redSystemMessage(m_session, "Failed to recreate selected GameObject.");
        return true;
    }

    gameobject->setScale(scale);
    spawnManager->syncGameObjectSpawn(gameobject);
    gameobject->PushToWorld(map);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = spawnManager->isPersistentSpawn(gameobject->getGuid());

    if (persistent)
    {
        WorldDatabase.execute(
            "REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)",
            spawnId,
            VERSION_STRING,
            VERSION_STRING,
            scale,
            gameobject->getFactionTemplate(),
            gameobject->getFlags());

        greenSystemMessage(
            m_session,
            "Scale changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "Scale changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject scale of spawn ID: %u to %3.3lf persistent %u",
        spawnId,
        scale,
        persistent ? 1u : 0u);

    return true;
}

//.gobject set state
bool ChatCommandHandler::HandleGOSetStateCommand(const char* args, WorldSession* m_session)
{
    uint32_t go_state;

    if (sscanf(args, "%u", &go_state) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject setstate <state>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No GameObject selected!");
        return true;
    }

    gameobject->setState(static_cast<uint8_t>(go_state));
    syncGameObjectSpawn(gameobject);

    const uint32_t spawnId = gameobject->getSpawnId();
    const bool persistent = isPersistentGameObject(gameobject);

    if (persistent)
    {
        WorldDatabase.execute(
            "UPDATE gameobject_spawns SET state = %u WHERE id = %u AND min_build <= %u AND max_build >= %u",
            go_state,
            spawnId,
            VERSION_STRING,
            VERSION_STRING);

        greenSystemMessage(
            m_session,
            "State changed and saved for GameObject spawn ID {}.",
            spawnId);
    }
    else
    {
        greenSystemMessage(
            m_session,
            "State changed for ephemeral GameObject spawn ID {}.",
            spawnId);
    }

    sGMLog.writefromsession(
        m_session,
        "changed gameobject state of spawn ID: %u persistent %u.",
        spawnId,
        persistent ? 1u : 0u);

    return true;
}
