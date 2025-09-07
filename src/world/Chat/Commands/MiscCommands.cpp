/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommand.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/Battleground/Battleground.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/WorldSocket.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Spell/Spell.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"

// .command
bool ChatCommandHandler::handleCommandsCommand(const char* args, WorldSession* m_session)
{
    std::string output;
    uint32_t count = 0;

    output = "Available commands: \n\n";

    for (const auto table : sCommandTableStorage.getCommandRegistry())
    {
        std::string top;
        if (*args && !resolveTopLevelAbbrev(args, m_session, top))
            continue;

        if (table.commandPermission == "z")
        {
            output += "|cffff6060";
            output += table.command;
            output += "|r, ";
        }
        else if (table.commandPermission == "m")
        {
            output += "|cff00ffff";
            output += table.command;
            output += "|r, ";
        }
        else if (table.commandPermission == "c")
        {
            output += "|cff00ff00";
            output += table.command;
            output += "|r, ";
        }
        else
        {
            output += "|cff00ccff";
            output += table.command;
            output += "|r, ";
        }

        count++;
        if (count == 5)  // 5 per line
        {
            output += "\n";
            count = 0;
        }
    }
    if (count)
        output += "\n";

    SendMultilineMessage(m_session, output.c_str());

    return true;
}

bool ChatCommandHandler::showHelpForCommand(WorldSession* m_session, const char* args)
{
    auto &reg = sCommandTableStorage.getCommandRegistry();

    // normalize input: strip leading spaces and an optional dot
    std::string_view sv = args ? std::string_view(args) : std::string_view{};
    size_t i = 0;

    while (i < sv.size() && std::isspace(static_cast<unsigned char>(sv[i])))
        ++i;

    // be consequent if you allow '.' or '!' in the normalization, you need to allow it here too 
    if (i < sv.size() && (sv[i] == '.' || sv[i] == '!'))
        ++i;

    while (i < sv.size() && std::isspace(static_cast<unsigned char>(sv[i])))
        ++i;

    sv.remove_prefix(i);
    if (sv.empty())
        return false;

    // split input into tokens (views)
    std::vector<std::string_view> tokens;
    {
        size_t p = 0;
        while (p < sv.size())
        {
            while (p < sv.size() && std::isspace(static_cast<unsigned char>(sv[p])))
                ++p;

            if (p >= sv.size())
                break;

            size_t q = p;
            while (q < sv.size() && !std::isspace(static_cast<unsigned char>(sv[q])))
                ++q;

            tokens.emplace_back(sv.substr(p, q - p));
            p = q;
        }
    }

    if (tokens.empty())
        return false;

    // resolve ONLY the first token to a top-level command
    std::string top;
    if (!resolveTopLevelAbbrev(tokens[0], m_session, top))
        return false; // unknown/ambiguous top-level

    auto lo = [](char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        };

    auto istarts_with = [&](std::string_view a, std::string_view b) {
        if (b.size() > a.size())
            return false;

        for (size_t k = 0; k < b.size(); ++k)
            if (lo(a[k]) != lo(b[k]))
                return false;

        return true;
    };

    // exact depth we want help for (do NOT auto-descend)
    const size_t wantedDepth = tokens.size();

    // Try to find an entry exactly at that depth: first word == resolved top-level,
    // the remaining words are matched by abbreviation.
    const ChatCommand* match = nullptr;
    std::string matchCmd;

    for (const auto& e : reg)
    {
        if (e.command.empty())
            continue;

        // split command into words
        std::vector<std::string> words;
        {
            std::istringstream is(e.command);
            for (std::string w; is >> w; )
                words.push_back(std::move(w));
        }

        if (words.size() != wantedDepth)
            continue;     // exact depth only

        if (words.empty())
            continue;

        // first word must equal resolved top (case-insensitive)
        if (words[0].size() != top.size())
        {
            // handled below by per-char compare anyway
        }
        {
            bool eq = true;
            for (size_t k=0; k<top.size(); ++k)
                if (lo(words[0][k]) != lo(top[k])) { eq = false; break; }
            if (!eq) continue;
        }

        // remaining words matched by abbreviation
        bool ok = true;
        for (size_t w = 1; w < wantedDepth; ++w)
        {
            if (!istarts_with(words[w], tokens[w]))
            {
                ok = false;
                break;
            }
        }

        if (!ok)
            continue;

        const char perm = e.commandPermission.empty() ? '0' : e.commandPermission[0];
        if (perm != '0' && !m_session->CanUseCommand(perm))
            continue;

        match = &e;
        matchCmd = e.command;
        break;
    }

    if (match)
    {
        // exact-depth entry exists → show its help (or say there is none)
        if (!match->help.empty())
            SendMultilineMessage(m_session, match->help.c_str());
        else
            systemMessage(m_session, "There is no help for that command");

        return true;
    }

    // No exact-depth entry → treat as a node: list children under that prefix.
    // Build the typed prefix string: <top> + the typed (abbrev) tokens joined by spaces.
    std::string typedPrefix = top;
    for (size_t t = 1; t < tokens.size(); ++t)
    {
        typedPrefix.push_back(' ');
        typedPrefix.append(tokens[t].data(), tokens[t].size());
    }

    // But for listing children we need the *resolved* prefix (first word = resolved top),
    // without auto-resolving further words. So children start with "<top> ".
    const std::string listPrefix = top + ' ';

    bool any = false;
    for (const auto &cmd : reg)
    {
        if (cmd.command.size() <= listPrefix.size())
            continue;

        // starts with "<top> " (case-insensitive)
        bool starts = true;
        for (size_t k = 0; k < listPrefix.size(); ++k)
        {
            if (lo(cmd.command[k]) != lo(listPrefix[k]))
            {
                starts = false;
                break;
            }
        }

        if (!starts)
            continue;

        const char perm = cmd.commandPermission.empty() ? '0' : cmd.commandPermission[0];
        if (perm != '0' && !m_session->CanUseCommand(perm))
            continue;

        // show only the remainder after "<top> "
        auto help = cmd.help.empty() ? "No Help Available" : cmd.help;

        blueSystemMessage(m_session, " {} - {}", cmd.command.substr(listPrefix.size()), help);

        any = true;
    }

    if (!any)
        systemMessage(m_session, "There is no help for that command");
    else
        greenSystemMessage(m_session, "Available Subcommands:");

    return true;
}

