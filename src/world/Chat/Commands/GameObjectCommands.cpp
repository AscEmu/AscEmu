/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"

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

    if (selected_gobject->m_spawn != nullptr && selected_gobject->m_spawn->entry == selected_gobject->getEntry())
    {
        uint32_t cellx = uint32_t(((Map::Terrain::_maxX - selected_gobject->m_spawn->spawnPoint.x) / Map::Cell::cellSize));
        uint32_t celly = uint32_t(((Map::Terrain::_maxY - selected_gobject->m_spawn->spawnPoint.y) / Map::Cell::cellSize));

        if (cellx < Map::Cell::_sizeX && celly < Map::Cell::_sizeY)
        {
            CellSpawns* cell_spawns = selected_gobject->getWorldMap()->getBaseMap()->getSpawnsList(cellx, celly);
            if (cell_spawns != nullptr)
            {
                for (GameobjectSpawnList::iterator itr = cell_spawns->GameobjectSpawns.begin(); itr != cell_spawns->GameobjectSpawns.end(); ++itr)
                {
                    if ((*itr) == selected_gobject->m_spawn)
                    {
                        cell_spawns->GameobjectSpawns.erase(itr);
                        break;
                    }
                }
            }

            selected_gobject->deleteFromDB();

            delete selected_gobject->m_spawn;
            selected_gobject->m_spawn = nullptr;
        }
    }
    sGMLog.writefromsession(m_session, "deleted game object entry %u on map %u at X:%f Y:%f Z:%f Name %s", selected_gobject->getEntry(),
        selected_gobject->GetMapId(), selected_gobject->GetPositionX(), selected_gobject->GetPositionY(), selected_gobject->GetPositionZ(),
        sMySQLStore.getGameObjectProperties(selected_gobject->getEntry())->name.c_str());
    selected_gobject->despawn(0, 0);

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

    if (gameobject->IsActive())
    {
        // Deactivate
        gameobject->setDynamicFlags(GO_DYN_FLAG_NONE);
        blueSystemMessage(m_session, "Gameobject deactivated.");
    }
    else
    {
        // /Activate
        gameobject->setDynamicFlags(GO_DYN_FLAG_INTERACTABLE);
        blueSystemMessage(m_session, "Gameobject activated.");
    }

    sGMLog.writefromsession(m_session, "activated/deactivated gameobject %s, entry %u", sMySQLStore.getGameObjectProperties(gameobject->getEntry())->name.c_str(), gameobject->getEntry());

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
    systemMessage(m_session, "{} SpawnID:{}{}", MSG_COLOR_GREEN, MSG_COLOR_LIGHTBLUE, gameobject->m_spawn != nullptr ? gameobject->m_spawn->id : 0);
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

    if (gameobject->m_spawn != nullptr)
        systemMessage(m_session, "Is part of table: {}", gameobject->m_spawn->origine);
    else
        systemMessage(m_session, "Is spawnd by an internal script");

    return true;
}

