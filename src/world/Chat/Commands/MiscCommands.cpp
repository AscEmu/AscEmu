/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellMgr.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/Customization/SpellCustomizations.hpp"

//.mount
bool ChatHandler::HandleMountCommand(const char* args, WorldSession* m_session)
{
    Unit* unit_target = GetSelectedUnit(m_session, true);
    if (unit_target == nullptr)
        return true;

    if (!args)
    {
        RedSystemMessage(m_session, "No model specified!");
        return true;
    }

    uint32 modelid = atol(args);
    if (!modelid)
    {
        RedSystemMessage(m_session, "No model specified!");
        return true;
    }

    if (unit_target->GetMount() != 0)
    {
        RedSystemMessage(m_session, "Target is already mounted.");
        return true;
    }

    unit_target->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, modelid);

    BlueSystemMessage(m_session, "Now mounted with model %d.", modelid);
    sGMLog.writefromsession(m_session, "used mount command to model %u", modelid);

    return true;
}

//.dismount
bool ChatHandler::HandleDismountCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit_target = GetSelectedUnit(m_session, true);
    if (unit_target == nullptr)
        return true;

    if (unit_target->GetMount() == 0)
    {
        RedSystemMessage(m_session, "Target is not mounted.");
        return true;
    }

    if (unit_target->IsPlayer())
        static_cast<Player*>(unit_target)->Dismount();

    unit_target->setUInt32Value(UNIT_FIELD_MOUNTDISPLAYID, 0);

    BlueSystemMessage(m_session, "Target is now unmounted.");
    return true;
}

//.gocreature
bool ChatHandler::HandleGoCreatureSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32 spawn_id;
    if (sscanf(args, "%u", (unsigned int*)&spawn_id) != 1)
    {
        RedSystemMessage(m_session, "Command must be in format: .gocreature <creature_spawnid>.");
        return true;
    }

    QueryResult* query_result = WorldDatabase.Query("SELECT * FROM creature_spawns WHERE id=%u", spawn_id);
    if (!query_result)
    {
        RedSystemMessage(m_session, "No creature found in creature_spawns table with id %u.", spawn_id);
        return true;
    }

    uint32 spawn_map = query_result->Fetch()[2].GetUInt32();
    float spawn_x = query_result->Fetch()[3].GetFloat();
    float spawn_y = query_result->Fetch()[4].GetFloat();
    float spawn_z = query_result->Fetch()[5].GetFloat();

    LocationVector vec(spawn_x, spawn_y, spawn_z, 0);
    m_session->GetPlayer()->SafeTeleport(spawn_map, 0, vec);

    delete query_result;
    return true;
}

//.gogameobject
bool ChatHandler::HandleGoGameObjectSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32 spawn_id;
    if (sscanf(args, "%u", (unsigned int*)&spawn_id) != 1)
    {
        RedSystemMessage(m_session, "Command must be in format: .gocreature <creature_spawnid>.");
        return true;
    }

    QueryResult* query_result = WorldDatabase.Query("SELECT * FROM gameobject_spawns WHERE id=%u", spawn_id);
    if (!query_result)
    {
        RedSystemMessage(m_session, "No gameobject found in gameobject_spawns table with id %u.", spawn_id);
        return true;
    }

    uint32 spawn_map = query_result->Fetch()[2].GetUInt32();
    float spawn_x = query_result->Fetch()[3].GetFloat();
    float spawn_y = query_result->Fetch()[4].GetFloat();
    float spawn_z = query_result->Fetch()[5].GetFloat();

    LocationVector vec(spawn_x, spawn_y, spawn_z, 0);
    m_session->GetPlayer()->SafeTeleport(spawn_map, 0, vec);

    delete query_result;
    return true;
}

//.gostartlocation
bool ChatHandler::HandleGoStartLocationCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    std::string race;
    uint8 raceid = 0;
    uint8 classid = 0;

    if (strlen(args) > 0)
    {
        race = args;
        if (race == "human")
            raceid = 1;
        else if (race == "orc")
            raceid = 2;
        else if (race == "dwarf")
            raceid = 3;
        else if (race == "nightelf")
            raceid = 4;
        else if (race == "undead")
            raceid = 5;
        else if (race == "tauren")
            raceid = 6;
        else if (race == "gnome")
            raceid = 7;
        else if (race == "troll")
            raceid = 8;
#if VERSION_STRING == Cata
        else if (race == "goblin")
            raceid = 9;
#endif
        else if (race == "bloodelf")
            raceid = 10;
        else if (race == "draenei")
            raceid = 11;
        else if (race == "deathknight")
            classid = 6;
#if VERSION_STRING == Cata
        else if (race == "worgen")
            raceid = 22;
#endif
        else
        {
#if VERSION_STRING == Cata
            RedSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, goblin, tauren, undead, bloodelf, worgen, deathknight");
#else
            RedSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, tauren, undead, bloodelf, deathknight");
#endif
            return true;
        }
    }
    else
    {
        raceid = player_target->getRace();
        classid = player_target->getClass();
        race = "own";
    }

    PlayerCreateInfo const* player_info = nullptr;
    for (uint8 i = 1; i <= 11; ++i)
    {
        player_info = sMySQLStore.getPlayerCreateInfo((raceid ? raceid : i), (classid ? classid : i));
        if (player_info != nullptr)
            break;
    }

    if (player_info == nullptr)
    {
        RedSystemMessage(m_session, "Internal error: Could not find create info for race %u and class %u.", raceid, classid);
        return false;
    }

    GreenSystemMessage(m_session, "Teleporting %s to %s starting location.", player_target->GetName(), race.c_str());

    player_target->SafeTeleport(player_info->mapId, 0, LocationVector(player_info->positionX, player_info->positionY, player_info->positionZ));
    return true;
}