// .help
bool ChatCommandHandler::handleHelpCommand(const char* args, WorldSession* m_session)
{
    if (!args || !*args)
        return false;

    if (!showHelpForCommand(m_session, args))
        redSystemMessage(m_session, "Sorry, no help was found for this command, or that command does not exist.");

    return true;
}

//.mount
bool ChatCommandHandler::HandleMountCommand(const char* args, WorldSession* m_session)
{
    Unit* unit_target = GetSelectedUnit(m_session, true);
    if (unit_target == nullptr)
        return true;

    if (!args)
    {
        redSystemMessage(m_session, "No model specified!");
        return true;
    }

    uint32_t modelid = std::stoul(args);
    if (!modelid)
    {
        redSystemMessage(m_session, "No model specified!");
        return true;
    }

    if (unit_target->getMountDisplayId() != 0)
    {
        redSystemMessage(m_session, "Target is already mounted.");
        return true;
    }

    unit_target->setMountDisplayId(modelid);

    blueSystemMessage(m_session, "Now mounted with model {}.", modelid);
    sGMLog.writefromsession(m_session, "used mount command to model %u", modelid);

    return true;
}

//.dismount
bool ChatCommandHandler::HandleDismountCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit_target = GetSelectedUnit(m_session, true);
    if (unit_target == nullptr)
        return true;

    if (unit_target->getMountDisplayId() == 0)
    {
        redSystemMessage(m_session, "Target is not mounted.");
        return true;
    }

    if (unit_target->isPlayer())
        static_cast<Player*>(unit_target)->dismount();

    unit_target->setMountDisplayId(0);

    blueSystemMessage(m_session, "Target is now unmounted.");
    return true;
}

//.gocreature
bool ChatCommandHandler::HandleGoCreatureSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32_t spawn_id;
    if (sscanf(args, "%u", &spawn_id) != 1)
    {
        redSystemMessage(m_session, "Command must be in format: .gocreature <creature_spawnid>.");
        return true;
    }

    for (const auto& creatureSpawnMap : sMySQLStore._creatureSpawnsStore)
    {
        for (const auto& creatureSpawn : creatureSpawnMap)
        {
            if (creatureSpawn->id == spawn_id)
            {
                m_session->GetPlayer()->safeTeleport(creatureSpawn->mapId, 0, LocationVector(creatureSpawn->x, creatureSpawn->y, creatureSpawn->z));
                return true;
            }
        }
    }

    redSystemMessage(m_session, "No creature found in creature_spawns table with id {}.", spawn_id);
    return true;
}

//.gogameobject
bool ChatCommandHandler::HandleGoGameObjectSpawnCommand(const char* args, WorldSession* m_session)
{
    uint32_t spawn_id;
    if (sscanf(args, "%u", &spawn_id) != 1)
    {
        redSystemMessage(m_session, "Command must be in format: .gogameobject <gameobject_spawnid>.");
        return true;
    }

    for (const auto& goSpawnMap : sMySQLStore._gameobjectSpawnsStore)
    {
        for (const auto& goSpawn : goSpawnMap)
        {
            if (goSpawn->id == spawn_id)
            {
                m_session->GetPlayer()->safeTeleport(goSpawn->map, 0, LocationVector(goSpawn->spawnPoint.x, goSpawn->spawnPoint.y, goSpawn->spawnPoint.z));
                return true;
            }
        }
    }

    redSystemMessage(m_session, "No gameobject found in gameobject_spawns table with id {}.", spawn_id);
    return true;
}

//.gostartlocation
bool ChatCommandHandler::HandleGoStartLocationCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    std::string race;
    uint8_t raceid = 0;
    uint8_t classid = 0;

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
#if VERSION_STRING >= Cata
        else if (race == "goblin")
            raceid = 9;
#endif
        else if (race == "bloodelf")
            raceid = 10;
        else if (race == "draenei")
            raceid = 11;
        else if (race == "deathknight")
            classid = 6;
#if VERSION_STRING >= Cata
        else if (race == "worgen")
            raceid = 22;
#endif
        else
        {
#if VERSION_STRING >= Cata
            redSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, goblin, tauren, undead, bloodelf, worgen, deathknight");
#else
            redSystemMessage(m_session, "Invalid start location! Valid locations are: human, dwarf, gnome, nightelf, draenei, orc, troll, tauren, undead, bloodelf, deathknight");
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
    for (uint8_t i = 1; i <= 11; ++i)
    {
        player_info = sMySQLStore.getPlayerCreateInfo((raceid ? raceid : i), (classid ? classid : i));
        if (player_info != nullptr)
            break;
    }

    if (player_info == nullptr)
    {
        redSystemMessage(m_session, "Internal error: Could not find create info for race {} and class {}.", raceid, classid);
        return false;
    }

    greenSystemMessage(m_session, "Teleporting {} to {} starting location.", player_target->getName(), race);

    player_target->safeTeleport(player_info->mapId, 0, LocationVector(player_info->positionX, player_info->positionY, player_info->positionZ));
    return true;
}

