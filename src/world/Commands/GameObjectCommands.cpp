/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"


//.gobject damage
bool ChatHandler::HandleGODamageCommand(const char* args, WorldSession* session)
{
    uint32 damage = 0;
    uint32 spellid = 0;

    if (sscanf(args, "%u %u", &damage, &spellid) < 1)
    {
        if (damage == 0)
        {
            RedSystemMessage(session, "You need to specify how much you want to damage the selected GO!");
            return true;
        }
    }

    auto gameobject = session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(session, "You need to select a GO first!");
        return true;
    }

    if (gameobject->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        RedSystemMessage(session, "The selected GO must be a destructible building!");
        return true;
    }

    if (spellid == 0)
        spellid = 57609;

    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameobject);
    if (dgo->GetHP() == 0)
    {
        RedSystemMessage(session, "Cannot further damage a destroyed GameObject");
        return true;
    }

    uint64 guid = session->GetPlayer()->GetGUID();
    dgo->Damage(damage, guid, 0, spellid);

    GreenSystemMessage(session, "GameObject has been damaged for %u hitpoints", damage);
    GreenSystemMessage(session, "New hitpoints %u", dgo->GetHP());

    return true;
}

//.gobject rebuild
bool ChatHandler::HandleGORebuildCommand(const char* /*args*/, WorldSession* session)
{
    auto gameobject = session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(session, "You need to select a GO first!");
        return true;
    }

    if (gameobject->GetGameObjectProperties()->type != GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING)
    {
        RedSystemMessage(session, "The selected GO must be a destructible building!");
        return true;
    }

    GameObject_Destructible* dgo = static_cast<GameObject_Destructible*>(gameobject);

    uint32 oldHitPoints = dgo->GetHP();

    dgo->Rebuild();

    BlueSystemMessage(session, "GameObject has been rebuilt.");
    GreenSystemMessage(session, "Old hitpoints: %u New hitpoints %u", oldHitPoints, dgo->GetHP());

    return true;
}

//.gobject movehere
bool ChatHandler::HandleGOMoveHereCommand(const char* args, WorldSession* m_session)
{
    uint32 save = 0;
    sscanf(args, "%u", &save);

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    float position_x = m_session->GetPlayer()->GetPositionX();
    float position_y = m_session->GetPlayer()->GetPositionY();
    float position_z = m_session->GetPlayer()->GetPositionZ();
    float position_o = gameobject->GetOrientation();

    gameobject->SetPosition(position_x, position_y, position_z, position_o);
    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Position changed in gameobject_spawns table for spawn ID: %u.", go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET position_x = %f, position_y = %f, position_z = %f WHERE id = %u", position_x, position_y, position_z, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject position of gameobject_spawns ID: %u.", go_spawn->id);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "GameObject position temporarily set to your current position.");
    }

    uint32 new_go_guid = m_session->GetPlayer()->GetMapMgr()->GenerateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    m_session->GetPlayer()->m_GM_SelectedGO = new_go_guid;

    return true;
}

//.gobject selectguid
bool ChatHandler::HandleGOSelectGuidCommand(const char* args, WorldSession* m_session)
{
    uint32 guid = 0;
    if (sscanf(args, "%u", &guid) != 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject selectguid <guid>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetMapMgr()->GetGameObject(guid);
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No GameObject found with guid %u", guid);
        return true;
    }

    m_session->GetPlayer()->m_GM_SelectedGO = gameobject->GetGUID();
    GreenSystemMessage(m_session, "GameObject [ %s ] with distance %.3f to your position selected.", gameobject->GetGameObjectProperties()->name.c_str(), m_session->GetPlayer()->CalcDistance(gameobject));
    return true;
}

//.gobject open
bool ChatHandler::HandleGOOpenCommand(const char* /*args*/, WorldSession* m_session)
{
    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    if (gameobject->GetState() != GO_STATE_OPEN)
    {
        gameobject->SetState(GO_STATE_OPEN);
        BlueSystemMessage(m_session, "Gameobject opened.");
    }
    else
    {
        gameobject->SetState(GO_STATE_CLOSED);
        BlueSystemMessage(m_session, "Gameobject closed.");
    }

    return true;
}