//.gotrig
bool ChatHandler::HandleGoTriggerCommand(const char* args, WorldSession* m_session)
{
    uint32 trigger_id;
    int32 instance_id = 0;

    if (sscanf(args, "%u %d", (unsigned int*)&trigger_id, (int*)&instance_id) < 1)
    {
        RedSystemMessage(m_session, "Command must be at least in format: .gotrig <trigger_id>.");
        RedSystemMessage(m_session, "You can use: .gotrig <trigger_id> <instance_id>");
        return true;
    }

    auto area_trigger_entry = sAreaTriggerStore.LookupEntry(trigger_id);
    if (area_trigger_entry == nullptr)
    {
        RedSystemMessage(m_session, "Could not find trigger %s", args);
        return true;
    }

    m_session->GetPlayer()->SafeTeleport(area_trigger_entry->mapid, instance_id, LocationVector(area_trigger_entry->x, area_trigger_entry->y, area_trigger_entry->z, area_trigger_entry->o));
    BlueSystemMessage(m_session, "Teleported to trigger %u on [%u][%.2f][%.2f][%.2f]", area_trigger_entry->id, area_trigger_entry->mapid, area_trigger_entry->x, area_trigger_entry->y, area_trigger_entry->z);
    return true;
}

//.kill - kills target or player with <name>
bool ChatHandler::HandleKillCommand(const char* args, WorldSession* m_session)
{
    bool is_name_set = false;
    auto unit_target = GetSelectedUnit(m_session);

    if (*args)
        is_name_set = true;

    if (is_name_set)
    {
        auto named_player = objmgr.GetPlayer(args, false);
        if (named_player == nullptr)
        {
            RedSystemMessage(m_session, "Player %s is not online or does not exist!", args);
            return true;
        }
        else
        {
            named_player->SetHealth(0);
            named_player->KillPlayer();
            RedSystemMessage(named_player->GetSession(), "You were killed by %s with a GM command.", m_session->GetPlayer()->GetName());
            GreenSystemMessage(m_session, "Killed player %s.", args);
            sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid:  " I64FMT " ", m_session->GetPlayer()->GetName(), named_player->GetGUID(), named_player->GetNameString());
        }
    }
    else
    {
        if (unit_target != nullptr)
        {
            switch (unit_target->GetTypeId())
            {
                case TYPEID_UNIT:
                {
                    auto creature = static_cast<Creature*>(unit_target);
                    auto kill_spell = sSpellCustomizations.GetSpellInfo(5);
                    if (kill_spell == nullptr)
                        return true;

                    SpellCastTargets targets(unit_target->GetGUID());
                    Spell* spell = sSpellFactoryMgr.NewSpell(m_session->GetPlayer(), kill_spell, true, 0);
                    spell->prepare(&targets);

                    GreenSystemMessage(m_session, "Killed Creature %s.", creature->GetCreatureProperties()->Name.c_str());
                    sGMLog.writefromsession(m_session, "used kill command on Creature %u [%s], spawn ID: %u", creature->GetEntry(), creature->GetCreatureProperties()->Name.c_str(), creature->spawnid);
                    break;
                }
                case TYPEID_PLAYER:
                {
                    auto player = static_cast<Player*>(unit_target);

                    player->SetHealth(0);
                    player->KillPlayer();
                    RedSystemMessage(player->GetSession(), "You were killed by %s with a GM command.", m_session->GetPlayer()->GetName());
                    GreenSystemMessage(m_session, "Killed player %s.", player->GetName());
                    sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid:  " I64FMT " ", m_session->GetPlayer()->GetName(), player->GetGUID());
                    break;
                }
                default:
                    RedSystemMessage(m_session, "Something went wrong while killing a unit with this command!");
                    break;

            }
        }
        else
        {
            RedSystemMessage(m_session, "No selected target or player name set.");
            RedSystemMessage(m_session, "Use .kill on an selected unit (player/creature)");
            RedSystemMessage(m_session, "Use .kill <playername> to kill players");
            return true;
        }

    }

    return true;
}