//.gotrig
bool ChatCommandHandler::HandleGoTriggerCommand(const char* args, WorldSession* m_session)
{
    uint32_t trigger_id;
    int32_t instance_id = 0;

    if (sscanf(args, "%u %d", &trigger_id, &instance_id) < 1)
    {
        redSystemMessage(m_session, "Command must be at least in format: .gotrig <trigger_id>.");
        redSystemMessage(m_session, "You can use: .gotrig <trigger_id> <instance_id>");
        return true;
    }

    auto area_trigger_entry = sAreaTriggerStore.lookupEntry(trigger_id);
    if (area_trigger_entry == nullptr)
    {
        redSystemMessage(m_session, "Could not find trigger {}", args);
        return true;
    }

    m_session->GetPlayer()->safeTeleport(area_trigger_entry->mapid, instance_id, LocationVector(area_trigger_entry->x, area_trigger_entry->y, area_trigger_entry->z, area_trigger_entry->box_radius));
    blueSystemMessage(m_session, "Teleported to trigger {} on [{}][{}][{}][{}]", area_trigger_entry->id, area_trigger_entry->mapid, area_trigger_entry->x, area_trigger_entry->y, area_trigger_entry->z);
    return true;
}

//.kill - kills target or player with <name>
bool ChatCommandHandler::HandleKillCommand(const char* args, WorldSession* m_session)
{
    bool is_name_set = false;
    auto unit_target = GetSelectedUnit(m_session);

    if (*args)
        is_name_set = true;

    if (is_name_set)
    {
        auto named_player = sObjectMgr.getPlayer(args, false);
        if (named_player == nullptr)
        {
            redSystemMessage(m_session, "Player {} is not online or does not exist!", args);
            return true;
        }
        named_player->die(nullptr, 0, 0);
        redSystemMessage(named_player->getSession(), "You were killed by {} with a GM command.", m_session->GetPlayer()->getName());
        greenSystemMessage(m_session, "Killed player {}.", args);
        sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid: %s ", named_player->getName().c_str(), std::to_string(named_player->getGuid()).c_str());
    }
    else
    {
        if (unit_target != nullptr)
        {
            switch (unit_target->getObjectTypeId())
            {
                case TYPEID_UNIT:
                {
                    auto creature = static_cast<Creature*>(unit_target);
                    auto kill_spell = sSpellMgr.getSpellInfo(5);
                    if (kill_spell == nullptr)
                        return true;

                    SpellCastTargets targets(unit_target->getGuid());
                    Spell* spell = sSpellMgr.newSpell(m_session->GetPlayer(), kill_spell, true, 0);
                    spell->prepare(&targets);

                    greenSystemMessage(m_session, "Killed Creature {}.", creature->GetCreatureProperties()->Name);
                    sGMLog.writefromsession(m_session, "used kill command on Creature %u [%s], spawn ID: %u", creature->getEntry(), creature->GetCreatureProperties()->Name.c_str(), creature->spawnid);
                    break;
                }
                case TYPEID_PLAYER:
                {
                    auto player = static_cast<Player*>(unit_target);

                    player->setHealth(0);
                    player->die(nullptr, 0, 0);
                    redSystemMessage(player->getSession(), "You were killed by {} with a GM command.", m_session->GetPlayer()->getName());
                    greenSystemMessage(m_session, "Killed player {}.", player->getName());
                    sGMLog.writefromsession(m_session, "used kill command on Player Name: %s Guid: %s", m_session->GetPlayer()->getName().c_str(), std::to_string(player->getGuid()).c_str());
                    break;
                }
                default:
                    redSystemMessage(m_session, "Something went wrong while killing a unit with this command!");
                    break;

            }
        }
        else
        {
            redSystemMessage(m_session, "No selected target or player name set.");
            redSystemMessage(m_session, "Use .kill on an selected unit (player/creature)");
            redSystemMessage(m_session, "Use .kill <playername> to kill players");
            return true;
        }

    }

    return true;
}

//.revive - revives self or player with <name>
// if player is not dead, player is revived to full health and mana
bool ChatCommandHandler::HandleReviveCommand(const char* args, WorldSession* m_session)
{
    Player* reviveTarget = nullptr;
    if (*args)
        reviveTarget = sObjectMgr.getPlayer(args, false);
    else
        reviveTarget = GetSelectedPlayer(m_session, false, true);

    if (reviveTarget == nullptr)
    {
        redSystemMessage(m_session, "Player not found!");
        return true;
    }

    const auto revivedSelf = reviveTarget == m_session->GetPlayer();
    if (reviveTarget->isDead())
    {
        reviveTarget->setMoveRoot(false);
        reviveTarget->resurrect();

        if (!revivedSelf)
            greenSystemMessage(m_session, "Player {} revived.", reviveTarget->getName());
    }

    reviveTarget->setFullHealthMana();

    // Write to GM log
    if (revivedSelf)
        sGMLog.writefromsession(m_session, "revived himself");
    else
        sGMLog.writefromsession(m_session, "revived player %s", reviveTarget->getName().c_str());

    return true;
}