//.gobject rotate
bool ChatHandler::HandleGORotate(const char* args, WorldSession* m_session)
{
    char Axis;
    float deg;
    if (sscanf(args, "%c %f", &Axis, &deg) < 1)
        return false;

    GameObject* go = m_session->GetPlayer()->GetSelectedGo();
    if (!go)
    {
        RedSystemMessage(m_session, "No selected GameObject...");
        return true;
    }

    float rotation_x = m_session->GetPlayer()->go_last_x_rotation;
    float rotation_y = m_session->GetPlayer()->go_last_y_rotation;
    float orientation = go->GetOrientation();

    switch (tolower(Axis))
    {
        case 'x':
            go->SetRotationAngles(orientation, rotation_y, deg);
            m_session->GetPlayer()->go_last_x_rotation = deg;
            break;
        case 'y':
            go->SetRotationAngles(orientation, deg, rotation_x);
            m_session->GetPlayer()->go_last_y_rotation = deg;
            break;
        case 'o':
            go->SetOrientation(m_session->GetPlayer()->GetOrientation());
            go->SetRotationAngles(go->GetOrientation(), rotation_y, rotation_x);
            break;
        default:
            RedSystemMessage(m_session, "Invalid Axis, Please use x, y, or o.");
            return true;
    }

    GreenSystemMessage(m_session, "Gameobject rotated");

    uint32 NewGuid = m_session->GetPlayer()->GetMapMgr()->GenerateGameobjectGuid();
    go->RemoveFromWorld(true);
    go->SetNewGuid(NewGuid);
    go->PushToWorld(m_session->GetPlayer()->GetMapMgr());
    go->SaveToDB();

    m_session->GetPlayer()->m_GM_SelectedGO = NewGuid;
    return true;
}