//.revive - revives selfe or player with <name>
bool ChatHandler::HandleReviveCommand(const char* args, WorldSession* m_session)
{
    bool is_name_set = false;

    if (*args)
        is_name_set = true;

    if (is_name_set)
    {
        auto named_player = objmgr.GetPlayer(args, false);
        if (named_player != nullptr)
        {
            if (named_player->IsDead())
            {
                named_player->RemoteRevive();
                GreenSystemMessage(m_session, "Player %s revived.", args);
                sGMLog.writefromsession(m_session, "revived player %s.", args);
            }
            else
            {
                SystemMessage(m_session, "Player %s is not dead.", args);
                return true;
            }
        }
        else
        {
            RedSystemMessage(m_session, "Player %s is not online or does not exist!", args);
            return true;
        }
    }
    else
    {
        auto player_target = GetSelectedPlayer(m_session, false, true);
        if (player_target == nullptr)
        {
            RedSystemMessage(m_session, "Something went wrong while reviving a player with this command!");
            return true;
        }

        if (player_target->IsDead())
        {
            player_target->setMoveRoot(false);
            player_target->ResurrectPlayer();
            player_target->SetHealth(player_target->GetMaxHealth());
            player_target->SetPower(POWER_TYPE_MANA, player_target->GetMaxPower(POWER_TYPE_MANA));
            player_target->SetPower(POWER_TYPE_ENERGY, player_target->GetMaxPower(POWER_TYPE_ENERGY));

            if (player_target == m_session->GetPlayer())
            {
                sGMLog.writefromsession(m_session, "revived himself");
            }
            else
            {
                GreenSystemMessage(m_session, "Player %s revived.", player_target->GetName());
                sGMLog.writefromsession(m_session, "revived player %s", player_target->GetName());
            }
        }
        else
        {
            if (player_target == m_session->GetPlayer())
                RedSystemMessage(m_session, "You are not dead!");
            else
                RedSystemMessage(m_session, "Player %s is not dead!", player_target->GetName());

            return true;
        }
    }

    return true;
}

//.root
bool ChatHandler::HandleRootCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    unit->setMoveRoot(true);

    if (unit->IsPlayer())
    {
        SystemMessage(m_session, "Rooting Player %s.", static_cast<Player*>(unit)->GetName());
        BlueSystemMessage(static_cast<Player*>(unit)->GetSession(), "You have been rooted by %s.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "rooted player %s", static_cast<Player*>(unit)->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Rooting Creature %s.", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
        sGMLog.writefromsession(m_session, "rooted creature %s", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
    }

    return true;
}

//.unroot
bool ChatHandler::HandleUnrootCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    unit->setMoveRoot(false);

    if (unit->IsPlayer())
    {
        SystemMessage(m_session, "Unrooting Player %s.", static_cast<Player*>(unit)->GetName());
        BlueSystemMessage(static_cast<Player*>(unit)->GetSession(), "You have been unrooted by %s.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "unrooted player %s", static_cast<Player*>(unit)->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Unrooting Creature %s.", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
        sGMLog.writefromsession(m_session, "unrooted creature %s", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
    }

    return true;
}

//.autosavechanges
bool ChatHandler::HandleAutoSaveChangesCommand(const char* /*args*/, WorldSession* m_session)
{
    if (m_session->GetPlayer()->SaveAllChangesCommand == false)
    {
        GreenSystemMessage(m_session, "SaveAllChanges activated! All commands will be executed as 'save to db = true");
        m_session->GetPlayer()->SaveAllChangesCommand = true;
    }
    else
    {
        GreenSystemMessage(m_session, "SaveAllChanges deactivated! All commands will be executed as 'save to db = false");
        m_session->GetPlayer()->SaveAllChangesCommand = false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .kick commands
//.kick player
bool ChatHandler::HandleKickByNameCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "A player name is required!");
        RedSystemMessage(m_session, "Usage: .kick player <player_name>");
        RedSystemMessage(m_session, "Optional: .kick player <player_name> <reason>");
        return true;
    }

    char* player_name = strtok((char*)args, " ");
    auto player_target = objmgr.GetPlayer((const char*)player_name, false);
    if (player_target != nullptr)
    {
        char* reason = strtok(NULL, "\n");
        std::string kickreason = "No reason";

        if (reason)
            kickreason = reason;

        BlueSystemMessage(m_session, "Attempting to kick %s from the server for \'%s\'.", player_target->GetName(), kickreason.c_str());
        sGMLog.writefromsession(m_session, "Kicked player %s from the server for %s", player_target->GetName(), kickreason.c_str());

        if (!m_session->CanUseCommand('z') && player_target->GetSession()->CanUseCommand('z'))
        {
            RedSystemMessage(m_session, "You cannot kick %s, a GM whose permissions outrank yours.", player_target->GetName());
            return true;
        }

        if (worldConfig.gm.worldAnnounceOnKickPlayer)
        {
            std::stringstream worldAnnounce;
            worldAnnounce << MSG_COLOR_RED << "GM: " << player_target->GetName() << " was kicked from the server by " << m_session->GetPlayer()->GetName() << ". Reason: " << kickreason;
            sWorld.sendMessageToAll(worldAnnounce.str(), nullptr);
        }

        SystemMessage(player_target->GetSession(), "You are being kicked from the server by %s. Reason: %s", m_session->GetPlayer()->GetName(), kickreason.c_str());
        player_target->Kick(6000);
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Player is not online at the moment.");
    }

    return true;
}

//.kick account
bool ChatHandler::HandleKKickBySessionCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "A account name is required!");
        RedSystemMessage(m_session, "Usage: .kick account <account_name>");
        return true;
    }

    char* player_name = strtok((char*)args, " ");
    auto player_target = objmgr.GetPlayer((const char*)player_name, false);
    if (player_target != nullptr)
    {
        if (!m_session->CanUseCommand('z') && player_target->GetSession()->CanUseCommand('z'))
        {
            RedSystemMessage(m_session, "You cannot kick %s, a GM whose permissions outrank yours.", player_target->GetName());
            return true;
        }

        sWorld.disconnectSessionByAccountName(args, m_session);
        sGMLog.writefromsession(m_session, "kicked player with account %s", args);
    }
    else
    {
        RedSystemMessage(m_session, "Player is not online at the moment.");
    }

    return true;
}