//.root
bool ChatCommandHandler::HandleRootCommand(const char* /*args*/, WorldSession* m_session)
{
    Unit* unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    unit->setMoveRoot(true);

    if (unit->isPlayer())
    {
        systemMessage(m_session, "Rooting Player {}.", static_cast<Player*>(unit)->getName());
        blueSystemMessage(static_cast<Player*>(unit)->getSession(), "You have been rooted by {}.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "rooted player %s", static_cast<Player*>(unit)->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Rooting Creature {}.", static_cast<Creature*>(unit)->GetCreatureProperties()->Name);
        sGMLog.writefromsession(m_session, "rooted creature %s", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
    }

    return true;
}

//.unroot
bool ChatCommandHandler::HandleUnrootCommand(const char* /*args*/, WorldSession* m_session)
{
    auto unit = GetSelectedUnit(m_session, true);
    if (unit == nullptr)
        return true;

    unit->setMoveRoot(false);

    if (unit->isPlayer())
    {
        systemMessage(m_session, "Unrooting Player {}.", static_cast<Player*>(unit)->getName());
        blueSystemMessage(static_cast<Player*>(unit)->getSession(), "You have been unrooted by {}.", m_session->GetPlayer()->getName());
        sGMLog.writefromsession(m_session, "unrooted player %s", static_cast<Player*>(unit)->getName().c_str());
    }
    else
    {
        blueSystemMessage(m_session, "Unrooting Creature {}.", static_cast<Creature*>(unit)->GetCreatureProperties()->Name);
        sGMLog.writefromsession(m_session, "unrooted creature %s", static_cast<Creature*>(unit)->GetCreatureProperties()->Name.c_str());
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// .kick commands
//.kick player
bool ChatCommandHandler::HandleKickByNameCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "A player name is required!");
        redSystemMessage(m_session, "Usage: .kick player <player_name>");
        redSystemMessage(m_session, "Optional: .kick player <player_name> <reason>");
        return true;
    }

    char* player_name = strtok((char*)args, " ");
    auto player_target = sObjectMgr.getPlayer(player_name, false);
    if (player_target != nullptr)
    {
        char* reason = strtok(nullptr, "\n");
        std::string kickreason = "No reason";

        if (reason)
            kickreason = reason;

        blueSystemMessage(m_session, "Attempting to kick {} from the server for '{}'.", player_target->getName(), kickreason);
        sGMLog.writefromsession(m_session, "Kicked player %s from the server for %s", player_target->getName().c_str(), kickreason.c_str());

        if (!m_session->CanUseCommand('z') && player_target->getSession()->CanUseCommand('z'))
        {
            redSystemMessage(m_session, "You cannot kick {}, a GM whose permissions outrank yours.", player_target->getName());
            return true;
        }

        if (worldConfig.gm.worldAnnounceOnKickPlayer)
        {
            std::stringstream worldAnnounce;
            worldAnnounce << MSG_COLOR_RED << "GM: " << player_target->getName().c_str() << " was kicked from the server by " << m_session->GetPlayer()->getName().c_str() << ". Reason: " << kickreason;
            sWorld.sendMessageToAll(worldAnnounce.str(), nullptr);
        }

        systemMessage(player_target->getSession(), "You are being kicked from the server by {}. Reason: {}", m_session->GetPlayer()->getName(), kickreason);
        player_target->kickFromServer(6000);
        return true;
    }
    redSystemMessage(m_session, "Player is not online at the moment.");

    return true;
}

//.kick account
bool ChatCommandHandler::HandleKKickBySessionCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "A account name is required!");
        redSystemMessage(m_session, "Usage: .kick account <account_name>");
        return true;
    }

    char* player_name = strtok((char*)args, " ");
    auto player_target = sObjectMgr.getPlayer(player_name, false);
    if (player_target != nullptr)
    {
        if (!m_session->CanUseCommand('z') && player_target->getSession()->CanUseCommand('z'))
        {
            redSystemMessage(m_session, "You cannot kick {}, a GM whose permissions outrank yours.", player_target->getName());
            return true;
        }

        sWorld.disconnectSessionByAccountName(args, m_session);
        sGMLog.writefromsession(m_session, "kicked player with account %s", args);
    }
    else
    {
        redSystemMessage(m_session, "Player is not online at the moment.");
    }

    return true;
}

//.kick ip
bool ChatCommandHandler::HandleKickByIPCommand(const char* args, WorldSession* m_session)
{
    if (!args || strlen(args) < 8)
    {
        redSystemMessage(m_session, "An IP is required!");
        redSystemMessage(m_session, "Usage: .kick ip <AN.IP.ADD.RES>");
        return true;
    }

    sWorld.disconnectSessionByIp(args, m_session);
    sGMLog.writefromsession(m_session, "kicked players with IP %s", args);

    return true;
}

//.worldport
bool ChatCommandHandler::HandleWorldPortCommand(const char* args, WorldSession* m_session)
{
    float x, y, z, o = 0.0f;
    uint32_t mapid;

    if (sscanf(args, "%u %f %f %f %f", &mapid, &x, &y, &z, &o) < 4)
    {
        redSystemMessage(m_session, "You have to use at least .worldport <mapid> <x> <y> <z>");
        return true;
    }

    if (x >= Map::Terrain::_maxX || x <= Map::Terrain::_minX || y <= Map::Terrain::_minY || y >= Map::Terrain::_maxY)
    {
        redSystemMessage(m_session, "<x> <y> value is out of range!");
        return true;
    }

    LocationVector vec(x, y, z, o);
    m_session->GetPlayer()->safeTeleport(mapid, 0, vec);

    return true;
}

