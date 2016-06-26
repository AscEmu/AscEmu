/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include <Exceptions/PlayerExceptions.hpp>

initialiseSingleton(ChatHandler);
initialiseSingleton(CommandTableStorage);

ChatCommand* ChatHandler::getCommandTable()
{
    ARCEMU_ASSERT(false);
    return 0;
}

ChatCommand* CommandTableStorage::GetSubCommandTable(const char* name)
{
    if (!stricmp(name, "modify"))
        return _modifyCommandTable;
    else if (!stricmp(name, "waypoint"))
        return _waypointCommandTable;
    else if (!stricmp(name, "event"))
        return _eventCommandTable;
    else if (!stricmp(name, "debug"))
        return _debugCommandTable;
    else if (!stricmp(name, "gmTicket"))
        return _GMTicketCommandTable;
    else if (!stricmp(name, "ticket"))
        return _TicketCommandTable;
    else if (!stricmp(name, "gobject"))
        return _GameObjectCommandTable;
    else if (!stricmp(name, "battleground"))
        return _BattlegroundCommandTable;
    else if (!stricmp(name, "npc"))
        return _NPCCommandTable;
    else if (!stricmp(name, "cheat"))
        return _CheatCommandTable;
    else if (!stricmp(name, "account"))
        return _accountCommandTable;
    else if (!stricmp(name, "quest"))
        return _questCommandTable;
    else if (!stricmp(name, "pet"))
        return _petCommandTable;
    else if (!stricmp(name, "recall"))
        return _recallCommandTable;
    else if (!stricmp(name, "guild"))
        return _GuildCommandTable;
    else if (!stricmp(name, "gm"))
        return _gmCommandTable;
    else if (!stricmp(name, "server"))
        return _serverCommandTable;
    else if (!stricmp(name, "character"))
        return _characterCommandTable;
    else if (!stricmp(name, "lookup"))
        return _lookupCommandTable;
    else if (!stricmp(name, "admin"))
        return _adminCommandTable;
    else if (!stricmp(name, "kick"))
        return _kickCommandTable;
    else if (!stricmp(name, "ban"))
        return _banCommandTable;
    else if (!stricmp(name, "unban"))
        return _unbanCommandTable;
    else if (!stricmp(name, "instance"))
        return _instanceCommandTable;
    else if (!stricmp(name, "arena"))
        return _arenaCommandTable;
    else if (!stricmp(name, "achieve"))
        return _achievementCommandTable;
    else if (!stricmp(name, "vehicle"))
        return _vehicleCommandTable;
    else if (!stricmp(name, "transport"))
        return _transportCommandTable;
    return 0;
}

ChatCommand* CommandTableStorage::GetCharSubCommandTable(const char* name)
{
    if (0 == stricmp(name, "add"))
        return _characterAddCommandTable;
    if (0 == stricmp(name, "set"))
        return _characterSetCommandTable;
    if (0 == stricmp(name, "list"))
        return _characterListCommandTable;
    return nullptr;
}

ChatCommand* CommandTableStorage::GetNPCSubCommandTable(const char* name)
{
    if (0 == stricmp(name, "set"))
        return _NPCSetCommandTable;
    return nullptr;
}

ChatCommand* CommandTableStorage::GetGOSubCommandTable(const char* name)
{
    if (0 == stricmp(name, "set"))
        return _GameObjectSetCommandTable;
    return nullptr;
}

ChatCommand* CommandTableStorage::GetReloadCommandTable(const char* name)
{
    if (0 == stricmp(name, "reload"))
        return _reloadTableCommandTable;
    return nullptr;
}

#define dupe_command_table(ct, dt) this->dt = (ChatCommand*)allocate_and_copy(sizeof(ct)/* / sizeof(ct[0])*/, ct)
inline void* allocate_and_copy(uint32 len, void* pointer)
{
    void* data = (void*)malloc(len);
    memcpy(data, pointer, len);
    return data;
}

void CommandTableStorage::Load()
{
    QueryResult* result = CharacterDatabase.Query("SELECT command_name, access_level FROM command_overrides");
    if (!result) return;

    do
    {
        const char* name = result->Fetch()[0].GetString();
        const char* level = result->Fetch()[1].GetString();
        Override(name, level);
    }
    while(result->NextRow());
    delete result;
}

void CommandTableStorage::Override(const char* command, const char* level)
{
    std::stringstream command_stream(command);
    std::string main_command;
    std::string sub_command;
    std::string sec_sub_command;

    command_stream >> main_command;
    command_stream >> sub_command;
    command_stream >> sec_sub_command;

    if (sec_sub_command.empty())
    {
        if (sub_command.empty())
        {
            ChatCommand* p = &_commandTable[0];
            while (p->Name != 0)
            {
                std::string curr_table(p->Name);
                if (!curr_table.compare(main_command))
                {
                    p->CommandGroup = level[0];
                    Log.Debug("Command_Override", "Changing command level of .`%s` to %c.", main_command.c_str(), level[0]);
                    break;
                }
                ++p;
            }
        }
        else
        {
            ChatCommand* p = &_commandTable[0];
            while (p->Name != 0)
            {
                std::string curr_table(p->Name);
                if (!curr_table.compare(main_command))
                {
                    ChatCommand* p2 = p->ChildCommands;
                    while (p2->Name != 0)
                    {
                        std::string curr_subcommand(p2->Name);
                        if (!curr_subcommand.compare(sub_command))
                        {
                            p2->CommandGroup = level[0];
                            Log.Debug("Command_Override", "Changing command level of .`%s %s` to %c.", main_command.c_str(), sub_command.c_str(), level[0]);
                            break;
                        }
                        ++p2;
                    }
                }
                ++p;
            }
        }
    }
    else
    {
        ChatCommand* p = &_commandTable[0];
        while (p->Name != 0)
        {
            std::string curr_table(p->Name);
            if (!curr_table.compare(main_command))
            {
                ChatCommand* p2 = p->ChildCommands;
                while (p2->Name != 0)
                {
                    std::string curr_subcommand(p2->Name);
                    if (!curr_subcommand.compare(sub_command))
                    {
                        ChatCommand* p3 = nullptr;
                        if (0 == stricmp(main_command.c_str(), "character"))
                        {
                            if (0 == stricmp(sub_command.c_str(), "add"))
                                p3 = &_characterAddCommandTable[0];
                            else if (0 == stricmp(sub_command.c_str(), "set"))
                                p3 = &_characterSetCommandTable[0];
                            else if (0 == stricmp(sub_command.c_str(), "list"))
                                p3 = &_characterListCommandTable[0];
                        }
                        else if (0 == stricmp(main_command.c_str(), "npc"))
                        {
                            if (0 == stricmp(sub_command.c_str(), "set"))
                                p3 = &_NPCSetCommandTable[0];
                        }
                        else if (0 == stricmp(main_command.c_str(), "gameobject"))
                        {
                            if (0 == stricmp(sub_command.c_str(), "set"))
                                p3 = &_GameObjectSetCommandTable[0];
                        }
                        else if (0 == stricmp(main_command.c_str(), "server"))
                        {
                            if (0 == stricmp(sub_command.c_str(), "reload"))
                                p3 = &_reloadTableCommandTable[0];
                        }

                        if (p3 == nullptr)
                            break;

                        while (p3->Name != 0)
                        {
                            std::string curr_sec_subcommand(p3->Name);
                            if (!curr_sec_subcommand.compare(sec_sub_command))
                            {
                                p3->CommandGroup = level[0];
                                Log.Debug("Command_Override", "Changing command level of .`%s %s %s` to %c.", main_command.c_str(), sub_command.c_str(), sec_sub_command.c_str(), level[0]);
                                break;
                            }
                            ++p3;
                        }
                    }
                    ++p2;
                }
            }
            ++p;
        }
    }
}

void CommandTableStorage::Dealloc()
{
    free(_modifyCommandTable);
    free(_debugCommandTable);
    free(_eventCommandTable);
    free(_waypointCommandTable);
    free(_GMTicketCommandTable);
    free(_TicketCommandTable);
    free(_GuildCommandTable);
    free(_GameObjectCommandTable);
    free(_GameObjectSetCommandTable);
    free(_BattlegroundCommandTable);
    free(_NPCCommandTable);
    free(_NPCSetCommandTable);
    free(_CheatCommandTable);
    free(_accountCommandTable);
    free(_petCommandTable);
    free(_recallCommandTable);
    free(_questCommandTable);
    free(_serverCommandTable);
    free(_reloadTableCommandTable);
    free(_gmCommandTable);
    free(_characterCommandTable);
    free(_characterAddCommandTable);
    free(_characterSetCommandTable);
    free(_characterListCommandTable);
    free(_lookupCommandTable);
    free(_adminCommandTable);
    free(_kickCommandTable);
    free(_banCommandTable);
    free(_unbanCommandTable);
    free(_instanceCommandTable);
    free(_arenaCommandTable);
    free(_achievementCommandTable);
    free(_vehicleCommandTable);
    free(_transportCommandTable);
    free(_commandTable);
}

