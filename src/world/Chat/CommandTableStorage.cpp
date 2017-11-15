/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "ChatCommand.hpp"
#include "CommandTableStorage.hpp"
#include "ChatHandler.hpp"

initialiseSingleton(CommandTableStorage);

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
    } while (result->NextRow());
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
                    LOG_DEBUG("Changing command level of .`%s` to %c.", main_command.c_str(), level[0]);
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
                            LOG_DEBUG("Changing command level of .`%s %s` to %c.", main_command.c_str(), sub_command.c_str(), level[0]);
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
                                LOG_DEBUG("Changing command level of .`%s %s %s` to %c.", main_command.c_str(), sub_command.c_str(), sec_sub_command.c_str(), level[0]);
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
        { "hp",                 'm', nullptr,       "Mods health points (HP) of selected target",           nullptr, UNIT_FIELD_HEALTH,                 UNIT_FIELD_MAXHEALTH, 1 },
        { "mana",               'm', nullptr,       "Mods mana points (MP) of selected target.",            nullptr, UNIT_FIELD_POWER1,                 UNIT_FIELD_MAXPOWER1, 1 },
        { "rage",               'm', nullptr,       "Mods rage points of selected target.",                 nullptr, UNIT_FIELD_POWER2,                 UNIT_FIELD_MAXPOWER2, 1 },
        { "energy",             'm', nullptr,       "Mods energy points of selected target.",               nullptr, UNIT_FIELD_POWER4,                 UNIT_FIELD_MAXPOWER4, 1 },
#if VERSION_STRING == WotLK
        { "runicpower",         'm', nullptr,       "Mods runic power points of selected target.",          nullptr, UNIT_FIELD_POWER7,                 UNIT_FIELD_MAXPOWER7, 1 },