//.gobject movehere
bool ChatCommandHandler::HandleGOMoveHereCommand(const char* /*args*/, WorldSession* m_session)
{
    auto gameobject = m_session->GetPlayer()->getSelectedGo();
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    float position_x = m_session->GetPlayer()->GetPositionX();
    float position_y = m_session->GetPlayer()->GetPositionY();
    float position_z = m_session->GetPlayer()->GetPositionZ();
    float position_o = gameobject->GetOrientation();

    gameobject->SetPosition(position_x, position_y, position_z, position_o);
    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }

    greenSystemMessage(m_session, "Position changed in gameobject_spawns table for spawn ID: {}.", go_spawn->id);
    WorldDatabase.Execute("UPDATE gameobject_spawns SET position_x = %f, position_y = %f, position_z = %f WHERE id = %u AND min_build <= %u AND max_build >= %u", position_x, position_y, position_z, go_spawn->id, VERSION_STRING, VERSION_STRING);
    sGMLog.writefromsession(m_session, "changed gameobject position of gameobject_spawns ID: %u.", go_spawn->id);

    uint32_t new_go_guid = m_session->GetPlayer()->getWorldMap()->generateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->getWorldMap());

    m_session->GetPlayer()->setSelectedGo(new_go_guid);

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

    GameObject* go = m_session->GetPlayer()->getSelectedGo();
    if (!go)
    {
        redSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    float rotation_x = m_session->GetPlayer()->m_goLastXRotation;
    float rotation_y = m_session->GetPlayer()->m_goLastYRotation;
    float orientation = go->GetOrientation();

    switch (tolower(Axis))
    {
        case 'x':
            go->setLocalRotationAngles(orientation, rotation_y, deg);
            m_session->GetPlayer()->m_goLastXRotation = deg;
            break;
        case 'y':
            go->setLocalRotationAngles(orientation, deg, rotation_x);
            m_session->GetPlayer()->m_goLastYRotation = deg;
            break;
        case 'o':
            go->SetOrientation(m_session->GetPlayer()->GetOrientation());
            go->setLocalRotationAngles(go->GetOrientation(), rotation_y, rotation_x);
            break;
        default:
            redSystemMessage(m_session, "Invalid Axis, Please use x, y, or o.");
            return true;
    }

    greenSystemMessage(m_session, "Gameobject spawn id: {} rotated", go->m_spawn->id);

    uint32_t NewGuid = m_session->GetPlayer()->getWorldMap()->generateGameobjectGuid();
    go->RemoveFromWorld(true);
    go->SetNewGuid(NewGuid);
    go->PushToWorld(m_session->GetPlayer()->getWorldMap());
    go->saveToDB();

    m_session->GetPlayer()->setSelectedGo(NewGuid);
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

    greenSystemMessage(m_session, "Selected GameObject [ {} ] which is {} meters away from you.",
        sMySQLStore.getGameObjectProperties(GObj->getEntry())->name, m_session->GetPlayer()->CalcDistance(GObj));

    return true;
}