//.gps
bool ChatCommandHandler::HandleGPSCommand(const char* args, WorldSession* m_session)
{
    Object* obj;
    uint64_t guid = m_session->GetPlayer()->getTargetGuid();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->getWorldMap()->getUnit(guid)) == 0)
        {
            systemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    auto at = obj->GetArea();
    if (!at)
    {
        systemMessage(m_session, "Current Position: Map:{} X:{} Y:{} Z:{} Orientation:{}", obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
        return true;
    }

    auto out_map_id = obj->GetMapId();
    auto out_zone_id = at->zone; // uint32_t at_old->ZoneId
    auto out_area_id = at->id; // uint32_t at_old->AreaId
    auto out_phase = obj->GetPhase();
    auto out_x = obj->GetPositionX();
    auto out_y = obj->GetPositionY();
    auto out_z = obj->GetPositionZ();
    auto out_o = obj->GetOrientation();
#if VERSION_STRING < Cata
    auto out_area_name = at->area_name[sWorld.getDbcLocaleLanguageId()];
#else
    auto out_area_name = at->area_name[0];
#endif

    systemMessage(m_session, "Current Position: Map:{} Zone:{} Area:{} Phase:{} X:{} Y:{} Z:{} Orientation:{} Area Name: {}",
        out_map_id, out_zone_id, out_area_id, out_phase, out_x, out_y, out_z, out_o, out_area_name);

#if VERSION_STRING < Cata
    if (obj->obj_movement_info.hasMovementFlag(MOVEFLAG_TRANSPORT))
    {
        systemMessage(m_session, "Position on Transport:");
        systemMessage(m_session, "  tX: {}  tY: {}  tZ: {}  tO: {}", obj->GetTransOffsetX(), obj->GetTransOffsetY(), obj->GetTransOffsetZ(), obj->GetTransOffsetO());
    }
#endif

    // ".gps 1" will save gps info to file logs/gps.log - This probably isn't very multithread safe so don't have many gms spamming it!
    if (args != nullptr && *args == '1')
    {
        FILE* gpslog = fopen(AscEmu::Logging::getFormattedFileName("logs", "gps", false).c_str(), "at");
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
bool ChatCommandHandler::HandleInvincibleCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->m_isInvincible)
    {
        selected_player->m_isInvincible = false;

        if (selected_player != m_session->GetPlayer())
        {
            greenSystemMessage(selected_player->getSession(), "{} turns your invincibility off", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "turns invincibility off for %s", selected_player->getName().c_str());
        }
        else
        {
            greenSystemMessage(m_session, "Invincibility is now off");
        }
    }
    else
    {
        selected_player->m_isInvincible = true;

        if (selected_player != m_session->GetPlayer())
        {
            greenSystemMessage(selected_player->getSession(), "{} turns your invincibility on", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "turns invincibility on for %s", selected_player->getName().c_str());
        }
        else
        {
            greenSystemMessage(m_session, "Invincibility is now on");
        }
    }

    return true;
}

//.invisible
bool ChatCommandHandler::HandleInvisibleCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->m_isGmInvisible)
    {
        selected_player->m_isGmInvisible = false;
        selected_player->m_isInvincible = false;

        selected_player->sendFriendStatus(true);

        if (selected_player->getBattleground())
            selected_player->getBattleground()->removeInvisGM();

        if (selected_player != m_session->GetPlayer())
        {
            greenSystemMessage(selected_player->getSession(), "{} turns your invisibility and invincibility off", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "turns invisibility and invincibility off for %s", selected_player->getName().c_str());
        }
        else
        {
            greenSystemMessage(m_session, "Invisibility and Invincibility are now off");
        }
    }
    else
    {
        selected_player->m_isGmInvisible = true;
        selected_player->m_isInvincible = true;

        selected_player->sendFriendStatus(false);

        if (selected_player->getBattleground())
            selected_player->getBattleground()->addInvisGM();

        if (selected_player != m_session->GetPlayer())
        {
            greenSystemMessage(selected_player->getSession(), "{} turns your invisibility and invincibility on", m_session->GetPlayer()->getName());
            sGMLog.writefromsession(m_session, "turns invisibility and invincibility on for %s", selected_player->getName().c_str());
        }
        else
        {
            greenSystemMessage(m_session, "Invisibility and Invincibility are now on");
        }
    }

    selected_player->updateVisibility();

    return true;
}

//.announce
bool ChatCommandHandler::HandleAnnounceCommand(const char* args, WorldSession* m_session)
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
        else if (m_session->hasPermissions())
            worldAnnounce << "<GM>";
    }

    if (worldConfig.announce.showNameInAnnounce)
    {
        worldAnnounce << "|r" << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor) << "|Hplayer:";
        worldAnnounce << m_session->GetPlayer()->getName().c_str();
        worldAnnounce << "|h[";
        worldAnnounce << m_session->GetPlayer()->getName().c_str();
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
bool ChatCommandHandler::HandleWAnnounceCommand(const char* args, WorldSession* m_session)
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
        else if (m_session->hasPermissions())
            colored_widescreen_text << "<GM>";
    }

    if (worldConfig.announce.showNameInWAnnounce)
    {
        colored_widescreen_text << "|r" << worldConfig.getColorStringForNumber(worldConfig.announce.tagColor) << "[";
        colored_widescreen_text << m_session->GetPlayer()->getName().c_str();
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
bool ChatCommandHandler::HandleAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* chr = sObjectMgr.getPlayer(args, false);
    if (chr)
    {
        if (!m_session->CanUseCommand('z') && chr->isAppearingDisabled())
        {
            systemMessage(m_session, "{} has blocked other GMs from appearing to them.", chr->getName());
            return true;
        }
        if (chr->getWorldMap() == nullptr)
        {
            systemMessage(m_session, "{} is already being teleported.", chr->getName());
            return true;
        }
        systemMessage(m_session, "Appearing at {}'s location.", chr->getName());
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            systemMessage(chr->getSession(), "{} is appearing to your location.", m_session->GetPlayer()->getName());
        }

#if VERSION_STRING < Cata
        if (m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID())
            m_session->GetPlayer()->safeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        else
            m_session->GetPlayer()->safeTeleport(chr->getWorldMap(), chr->GetPosition());
#else
        m_session->GetPlayer()->safeTeleport(chr->GetMapId(), 0, chr->GetPosition());
#endif

    }
    else
    {
        systemMessage(m_session, "Player ({}) does not exist or is not logged in.", args);
    }
    return true;
}