//.kick ip
bool ChatHandler::HandleKickByIPCommand(const char* args, WorldSession* m_session)
{
    if (!args || strlen(args) < 8)
    {
        RedSystemMessage(m_session, "An IP is required!");
        RedSystemMessage(m_session, "Usage: .kick ip <AN.IP.ADD.RES>");
        return true;
    }

    sWorld.disconnectSessionByIp(args, m_session);
    sGMLog.writefromsession(m_session, "kicked players with IP %s", args);

    return true;
}

//.worldport
bool ChatHandler::HandleWorldPortCommand(const char* args, WorldSession* m_session)
{
    float x, y, z, o = 0.0f;
    uint32 mapid;

    if (sscanf(args, "%u %f %f %f %f", (unsigned int*)&mapid, &x, &y, &z, &o) < 4)
    {
        RedSystemMessage(m_session, "You have to use at least .worldport <mapid> <x> <y> <z>");
        return true;
    }

    if (x >= _maxX || x <= _minX || y <= _minY || y >= _maxY)
    {
        RedSystemMessage(m_session, "<x> <y> value is out of range!");
        return true;
    }

    LocationVector vec(x, y, z, o);
    m_session->GetPlayer()->SafeTeleport(mapid, 0, vec);

    return true;
}

//.gps
bool ChatHandler::HandleGPSCommand(const char* args, WorldSession* m_session)
{
    Object* obj;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char buf[400];
    auto at = obj->GetArea();
    if (!at)
    {
        snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f|r",
            (unsigned int)obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
        SystemMessage(m_session, buf);
        return true;
    }

    auto out_map_id = obj->GetMapId();
    auto out_zone_id = at->zone; // uint32 at_old->ZoneId
    auto out_area_id = at->id; // uint32 at_old->AreaId
    auto out_phase = obj->GetPhase();
    auto out_x = obj->GetPositionX();
    auto out_y = obj->GetPositionY();
    auto out_z = obj->GetPositionZ();
    auto out_o = obj->GetOrientation();
#if VERSION_STRING != Cata
    auto out_area_name = at->area_name[0]; // enUS, hardcoded until locale is implemented properly
#else
    auto out_area_name = at->area_name;
#endif

    snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffZone: |cff00ff00%u |cffffffffArea: |cff00ff00%u |cffffffffPhase: |cff00ff00%u |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f |cffffffffArea Name: |cff00ff00%s |r",
        out_map_id, out_zone_id, out_area_id, out_phase, out_x, out_y, out_z, out_o, out_area_name);
    SystemMessage(m_session, buf);

#if VERSION_STRING != Cata
    if (obj->obj_movement_info.IsOnTransport())
#else
    if (!obj->obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
        SystemMessage(m_session, "Position on Transport:");
        SystemMessage(m_session, "  tX: %f  tY: %f  tZ: %f  tO: %f", obj->GetTransPositionX(), obj->GetTransPositionY(), obj->GetTransPositionZ(), obj->GetTransPositionO());
    }

    // ".gps 1" will save gps info to file logs/gps.log - This probably isn't very multithread safe so don't have many gms spamming it!
    if (args != NULL && *args == '1')
    {
        FILE* gpslog = fopen(AELog::GetFormattedFileName("logs", "gps", false).c_str(), "at");
        if (gpslog)
        {
            fprintf(gpslog, "%d, %u, %u, %f, %f, %f, %f, \'%s\'", out_map_id, out_zone_id, out_area_id, out_x, out_y, out_z, out_o, out_area_name);
            // ".gps 1 comment" will save comment after the gps data
            if (*(args + 1) == ' ')
                fprintf(gpslog, ",%s\n", args + 2);
            else
                fprintf(gpslog, "\n");
            fclose(gpslog);
        }
    }
    return true;
}

