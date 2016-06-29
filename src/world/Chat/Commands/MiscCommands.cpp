/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

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
                    auto kill_spell = dbcSpell.LookupEntryForced(5);
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
            player_target->SetMovement(MOVE_UNROOT, 1);
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

    unit->Root();

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

    unit->Unroot();

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

        if (sWorld.gamemaster_announceKick)
        {
            char msg[200];
            snprintf(msg, 200, "%sGM: %s was kicked from the server by %s. Reason: %s", MSG_COLOR_RED, player_target->GetName(), m_session->GetPlayer()->GetName(), kickreason.c_str());
            sWorld.SendWorldText(msg, NULL);
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

        sWorld.DisconnectUsersWithAccount(args, m_session);
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

    sWorld.DisconnectUsersWithIP(args, m_session);
    sGMLog.writefromsession(m_session, "kicked players with IP %s", args);

    return true;
}