//.blockappear
bool ChatCommandHandler::HandleBlockAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (AscEmu::Util::Strings::isEqual(args, "on"))
    {
        if (m_session->GetPlayer()->isAppearingDisabled())
        {
            blueSystemMessage(m_session, "Appear blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->disableAppearing(true);
            greenSystemMessage(m_session, "Appear blocking is now enabled");
        }
    }
    else if (AscEmu::Util::Strings::isEqual(args, "off"))
    {
        if (m_session->GetPlayer()->isAppearingDisabled())
        {
            m_session->GetPlayer()->disableAppearing(false);
            greenSystemMessage(m_session, "Appear blocking is now disabled");
        }
        else
        {
            blueSystemMessage(m_session, "Appear blocking is already disabled");
        }
    }

    return true;
}

//.summon
bool ChatCommandHandler::HandleSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    Player* chr = sObjectMgr.getPlayer(args, false);
    if (chr)
    {
        if (!m_session->CanUseCommand('z') && chr->isSummoningDisabled())
        {
            systemMessage(m_session, "{} has blocked other GMs from summoning them.", chr->getName());
            return true;
        }

        if (chr->getWorldMap() == nullptr)
        {
            systemMessage(m_session, "{} is already being teleported.", chr->getName());
            return true;
        }

        systemMessage(m_session, "You are summoning {}.", chr->getName());

        if (!m_session->GetPlayer()->m_isGmInvisible)
            systemMessage(chr->getSession(), "You are being summoned by %s.", m_session->GetPlayer()->getName().c_str());

        Player* plr = m_session->GetPlayer();
        if (plr->getWorldMap() == chr->getWorldMap())
            chr->_Relocate(plr->GetMapId(), plr->GetPosition(), false, false, plr->GetInstanceID());
        else
            sEventMgr.AddEvent(chr, &Player::eventPortToGm, plr, 0, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    }
    else
    {
        const auto pinfo = sObjectMgr.getCachedCharacterInfoByName(args);
        if (!pinfo)
        {
            systemMessage(m_session, "Player ({}) does not exist.", args);
            return true;
        }
        Player* pPlayer = m_session->GetPlayer();
        char query[512];
        snprintf((char*)&query, 512, "UPDATE characters SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->getZoneId(), pinfo->guid);
        CharacterDatabase.Execute(query);

        systemMessage(m_session, "(Offline) {} has been summoned.", pinfo->name);
        return true;
    }
    sGMLog.writefromsession(m_session, "summoned %s on map %u, %f %f %f", args, m_session->GetPlayer()->GetMapId(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    return true;
}

//.blocksummon
bool ChatCommandHandler::HandleBlockSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    if (AscEmu::Util::Strings::isEqual(args, "on"))
    {
        if (m_session->GetPlayer()->isSummoningDisabled())
        {
            blueSystemMessage(m_session, "Summon blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->disableSummoning(true);
            greenSystemMessage(m_session, "Summon blocking is now enabled");
        }
    }
    else if (AscEmu::Util::Strings::isEqual(args, "off"))
    {
        if (m_session->GetPlayer()->isSummoningDisabled())
        {
            m_session->GetPlayer()->disableSummoning(false);
            greenSystemMessage(m_session, "Summon blocking is now disabled");
        }
        else
        {
            blueSystemMessage(m_session, "Summon blocking is already disabled");
        }
    }

    return true;
}

//.playerinfo
bool ChatCommandHandler::HandlePlayerInfo(const char* args, WorldSession* m_session)
{
    Player* plr;
    if (strlen(args) >= 4)
    {
        plr = sObjectMgr.getPlayer(args, false);
        if (!plr)
        {
            redSystemMessage(m_session, "Unable to locate player {}.", args);
            return true;
        }
    }
    else
        plr = GetSelectedPlayer(m_session, true, true);

    if (!plr) return true;
    if (!plr->getSession())
    {
        redSystemMessage(m_session, "ERROR: this player hasn't got any session !");
        return true;
    }
    if (!plr->getSession()->GetSocket())
    {
        redSystemMessage(m_session, "ERROR: this player hasn't got any socket !");
        return true;
    }
    WorldSession* sess = plr->getSession();

    static const char* classes[MAX_PLAYER_CLASSES] =
    { "None", "Warrior", "Paladin", "Hunter", "Rogue", "Priest", "Death Knight", "Shaman", "Mage", "Warlock", "None", "Druid" };
#if VERSION_STRING < Cata
    // wrong for classic!
    static const char* races[11 + 1] =
    { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "None", "Blood Elf", "Draenei" };
#else
    static const char* races[DBC_NUM_RACES] =
    { "None", "Human", "Orc", "Dwarf", "Night Elf", "Undead", "Tauren", "Gnome", "Troll", "Goblin", "Blood Elf", "Draenei", "None", "None", "None", "None", "None", "None", "None", "None", "None", "None", "Worgen" };
#endif

    char playedLevel[64];
    char playedTotal[64];

    int seconds = (plr->getPlayedTime())[0];
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

    seconds = (plr->getPlayedTime())[1];
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

    greenSystemMessage(m_session, "{} is a {} {} {}", plr->getName(), (plr->getGender() ? "Female" : "Male"), races[plr->getRace()], classes[plr->getClass()]);

    blueSystemMessage(m_session, "{} has played {} at this level", (plr->getGender() ? "She" : "He"), playedLevel);
    blueSystemMessage(m_session, "and {} overall", playedTotal);

    blueSystemMessage(m_session, "{} is connecting from account '{}'[{}] with permissions '{}'", (plr->getGender() ? "She" : "He"), sess->GetAccountName(), sess->GetAccountId(), sess->GetPermissions().get());

    blueSystemMessage(m_session, "Factiontemplate: {}", plr->getFactionTemplate());

    std::string clientFlags = "WoW";

    if (sess->GetFlags() == AF_FULL_MOP)
        clientFlags += " TBC, WotLK, Cata and MoP";
    else if (sess->GetFlags() == AF_FULL_CATA)
        clientFlags += " TBC, WotLK and Cata";
    else if (sess->GetFlags() == AF_FULL_WOTLK)
        clientFlags += " TBC and WotLK";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_04))
        clientFlags += " Mists of Pandaria";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_03))
        clientFlags += " Cataclysm";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_02))
        clientFlags += " Wrath of the Lich King";
    else if (sess->HasFlag(ACCOUNT_FLAG_XPACK_01))
        clientFlags += " The Burning Crusade";

    blueSystemMessage(m_session, "{} uses {} (build {})", (plr->getGender() ? "She" : "He"), clientFlags, sess->GetClientBuild());

    blueSystemMessage(m_session, "{} IP is '{}', and has a latency of {}ms", (plr->getGender() ? "Her" : "His"), sess->GetSocket()->GetRemoteIP(), sess->GetLatency());

    return true;
}