//.gobject spawn
bool ChatHandler::HandleGOSpawn(const char* args, WorldSession* m_session)
{
    uint32 go_entry = 0;
    uint32 save = 0;
    if (sscanf(args, "%u %u", &go_entry, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject spawn <entry>");
        RedSystemMessage(m_session, "Use: .gobject spawn <entry> 1 to save it in gameobject_spawns table.");
        return true;
    }

    auto gameobject_prop = sMySQLStore.GetGameObjectProperties(go_entry);
    if (gameobject_prop == nullptr)
    {
        RedSystemMessage(m_session, "GameObject entry %u is a invalid entry!", go_entry);
        return true;
    }

    auto player = m_session->GetPlayer();
    auto gameobject = player->GetMapMgr()->CreateGameObject(go_entry);

    uint32 mapid = player->GetMapId();
    float x = player->GetPositionX();
    float y = player->GetPositionY();
    float z = player->GetPositionZ();
    float o = player->GetOrientation();

    gameobject->CreateFromProto(go_entry, mapid, x, y, z, o);
    gameobject->PushToWorld(player->GetMapMgr());
    gameobject->Phase(PHASE_SET, player->GetPhase());

    // Create spawn instance
    GameobjectSpawn* go_spawn = new GameobjectSpawn;
    go_spawn->entry = gameobject->GetEntry();
    go_spawn->id = objmgr.GenerateGameObjectSpawnID();
    go_spawn->map = gameobject->GetMapId();
    go_spawn->position_x = gameobject->GetPositionX();
    go_spawn->position_y = gameobject->GetPositionY();
    go_spawn->position_z = gameobject->GetPositionZ();
    go_spawn->orientation = gameobject->GetOrientation();
    go_spawn->rotation_0 = gameobject->GetParentRotation(0);
    go_spawn->rotation_1 = gameobject->GetParentRotation(1);
    go_spawn->rotation_2 = gameobject->GetParentRotation(2);
    go_spawn->rotation_3 = gameobject->GetParentRotation(3);
    go_spawn->state = gameobject->GetState();
    go_spawn->flags = gameobject->GetFlags();
    go_spawn->faction = gameobject->GetFaction();
    go_spawn->scale = gameobject->GetScale();
    //go_spawn->npclink = 0;
    go_spawn->phase = gameobject->GetPhase();
    go_spawn->overrides = gameobject->GetOverrides();

    uint32 cx = player->GetMapMgr()->GetPosX(player->GetPositionX());
    uint32 cy = player->GetMapMgr()->GetPosY(player->GetPositionY());
    player->GetMapMgr()->GetBaseMap()->GetSpawnsListAndCreate(cx, cy)->GameobjectSpawns.push_back(go_spawn);
    gameobject->m_spawn = go_spawn;

    MapCell* mCell = player->GetMapMgr()->GetCell(cx, cy);
    if (mCell != nullptr)
        mCell->SetLoaded();

    bool save_to_db = false;
    if (m_session->GetPlayer()->SaveAllChangesCommand || save > 0)
        save_to_db = true;

    if (save_to_db)
    {
        GreenSystemMessage(m_session, "Spawning GameObject by entry '%u'. Added to gameobject_spawns table.", go_spawn->id);
        gameobject->SaveToDB();
        sGMLog.writefromsession(m_session, "spawned gameobject %s, entry %u at %u %f %f %f%s", sMySQLStore.GetGameObjectProperties(go_spawn->entry)->name.c_str(), go_spawn->entry, player->GetMapId(), go_spawn->position_x, go_spawn->position_y, go_spawn->position_z, save == 1 ? ", saved in DB" : "");
    }
    else
    {
        GreenSystemMessage(m_session, "Spawning temporarily GameObject with entry '%u'", go_spawn->entry);
    }

    m_session->GetPlayer()->m_GM_SelectedGO = gameobject->GetGUID();

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .gobject set commands
//.gobject set animprogress
bool ChatHandler::HandleGOSetAnimProgressCommand(const char* args, WorldSession* m_session)
{
    uint32 animprogress;

    if (sscanf(args, "%u", &animprogress) != 1)
    {
        RedSystemMessage(m_session, "You need to define the animprogress value!");
        RedSystemMessage(m_session, ".gobject setanimprogress <animprogress>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    gameobject->SetAnimProgress(static_cast<uint8>(animprogress));
    GreenSystemMessage(m_session, "Gameobject animprogress set to %u", animprogress);

    return true;
}

//.gobject set faction
bool ChatHandler::HandleGOSetFactionCommand(const char* args, WorldSession* m_session)
{
    uint32 go_faction = 0;
    uint32 save = 0;
    if (sscanf(args, "%u %u", &go_faction, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject setfaction <faction>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No GameObject is selected.");
        return true;
    }

    auto faction_template = sFactionTemplateStore.LookupEntry(go_faction);
    if (faction_template == nullptr)
    {
        RedSystemMessage(m_session, "The entered faction is invalid! Use a valid faction id.");
        return false;
    }

    gameobject->SetFaction(go_faction);

    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Faction changed in gameobject_spawns table for spawn ID: %u.", go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET faction = %u WHERE id = %u", go_faction, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject faction of gameobject_spawns ID: %u.", go_spawn->id);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "GameObject faction temporarily set to %u.", go_faction);
    }

    return true;
}

//.gobject set flags
bool ChatHandler::HandleGOSetFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32 go_flags;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &go_flags, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject setflags <flags>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No GameObject selected!");
        return true;
    }

    gameobject->SetFlags(go_flags);

    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Flags changed in gameobject_spawns table for spawn ID: %u.", go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET flags = %u WHERE id = %u", go_flags, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject flags of gameobject_spawns ID: %u.", go_spawn->id);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "GameObject flags temporarily set to %u.", go_flags);
    }

    return true;
}

//.gobject set overrides
bool ChatHandler::HandleGOSetOverridesCommand(const char* args, WorldSession* m_session)
{
    uint32 go_override;
    uint32 save = 0;
    if (sscanf(args, "%u %u", &go_override, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject setoverride <value>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    gameobject->SetOverrides(go_override);
    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Overrides changed in gameobject_spawns table to %u for spawn ID: %u.", go_override, go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET overrides = %u WHERE id = %u", go_override, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject scale of gameobject_spawns ID: %u to %u", go_spawn->id, go_override);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "Gameobject overrides temporarily set to %u for spawn ID: %u.", go_override, go_spawn->id);
    }

    uint32 new_go_guid = m_session->GetPlayer()->GetMapMgr()->GenerateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    m_session->GetPlayer()->m_GM_SelectedGO = new_go_guid;

    return true;
}


//.gobject set phase
bool ChatHandler::HandleGOSetPhaseCommand(const char* args, WorldSession* m_session)
{
    uint32 phase;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &phase, &save) < 1)
    {
        RedSystemMessage(m_session, "You need to define the phase value!");
        RedSystemMessage(m_session, ".gobject setphase <phase>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    uint32 old_phase = gameobject->GetPhase();
    auto go_spawn = gameobject->m_spawn;
    gameobject->Phase(PHASE_SET, phase);

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Phase changed in gameobject_spawns table to %u for spawn ID: %u.", phase, go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET phase = '%lu' WHERE id = %lu", phase, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject phase of gameobject_spawns ID: %u to %u", go_spawn->id, phase);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "GameObject phase temporarily set to %u.", phase);
    }

    uint32 new_go_guid = m_session->GetPlayer()->GetMapMgr()->GenerateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    m_session->GetPlayer()->m_GM_SelectedGO = new_go_guid;

    return true;
}

//.gobject set scale
bool ChatHandler::HandleGOSetScaleCommand(const char* args, WorldSession* m_session)
{
    float scale = 0.0f;
    uint32 save = 0;
    if (sscanf(args, "%f %u", &scale, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject setscale <scale>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No selected GameObject!");
        return true;
    }

    gameobject->SetScale(scale);
    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "Scale changed in gameobject_spawns table to %3.3lf for spawn ID: %u.", scale, go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET scale = %3.3lf WHERE id = %u", scale, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject scale of gameobject_spawns ID: %u to %3.3lf", go_spawn->id, scale);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "Gameobject scale temporarily set to %3.3lf for spawn ID: %u.", scale, go_spawn->id);
    }

    uint32 new_go_guid = m_session->GetPlayer()->GetMapMgr()->GenerateGameobjectGuid();
    gameobject->RemoveFromWorld(true);
    gameobject->SetNewGuid(new_go_guid);
    gameobject->PushToWorld(m_session->GetPlayer()->GetMapMgr());

    m_session->GetPlayer()->m_GM_SelectedGO = new_go_guid;

    return true;
}