//.invincible
bool ChatHandler::HandleInvincibleCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->bInvincible)
    {
        selected_player->bInvincible = false;

        if (selected_player != m_session->GetPlayer())
        {
            GreenSystemMessage(selected_player->GetSession(), "%s turns your invincibility off", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "turns invincibility off for %s", selected_player->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Invincibility is now off");
        }
    }
    else
    {
        selected_player->bInvincible = true;

        if (selected_player != m_session->GetPlayer())
        {
            GreenSystemMessage(selected_player->GetSession(), "%s turns your invincibility on", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "turns invincibility on for %s", selected_player->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Invincibility is now on");
        }
    }

    return true;
}

//.invisible
bool ChatHandler::HandleInvisibleCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->m_isGmInvisible)
    {
        selected_player->m_isGmInvisible = false;
        selected_player->m_invisible = false;
        selected_player->bInvincible = false;

        selected_player->Social_TellFriendsOnline();

        if (selected_player->m_bg)
            selected_player->m_bg->RemoveInvisGM();

        if (selected_player != m_session->GetPlayer())
        {
            GreenSystemMessage(selected_player->GetSession(), "%s turns your invisibility and invincibility off", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "turns invisibility and invincibility off for %s", selected_player->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Invisibility and Invincibility are now off");
        }
    }
    else
    {
        selected_player->m_isGmInvisible = true;
        selected_player->m_invisible = true;
        selected_player->bInvincible = true;

        selected_player->Social_TellFriendsOffline();

        if (selected_player->m_bg)
            selected_player->m_bg->AddInvisGM();

        if (selected_player != m_session->GetPlayer())
        {
            GreenSystemMessage(selected_player->GetSession(), "%s turns your invisibility and invincibility on", m_session->GetPlayer()->GetName());
            sGMLog.writefromsession(m_session, "turns invisibility and invincibility on for %s", selected_player->GetName());
        }
        else
        {
            GreenSystemMessage(m_session, "Invisibility and Invincibility are now on");
        }
    }

    selected_player->UpdateVisibility();

    return true;
}

//.announce
bool ChatHandler::HandleAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args || strlen(args) < 4 || strchr(args, '%'))
    {
        m_session->SystemMessage("Announces cannot contain the %% character and must be at least 4 characters.");
        return true;
    }

    std::stringstream worldAnnounce;
    worldAnnounce << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor);
    worldAnnounce << "[";
    worldAnnounce << worldConfig.announce.announceTag;
    worldAnnounce << "]";
    worldAnnounce << worldConfig.getColorStringForNumber(worldConfig.announce.tagGmColor);

    if (worldConfig.announce.enableGmAdminTag)
    {
        if (m_session->CanUseCommand('z'))
            worldAnnounce << "<Admin>";
        else if (m_session->GetPermissionCount())
            worldAnnounce << "<GM>";
    }

    if (worldConfig.announce.showNameInAnnounce)
    {
        worldAnnounce << "|r" << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor) << "|Hplayer:";
        worldAnnounce << m_session->GetPlayer()->GetName();
        worldAnnounce << "|h[";
        worldAnnounce << m_session->GetPlayer()->GetName();
        worldAnnounce << "]|h:|r " << worldConfig.getColorStringForNumber(worldConfig.announce.msgColor);
    }
    else if (!worldConfig.announce.showNameInAnnounce)
    {
        worldAnnounce << ": ";
        worldAnnounce << worldConfig.getColorStringForNumber(worldConfig.announce.msgColor);
    }

    worldAnnounce << args;

    sWorld.sendMessageToAll(worldAnnounce.str());

    sGMLog.writefromsession(m_session, "used announce command, [%s]", args);

    return true;
}

//.wannounce
bool ChatHandler::HandleWAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::stringstream colored_widescreen_text;
    colored_widescreen_text << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor);
    colored_widescreen_text << "[";
    colored_widescreen_text << worldConfig.announce.announceTag;
    colored_widescreen_text << "]";
    colored_widescreen_text << worldConfig.getColorStringForNumber(worldConfig.announce.tagGmColor);

    if (worldConfig.announce.enableGmAdminTag)
    {
        if (m_session->CanUseCommand('z'))
            colored_widescreen_text << "<Admin>";
        else if (m_session->GetPermissionCount())
            colored_widescreen_text << "<GM>";
    }

    if (worldConfig.announce.showNameInWAnnounce)
    {
        colored_widescreen_text << "|r" << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor) << "[";
        colored_widescreen_text << m_session->GetPlayer()->GetName();
        colored_widescreen_text << "]:|r " << worldConfig.getColorStringForNumber(worldConfig.announce.msgColor);
    }
    else if (!worldConfig.announce.showNameInWAnnounce)
    {
        colored_widescreen_text << ": "; colored_widescreen_text << worldConfig.getColorStringForNumber(worldConfig.announce.msgColor);
    }

    colored_widescreen_text << args;

    sWorld.sendAreaTriggerMessage(colored_widescreen_text.str());

    sGMLog.writefromsession(m_session, "used wannounce command [%s]", args);

    return true;
}

//.appear
bool ChatHandler::HandleAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        if (!m_session->CanUseCommand('z') && chr->IsAppearDisabled())
        {
            SystemMessage(m_session, "%s has blocked other GMs from appearing to them.", chr->GetName());
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            SystemMessage(m_session, "%s is already being teleported.", chr->GetName());
            return true;
        }
        SystemMessage(m_session, "Appearing at %s's location.", chr->GetName());
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "%s is appearing to your location.", m_session->GetPlayer()->GetName());
        }

#if VERSION_STRING != Cata
        if (m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID())
            m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        else
            m_session->GetPlayer()->SafeTeleport(chr->GetMapMgr(), chr->GetPosition());
#else
        m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), 0, chr->GetPosition());
#endif

    }
    else
    {
        SystemMessage(m_session, "Player (%s) does not exist or is not logged in.", args);
    }
    return true;
}