#endif
        { "strength",           'm', nullptr,       "Mods strength value of the selected target.",          nullptr, UNIT_FIELD_STAT0,                  0,                    1 },
        { "agility",            'm', nullptr,       "Mods agility value of the selected target.",           nullptr, UNIT_FIELD_STAT1,                  0,                    1 },
        { "intelligence",       'm', nullptr,       "Mods intelligence value of the selected target.",      nullptr, UNIT_FIELD_STAT3,                  0,                    1 },
        { "spirit",             'm', nullptr,       "Mods spirit value of the selected target.",            nullptr, UNIT_FIELD_STAT4,                  0,                    1 },
        { "armor",              'm', nullptr,       "Mods armor of selected target.",                       nullptr, UNIT_FIELD_RESISTANCES,            0,                    1 },
        { "holy",               'm', nullptr,       "Mods holy resistance of selected target.",             nullptr, UNIT_FIELD_RESISTANCES + 1,        0,                    1 },
        { "fire",               'm', nullptr,       "Mods fire resistance of selected target.",             nullptr, UNIT_FIELD_RESISTANCES + 2,        0,                    1 },
        { "nature",             'm', nullptr,       "Mods nature resistance of selected target.",           nullptr, UNIT_FIELD_RESISTANCES + 3,        0,                    1 },
        { "frost",              'm', nullptr,       "Mods frost resistance of selected target.",            nullptr, UNIT_FIELD_RESISTANCES + 4,        0,                    1 },
        { "shadow",             'm', nullptr,       "Mods shadow resistance of selected target.",           nullptr, UNIT_FIELD_RESISTANCES + 5,        0,                    1 },
        { "arcane",             'm', nullptr,       "Mods arcane resistance of selected target.",           nullptr, UNIT_FIELD_RESISTANCES + 6,        0,                    1 },
        { "damage",             'm', nullptr,       "Mods damage done by the selected target.",             nullptr, UNIT_FIELD_MINDAMAGE,              UNIT_FIELD_MAXDAMAGE, 2 },
        { "ap",                 'm', nullptr,       "Mods attack power of the selected target.",            nullptr, UNIT_FIELD_ATTACK_POWER,           0,                    1 },
        { "rangeap",            'm', nullptr,       "Mods range attack power of the selected target.",      nullptr, UNIT_FIELD_RANGED_ATTACK_POWER,    0,                    1 },
        { "scale",              'm', nullptr,       "Mods scale of the selected target.",                   nullptr, OBJECT_FIELD_SCALE_X,              0,                    2 },
        { "nativedisplayid",    'm', nullptr,       "Mods native display identifier of the target.",        nullptr, UNIT_FIELD_NATIVEDISPLAYID,        0,                    1 },
        { "displayid",          'm', nullptr,       "Mods display identifier (DisplayID) of the target.",   nullptr, UNIT_FIELD_DISPLAYID,              0,                    1 },
        { "flags",              'm', nullptr,       "Mods flags of the selected target.",                   nullptr, UNIT_FIELD_FLAGS,                  0,                    1 },
        { "faction",            'm', nullptr,       "Mods faction template of the selected target.",        nullptr, UNIT_FIELD_FACTIONTEMPLATE,        0,                    1 },
        { "dynamicflags",       'm', nullptr,       "Mods dynamic flags of the selected target.",           nullptr, UNIT_DYNAMIC_FLAGS,                0,                    1 },
        { "happiness",          'm', nullptr,       "Mods happiness value of the selected target.",         nullptr, UNIT_FIELD_POWER5,                 UNIT_FIELD_MAXPOWER5, 1 },
        { "boundingraidius",    'm', nullptr,       "Mods bounding radius of the selected target.",         nullptr, UNIT_FIELD_BOUNDINGRADIUS,         0,                    2 },
        { "combatreach",        'm', nullptr,       "Mods combat reach of the selected target.",            nullptr, UNIT_FIELD_COMBATREACH,            0,                    2 },
        { "emotestate",         'm', nullptr,       "Mods Unit emote state of the selected target.",        nullptr, UNIT_NPC_EMOTESTATE,               0,                    1 },
        { "bytes0",             'm', nullptr,       "Mods bytes0 entry of selected target.",                nullptr, UNIT_FIELD_BYTES_0,                0,                    1 },
        { "bytes1",             'm', nullptr,       "Mods bytes1 entry of selected target.",                nullptr, UNIT_FIELD_BYTES_1,                0,                    1 },
        { "bytes2",             'm', nullptr,       "Mods bytes2 entry of selected target.",                nullptr, UNIT_FIELD_BYTES_2,                0,                    1 },
        { nullptr,              '0', nullptr,       "",                                                     nullptr, 0,                                 0,                    0 }
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
        { "dopctdamage",        'z', &ChatHandler::HandleDoPercentDamageCommand,    "Do percent damage to creature target",                     nullptr, 0, 0, 0 },
        { "setscriptphase",     'z', &ChatHandler::HandleSetScriptPhaseCommand,     "ScriptPhase test",                                         nullptr, 0, 0, 0 },
        { "aicharge",           'z', &ChatHandler::HandleAiChargeCommand,           "AiCharge test",                                            nullptr, 0, 0, 0 },
        { "aiknockback",        'z', &ChatHandler::HandleAiKnockbackCommand,        "AiKnockBack test",                                         nullptr, 0, 0, 0 },
        { "aijump",             'z', &ChatHandler::HandleAiJumpCommand,             "AiJump test",                                              nullptr, 0, 0, 0 },
        { "aifalling",          'z', &ChatHandler::HandleAiFallingCommand,          "AiFalling test",                                           nullptr, 0, 0, 0 },
        { "movetospawn",        'z', &ChatHandler::HandleMoveToSpawnCommand,        "Move target to spwn",                                      nullptr, 0, 0, 0 },
        { "position",           'z', &ChatHandler::HandlePositionCommand,           "Show position",                                            nullptr, 0, 0, 0 },
        { "setorientation",     'z', &ChatHandler::HandleSetOrientationCommand,     "Sets orientation on npc",                                  nullptr, 0, 0, 0 },
        { "dumpmovement",       'd', &ChatHandler::HandleDebugDumpMovementCommand,  "Dumps the player's movement information to chat",          nullptr, 0, 0, 0 },
        { "infront",            'd', &ChatHandler::HandleDebugInFrontCommand,       "",                                                         nullptr, 0, 0, 0 },
        { "showreact",          'd', &ChatHandler::HandleShowReactionCommand,       "",                                                         nullptr, 0, 0, 0 },
        { "aimove",             'd', &ChatHandler::HandleAIMoveCommand,             "",                                                         nullptr, 0, 0, 0 },
        { "dist",               'd', &ChatHandler::HandleDistanceCommand,           "",                                                         nullptr, 0, 0, 0 },
        { "face",               'd', &ChatHandler::HandleFaceCommand,               "",                                                         nullptr, 0, 0, 0 },
        { "dumpstate",          'd', &ChatHandler::HandleDebugDumpState,            "",                                                         nullptr, 0, 0, 0 },
        { "moveinfo",           'd', &ChatHandler::HandleDebugMoveInfo,             "",                                                         nullptr, 0, 0, 0 },
        { "setbytes",           'd', &ChatHandler::HandleSetBytesCommand,           "",                                                         nullptr, 0, 0, 0 },
        { "getbytes",           'd', &ChatHandler::HandleGetBytesCommand,           "",                                                         nullptr, 0, 0, 0 },
        { "landwalk",           'd', &ChatHandler::HandleDebugLandWalk,             "Sets landwalk move for unit",                              nullptr, 0, 0, 0 },
        { "waterwalk",          'd', &ChatHandler::HandleDebugWaterWalk,            "Sets waterwal move for unit",                              nullptr, 0, 0, 0 },
        { "hover",              'd', &ChatHandler::HandleDebugHover,                "Toggles hover move on/off for unit",                       nullptr, 0, 0, 0 },
        { "state",              'd', &ChatHandler::HandleDebugState,                "Display MovementFlags for unit",                           nullptr, 0, 0, 0 },
        { "swim",               'd', &ChatHandler::HandleDebugSwim,                 "Toggles swim move for unit",                               nullptr, 0, 0, 0 },
        { "fly",                'd', &ChatHandler::HandleDebugFly,                  "Toggles fly move for unit",                                nullptr, 0, 0, 0 },
        { "disablegravity",     'd', &ChatHandler::HandleDebugDisableGravity,       "Toggles disablegravitiy move for unit",                    nullptr, 0, 0, 0 },
        { "featherfall",        'd', &ChatHandler::HandleDebugFeatherFall,          "Toggles featherfall move for unit",                        nullptr, 0, 0, 0 },
        { "speed",              'd', &ChatHandler::HandleDebugSpeed,                "Sets move speed for unit. Usage: .debug speed <value>",    nullptr, 0, 0, 0 },
        { "castspell",          'd', &ChatHandler::HandleCastSpellCommand,          "Casts spell on target.",                                   nullptr, 0, 0, 0 },
        { "castself",           'd', &ChatHandler::HandleCastSelfCommand,           "Target casts spell <spellId> on itself.",                  nullptr, 0, 0, 0 },
        { "castspellne",        'd', &ChatHandler::HandleCastSpellNECommand,        "Casts spell by spellid on target (only plays animations)", nullptr, 0, 0, 0 },
        { "aggrorange",         'd', &ChatHandler::HandleAggroRangeCommand,         "Shows aggro Range of the selected Creature.",              nullptr, 0, 0, 0 },
        { "knockback",          'd', &ChatHandler::HandleKnockBackCommand,          "Knocks you back by <balue>.",                              nullptr, 0, 0, 0 },
        { "fade",               'd', &ChatHandler::HandleFadeCommand,               "Calls ModThreatModifyer() with <value>.",                  nullptr, 0, 0, 0 },
        { "threatMod",          'd', &ChatHandler::HandleThreatModCommand,          "Calls ModGeneratedThreatModifyer() with <value>.",         nullptr, 0, 0, 0 },
        { "calcThreat",         'd', &ChatHandler::HandleCalcThreatCommand,         "Calculates threat <dmg> <spellId>.",                       nullptr, 0, 0, 0 },
        { "threatList",         'd', &ChatHandler::HandleThreatListCommand,         "Returns all AI_Targets of the selected Creature.",         nullptr, 0, 0, 0 },
        { "gettptime",          'd', &ChatHandler::HandleGetTransporterTime,        "Grabs transporter travel time",                            nullptr, 0, 0, 0 },
        { "itempushresult",     'd', &ChatHandler::HandleSendItemPushResult,        "Sends item push result",                                   nullptr, 0, 0, 0 },
        { "setbit",             'd', &ChatHandler::HandleModifyBitCommand,          "",                                                         nullptr, 0, 0, 0 },
        { "setvalue",           'd', &ChatHandler::HandleModifyValueCommand,        "",                                                         nullptr, 0, 0, 0 },
        { "aispelltestbegin",   'd', &ChatHandler::HandleAIAgentDebugBegin,         "",                                                         nullptr, 0, 0, 0 },
        { "aispelltestcontinue",'d', &ChatHandler::HandleAIAgentDebugContinue,      "",                                                         nullptr, 0, 0, 0 },
        { "aispelltestskip",    'd', &ChatHandler::HandleAIAgentDebugSkip,          "",                                                         nullptr, 0, 0, 0 },
        { "dumpcoords",         'd', &ChatHandler::HandleDebugDumpCoordsCommmand,   "",                                                         nullptr, 0, 0, 0 },
        { "sendpacket",         'd', &ChatHandler::HandleSendpacket,                "<Opcode ID>, <data>",                                      nullptr, 0, 0, 0 },
        { "sqlquery",           'd', &ChatHandler::HandleSQLQueryCommand,           "<Sql query>",                                              nullptr, 0, 0, 0 },
        { "rangecheck",         'd', &ChatHandler::HandleRangeCheckCommand,         "Checks the range between the player and the target.",      nullptr, 0, 0, 0 },
        { "testlos",            'd', &ChatHandler::HandleCollisionTestLOS,          "Tests LoS",                                                nullptr, 0, 0, 0 },
        { "testindoor",         'd', &ChatHandler::HandleCollisionTestIndoor,       "Tests indoor",                                             nullptr, 0, 0, 0 },
        { "getheight",          'd', &ChatHandler::HandleCollisionGetHeight,        "Gets height",                                              nullptr, 0, 0, 0 },
        { "deathstate",         'd', &ChatHandler::HandleGetDeathState,             "Returns current deathstate for target",                    nullptr, 0, 0, 0 },
        { "sendfailed",         'd', &ChatHandler::HandleSendCastFailed,            "Sends failed cast result <x>",                             nullptr, 0, 0, 0 },
        { "playmovie",          'd', &ChatHandler::HandlePlayMovie,                 "Triggers a movie for selected player",                     nullptr, 0, 0, 0 },
        { "auraupdate",         'd', &ChatHandler::HandleAuraUpdateAdd,             "<SpellID> <Flags> <StackCount>",                           nullptr, 0, 0, 0 },
        { "auraremove",         'd', &ChatHandler::HandleAuraUpdateRemove,          "Remove Auras in visual slot",                              nullptr, 0, 0, 0 },
        { "spawnwar",           'd', &ChatHandler::HandleDebugSpawnWarCommand,      "Spawns desired amount of npcs to fight with eachother",    nullptr, 0, 0, 0 },
        { "updateworldstate",   'd', &ChatHandler::HandleUpdateWorldStateCommand,   "Sets the worldstate field to the specified value",         nullptr, 0, 0, 0 },
        { "initworldstates",    'd', &ChatHandler::HandleInitWorldStatesCommand,    "(Re)initializes the worldstates.",                         nullptr, 0, 0, 0 },
        { "clearworldstates",   'd', &ChatHandler::HandleClearWorldStatesCommand,   "Clears the worldstates",                                   nullptr, 0, 0, 0 },
        { "pvpcredit",          'm', &ChatHandler::HandleDebugPVPCreditCommand,     "Sends PVP credit packet, with specified rank and points",  nullptr, 0, 0, 0 },
        { "calcdist",           'd', &ChatHandler::HandleSimpleDistanceCommand,     "Displays distance between your position and x y z",        nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr, 0, 0, 0 }
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
        { "get",                'c', &ChatHandler::HandleGMTicketListCommand,           "Gets GM Ticket list.",                             nullptr, 0, 0, 0 },
        { "getId",              'c', &ChatHandler::HandleGMTicketGetByIdCommand,        "Gets GM Ticket by player name.",                   nullptr, 0, 0, 0 },
        { "delId",              'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,     "Deletes GM Ticket by player name.",                nullptr, 0, 0, 0 },
#else
        { "list",               'c', &ChatHandler::HandleGMTicketListCommand,           "Lists all active GM Tickets.",                     nullptr, 0, 0, 0 },
        { "get",                'c', &ChatHandler::HandleGMTicketGetByIdCommand,        "Gets GM Ticket with ID x.",                        nullptr, 0, 0, 0 },
        { "remove",             'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,     "Removes GM Ticket with ID x.",                     nullptr, 0, 0, 0 },
        { "deletepermanent",    'z', &ChatHandler::HandleGMTicketDeletePermanentCommand, "Deletes GM Ticket with ID x permanently.",        nullptr, 0, 0, 0 },
        { "assign",             'c', &ChatHandler::HandleGMTicketAssignToCommand,       "Assigns GM Ticket with id x to GM y.",             nullptr, 0, 0, 0 },
        { "release",            'c', &ChatHandler::HandleGMTicketReleaseCommand,        "Releases assigned GM Ticket with ID x.",           nullptr, 0, 0, 0 },
        { "comment",            'c', &ChatHandler::HandleGMTicketCommentCommand,        "Sets comment x to GM Ticket with ID y.",           nullptr, 0, 0, 0 },
#endif
        { "toggle",             'z', &ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand, "Toggles the ticket system status.",      nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
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
        { "create",             'm', &ChatHandler::HandleGuildCreateCommand,            "Creates a guild.",                                 nullptr, 0, 0, 0 },
        { "disband",            'm', &ChatHandler::HandleGuildDisbandCommand,           "Disbands the guild of your target.",               nullptr, 0, 0, 0 },
#if VERSION_STRING == Cata
        { "info",               'm', &ChatHandler::HandleGuildInfoCommand,              "Shows guild info of your target.",                 nullptr, 0, 0, 0 },
#endif
        { "join",               'm', &ChatHandler::HandleGuildJoinCommand,              "Force selected player to join a guild by name",    nullptr, 0, 0, 0 },
        { "listmembers",        'm', &ChatHandler::HandleGuildListMembersCommand,       "Lists guildmembers with ranks by guild name.",     nullptr, 0, 0, 0 },
        { "rename",             'm', &ChatHandler::HandleRenameGuildCommand,            "Renames a guild.",                                 nullptr, 0, 0, 0 },
        { "removeplayer",       'm', &ChatHandler::HandleGuildRemovePlayerCommand,      "Removes a player from a guild.",                   nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
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
        { "delete",             'o', &ChatHandler::HandleGODeleteCommand,               "Deletes selected GameObject",                      nullptr, 0, 0, 0 },
        { "enable",             'o', &ChatHandler::HandleGOEnableCommand,               "Enables the selected GO for use.",                 nullptr, 0, 0, 0 },
        { "export",             'o', &ChatHandler::HandleGOExportCommand,               "Exports the selected GO to .sql file",             nullptr, 0, 0, 0 },
        { "info",               'o', &ChatHandler::HandleGOInfoCommand,                 "Gives you information about selected GO",          nullptr, 0, 0, 0 },
        { "movehere",           'g', &ChatHandler::HandleGOMoveHereCommand,             "Moves gameobject to your position",                nullptr, 0, 0, 0 },
        { "open",               'o', &ChatHandler::HandleGOOpenCommand,                 "Toggles open/close (state) of selected GO.",       nullptr, 0, 0, 0 },
        { "rebuild",            'o', &ChatHandler::HandleGORebuildCommand,              "Rebuilds the GO.",                                 nullptr, 0, 0, 0 },
        { "rotate",             'g', &ChatHandler::HandleGORotateCommand,               "Rotates the object. <Axis> x,y, Default o.",       nullptr, 0, 0, 0 },
        { "select",             'o', &ChatHandler::HandleGOSelectCommand,               "Selects the nearest GameObject to you",            nullptr, 0, 0, 0 },
        { "selectguid",         'o', &ChatHandler::HandleGOSelectGuidCommand,           "Selects GO with <guid>",                           nullptr, 0, 0, 0 },
        { "set",                'o', nullptr,                                           "",                               GameObjectSetCommandTable, 0, 0, 0 },
        { "spawn",              'o', &ChatHandler::HandleGOSpawnCommand,                "Spawns a GameObject by ID",                        nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

    static ChatCommand BattlegroundCommandTable[] =
    {
        { "forceinitqueue",     'z', &ChatHandler::HandleBGForceInitQueueCommand,       "Forces init of all bgs with in queue.",            nullptr, 0, 0, 0 },
        { "getqueue",           'z', &ChatHandler::HandleBGGetQueueCommand,             "Gets common battleground queue information.",      nullptr, 0, 0, 0 },
        { "info",               'e', &ChatHandler::HandleBGInfoCommand,                 "Displays information about current bg.",           nullptr, 0, 0, 0 },
        { "leave",              'e', &ChatHandler::HandleBGLeaveCommand,                "Leaves the current battleground.",                 nullptr, 0, 0, 0 },
        { "menu",               'e', &ChatHandler::HandleBGMenuCommand,                 "Shows BG Menu for selected player by type <x>",    nullptr, 0, 0, 0 },
        { "pause",              'e', &ChatHandler::HandleBGPauseCommand,                "Pauses current battleground match.",               nullptr, 0, 0, 0 },
        { "playsound",          'e', &ChatHandler::HandleBGPlaySoundCommand,            "Plays sound to all players in bg <sound_id>",      nullptr, 0, 0, 0 },
        { "sendstatus",         'e', &ChatHandler::HandleBGSendStatusCommand,           "Sends status of bg by type <x>",                   nullptr, 0, 0, 0 },
        { "setscore",           'e', &ChatHandler::HandleBGSetScoreCommand,             "Sets bg score <Teamid> <Score>.",                  nullptr, 0, 0, 0 },
        { "setworldstate",      'e', &ChatHandler::HandleBGSetWorldStateCommand,        "Sets singe worldsate value.",                      nullptr, 0, 0, 0 },
        { "setworldstates",     'e', &ChatHandler::HandleBGSetWorldStatesCommand,       "Sets multipe worldstate values for start/end id",  nullptr, 0, 0, 0 },
        { "start",              'e', &ChatHandler::HandleBGStartCommand,                "Starts current battleground match.",               nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
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
        { "cast",               'n', &ChatHandler::HandleNpcCastCommand,                "Makes NPC cast <spellid>.",                        nullptr, 0, 0, 0 },
        { "come",               'n', &ChatHandler::HandleNpcComeCommand,                "Makes NPC move to your position",                  nullptr, 0, 0, 0 },
        { "delete",             'n', &ChatHandler::HandleNpcDeleteCommand,              "Deletes mob from world optional from DB",          nullptr, 0, 0, 0 },
        { "info",               'n', &ChatHandler::HandleNpcInfoCommand,                "Displays NPC information",                         nullptr, 0, 0, 0 },
        { "listAgent",          'n', &ChatHandler::HandleNpcListAIAgentCommand,         "List AIAgents of selected target.",                nullptr, 0, 0, 0 },
        { "listloot",           'm', &ChatHandler::HandleNpcListLootCommand,            "Displays possible loot for the selected NPC.",     nullptr, 0, 0, 0 },
        { "follow",             'm', &ChatHandler::HandleNpcFollowCommand,              "Sets NPC to follow you",                           nullptr, 0, 0, 0 },
        { "stopfollow",         'm', &ChatHandler::HandleNpcStopFollowCommand,          "Sets NPC to not follow anything",                  nullptr, 0, 0, 0 },
        { "possess",            'n', &ChatHandler::HandlePossessCommand,                "Possess targeted NPC (mind control)",              nullptr, 0, 0, 0 },
        { "unpossess",          'n', &ChatHandler::HandleUnPossessCommand,              "Unpossess any currently possessed npc.",           nullptr, 0, 0, 0 },
        { "return",             'n', &ChatHandler::HandleNpcReturnCommand,              "Returns NPC to spawnpoint.",                       nullptr, 0, 0, 0 },
        { "respawn",            'n', &ChatHandler::HandleNpcRespawnCommand,             "Respawns a dead NPC from its corpse.",             nullptr, 0, 0, 0 },
        { "say",                'n', &ChatHandler::HandleNpcSayCommand,                 "Makes selected NPC say <text>.",                   nullptr, 0, 0, 0 },
        { "select",             'n', &ChatHandler::HandleNpcSelectCommand,              "Selects closest NPC",                               nullptr, 0, 0, 0 },
        { "set",                '0', nullptr,                                           "",                                      NPCSetCommandTable, 0, 0, 0 },
        { "spawn",              'n', &ChatHandler::HandleNpcSpawnCommand,               "Spawns NPC of entry <id>",                         nullptr, 0, 0, 0 },
        { "showtimers",         'm', &ChatHandler::HandleNpcShowTimersCommand,          "Shows timers for selected creature",               nullptr, 0, 0, 0 },
        { "vendoradditem",      'n', &ChatHandler::HandleNpcVendorAddItemCommand,       "Adds item to vendor",                              nullptr, 0, 0, 0 },
        { "vendorremoveitem",   'n', &ChatHandler::HandleNpcVendorRemoveItemCommand,    "Removes item from vendor.",                        nullptr, 0, 0, 0 },
        { "yell",               'n', &ChatHandler::HandleNpcYellCommand,                "Makes selected NPC yell <text>.",                  nullptr, 0, 0, 0 },
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
        { "getid",      '1', &ChatHandler::HandleAccountGetAccountID,   "Get Account ID for account name X",                                nullptr, 0, 0, 0 },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(accountCommandTable, _accountCommandTable);

    static ChatCommand petCommandTable[] =
    {
        { "create",     'm', &ChatHandler::HandlePetCreateCommand,      "Creates a pet with <entry>.",                                      nullptr, 0, 0, 0 },
        { "dismiss",    'm', &ChatHandler::HandlePetDismissCommand,     "Dismisses a pet by for selected player or selected pet.",          nullptr, 0, 0, 0 },
        { "rename",     'm', &ChatHandler::HandlePetRenameCommand,      "Renames a pet to <name>.",                                         nullptr, 0, 0, 0 },
        { "addspell",   'm', &ChatHandler::HandlePetAddSpellCommand,    "Teaches pet <spell>.",                                             nullptr, 0, 0, 0 },
        { "removespell",'m', &ChatHandler::HandlePetRemoveSpellCommand, "Removes pet spell <spell>.",                                       nullptr, 0, 0, 0 },
        { "setlevel",   'm', &ChatHandler::HandlePetSetLevelCommand,    "Sets pet level to <level>.",                                       nullptr, 0, 0, 0 },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(petCommandTable, _petCommandTable);

    //teleport
    static ChatCommand recallCommandTable[] =
    {
        { "list",       'q', &ChatHandler::HandleRecallListCommand,     "List recall locations",                                            nullptr, 0, 0, 0 },
        { "add",        'q', &ChatHandler::HandleRecallAddCommand,      "Add a recall location",                                            nullptr, 0, 0, 0 },
        { "del",        'q', &ChatHandler::HandleRecallDelCommand,      "Remove a recall location",                                         nullptr, 0, 0, 0 },
        { "port",       'q', &ChatHandler::HandleRecallGoCommand,       "Ports you to recalled location",                                   nullptr, 0, 0, 0 },
        { "portplayer", 'm', &ChatHandler::HandleRecallPortPlayerCommand,"Ports specified player to a recalled location",                   nullptr, 0, 0, 0 },
        { "portus",     'm', &ChatHandler::HandleRecallPortUsCommand,   "Ports you and the selected player to recalled location",           nullptr, 0, 0, 0 },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(recallCommandTable, _recallCommandTable);

    static ChatCommand questCommandTable[] =
    {
        { "addboth",    '2', &ChatHandler::HandleQuestAddBothCommand,   "Add quest <id> to the targeted NPC as start & finish",             nullptr, 0, 0, 0 },
        { "addfinish",  '2', &ChatHandler::HandleQuestAddFinishCommand, "Add quest <id> to the targeted NPC as finisher",                   nullptr, 0, 0, 0 },
        { "addstart",   '2', &ChatHandler::HandleQuestAddStartCommand,  "Add quest <id> to the targeted NPC as starter",                    nullptr, 0, 0, 0 },
        { "delboth",    '2', &ChatHandler::HandleQuestDelBothCommand,   "Delete quest <id> from the targeted NPC as start & finish",        nullptr, 0, 0, 0 },
        { "delfinish",  '2', &ChatHandler::HandleQuestDelFinishCommand, "Delete quest <id> from the targeted NPC as finisher",              nullptr, 0, 0, 0 },
        { "delstart",   '2', &ChatHandler::HandleQuestDelStartCommand,  "Delete quest <id> from the targeted NPC as starter",               nullptr, 0, 0, 0 },
        { "complete",   '2', &ChatHandler::HandleQuestFinishCommand,    "Complete/Finish quest <id>",                                       nullptr, 0, 0, 0 },
        { "fail",       '2', &ChatHandler::HandleQuestFailCommand,      "Fail quest <id>",                                                  nullptr, 0, 0, 0 },
        { "finisher",   '2', &ChatHandler::HandleQuestFinisherCommand,  "Lookup quest finisher for quest <id>",                             nullptr, 0, 0, 0 },
        { "item",       '2', &ChatHandler::HandleQuestItemCommand,      "Lookup itemid necessary for quest <id>",                           nullptr, 0, 0, 0 },
        { "list",       '2', &ChatHandler::HandleQuestListCommand,      "Lists the quests for the npc <id>",                                nullptr, 0, 0, 0 },
        { "load",       '2', &ChatHandler::HandleQuestLoadCommand,      "Loads quests from database",                                       nullptr, 0, 0, 0 },
        { "giver",      '2', &ChatHandler::HandleQuestGiverCommand,     "Lookup quest giver for quest <id>",                                nullptr, 0, 0, 0 },
        { "remove",     '2', &ChatHandler::HandleQuestRemoveCommand,    "Removes the quest <id> from the targeted player",                  nullptr, 0, 0, 0 },
        { "reward",     '2', &ChatHandler::HandleQuestRewardCommand,    "Shows reward for quest <id>",                                      nullptr, 0, 0, 0 },
        { "status",     '2', &ChatHandler::HandleQuestStatusCommand,    "Lists the status of quest <id>",                                   nullptr, 0, 0, 0 },
        { "start",      '2', &ChatHandler::HandleQuestStartCommand,     "Starts quest <id>",                                                nullptr, 0, 0, 0 },
        { "startspawn", '2', &ChatHandler::HandleQuestStarterSpawnCommand, "Port to spawn location for quest <id> (starter)",               nullptr, 0, 0, 0 },
        { "finishspawn",'2', &ChatHandler::HandleQuestFinisherSpawnCommand, "Port to spawn location for quest <id> (finisher)",             nullptr, 0, 0, 0 },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr, 0, 0, 0 }
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
        { "pet_level_abilities",'z', &ChatHandler::HandleReloadPetLevelAbilitiesCommand,    "Reload pet_level_abilities table",             nullptr, 0, 0, 0 },
        { "player_xp_for_level",'z', &ChatHandler::HandleReloadPlayerXpForLevelCommand,     "Reload player_xp_for_level table",             nullptr, 0, 0, 0 },
        { "points_of_interest", 'z', &ChatHandler::HandleReloadPointsOfInterestCommand,     "Reload points_of_interest table",              nullptr, 0, 0, 0 },
        { "quests",             'z', &ChatHandler::HandleReloadQuestsCommand,               "Reload quests table",                          nullptr, 0, 0, 0 },
        { "spell_teleport_coords",'z', &ChatHandler::HandleReloadTeleportCoordsCommand,     "Reload teleport_coords table",                 nullptr, 0, 0, 0 },
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
        { "active",             't', &ChatHandler::HandleGMActiveCommand,               "Activate/Deactivate <GM> tag",                     nullptr, 0, 0, 0 },
        { "allowwhispers",      'c', &ChatHandler::HandleGMAllowWhispersCommand,        "Allows whispers from player <s>.",                 nullptr, 0, 0, 0 },
        { "announce",           'u', &ChatHandler::HandleGMAnnounceCommand,             "Sends announce to all online GMs",                 nullptr, 0, 0, 0 },
        { "blockwhispers",      'c', &ChatHandler::HandleGMBlockWhispersCommand,        "Blocks whispers from player <s>.",                 nullptr, 0, 0, 0 },
        { "devtag",             '1', &ChatHandler::HandleGMDevTagCommand,               "Activate/Deactivate <DEV> tag",                    nullptr, 0, 0, 0 },
        { "list",               '0', &ChatHandler::HandleGMListCommand,                 "Shows active GM's",                                nullptr, 0, 0, 0 },
        { "logcomment",         '1', &ChatHandler::HandleGMLogCommentCommand,           "Adds a comment to the GM log.",                    nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(gmCommandTable, _gmCommandTable);

    static ChatCommand characterAddCommandTable[] =
    {
        { "copper",             'm', &ChatHandler::HandleCharAddCopperCommand,          "Adds x copper to character.",                      nullptr, 0, 0, 0 },
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
        { "add",                'm', nullptr,                                           "",                                characterAddCommandTable, 0, 0, 0 },
        { "set",                'm', nullptr,                                           "",                                characterSetCommandTable, 0, 0, 0 },
        { "list",               'm', nullptr,                                           "",                               characterListCommandTable, 0, 0, 0 },
        { "clearcooldowns",     'm', &ChatHandler::HandleCharClearCooldownsCommand,     "Clears all cooldowns for your class.",             nullptr, 0, 0, 0 },
        { "demorph",            'm', &ChatHandler::HandleCharDeMorphCommand,            "Demorphs from morphed model.",                     nullptr, 0, 0, 0 },
        { "levelup",            'm', &ChatHandler::HandleCharLevelUpCommand,            "Player target will be levelup x levels",           nullptr, 0, 0, 0 },
        { "removeauras",        'm', &ChatHandler::HandleCharRemoveAurasCommand,        "Removes all auras from target",                    nullptr, 0, 0, 0 },
        { "removesickness",     'm', &ChatHandler::HandleCharRemoveSickessCommand,      "Removes ressurrection sickness from target",       nullptr, 0, 0, 0 },
        { "learn",              'm', &ChatHandler::HandleCharLearnCommand,              "Learns spell <x> or all available spells by race", nullptr, 0, 0, 0 },
        { "unlearn",            'm', &ChatHandler::HandleCharUnlearnCommand,            "Unlearns spell",                                   nullptr, 0, 0, 0 },
        { "learnskill",         'm', &ChatHandler::HandleCharLearnSkillCommand,         "Learns skill id skillid opt: min max.",            nullptr, 0, 0, 0 },
        { "advanceskill",       'm', &ChatHandler::HandleCharAdvanceSkillCommand,       "Advances skill line x y times.",                   nullptr, 0, 0, 0 },
        { "removeskill",        'm', &ChatHandler::HandleCharRemoveSkillCommand,        "Removes skill.",                                   nullptr, 0, 0, 0 },
        { "increaseweaponskill",'m', &ChatHandler::HandleCharIncreaseWeaponSkill,       "Increase equipped weapon skill x times.",          nullptr, 0, 0, 0 },
        { "resetreputation",    'n', &ChatHandler::HandleCharResetReputationCommand,    "Resets reputation to start levels.",               nullptr, 0, 0, 0 },
        { "resetspells",        'n', &ChatHandler::HandleCharResetSpellsCommand,        "Resets all spells of selected player.",            nullptr, 0, 0, 0 },
        { "resettalents",       'n', &ChatHandler::HandleCharResetTalentsCommand,       "Resets all talents of selected player.",           nullptr, 0, 0, 0 },
        { "resetskills",        'n', &ChatHandler::HandleCharResetSkillsCommand,        "Resets all skills.",                               nullptr, 0, 0, 0 },
        { "removeitem",         'm', &ChatHandler::HandleCharRemoveItemCommand,         "Removes item x count y.",                          nullptr, 0, 0, 0 },
        { "advanceallskills",   'm', &ChatHandler::HandleAdvanceAllSkillsCommand,       "Advances all skills <x> points.",                  nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(characterCommandTable, _characterCommandTable);

    static ChatCommand lookupCommandTable[] =
    {
        { "achievement",    'l', &ChatHandler::HandleLookupAchievementCommand,  "Looks up achievement string x.",                                   nullptr, 0, 0, 0 },
        { "creature",       'l', &ChatHandler::HandleLookupCreatureCommand,     "Looks up creature string x.",                                      nullptr, 0, 0, 0 },
        { "faction",        'l', &ChatHandler::HandleLookupFactionCommand,      "Looks up faction string x.",                                       nullptr, 0, 0, 0 },
        { "item",           'l', &ChatHandler::HandleLookupItemCommand,         "Looks up item string x.",                                          nullptr, 0, 0, 0 },
        { "object",         'l', &ChatHandler::HandleLookupObjectCommand,       "Looks up gameobject string x.",                                    nullptr, 0, 0 ,0 },
        { "quest",          'l', &ChatHandler::HandleLookupQuestCommand,        "Looks up quest string x.",                                         nullptr, 0, 0, 0 },
        { "spell",          'l', &ChatHandler::HandleLookupSpellCommand,        "Looks up spell string x.",                                         nullptr, 0, 0, 0 },
        { "skill",          'l', &ChatHandler::HandleLookupSkillCommand,        "Looks up skill string x.",                                         nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(lookupCommandTable, _lookupCommandTable);

    static ChatCommand adminCommandTable[] =
    {
        { "castall",        'z', &ChatHandler::HandleAdminCastAllCommand,       "Makes all players online cast spell <x>.",                         nullptr, 0, 0, 0 },
        { "dispelall",      'z', &ChatHandler::HandleAdminDispelAllCommand,     "Dispels all negative (or positive w/ 1) auras on all players.",    nullptr, 0, 0, 0 },
        { "masssummon",     'z', &ChatHandler::HandleAdminMassSummonCommand,    "Summons all online players to you, use a/h for alliance/horde.",   nullptr, 0, 0, 0 },
        { "playall",        'z', &ChatHandler::HandleAdminPlayGlobalSoundCommand, "Plays a sound to everyone on the realm.",                        nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(adminCommandTable, _adminCommandTable);

    static ChatCommand kickCommandTable[] =
    {
        { "player",         'f', &ChatHandler::HandleKickByNameCommand,         "Disconnects the player with name <s>.",                            nullptr, 0, 0, 0 },
        { "account",        'f', &ChatHandler::HandleKKickBySessionCommand,     "Disconnects the session with account name <s>.",                   nullptr, 0, 0, 0 },
        { "ip",             'f', &ChatHandler::HandleKickByIPCommand,           "Disconnects the session with the ip <s>.",                         nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(kickCommandTable, _kickCommandTable);

    static ChatCommand banCommandTable[] =
    {
        { "ip",             'm', &ChatHandler::HandleIPBanCommand,              "Bans IP by <address> [duration]",                                  nullptr, 0, 0, 0 },
        { "character",      'b', &ChatHandler::HandleBanCharacterCommand,       "Bans character by <charname> [duration] [reason]",                 nullptr, 0, 0, 0 },
        { "all",            'a', &ChatHandler::HandleBanAllCommand,             "Bans all by <charname> [duration] [reason]",                       nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(banCommandTable, _banCommandTable);

    static ChatCommand unbanCommandTable[] =
    {
        { "ip",             'm', &ChatHandler::HandleIPUnBanCommand,            "Deletes an address from the IP ban table: <address>",              nullptr, 0, 0, 0 },
        { "character",      'b', &ChatHandler::HandleUnBanCharacterCommand,     "Unbans character x",                                               nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(unbanCommandTable, _unbanCommandTable);

    static ChatCommand instanceCommandTable[] =
    {
        { "create",         'z', &ChatHandler::HandleCreateInstanceCommand,     "Creates instance by mapid x y z",                                  nullptr, 0, 0, 0 },
        { "countcreature",  'z', &ChatHandler::HandleCountCreaturesCommand,     "Returns number of creatures with entry x",                         nullptr, 0, 0, 0 },
        { "exit",           'm', &ChatHandler::HandleExitInstanceCommand,       "Exits current instance, return to entry point.",                   nullptr, 0, 0, 0 },
        { "info",           'm', &ChatHandler::HandleGetInstanceInfoCommand,    "Gets info about instance with ID x (default current instance).",   nullptr, 0, 0, 0 },
        { "reset",          'z', &ChatHandler::HandleResetInstanceCommand,      "Removes instance ID x from target player.",                        nullptr, 0, 0, 0 },
        { "resetall",       'm', &ChatHandler::HandleResetAllInstancesCommand,  "Removes all instance IDs from target player.",                     nullptr, 0, 0, 0 },
        { "shutdown",       'z', &ChatHandler::HandleShutdownInstanceCommand,   "Shutdown instance with ID x (default is current instance).",       nullptr, 0, 0, 0 },
        { "showtimers",     'm', &ChatHandler::HandleShowTimersCommand,         "Show timers for current instance.",                                nullptr, 0, 0, 0 },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(instanceCommandTable, _instanceCommandTable);

    static ChatCommand arenaCommandTable[] =
    {
        { "createteam",      'e', &ChatHandler::HandleArenaCreateTeam,          "Creates arena team with <type> <name>",                            nullptr, 0, 0, 0 },
        { "setteamleader",   'e', &ChatHandler::HandleArenaSetTeamLeader,       "Sets the arena team leader for <type>",                            nullptr, 0, 0, 0 },
        { "resetallratings", 'z', &ChatHandler::HandleArenaTeamResetAllRatings, "Resets all arena teams to their default rating",                   nullptr, 0, 0, 0 },
        { nullptr,           '0', nullptr,                                      "",                                                                 nullptr, 0, 0, 0 }
    };
    dupe_command_table(arenaCommandTable, _arenaCommandTable);

    static ChatCommand achievementCommandTable[] =
    {
#if VERSION_STRING > TBC
        { "complete",       'm', &ChatHandler::HandleAchievementCompleteCommand,    "Completes the specified achievement.",                         nullptr, 0, 0, 0 },
        { "criteria",       'm', &ChatHandler::HandleAchievementCriteriaCommand,    "Completes the specified achievement criteria.",                nullptr, 0, 0, 0 },
        { "reset",          'm', &ChatHandler::HandleAchievementResetCommand,       "Resets achievement data from the target.",                     nullptr, 0, 0, 0 },
#endif
        { nullptr,          '0', nullptr,                                           "",                                                             nullptr, 0, 0, 0 }
    };
    dupe_command_table(achievementCommandTable, _achievementCommandTable);

    static ChatCommand vehicleCommandTable[] = {
        { "ejectpassenger",     'm', &ChatHandler::HandleVehicleEjectPassengerCommand,      "Ejects the passenger from the specified seat",         nullptr, 0, 0, 0 },
        { "ejectallpassengers", 'm', &ChatHandler::HandleVehicleEjectAllPassengersCommand,  "Ejects all passengers from the vehicle",               nullptr, 0, 0, 0 },
        { "installaccessories", 'm', &ChatHandler::HandleVehicleInstallAccessoriesCommand,  "Installs the accessories for the selected vehicle",    nullptr, 0, 0, 0 },
        { "removeaccessories",  'm', &ChatHandler::HandleVehicleRemoveAccessoriesCommand,   "Removes the accessories of the selected vehicle",      nullptr, 0, 0, 0 },
        { "addpassenger",       'm', &ChatHandler::HandleVehicleAddPassengerCommand,        "Adds a new NPC passenger to the vehicle",              nullptr, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                               "",                                                     nullptr, 0, 0, 0 }
    };

    dupe_command_table(vehicleCommandTable, _vehicleCommandTable);

    static ChatCommand commandTable[] =
    {
        { "commands",           '0', &ChatHandler::HandleCommandsCommand,               "Shows commands",                                           nullptr, 0, 0, 0 },
        { "help",               '0', &ChatHandler::HandleHelpCommand,                   "Shows help for command",                                   nullptr, 0, 0, 0 },
        { "autosavechanges",    '1', &ChatHandler::HandleAutoSaveChangesCommand,        "Toggles auto save for db table related commands.",         nullptr, 0, 0, 0 },
        { "event",              '0', nullptr,                                           "",                                               eventCommandTable, 0, 0, 0 },
        { "announce",           'u', &ChatHandler::HandleAnnounceCommand,               "Sends a normal chat message to all players.",              nullptr, 0, 0, 0 },
        { "wannounce",          'u', &ChatHandler::HandleWAnnounceCommand,              "Sends a widescreen announcement to all players.",          nullptr, 0, 0, 0 },
        { "appear",             'v', &ChatHandler::HandleAppearCommand,                 "Teleports to x's position.",                               nullptr, 0, 0, 0 },
        { "blockappear",        'v', &ChatHandler::HandleBlockAppearCommand,            "Blocks appearance to your position.",                      nullptr, 0, 0, 0 },
        { "summon",             'v', &ChatHandler::HandleSummonCommand,                 "Summons x to your position.",                              nullptr, 0, 0, 0 },
        { "blocksummon",        'v', &ChatHandler::HandleBlockSummonCommand,            "Blocks summons to others position.",                       nullptr, 0, 0, 0 },
        { "kill",               'r', &ChatHandler::HandleKillCommand,                   "Kills selected unit or player by name",                    nullptr, 0, 0, 0 },
        { "revive",             'r', &ChatHandler::HandleReviveCommand,                 "Revives you or a selected target or player by name",       nullptr, 0, 0, 0 },
        { "mount",              'm', &ChatHandler::HandleMountCommand,                  "Mounts targeted unit with modelid x.",                     nullptr, 0, 0, 0 },
        { "dismount",           'h', &ChatHandler::HandleDismountCommand,               "Dismounts targeted unit.",                                 nullptr, 0, 0, 0 },
        { "gps",                '0', &ChatHandler::HandleGPSCommand,                    "Shows position of targeted unit",                          nullptr, 0, 0, 0 },
        { "worldport",          'v', &ChatHandler::HandleWorldPortCommand,              "Teleports you to a location with mapid x y z",             nullptr, 0, 0, 0 },
        { "invincible",         'j', &ChatHandler::HandleInvincibleCommand,             "Toggles invincibility on/off",                             nullptr, 0, 0, 0 },
        { "invisible",          'i', &ChatHandler::HandleInvisibleCommand,              "Toggles invisibility and invincibility on/off",            nullptr, 0, 0, 0 },
        { "playerinfo",         'm', &ChatHandler::HandlePlayerInfo,                    "Displays info for selected character or <charname>",       nullptr, 0, 0, 0 },
        { "modify",             '0', nullptr,                                           "",                                              modifyCommandTable, 0, 0, 0 },
        { "waypoint",           '0', nullptr,                                           "",                                            waypointCommandTable, 0, 0, 0 },
        { "debug",              '0', nullptr,                                           "",                                               debugCommandTable, 0, 0, 0 },
        { "gm",                 '0', nullptr,                                           "",                                                  gmCommandTable, 0, 0, 0 },
        { "gmTicket",           '0', nullptr,                                           "",                                            GMTicketCommandTable, 0, 0, 0 },
        { "ticket",             '0', nullptr,                                           "",                                              TicketCommandTable, 0, 0, 0 },
        { "gobject",            '0', nullptr,                                           "",                                          GameObjectCommandTable, 0, 0, 0 },
        { "battleground",       '0', nullptr,                                           "",                                        BattlegroundCommandTable, 0, 0, 0 },
        { "npc",                '0', nullptr,                                           "",                                                 NPCCommandTable, 0, 0, 0 },
        { "cheat",              '0', nullptr,                                           "",                                               CheatCommandTable, 0, 0, 0 },
        { "account",            '0', nullptr,                                           "",                                             accountCommandTable, 0, 0, 0 },
        { "quest",              '0', nullptr,                                           "",                                               questCommandTable, 0, 0, 0 },
        { "pet",                '0', nullptr,                                           "",                                                 petCommandTable, 0, 0, 0 },
        { "recall",             '0', nullptr,                                           "",                                              recallCommandTable, 0, 0, 0 },
        { "guild",              '0', nullptr,                                           "",                                               GuildCommandTable, 0, 0, 0 },
        { "server",             '0', nullptr,                                           "",                                              serverCommandTable, 0, 0, 0 },
        { "character",          '0', nullptr,                                           "",                                           characterCommandTable, 0, 0, 0 },
        { "lookup",             '0', nullptr,                                           "",                                              lookupCommandTable, 0, 0, 0 },
        { "admin",              '0', nullptr,                                           "",                                               adminCommandTable, 0, 0, 0 },
        { "kick",               '0', nullptr,                                           "",                                                kickCommandTable, 0, 0, 0 },
        { "ban",                '0', nullptr,                                           "",                                                 banCommandTable, 0, 0, 0 },
        { "unban",              '0', nullptr,                                           "",                                               unbanCommandTable, 0, 0, 0 },
        { "instance",           '0', nullptr,                                           "",                                            instanceCommandTable, 0, 0, 0 },
        { "arena",              '0', nullptr,                                           "",                                               arenaCommandTable, 0, 0, 0 },
        { "unroot",             'b', &ChatHandler::HandleUnrootCommand,                 "Unroots selected target.",                                 nullptr, 0, 0, 0 },
        { "root",               'b', &ChatHandler::HandleRootCommand,                   "Roots selected target.",                                   nullptr, 0, 0, 0 },
        { "gocreature",         'v', &ChatHandler::HandleGoCreatureSpawnCommand,        "Teleports you to the creature with <spwn_id>.",            nullptr, 0, 0, 0 },
        { "gogameobject",       'v', &ChatHandler::HandleGoGameObjectSpawnCommand,      "Teleports you to the gameobject with <spawn_id>.",         nullptr, 0, 0, 0 },
        { "gostartlocation",    'm', &ChatHandler::HandleGoStartLocationCommand,        "Teleports you to a starting location",                     nullptr, 0, 0, 0 },
        { "gotrig",             'v', &ChatHandler::HandleGoTriggerCommand,              "Teleports you to the areatrigger with <id>.",              nullptr, 0, 0, 0 },
        { "achieve",            '0', nullptr,                                           "",                                         achievementCommandTable, 0, 0, 0 },
        { "vehicle",            'm', nullptr,                                           "",                                             vehicleCommandTable, 0, 0, 0 },
        { "transport",          'm', nullptr,                                           "",                                           transportCommandTable, 0, 0, 0 },
        { nullptr,              '0', nullptr,                                           "",                                                         nullptr, 0, 0, 0 }
    };
    dupe_command_table(commandTable, _commandTable);

    /* set the correct pointers */
    ChatCommand* p = &_commandTable[0];
    while (p->Name != 0)
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