void CommandTableStorage::Init()
{
    static ChatCommand modifyCommandTable[] =
    {
        { "hp",              'm', NULL,                                   "Modifies health points (HP) of selected target",                  NULL, UNIT_FIELD_HEALTH,                 UNIT_FIELD_MAXHEALTH, 1 },
        { "mana",            'm', NULL,                                   "Modifies mana points (MP) of selected target.",                   NULL, UNIT_FIELD_POWER1,                 UNIT_FIELD_MAXPOWER1, 1 },
        { "rage",            'm', NULL,                                   "Modifies rage points of selected target.",                        NULL, UNIT_FIELD_POWER2,                 UNIT_FIELD_MAXPOWER2, 1 },
        { "energy",          'm', NULL,                                   "Modifies energy points of selected target.",                      NULL, UNIT_FIELD_POWER4,                 UNIT_FIELD_MAXPOWER4, 1 },
        { "runicpower",      'm', NULL,                                   "Modifies runic power points of selected target.",                 NULL, UNIT_FIELD_POWER7,                 UNIT_FIELD_MAXPOWER7, 1 },
        { "strength",        'm', NULL,                                   "Modifies the strength value of the selected target.",             NULL, UNIT_FIELD_STAT0,                  0,                    1 },
        { "agility",         'm', NULL,                                   "Modifies the agility value of the selected target.",              NULL, UNIT_FIELD_STAT1,                  0,                    1 },
        { "intelligence",    'm', NULL,                                   "Modifies the intelligence value of the selected target.",         NULL, UNIT_FIELD_STAT3,                  0,                    1 },
        { "spirit",          'm', NULL,                                   "Modifies the spirit value of the selected target.",               NULL, UNIT_FIELD_STAT4,                  0,                    1 },
        { "armor",           'm', NULL,                                   "Modifies the armor of selected target.",                          NULL, UNIT_FIELD_RESISTANCES,            0,                    1 },
        { "holy",            'm', NULL,                                   "Modifies the holy resistance of selected target.",                NULL, UNIT_FIELD_RESISTANCES + 1,         0,                    1 },
        { "fire",            'm', NULL,                                   "Modifies the fire resistance of selected target.",                NULL, UNIT_FIELD_RESISTANCES + 2,         0,                    1 },
        { "nature",          'm', NULL,                                   "Modifies the nature resistance of selected target.",              NULL, UNIT_FIELD_RESISTANCES + 3,         0,                    1 },
        { "frost",           'm', NULL,                                   "Modifies the frost resistance of selected target.",               NULL, UNIT_FIELD_RESISTANCES + 4,         0,                    1 },
        { "shadow",          'm', NULL,                                   "Modifies the shadow resistance of selected target.",              NULL, UNIT_FIELD_RESISTANCES + 5,         0,                    1 },
        { "arcane",          'm', NULL,                                   "Modifies the arcane resistance of selected target.",              NULL, UNIT_FIELD_RESISTANCES + 6,         0,                    1 },
        { "damage",          'm', NULL,                                   "Modifies the damage done by the selected target.",                NULL, UNIT_FIELD_MINDAMAGE,              UNIT_FIELD_MAXDAMAGE, 2 },
        { "ap",              'm', NULL,                                   "Modifies the attack power of the selected target.",               NULL, UNIT_FIELD_ATTACK_POWER,           0,                    1 },
        { "rangeap",         'm', NULL,                                   "Modifies the range attack power of the selected target.",         NULL, UNIT_FIELD_RANGED_ATTACK_POWER,    0,                    1 },
        { "scale",           'm', NULL,                                   "Modifies the scale of the selected target.",                      NULL, OBJECT_FIELD_SCALE_X,              0,                    2 },
        { "nativedisplayid", 'm', NULL,                                   "Modifies the native display identifier of the target.",           NULL, UNIT_FIELD_NATIVEDISPLAYID,        0,                    1 },
        { "displayid",       'm', NULL,                                   "Modifies the display identifier (DisplayID) of the target.",      NULL, UNIT_FIELD_DISPLAYID,              0,                    1 },
        { "flags",           'm', NULL,                                   "Modifies the flags of the selected target.",                      NULL, UNIT_FIELD_FLAGS,                  0,                    1 },
        { "faction",         'm', NULL,                                   "Modifies the faction template of the selected target.",           NULL, UNIT_FIELD_FACTIONTEMPLATE,        0,                    1 },
        { "dynamicflags",    'm', NULL,                                   "Modifies the dynamic flags of the selected target.",              NULL, UNIT_DYNAMIC_FLAGS,                0,                    1 },
        { "happiness",       'm', NULL,                                   "Modifies the happiness value of the selected target.",            NULL, UNIT_FIELD_POWER5,                 UNIT_FIELD_MAXPOWER5, 1 },
        { "boundingraidius", 'm', NULL,                                   "Modifies the bounding radius of the selected target.",            NULL, UNIT_FIELD_BOUNDINGRADIUS,         0,                    2 },
        { "combatreach",     'm', NULL,                                   "Modifies the combat reach of the selected target.",               NULL, UNIT_FIELD_COMBATREACH,            0,                    2 },
        { "emotestate",       'm', NULL,                                   "Modifies the Unit emote state of the selected target.",           NULL, UNIT_NPC_EMOTESTATE,               0,                    1 },
        { "bytes0",          'm', NULL,                                   "WARNING! Modifies the bytes0 entry of selected target.",          NULL, UNIT_FIELD_BYTES_0,                0,                    1 },
        { "bytes1",          'm', NULL,                                   "WARNING! Modifies the bytes1 entry of selected target.",          NULL, UNIT_FIELD_BYTES_1,                0,                    1 },
        { "bytes2",          'm', NULL,                                   "WARNING! Modifies the bytes2 entry of selected target.",          NULL, UNIT_FIELD_BYTES_2,                0,                    1 },
        { NULL,              '0', NULL,                                   "",                                                                NULL, 0,                                 0,                    0 }
    };
    dupe_command_table(modifyCommandTable, _modifyCommandTable);

    static ChatCommand eventCommandTable[] =
    {
        { "list",               'm', &ChatHandler::HandleEventListEvents,           "Shows list of currently active events",                    nullptr, 0, 0, 0 },
        { "start",              'm', &ChatHandler::HandleEventStartEvent,           "Force start an event",                                     nullptr, 0, 0, 0 },
        { "stop",               'm', &ChatHandler::HandleEventStopEvent,            "Force stop an event",                                      nullptr, 0, 0, 0 },
        { "reset",              'm', &ChatHandler::HandleEventResetEvent,           "Resets force flags for an event",                          nullptr, 0, 0, 0 },
        { "reload",             'a', &ChatHandler::HandleEventReloadAllEvents,      "Reloads all events from the database",                     nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr, 0, 0, 0 }
    };
    dupe_command_table(eventCommandTable, _eventCommandTable);

    static ChatCommand transportCommandTable[] =
    {
        { "info",               'm', &ChatHandler::HandleGetTransporterInfo,        "Displays the current transport info",                      nullptr, 0, 0, 0 },
        { "spawn",              'm', &ChatHandler::HandleSpawnInstanceTransport,    "Spawns transport with entry/period in current instance",   nullptr, 0, 0, 0 },
        { "despawn",            'm', &ChatHandler::HandleDespawnInstanceTransport,  "Despawns the transport you are currently on",              nullptr, 0, 0, 0 },
        { "start",              'm', &ChatHandler::HandleStartTransport,            "Force starts the current transport",                       nullptr, 0, 0, 0 },
        { "stop",               'm', &ChatHandler::HandleStopTransport,             "Force stops the current transport",                        nullptr, 0, 0, 0 },
        { "modperiod",          'm', &ChatHandler::HandleModPeriodCommand,          "Changes the period of the current transport",              nullptr, 0, 0, 0 },
        { "getperiod",          'm', &ChatHandler::HandleGetTransporterTime,        "Displays the current transport period in ms",              nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr, 0, 0, 0 }
    };
    dupe_command_table(transportCommandTable, _transportCommandTable);

    static ChatCommand debugCommandTable[] =
    {
        { "dumpmovement", 'd', &ChatHandler::HandleDebugDumpMovementCommand, "!dumpmovement - Dumps the player's movement information to chat", nullptr, 0, 0, 0},
        { "infront",             'd', &ChatHandler::HandleDebugInFrontCommand,     "",                                                                                                                  NULL, 0, 0, 0 },
        { "showreact",           'd', &ChatHandler::HandleShowReactionCommand,     "",                                                                                                                  NULL, 0, 0, 0 },
        { "aimove",              'd', &ChatHandler::HandleAIMoveCommand,           "",                                                                                                                  NULL, 0, 0, 0 },
        { "dist",                'd', &ChatHandler::HandleDistanceCommand,         "",                                                                                                                  NULL, 0, 0, 0 },
        { "face",                'd', &ChatHandler::HandleFaceCommand,             "",                                                                                                                  NULL, 0, 0, 0 },
        { "moveinfo",            'd', &ChatHandler::HandleDebugMoveInfo,         "",                                                                                                                  NULL, 0, 0, 0 },
        { "setbytes",            'd', &ChatHandler::HandleSetBytesCommand,         "",                                                                                                                  NULL, 0, 0, 0 },
        { "getbytes",            'd', &ChatHandler::HandleGetBytesCommand,         "",                                                                                                                  NULL, 0, 0, 0 },
        { "landwalk",            'd', &ChatHandler::HandleDebugLandWalk,           "",                                                                                                                  NULL, 0, 0, 0 },
        { "waterwalk",           'd', &ChatHandler::HandleDebugWaterWalk,          "",                                                                                                                  NULL, 0, 0, 0 },
        { "castspell",           'd', &ChatHandler::HandleCastSpellCommand,        ".castspell <spellid> - Casts spell on target.",                                                                     NULL, 0, 0, 0 },
        { "castself",            'd', &ChatHandler::HandleCastSelfCommand,         ".castself <spellId> - Target casts spell <spellId> on itself.",                                                     NULL, 0, 0, 0 },
        { "castspellne",         'd', &ChatHandler::HandleCastSpellNECommand,      ".castspellne <spellid> - Casts spell on target (only plays animations, doesn't handle effects or range/facing/etc.", NULL, 0, 0, 0 },
        { "aggrorange",          'd', &ChatHandler::HandleAggroRangeCommand,       ".aggrorange - Shows aggro Range of the selected Creature.",                                                         NULL, 0, 0, 0 },
        { "knockback",           'd', &ChatHandler::HandleKnockBackCommand,        ".knockback <value> - Knocks you back.",                                                                             NULL, 0, 0, 0 },
        { "fade",                'd', &ChatHandler::HandleFadeCommand,             ".fade <value> - calls ModThreatModifyer().",                                                                        NULL, 0, 0, 0 },
        { "threatMod",           'd', &ChatHandler::HandleThreatModCommand,        ".threatMod <value> - calls ModGeneratedThreatModifyer().",                                                          NULL, 0, 0, 0 },
        { "calcThreat",          'd', &ChatHandler::HandleCalcThreatCommand,       ".calcThreat <dmg> <spellId> - calculates threat.",                                                                  NULL, 0, 0, 0 },
        { "threatList",          'd', &ChatHandler::HandleThreatListCommand,       ".threatList  - returns all AI_Targets of the selected Creature.",                                                   NULL, 0, 0, 0 },
        { "gettptime",           'd', &ChatHandler::HandleGetTransporterTime,      "grabs transporter travel time",                                                                                     NULL, 0, 0, 0 },
        { "itempushresult",      'd', &ChatHandler::HandleSendItemPushResult,      "sends item push result",                                                                                            NULL, 0, 0, 0 },
        { "setbit",              'd', &ChatHandler::HandleModifyBitCommand,        "",                                                                                                                  NULL, 0, 0, 0 },
        { "setvalue",            'd', &ChatHandler::HandleModifyValueCommand,      "",                                                                                                                  NULL, 0, 0, 0 },
        { "aispelltestbegin",    'd', &ChatHandler::HandleAIAgentDebugBegin,       "",                                                                                                                  NULL, 0, 0, 0 },
        { "aispelltestcontinue", 'd', &ChatHandler::HandleAIAgentDebugContinue,    "",                                                                                                                  NULL, 0, 0, 0 },
        { "aispelltestskip",     'd', &ChatHandler::HandleAIAgentDebugSkip,        "",                                                                                                                  NULL, 0, 0, 0 },
        { "dumpcoords",          'd', &ChatHandler::HandleDebugDumpCoordsCommmand, "",                                                                                                                  NULL, 0, 0, 0 },
        { "sendpacket",          'd', &ChatHandler::HandleSendpacket,              "<opcode ID>, <data>",                                                                                               NULL, 0, 0, 0 },
        { "sqlquery",            'd', &ChatHandler::HandleSQLQueryCommand,         "<sql query>",                                                                                                       NULL, 0, 0, 0 },
        { "rangecheck",          'd', &ChatHandler::HandleRangeCheckCommand,       "Checks the 'yard' range and internal range between the player and the target.",                                     NULL, 0, 0, 0 },
        { "setallratings",       'd', &ChatHandler::HandleRatingsCommand,          "Sets rating values to incremental numbers based on their index.",                                                   NULL, 0, 0, 0 },
        { "testlos",             'd', &ChatHandler::HandleCollisionTestLOS,        "tests los",                                                                                                         NULL, 0, 0, 0 },
        { "testindoor",          'd', &ChatHandler::HandleCollisionTestIndoor,     "tests indoor",                                                                                                      NULL, 0, 0, 0 },
        { "getheight",           'd', &ChatHandler::HandleCollisionGetHeight,      "Gets height",                                                                                                       NULL, 0, 0, 0 },
        { "deathstate",          'd', &ChatHandler::HandleGetDeathState,           "returns current deathstate for target",                                                                             NULL, 0, 0, 0 },
        { "sendfailed",             'd', &ChatHandler::HandleSendFailed,      "",                                                                                                                  NULL, 0, 0, 0 },
        { "playmovie",             'd', &ChatHandler::HandlePlayMovie,               "Triggers a movie for a player",                                    NULL, 0, 0, 0 },
        { "auraupdate",             'd', &ChatHandler::HandleAuraUpdateAdd,               "<SpellID> <Flags> <StackCount> (caster guid = player target)",                                    NULL, 0, 0, 0 },
        { "auraremove",             'd', &ChatHandler::HandleAuraUpdateRemove,               "<VisualSlot>",                                    NULL, 0, 0, 0 },
        { "spawnwar",             'd', &ChatHandler::HandleDebugSpawnWarCommand,       "Spawns desired amount of npcs to fight with eachother",                                                                NULL, 0, 0, 0 },
        { "updateworldstate",    'd', &ChatHandler::HandleUpdateWorldStateCommand, "Sets the specified worldstate field to the specified value",                                                        NULL, 0, 0, 0 },
        { "initworldstates",     'd', &ChatHandler::HandleInitWorldStatesCommand,  "(re)initializes the worldstates.",                                                                                  NULL, 0, 0, 0 },
        { "clearworldstates",    'd', &ChatHandler::HandleClearWorldStatesCommand, "Clears the worldstates",                                                                                            NULL, 0, 0, 0 },
        { "pvpcredit",         'm', &ChatHandler::HandleDebugPVPCreditCommand,      "Sends PVP credit packet, with specified rank and points", NULL, 0, 0, 0 },
        { NULL,                  '0', NULL,                                        "",                                                                                                                  NULL, 0, 0, 0 }
    };
    dupe_command_table(debugCommandTable, _debugCommandTable);

    static ChatCommand waypointCommandTable[] =
    {
        { "add",                'w', &ChatHandler::HandleWayPointAddCommand,            "Add wp for selected creature at current pos.",     nullptr, 0, 0, 0 },
        { "addfly",             'w', &ChatHandler::HandleWayPointAddFlyCommand,         "Adds a flying waypoint for selected creature.",    nullptr, 0, 0, 0 },
        { "change",             'w', &ChatHandler::HandleWayPointChangeNumberCommand,   "Change wp ID for selected wp.",                    nullptr, 0, 0, 0 },
        { "delete",             'w', &ChatHandler::HandleWayPointDeleteCommand,         "Deletes selected wp.",                             nullptr, 0, 0, 0 },
        { "deleteall",          'w', &ChatHandler::HandleWayPointDeleteAllCommand,      "Deletes all waypoints of selected creature.",      nullptr, 0, 0, 0 },
        { "emote",              'w', &ChatHandler::HandleWayPointEmoteCommand,          "Set emote ID for selected wp.",                    nullptr, 0, 0, 0 },
        { "flags",              'w', &ChatHandler::HandleWayPointFlagsCommand,          "Set flags for selected wp.",                       nullptr, 0, 0, 0 },
        { "generate",           'w', &ChatHandler::HandleWayPointGenerateCommand,       "Randomly generate <x> wps for selected creature.", nullptr, 0, 0, 0 },
        { "hide",               'w', &ChatHandler::HandleWayPointHideCommand,           "Hide wp's for selected creature.",                 nullptr, 0, 0, 0 },
        { "info",               'w', &ChatHandler::HandleWayPointInfoCommand,           "Show info for selected wp.",                       nullptr, 0, 0, 0 },
        { "movehere",           'w', &ChatHandler::HandleWayPpointMoveHereCommand,      "Moves the selected wp to your position.",          nullptr, 0, 0, 0 },
        { "movetype",           'w', &ChatHandler::HandleWayPointMoveTypeCommand,       "Change movement type for selected wp.",            nullptr, 0, 0, 0 },
        { "save",               'w', &ChatHandler::HandleWayPointSaveCommand,           "Save all waypoints for selected creature.",        nullptr, 0, 0, 0 },
        { "show",               'w', &ChatHandler::HandleWayPointShowCommand,           "Show wp's for selected creature <bool backwards>", nullptr, 0, 0, 0 },
        { "skin",               'w', &ChatHandler::HandleWayPointSkinCommand,           "Sets Skin ID for selected wp.",                    nullptr, 0, 0, 0 },
        { "waittime",           'w', &ChatHandler::HandleWayPointWaitCommand,           "Sets Wait time in ms for selected wp.",            nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(waypointCommandTable, _waypointCommandTable);

    static ChatCommand GMTicketCommandTable[] =
    {
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        { "get",             'c', &ChatHandler::HandleGMTicketListCommand,                     "Gets GM Ticket list.",                                          NULL, 0, 0, 0 },
        { "getId",           'c', &ChatHandler::HandleGMTicketGetByIdCommand,                  "Gets GM Ticket by player name.",                                NULL, 0, 0, 0 },
        { "delId",           'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,               "Deletes GM Ticket by player name.",                             NULL, 0, 0, 0 },
#else
        { "list",            'c', &ChatHandler::HandleGMTicketListCommand,                     "Lists all active GM Tickets.",                                  NULL, 0, 0, 0 },
        { "get",             'c', &ChatHandler::HandleGMTicketGetByIdCommand,                  "Gets GM Ticket with ID x.",                                     NULL, 0, 0, 0 },
        { "remove",          'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,               "Removes GM Ticket with ID x.",                                  NULL, 0, 0, 0 },
        { "deletepermanent", 'z', &ChatHandler::HandleGMTicketDeletePermanentCommand,          "Deletes GM Ticket with ID x permanently.",                      NULL, 0, 0, 0 },
        { "assign",          'c', &ChatHandler::HandleGMTicketAssignToCommand,                 "Assigns GM Ticket with id x to GM y (if empty to your self).", NULL, 0, 0, 0 },
        { "release",         'c', &ChatHandler::HandleGMTicketReleaseCommand,                  "Releases assigned GM Ticket with ID x.",                        NULL, 0, 0, 0 },
        { "comment",         'c', &ChatHandler::HandleGMTicketCommentCommand,                  "Sets comment x to GM Ticket with ID y.",                        NULL, 0, 0, 0 },
#endif
        { "toggle",          'z', &ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand, "Toggles the ticket system status.",                             NULL, 0, 0, 0 },
        { NULL,              '0', NULL,                                                        "",                                                              NULL, 0, 0, 0 }
    };
    dupe_command_table(GMTicketCommandTable, _GMTicketCommandTable);

    static ChatCommand TicketCommandTable[] =
    {
        { "list",               'c', &ChatHandler::HandleTicketListCommand,             "Shows all active tickets",                         nullptr, 0, 0, 0 },
        { "listall",            'c', &ChatHandler::HandleTicketListAllCommand,          "Shows all tickets in the database",                nullptr, 0, 0, 0 },
        { "get",                'c', &ChatHandler::HandleTicketGetCommand,              "Returns the content of the specified ID",          nullptr, 0, 0, 0 },
        { "close",              'c', &ChatHandler::HandleTicketCloseCommand,            "Close ticket with specified ID",                   nullptr, 0, 0, 0 },
        { "delete",             'a', &ChatHandler::HandleTicketDeleteCommand,           "Delete ticket by specified ID",                    nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(TicketCommandTable, _TicketCommandTable);

    static ChatCommand GuildCommandTable[] =
    {
        { "join",         'm', &ChatHandler::HandleGuildJoinCommand,         "Force joins a guild",                 NULL, 0, 0, 0 },
        { "create",       'm', &ChatHandler::CreateGuildCommand,             "Creates a guild.",                    NULL, 0, 0, 0 },
        { "rename",       'm', &ChatHandler::HandleRenameGuildCommand,       "Renames a guild.",                    NULL, 0, 0, 0 },
        { "members",      'm', &ChatHandler::HandleGuildMembersCommand,      "Lists guildmembers and their ranks.", NULL, 0, 0, 0 },
        { "removeplayer", 'm', &ChatHandler::HandleGuildRemovePlayerCommand, "Removes a player from a guild.",      NULL, 0, 0, 0 },
        { "disband",      'm', &ChatHandler::HandleGuildDisbandCommand,      "Disbands the guild of your target.",  NULL, 0, 0, 0 },
        { NULL,           '0', NULL,                                         "",                                    NULL, 0, 0, 0 }
    };
    dupe_command_table(GuildCommandTable, _GuildCommandTable);

    static ChatCommand GameObjectSetCommandTable[] =
    {
        { "animprogress",       'o', &ChatHandler::HandleGOSetAnimProgressCommand,      "Sets anim progress of selected GO",                nullptr, 0, 0, 0 },
        { "faction",            'o', &ChatHandler::HandleGOSetFactionCommand,           "Sets the faction of the GO",                       nullptr, 0, 0, 0 },
        { "flags",              'o', &ChatHandler::HandleGOSetFlagsCommand,             "Sets the flags of the GO",                         nullptr, 0, 0, 0 },
        { "overrides",          'o', &ChatHandler::HandleGOSetOverridesCommand,         "Sets override of selected GO",                     nullptr, 0, 0, 0 },
        { "phase",              'o', &ChatHandler::HandleGOSetPhaseCommand,             "Sets phase of selected GO",                        nullptr, 0, 0, 0 },
        { "scale",              'o', &ChatHandler::HandleGOSetScaleCommand,             "Sets scale of selected GO",                        nullptr, 0, 0, 0 },
        { "state",              'o', &ChatHandler::HandleGOSetStateCommand,             "Sets the state byte of the GO",                    nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(GameObjectSetCommandTable, _GameObjectSetCommandTable);

    static ChatCommand GameObjectCommandTable[] =
    {
        { "damage",             'o', &ChatHandler::HandleGODamageCommand,               "Damages the GO for the specified hitpoints",       nullptr, 0, 0, 0 },
        { "delete",             'o', &ChatHandler::HandleGODelete,                      "Deletes selected GameObject",                      nullptr, 0, 0, 0 },
        { "enable",             'o', &ChatHandler::HandleGOEnable,                      "Enables the selected GO for use.",                 nullptr, 0, 0, 0 },
        { "export",             'o', &ChatHandler::HandleGOExport,                      "Exports the current GO selected",                  nullptr, 0, 0, 0 },
        { "info",               'o', &ChatHandler::HandleGOInfo,                        "Gives you information about selected GO",          nullptr, 0, 0, 0 },
        { "movehere",           'g', &ChatHandler::HandleGOMoveHereCommand,             "Moves gameobject to your position",                nullptr, 0, 0, 0 },
        { "open",               'o', &ChatHandler::HandleGOOpenCommand,                 "Toggles open/close (state) of selected GO.",       nullptr, 0, 0, 0 },
        { "rebuild",            'o', &ChatHandler::HandleGORebuildCommand,              "Rebuilds the GO.",                                 nullptr, 0, 0, 0 },
        { "rotate",             'g', &ChatHandler::HandleGORotate,                      "Rotates the object. <Axis> x,y, Default o.",       nullptr, 0, 0, 0 },
        { "select",             'o', &ChatHandler::HandleGOSelect,                      "Selects the nearest GameObject to you",            nullptr, 0, 0, 0 },
        { "selectguid",         'o', &ChatHandler::HandleGOSelectGuidCommand,           "Selects GO with <guid>",                           nullptr, 0, 0, 0 },
        { "set",                'o', nullptr,                                           "",                               GameObjectSetCommandTable, 0, 0, 0 },
        { "spawn",              'o', &ChatHandler::HandleGOSpawn,                       "Spawns a GameObject by ID",                        nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

    static ChatCommand BattlegroundCommandTable[] =
    {
        { "setbgscore",    'e', &ChatHandler::HandleSetBGScoreCommand,                       "<Teamid> <Score> - Sets battleground score. 2 Arguments.",      NULL, 0, 0, 0 },
        { "startbg",       'e', &ChatHandler::HandleStartBGCommand,                          "Starts current battleground match.",                            NULL, 0, 0, 0 },
        { "pausebg",       'e', &ChatHandler::HandlePauseBGCommand,                          "Pauses current battleground match.",                            NULL, 0, 0, 0 },
        { "bginfo",        'e', &ChatHandler::HandleBGInfoCommnad,                           "Displays information about current battleground.",              NULL, 0, 0, 0 },
        { "battleground",  'e', &ChatHandler::HandleBattlegroundCommand,                     "Shows BG Menu",                                                 NULL, 0, 0, 0 },
        { "setworldstate", 'e', &ChatHandler::HandleSetWorldStateCommand,                    "<var> <val> - Var can be in hex. WS Value.",                    NULL, 0, 0, 0 },
        { "setworldstates", 'e', &ChatHandler::HandleSetWorldStatesCommand,                    "<var> <val> - Var can be in hex. WS Value.",                   NULL, 0, 0, 0 },
        { "playsound",     'e', &ChatHandler::HandlePlaySoundCommand,                        "<val>. Val can be in hex.",                                     NULL, 0, 0, 0 },
        { "setbfstatus",   'e', &ChatHandler::HandleSetBattlefieldStatusCommand,             ".setbfstatus - NYI.",                                           NULL, 0, 0, 0 },
        { "leave",         'e', &ChatHandler::HandleBattlegroundExitCommand,                 "Leaves the current battleground.",                              NULL, 0, 0, 0 },
        { "getqueue",      'z', &ChatHandler::HandleGetBattlegroundQueueCommand,             "Gets common battleground queue information.",                   NULL, 0, 0, 0 },
        { "forcestart",    'z', &ChatHandler::HandleInitializeAllQueuedBattlegroundsCommand, "Forces initialization of all battlegrounds with active queue.", NULL, 0, 0, 0 },
        { NULL,            '0', NULL,                                                        "",                                                              NULL, 0, 0, 0 }
    };
    dupe_command_table(BattlegroundCommandTable, _BattlegroundCommandTable);

    static ChatCommand NPCSetCommandTable[] =
    {
        { "canfly",             'n', &ChatHandler::HandleNpcSetCanFlyCommand,           "Toggles CanFly state",                             nullptr, 0, 0, 0 },
        { "emote",              'n', &ChatHandler::HandleNpcSetEmoteCommand,            "Sets emote state",                                 nullptr, 0, 0, 0 },
        { "equip",              'm', &ChatHandler::HandleNpcSetEquipCommand,            "Sets equipment itemt",                             nullptr, 0, 0, 0 },
        { "flags",              'n', &ChatHandler::HandleNpcSetFlagsCommand,            "Sets NPC flags",                                   nullptr, 0, 0, 0 },
        { "formationmaster",    'm', &ChatHandler::HandleNpcSetFormationMasterCommand,  "Sets formation master.",                           nullptr, 0, 0, 0 },
        { "formationslave",     'm', &ChatHandler::HandleNpcSetFormationSlaveCommand,   "Sets formation slave with distance and angle",     nullptr, 0, 0, 0 },
        { "formationclear",     'm', &ChatHandler::HandleNpcSetFormationClearCommand,   "Removes formation from creature",                  nullptr, 0, 0, 0 },
        { "ongameobject",       'n', &ChatHandler::HandleNpcSetOnGOCommand,             "Toggles onGameobject state.",                      nullptr, 0, 0, 0 },
        { "phase",              'n', &ChatHandler::HandleNpcSetPhaseCommand,            "Sets phase for selected creature",                 nullptr, 0, 0, 0 },
        { "standstate",         'm', &ChatHandler::HandleNpcSetStandstateCommand,       "Sets standstate for selected creature",            nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(NPCSetCommandTable, _NPCSetCommandTable);

    static ChatCommand NPCCommandTable[] =
    {
        { "addagent",           'n', &ChatHandler::HandleNpcAddAgentCommand,            "Add ai agents to npc.",                            nullptr, 0, 0, 0 },
        { "addtrainerspell",    'm', &ChatHandler::HandleNpcAddTrainerSpellCommand,     "Add spells to trainer learn list.",                nullptr, 0, 0, 0 },
        { "cast",               'n', &ChatHandler::HandleNpcCastCommand,                "Makes Npc cast <spellid>.",                        nullptr, 0, 0, 0 },
        { "come",               'n', &ChatHandler::HandleNpcComeCommand,                "Makes npc move to your position",                  nullptr, 0, 0, 0 },
        { "delete",             'n', &ChatHandler::HandleNpcDeleteCommand,              "Deletes mob from world optional from DB",          nullptr, 0, 0, 0 },
        { "info",               'n', &ChatHandler::HandleNpcInfoCommand,                "Displays NPC information",                         nullptr, 0, 0, 0 },
        { "listAgent",          'n', &ChatHandler::HandleNpcListAIAgentCommand,         "List AIAgents of selected target.",                nullptr, 0, 0, 0 },
        { "listloot",           'm', &ChatHandler::HandleNpcListLootCommand,            "Displays possible loot for the selected NPC.",     nullptr, 0, 0, 0 },
        { "follow",             'm', &ChatHandler::HandleNpcFollowCommand,              "Sets npc to follow you",                           nullptr, 0, 0, 0 },
        { "stopfollow",         'm', &ChatHandler::HandleNpcStopFollowCommand,          "Sets npc to not follow anything",                  nullptr, 0, 0, 0 },
        { "possess",            'n', &ChatHandler::HandlePossessCommand,                "Possess targeted npc (mind control)",              nullptr, 0, 0, 0 },
        { "unpossess",          'n', &ChatHandler::HandleUnPossessCommand,              "Unpossess any currently possessed npc.",           nullptr, 0, 0, 0 },
        { "return",             'n', &ChatHandler::HandleNpcReturnCommand,              "Returns ncp to spawnpoint.",                       nullptr, 0, 0, 0 },
        { "respawn",            'n', &ChatHandler::HandleNpcRespawnCommand,             "Respawns a dead npc from its corpse.",             nullptr, 0, 0, 0 },
        { "say",                'n', &ChatHandler::HandleNpcSayCommand,                 "Makes selected npc say <text>.",                   nullptr, 0, 0, 0 },
        { "select",             'n', &ChatHandler::HandleNpcSelectCommand,              "Slects npc closest",                               nullptr, 0, 0, 0 },
        { "set",                '0', nullptr,                                           "",                                      NPCSetCommandTable, 0, 0, 0 },
        { "spawn",              'n', &ChatHandler::HandleNpcSpawnCommand,               "Spawns npc of entry <id>",                         nullptr, 0, 0, 0 },
        { "vendoradditem",      'n', &ChatHandler::HandleItemCommand,                   "Adds item to vendor",                              nullptr, 0, 0, 0 },
        { "vendorremoveitem",   'n', &ChatHandler::HandleItemRemoveCommand,             "Removes item from vendor.",                        nullptr, 0, 0, 0 },
        { "yell",               'n', &ChatHandler::HandleNpcYellCommand,                "Makes selected npc yell <text>.",                  nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(NPCCommandTable, _NPCCommandTable);

    static ChatCommand CheatCommandTable[] =
    {
        { "list",               'm', &ChatHandler::HandleCheatListCommand,              "Shows active cheats.",                             nullptr, 0, 0, 0 },
        { "taxi",               'm', &ChatHandler::HandleCheatTaxiCommand,              "Toggles TaxiCheat.",                               nullptr, 0, 0, 0 },
        { "cooldown",           'm', &ChatHandler::HandleCheatCooldownCommand,          "Toggles CooldownCheat.",                           nullptr, 0, 0, 0 },
        { "casttime",           'm', &ChatHandler::HandleCheatCastTimeCommand,          "Toggles CastTimeCheat.",                           nullptr, 0, 0, 0 },
        { "power",              'm', &ChatHandler::HandleCheatPowerCommand,             "Toggles PowerCheat. Disables mana consumption.",   nullptr, 0, 0, 0 },
        { "god",                'm', &ChatHandler::HandleCheatGodCommand,               "Toggles GodCheat.",                                nullptr, 0, 0, 0 },
        { "fly",                'm', &ChatHandler::HandleCheatFlyCommand,               "Toggles FlyCheat.",                                nullptr, 0, 0, 0 },
        { "aurastack",          'm', &ChatHandler::HandleCheatAuraStackCommand,         "Toggles AuraStackCheat.",                          nullptr, 0, 0, 0 },
        { "itemstack",          'm', &ChatHandler::HandleCheatItemStackCommand,         "Toggles ItemStackCheat.",                          nullptr, 0, 0, 0 },
        { "triggerpass",        'm', &ChatHandler::HandleCheatTriggerpassCommand,       "Ignores area trigger prerequisites.",              nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(CheatCommandTable, _CheatCommandTable);

    static ChatCommand accountCommandTable[] =
    {
        { "create",     'a', &ChatHandler::HandleAccountCreate,         "Creates an account with name and password",                        nullptr, 0, 0, 0 },
        { "setgm",      'z', &ChatHandler::HandleAccountSetGMCommand,   "Sets gm level on account. Pass it username and 0,1,2,3,az, etc.",  nullptr, 0, 0, 0 },
        { "mute",       'a', &ChatHandler::HandleAccountMuteCommand,    "Mutes account for <timeperiod>.",                                  nullptr, 0, 0, 0 },
        { "unmute",     'a', &ChatHandler::HandleAccountUnmuteCommand,  "Unmutes account <x>",                                              nullptr, 0, 0, 0 },
        { "ban",        'a', &ChatHandler::HandleAccountBannedCommand,  "Bans account: .ban account <name> [duration] [reason]",            nullptr, 0, 0, 0 },
        { "unban",      'z', &ChatHandler::HandleAccountUnbanCommand,   "Unbans account x.",                                                nullptr, 0, 0, 0 },
        { "changepw",   '0', &ChatHandler::HandleAccountChangePassword, "Change the password of your account.",                             nullptr, 0, 0, 0 },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(accountCommandTable, _accountCommandTable);

    static ChatCommand petCommandTable[] =
    {
        { "createpet",   'm', &ChatHandler::HandleCreatePetCommand,      "Creates a pet with <entry>.",                            NULL, 0, 0, 0 },
        { "dismiss",     'm', &ChatHandler::HandleDismissPetCommand,     "Dismisses selected pet.",                                NULL, 0, 0, 0 },
        { "renamepet",   'm', &ChatHandler::HandleRenamePetCommand,      "Renames a pet to <name>.",                               NULL, 0, 0, 0 },
        { "addspell",    'm', &ChatHandler::HandleAddPetSpellCommand,    "Teaches pet <spell>.",                                   NULL, 0, 0, 0 },
        { "removespell", 'm', &ChatHandler::HandleRemovePetSpellCommand, "Removes pet spell <spell>.",                             NULL, 0, 0, 0 },
        { "setlevel",    'm', &ChatHandler::HandlePetLevelCommand,       "Sets pet level to <level>.",                             NULL, 0, 0, 0 },
#ifdef USE_SPECIFIC_AIAGENTS
        { "spawnbot",    'a', &ChatHandler::HandlePetSpawnAIBot,         ".pet spawnbot <type> - spawn a helper bot for your aid", NULL, 0, 0, 0 },
#endif
        { NULL,          '0', NULL,                                      "",                                                       NULL, 0, 0, 0 }
    };
    dupe_command_table(petCommandTable, _petCommandTable);

    //teleport
    static ChatCommand recallCommandTable[] =
    {
        { "list",       'q', &ChatHandler::HandleRecallListCommand,       "List recall locations",     NULL, 0, 0, 0 },
        { "add",        'q', &ChatHandler::HandleRecallAddCommand,        "Add a recall location",       NULL, 0, 0, 0 },
        { "del",        'q', &ChatHandler::HandleRecallDelCommand,        "Remove a recall location",  NULL, 0, 0, 0 },
        { "port",       'q', &ChatHandler::HandleRecallGoCommand,         "Ports you to recalled location", NULL, 0, 0, 0 },
        { "portplayer", 'm', &ChatHandler::HandleRecallPortPlayerCommand, "Ports specified player to a recalled location", NULL, 0, 0, 0 },
        { "portus",        'm', &ChatHandler::HandleRecallPortUsCommand,      "Ports you and the selected player to recalled location",       NULL, 0, 0, 0 },
        { NULL,         '0', NULL,                                        "",                          NULL, 0, 0, 0 }
    };
    dupe_command_table(recallCommandTable, _recallCommandTable);

    static ChatCommand questCommandTable[] =
    {
        //npc
        { "addboth",   '2', &ChatHandler::HandleQuestAddBothCommand,   "Add quest <id> to the targeted NPC as start & finish",      NULL, 0, 0, 0 },
        //npc
        { "addfinish", '2', &ChatHandler::HandleQuestAddFinishCommand, "Add quest <id> to the targeted NPC as finisher",            NULL, 0, 0, 0 },
        //npc
        { "addstart",  '2', &ChatHandler::HandleQuestAddStartCommand,  "Add quest <id> to the targeted NPC as starter",             NULL, 0, 0, 0 },
        //npc
        { "delboth",   '2', &ChatHandler::HandleQuestDelBothCommand,   "Delete quest <id> from the targeted NPC as start & finish", NULL, 0, 0, 0 },
        //npc
        { "delfinish", '2', &ChatHandler::HandleQuestDelFinishCommand, "Delete quest <id> from the targeted NPC as finisher",       NULL, 0, 0, 0 },
        //npc
        { "delstart",  '2', &ChatHandler::HandleQuestDelStartCommand,  "Delete quest <id> from the targeted NPC as starter",        NULL, 0, 0, 0 },
        //char
        { "complete",  '2', &ChatHandler::HandleQuestFinishCommand,    "Complete/Finish quest <id>",                                NULL, 0, 0, 0 },
        //char
        { "fail",      '2', &ChatHandler::HandleQuestFailCommand,      "Fail quest <id>",                                           NULL, 0, 0, 0 },
        { "finisher",  '2', &ChatHandler::HandleQuestFinisherCommand,  "Lookup quest finisher for quest <id>",                      NULL, 0, 0, 0 },
        { "item",      '2', &ChatHandler::HandleQuestItemCommand,      "Lookup itemid necessary for quest <id>",                    NULL, 0, 0, 0 },
        { "list",      '2', &ChatHandler::HandleQuestListCommand,      "Lists the quests for the npc <id>",                         NULL, 0, 0, 0 },
        // npc
        { "load",      '2', &ChatHandler::HandleQuestLoadCommand,      "Loads quests from database",                                NULL, 0, 0, 0 },
        { "giver",     '2', &ChatHandler::HandleQuestGiverCommand,     "Lookup quest giver for quest <id>",                         NULL, 0, 0, 0 },
        //char
        { "remove",    '2', &ChatHandler::HandleQuestRemoveCommand,    "Removes the quest <id> from the targeted player",           NULL, 0, 0, 0 },
        { "reward",    '2', &ChatHandler::HandleQuestRewardCommand,    "Shows reward for quest <id>",                               NULL, 0, 0, 0 },
        //char
        { "status",    '2', &ChatHandler::HandleQuestStatusCommand,    "Lists the status of quest <id>",                            NULL, 0, 0, 0 },
        //char
        { "start",     '2', &ChatHandler::HandleQuestStartCommand,     "Starts quest <id>",                                         NULL, 0, 0, 0 },
        //teleport
        { "startspawn", '2', &ChatHandler::HandleQuestStarterSpawnCommand, "Port to spawn location for quest <id> (starter)",        NULL, 0, 0, 0 },
        //teleport
        { "finishspawn", '2', &ChatHandler::HandleQuestFinisherSpawnCommand, "Port to spawn location for quest <id> (finisher)",       NULL, 0, 0, 0 },
        { NULL,        '0', NULL,                                      "",                                                          NULL, 0, 0, 0 }
    };
    dupe_command_table(questCommandTable, _questCommandTable);

    static ChatCommand reloadTableCommandTable[] =
    {
        { "gameobjects",        'z', &ChatHandler::HandleReloadGameobjectsCommand,          "Reload gameobjets",                            nullptr, 0, 0, 0 },
        { "creatures",          'z', &ChatHandler::HandleReloadCreaturesCommand,            "Reload creatures",                             nullptr, 0, 0, 0 },
        { "areatriggers",       'z', &ChatHandler::HandleReloadAreaTriggersCommand,         "Reload areatriggers table",                    nullptr, 0, 0, 0 },
        { "command_overrides",  'z', &ChatHandler::HandleReloadCommandOverridesCommand,     "Reload command_overrides table",               nullptr, 0, 0, 0 },
        { "fishing",            'z', &ChatHandler::HandleReloadFishingCommand,              "Reload fishing table",                         nullptr, 0, 0, 0 },
        { "gossip_menu_option", 'z', &ChatHandler::HandleReloadGossipMenuOptionCommand,     "Reload gossip_menu_option table",              nullptr, 0, 0, 0 },
        { "graveyards",         'z', &ChatHandler::HandleReloadGraveyardsCommand,           "Reload graveyards table",                      nullptr, 0, 0, 0 },
        { "items",              'z', &ChatHandler::HandleReloadItemsCommand,                "Reload items table",                           nullptr, 0, 0, 0 },
        { "itempages",          'z', &ChatHandler::HandleReloadItempagesCommand,            "Reload itempages table",                       nullptr, 0, 0, 0 },
        { "npc_script_text",    'z', &ChatHandler::HandleReloadNpcScriptTextCommand,        "Reload npc_script_text table",                 nullptr, 0, 0, 0 },
        { "npc_text",           'z', &ChatHandler::HandleReloadNpcTextCommand,              "Reload npc_text table",                        nullptr, 0, 0, 0 },
        { "player_xp_for_level",'z', &ChatHandler::HandleReloadPlayerXpForLevelCommand,     "Reload player_xp_for_level table",             nullptr, 0, 0, 0 },
        { "points_of_interest", 'z', &ChatHandler::HandleReloadPointsOfInterestCommand,     "Reload points_of_interest table",              nullptr, 0, 0, 0 },
        { "quests",             'z', &ChatHandler::HandleReloadQuestsCommand,               "Reload quests table",                          nullptr, 0, 0, 0 },
        { "spell_teleport_coords", 'z', &ChatHandler::HandleReloadTeleportCoordsCommand,       "Reload teleport_coords table",                 nullptr, 0, 0, 0 },
        { "worldbroadcast",     'z', &ChatHandler::HandleReloadWorldbroadcastCommand,       "Reload worldbroadcast table",                  nullptr, 0, 0, 0 },
        { "worldmap_info",      'z', &ChatHandler::HandleReloadWorldmapInfoCommand,         "Reload worldmap_info table",                   nullptr, 0, 0, 0 },
        { "worldstring_tables", 'z', &ChatHandler::HandleReloadWorldstringTablesCommand,    "Reload worldstring_tables table",              nullptr, 0, 0, 0 },
        { "zoneguards",         'z', &ChatHandler::HandleReloadZoneguardsCommand,           "Reload zoneguards table",                      nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                                  "",                                          nullptr, 0, 0, 0 }
    };
    dupe_command_table(reloadTableCommandTable, _reloadTableCommandTable);

    static ChatCommand serverCommandTable[] =
    {
        { "info",               '0', &ChatHandler::HandleServerInfoCommand,             "Shows detailed Server info.",                      nullptr, 0, 0, 0 },
        { "rehash",             'z', &ChatHandler::HandleServerRehashCommand,           "Reloads config file.",                             nullptr, 0, 0, 0 },
        { "save",               's', &ChatHandler::HandleServerSaveCommand,             "Save targeted or named player.",                   nullptr, 0, 0, 0 },
        { "saveall",            's', &ChatHandler::HandleServerSaveAllCommand,          "Save all online player.",                          nullptr, 0, 0, 0 },
        { "setmotd",            'm', &ChatHandler::HandleServerSetMotdCommand,          "Sets server MessageOfTheDay.",                     nullptr, 0, 0, 0 },
        { "shutdown",           'z', &ChatHandler::HandleServerShutdownCommand,         "Initiates server shutdown in <x> seconds.",        nullptr, 0, 0, 0 },
        { "cancelshutdown",     'z', &ChatHandler::HandleServerCancelShutdownCommand,   "Cancels a Server Restart/Shutdown.",               nullptr, 0, 0, 0 },
        { "restart",            'z', &ChatHandler::HandleServerRestartCommand,          "Initiates server restart in <x> seconds.",         nullptr, 0, 0, 0 },
        { "reloadtable",        'm', nullptr,                                           "",                                 reloadTableCommandTable, 0, 0, 0 },
        { "reloadscript",       'm', &ChatHandler::HandleServerReloadScriptsCommand,    "",                                                 nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(serverCommandTable, _serverCommandTable);

    static ChatCommand gmCommandTable[] =
    {
        { "active",             't', &ChatHandler::HandleGMActiveCommand,               "Avtivate/Deactivate <GM> tag",                     nullptr, 0, 0, 0 },
        { "allowwhispers",      'c', &ChatHandler::HandleGMAllowWhispersCommand,        "Allows whispers from player <s>.",                 nullptr, 0, 0, 0 },
        { "announce",           'u', &ChatHandler::HandleGMAnnounceCommand,             "Sends announce to all online GMs",                 nullptr, 0, 0, 0 },
        { "blockwhispers",      'c', &ChatHandler::HandleGMBlockWhispersCommand,        "Blocks whispers from player <s>.",                 nullptr, 0, 0, 0 },
        { "devtag",             '1', &ChatHandler::HandleGMDevTagCommand,               "Avtivate/Deactivate <DEV> tag",                    nullptr, 0, 0, 0 },
        { "list",               '0', &ChatHandler::HandleGMListCommand,                 "Shows active GM's",                                nullptr, 0, 0, 0 },
        { "logcomment",         '1', &ChatHandler::HandleGMLogCommentCommand,           "Adds a comment to the GM log.",                    nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(gmCommandTable, _gmCommandTable);

    static ChatCommand characterAddCommandTable[] =
    {
        { "copper",             'm', &ChatHandler::HandleCharAddCopperCommand,          "Adds x Copper to character.",                      nullptr, 0, 0, 0 },
        { "silver",             'm', &ChatHandler::HandleCharAddSilverCommand,          "Adds x silver to character.",                      nullptr, 0, 0, 0 },
        { "gold",               'm', &ChatHandler::HandleCharAddGoldCommand,            "Adds x gold to character.",                        nullptr, 0, 0, 0 },
        { "honorpoints",        'm', &ChatHandler::HandleCharAddHonorPointsCommand,     "Adds x amount of honor points/currency",           nullptr, 0, 0, 0 },
        { "honorkills",         'm', &ChatHandler::HandleCharAddHonorKillCommand,       "Adds x amount of honor kills",                     nullptr, 0, 0, 0 },
        { "item",               'm', &ChatHandler::HandleCharAddItemCommand,            "Adds item x count y",                              nullptr, 0, 0, 0 },
        { "itemset",            'm', &ChatHandler::HandleCharAddItemSetCommand,         "Adds item set to inv.",                            nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(characterAddCommandTable, _characterAddCommandTable);

    static ChatCommand characterSetCommandTable[] =
    {
        { "allexplored",    'm', &ChatHandler::HandleCharSetAllExploredCommand,     "Reveals the unexplored parts of the map.",             nullptr, 0, 0, 0 },
        { "gender",         'm', &ChatHandler::HandleCharSetGenderCommand,          "Changes gender of target. 0=male, 1=female.",          nullptr, 0, 0, 0 },
        { "itemsrepaired",  'n', &ChatHandler::HandleCharSetItemsRepairedCommand,   "Sets all items repaired for selected player",          nullptr, 0, 0, 0 },
        { "level",          'm', &ChatHandler::HandleCharSetLevelCommand,           "Sets level of selected target to <x>.",                nullptr, 0, 0, 0 },
        { "name",           'm', &ChatHandler::HandleCharSetNameCommand,            "Renames character x to y.",                            nullptr, 0, 0, 0 },
        { "phase",          'm', &ChatHandler::HandleCharSetPhaseCommand,           "Sets phase of selected player",                        nullptr, 0, 0, 0 },
        { "speed",          'm', &ChatHandler::HandleCharSetSpeedCommand,           "Sets speed of the selected target to <x>.",            nullptr, 0, 0, 0 },
        { "standing",       'm', &ChatHandler::HandleCharSetStandingCommand,        "Sets standing of faction x to y.",                     nullptr, 0, 0, 0 },
        { "talentpoints",   'm', &ChatHandler::HandleCharSetTalentpointsCommand,    "Sets available talent points of the target.",          nullptr, 0, 0, 0 },
        { "title",          'm', &ChatHandler::HandleCharSetTitleCommand,           "Sets pvp title for target",                            nullptr, 0, 0, 0 },
        { "forcerename",    'm', &ChatHandler::HandleCharSetForceRenameCommand,     "Forces char x to rename on next login",                nullptr, 0, 0, 0 },
        { "customize",      'm', &ChatHandler::HandleCharSetCustomizeCommand,       "Allows char x to customize on next login",             nullptr, 0, 0, 0 },
        { "factionchange",  'm', &ChatHandler::HandleCharSetFactionChangeCommand,   "Allows char x to change the faction on next login",    nullptr, 0, 0, 0 },
        { "racechange",     'm', &ChatHandler::HandleCharSetCustomizeCommand,       "Allows char x to change the race on next login",       nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                           "",                                                     nullptr, 0, 0, 0 }
    };
    dupe_command_table(characterSetCommandTable, _characterSetCommandTable);

    static ChatCommand characterListCommandTable[] =
    {
        { "skills",         'm', &ChatHandler::HandleCharListSkillsCommand,         "Lists all the skills from a player",                   nullptr, 0, 0, 0 },
        { "standing",       'm', &ChatHandler::HandleCharListStandingCommand,       "Lists standing of faction x.",                         nullptr, 0, 0, 0 },
        { "items",          'm', &ChatHandler::HandleCharListItemsCommand,          "Lists items of selected Player",                       nullptr, 0, 0, 0 },
        { "kills",          'm', &ChatHandler::HandleCharListKillsCommand,          "Lists all kills of selected Player",                   nullptr, 0, 0, 0 },
        { "instances",      'z', &ChatHandler::HandleCharListInstanceCommand,       "Lists persistent instances of selected Player",        nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                           "",                                                     nullptr, 0, 0, 0 }
    };
    dupe_command_table(characterListCommandTable, _characterListCommandTable);

    static ChatCommand characterCommandTable[] =
    {
        { "add",                'm', nullptr,    "",                                                                       characterAddCommandTable, 0, 0, 0 },
        { "set",                'm', nullptr,    "",                                                                       characterSetCommandTable, 0, 0, 0 },
        { "list",               'm', nullptr,    "",                                                                      characterListCommandTable, 0, 0, 0 },
        { "clearcooldowns",     'm', &ChatHandler::HandleCharClearCooldownsCommand,     "Clears all cooldowns for your class.",             nullptr, 0, 0, 0 },
        { "demorph",            'm', &ChatHandler::HandleCharDeMorphCommand,            "Demorphs from morphed model.",                     nullptr, 0, 0, 0 },
        { "levelup",            'm', &ChatHandler::HandleCharLevelUpCommand,            "Player target will be levelup x levels",           nullptr, 0, 0, 0 },
        { "removeauras",        'm', &ChatHandler::HandleCharRemoveAurasCommand,        "Removes all auras from target",                    nullptr, 0, 0, 0 },
        { "removesickness",     'm', &ChatHandler::HandleCharRemoveSickessCommand,      "Removes ressurrection sickness from target",       nullptr, 0, 0, 0 },
        { "learn",               'm', &ChatHandler::HandleLearnCommand,            "Learns spell",                                                                                                      NULL, 0, 0, 0 },
        { "unlearn",            'm', &ChatHandler::HandleCharUnlearnCommand,            "Unlearns spell",                                   nullptr, 0, 0, 0 },
        { "learnskill",         'm', &ChatHandler::HandleCharLearnSkillCommand,         "Learns skill id skillid opt: min max.",            nullptr, 0, 0, 0 },
        { "advanceskill",       'm', &ChatHandler::HandleCharAdvanceSkillCommand,       "Advances skill line x y times.",                   nullptr, 0, 0, 0 },
        { "removeskill",        'm', &ChatHandler::HandleCharRemoveSkillCommand,        "Removes skill.",                                   nullptr, 0, 0, 0 },
        { "increaseweaponskill", 'm', &ChatHandler::HandleIncreaseWeaponSkill,     ".increaseweaponskill <count> - Increase equipped weapon skill x times (defaults to 1).",                             NULL, 0, 0, 0 },
        { "resetreputation",     'n', &ChatHandler::HandleResetReputationCommand,  ".resetreputation - Resets reputation to start levels. (use on characters that were made before reputation fixes.)", NULL, 0, 0, 0 },
        { "resetspells",         'n', &ChatHandler::HandleResetSpellsCommand,      ".resetspells - Resets all spells to starting spells of targeted player. DANGEROUS.",                                NULL, 0, 0, 0 },
        { "resettalents",        'n', &ChatHandler::HandleResetTalentsCommand,     ".resettalents - Resets all talents of targeted player to that of their current level. DANGEROUS.",                  NULL, 0, 0, 0 },
        { "resetskills",         'n', &ChatHandler::HandleResetSkillsCommand,      ".resetskills - Resets all skills.",                                                                                 NULL, 0, 0, 0 },
        { "removeitem",          'm', &ChatHandler::HandleRemoveItemCommand,       "Removes item x count y.",                                                                                         NULL, 0, 0, 0 },
        { "advanceallskills",    'm', &ChatHandler::HandleAdvanceAllSkillsCommand, "Advances all skills <x> points.",                                                                                   NULL, 0, 0, 0 },
        { NULL,                  '0', NULL,                                        "",                                                                                                                  NULL, 0, 0, 0 }
    };
    dupe_command_table(characterCommandTable, _characterCommandTable);

    static ChatCommand lookupCommandTable[] =
    {
#ifdef ENABLE_ACHIEVEMENTS
        { "achievement", 'l', &ChatHandler::HandleLookupAchievementCommand,  "Looks up achievement string x.",                              nullptr, 0, 0, 0 },
#endif
        { "creature",       'l', &ChatHandler::HandleLookupCreatureCommand,         "Looks up item string x.",                              nullptr, 0, 0, 0 },
        { "faction",        'l', &ChatHandler::HandleLookupFactionCommand,          "Looks up faction string x.",                           nullptr, 0, 0, 0 },
        { "item",           'l', &ChatHandler::HandleLookupItemCommand,             "Looks up item string x.",                              nullptr, 0, 0, 0 },
        { "object",         'l', &ChatHandler::HandleLookupObjectCommand,           "Looks up gameobject string x.",                        nullptr, 0, 0 , 0},
        { "quest",          'l', &ChatHandler::HandleLookupQuestCommand,            "Looks up quest string x.",                             nullptr, 0, 0, 0 },
        { "spell",          'l', &ChatHandler::HandleLookupSpellCommand,            "Looks up spell string x.",                             nullptr, 0, 0, 0 },
        { "skill",          'l', &ChatHandler::HandleLookupSkillCommand,            "Looks up skill string x.",                             nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                           "",                                                     nullptr, 0, 0, 0 }
    };
    dupe_command_table(lookupCommandTable, _lookupCommandTable);

    static ChatCommand adminCommandTable[] =
    {
        { "castall",               'z', &ChatHandler::HandleCastAllCommand,         "Makes all players online cast spell <x>.",                      NULL, 0, 0, 0 },
        { "dispelall",             'z', &ChatHandler::HandleDispelAllCommand,       "Dispels all negative (or positive w/ 1) auras on all players.", NULL, 0, 0, 0 },
        { "renameallinvalidchars", 'z', &ChatHandler::HandleRenameAllCharacter,     "Renames all invalid character names",                           NULL, 0, 0, 0 },
        { "masssummon",            'z', &ChatHandler::HandleMassSummonCommand,      "Summons all online players to your location,add the a/A parameter for alliance or h/H for horde.",                            NULL, 0, 0, 0 },
        { "playall",               'z', &ChatHandler::HandleGlobalPlaySoundCommand, "Plays a sound to everyone on the realm.",                       NULL, 0, 0, 0 },
        { NULL,                    '0', NULL,                                       "",                                                              NULL, 0, 0, 0 }
    };
    dupe_command_table(adminCommandTable, _adminCommandTable);

    static ChatCommand kickCommandTable[] =
    {
        { "player",                 'f', &ChatHandler::HandleKickByNameCommand,     "Disconnects the player with name <s>.",                        nullptr, 0, 0, 0 },
        { "account",                'f', &ChatHandler::HandleKKickBySessionCommand, "Disconnects the session with account name <s>.",               nullptr, 0, 0, 0 },
        { "ip",                     'f', &ChatHandler::HandleKickByIPCommand,       "Disconnects the session with the ip <s>.",                     nullptr, 0, 0, 0 },
        { nullptr,                  '0', nullptr,                                   "",                                                             nullptr, 0, 0, 0 }
    };
    dupe_command_table(kickCommandTable, _kickCommandTable);

    static ChatCommand banCommandTable[] =
    {
        { "ip",        'm', &ChatHandler::HandleIPBanCommand,         "Adds an address to the IP ban table: .ban ip <address> [duration] [reason]\nDuration must be a number optionally followed by a character representing the calendar subdivision to use (h>hours, d>days, w>weeks, m>months, y>years, default minutes)\nLack of duration results in a permanent ban.", NULL, 0, 0, 0 },
        { "character", 'b', &ChatHandler::HandleBanCharacterCommand,  "Bans character: .ban character <char> [duration] [reason]",                                                                                                                                                                                                                                          NULL, 0, 0, 0 },
        { "all",       'a', &ChatHandler::HandleBanAllCommand,        "Bans account, ip, and character: .ban all <char> [duration] [reason]",                                                                                                                                                                                                                               NULL, 0, 0, 0 },
        { NULL,        '0', NULL,                                     "",                                                                                                                                                                                                                                                                                                   NULL, 0, 0, 0 }
    };
    dupe_command_table(banCommandTable, _banCommandTable);

    static ChatCommand unbanCommandTable[] =
    {
        { "ip",        'm', &ChatHandler::HandleIPUnBanCommand,        "Deletes an address from the IP ban table: <address>", NULL, 0, 0, 0 },
        { "character", 'b', &ChatHandler::HandleUnBanCharacterCommand, "Unbans character x",                                  NULL, 0, 0, 0 },
        { NULL,        '0', NULL,                                      "",                                                    NULL, 0, 0, 0 }
    };
    dupe_command_table(unbanCommandTable, _unbanCommandTable);

    static ChatCommand instanceCommandTable[] =
    {
        { "create",   'z', &ChatHandler::HandleCreateInstanceCommand,    "Generically instances a map that requires instancing, mapid x y z",              NULL, 0, 0, 0 },
        { "reset",    'z', &ChatHandler::HandleResetInstanceCommand,     "Removes instance ID x from target player.",                         NULL, 0, 0, 0 },
        { "resetall", 'm', &ChatHandler::HandleResetAllInstancesCommand, "Removes all instance IDs from target player.",                      NULL, 0, 0, 0 },
        { "shutdown", 'z', &ChatHandler::HandleShutdownInstanceCommand,  "Shutdown instance with ID x (default is current instance).",        NULL, 0, 0, 0 },
        { "info",     'm', &ChatHandler::HandleGetInstanceInfoCommand,   "Gets info about instance with ID x (default is current instance).", NULL, 0, 0, 0 },
        { "exit",     'm', &ChatHandler::HandleExitInstanceCommand,      "Exits current instance, return to entry point.",                    NULL, 0, 0, 0 },
        { NULL,       '0', NULL,                                         "",                                                                  NULL, 0, 0, 0 }
    };
    dupe_command_table(instanceCommandTable, _instanceCommandTable);

    static ChatCommand arenaCommandTable[] =
    {
        { "createteam",      'e', &ChatHandler::HandleArenaCreateTeam,      "Creates arena team with <type> <name>",                            NULL, 0, 0, 0 },
        { "setteamleader",   'e', &ChatHandler::HandleArenaSetTeamLeader,   "Sets the arena team leader for <type>",                    NULL, 0, 0, 0 },
        { "resetallratings", 'z', &ChatHandler::HandleArenaTeamResetAllRatings, "Resets all arena teams to their default rating", NULL, 0, 0, 0 },
        { NULL,              '0', NULL,                                            "",                                              NULL, 0, 0, 0 }
    };
    dupe_command_table(arenaCommandTable, _arenaCommandTable);

    static ChatCommand achievementCommandTable[] =
    {
#ifdef ENABLE_ACHIEVEMENTS
        { "complete", 'm', &ChatHandler::HandleAchievementCompleteCommand, "Completes the specified achievement.",          NULL, 0, 0, 0 },
        { "criteria", 'm', &ChatHandler::HandleAchievementCriteriaCommand, "Completes the specified achievement criteria.", NULL, 0, 0, 0 },
        { "reset",    'm', &ChatHandler::HandleAchievementResetCommand,    "Resets achievement data from the target.",      NULL, 0, 0, 0 },
#endif
        { NULL,       '0', NULL,                                           "",                                              NULL, 0, 0, 0 }
    };
    dupe_command_table(achievementCommandTable, _achievementCommandTable);

    static ChatCommand vehicleCommandTable[] = {
        { "ejectpassenger",       'm', &ChatHandler::HandleVehicleEjectPassengerCommand,     "Ejects the passenger from the specified seat",      NULL, 0, 0, 0 },
        { "ejectallpassengers",   'm', &ChatHandler::HandleVehicleEjectAllPassengersCommand, "Ejects all passengers from the vehicle",            NULL, 0, 0, 0 },
        { "installaccessories",   'm', &ChatHandler::HandleVehicleInstallAccessoriesCommand, "Installs the accessories for the selected vehicle", NULL, 0, 0, 0 },
        { "removeaccessories",    'm', &ChatHandler::HandleVehicleRemoveAccessoriesCommand,  "Removes the accessories of the selected vehicle",   NULL, 0, 0, 0 },
        { "addpassenger",         'm', &ChatHandler::HandleVehicleAddPassengerCommand,       "Adds a new NPC passenger to the vehicle",           NULL, 0, 0, 0 },
        { NULL,                   '0', NULL,                                                 "",                                                  NULL, 0, 0, 0 }
    };

    dupe_command_table(vehicleCommandTable, _vehicleCommandTable);

    static ChatCommand commandTable[] =
    {
        { "commands",        '0', &ChatHandler::HandleCommandsCommand,                      "Shows commands",                                                                                                                          NULL,                     0, 0, 0 },
        { "help",            '0', &ChatHandler::HandleHelpCommand,                          "Shows help for command",                                                                                                                  NULL,                     0, 0, 0 },
        { "autosavechanges",  '1', &ChatHandler::HandleAutoSaveChangesCommand,                "Toggles activated/deactivated auto execute commands to save to the corresponding db table.",                                                                                                                  NULL,                     0, 0, 0 },
        { "event", '0', NULL, "", eventCommandTable, 0, 0, 0 },
        //debug
        { "calcdist",        '0', &ChatHandler::HandleSimpleDistanceCommand,                "Display the distance between your current position and the specified point x y z",                                                           NULL,                     0, 0, 0 },
        { "announce",        'u', &ChatHandler::HandleAnnounceCommand,                      "Sends a normal chat message broadcast to all players.",                                                                                                                        NULL,                     0, 0, 0 },
        { "wannounce",       'u', &ChatHandler::HandleWAnnounceCommand,                     "Sends a widescreen raid style announcement to all players.",                                                                                                             NULL,                     0, 0, 0 },
        // teleport
        { "appear",          'v', &ChatHandler::HandleAppearCommand,                        "Teleports to x's position.",                                                                                                              NULL,                     0, 0, 0 },
        { "summon",          'v', &ChatHandler::HandleSummonCommand,                        "Summons x to your position.",                                                                                                              NULL,                     0, 0, 0 },
        { "kill",            'r', &ChatHandler::HandleKillCommand,                          ".kill - Kills selected unit .kill <playername> kills player with <playername>",                                                                                                            NULL,                     0, 0, 0 },
        { "revive",          'r', &ChatHandler::HandleReviveCommand,                        ".revive - revives you or a selected target .revive <player_name> revives player with <playername>",                                                                                                                            NULL,                     0, 0, 0 },
        //misc player/creature
        { "mount",           'm', &ChatHandler::HandleMountCommand,                         "Mounts into modelid x.",                                                                                                                  NULL,                     0, 0, 0 },
        //misc player/creature
        { "dismount",        'h', &ChatHandler::HandleDismountCommand,                      "Dismounts.",                                                                                                                              NULL,                     0, 0, 0 },
        { "gps",             '0', &ChatHandler::HandleGPSCommand,                           "Shows Position",                                                                                                                          NULL,                     0, 0, 0 },
        //teleport
        { "worldport",       'v', &ChatHandler::HandleWorldPortCommand,                     "Teleports you to a location with mapid x y z",                                                                                                                                        NULL,                     0, 0, 0 },
        //teleport
        { "start",           'm', &ChatHandler::HandleStartCommand,                         "Teleports you to a starting location",                                                                                                   NULL,                     0, 0, 0 },
        { "invincible",      'j', &ChatHandler::HandleInvincibleCommand,                    ".invincible - Toggles INVINCIBILITY (mobs won't attack you)",                                                                             NULL,                     0, 0, 0 },
        { "invisible",       'i', &ChatHandler::HandleInvisibleCommand,                     ".invisible - Toggles INVINCIBILITY and INVISIBILITY (mobs won't attack you and nobody can see you, but they can see your chat messages)", NULL,                     0, 0, 0 },
        { "playerinfo",      'm', &ChatHandler::HandlePlayerInfo,                           ".playerinfo - Displays information about the selected character (account...)",                                                           NULL,                     0, 0, 0 },
        { "modify",          '0', NULL,                                                     "",                                                                                                                                        modifyCommandTable,       0, 0, 0 },
        { "waypoint",        '0', NULL,                                                     "",                                                                                                                                        waypointCommandTable,     0, 0, 0 },
        { "debug",           '0', NULL,                                                     "",                                                                                                                                        debugCommandTable,        0, 0, 0 },
        { "gm",              '0', NULL,                                                     "",                                                                                                                                        gmCommandTable,           0, 0, 0 },
        { "gmTicket",        '0', NULL,                                                     "",                                                                                                                                        GMTicketCommandTable,     0, 0, 0 },
        { "ticket",          '0', NULL,                                                     "",                                                                                                                                        TicketCommandTable,       0, 0, 0 },
        { "gobject",         '0', NULL,                                                     "",                                                                                                                                        GameObjectCommandTable,   0, 0, 0 },
        { "battleground",    '0', NULL,                                                     "",                                                                                                                                        BattlegroundCommandTable, 0, 0, 0 },
        { "npc",             '0', NULL,                                                     "",                                                                                                                                        NPCCommandTable,          0, 0, 0 },
        { "cheat",           '0', NULL,                                                     "",                                                                                                                                        CheatCommandTable,        0, 0, 0 },
        { "account",         '0', NULL,                                                     "",                                                                                                                                        accountCommandTable,      0, 0, 0 },
        { "quest",           '0', NULL,                                                     "",                                                                                                                                        questCommandTable,        0, 0, 0 },
        { "pet",             '0', NULL,                                                     "",                                                                                                                                        petCommandTable,          0, 0, 0 },
        { "recall",          '0', NULL,                                                     "",                                                                                                                                        recallCommandTable,       0, 0, 0 },
        { "guild",           '0', NULL,                                                     "",                                                                                                                                        GuildCommandTable,        0, 0, 0 },
        { "server",          '0', NULL,                                                     "",                                                                                                                                        serverCommandTable,       0, 0, 0 },
        { "character",       '0', NULL,                                                     "",                                                                                                                                        characterCommandTable,    0, 0, 0 },
        { "lookup",          '0', NULL,                                                     "",                                                                                                                                        lookupCommandTable,       0, 0, 0 },
        { "admin",           '0', NULL,                                                     "",                                                                                                                                        adminCommandTable,        0, 0, 0 },
        { "kick",            '0', NULL,                                                     "",                                                                                                                                        kickCommandTable,         0, 0, 0 },
        { "ban",             '0', NULL,                                                     "",                                                                                                                                        banCommandTable,          0, 0, 0 },
        { "unban",           '0', NULL,                                                     "",                                                                                                                                        unbanCommandTable,        0, 0, 0 },
        { "instance",        '0', NULL,                                                     "",                                                                                                                                        instanceCommandTable,     0, 0, 0 },
        { "arena",           '0', NULL,                                                     "",                                                                                                                                        arenaCommandTable,        0, 0, 0 },
        { "unroot",          'b', &ChatHandler::HandleUnrootCommand,                        "Unroots selected target.",                                                                                                                NULL,                     0, 0, 0 },
        { "root",            'b', &ChatHandler::HandleRootCommand,                          "Roots selected target.",                                                                                                                  NULL,                     0, 0, 0 },
        { "gocreature",      'v', &ChatHandler::HandleGoCreatureSpawnCommand,               "Teleports you to the creature with <spwn_id>.",                                                                                           NULL,                     0, 0, 0 },
        { "gogameobject",    'v', &ChatHandler::HandleGoGameObjectSpawnCommand,             "Teleports you to the gameobject with <spawn_id>.",                                                                                        NULL,                     0, 0, 0 },
        { "gotrig",          'v', &ChatHandler::HandleGoTriggerCommand,                     "Teleports you to the areatrigger with <id>.",                                                                                             NULL,                     0, 0, 0 },
        //npc
        { "fixscale",        'm', &ChatHandler::HandleFixScaleCommand,                      "",                                                                                                                                        NULL,                     0, 0, 0 },
        { "achieve",         '0', NULL,                                                     "",                                                                                                                                        achievementCommandTable,  0, 0, 0 },
        { "vehicle",         'm', NULL,                                                     "",                                                                                                                                        vehicleCommandTable,      0, 0, 0 },
        { "transport",       'm', NULL,                                                     "",                                                                                                                                        transportCommandTable,    0, 0, 0 },
        { NULL,              '0', NULL,                                                     "",                                                                                                                                        NULL,                     0, 0, 0 }
    };
    dupe_command_table(commandTable, _commandTable);

    /* set the correct pointers */
    ChatCommand* p = &_commandTable[0];
    while(p->Name != 0)
    {
        if (p->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np = GetSubCommandTable(p->Name);
            ARCEMU_ASSERT(np != NULL);
            p->ChildCommands = np;
        }
        ++p;
    }

    // set subcommand for .npc command table
    ChatCommand* p_char = &_characterCommandTable[0];
    while (p_char->Name != 0)
    {
        if (p_char->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np_char = GetCharSubCommandTable(p_char->Name);
            ARCEMU_ASSERT(np_char != NULL);
            p_char->ChildCommands = np_char;
        }
        ++p_char;
    }

    // set subcommand for .npc command table
    ChatCommand* p_npc = &_NPCCommandTable[0];
    while (p_npc->Name != 0)
    {
        if (p_npc->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np_npc = GetNPCSubCommandTable(p_npc->Name);
            ARCEMU_ASSERT(np_npc != NULL);
            p_npc->ChildCommands = np_npc;
        }
        ++p_npc;
    }

    // set subcommand for .gobject command table
    ChatCommand* p_gobject = &_GameObjectCommandTable[0];
    while (p_gobject->Name != 0)
    {
        if (p_gobject->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np_gobject = GetGOSubCommandTable(p_gobject->Name);
            ARCEMU_ASSERT(np_gobject != NULL);
            p_gobject->ChildCommands = np_gobject;
        }
        ++p_gobject;
    }

    // set subcommand for .reload command table
    ChatCommand* p_reloadtable = &_reloadTableCommandTable[0];
    while (p_reloadtable->Name != 0)
    {
        if (p_reloadtable->ChildCommands != 0)
        {
            // Set the correct pointer.
            ChatCommand* np_reloadtable = GetReloadCommandTable(p_reloadtable->Name);
            ARCEMU_ASSERT(np_reloadtable != NULL);
            p_reloadtable->ChildCommands = np_reloadtable;
        }
        ++p_reloadtable;
    }
}

ChatHandler::ChatHandler()
{
    new CommandTableStorage;
    CommandTableStorage::getSingleton().Init();
    SkillNameManager = new SkillNameMgr;
}

ChatHandler::~ChatHandler()
{
    CommandTableStorage::getSingleton().Dealloc();
    delete CommandTableStorage::getSingletonPtr();
    delete SkillNameManager;
}

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
    for (;;)
    {
        if (!*s2)
            return true;
        else if (!*s1)
            return false;
        else if (tolower(*s1) != tolower(*s2))
            return false;
        s1++;
        s2++;
    }
}

void ChatHandler::SendMultilineMessage(WorldSession* m_session, const char* str)
{
    char* start = (char*)str, *end;
    for (;;)
    {
        end = strchr(start, '\n');
        if (!end)
            break;

        *end = '\0';
        SystemMessage(m_session, start);
        start = end + 1;
    }
    if (*start != '\0')
        SystemMessage(m_session, start);
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand* table, const char* text, WorldSession* m_session)
{
    std::string cmd = "";

    // get command
    while(*text != ' ' && *text != '\0')
    {
        cmd += *text;
        text++;
    }

    while(*text == ' ') text++;  // skip whitespace

    if (!cmd.length())
        return false;

    for (uint32 i = 0; table[i].Name != NULL; i++)
    {
        if (!hasStringAbbr(table[i].Name, cmd.c_str()))
            continue;

        if (table[i].CommandGroup != '0' && !m_session->CanUseCommand(table[i].CommandGroup))
            continue;

        if (table[i].ChildCommands != NULL)
        {
            if (!ExecuteCommandInTable(table[i].ChildCommands, text, m_session))
            {
                if (table[i].Help != "")
                    SendMultilineMessage(m_session, table[i].Help.c_str());
                else
                {
                    GreenSystemMessage(m_session, "Available Subcommands:");
                    for (uint32 k = 0; table[i].ChildCommands[k].Name; k++)
                    {
                        if (table[i].ChildCommands[k].CommandGroup == '0' || (table[i].ChildCommands[k].CommandGroup != '0' && m_session->CanUseCommand(table[i].ChildCommands[k].CommandGroup)))
                        {
                            BlueSystemMessage(m_session, " %s - %s", table[i].ChildCommands[k].Name, table[i].ChildCommands[k].Help.size() ? table[i].ChildCommands[k].Help.c_str() : "No Help Available");
                        }
                    }
                }
            }

            return true;
        }

        // Check for field-based commands
        if (table[i].Handler == NULL && (table[i].MaxValueField || table[i].NormalValueField))
        {
            bool result = false;
            if (strlen(text) == 0)
            {
                RedSystemMessage(m_session, "No values specified.");
            }
            if (table[i].ValueType == 2)
                result = CmdSetFloatField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
            else
                result = CmdSetValueField(m_session, table[i].NormalValueField, table[i].MaxValueField, table[i].Name, text);
            if (!result)
                RedSystemMessage(m_session, "Must be in the form of (command) <value>, or, (command) <value> <maxvalue>");
        }
        else
        {
            if (!(this->*(table[i].Handler))(text, m_session))
            {
                if (table[i].Help != "")
                    SendMultilineMessage(m_session, table[i].Help.c_str());
                else
                {
                    RedSystemMessage(m_session, "Incorrect syntax specified. Try .help %s for the correct syntax.", table[i].Name);
                }
            }
        }

        return true;
    }
    return false;
}

int ChatHandler::ParseCommands(const char* text, WorldSession* session)
{
    if (!session)
        return 0;

    if (!*text)
        return 0;

    if (session->GetPermissionCount() == 0 && sWorld.m_reqGmForCommands)
        return 0;

    if (text[0] != '!' && text[0] != '.') // let's not confuse users
        return 0;

    /* skip '..' :P that pisses me off */
    if (text[1] == '.')
        return 0;

    text++;

    try
    {
        bool success = ExecuteCommandInTable(CommandTableStorage::getSingleton().Get(), text, session);
        if (!success)
        {
            SystemMessage(session, "There is no such command, or you do not have access to it.");
        }
    }
    catch (AscEmu::Exception::PlayerNotFoundException e)
    {
        // TODO: Handle this properly (what do we do when we're running commands with no player object?)
        LOG_ERROR("PlayerNotFoundException occurred when processing command [%s]. Exception: %s", text, e.what());
    }

    return 1;
}

WorldPacket* ChatHandler::FillMessageData(uint32 type, uint32 language, const char* message, uint64 guid , uint8 flag) const
{
    //Packet    structure
    //uint8        type;
    //uint32    language;
    //uint64    guid;
    //uint64    guid;
    //uint32    len_of_text;
    //char        text[];         // not sure ? i think is null terminated .. not null terminated
    //uint8        afk_state;
    ARCEMU_ASSERT(type != CHAT_MSG_CHANNEL);
    //channels are handled in channel handler and so on
    uint32 messageLength = (uint32)strlen(message) + 1;

    WorldPacket* data = new WorldPacket(SMSG_MESSAGECHAT, messageLength + 30);

    *data << uint8(type);
    *data << language;

    *data << guid;
    *data << uint32(0);

    *data << guid;

    *data << messageLength;
    *data << message;

    *data << uint8(flag);
    return data;
}

WorldPacket* ChatHandler::FillSystemMessageData(const char* message) const
{
    uint32 messageLength = (uint32)strlen(message) + 1;

    WorldPacket* data = new WorldPacket(SMSG_MESSAGECHAT, 30 + messageLength);
    *data << uint8(CHAT_MSG_SYSTEM);
    *data << uint32(LANG_UNIVERSAL);

    // Who cares about guid when there's no nickname displayed heh ?
    *data << uint64(0);
    *data << uint32(0);
    *data << uint64(0);

    *data << messageLength;
    *data << message;

    *data << uint8(0);

    return data;
}

Player* ChatHandler::GetSelectedPlayer(WorldSession* m_session, bool showerror, bool auto_self)
{
    if (m_session == nullptr)
        return nullptr;

    bool is_creature = false;
    Player* player_target = nullptr;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    switch (GET_TYPE_FROM_GUID(guid))
    {
        case HIGHGUID_TYPE_PET:
        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
        {
            is_creature = true;
            break;
        }
        default:
            break;
    }

    if (guid == 0 || is_creature)
    {
        if (auto_self)
        {
            GreenSystemMessage(m_session, "Auto-targeting self.");
            player_target = m_session->GetPlayer();
        }
        else
        {
            if (showerror)
                RedSystemMessage(m_session, "This command requires a selected player.");

            return nullptr;
        }
    }
    else
    {
        player_target = m_session->GetPlayer()->GetMapMgr()->GetPlayer((uint32)guid);
    }

    return player_target;
}

Creature* ChatHandler::GetSelectedCreature(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr)
        return nullptr;

    Creature* creature = nullptr;
    bool is_invalid_type = false;
    uint64 guid = m_session->GetPlayer()->GetSelection();

    switch(GET_TYPE_FROM_GUID(guid))
    {
        case HIGHGUID_TYPE_PET:
            creature = m_session->GetPlayer()->GetMapMgr()->GetPet(GET_LOWGUID_PART(guid));
            break;

        case HIGHGUID_TYPE_UNIT:
        case HIGHGUID_TYPE_VEHICLE:
            creature = m_session->GetPlayer()->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
            break;
        default:
            is_invalid_type = true;
            break;
    }

    if (creature == nullptr || is_invalid_type)
    {
        if (showerror)
            RedSystemMessage(m_session, "This command requires a selected a creature.");

        return nullptr;
    }

    return creature;
}

Unit* ChatHandler::GetSelectedUnit(WorldSession* m_session, bool showerror)
{
    if (m_session == nullptr || m_session->GetPlayer() == nullptr)
        return nullptr;

    uint64 guid = m_session->GetPlayer()->GetSelection();
    
    Unit* unit = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid);
    if (unit == nullptr)
    {
        if (showerror)
            RedSystemMessage(m_session, "You need to select a unit!");
        return nullptr;
    }

    return unit;
}

uint32 ChatHandler::GetSelectedWayPointId(WorldSession* m_session)
{
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid == 0)
    {
        SystemMessage(m_session, "No selection.");
        return 0;
    }

    if (GET_TYPE_FROM_GUID(guid) != HIGHGUID_TYPE_WAYPOINT)
    {
        SystemMessage(m_session, "You should select a Waypoint.");
        return 0;
    }

    return Arcemu::Util::GUID_LOPART(guid);
}

const char* ChatHandler::GetMapTypeString(uint8 type)
{
    switch (type)
    {
        case INSTANCE_NULL:
            return "Continent";
        case INSTANCE_RAID:
            return "Raid";
        case INSTANCE_NONRAID:
            return "Non-Raid";
        case INSTANCE_BATTLEGROUND:
            return "PvP";
        case INSTANCE_MULTIMODE:
            return "MultiMode";
        default:
            return "Unknown";
    }
}

const char* ChatHandler::GetDifficultyString(uint8 difficulty)
{
    switch (difficulty)
    {
        case MODE_NORMAL:
            return "normal";
        case MODE_HEROIC:
            return "heroic";
        default:
            return "unknown";
    }
}

const char* ChatHandler::GetRaidDifficultyString(uint8 diff)
{
    switch (diff)
    {
        case MODE_NORMAL_10MEN:
            return "normal 10men";
        case MODE_NORMAL_25MEN:
            return "normal 25men";
        case MODE_HEROIC_10MEN:
            return "heroic 10men";
        case MODE_HEROIC_25MEN:
            return "heroic 25men";
        default:
            return "unknown";
    }
}

void ChatHandler::SystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    WorldPacket* data = FillSystemMessageData(msg1);
    if (m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::ColorSystemMessage(WorldSession* m_session, const char* colorcode, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", colorcode, msg1);

    WorldPacket* data = FillSystemMessageData(msg);
    if (m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::RedSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_LIGHTRED/*MSG_COLOR_RED*/, msg1);

    WorldPacket* data = FillSystemMessageData(msg);
    if (m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::GreenSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_GREEN, msg1);

    WorldPacket* data = FillSystemMessageData(msg);
    if (m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

void ChatHandler::BlueSystemMessage(WorldSession* m_session, const char* message, ...)
{
    if (!message)
        return;

    va_list ap;
    va_start(ap, message);
    char msg1[1024];
    vsnprintf(msg1, 1024, message, ap);
    va_end(ap);

    char msg[1024];
    snprintf(msg, 1024, "%s%s|r", MSG_COLOR_LIGHTBLUE, msg1);

    WorldPacket* data = FillSystemMessageData(msg);
    if (m_session != NULL)
        m_session->SendPacket(data);
    delete data;
}

bool ChatHandler::CmdSetValueField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args)
{
    char* pvalue;
    uint32 mv, av;

    if (!args || !m_session) return false;

    pvalue = strtok((char*)args, " ");
    if (!pvalue)
        return false;
    else
        av = atol(pvalue);

    if (fieldmax)
    {
        char* pvaluemax = strtok(NULL, " ");
        if (!pvaluemax)
            return false;
        else
            mv = atol(pvaluemax);
    }
    else
    {
        mv = 0;
    }

    if (av <= 0 && mv > 0)
    {
        RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
        return true;
    }
    if (fieldmax)
    {
        if (mv < av || mv <= 0)
        {
            RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
            return true;
        }
    }

    Player* plr = GetSelectedPlayer(m_session, false, true);
    if (plr)
    {
        sGMLog.writefromsession(m_session, "used modify field value: %s, %u on %s", fieldname, av, plr->GetName());
        if (fieldmax)
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %d/%d.", fieldname, plr->GetName(), av, mv);
            GreenSystemMessage(plr->GetSession(), "%s set your %s to %d/%d.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %d.", fieldname, plr->GetName(), av);
            GreenSystemMessage(plr->GetSession(), "%s set your %s to %d.", m_session->GetPlayer()->GetName(), fieldname, av);
        }

        if (field == UNIT_FIELD_STAT1) av /= 2;
        if (field == UNIT_FIELD_BASE_HEALTH)
        {
            plr->SetHealth(av);
        }

        plr->SetUInt32Value(field, av);

        if (fieldmax)
        {
            plr->SetUInt32Value(fieldmax, mv);
        }
    }
    else
    {
        Creature* cr = GetSelectedCreature(m_session, false);
        if (cr)
        {
            if (!(field < UNIT_END && fieldmax < UNIT_END)) return false;
            std::string creaturename = cr->GetCreatureProperties()->Name;
            if (fieldmax)
                BlueSystemMessage(m_session, "Setting %s of %s to %d/%d.", fieldname, creaturename.c_str(), av, mv);
            else
                BlueSystemMessage(m_session, "Setting %s of %s to %d.", fieldname, creaturename.c_str(), av);
            sGMLog.writefromsession(m_session, "used modify field value: [creature]%s, %u on %s", fieldname, av, creaturename.c_str());
            if (field == UNIT_FIELD_STAT1) av /= 2;
            if (field == UNIT_FIELD_BASE_HEALTH)
                cr->SetHealth(av);

            switch(field)
            {
                case UNIT_FIELD_FACTIONTEMPLATE:
                    {
                        if (cr->m_spawn)
                            WorldDatabase.Execute("UPDATE creature_spawns SET faction = %u WHERE entry = %u", av, cr->m_spawn->entry);
                    }
                    break;
                case UNIT_NPC_FLAGS:
                    {
                        WorldDatabase.Execute("UPDATE creature_properties SET npcflags = %u WHERE entry = %u", av, cr->GetCreatureProperties()->Id);
                    }
                    break;
            }

            cr->SetUInt32Value(field, av);

            if (fieldmax)
            {
                cr->SetUInt32Value(fieldmax, mv);
            }
            // reset faction
            if (field == UNIT_FIELD_FACTIONTEMPLATE)
                cr->_setFaction();

            // Only actually save the change if we are modifying a spawn
            if (cr->GetSQL_id() != 0)
                cr->SaveToDB();
        }
        else
        {
            RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

bool ChatHandler::CmdSetFloatField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args)
{
    char* pvalue;
    float mv, av;

    if (!args || !m_session) return false;

    pvalue = strtok((char*)args, " ");
    if (!pvalue)
        return false;
    else
        av = (float)atof(pvalue);

    if (fieldmax)
    {
        char* pvaluemax = strtok(NULL, " ");
        if (!pvaluemax)
            return false;
        else
            mv = (float)atof(pvaluemax);
    }
    else
    {
        mv = 0;
    }

    if (av <= 0)
    {
        RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
        return true;
    }
    if (fieldmax)
    {
        if (mv < av || mv <= 0)
        {
            RedSystemMessage(m_session, "Values are invalid. Value must be < max (if max exists), and both must be > 0.");
            return true;
        }
    }

    Player* plr = GetSelectedPlayer(m_session, false, true);
    if (plr)
    {
        sGMLog.writefromsession(m_session, "used modify field value: %s, %f on %s", fieldname, av, plr->GetName());
        if (fieldmax)
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %.1f/%.1f.", fieldname, plr->GetName(), av, mv);
            GreenSystemMessage(plr->GetSession(), "%s set your %s to %.1f/%.1f.", m_session->GetPlayer()->GetName(), fieldname, av, mv);
        }
        else
        {
            BlueSystemMessage(m_session, "You set the %s of %s to %.1f.", fieldname, plr->GetName(), av);
            GreenSystemMessage(plr->GetSession(), "%s set your %s to %.1f.", m_session->GetPlayer()->GetName(), fieldname, av);
        }
        plr->SetFloatValue(field, av);
        if (fieldmax) plr->SetFloatValue(fieldmax, mv);
    }
    else
    {
        Creature* cr = GetSelectedCreature(m_session, false);
        if (cr)
        {
            if (!(field < UNIT_END && fieldmax < UNIT_END)) return false;
            std::string creaturename = cr->GetCreatureProperties()->Name;
            if (fieldmax)
                BlueSystemMessage(m_session, "Setting %s of %s to %.1f/%.1f.", fieldname, creaturename.c_str(), av, mv);
            else
                BlueSystemMessage(m_session, "Setting %s of %s to %.1f.", fieldname, creaturename.c_str(), av);
            cr->SetFloatValue(field, av);
            sGMLog.writefromsession(m_session, "used modify field value: [creature]%s, %f on %s", fieldname, av, creaturename.c_str());
            if (fieldmax)
            {
                cr->SetFloatValue(fieldmax, mv);
            }
            //cr->SaveToDB();
        }
        else
        {
            RedSystemMessage(m_session, "Invalid Selection.");
        }
    }
    return true;
}

std::string ChatHandler::GetNpcFlagString(Creature* creature)
{
    std::string s = "";
    if (creature->isBattleMaster())
        s.append(" (Battlemaster)");
    if (creature->isTrainer())
        s.append(" (Trainer)");
    if (creature->isProf())
        s.append(" (Profession Trainer)");
    if (creature->isQuestGiver())
        s.append(" (Quests)");
    if (creature->isGossip())
        s.append(" (Gossip)");
    if (creature->isTaxi())
        s.append(" (Taxi)");
    if (creature->isCharterGiver())
        s.append(" (Charter)");
    if (creature->isGuildBank())
        s.append(" (Guild Bank)");
    if (creature->isSpiritHealer())
        s.append(" (Spirit Healer)");
    if (creature->isInnkeeper())
        s.append(" (Innkeeper)");
    if (creature->isTabardDesigner())
        s.append(" (Tabard Designer)");
    if (creature->isAuctioner())
        s.append(" (Auctioneer)");
    if (creature->isStableMaster())
        s.append(" (Stablemaster)");
    if (creature->isArmorer())
        s.append(" (Armorer)");

    return s;
}

std::string ChatHandler::MyConvertIntToString(const int arg)
{
    std::stringstream out;
    out << arg;
    return out.str();
}

std::string ChatHandler::MyConvertFloatToString(const float arg)
{
    std::stringstream out;
    out << arg;
    return out.str();
}