//.blockappear
bool ChatHandler::HandleBlockAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            BlueSystemMessage(m_session, "Appear blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableAppear(true);
            GreenSystemMessage(m_session, "Appear blocking is now enabled");
        }
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            m_session->GetPlayer()->DisableAppear(false);
            GreenSystemMessage(m_session, "Appear blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Appear blocking is already disabled");
        }
    }

    return true;
}

//.summon
bool ChatHandler::HandleSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {

        char buf[256];
        if (!m_session->CanUseCommand('z') && chr->IsSummonDisabled())
        {
            snprintf((char*)buf, 256, "%s has blocked other GMs from summoning them.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            snprintf((char*)buf, 256, "%s is already being teleported.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        snprintf((char*)buf, 256, "You are summoning %s.", chr->GetName());
        SystemMessage(m_session, buf);

        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "You are being summoned by %s.", m_session->GetPlayer()->GetName());
        }
        Player* plr = m_session->GetPlayer();
        if (plr->GetMapMgr() == chr->GetMapMgr())
            chr->_Relocate(plr->GetMapId(), plr->GetPosition(), false, false, plr->GetInstanceID());
        else
        {
            sEventMgr.AddEvent(chr, &Player::EventPortToGM, plr, 0, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
    else
    {
        PlayerInfo* pinfo = objmgr.GetPlayerInfoByName(args);
        if (!pinfo)
        {
            char buf[256];
            snprintf((char*)buf, 256, "Player (%s) does not exist.", args);
            SystemMessage(m_session, buf);
            return true;
        }
        else
        {
            Player* pPlayer = m_session->GetPlayer();
            char query[512];
            snprintf((char*)&query, 512, "UPDATE characters SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetZoneId(), pinfo->guid);
            CharacterDatabase.Execute(query);
            char buf[256];
            snprintf((char*)buf, 256, "(Offline) %s has been summoned.", pinfo->name);
            SystemMessage(m_session, buf);
            return true;
        }
    }
    sGMLog.writefromsession(m_session, "summoned %s on map %u, %f %f %f", args, m_session->GetPlayer()->GetMapId(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    return true;
}

//.blocksummon
bool ChatHandler::HandleBlockSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            BlueSystemMessage(m_session, "Summon blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableSummon(true);
            GreenSystemMessage(m_session, "Summon blocking is now enabled");
        }
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            m_session->GetPlayer()->DisableSummon(false);
            GreenSystemMessage(m_session, "Summon blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Summon blocking is already disabled");
        }
    }

    return true;
}

//.playerinfo
bool ChatHandler::HandlePlayerInfo(const char* args, WorldSession* m_session)
{
    Player* plr;
    if (strlen(args) >= 4)
    {
        plr = objmgr.GetPlayer(args, false);
        if (!plr)
        {
            RedSystemMessage(m_session, "Unable to locate player %s.", args);
            return true;
        }
    }
    else
        plr = GetSelectedPlayer(m_session, true, true);

    if (!plr) return true;
    if (!plr->GetSession())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any session !");
        return true;
    }
    if (!plr->GetSession()->GetSocket())
    {
        RedSystemMessage(m_session, "ERROR: this player hasn't got any socket !");
        return true;
    }
    WorldSession* sess = plr->GetSession();

    static const char* classes[MAX_PLAYER_CLASSES] =
    { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
#if VERSION_STRING != Cata
    static const char* races[NUM_RACES] =
    { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "None", "Blood Elf", "Draenei" };
#else
    static const char* races[NUM_RACES] =
    { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "Goblin", "Blood Elf", "Draenei", "None", "None", "None", "None", "None", "None", "None", "None", "None", "None", "Worgen" };
#endif

    char playedLevel[64];
    char playedTotal[64];

    int seconds = (plr->GetPlayedtime())[0];
    int mins = 0;
    int hours = 0;
    int days = 0;
    if (seconds >= 60)
    {
        mins = seconds / 60;
        if (mins)
        {
            seconds -= mins * 60;
            if (mins >= 60)
            {
                hours = mins / 60;
                if (hours)
                {
                    mins -= hours * 60;
                    if (hours >= 24)
                    {
                        days = hours / 24;
                        if (days)
                            hours -= days * 24;
                    }
                }
            }
        }
    }
    snprintf(playedLevel, 64, "[%d days, %d hours, %d minutes, %d seconds]", days, hours, mins, seconds);

    seconds = (plr->GetPlayedtime())[1];
    mins = 0;
    hours = 0;
    days = 0;
    if (seconds >= 60)
    {
        mins = seconds / 60;
        if (mins)
        {
            seconds -= mins * 60;
            if (mins >= 60)
            {
                hours = mins / 60;
                if (hours)
                {
                    mins -= hours * 60;
                    if (hours >= 24)
                    {
                        days = hours / 24;
                        if (days)
                            hours -= days * 24;
                    }
                }
            }
        }
    }
    snprintf(playedTotal, 64, "[%d days, %d hours, %d minutes, %d seconds]", days, hours, mins, seconds);

    GreenSystemMessage(m_session, "%s is a %s %s %s", plr->GetName(), (plr->getGender() ? "Female" : "Male"), races[plr->getRace()], classes[plr->getClass()]);

    BlueSystemMessage(m_session, "%s has played %s at this level", (plr->getGender() ? "She" : "He"), playedLevel);
    BlueSystemMessage(m_session, "and %s overall", playedTotal);

    BlueSystemMessage(m_session, "%s is connecting from account '%s'[%u] with permissions '%s'", (plr->getGender() ? "She" : "He"), sess->GetAccountName().c_str(), sess->GetAccountId(), sess->GetPermissions());

    const char* client;

    if (sess->HasFlag(ACCOUNT_FLAG_XPACK_02) && sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        client = "TBC and WotLK";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_02))
        client = "Wrath of the Lich King";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        client = "WoW Burning Crusade";
    else
        client = "WoW";

    BlueSystemMessage(m_session, "%s uses %s (build %u)", (plr->getGender() ? "She" : "He"), client, sess->GetClientBuild());

    BlueSystemMessage(m_session, "%s IP is '%s', and has a latency of %ums", (plr->getGender() ? "Her" : "His"), sess->GetSocket()->GetRemoteIP().c_str(), sess->GetLatency());

    return true;
}

//.unban ip
bool ChatHandler::HandleIPUnBanCommand(const char* args, WorldSession* m_session)
{
    std::string pIp = args;
    if (pIp.length() == 0)
        return false;

    if (pIp.find("/") == std::string::npos)
    {
        RedSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        pIp.append("/32");
    }

    SystemMessage(m_session, "Deleting [%s] from ip ban table if it exists", pIp.c_str());
    sLogonCommHandler.removeIpBan(pIp.c_str());
    sGMLog.writefromsession(m_session, "unbanned ip address %s", pIp.c_str());
    return true;
}

//.unban character
bool ChatHandler::HandleUnBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char Character[255];
    if (sscanf(args, "%s", Character) != 1)
    {
        RedSystemMessage(m_session, "A character name and reason is required.");
        return true;
    }

    Player* pPlayer = ObjectMgr::getSingleton().GetPlayer(Character, false);
    if (pPlayer != nullptr)
    {
        GreenSystemMessage(m_session, "Unbanned player %s ingame.", pPlayer->GetName());
        pPlayer->UnSetBanned();
    }
    else
    {
        GreenSystemMessage(m_session, "Player %s not found ingame.", Character);
    }

    CharacterDatabase.Execute("UPDATE characters SET banned = 0 WHERE name = '%s'", CharacterDatabase.EscapeString(std::string(Character)).c_str());

    SystemMessage(m_session, "Unbanned character %s in database.", Character);
    sGMLog.writefromsession(m_session, "unbanned %s", Character);

    return true;
}