//.unban ip
bool ChatCommandHandler::HandleIPUnBanCommand(const char* args, WorldSession* m_session)
{
    std::string pIp = args;
    if (pIp.length() == 0)
        return false;

    if (pIp.find("/") == std::string::npos)
    {
        redSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        pIp.append("/32");
    }

    systemMessage(m_session, "Deleting [{}] from ip ban table if it exists", pIp);
    sLogonCommHandler.removeIpBan(pIp.c_str());
    sGMLog.writefromsession(m_session, "unbanned ip address %s", pIp.c_str());
    return true;
}

//.unban character
bool ChatCommandHandler::HandleUnBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string character;

    std::istringstream iss(std::string{ args });

    if (!(iss >> character))
    {
        redSystemMessage(m_session, "A character name is required.");
        return true;
    }

    Player* pPlayer = sObjectMgr.getPlayer(character.c_str(), false);
    if (pPlayer != nullptr)
    {
        greenSystemMessage(m_session, "Unbanned player {} ingame.", pPlayer->getName());
        pPlayer->unsetBanned();
    }
    else
    {
        greenSystemMessage(m_session, "Player {} not found ingame.", character);
    }

    CharacterDatabase.Execute("UPDATE characters SET banned = 0 WHERE name = '%s'", CharacterDatabase.EscapeString(character).c_str());

    systemMessage(m_session, "Unbanned character {} in database.", character);
    sGMLog.writefromsession(m_session, "unbanned %s", character.c_str());

    return true;
}

void ParseBanArgs(char* args, char** BanDuration, char** BanReason)
{
    char* pBanDuration = strchr(args, ' ');
    char* pReason = nullptr;
    if (pBanDuration != nullptr)
    {
        if (isdigit(*(pBanDuration + 1)))       // this is the duration of the ban
        {
            *pBanDuration = 0;                  // nullptr-terminate the first string (character/account/ip)
            ++pBanDuration;                     // point to next arg
            pReason = strchr(pBanDuration + 1, ' ');
            if (pReason != nullptr)                // BanReason is OPTIONAL
            {
                *pReason = 0;                   // BanReason was given, so nullptr-terminate the duration string
                ++pReason;                      // and point to the ban reason
            }
        }
        else                                    // no duration was given (didn't start with a digit) - so this arg must be ban reason and duration defaults to permanent
        {
            pReason = pBanDuration;
            pBanDuration = nullptr;
            *pReason = 0;
            ++pReason;
        }
    }
    *BanDuration = pBanDuration;
    *BanReason = pReason;
}

//.ban ip
bool ChatCommandHandler::HandleIPBanCommand(const char* args, WorldSession* m_session)
{
    char* pIp = (char*)args;
    char* pReason;
    char* pDuration;
    ParseBanArgs(pIp, &pDuration, &pReason);

    uint32_t timeperiod = 0;
    if (pDuration != nullptr)
    {
        timeperiod = Util::GetTimePeriodFromString(pDuration);
        if (timeperiod == 0)
            return false;
    }

    uint32_t o1, o2, o3, o4;
    if (sscanf(pIp, "%3u.%3u.%3u.%3u", &o1, &o2, &o3, &o4) != 4
        || o1 > 255 || o2 > 255 || o3 > 255 || o4 > 255)
    {
        redSystemMessage(m_session, "Invalid IPv4 address [{}]", pIp);
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
        redSystemMessage(m_session, "Lack of CIDR address assumes a 32bit match (if you don't understand, don't worry, it worked)");
        IP.append("/32");
    }

    const std::string reason = pReason;

    systemMessage(m_session, "Adding [{}] to IP ban table, expires {}. Reason is: {}", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time), reason);
    sLogonCommHandler.addIpBan(IP.c_str(), (uint32_t)expire_time, reason.c_str());
    sWorld.disconnectSessionByIp(IP.substr(0, IP.find("/")), m_session);
    sGMLog.writefromsession(m_session, "banned ip address %s, expires %s", pIp, (expire_time == 0) ? "Never" : ctime(&expire_time));

    return true;
}

