/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
        { "hp",                 'm', &ChatHandler::HandleModifyHp,                  "Mods health points (HP) of selected target",               nullptr },
        { "mana",               'm', &ChatHandler::HandleModifyMana,                "Mods mana points (MP) of selected target.",                nullptr },
        { "rage",               'm', &ChatHandler::HandleModifyRage,                "Mods rage points of selected target.",                     nullptr },
        { "energy",             'm', &ChatHandler::HandleModifyEnergy,              "Mods energy points of selected target.",                   nullptr },
#if VERSION_STRING == WotLK
        { "runicpower",         'm', &ChatHandler::HandleModifyRunicpower,          "Mods runic power points of selected target.",              nullptr },
#endif
        { "strength",           'm', &ChatHandler::HandleModifyStrength,            "Mods strength value of the selected target.",              nullptr },
        { "agility",            'm', &ChatHandler::HandleModifyAgility,             "Mods agility value of the selected target.",               nullptr },
        { "intelligence",       'm', &ChatHandler::HandleModifyIntelligence,        "Mods intelligence value of the selected target.",          nullptr },
        { "spirit",             'm', &ChatHandler::HandleModifySpirit,              "Mods spirit value of the selected target.",                nullptr },
        { "armor",              'm', &ChatHandler::HandleModifyArmor,               "Mods armor of selected target.",                           nullptr },
        { "holy",               'm', &ChatHandler::HandleModifyHoly,                "Mods holy resistance of selected target.",                 nullptr },
        { "fire",               'm', &ChatHandler::HandleModifyFire,                "Mods fire resistance of selected target.",                 nullptr },
        { "nature",             'm', &ChatHandler::HandleModifyNature,              "Mods nature resistance of selected target.",               nullptr },
        { "frost",              'm', &ChatHandler::HandleModifyFrost,               "Mods frost resistance of selected target.",                nullptr },
        { "shadow",             'm', &ChatHandler::HandleModifyShadow,              "Mods shadow resistance of selected target.",               nullptr },
        { "arcane",             'm', &ChatHandler::HandleModifyArcane,              "Mods arcane resistance of selected target.",               nullptr },
        { "damage",             'm', &ChatHandler::HandleModifyDamage,              "Mods damage done by the selected target.",                 nullptr },
        { "ap",                 'm', &ChatHandler::HandleModifyAp,                  "Mods attack power of the selected target.",                nullptr },
        { "rangeap",            'm', &ChatHandler::HandleModifyRangeap,             "Mods range attack power of the selected target.",          nullptr },
        { "scale",              'm', &ChatHandler::HandleModifyScale,               "Mods scale of the selected target.",                       nullptr },
        { "nativedisplayid",    'm', &ChatHandler::HandleModifyNativedisplayid,     "Mods native display identifier of the target.",            nullptr },
        { "displayid",          'm', &ChatHandler::HandleModifyDisplayid,           "Mods display identifier (DisplayID) of the target.",       nullptr },
        { "flags",              'm', &ChatHandler::HandleModifyFlags,               "Mods flags of the selected target.",                       nullptr },
        { "faction",            'm', &ChatHandler::HandleModifyFaction,             "Mods faction template of the selected target.",            nullptr },
        { "dynamicflags",       'm', &ChatHandler::HandleModifyDynamicflags,        "Mods dynamic flags of the selected target.",               nullptr },
        { "happiness",          'm', &ChatHandler::HandleModifyHappiness,           "Mods happiness value of the selected target.",             nullptr },
        { "boundingradius",     'm', &ChatHandler::HandleModifyBoundingradius,      "Mods bounding radius of the selected target.",             nullptr },
        { "combatreach",        'm', &ChatHandler::HandleModifyCombatreach,         "Mods combat reach of the selected target.",                nullptr },
        { "emotestate",         'm', &ChatHandler::HandleModifyEmotestate,          "Mods Unit emote state of the selected target.",            nullptr },
        { "bytes0",             'm', &ChatHandler::HandleModifyBytes0,              "Mods bytes0 entry of selected target.",                    nullptr },
        { "bytes1",             'm', &ChatHandler::HandleModifyBytes1,              "Mods bytes1 entry of selected target.",                    nullptr },
        { "bytes2",             'm', &ChatHandler::HandleModifyBytes2,              "Mods bytes2 entry of selected target.",                    nullptr },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr }
    };
    dupe_command_table(modifyCommandTable, _modifyCommandTable);

    static ChatCommand eventCommandTable[] =
    {
        { "list",               'm', &ChatHandler::HandleEventListEvents,           "Shows list of currently active events",                    nullptr },
        { "start",              'm', &ChatHandler::HandleEventStartEvent,           "Force start an event",                                     nullptr },
        { "stop",               'm', &ChatHandler::HandleEventStopEvent,            "Force stop an event",                                      nullptr },
        { "reset",              'm', &ChatHandler::HandleEventResetEvent,           "Resets force flags for an event",                          nullptr },
        { "reload",             'a', &ChatHandler::HandleEventReloadAllEvents,      "Reloads all events from the database",                     nullptr },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr }
    };
    dupe_command_table(eventCommandTable, _eventCommandTable);

    static ChatCommand transportCommandTable[] =
    {
        { "info",               'm', &ChatHandler::HandleGetTransporterInfo,        "Displays the current transport info",                      nullptr },
        { "spawn",              'm', &ChatHandler::HandleSpawnInstanceTransport,    "Spawns transport with entry/period in current instance",   nullptr },
        { "despawn",            'm', &ChatHandler::HandleDespawnInstanceTransport,  "Despawns the transport you are currently on",              nullptr },
        { "start",              'm', &ChatHandler::HandleStartTransport,            "Force starts the current transport",                       nullptr },
        { "stop",               'm', &ChatHandler::HandleStopTransport,             "Force stops the current transport",                        nullptr },
        { "modperiod",          'm', &ChatHandler::HandleModPeriodCommand,          "Changes the period of the current transport",              nullptr },
        { "getperiod",          'm', &ChatHandler::HandleGetTransporterTime,        "Displays the current transport period in ms",              nullptr },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr }
    };
    dupe_command_table(transportCommandTable, _transportCommandTable);

    static ChatCommand debugCommandTable[] =
    {
        { "sendcreaturemove", 'd', &ChatHandler::HandleDebugSendCreatureMove, "Requests the target creature moves to you using movement manager.", nullptr },
        { "dopctdamage",        'z', &ChatHandler::HandleDoPercentDamageCommand,    "Do percent damage to creature target",                     nullptr },
        { "setscriptphase",     'z', &ChatHandler::HandleSetScriptPhaseCommand,     "ScriptPhase test",                                         nullptr },
        { "aicharge",           'z', &ChatHandler::HandleAiChargeCommand,           "AiCharge test",                                            nullptr },
        { "aiknockback",        'z', &ChatHandler::HandleAiKnockbackCommand,        "AiKnockBack test",                                         nullptr },
        { "aijump",             'z', &ChatHandler::HandleAiJumpCommand,             "AiJump test",                                              nullptr },
        { "aifalling",          'z', &ChatHandler::HandleAiFallingCommand,          "AiFalling test",                                           nullptr },
        { "movetospawn",        'z', &ChatHandler::HandleMoveToSpawnCommand,        "Move target to spwn",                                      nullptr },
        { "position",           'z', &ChatHandler::HandlePositionCommand,           "Show position",                                            nullptr },
        { "setorientation",     'z', &ChatHandler::HandleSetOrientationCommand,     "Sets orientation on npc",                                  nullptr },
        { "dumpmovement",       'd', &ChatHandler::HandleDebugDumpMovementCommand,  "Dumps the player's movement information to chat",          nullptr },
        { "infront",            'd', &ChatHandler::HandleDebugInFrontCommand,       "",                                                         nullptr },
        { "showreact",          'd', &ChatHandler::HandleShowReactionCommand,       "",                                                         nullptr },
        { "aimove",             'd', &ChatHandler::HandleAIMoveCommand,             "",                                                         nullptr },
        { "dist",               'd', &ChatHandler::HandleDistanceCommand,           "",                                                         nullptr },
        { "face",               'd', &ChatHandler::HandleFaceCommand,               "",                                                         nullptr },
        { "dumpstate",          'd', &ChatHandler::HandleDebugDumpState,            "",                                                         nullptr },
        { "moveinfo",           'd', &ChatHandler::HandleDebugMoveInfo,             "",                                                         nullptr },
        { "setbytes",           'd', &ChatHandler::HandleSetBytesCommand,           "",                                                         nullptr },
        { "getbytes",           'd', &ChatHandler::HandleGetBytesCommand,           "",                                                         nullptr },
        { "landwalk",           'd', &ChatHandler::HandleDebugLandWalk,             "Sets landwalk move for unit",                              nullptr },
        { "waterwalk",          'd', &ChatHandler::HandleDebugWaterWalk,            "Sets waterwal move for unit",                              nullptr },
        { "hover",              'd', &ChatHandler::HandleDebugHover,                "Toggles hover move on/off for unit",                       nullptr },
        { "state",              'd', &ChatHandler::HandleDebugState,                "Display MovementFlags for unit",                           nullptr },
        { "swim",               'd', &ChatHandler::HandleDebugSwim,                 "Toggles swim move for unit",                               nullptr },
        { "fly",                'd', &ChatHandler::HandleDebugFly,                  "Toggles fly move for unit",                                nullptr },
        { "disablegravity",     'd', &ChatHandler::HandleDebugDisableGravity,       "Toggles disablegravitiy move for unit",                    nullptr },
        { "featherfall",        'd', &ChatHandler::HandleDebugFeatherFall,          "Toggles featherfall move for unit",                        nullptr },
        { "speed",              'd', &ChatHandler::HandleDebugSpeed,                "Sets move speed for unit. Usage: .debug speed <value>",    nullptr },
        { "castspell",          'd', &ChatHandler::HandleCastSpellCommand,          "Casts spell on target.",                                   nullptr },
        { "castself",           'd', &ChatHandler::HandleCastSelfCommand,           "Target casts spell <spellId> on itself.",                  nullptr },
        { "castspellne",        'd', &ChatHandler::HandleCastSpellNECommand,        "Casts spell by spellid on target (only plays animations)", nullptr },
        { "aggrorange",         'd', &ChatHandler::HandleAggroRangeCommand,         "Shows aggro Range of the selected Creature.",              nullptr },
        { "knockback",          'd', &ChatHandler::HandleKnockBackCommand,          "Knocks you back by <balue>.",                              nullptr },
        { "fade",               'd', &ChatHandler::HandleFadeCommand,               "Calls ModThreatModifyer() with <value>.",                  nullptr },
        { "threatMod",          'd', &ChatHandler::HandleThreatModCommand,          "Calls ModGeneratedThreatModifyer() with <value>.",         nullptr },
        { "calcThreat",         'd', &ChatHandler::HandleCalcThreatCommand,         "Calculates threat <dmg> <spellId>.",                       nullptr },
        { "threatList",         'd', &ChatHandler::HandleThreatListCommand,         "Returns all AI_Targets of the selected Creature.",         nullptr },
        { "gettptime",          'd', &ChatHandler::HandleGetTransporterTime,        "Grabs transporter travel time",                            nullptr },
        { "itempushresult",     'd', &ChatHandler::HandleSendItemPushResult,        "Sends item push result",                                   nullptr },
        { "setbit",             'd', &ChatHandler::HandleModifyBitCommand,          "",                                                         nullptr },
        { "setvalue",           'd', &ChatHandler::HandleModifyValueCommand,        "",                                                         nullptr },
        { "aispelltestbegin",   'd', &ChatHandler::HandleAIAgentDebugBegin,         "",                                                         nullptr },
        { "aispelltestcontinue",'d', &ChatHandler::HandleAIAgentDebugContinue,      "",                                                         nullptr },
        { "aispelltestskip",    'd', &ChatHandler::HandleAIAgentDebugSkip,          "",                                                         nullptr },
        { "dumpcoords",         'd', &ChatHandler::HandleDebugDumpCoordsCommmand,   "",                                                         nullptr },
        { "rangecheck",         'd', &ChatHandler::HandleRangeCheckCommand,         "Checks the range between the player and the target.",      nullptr },
        { "testlos",            'd', &ChatHandler::HandleCollisionTestLOS,          "Tests LoS",                                                nullptr },
        { "testindoor",         'd', &ChatHandler::HandleCollisionTestIndoor,       "Tests indoor",                                             nullptr },
        { "getheight",          'd', &ChatHandler::HandleCollisionGetHeight,        "Gets height",                                              nullptr },
        { "deathstate",         'd', &ChatHandler::HandleGetDeathState,             "Returns current deathstate for target",                    nullptr },
        { "sendfailed",         'd', &ChatHandler::HandleSendCastFailed,            "Sends failed cast result <x>",                             nullptr },
        { "playmovie",          'd', &ChatHandler::HandlePlayMovie,                 "Triggers a movie for selected player",                     nullptr },
        { "auraupdate",         'd', &ChatHandler::HandleAuraUpdateAdd,             "<SpellID> <Flags> <StackCount>",                           nullptr },
        { "auraremove",         'd', &ChatHandler::HandleAuraUpdateRemove,          "Remove Auras in visual slot",                              nullptr },
        { "spawnwar",           'd', &ChatHandler::HandleDebugSpawnWarCommand,      "Spawns desired amount of npcs to fight with eachother",    nullptr },
        { "updateworldstate",   'd', &ChatHandler::HandleUpdateWorldStateCommand,   "Sets the worldstate field to the specified value",         nullptr },
        { "initworldstates",    'd', &ChatHandler::HandleInitWorldStatesCommand,    "(Re)initializes the worldstates.",                         nullptr },
        { "clearworldstates",   'd', &ChatHandler::HandleClearWorldStatesCommand,   "Clears the worldstates",                                   nullptr },
        { "pvpcredit",          'm', &ChatHandler::HandleDebugPVPCreditCommand,     "Sends PVP credit packet, with specified rank and points",  nullptr },
        { "calcdist",           'd', &ChatHandler::HandleSimpleDistanceCommand,     "Displays distance between your position and x y z",        nullptr },
        { "setunitbyte",        'd', &ChatHandler::HandleDebugSetUnitByteCommand,   "Set value z for unit byte x with offset y.",               nullptr },
        { "setplayerflags",     'd', &ChatHandler::HandleDebugSetPlayerFlagsCommand,"Add player flags x to selected player",                    nullptr },
        { "getplayerflags",     'd', &ChatHandler::HandleDebugGetPlayerFlagsCommand,"Display current player flags of selected player x",        nullptr },
        { "setweather",         'd', &ChatHandler::HandleDebugSetWeatherCommand,    "Change zone weather <type> <densitiy>",        nullptr },
        { nullptr,              '0', nullptr,                                       "",                                                         nullptr }
    };
    dupe_command_table(debugCommandTable, _debugCommandTable);

    static ChatCommand waypointCommandTable[] =
    {
        { "add",                'w', &ChatHandler::HandleWayPointAddCommand,            "Add wp for selected creature at current pos.",     nullptr },
        { "addfly",             'w', &ChatHandler::HandleWayPointAddFlyCommand,         "Adds a flying waypoint for selected creature.",    nullptr },
        { "change",             'w', &ChatHandler::HandleWayPointChangeNumberCommand,   "Change wp ID for selected wp.",                    nullptr },
        { "delete",             'w', &ChatHandler::HandleWayPointDeleteCommand,         "Deletes selected wp.",                             nullptr },
        { "deleteall",          'w', &ChatHandler::HandleWayPointDeleteAllCommand,      "Deletes all waypoints of selected creature.",      nullptr },
        { "emote",              'w', &ChatHandler::HandleWayPointEmoteCommand,          "Set emote ID for selected wp.",                    nullptr },
        { "flags",              'w', &ChatHandler::HandleWayPointFlagsCommand,          "Set flags for selected wp.",                       nullptr },
        { "generate",           'w', &ChatHandler::HandleWayPointGenerateCommand,       "Randomly generate <x> wps for selected creature.", nullptr },
        { "hide",               'w', &ChatHandler::HandleWayPointHideCommand,           "Hide wp's for selected creature.",                 nullptr },
        { "info",               'w', &ChatHandler::HandleWayPointInfoCommand,           "Show info for selected wp.",                       nullptr },
        { "movehere",           'w', &ChatHandler::HandleWayPpointMoveHereCommand,      "Moves the selected wp to your position.",          nullptr },
        { "movetype",           'w', &ChatHandler::HandleWayPointMoveTypeCommand,       "Change movement type for selected wp.",            nullptr },
        { "save",               'w', &ChatHandler::HandleWayPointSaveCommand,           "Save all waypoints for selected creature.",        nullptr },
        { "show",               'w', &ChatHandler::HandleWayPointShowCommand,           "Show wp's for selected creature <bool backwards>", nullptr },
        { "skin",               'w', &ChatHandler::HandleWayPointSkinCommand,           "Sets Skin ID for selected wp.",                    nullptr },
        { "waittime",           'w', &ChatHandler::HandleWayPointWaitCommand,           "Sets Wait time in ms for selected wp.",            nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(waypointCommandTable, _waypointCommandTable);

    static ChatCommand GMTicketCommandTable[] =
    {
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        { "get",                'c', &ChatHandler::HandleGMTicketListCommand,           "Gets GM Ticket list.",                             nullptr },
        { "getId",              'c', &ChatHandler::HandleGMTicketGetByIdCommand,        "Gets GM Ticket by player name.",                   nullptr },
        { "delId",              'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,     "Deletes GM Ticket by player name.",                nullptr },
#else
        { "list",               'c', &ChatHandler::HandleGMTicketListCommand,           "Lists all active GM Tickets.",                     nullptr },
        { "get",                'c', &ChatHandler::HandleGMTicketGetByIdCommand,        "Gets GM Ticket with ID x.",                        nullptr },
        { "remove",             'c', &ChatHandler::HandleGMTicketRemoveByIdCommand,     "Removes GM Ticket with ID x.",                     nullptr },
        { "deletepermanent",    'z', &ChatHandler::HandleGMTicketDeletePermanentCommand, "Deletes GM Ticket with ID x permanently.",        nullptr },
        { "assign",             'c', &ChatHandler::HandleGMTicketAssignToCommand,       "Assigns GM Ticket with id x to GM y.",             nullptr },
        { "release",            'c', &ChatHandler::HandleGMTicketReleaseCommand,        "Releases assigned GM Ticket with ID x.",           nullptr },
        { "comment",            'c', &ChatHandler::HandleGMTicketCommentCommand,        "Sets comment x to GM Ticket with ID y.",           nullptr },
#endif
        { "toggle",             'z', &ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand, "Toggles the ticket system status.",      nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(GMTicketCommandTable, _GMTicketCommandTable);

    static ChatCommand TicketCommandTable[] =
    {
        { "list",               'c', &ChatHandler::HandleTicketListCommand,             "Shows all active tickets",                         nullptr },
        { "listall",            'c', &ChatHandler::HandleTicketListAllCommand,          "Shows all tickets in the database",                nullptr },
        { "get",                'c', &ChatHandler::HandleTicketGetCommand,              "Returns the content of the specified ID",          nullptr },
        { "close",              'c', &ChatHandler::HandleTicketCloseCommand,            "Close ticket with specified ID",                   nullptr },
        { "delete",             'a', &ChatHandler::HandleTicketDeleteCommand,           "Delete ticket by specified ID",                    nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(TicketCommandTable, _TicketCommandTable);

    static ChatCommand GuildCommandTable[] =
    {
        { "create",             'm', &ChatHandler::HandleGuildCreateCommand,            "Creates a guild.",                                 nullptr },
        { "disband",            'm', &ChatHandler::HandleGuildDisbandCommand,           "Disbands the guild of your target.",               nullptr },
#if VERSION_STRING >= Cata
        { "info",               'm', &ChatHandler::HandleGuildInfoCommand,              "Shows guild info of your target.",                 nullptr },
#endif
        { "join",               'm', &ChatHandler::HandleGuildJoinCommand,              "Force selected player to join a guild by name",    nullptr },
        { "listmembers",        'm', &ChatHandler::HandleGuildListMembersCommand,       "Lists guildmembers with ranks by guild name.",     nullptr },
        { "rename",             'm', &ChatHandler::HandleRenameGuildCommand,            "Renames a guild.",                                 nullptr },
        { "removeplayer",       'm', &ChatHandler::HandleGuildRemovePlayerCommand,      "Removes a player from a guild.",                   nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(GuildCommandTable, _GuildCommandTable);

    static ChatCommand GameObjectSetCommandTable[] =
    {
        { "animprogress",       'o', &ChatHandler::HandleGOSetAnimProgressCommand,      "Sets anim progress of selected GO",                nullptr },
        { "faction",            'o', &ChatHandler::HandleGOSetFactionCommand,           "Sets the faction of the GO",                       nullptr },
        { "flags",              'o', &ChatHandler::HandleGOSetFlagsCommand,             "Sets the flags of the GO",                         nullptr },
        { "overrides",          'o', &ChatHandler::HandleGOSetOverridesCommand,         "Sets override of selected GO",                     nullptr },
        { "phase",              'o', &ChatHandler::HandleGOSetPhaseCommand,             "Sets phase of selected GO",                        nullptr },
        { "scale",              'o', &ChatHandler::HandleGOSetScaleCommand,             "Sets scale of selected GO",                        nullptr },
        { "state",              'o', &ChatHandler::HandleGOSetStateCommand,             "Sets the state byte of the GO",                    nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(GameObjectSetCommandTable, _GameObjectSetCommandTable);

    static ChatCommand GameObjectCommandTable[] =
    {
        { "damage",             'o', &ChatHandler::HandleGODamageCommand,               "Damages the GO for the specified hitpoints",       nullptr },
        { "delete",             'o', &ChatHandler::HandleGODeleteCommand,               "Deletes selected GameObject",                      nullptr },
        { "enable",             'o', &ChatHandler::HandleGOEnableCommand,               "Enables the selected GO for use.",                 nullptr },
        { "export",             'o', &ChatHandler::HandleGOExportCommand,               "Exports the selected GO to .sql file",             nullptr },
        { "info",               'o', &ChatHandler::HandleGOInfoCommand,                 "Gives you information about selected GO",          nullptr },
        { "movehere",           'g', &ChatHandler::HandleGOMoveHereCommand,             "Moves gameobject to your position",                nullptr },
        { "open",               'o', &ChatHandler::HandleGOOpenCommand,                 "Toggles open/close (state) of selected GO.",       nullptr },
        { "rebuild",            'o', &ChatHandler::HandleGORebuildCommand,              "Rebuilds the GO.",                                 nullptr },
        { "rotate",             'g', &ChatHandler::HandleGORotateCommand,               "Rotates the object. <Axis> x,y, Default o.",       nullptr },
        { "select",             'o', &ChatHandler::HandleGOSelectCommand,               "Selects the nearest GameObject to you",            nullptr },
        { "selectguid",         'o', &ChatHandler::HandleGOSelectGuidCommand,           "Selects GO with <guid>",                           nullptr },
        { "set",                'o', nullptr,                                           "",                               GameObjectSetCommandTable},
        { "spawn",              'o', &ChatHandler::HandleGOSpawnCommand,                "Spawns a GameObject by ID",                        nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(GameObjectCommandTable, _GameObjectCommandTable);

    static ChatCommand BattlegroundCommandTable[] =
    {
        { "forceinitqueue",     'z', &ChatHandler::HandleBGForceInitQueueCommand,       "Forces init of all bgs with in queue.",            nullptr },
        { "getqueue",           'z', &ChatHandler::HandleBGGetQueueCommand,             "Gets common battleground queue information.",      nullptr },
        { "info",               'e', &ChatHandler::HandleBGInfoCommand,                 "Displays information about current bg.",           nullptr },
        { "leave",              'e', &ChatHandler::HandleBGLeaveCommand,                "Leaves the current battleground.",                 nullptr },
        { "menu",               'e', &ChatHandler::HandleBGMenuCommand,                 "Shows BG Menu for selected player by type <x>",    nullptr },
        { "pause",              'e', &ChatHandler::HandleBGPauseCommand,                "Pauses current battleground match.",               nullptr },
        { "playsound",          'e', &ChatHandler::HandleBGPlaySoundCommand,            "Plays sound to all players in bg <sound_id>",      nullptr },
        { "sendstatus",         'e', &ChatHandler::HandleBGSendStatusCommand,           "Sends status of bg by type <x>",                   nullptr },
        { "setscore",           'e', &ChatHandler::HandleBGSetScoreCommand,             "Sets bg score <Teamid> <Score>.",                  nullptr },
        { "setworldstate",      'e', &ChatHandler::HandleBGSetWorldStateCommand,        "Sets singe worldsate value.",                      nullptr },
        { "setworldstates",     'e', &ChatHandler::HandleBGSetWorldStatesCommand,       "Sets multipe worldstate values for start/end id",  nullptr },
        { "start",              'e', &ChatHandler::HandleBGStartCommand,                "Starts current battleground match.",               nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(BattlegroundCommandTable, _BattlegroundCommandTable);

    static ChatCommand NPCSetCommandTable[] =
    {
        { "canfly",             'n', &ChatHandler::HandleNpcSetCanFlyCommand,           "Toggles CanFly state",                             nullptr },
        { "emote",              'n', &ChatHandler::HandleNpcSetEmoteCommand,            "Sets emote state",                                 nullptr },
        { "equip",              'm', &ChatHandler::HandleNpcSetEquipCommand,            "Sets equipment itemt",                             nullptr },
        { "flags",              'n', &ChatHandler::HandleNpcSetFlagsCommand,            "Sets NPC flags",                                   nullptr },
        { "formationmaster",    'm', &ChatHandler::HandleNpcSetFormationMasterCommand,  "Sets formation master.",                           nullptr },
        { "formationslave",     'm', &ChatHandler::HandleNpcSetFormationSlaveCommand,   "Sets formation slave with distance and angle",     nullptr },
        { "formationclear",     'm', &ChatHandler::HandleNpcSetFormationClearCommand,   "Removes formation from creature",                  nullptr },
        { "ongameobject",       'n', &ChatHandler::HandleNpcSetOnGOCommand,             "Toggles onGameobject state.",                      nullptr },
        { "phase",              'n', &ChatHandler::HandleNpcSetPhaseCommand,            "Sets phase for selected creature",                 nullptr },
        { "standstate",         'm', &ChatHandler::HandleNpcSetStandstateCommand,       "Sets standstate for selected creature",            nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(NPCSetCommandTable, _NPCSetCommandTable);

    static ChatCommand NPCCommandTable[] =
    {
        { "addagent",           'n', &ChatHandler::HandleNpcAddAgentCommand,            "Add ai agents to npc.",                            nullptr },
        { "addtrainerspell",    'm', &ChatHandler::HandleNpcAddTrainerSpellCommand,     "Add spells to trainer learn list.",                nullptr },
        { "appear", 'n', &ChatHandler::HandleNpcAppearCommand, "Teleports you to the target NPC's location.", nullptr },
        { "cast",               'n', &ChatHandler::HandleNpcCastCommand,                "Makes NPC cast <spellid>.",                        nullptr },
        { "come",               'n', &ChatHandler::HandleNpcComeCommand,                "Makes NPC move to your position",                  nullptr },
        { "delete",             'n', &ChatHandler::HandleNpcDeleteCommand,              "Deletes mob from world optional from DB",          nullptr },
        { "info",               'n', &ChatHandler::HandleNpcInfoCommand,                "Displays NPC information",                         nullptr },
        { "listAgent",          'n', &ChatHandler::HandleNpcListAIAgentCommand,         "List AIAgents of selected target.",                nullptr },
        { "listloot",           'm', &ChatHandler::HandleNpcListLootCommand,            "Displays possible loot for the selected NPC.",     nullptr },
        { "follow",             'm', &ChatHandler::HandleNpcFollowCommand,              "Sets NPC to follow you",                           nullptr },
        { "stopfollow",         'm', &ChatHandler::HandleNpcStopFollowCommand,          "Sets NPC to not follow anything",                  nullptr },
        { "possess",            'n', &ChatHandler::HandlePossessCommand,                "Possess targeted NPC (mind control)",              nullptr },
        { "unpossess",          'n', &ChatHandler::HandleUnPossessCommand,              "Unpossess any currently possessed npc.",           nullptr },
        { "return",             'n', &ChatHandler::HandleNpcReturnCommand,              "Returns NPC to spawnpoint.",                       nullptr },
        { "respawn",            'n', &ChatHandler::HandleNpcRespawnCommand,             "Respawns a dead NPC from its corpse.",             nullptr },
        { "say",                'n', &ChatHandler::HandleNpcSayCommand,                 "Makes selected NPC say <text>.",                   nullptr },
        { "select",             'n', &ChatHandler::HandleNpcSelectCommand,              "Selects closest NPC",                               nullptr },
        { "set",                '0', nullptr,                                           "",                                      NPCSetCommandTable},
        { "spawn",              'n', &ChatHandler::HandleNpcSpawnCommand,               "Spawns NPC of entry <id>",                         nullptr },
        { "showtimers",         'm', &ChatHandler::HandleNpcShowTimersCommand,          "Shows timers for selected creature",               nullptr },
        { "vendoradditem",      'n', &ChatHandler::HandleNpcVendorAddItemCommand,       "Adds item to vendor",                              nullptr },
        { "vendorremoveitem",   'n', &ChatHandler::HandleNpcVendorRemoveItemCommand,    "Removes item from vendor.",                        nullptr },
        { "yell",               'n', &ChatHandler::HandleNpcYellCommand,                "Makes selected NPC yell <text>.",                  nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(NPCCommandTable, _NPCCommandTable);

    static ChatCommand CheatCommandTable[] =
    {
        { "list",               'm', &ChatHandler::HandleCheatListCommand,              "Shows active cheats.",                             nullptr },
        { "taxi",               'm', &ChatHandler::HandleCheatTaxiCommand,              "Toggles TaxiCheat.",                               nullptr },
        { "cooldown",           'm', &ChatHandler::HandleCheatCooldownCommand,          "Toggles CooldownCheat.",                           nullptr },
        { "casttime",           'm', &ChatHandler::HandleCheatCastTimeCommand,          "Toggles CastTimeCheat.",                           nullptr },
        { "power",              'm', &ChatHandler::HandleCheatPowerCommand,             "Toggles PowerCheat. Disables mana consumption.",   nullptr },
        { "god",                'm', &ChatHandler::HandleCheatGodCommand,               "Toggles GodCheat.",                                nullptr },
        { "fly",                'm', &ChatHandler::HandleCheatFlyCommand,               "Toggles FlyCheat.",                                nullptr },
        { "aurastack",          'm', &ChatHandler::HandleCheatAuraStackCommand,         "Toggles AuraStackCheat.",                          nullptr },
        { "itemstack",          'm', &ChatHandler::HandleCheatItemStackCommand,         "Toggles ItemStackCheat.",                          nullptr },
        { "triggerpass",        'm', &ChatHandler::HandleCheatTriggerpassCommand,       "Ignores area trigger prerequisites.",              nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(CheatCommandTable, _CheatCommandTable);

    static ChatCommand accountCommandTable[] =
    {
        { "create",     'a', &ChatHandler::HandleAccountCreate,         "Creates an account with name and password",                        nullptr },
        { "setgm",      'z', &ChatHandler::HandleAccountSetGMCommand,   "Sets gm level on account. Pass it username and 0,1,2,3,az, etc.",  nullptr },
        { "mute",       'a', &ChatHandler::HandleAccountMuteCommand,    "Mutes account for <timeperiod>.",                                  nullptr },
        { "unmute",     'a', &ChatHandler::HandleAccountUnmuteCommand,  "Unmutes account <x>",                                              nullptr },
        { "ban",        'a', &ChatHandler::HandleAccountBannedCommand,  "Bans account: .ban account <name> [duration] [reason]",            nullptr },
        { "unban",      'z', &ChatHandler::HandleAccountUnbanCommand,   "Unbans account x.",                                                nullptr },
        { "changepw",   '0', &ChatHandler::HandleAccountChangePassword, "Change the password of your account.",                             nullptr },
        { "getid",      '1', &ChatHandler::HandleAccountGetAccountID,   "Get Account ID for account name X",                                nullptr },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr }
    };
    dupe_command_table(accountCommandTable, _accountCommandTable);

    static ChatCommand petCommandTable[] =
    {
        { "create",     'm', &ChatHandler::HandlePetCreateCommand,      "Creates a pet with <entry>.",                                      nullptr },
        { "dismiss",    'm', &ChatHandler::HandlePetDismissCommand,     "Dismisses a pet by for selected player or selected pet.",          nullptr },
        { "rename",     'm', &ChatHandler::HandlePetRenameCommand,      "Renames a pet to <name>.",                                         nullptr },
        { "addspell",   'm', &ChatHandler::HandlePetAddSpellCommand,    "Teaches pet <spell>.",                                             nullptr },
        { "removespell",'m', &ChatHandler::HandlePetRemoveSpellCommand, "Removes pet spell <spell>.",                                       nullptr },
        { "setlevel",   'm', &ChatHandler::HandlePetSetLevelCommand,    "Sets pet level to <level>.",                                       nullptr },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr }
    };
    dupe_command_table(petCommandTable, _petCommandTable);

    //teleport
    static ChatCommand recallCommandTable[] =
    {
        { "list",       'q', &ChatHandler::HandleRecallListCommand,     "List recall locations",                                            nullptr },
        { "add",        'q', &ChatHandler::HandleRecallAddCommand,      "Add a recall location",                                            nullptr },
        { "del",        'q', &ChatHandler::HandleRecallDelCommand,      "Remove a recall location",                                         nullptr },
        { "port",       'q', &ChatHandler::HandleRecallGoCommand,       "Ports you to recalled location",                                   nullptr },
        { "portplayer", 'm', &ChatHandler::HandleRecallPortPlayerCommand,"Ports specified player to a recalled location",                   nullptr },
        { "portus",     'm', &ChatHandler::HandleRecallPortUsCommand,   "Ports you and the selected player to recalled location",           nullptr },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr }
    };
    dupe_command_table(recallCommandTable, _recallCommandTable);

    static ChatCommand questCommandTable[] =
    {
        { "addboth",    '2', &ChatHandler::HandleQuestAddBothCommand,   "Add quest <id> to the targeted NPC as start & finish",             nullptr },
        { "addfinish",  '2', &ChatHandler::HandleQuestAddFinishCommand, "Add quest <id> to the targeted NPC as finisher",                   nullptr },
        { "addstart",   '2', &ChatHandler::HandleQuestAddStartCommand,  "Add quest <id> to the targeted NPC as starter",                    nullptr },
        { "delboth",    '2', &ChatHandler::HandleQuestDelBothCommand,   "Delete quest <id> from the targeted NPC as start & finish",        nullptr },
        { "delfinish",  '2', &ChatHandler::HandleQuestDelFinishCommand, "Delete quest <id> from the targeted NPC as finisher",              nullptr },
        { "delstart",   '2', &ChatHandler::HandleQuestDelStartCommand,  "Delete quest <id> from the targeted NPC as starter",               nullptr },
        { "complete",   '2', &ChatHandler::HandleQuestFinishCommand,    "Complete/Finish quest <id>",                                       nullptr },
        { "fail",       '2', &ChatHandler::HandleQuestFailCommand,      "Fail quest <id>",                                                  nullptr },
        { "finisher",   '2', &ChatHandler::HandleQuestFinisherCommand,  "Lookup quest finisher for quest <id>",                             nullptr },
        { "item",       '2', &ChatHandler::HandleQuestItemCommand,      "Lookup itemid necessary for quest <id>",                           nullptr },
        { "list",       '2', &ChatHandler::HandleQuestListCommand,      "Lists the quests for the npc <id>",                                nullptr },
        { "load",       '2', &ChatHandler::HandleQuestLoadCommand,      "Loads quests from database",                                       nullptr },
        { "giver",      '2', &ChatHandler::HandleQuestGiverCommand,     "Lookup quest giver for quest <id>",                                nullptr },
        { "remove",     '2', &ChatHandler::HandleQuestRemoveCommand,    "Removes the quest <id> from the targeted player",                  nullptr },
        { "reward",     '2', &ChatHandler::HandleQuestRewardCommand,    "Shows reward for quest <id>",                                      nullptr },
        { "status",     '2', &ChatHandler::HandleQuestStatusCommand,    "Lists the status of quest <id>",                                   nullptr },
        { "start",      '2', &ChatHandler::HandleQuestStartCommand,     "Starts quest <id>",                                                nullptr },
        { "startspawn", '2', &ChatHandler::HandleQuestStarterSpawnCommand, "Port to spawn location for quest <id> (starter)",               nullptr },
        { "finishspawn",'2', &ChatHandler::HandleQuestFinisherSpawnCommand, "Port to spawn location for quest <id> (finisher)",             nullptr },
        { nullptr,      '0', nullptr,                                   "",                                                                 nullptr }
    };
    dupe_command_table(questCommandTable, _questCommandTable);

    static ChatCommand reloadTableCommandTable[] =
    {
        { "gameobjects",        'z', &ChatHandler::HandleReloadGameobjectsCommand,          "Reload gameobjets",                            nullptr },
        { "creatures",          'z', &ChatHandler::HandleReloadCreaturesCommand,            "Reload creatures",                             nullptr },
        { "areatriggers",       'z', &ChatHandler::HandleReloadAreaTriggersCommand,         "Reload areatriggers table",                    nullptr },
        { "command_overrides",  'z', &ChatHandler::HandleReloadCommandOverridesCommand,     "Reload command_overrides table",               nullptr },
        { "fishing",            'z', &ChatHandler::HandleReloadFishingCommand,              "Reload fishing table",                         nullptr },
        { "gossip_menu_option", 'z', &ChatHandler::HandleReloadGossipMenuOptionCommand,     "Reload gossip_menu_option table",              nullptr },
        { "graveyards",         'z', &ChatHandler::HandleReloadGraveyardsCommand,           "Reload graveyards table",                      nullptr },
        { "items",              'z', &ChatHandler::HandleReloadItemsCommand,                "Reload items table",                           nullptr },
        { "itempages",          'z', &ChatHandler::HandleReloadItempagesCommand,            "Reload itempages table",                       nullptr },
        { "npc_script_text",    'z', &ChatHandler::HandleReloadNpcScriptTextCommand,        "Reload npc_script_text table",                 nullptr },
        { "npc_text",           'z', &ChatHandler::HandleReloadNpcTextCommand,              "Reload npc_text table",                        nullptr },
        { "pet_level_abilities",'z', &ChatHandler::HandleReloadPetLevelAbilitiesCommand,    "Reload pet_level_abilities table",             nullptr },
        { "player_xp_for_level",'z', &ChatHandler::HandleReloadPlayerXpForLevelCommand,     "Reload player_xp_for_level table",             nullptr },
        { "points_of_interest", 'z', &ChatHandler::HandleReloadPointsOfInterestCommand,     "Reload points_of_interest table",              nullptr },
        { "quests",             'z', &ChatHandler::HandleReloadQuestsCommand,               "Reload quests table",                          nullptr },
        { "spell_teleport_coords",'z', &ChatHandler::HandleReloadTeleportCoordsCommand,     "Reload teleport_coords table",                 nullptr },
        { "worldbroadcast",     'z', &ChatHandler::HandleReloadWorldbroadcastCommand,       "Reload worldbroadcast table",                  nullptr },
        { "worldmap_info",      'z', &ChatHandler::HandleReloadWorldmapInfoCommand,         "Reload worldmap_info table",                   nullptr },
        { "worldstring_tables", 'z', &ChatHandler::HandleReloadWorldstringTablesCommand,    "Reload worldstring_tables table",              nullptr },
        { "zoneguards",         'z', &ChatHandler::HandleReloadZoneguardsCommand,           "Reload zoneguards table",                      nullptr },
        { nullptr,              '0', nullptr,                                                  "",                                          nullptr }
    };
    dupe_command_table(reloadTableCommandTable, _reloadTableCommandTable);

    static ChatCommand serverCommandTable[] =
    {
        { "info",               '0', &ChatHandler::HandleServerInfoCommand,             "Shows detailed Server info.",                      nullptr },
        { "rehash",             'z', &ChatHandler::HandleServerRehashCommand,           "Reloads config file.",                             nullptr },
        { "save",               's', &ChatHandler::HandleServerSaveCommand,             "Save targeted or named player.",                   nullptr },
        { "saveall",            's', &ChatHandler::HandleServerSaveAllCommand,          "Save all online player.",                          nullptr },
        { "setmotd",            'm', &ChatHandler::HandleServerSetMotdCommand,          "Sets server MessageOfTheDay.",                     nullptr },
        { "shutdown",           'z', &ChatHandler::HandleServerShutdownCommand,         "Initiates server shutdown in <x> seconds.",        nullptr },
        { "cancelshutdown",     'z', &ChatHandler::HandleServerCancelShutdownCommand,   "Cancels a Server Restart/Shutdown.",               nullptr },
        { "restart",            'z', &ChatHandler::HandleServerRestartCommand,          "Initiates server restart in <x> seconds.",         nullptr },
        { "reloadtable",        'm', nullptr,                                           "",                                 reloadTableCommandTable},
        { "reloadscript",       'm', &ChatHandler::HandleServerReloadScriptsCommand,    "",                                                 nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(serverCommandTable, _serverCommandTable);

    static ChatCommand gmCommandTable[] =
    {
        { "active",             't', &ChatHandler::HandleGMActiveCommand,               "Activate/Deactivate <GM> tag",                     nullptr },
        { "allowwhispers",      'c', &ChatHandler::HandleGMAllowWhispersCommand,        "Allows whispers from player <s>.",                 nullptr },
        { "announce",           'u', &ChatHandler::HandleGMAnnounceCommand,             "Sends announce to all online GMs",                 nullptr },
        { "blockwhispers",      'c', &ChatHandler::HandleGMBlockWhispersCommand,        "Blocks whispers from player <s>.",                 nullptr },
        { "devtag",             '1', &ChatHandler::HandleGMDevTagCommand,               "Activate/Deactivate <DEV> tag",                    nullptr },
        { "list",               '0', &ChatHandler::HandleGMListCommand,                 "Shows active GM's",                                nullptr },
        { "logcomment",         '1', &ChatHandler::HandleGMLogCommentCommand,           "Adds a comment to the GM log.",                    nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(gmCommandTable, _gmCommandTable);

    static ChatCommand characterAddCommandTable[] =
    {
        { "copper",             'm', &ChatHandler::HandleCharAddCopperCommand,          "Adds x copper to character.",                      nullptr },
        { "silver",             'm', &ChatHandler::HandleCharAddSilverCommand,          "Adds x silver to character.",                      nullptr },
        { "gold",               'm', &ChatHandler::HandleCharAddGoldCommand,            "Adds x gold to character.",                        nullptr },
        { "honorpoints",        'm', &ChatHandler::HandleCharAddHonorPointsCommand,     "Adds x amount of honor points/currency",           nullptr },
        { "honorkills",         'm', &ChatHandler::HandleCharAddHonorKillCommand,       "Adds x amount of honor kills",                     nullptr },
        { "item",               'm', &ChatHandler::HandleCharAddItemCommand,            "Adds item x count y",                              nullptr },
        { "itemset",            'm', &ChatHandler::HandleCharAddItemSetCommand,         "Adds item set to inv.",                            nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(characterAddCommandTable, _characterAddCommandTable);

    static ChatCommand characterSetCommandTable[] =
    {
        { "allexplored",    'm', &ChatHandler::HandleCharSetAllExploredCommand,     "Reveals the unexplored parts of the map.",             nullptr },
        { "gender",         'm', &ChatHandler::HandleCharSetGenderCommand,          "Changes gender of target. 0=male, 1=female.",          nullptr },
        { "itemsrepaired",  'n', &ChatHandler::HandleCharSetItemsRepairedCommand,   "Sets all items repaired for selected player",          nullptr },
        { "level",          'm', &ChatHandler::HandleCharSetLevelCommand,           "Sets level of selected target to <x>.",                nullptr },
        { "name",           'm', &ChatHandler::HandleCharSetNameCommand,            "Renames character x to y.",                            nullptr },
        { "phase",          'm', &ChatHandler::HandleCharSetPhaseCommand,           "Sets phase of selected player",                        nullptr },
        { "speed",          'm', &ChatHandler::HandleCharSetSpeedCommand,           "Sets speed of the selected target to <x>.",            nullptr },
        { "standing",       'm', &ChatHandler::HandleCharSetStandingCommand,        "Sets standing of faction x to y.",                     nullptr },
        { "talentpoints",   'm', &ChatHandler::HandleCharSetTalentpointsCommand,    "Sets available talent points of the target.",          nullptr },
        { "title",          'm', &ChatHandler::HandleCharSetTitleCommand,           "Sets pvp title for target",                            nullptr },
        { "forcerename",    'm', &ChatHandler::HandleCharSetForceRenameCommand,     "Forces char x to rename on next login",                nullptr },
        { "customize",      'm', &ChatHandler::HandleCharSetCustomizeCommand,       "Allows char x to customize on next login",             nullptr },
        { "factionchange",  'm', &ChatHandler::HandleCharSetFactionChangeCommand,   "Allows char x to change the faction on next login",    nullptr },
        { "racechange",     'm', &ChatHandler::HandleCharSetCustomizeCommand,       "Allows char x to change the race on next login",       nullptr },
        { nullptr,          '0', nullptr,                                           "",                                                     nullptr }
    };
    dupe_command_table(characterSetCommandTable, _characterSetCommandTable);

    static ChatCommand characterListCommandTable[] =
    {
        { "skills",         'm', &ChatHandler::HandleCharListSkillsCommand,         "Lists all the skills from a player",                   nullptr },
        { "standing",       'm', &ChatHandler::HandleCharListStandingCommand,       "Lists standing of faction x.",                         nullptr },
        { "items",          'm', &ChatHandler::HandleCharListItemsCommand,          "Lists items of selected Player",                       nullptr },
        { "kills",          'm', &ChatHandler::HandleCharListKillsCommand,          "Lists all kills of selected Player",                   nullptr },
        { "instances",      'z', &ChatHandler::HandleCharListInstanceCommand,       "Lists persistent instances of selected Player",        nullptr },
        { nullptr,          '0', nullptr,                                           "",                                                     nullptr }
    };
    dupe_command_table(characterListCommandTable, _characterListCommandTable);

    static ChatCommand characterCommandTable[] =
    {
        { "add",                'm', nullptr,                                           "",                                characterAddCommandTable},
        { "set",                'm', nullptr,                                           "",                                characterSetCommandTable},
        { "list",               'm', nullptr,                                           "",                               characterListCommandTable},
        { "clearcooldowns",     'm', &ChatHandler::HandleCharClearCooldownsCommand,     "Clears all cooldowns for your class.",             nullptr },
        { "demorph",            'm', &ChatHandler::HandleCharDeMorphCommand,            "Demorphs from morphed model.",                     nullptr },
        { "levelup",            'm', &ChatHandler::HandleCharLevelUpCommand,            "Player target will be levelup x levels",           nullptr },
        { "removeauras",        'm', &ChatHandler::HandleCharRemoveAurasCommand,        "Removes all auras from target",                    nullptr },
        { "removesickness",     'm', &ChatHandler::HandleCharRemoveSickessCommand,      "Removes ressurrection sickness from target",       nullptr },
        { "learn",              'm', &ChatHandler::HandleCharLearnCommand,              "Learns spell <x> or all available spells by race", nullptr },
        { "unlearn",            'm', &ChatHandler::HandleCharUnlearnCommand,            "Unlearns spell",                                   nullptr },
        { "learnskill",         'm', &ChatHandler::HandleCharLearnSkillCommand,         "Learns skill id skillid opt: min max.",            nullptr },
        { "advanceskill",       'm', &ChatHandler::HandleCharAdvanceSkillCommand,       "Advances skill line x y times.",                   nullptr },
        { "removeskill",        'm', &ChatHandler::HandleCharRemoveSkillCommand,        "Removes skill.",                                   nullptr },
        { "increaseweaponskill",'m', &ChatHandler::HandleCharIncreaseWeaponSkill,       "Increase equipped weapon skill x times.",          nullptr },
        { "resetreputation",    'n', &ChatHandler::HandleCharResetReputationCommand,    "Resets reputation to start levels.",               nullptr },
        { "resetspells",        'n', &ChatHandler::HandleCharResetSpellsCommand,        "Resets all spells of selected player.",            nullptr },
        { "resettalents",       'n', &ChatHandler::HandleCharResetTalentsCommand,       "Resets all talents of selected player.",           nullptr },
        { "resetskills",        'n', &ChatHandler::HandleCharResetSkillsCommand,        "Resets all skills.",                               nullptr },
        { "removeitem",         'm', &ChatHandler::HandleCharRemoveItemCommand,         "Removes item x count y.",                          nullptr },
        { "advanceallskills",   'm', &ChatHandler::HandleAdvanceAllSkillsCommand,       "Advances all skills <x> points.",                  nullptr },
        { nullptr,              '0', nullptr,                                           "",                                                 nullptr }
    };
    dupe_command_table(characterCommandTable, _characterCommandTable);

    static ChatCommand lookupCommandTable[] =
    {
        { "achievement",    'l', &ChatHandler::HandleLookupAchievementCommand,  "Looks up achievement string x.",                                   nullptr },
        { "creature",       'l', &ChatHandler::HandleLookupCreatureCommand,     "Looks up creature string x.",                                      nullptr },
        { "faction",        'l', &ChatHandler::HandleLookupFactionCommand,      "Looks up faction string x.",                                       nullptr },
        { "item",           'l', &ChatHandler::HandleLookupItemCommand,         "Looks up item string x.",                                          nullptr },
        { "object",         'l', &ChatHandler::HandleLookupObjectCommand,       "Looks up gameobject string x.",                                    nullptr },
        { "quest",          'l', &ChatHandler::HandleLookupQuestCommand,        "Looks up quest string x.",                                         nullptr },
        { "spell",          'l', &ChatHandler::HandleLookupSpellCommand,        "Looks up spell string x.",                                         nullptr },
        { "skill",          'l', &ChatHandler::HandleLookupSkillCommand,        "Looks up skill string x.",                                         nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(lookupCommandTable, _lookupCommandTable);

    static ChatCommand adminCommandTable[] =
    {
        { "castall",        'z', &ChatHandler::HandleAdminCastAllCommand,       "Makes all players online cast spell <x>.",                         nullptr },
        { "dispelall",      'z', &ChatHandler::HandleAdminDispelAllCommand,     "Dispels all negative (or positive w/ 1) auras on all players.",    nullptr },
        { "masssummon",     'z', &ChatHandler::HandleAdminMassSummonCommand,    "Summons all online players to you, use a/h for alliance/horde.",   nullptr },
        { "playall",        'z', &ChatHandler::HandleAdminPlayGlobalSoundCommand, "Plays a sound to everyone on the realm.",                        nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(adminCommandTable, _adminCommandTable);

    static ChatCommand kickCommandTable[] =
    {
        { "player",         'f', &ChatHandler::HandleKickByNameCommand,         "Disconnects the player with name <s>.",                            nullptr },
        { "account",        'f', &ChatHandler::HandleKKickBySessionCommand,     "Disconnects the session with account name <s>.",                   nullptr },
        { "ip",             'f', &ChatHandler::HandleKickByIPCommand,           "Disconnects the session with the ip <s>.",                         nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(kickCommandTable, _kickCommandTable);

    static ChatCommand banCommandTable[] =
    {
        { "ip",             'm', &ChatHandler::HandleIPBanCommand,              "Bans IP by <address> [duration]",                                  nullptr },
        { "character",      'b', &ChatHandler::HandleBanCharacterCommand,       "Bans character by <charname> [duration] [reason]",                 nullptr },
        { "all",            'a', &ChatHandler::HandleBanAllCommand,             "Bans all by <charname> [duration] [reason]",                       nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(banCommandTable, _banCommandTable);

    static ChatCommand unbanCommandTable[] =
    {
        { "ip",             'm', &ChatHandler::HandleIPUnBanCommand,            "Deletes an address from the IP ban table: <address>",              nullptr },
        { "character",      'b', &ChatHandler::HandleUnBanCharacterCommand,     "Unbans character x",                                               nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(unbanCommandTable, _unbanCommandTable);

    static ChatCommand instanceCommandTable[] =
    {
        { "create",         'z', &ChatHandler::HandleCreateInstanceCommand,     "Creates instance by mapid x y z",                                  nullptr },
        { "countcreature",  'z', &ChatHandler::HandleCountCreaturesCommand,     "Returns number of creatures with entry x",                         nullptr },
        { "exit",           'm', &ChatHandler::HandleExitInstanceCommand,       "Exits current instance, return to entry point.",                   nullptr },
        { "info",           'm', &ChatHandler::HandleGetInstanceInfoCommand,    "Gets info about instance with ID x (default current instance).",   nullptr },
        { "reset",          'z', &ChatHandler::HandleResetInstanceCommand,      "Removes instance ID x from target player.",                        nullptr },
        { "resetall",       'm', &ChatHandler::HandleResetAllInstancesCommand,  "Removes all instance IDs from target player.",                     nullptr },
        { "shutdown",       'z', &ChatHandler::HandleShutdownInstanceCommand,   "Shutdown instance with ID x (default is current instance).",       nullptr },
        { "showtimers",     'm', &ChatHandler::HandleShowTimersCommand,         "Show timers for current instance.",                                nullptr },
        { nullptr,          '0', nullptr,                                       "",                                                                 nullptr }
    };
    dupe_command_table(instanceCommandTable, _instanceCommandTable);

    static ChatCommand arenaCommandTable[] =
    {
        { "createteam",      'e', &ChatHandler::HandleArenaCreateTeam,          "Creates arena team with <type> <name>",                            nullptr },
        { "setteamleader",   'e', &ChatHandler::HandleArenaSetTeamLeader,       "Sets the arena team leader for <type>",                            nullptr },
        { "resetallratings", 'z', &ChatHandler::HandleArenaTeamResetAllRatings, "Resets all arena teams to their default rating",                   nullptr },
        { nullptr,           '0', nullptr,                                      "",                                                                 nullptr }
    };
    dupe_command_table(arenaCommandTable, _arenaCommandTable);

    static ChatCommand achievementCommandTable[] =
    {
#if VERSION_STRING > TBC
        { "complete",       'm', &ChatHandler::HandleAchievementCompleteCommand,    "Completes the specified achievement.",                         nullptr },
        { "criteria",       'm', &ChatHandler::HandleAchievementCriteriaCommand,    "Completes the specified achievement criteria.",                nullptr },
        { "reset",          'm', &ChatHandler::HandleAchievementResetCommand,       "Resets achievement data from the target.",                     nullptr },
#endif
        { nullptr,          '0', nullptr,                                           "",                                                             nullptr }
    };
    dupe_command_table(achievementCommandTable, _achievementCommandTable);

    static ChatCommand vehicleCommandTable[] = {
        { "ejectpassenger",     'm', &ChatHandler::HandleVehicleEjectPassengerCommand,      "Ejects the passenger from the specified seat",         nullptr },
        { "ejectallpassengers", 'm', &ChatHandler::HandleVehicleEjectAllPassengersCommand,  "Ejects all passengers from the vehicle",               nullptr },
        { "installaccessories", 'm', &ChatHandler::HandleVehicleInstallAccessoriesCommand,  "Installs the accessories for the selected vehicle",    nullptr },
        { "removeaccessories",  'm', &ChatHandler::HandleVehicleRemoveAccessoriesCommand,   "Removes the accessories of the selected vehicle",      nullptr },
        { "addpassenger",       'm', &ChatHandler::HandleVehicleAddPassengerCommand,        "Adds a new NPC passenger to the vehicle",              nullptr },
        { nullptr,              '0', nullptr,                                               "",                                                     nullptr }
    };

    dupe_command_table(vehicleCommandTable, _vehicleCommandTable);

    static ChatCommand commandTable[] =
    {
        { "commands",           '0', &ChatHandler::HandleCommandsCommand,               "Shows commands",                                           nullptr },
        { "help",               '0', &ChatHandler::HandleHelpCommand,                   "Shows help for command",                                   nullptr },
        { "autosavechanges",    '1', &ChatHandler::HandleAutoSaveChangesCommand,        "Toggles auto save for db table related commands.",         nullptr },
        { "event",              '0', nullptr,                                           "",                                               eventCommandTable},
        { "announce",           'u', &ChatHandler::HandleAnnounceCommand,               "Sends a normal chat message to all players.",              nullptr },
        { "wannounce",          'u', &ChatHandler::HandleWAnnounceCommand,              "Sends a widescreen announcement to all players.",          nullptr },
        { "appear",             'v', &ChatHandler::HandleAppearCommand,                 "Teleports to x's position.",                               nullptr },
        { "blockappear",        'v', &ChatHandler::HandleBlockAppearCommand,            "Blocks appearance to your position.",                      nullptr },
        { "summon",             'v', &ChatHandler::HandleSummonCommand,                 "Summons x to your position.",                              nullptr },
        { "blocksummon",        'v', &ChatHandler::HandleBlockSummonCommand,            "Blocks summons to others position.",                       nullptr },
        { "kill",               'r', &ChatHandler::HandleKillCommand,                   "Kills selected unit or player by name",                    nullptr },
        { "revive",             'r', &ChatHandler::HandleReviveCommand,                 "Revives you or a selected target or player by name",       nullptr },
        { "mount",              'm', &ChatHandler::HandleMountCommand,                  "Mounts targeted unit with modelid x.",                     nullptr },
        { "dismount",           'h', &ChatHandler::HandleDismountCommand,               "Dismounts targeted unit.",                                 nullptr },
        { "gps",                '0', &ChatHandler::HandleGPSCommand,                    "Shows position of targeted unit",                          nullptr },
        { "worldport",          'v', &ChatHandler::HandleWorldPortCommand,              "Teleports you to a location with mapid x y z",             nullptr },
        { "invincible",         'j', &ChatHandler::HandleInvincibleCommand,             "Toggles invincibility on/off",                             nullptr },
        { "invisible",          'i', &ChatHandler::HandleInvisibleCommand,              "Toggles invisibility and invincibility on/off",            nullptr },
        { "playerinfo",         'm', &ChatHandler::HandlePlayerInfo,                    "Displays info for selected character or <charname>",       nullptr },
        { "modify",             '0', nullptr,                                           "",                                              modifyCommandTable},
        { "waypoint",           '0', nullptr,                                           "",                                            waypointCommandTable},
        { "debug",              '0', nullptr,                                           "",                                               debugCommandTable},
        { "gm",                 '0', nullptr,                                           "",                                                  gmCommandTable},
        { "gmTicket",           '0', nullptr,                                           "",                                            GMTicketCommandTable},
        { "ticket",             '0', nullptr,                                           "",                                              TicketCommandTable},
        { "gobject",            '0', nullptr,                                           "",                                          GameObjectCommandTable},
        { "battleground",       '0', nullptr,                                           "",                                        BattlegroundCommandTable},
        { "npc",                '0', nullptr,                                           "",                                                 NPCCommandTable},
        { "cheat",              '0', nullptr,                                           "",                                               CheatCommandTable},
        { "account",            '0', nullptr,                                           "",                                             accountCommandTable},
        { "quest",              '0', nullptr,                                           "",                                               questCommandTable},
        { "pet",                '0', nullptr,                                           "",                                                 petCommandTable},
        { "recall",             '0', nullptr,                                           "",                                              recallCommandTable},
        { "guild",              '0', nullptr,                                           "",                                               GuildCommandTable},
        { "server",             '0', nullptr,                                           "",                                              serverCommandTable},
        { "character",          '0', nullptr,                                           "",                                           characterCommandTable},
        { "lookup",             '0', nullptr,                                           "",                                              lookupCommandTable},
        { "admin",              '0', nullptr,                                           "",                                               adminCommandTable},
        { "kick",               '0', nullptr,                                           "",                                                kickCommandTable},
        { "ban",                '0', nullptr,                                           "",                                                 banCommandTable},
        { "unban",              '0', nullptr,                                           "",                                               unbanCommandTable},
        { "instance",           '0', nullptr,                                           "",                                            instanceCommandTable},
        { "arena",              '0', nullptr,                                           "",                                               arenaCommandTable},
        { "unroot",             'b', &ChatHandler::HandleUnrootCommand,                 "Unroots selected target.",                                 nullptr },
        { "root",               'b', &ChatHandler::HandleRootCommand,                   "Roots selected target.",                                   nullptr },
        { "gocreature",         'v', &ChatHandler::HandleGoCreatureSpawnCommand,        "Teleports you to the creature with <spwn_id>.",            nullptr },
        { "gogameobject",       'v', &ChatHandler::HandleGoGameObjectSpawnCommand,      "Teleports you to the gameobject with <spawn_id>.",         nullptr },
        { "gostartlocation",    'm', &ChatHandler::HandleGoStartLocationCommand,        "Teleports you to a starting location",                     nullptr },
        { "gotrig",             'v', &ChatHandler::HandleGoTriggerCommand,              "Teleports you to the areatrigger with <id>.",              nullptr },
        { "achieve",            '0', nullptr,                                           "",                                         achievementCommandTable},
        { "vehicle",            'm', nullptr,                                           "",                                             vehicleCommandTable},
        { "transport",          'm', nullptr,                                           "",                                           transportCommandTable},
        { nullptr,              '0', nullptr,                                           "",                                                         nullptr }
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