void ParseBanArgs(char* args, char** BanDuration, char** BanReason)
{
    char* pBanDuration = strchr(args, ' ');
    char* pReason = NULL;
    if (pBanDuration != NULL)
    {
        if (isdigit(*(pBanDuration + 1)))       // this is the duration of the ban
        {
            *pBanDuration = 0;                  // NULL-terminate the first string (character/account/ip)
            ++pBanDuration;                     // point to next arg
            pReason = strchr(pBanDuration + 1, ' ');
            if (pReason != NULL)                // BanReason is OPTIONAL
            {
                *pReason = 0;                   // BanReason was given, so NULL-terminate the duration string
                ++pReason;                      // and point to the ban reason
            }
        }
        else                                    // no duration was given (didn't start with a digit) - so this arg must be ban reason and duration defaults to permanent
        {
            pReason = pBanDuration;
            pBanDuration = NULL;
            *pReason = 0;
            ++pReason;
        }
    }
    *BanDuration = pBanDuration;
    *BanReason = pReason;
}

//.ban ip
bool ChatHandler::HandleIPBanCommand(const char* args, WorldSession* m_session)
{
    char* pIp = (char*)args;
    char* pReason;
    char* pDuration;
    ParseBanArgs(pIp, &pDuration, &pReason);

    uint32_t timeperiod = 0;
    if (pDuration != NULL)
    {
        timeperiod = Util::GetTimePeriodFromString(pDuration);
        if (timeperiod == 0)
            return false;
    }

    uint32 o1, o2, o3, o4;
    if (sscanf(pIp, "%3u.%3u.%3u.%3u", (unsigned int*)&o1, (unsigned int*)&o2, (unsigned int*)&o3, (unsigned int*)&o4) != 4
        || o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255)
    {
        RedSystemMessage(m_session, "Invalid IPv4 address [%s]", pIp);
        return true;    // error in syntax, but we wont remind client of command usage
    }

    time_t expire_time;

    if (timeperiod == 0)        // permanent ban
        expire_time = 0;
    else
        expire_time = UNIXTIME + (time_t)timeperiod;

    std::string IP = pIp;
    std::string::size_type i = IP.find("/");

    if (i == std::string::npos)
    {
        RedSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        IP.append("/32");
    }

    char emptystring = 0;
    if (pReason == NULL)
        pReason = &emptystring;

    SystemMessage(m_session, "Adding [%s] to IP ban table, expires %s.Reason is :%s", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time), pReason);
    sLogonCommHandler.addIpBan(IP.c_str(), (uint32)expire_time, pReason);
    sWorld.disconnectSessionByIp(IP.substr(0, IP.find("/")).c_str(), m_session);
    sGMLog.writefromsession(m_session, "banned ip address %s, expires %s", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time));

    return true;
}