//.ban character
bool ChatCommandHandler::HandleBanCharacterCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    char* pCharacter = (char*)args;
    CachedCharacterInfo const* pInfo = nullptr;
    char* pReason;
    char* pDuration;
    uint32_t BanTime = 0;
    ParseBanArgs(pCharacter, &pDuration, &pReason);
    if (pDuration != nullptr)
    {
        BanTime = Util::GetTimePeriodFromString(pDuration);
        if (BanTime == 0)
            return false;
    }

    Player* pPlayer = sObjectMgr.getPlayer(pCharacter, false);
    if (pPlayer == nullptr)
    {
        pInfo = sObjectMgr.getCachedCharacterInfoByName(pCharacter);
        if (pInfo == nullptr)
        {
            systemMessage(m_session, "Player not found.");
            return true;
        }
        systemMessage(m_session, "Banning player '{}' in database for '{}'.", pCharacter, (pReason == nullptr) ? "No reason." : pReason);
        std::string escaped_reason = (pReason == nullptr) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason));
        CharacterDatabase.Execute("UPDATE characters SET banned = %u, banReason = '%s' WHERE guid = %u",
            BanTime ? BanTime + (uint32_t)UNIXTIME : 1, escaped_reason.c_str(), pInfo->guid);
    }
    else
    {
        systemMessage(m_session, "Banning player '{}' ingame for '{}'.", pCharacter, (pReason == nullptr) ? "No reason." : pReason);
        std::string sReason = (pReason == nullptr) ? "No Reason." : std::string(pReason);
        uint32_t uBanTime = BanTime ? BanTime + (uint32_t)UNIXTIME : 1;
        pPlayer->setBanned(uBanTime, sReason);
        pInfo = pPlayer->getPlayerInfo();
    }
    systemMessage(m_session, "This ban is due to expire {}{}.", BanTime ? "on " : "", BanTime ? Util::GetDateTimeStringFromTimeStamp(BanTime + (uint32_t)UNIXTIME) : "Never");

    sGMLog.writefromsession(m_session, "banned %s, reason %s, for %s", pCharacter, (pReason == nullptr) ? "No reason" : pReason, BanTime ? Util::GetDateStringFromSeconds(BanTime).c_str() : "ever");

    std::stringstream worldAnnounce;
    worldAnnounce << MSG_COLOR_RED << "GM: " << pCharacter << " has been banned by " << m_session->GetPlayer()->getName().c_str() << " for ";
    worldAnnounce << (BanTime ? Util::GetDateStringFromSeconds(BanTime) : "ever") << " Reason: " << ((pReason == nullptr) ? "No reason." : pReason);
    sWorld.sendMessageToAll(worldAnnounce.str());

    if (sWorld.settings.logger.enableSqlBanLog && pInfo)
    {
        CharacterDatabase.Execute("INSERT INTO `banned_char_log` VALUES('%s', '%s', %u, %u, '%s')", m_session->GetPlayer()->getName().c_str(), pInfo->name.c_str(), (uint32_t)UNIXTIME, (uint32_t)UNIXTIME + BanTime, (pReason == nullptr) ? "No reason." : CharacterDatabase.EscapeString(std::string(pReason)).c_str());
    }

    if (pPlayer)
    {
        systemMessage(m_session, "Kicking {}.", pPlayer->getName().c_str());
        pPlayer->kickFromServer();
    }

    return true;
}

//.ban all
bool ChatCommandHandler::HandleBanAllCommand(const char* args, WorldSession* m_session)
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

    if (pDuration != nullptr)
    {
        BanTime = Util::GetTimePeriodFromString(pDuration);
        if (BanTime == 0)
            return false;
    }

    pBanned = sObjectMgr.getPlayer(pCharacter, false);
    if (!pBanned || !pBanned->IsInWorld())
    {
        redSystemMessage(m_session, "Player \'{}\' is not online or does not exists!", pCharacter);
        return true;
    }

    if (pBanned == m_session->GetPlayer())
    {
        redSystemMessage(m_session, "You cannot ban yourself!");
        return true;
    }

    if (pBanned->getSession() == nullptr)
    {
        redSystemMessage(m_session, "Player does not have a session!");
        return true;
    }

    if (pBanned->getSession()->GetSocket() == nullptr)
    {
        redSystemMessage(m_session, "Player does not have a socket!");
        return true;
    }

    pAcc = pBanned->getSession()->GetAccountName();
    pIP = pBanned->getSession()->GetSocket()->GetRemoteIP();
    if (pIP == m_session->GetSocket()->GetRemoteIP())
    {
        redSystemMessage(m_session, "That player has the same IP as you - ban failed");
        return true;
    }

    HandleBanCharacterCommand(pArgs.c_str(), m_session);
    char pIPCmd[256];
    snprintf(pIPCmd, 254, "%s %s %s", pIP.c_str(), pDuration, pReason);
    HandleIPBanCommand(pIPCmd, m_session);
    char pAccCmd[256];
    snprintf(pAccCmd, 254, "%s %s %s", pAcc.c_str(), pDuration, pReason);
    handleAccountBannedCommand(pAccCmd, m_session);

    return true;
}