//.gobject set state
bool ChatHandler::HandleGOSetStateCommand(const char* args, WorldSession* m_session)
{
    uint32 go_state;
    uint32 save = 0;

    if (sscanf(args, "%u %u", &go_state, &save) < 1)
    {
        RedSystemMessage(m_session, "Wrong Syntax! Use: .gobject setstate <state>");
        return true;
    }

    auto gameobject = m_session->GetPlayer()->GetSelectedGo();
    if (gameobject == nullptr)
    {
        RedSystemMessage(m_session, "No GameObject selected!");
        return true;
    }

    gameobject->SetState(static_cast<uint8>(go_state));

    auto go_spawn = gameobject->m_spawn;

    if (m_session->GetPlayer()->SaveAllChangesCommand)
        save = 1;

    if (save == 1)
    {
        if (go_spawn == nullptr)
        {
            RedSystemMessage(m_session, "The GameObject is not a spawn to save the data.");
            return true;
        }
        else
        {
            GreenSystemMessage(m_session, "State changed in gameobject_spawns table for spawn ID: %u.", go_spawn->id);
            WorldDatabase.Execute("UPDATE gameobject_spawns SET state = %u WHERE id = %u", go_state, go_spawn->id);
            sGMLog.writefromsession(m_session, "changed gameobject state of gameobject_spawns ID: %u.", go_spawn->id);
        }
    }
    else
    {
        GreenSystemMessage(m_session, "GameObject state temporarily set to %u.", go_state);
    }

    return true;
}