//.ban character
bool ChatHandler::HandleBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char* pCharacter = (char*)args;
    PlayerInfo* pInfo = NULL;
    char* pReason;
    char* pDuration;
    uint32_t BanTime = 0;
    ParseBanArgs(pCharacter, &pDuration, &pReason);
    if (pDuration != NULL)
    {
        BanTime = Util::GetTimePeriodFromString(pDuration);
        if (BanTime == 0)
            return false;
    }

    Player* pPlayer = objmgr.GetPlayer(pCharacter, false);
    if (pPlayer == NULL)
    {
        pInfo = objmgr.GetPlayerInfoByName(pCharacter);
        if (pInfo == NULL)
        {
            SystemMessage(m_session, "Player not found.");
            return true;
        }
        SystemMessage(m_session, "Banning player '%s' in database for '%s'.", pCharacter, (pReason == NULL) ? "No reason." : pReason);
        std::string escaped_reason = (pReason == NULL) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason));
        CharacterDatabase.Execute("UPDATE characters SET banned = %u, banReason = '%s' WHERE guid = %u",
            BanTime ? BanTime + (uint32)UNIXTIME : 1, escaped_reason.c_str(), pInfo->guid);
    }
    else
    {
        SystemMessage(m_session, "Banning player '%s' ingame for '%s'.", pCharacter, (pReason == NULL) ? "No reason." : pReason);
        std::string sReason = (pReason == NULL) ? "No Reason." : std::string(pReason);
        uint32 uBanTime = BanTime ? BanTime + (uint32)UNIXTIME : 1;
        pPlayer->SetBanned(uBanTime, sReason);
        pInfo = pPlayer->getPlayerInfo();
    }
    SystemMessage(m_session, "This ban is due to expire %s%s.", BanTime ? "on " : "", BanTime ? Util::GetDateTimeStringFromTimeStamp(BanTime + (uint32)UNIXTIME).c_str() : "Never");

    sGMLog.writefromsession(m_session, "banned %s, reason %s, for %s", pCharacter, (pReason == NULL) ? "No reason" : pReason, BanTime ? Util::GetDateStringFromSeconds(BanTime).c_str() : "ever");

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_RED << "GM: " << pCharacter << " has been banned by " << m_session->GetPlayer()->GetName() << " for ";
    worldAnnounce << (BanTime ? Util::GetDateStringFromSeconds(BanTime) : "ever") << " Reason: " << ((pReason == NULL) ? "No reason." : pReason);
    sWorld.sendMessageToAll(worldAnnounce.str());

    if (sWorld.settings.log.enableSqlBanLog && pInfo)
    {
        CharacterDatabase.Execute("INSERT INTO `banned_char_log` VALUES('%s', '%s', %u, %u, '%s')", m_session->GetPlayer()->GetName(), pInfo->name, (uint32)UNIXTIME, (uint32)UNIXTIME + BanTime, (pReason == NULL) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason)).c_str());
    }

    if (pPlayer)
    {
        SystemMessage(m_session, "Kicking %s.", pPlayer->GetName());
        pPlayer->Kick();
    }

    return true;
}

//.ban all
bool ChatHandler::HandleBanAllCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* pBanned;
    std::string pAcc;
    std::string pIP;
    std::string pArgs = args;
    char* pCharacter = (char*)args;
    char* pReason;
    char* pDuration;
    ParseBanArgs(pCharacter, &pDuration, &pReason);
    uint32_t BanTime = 0;

    if (pDuration != NULL)
    {
        BanTime = Util::GetTimePeriodFromString(pDuration);
        if (BanTime == 0)
            return false;
    }

    pBanned = objmgr.GetPlayer(pCharacter, false);
    if (!pBanned || !pBanned->IsInWorld())
    {
        RedSystemMessage(m_session, "Player \'%s\' is not online or does not exists!", pCharacter);
        return true;
    }

    if (pBanned == m_session->GetPlayer())
    {
        RedSystemMessage(m_session, "You cannot ban yourself!");
        return true;
    }

    if (pBanned->GetSession() == NULL)
    {
        RedSystemMessage(m_session, "Player does not have a session!");
        return true;
    }

    if (pBanned->GetSession()->GetSocket() == NULL)
    {
        RedSystemMessage(m_session, "Player does not have a socket!");
        return true;
    }

    pAcc = pBanned->GetSession()->GetAccountName();
    pIP = pBanned->GetSession()->GetSocket()->GetRemoteIP();
    if (pIP == m_session->GetSocket()->GetRemoteIP())
    {
        RedSystemMessage(m_session, "That player has the same IP as you - ban failed");
        return true;
    }

    HandleBanCharacterCommand(pArgs.c_str(), m_session);
    char pIPCmd[256];
    snprintf(pIPCmd, 254, "%s %s %s", pIP.c_str(), pDuration, pReason);
    HandleIPBanCommand(pIPCmd, m_session);
    char pAccCmd[256];
    snprintf(pAccCmd, 254, "%s %s %s", pAcc.c_str(), pDuration, pReason);
    HandleAccountBannedCommand((const char*)pAccCmd, m_session);

    return true;
}