//.gobject selectguid
bool ChatCommandHandler::HandleGOSelectGuidCommand(const char* args, WorldSession* m_session)
{
    uint32_t guid = 0;
    if (sscanf(args, "%u", &guid) != 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject selectguid <guid>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->getWorldMap()->getGameObject(guid);
    if (gameobject == nullptr)
    {
        redSystemMessage(m_session, "No GameObject found with guid {}", guid);
        return true;
    }

    m_session->GetPlayer()->setSelectedGo(gameobject->getGuid());
    greenSystemMessage(m_session, "GameObject [ {} ] with distance {} to your position selected.",
        gameobject->GetGameObjectProperties()->name, m_session->GetPlayer()->CalcDistance(gameobject));
    return true;
}

//.gobject spawn
bool ChatCommandHandler::HandleGOSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32_t go_entry = 0;
    if (sscanf(args, "%u", &go_entry) < 1)
    {
        redSystemMessage(m_session, "Wrong Syntax! Use: .gobject spawn <entry>");
        redSystemMessage(m_session, "Use: .gobject spawn <entry>");
        return true;
    }

    auto gameobject_prop = sMySQLStore.getGameObjectProperties(go_entry);
    if (gameobject_prop == nullptr)
    {
        redSystemMessage(m_session, "GameObject entry {} is a invalid entry!", go_entry);
        return true;
    }

    auto player = m_session->GetPlayer();
    auto gameobject = player->getWorldMap()->createAndSpawnGameObject(go_entry, player->GetPosition());

    greenSystemMessage(m_session, "Spawning GameObject by entry '{}'. Added to gameobject_spawns table.", gameobject->getSpawnId());
    gameobject->saveToDB(true);
    sGMLog.writefromsession(m_session, "spawned gameobject %s, entry %u at %u %f %f %f", gameobject_prop->name.c_str(), gameobject->getEntry(), player->GetMapId(), gameobject->GetPositionX(), gameobject->GetPositionY(), gameobject->GetPositionZ());

    m_session->GetPlayer()->setSelectedGo(gameobject->getGuid());

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

    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }

    greenSystemMessage(m_session, "Faction changed in gameobject_spawns table for spawn ID: {}.", go_spawn->id);
    WorldDatabase.Execute("REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)", go_spawn->id, VERSION_STRING, VERSION_STRING, gameobject->getScale(), go_faction, gameobject->getFlags());
    sGMLog.writefromsession(m_session, "changed gameobject faction of gameobject_spawns ID: %u.", go_spawn->id);

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

    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }
    greenSystemMessage(m_session, "Flags changed in gameobject_spawns table for spawn ID: {}.", go_spawn->id);
    WorldDatabase.Execute("REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)", go_spawn->id, VERSION_STRING, VERSION_STRING, gameobject->getScale(), gameobject->getFactionTemplate(), go_flags);
    sGMLog.writefromsession(m_session, "changed gameobject flags of gameobject_spawns ID: %u.", go_spawn->id);

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

    gameobject->SetOverrides(go_override);
    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }
    greenSystemMessage(m_session, "Overrides changed in gameobject_spawns table to {} for spawn ID: {}.", go_override, go_spawn->id);
    WorldDatabase.Execute("UPDATE gameobject_spawns SET overrides = %u WHERE id = %u AND min_build <= %u AND max_build >= %u", go_override, go_spawn->id, VERSION_STRING, VERSION_STRING);
    sGMLog.writefromsession(m_session, "changed gameobject scale of gameobject_spawns ID: %u to %u", go_spawn->id, go_override);

    uint32_t new_go_guid = m_session->GetPlayer()->getWorldMap()->generateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->getWorldMap());

    m_session->GetPlayer()->setSelectedGo(new_go_guid);

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

    auto go_spawn = gameobject->m_spawn;
    gameobject->Phase(PHASE_SET, phase);

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }
    greenSystemMessage(m_session, "Phase changed in gameobject_spawns table to {} for spawn ID: {}.", phase, go_spawn->id);
    WorldDatabase.Execute("UPDATE gameobject_spawns SET phase = '%lu' WHERE id = %lu AND min_build <= %u AND max_build >= %u", phase, go_spawn->id, VERSION_STRING, VERSION_STRING);
    sGMLog.writefromsession(m_session, "changed gameobject phase of gameobject_spawns ID: %u to %u", go_spawn->id, phase);

    uint32_t new_go_guid = m_session->GetPlayer()->getWorldMap()->generateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->getWorldMap());

    m_session->GetPlayer()->setSelectedGo(new_go_guid);

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

    gameobject->setScale(scale);
    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }
    greenSystemMessage(m_session, "Scale changed in gameobject_spawns_overrides table to {} for spawn ID: {}.", scale, go_spawn->id);
    WorldDatabase.Execute("REPLACE INTO gameobject_spawns_overrides VALUES(%u, %u, %u, %3.3lf,%u,%u)", go_spawn->id, VERSION_STRING, VERSION_STRING, scale, gameobject->getFactionTemplate(), gameobject->getFlags());
    sGMLog.writefromsession(m_session, "changed gameobject scale of gameobject_spawns ID: %u to %3.3lf", go_spawn->id, scale);

    uint32_t new_go_guid = m_session->GetPlayer()->getWorldMap()->generateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->getWorldMap());

    m_session->GetPlayer()->setSelectedGo(new_go_guid);

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

    auto go_spawn = gameobject->m_spawn;

    if (go_spawn == nullptr)
    {
        redSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
        return true;
    }
    greenSystemMessage(m_session, "State changed in gameobject_spawns table for spawn ID: {}.", go_spawn->id);
    WorldDatabase.Execute("UPDATE gameobject_spawns SET state = %u WHERE id = %u AND min_build <= %u AND max_build >= %u", go_state, go_spawn->id, VERSION_STRING, VERSION_STRING);
    sGMLog.writefromsession(m_session, "changed gameobject state of gameobject_spawns ID: %u.", go_spawn->id);

    return true;
}
