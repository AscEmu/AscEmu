/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CommandTableStorage.hpp"

#include <sstream>

#include "ChatCommand.hpp"
#include "ChatHandler.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Strings.hpp"

CommandTableStorage::CommandTableStorage()
{
    m_commandRegistry = {

        {"account",                       "0" },
        {"account create",                "a", wrap(&ChatHandler::handleAccountCreate),               "Creates an account with name and password" },
        {"account setgm",                 "z", wrap(&ChatHandler::handleAccountSetGMCommand),         "Sets gm level on account. Pass it username and 0,1,2,3,az, etc." },
        {"account mute",                  "a", wrap(&ChatHandler::handleAccountMuteCommand),          "Mutes account for <timeperiod>." },
        {"account unmute",                "a", wrap(&ChatHandler::handleAccountUnmuteCommand),        "Unmutes account <x>" },
        {"account ban",                   "a", wrap(&ChatHandler::handleAccountBannedCommand),        "Bans account: .ban account <name> [duration] [reason]" },
        {"account unban",                 "z", wrap(&ChatHandler::handleAccountUnbanCommand),         "Unbans account x." },
        {"account changepw",              "0", wrap(&ChatHandler::handleAccountChangePassword),       "Change the password of your account." },
        {"account getid",                 "1", wrap(&ChatHandler::handleAccountGetAccountID),         "Get Account ID for account name X" },

        {"achieve",                       "0" },
#if VERSION_STRING > TBC
        {"achieve complete",              "m", wrap(&ChatHandler::handleAchievementCompleteCommand),  "Completes the specified achievement." },
        {"achieve criteria",              "m", wrap(&ChatHandler::handleAchievementCriteriaCommand),  "Completes the specified achievement criteria." },
        {"achieve reset",                 "m", wrap(&ChatHandler::handleAchievementResetCommand),     "Resets achievement data from the target." },
#endif

        {"admin",                         "0" },
        {"admin castall",                 "z", wrap(&ChatHandler::HandleAdminCastAllCommand),         "Makes all players online cast spell <x>." },
        {"admin dispelall",               "z", wrap(&ChatHandler::HandleAdminDispelAllCommand),       "Dispels all negative (or positive w/ 1) auras on all players." },
        {"admin masssummon",              "z", wrap(&ChatHandler::HandleAdminMassSummonCommand),      "Summons all online players to you, use a/h for alliance/horde." },
        {"admin playall",                 "z", wrap(&ChatHandler::HandleAdminPlayGlobalSoundCommand), "Plays a sound to everyone on the realm." },

        {"announce",                      "u", wrap(&ChatHandler::HandleAnnounceCommand),             "Sends a normal chat message to all players." },
        {"appear",                        "v", wrap(&ChatHandler::HandleAppearCommand),               "Teleports to x's position." },

        {"arena",                         "0" },
        {"arena createteam",              "e", wrap(&ChatHandler::HandleArenaCreateTeam),             "Creates arena team with <type> <name>" },
        {"arena setteamleader",           "e", wrap(&ChatHandler::HandleArenaSetTeamLeader),          "Sets the arena team leader for <type>" },
        {"arena resetallratings",         "z", wrap(&ChatHandler::HandleArenaTeamResetAllRatings),    "Resets all arena teams to their default rating" },

        {"ban",                           "0" },
        {"ban ip",                        "m", wrap(&ChatHandler::HandleIPBanCommand),                "Bans IP by <address> [duration]" },
        {"ban character",                 "b", wrap(&ChatHandler::HandleBanCharacterCommand),         "Bans character by <charname> [duration] [reason]" },
        {"ban all",                       "a", wrap(&ChatHandler::HandleBanAllCommand),               "Bans all by <charname> [duration] [reason]" },

        {"battleground",                  "0" },
        {"battleground forceinitqueue",   "z", wrap(&ChatHandler::HandleBGForceInitQueueCommand),     "Forces init of all bgs with in queue." },
        {"battleground getqueue",         "z", wrap(&ChatHandler::HandleBGGetQueueCommand),           "Gets common battleground queue information." },
        {"battleground info",             "e", wrap(&ChatHandler::HandleBGInfoCommand),               "Displays information about current bg." },
        {"battleground leave",            "e", wrap(&ChatHandler::HandleBGLeaveCommand),              "Leaves the current battleground." },
        {"battleground menu",             "e", wrap(&ChatHandler::HandleBGMenuCommand),               "Shows BG Menu for selected player by type <x>" },
        {"battleground pause",            "e", wrap(&ChatHandler::HandleBGPauseCommand),              "Pauses current battleground match." },
        {"battleground playsound",        "e", wrap(&ChatHandler::HandleBGPlaySoundCommand),          "Plays sound to all players in bg <sound_id>" },
        {"battleground sendstatus",       "e", wrap(&ChatHandler::HandleBGSendStatusCommand),         "Sends status of bg by type <x>" },
        {"battleground setscore",         "e", wrap(&ChatHandler::HandleBGSetScoreCommand),           "Sets bg score <Teamid> <Score>." },
        {"battleground setworldstate",    "e", wrap(&ChatHandler::HandleBGSetWorldStateCommand),      "Sets singe worldsate value." },
        {"battleground setworldstates",   "e", wrap(&ChatHandler::HandleBGSetWorldStatesCommand),     "Sets multipe worldstate values for start/end id" },
        {"battleground start",            "e", wrap(&ChatHandler::HandleBGStartCommand),              "Starts current battleground match." },

        {"blockappear",                   "v", wrap(&ChatHandler::HandleBlockAppearCommand),          "Blocks appearance to your position." },
        {"blocksummon",                   "v", wrap(&ChatHandler::HandleBlockSummonCommand),          "Blocks summons to others position." },

        {"character",                     "0" },
        
        {"character add",                 "m" },
        {"character add copper",          "m", wrap(&ChatHandler::HandleCharAddCopperCommand),        "Adds x copper to character." },
        {"character add silver",          "m", wrap(&ChatHandler::HandleCharAddSilverCommand),        "Adds x silver to character." },
        {"character add gold",            "m", wrap(&ChatHandler::HandleCharAddGoldCommand),          "Adds x gold to character." },
        {"character add honorpoints",     "m", wrap(&ChatHandler::HandleCharAddHonorPointsCommand),   "Adds x amount of honor points/currency" },
        {"character add honorkills",      "m", wrap(&ChatHandler::HandleCharAddHonorKillCommand),     "Adds x amount of honor kills" },
        {"character add item",            "m", wrap(&ChatHandler::HandleCharAddItemCommand),          "Adds item x count y" },
        {"character add itemset",         "m", wrap(&ChatHandler::HandleCharAddItemSetCommand),       "Adds item set to inv." },
        
        {"character set",                 "m" },
        {"character set allexplored",     "m", wrap(&ChatHandler::HandleCharSetAllExploredCommand),   "Reveals the unexplored parts of the map." },
        {"character set gender",          "m", wrap(&ChatHandler::HandleCharSetGenderCommand),        "Changes gender of target. 0=male, 1=female." },
        {"character set itemsrepaired",   "n", wrap(&ChatHandler::HandleCharSetItemsRepairedCommand), "Sets all items repaired for selected player" },
        {"character set level",           "m", wrap(&ChatHandler::HandleCharSetLevelCommand),         "Sets level of selected target to <x>." },
        {"character set name",            "m", wrap(&ChatHandler::HandleCharSetNameCommand),          "Renames character x to y." },
        {"character set phase",           "m", wrap(&ChatHandler::HandleCharSetPhaseCommand),         "Sets phase of selected player" },
        {"character set speed",           "m", wrap(&ChatHandler::HandleCharSetSpeedCommand),         "Sets speed of the selected target to <x>." },
        {"character set standing",        "m", wrap(&ChatHandler::HandleCharSetStandingCommand),      "Sets standing of faction x to y." },
        {"character set talentpoints",    "m", wrap(&ChatHandler::HandleCharSetTalentpointsCommand),  "Sets available talent points of the target." },
        {"character set title",           "m", wrap(&ChatHandler::HandleCharSetTitleCommand),         "Sets pvp title for target" },
        {"character set forcerename",     "m", wrap(&ChatHandler::HandleCharSetForceRenameCommand),   "Forces char x to rename on next login" },
        {"character set customize",       "m", wrap(&ChatHandler::HandleCharSetCustomizeCommand),     "Allows char x to customize on next login" },
        {"character set factionchange",   "m", wrap(&ChatHandler::HandleCharSetFactionChangeCommand), "Allows char x to change the faction on next login" },
        {"character set racechange",      "m", wrap(&ChatHandler::HandleCharSetCustomizeCommand),     "Allows char x to change the race on next login" },
        
        {"character list",                "m" },
        {"character list skills",         "m", wrap(&ChatHandler::HandleCharListSkillsCommand),       "Lists all the skills from a player" },
        {"character list spells",         "m", wrap(&ChatHandler::handleCharListSpellsCommand),       "Lists all the spells from a player" },
        {"character list standing",       "m", wrap(&ChatHandler::HandleCharListStandingCommand),     "Lists standing of faction x." },
        {"character list items",          "m", wrap(&ChatHandler::HandleCharListItemsCommand),        "Lists items of selected Player" },
        {"character list kills",          "m", wrap(&ChatHandler::HandleCharListKillsCommand),        "Lists all kills of selected Player" },
        {"character list instances",      "z", wrap(&ChatHandler::HandleCharListInstanceCommand),     "Lists persistent instances of selected Player" },
        
        {"character clearcooldowns",      "m", wrap(&ChatHandler::HandleCharClearCooldownsCommand),   "Clears all cooldowns for your class." },
        {"character demorph",             "m", wrap(&ChatHandler::HandleCharDeMorphCommand),          "Demorphs from morphed model." },
        {"character levelup",             "m", wrap(&ChatHandler::HandleCharLevelUpCommand),          "Player target will be levelup x levels" },
        {"character removeauras",         "m", wrap(&ChatHandler::HandleCharRemoveAurasCommand),      "Removes all auras from target" },
        {"character removesickness",      "m", wrap(&ChatHandler::HandleCharRemoveSickessCommand),    "Removes ressurrection sickness from target" },
        {"character learn",               "m", wrap(&ChatHandler::HandleCharLearnCommand),            "Learns spell <x> or all available spells by race" },
        {"character unlearn",             "m", wrap(&ChatHandler::HandleCharUnlearnCommand),          "Unlearns spell" },
        {"character learnskill",          "m", wrap(&ChatHandler::HandleCharLearnSkillCommand),       "Learns skill id skillid opt: min max." },
        {"character advanceskill",        "m", wrap(&ChatHandler::HandleCharAdvanceSkillCommand),     "Advances skill line x y times." },
        {"character removeskill",         "m", wrap(&ChatHandler::HandleCharRemoveSkillCommand),      "Removes skill." },
        {"character increaseweaponskill", "m", wrap(&ChatHandler::HandleCharIncreaseWeaponSkill),     "Increase equipped weapon skill x times." },
        {"character resetreputation",     "n", wrap(&ChatHandler::HandleCharResetReputationCommand),  "Resets reputation to start levels." },
        {"character resetspells",         "n", wrap(&ChatHandler::HandleCharResetSpellsCommand),      "Resets all spells of selected player." },
        {"character resettalents",        "n", wrap(&ChatHandler::HandleCharResetTalentsCommand),     "Resets all talents of selected player." },
#if VERSION_STRING >= TBC
        {"character resetskills",         "n", wrap(&ChatHandler::HandleCharResetSkillsCommand),      "Resets all skills." },
#endif
        {"character removeitem",          "m", wrap(&ChatHandler::HandleCharRemoveItemCommand),       "Removes item x count y." },
        {"character advanceallskills",    "m", wrap(&ChatHandler::HandleAdvanceAllSkillsCommand),     "Advances all skills <x> points." },

        {"cheat",                         "0" },
        {"cheat list",                    "m", wrap(&ChatHandler::HandleCheatListCommand),            "Shows active cheats." },
        {"cheat taxi",                    "m", wrap(&ChatHandler::HandleCheatTaxiCommand),            "Toggles TaxiCheat." },
        {"cheat cooldown",                "m", wrap(&ChatHandler::HandleCheatCooldownCommand),        "Toggles CooldownCheat." },
        {"cheat casttime",                "m", wrap(&ChatHandler::HandleCheatCastTimeCommand),        "Toggles CastTimeCheat." },
        {"cheat power",                   "m", wrap(&ChatHandler::HandleCheatPowerCommand),           "Toggles PowerCheat. Disables mana consumption." },
        {"cheat god",                     "m", wrap(&ChatHandler::HandleCheatGodCommand),             "Toggles GodCheat." },
        {"cheat fly",                     "m", wrap(&ChatHandler::HandleCheatFlyCommand),             "Toggles FlyCheat." },
        {"cheat aurastack",               "m", wrap(&ChatHandler::HandleCheatAuraStackCommand),       "Toggles AuraStackCheat." },
        {"cheat itemstack",               "m", wrap(&ChatHandler::HandleCheatItemStackCommand),       "Toggles ItemStackCheat." },
        {"cheat triggerpass",             "m", wrap(&ChatHandler::HandleCheatTriggerpassCommand),     "Ignores area trigger prerequisites." },

        {"commands",                      "0", wrap(&ChatHandler::handleCommandsCommand),             "Shows commands" },

        {"debug",                         "0" },
        {"debug dumpscripts",             "d", wrap(&ChatHandler::HandleMoveHardcodedScriptsToDBCommand), "Dumps hardcoded aispells to cmdline for creatures on map X" },
        {"debug sendcreaturemove",        "d", wrap(&ChatHandler::HandleDebugSendCreatureMove),           "Requests the target creature moves to you using movement manager." },
        {"debug dopctdamage",             "z", wrap(&ChatHandler::HandleDoPercentDamageCommand),          "Do percent damage to creature target" },
        {"debug setscriptphase",          "z", wrap(&ChatHandler::HandleSetScriptPhaseCommand),           "ScriptPhase test" },
        {"debug aicharge",                "z", wrap(&ChatHandler::HandleAiChargeCommand),                 "AiCharge test" },
        {"debug aiknockback",             "z", wrap(&ChatHandler::HandleAiKnockbackCommand),              "AiKnockBack test" },
        {"debug aijump",                  "z", wrap(&ChatHandler::HandleAiJumpCommand),                   "AiJump test" },
        {"debug aifalling",               "z", wrap(&ChatHandler::HandleAiFallingCommand),                "AiFalling test" },
        {"debug movetospawn",             "z", wrap(&ChatHandler::HandleMoveToSpawnCommand),              "Move target to spwn" },
        {"debug position",                "z", wrap(&ChatHandler::HandlePositionCommand),                 "Show position" },
        {"debug setorientation",          "z", wrap(&ChatHandler::HandleSetOrientationCommand),           "Sets orientation on npc" },
        {"debug dumpmovement",            "d", wrap(&ChatHandler::HandleDebugDumpMovementCommand),        "Dumps the player's movement information to chat" },
        {"debug infront",                 "d", wrap(&ChatHandler::HandleDebugInFrontCommand),             "" },
        {"debug showreact",               "d", wrap(&ChatHandler::HandleShowReactionCommand),             "" },
        {"debug aimove",                  "d", wrap(&ChatHandler::HandleAIMoveCommand),                   "" },
        {"debug dist",                    "d", wrap(&ChatHandler::HandleDistanceCommand),                 "" },
        {"debug face",                    "d", wrap(&ChatHandler::HandleFaceCommand),                     "" },
        {"debug dumpstate",               "d", wrap(&ChatHandler::HandleDebugDumpState),                  "" },
        {"debug moveinfo",                "d", wrap(&ChatHandler::HandleDebugMoveInfo),                   "" },
        {"debug landwalk",                "d", wrap(&ChatHandler::HandleDebugLandWalk),                   "Sets landwalk move for unit" },
        {"debug waterwalk",               "d", wrap(&ChatHandler::HandleDebugWaterWalk),                  "Sets waterwal move for unit" },
        {"debug hover",                   "d", wrap(&ChatHandler::HandleDebugHover),                      "Toggles hover move on/off for unit" },
        {"debug state",                   "d", wrap(&ChatHandler::HandleDebugState),                      "Display MovementFlags for unit" },
        {"debug swim",                    "d", wrap(&ChatHandler::HandleDebugSwim),                       "Toggles swim move for unit" },
        {"debug fly",                     "d", wrap(&ChatHandler::HandleDebugFly),                        "Toggles fly move for unit" },
        {"debug disablegravity",          "d", wrap(&ChatHandler::HandleDebugDisableGravity),             "Toggles disablegravitiy move for unit" },
        {"debug featherfall",             "d", wrap(&ChatHandler::HandleDebugFeatherFall),                "Toggles featherfall move for unit" },
        {"debug speed",                   "d", wrap(&ChatHandler::HandleDebugSpeed),                      "Sets move speed for unit. Usage: .debug speed <value>" },
        {"debug castspell",               "d", wrap(&ChatHandler::HandleCastSpellCommand),                "Casts spell on target." },
        {"debug castself",                "d", wrap(&ChatHandler::HandleCastSelfCommand),                 "Target casts spell <spellId> on itself." },
        {"debug castspellne",             "d", wrap(&ChatHandler::HandleCastSpellNECommand),              "Casts spell by spellid on target (only plays animations)" },
        {"debug aggrorange",              "d", wrap(&ChatHandler::HandleAggroRangeCommand),               "Shows aggro Range of the selected Creature." },
        {"debug knockback",               "d", wrap(&ChatHandler::HandleKnockBackCommand),                "Knocks you back by <value>." },
        {"debug fade",                    "d", wrap(&ChatHandler::HandleFadeCommand),                     "Calls ModThreatModifyer() with <value>." },
        {"debug threatMod",               "d", wrap(&ChatHandler::HandleThreatModCommand),                "Calls ModGeneratedThreatModifyer() with <value>." },
        {"debug movefall",                "d", wrap(&ChatHandler::HandleMoveFallCommand),                 "Makes the creature fall to the ground" },
        {"debug threatList",              "d", wrap(&ChatHandler::HandleThreatListCommand),               "Returns all AI_Targets of the selected Creature." },
        {"debug gettptime",               "d", wrap(&ChatHandler::HandleGetTransporterTime),              "Grabs transporter travel time" },
        {"debug dumpcoords",              "d", wrap(&ChatHandler::HandleDebugDumpCoordsCommmand),         "" },
        {"debug rangecheck",              "d", wrap(&ChatHandler::HandleRangeCheckCommand),               "Checks the range between the player and the target." },
        {"debug testlos",                 "d", wrap(&ChatHandler::HandleCollisionTestLOS),                "Tests LoS" },
        {"debug testindoor",              "d", wrap(&ChatHandler::HandleCollisionTestIndoor),             "Tests indoor" },
        {"debug getheight",               "d", wrap(&ChatHandler::HandleCollisionGetHeight),              "Gets height" },
        {"debug deathstate",              "d", wrap(&ChatHandler::HandleGetDeathState),                   "Returns current deathstate for target" },
        {"debug sendfailed",              "d", wrap(&ChatHandler::HandleSendCastFailed),                  "Sends failed cast result <x>" },
        {"debug playmovie",               "d", wrap(&ChatHandler::HandlePlayMovie),                       "Triggers a movie for selected player" },
        {"debug auraupdate",              "d", wrap(&ChatHandler::HandleAuraUpdateAdd),                   "<SpellID> <Flags> <StackCount>" },
        {"debug auraremove",              "d", wrap(&ChatHandler::HandleAuraUpdateRemove),                "Remove Auras in visual slot" },
        {"debug spawnwar",                "d", wrap(&ChatHandler::HandleDebugSpawnWarCommand),            "Spawns desired amount of npcs to fight with eachother" },
        {"debug updateworldstate",        "d", wrap(&ChatHandler::HandleUpdateWorldStateCommand),         "Sets the worldstate field to the specified value" },
        {"debug initworldstates",         "d", wrap(&ChatHandler::HandleInitWorldStatesCommand),          "(Re)initializes the worldstates." },
        {"debug clearworldstates",        "d", wrap(&ChatHandler::HandleClearWorldStatesCommand),         "Clears the worldstates" },
        {"debug pvpcredit",               "m", wrap(&ChatHandler::HandleDebugPVPCreditCommand),           "Sends PVP credit packet, with specified rank and points" },
        {"debug calcdist",                "d", wrap(&ChatHandler::HandleSimpleDistanceCommand),           "Displays distance between your position and x y z" },
        {"debug setunitbyte",             "d", wrap(&ChatHandler::HandleDebugSetUnitByteCommand),         "Set value z for unit byte x with offset y." },
        {"debug setplayerflags",          "d", wrap(&ChatHandler::HandleDebugSetPlayerFlagsCommand),      "Add player flags x to selected player" },
        {"debug getplayerflags",          "d", wrap(&ChatHandler::HandleDebugGetPlayerFlagsCommand),      "Display current player flags of selected player x" },
        {"debug setweather",              "d", wrap(&ChatHandler::HandleDebugSetWeatherCommand),          "Change zone weather <type> <densitiy>" },

        {"dismount",                      "h", wrap(&ChatHandler::HandleDismountCommand),                "Dismounts targeted unit." },

        {"event",                         "0" },
        {"event list",                    "m", wrap(&ChatHandler::HandleEventListEvents),               "Shows list of currently active events" },
        {"event start",                   "m", wrap(&ChatHandler::HandleEventStartEvent),               "Force start an event" },
        {"event stop",                    "m", wrap(&ChatHandler::HandleEventStopEvent),                "Force stop an event" },
        {"event reset",                   "m", wrap(&ChatHandler::HandleEventResetEvent),               "Resets force flags for an event" },
        {"event reload",                  "a", wrap(&ChatHandler::HandleEventReloadAllEvents),          "Reloads all events from the database" },

        {"gm",                            "0" },
        {"gm active",                     "t", wrap(&ChatHandler::HandleGMActiveCommand),               "Activate/Deactivate <GM> tag" },
        {"gm allowwhispers",              "c", wrap(&ChatHandler::HandleGMAllowWhispersCommand),        "Allows whispers from player <s>." },
        {"gm announce",                   "u", wrap(&ChatHandler::HandleGMAnnounceCommand),             "Sends announce to all online GMs" },
        {"gm blockwhispers",              "c", wrap(&ChatHandler::HandleGMBlockWhispersCommand),        "Blocks whispers from player <s>." },
        {"gm devtag",                     "1", wrap(&ChatHandler::HandleGMDevTagCommand),               "Activate/Deactivate <DEV> tag" },
        {"gm list",                       "0", wrap(&ChatHandler::HandleGMListCommand),                 "Shows active GM's" },
        {"gm logcomment",                 "1", wrap(&ChatHandler::HandleGMLogCommentCommand),           "Adds a comment to the GM log." },

        {"gmTicket",                      "0" },
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        {"gmTicket get",                  "c", wrap(&ChatHandler::HandleGMTicketListCommand),           "Gets GM Ticket list." },
        {"gmTicket getId",                "c", wrap(&ChatHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket by player name." },
        {"gmTicket delId",                "c", wrap(&ChatHandler::HandleGMTicketRemoveByIdCommand),     "Deletes GM Ticket by player name." },
#else
        {"gmTicket list",                 "c", wrap(&ChatHandler::HandleGMTicketListCommand),           "Lists all active GM Tickets." },
        {"gmTicket get",                  "c", wrap(&ChatHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket with ID x." },
        {"gmTicket remove",               "c", wrap(&ChatHandler::HandleGMTicketRemoveByIdCommand),     "Removes GM Ticket with ID x." },
        {"gmTicket deletepermanent",      "z", wrap(&ChatHandler::HandleGMTicketDeletePermanentCommand),"Deletes GM Ticket with ID x permanently." },
        {"gmTicket assign",               "c", wrap(&ChatHandler::HandleGMTicketAssignToCommand),       "Assigns GM Ticket with id x to GM y." },
        {"gmTicket release",              "c", wrap(&ChatHandler::HandleGMTicketReleaseCommand),        "Releases assigned GM Ticket with ID x." },
        {"gmTicket comment",              "c", wrap(&ChatHandler::HandleGMTicketCommentCommand),        "Sets comment x to GM Ticket with ID y." },
#endif
        {"gmTicket toggle",               "z", wrap(&ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand), "Toggles the ticket system status." },

        {"gobject",                       "0" },
        {"gobject damage",                "o", wrap(&ChatHandler::HandleGODamageCommand),               "Damages the GO for the specified hitpoints" },
        {"gobject delete",                "o", wrap(&ChatHandler::HandleGODeleteCommand),               "Deletes selected GameObject" },
        {"gobject enable",                "o", wrap(&ChatHandler::HandleGOEnableCommand),               "Enables the selected GO for use." },
        {"gobject export",                "o", wrap(&ChatHandler::HandleGOExportCommand),               "Exports the selected GO to .sql file" },
        {"gobject info",                  "o", wrap(&ChatHandler::HandleGOInfoCommand),                 "Gives you information about selected GO" },
        {"gobject movehere",              "g", wrap(&ChatHandler::HandleGOMoveHereCommand),             "Moves gameobject to your position" },
        {"gobject open",                  "o", wrap(&ChatHandler::HandleGOOpenCommand),                 "Toggles open/close (state) of selected GO." },
        {"gobject rebuild",               "o", wrap(&ChatHandler::HandleGORebuildCommand),              "Rebuilds the GO." },
        {"gobject rotate",                "g", wrap(&ChatHandler::HandleGORotateCommand),               "Rotates the object. <Axis> x,y, Default o." },
        {"gobject select",                "o", wrap(&ChatHandler::HandleGOSelectCommand),               "Selects the nearest GameObject to you" },
        {"gobject selectguid",            "o", wrap(&ChatHandler::HandleGOSelectGuidCommand),           "Selects GO with <guid>" },

        {"gobject set",                   "o" },
        {"gobject set animprogress",      "o", wrap(&ChatHandler::HandleGOSetAnimProgressCommand),      "Sets anim progress of selected GO" },
        {"gobject set faction",           "o", wrap(&ChatHandler::HandleGOSetFactionCommand),           "Sets the faction of the GO" },
        {"gobject set flags",             "o", wrap(&ChatHandler::HandleGOSetFlagsCommand),             "Sets the flags of the GO" },
        {"gobject set overrides",         "o", wrap(&ChatHandler::HandleGOSetOverridesCommand),         "Sets override of selected GO" },
        {"gobject set phase",             "o", wrap(&ChatHandler::HandleGOSetPhaseCommand),             "Sets phase of selected GO" },
        {"gobject set scale",             "o", wrap(&ChatHandler::HandleGOSetScaleCommand),             "Sets scale of selected GO" },
        {"gobject set state",             "o", wrap(&ChatHandler::HandleGOSetStateCommand),             "Sets the state byte of the GO" },
        {"gobject spawn",                 "o", wrap(&ChatHandler::HandleGOSpawnCommand),               "Spawns a GameObject by ID" },

        {"gocreature",                    "v", wrap(&ChatHandler::HandleGoCreatureSpawnCommand),        "Teleports you to the creature with <spwn_id>." },
        {"gogameobject",                  "v", wrap(&ChatHandler::HandleGoGameObjectSpawnCommand),      "Teleports you to the gameobject with <spawn_id>." },
        {"gostartlocation",               "m", wrap(&ChatHandler::HandleGoStartLocationCommand),        "Teleports you to a starting location" },
        {"gotrig",                        "v", wrap(&ChatHandler::HandleGoTriggerCommand),              "Teleports you to the areatrigger with <id>." },
        {"gps",                           "0", wrap(&ChatHandler::HandleGPSCommand),                    "Shows position of targeted unit" },

        {"guild",                         "0" },
        {"guild create",                  "m", wrap(&ChatHandler::HandleGuildCreateCommand),            "Creates a guild." },
        {"guild disband",                 "m", wrap(&ChatHandler::HandleGuildDisbandCommand),           "Disbands the guild of your target." },
#if VERSION_STRING >= Cata
        {"guild info",                    "m", wrap(&ChatHandler::HandleGuildInfoCommand),              "Shows guild info of your target." },
#endif
        {"guild join",                    "m", wrap(&ChatHandler::HandleGuildJoinCommand),              "Force selected player to join a guild by name" },
        {"guild listmembers",             "m", wrap(&ChatHandler::HandleGuildListMembersCommand),       "Lists guildmembers with ranks by guild name." },
        {"guild rename",                  "m", wrap(&ChatHandler::HandleRenameGuildCommand),            "Renames a guild." },
        {"guild removeplayer",            "m", wrap(&ChatHandler::HandleGuildRemovePlayerCommand),      "Removes a player from a guild." },

        {"help",                          "0", wrap(&ChatHandler::handleHelpCommand),                   "Shows help for command" },

        {"instance",                      "0" },
        {"instance create",               "z", wrap(&ChatHandler::HandleCreateInstanceCommand),         "Creates instance by mapid x y z" },
        {"instance countcreature",        "z", wrap(&ChatHandler::HandleCountCreaturesCommand),         "Returns number of creatures with entry x" },
        {"instance exit",                 "m", wrap(&ChatHandler::HandleExitInstanceCommand),           "Exits current instance, return to entry point." },
        {"instance info",                 "m", wrap(&ChatHandler::HandleGetInstanceInfoCommand),        "Gets info about instance with ID x (default current instance)." },
        {"instance reset",                "z", wrap(&ChatHandler::HandleResetInstanceCommand),          "Removes instance ID x from target player." },
        {"instance resetall",             "m", wrap(&ChatHandler::HandleResetAllInstancesCommand),      "Removes all instance IDs from target player." },
        {"instance shutdown",             "z", wrap(&ChatHandler::HandleShutdownInstanceCommand),       "Shutdown instance with ID x (default is current instance)." },
        {"instance showtimers",           "m", wrap(&ChatHandler::HandleShowTimersCommand),             "Show timers for current instance." },

        {"invincible",                    "j", wrap(&ChatHandler::HandleInvincibleCommand),             "Toggles invincibility on/off" },
        {"invisible",                     "i", wrap(&ChatHandler::HandleInvisibleCommand),              "Toggles invisibility and invincibility on/off" },

        {"kick",                          "0" },
        {"kick player",                   "f", wrap(&ChatHandler::HandleKickByNameCommand),             "Disconnects the player with name <s>." },
        {"kick account",                  "f", wrap(&ChatHandler::HandleKKickBySessionCommand),         "Disconnects the session with account name <s>." },
        {"kick ip",                       "f", wrap(&ChatHandler::HandleKickByIPCommand),               "Disconnects the session with the ip <s>." },
        {"kill",                          "r", wrap(&ChatHandler::HandleKillCommand),                   "Kills selected unit or player by name" },

        {"lookup",                        "0" },
        {"lookup achievement",            "l", wrap(&ChatHandler::HandleLookupAchievementCommand),      "Looks up achievement string x." },
        {"lookup creature",               "l", wrap(&ChatHandler::HandleLookupCreatureCommand),         "Looks up creature string x." },
        {"lookup faction",                "l", wrap(&ChatHandler::HandleLookupFactionCommand),          "Looks up faction string x." },
        {"lookup item",                   "l", wrap(&ChatHandler::HandleLookupItemCommand),             "Looks up item string x." },
        {"lookup object",                 "l", wrap(&ChatHandler::HandleLookupObjectCommand),           "Looks up gameobject string x." },
        {"lookup quest",                  "l", wrap(&ChatHandler::HandleLookupQuestCommand),            "Looks up quest string x." },
        {"lookup spell",                  "l", wrap(&ChatHandler::HandleLookupSpellCommand),            "Looks up spell string x." },
        {"lookup skill",                  "l", wrap(&ChatHandler::HandleLookupSkillCommand),            "Looks up skill string x." },

        {"modify",                        "0" },
        {"modify hp",                     "m", wrap(&ChatHandler::HandleModifyHp),                      "Mods health points (HP) of selected target" },
        {"modify mana",                   "m", wrap(&ChatHandler::HandleModifyMana),                    "Mods mana points (MP) of selected target." },
        {"modify rage",                   "m", wrap(&ChatHandler::HandleModifyRage),                    "Mods rage points of selected target." },
        {"modify energy",                 "m", wrap(&ChatHandler::HandleModifyEnergy),                  "Mods energy points of selected target." },
#if VERSION_STRING >= WotLK
        {"modify runicpower",             "m", wrap(&ChatHandler::HandleModifyRunicpower),              "Mods runic power points of selected target." },
#endif
        {"modify strength",               "m", wrap(&ChatHandler::HandleModifyStrength),                "Mods strength value of the selected target." },
        {"modify agility",                "m", wrap(&ChatHandler::HandleModifyAgility),                 "Mods agility value of the selected target." },
        {"modify intelligence",           "m", wrap(&ChatHandler::HandleModifyIntelligence),            "Mods intelligence value of the selected target." },
        {"modify spirit",                 "m", wrap(&ChatHandler::HandleModifySpirit),                  "Mods spirit value of the selected target." },
        {"modify armor",                  "m", wrap(&ChatHandler::HandleModifyArmor),                   "Mods armor of selected target." },
        {"modify holy",                   "m", wrap(&ChatHandler::HandleModifyHoly),                    "Mods holy resistance of selected target." },
        {"modify fire",                   "m", wrap(&ChatHandler::HandleModifyFire),                    "Mods fire resistance of selected target." },
        {"modify nature",                 "m", wrap(&ChatHandler::HandleModifyNature),                  "Mods nature resistance of selected target." },
        {"modify frost",                  "m", wrap(&ChatHandler::HandleModifyFrost),                   "Mods frost resistance of selected target." },
        {"modify shadow",                 "m", wrap(&ChatHandler::HandleModifyShadow),                  "Mods shadow resistance of selected target." },
        {"modify arcane",                 "m", wrap(&ChatHandler::HandleModifyArcane),                  "Mods arcane resistance of selected target." },
        {"modify damage",                 "m", wrap(&ChatHandler::HandleModifyDamage),                  "Mods damage done by the selected target." },
        {"modify ap",                     "m", wrap(&ChatHandler::HandleModifyAp),                      "Mods attack power of the selected target." },
        {"modify rangeap",                "m", wrap(&ChatHandler::HandleModifyRangeap),                 "Mods range attack power of the selected target." },
        {"modify scale",                  "m", wrap(&ChatHandler::HandleModifyScale),                   "Mods scale of the selected target." },
        {"modify nativedisplayid",        "m", wrap(&ChatHandler::HandleModifyNativedisplayid),         "Mods native display identifier of the target." },
        {"modify displayid",              "m", wrap(&ChatHandler::HandleModifyDisplayid),               "Mods display identifier (DisplayID) of the target." },
        {"modify flags",                  "m", wrap(&ChatHandler::HandleModifyFlags),                   "Mods flags of the selected target." },
        {"modify faction",                "m", wrap(&ChatHandler::HandleModifyFaction),                 "Mods faction template of the selected target." },
        {"modify dynamicflags",           "m", wrap(&ChatHandler::HandleModifyDynamicflags),            "Mods dynamic flags of the selected target." },
#if VERSION_STRING < Cata
        {"modify happiness",              "m", wrap(&ChatHandler::HandleModifyHappiness),               "Mods happiness value of the selected target." },
#endif
        {"modify boundingradius",         "m", wrap(&ChatHandler::HandleModifyBoundingradius),          "Mods bounding radius of the selected target." },
        {"modify combatreach",            "m", wrap(&ChatHandler::HandleModifyCombatreach),             "Mods combat reach of the selected target." },
        {"modify emotestate",             "m", wrap(&ChatHandler::HandleModifyEmotestate),              "Mods Unit emote state of the selected target." },
        {"modify bytes0",                 "m", wrap(&ChatHandler::HandleModifyBytes0),                  "Mods bytes0 entry of selected target." },
        {"modify bytes1",                 "m", wrap(&ChatHandler::HandleModifyBytes1),                  "Mods bytes1 entry of selected target." },
        {"modify bytes2",                 "m", wrap(&ChatHandler::HandleModifyBytes2),                  "Mods bytes2 entry of selected target." },

        {"mount",                         "m", wrap(&ChatHandler::HandleMountCommand),                  "Mounts targeted unit with modelid x." },

        {"npc",                           "0" },
        {"npc addagent",                  "n", wrap(&ChatHandler::HandleNpcAddAgentCommand),            "Add ai agents to npc." },
        {"npc addtrainerspell",           "m", wrap(&ChatHandler::HandleNpcAddTrainerSpellCommand),     "Add spells to trainer learn list." },
        {"npc appear",                    "n", wrap(&ChatHandler::HandleNpcAppearCommand),              "Teleports you to the target NPC's location." },
        {"npc cast",                      "n", wrap(&ChatHandler::HandleNpcCastCommand),                "Makes NPC cast <spellid>." },
        {"npc come",                      "n", wrap(&ChatHandler::HandleNpcComeCommand),                "Makes NPC move to your position" },
        {"npc delete",                    "n", wrap(&ChatHandler::HandleNpcDeleteCommand),              "Deletes mob from world optional from DB" },
        {"npc info",                      "n", wrap(&ChatHandler::HandleNpcInfoCommand),                "Displays NPC information" },
        {"npc listAgent",                 "n", wrap(&ChatHandler::HandleNpcListAIAgentCommand),         "List AIAgents of selected target." },
        {"npc listloot",                  "m", wrap(&ChatHandler::HandleNpcListLootCommand),            "Displays possible loot for the selected NPC." },
        {"npc follow",                    "m", wrap(&ChatHandler::HandleNpcFollowCommand),              "Sets NPC to follow you" },
        {"npc stopfollow",                "m", wrap(&ChatHandler::HandleNpcStopFollowCommand),          "Sets NPC to not follow anything" },
        {"npc possess",                   "n", wrap(&ChatHandler::HandlePossessCommand),                "Possess targeted NPC (mind control)" },
        {"npc unpossess",                 "n", wrap(&ChatHandler::HandleUnPossessCommand),              "Unpossess any currently possessed npc." },
        {"npc return",                    "n", wrap(&ChatHandler::HandleNpcReturnCommand),              "Returns NPC to spawnpoint." },
        {"npc respawn",                   "n", wrap(&ChatHandler::HandleNpcRespawnCommand),             "Respawns a dead NPC from its corpse." },
        {"npc say",                       "n", wrap(&ChatHandler::HandleNpcSayCommand),                 "Makes selected NPC say <text>." },
        {"npc select",                    "n", wrap(&ChatHandler::HandleNpcSelectCommand),              "Selects closest NPC" },

        {"npc set",                       "0" },
        {"npc set canfly",                "n", wrap(&ChatHandler::HandleNpcSetCanFlyCommand),           "Toggles CanFly state" },
        {"npc set emote",                 "n", wrap(&ChatHandler::HandleNpcSetEmoteCommand),            "Sets emote state" },
        {"npc set equip",                 "m", wrap(&ChatHandler::HandleNpcSetEquipCommand),            "Sets equipment itemt" },
        {"npc set flags",                 "n", wrap(&ChatHandler::HandleNpcSetFlagsCommand),            "Sets NPC flags" },
        {"npc set formationmaster",       "m", wrap(&ChatHandler::HandleNpcSetFormationMasterCommand),  "Sets formation master." },
        {"npc set formationslave",        "m", wrap(&ChatHandler::HandleNpcSetFormationSlaveCommand),   "Sets formation slave with distance and angle" },
        {"npc set formationclear",        "m", wrap(&ChatHandler::HandleNpcSetFormationClearCommand),   "Removes formation from creature" },
        {"npc set phase",                 "n", wrap(&ChatHandler::HandleNpcSetPhaseCommand),            "Sets phase for selected creature" },
        {"npc set standstate",            "m", wrap(&ChatHandler::HandleNpcSetStandstateCommand),       "Sets standstate for selected creature" },
        {"npc set entry",                 "m", wrap(&ChatHandler::HandleNpcChangeEntry),                "Sets a New Entry for selected creature" },

        {"npc spawn",                     "n", wrap(&ChatHandler::HandleNpcSpawnCommand),               "Spawns NPC of entry <id>" },
        {"npc showtimers",                "m", wrap(&ChatHandler::HandleNpcShowTimersCommand),          "Shows timers for selected creature" },
        {"npc vendoradditem",             "n", wrap(&ChatHandler::HandleNpcVendorAddItemCommand),       "Adds item to vendor" },
        {"npc vendorremoveitem",          "n", wrap(&ChatHandler::HandleNpcVendorRemoveItemCommand),    "Removes item from vendor." },
        {"npc yell",                      "n", wrap(&ChatHandler::HandleNpcYellCommand),                "Makes selected NPC yell <text>." },

        {"pet",                           "0" },
        {"pet create",                    "m", wrap(&ChatHandler::HandlePetCreateCommand),              "Creates a pet with <entry>." },
        {"pet dismiss",                   "m", wrap(&ChatHandler::HandlePetDismissCommand),             "Dismisses a pet by for selected player or selected pet." },
        {"pet rename",                    "m", wrap(&ChatHandler::HandlePetRenameCommand),              "Renames a pet to <name>." },
        {"pet addspell",                  "m", wrap(&ChatHandler::HandlePetAddSpellCommand),            "Teaches pet <spell>." },
        {"pet removespell",               "m", wrap(&ChatHandler::HandlePetRemoveSpellCommand),         "Removes pet spell <spell>." },
        {"pet setlevel",                  "m", wrap(&ChatHandler::HandlePetSetLevelCommand),            "Sets pet level to <level>." },

        {"playerinfo",                    "m", wrap(&ChatHandler::HandlePlayerInfo),                    "Displays info for selected character or <charname>" },

        {"quest",                         "0" },
        {"quest addboth",                 "2", wrap(&ChatHandler::HandleQuestAddBothCommand),           "Add quest <id> to the targeted NPC as start & finish" },
        {"quest addfinish",               "2", wrap(&ChatHandler::HandleQuestAddFinishCommand),         "Add quest <id> to the targeted NPC as finisher" },
        {"quest addstart",                "2", wrap(&ChatHandler::HandleQuestAddStartCommand),          "Add quest <id> to the targeted NPC as starter" },
        {"quest delboth",                 "2", wrap(&ChatHandler::HandleQuestDelBothCommand),           "Delete quest <id> from the targeted NPC as start & finish" },
        {"quest delfinish",               "2", wrap(&ChatHandler::HandleQuestDelFinishCommand),         "Delete quest <id> from the targeted NPC as finisher" },
        {"quest delstart",                "2", wrap(&ChatHandler::HandleQuestDelStartCommand),          "Delete quest <id> from the targeted NPC as starter" },
        {"quest complete",                "2", wrap(&ChatHandler::HandleQuestFinishCommand),            "Complete/Finish quest <id>" },
        {"quest fail",                    "2", wrap(&ChatHandler::HandleQuestFailCommand),              "Fail quest <id>" },
        {"quest finisher",                "2", wrap(&ChatHandler::HandleQuestFinisherCommand),          "Lookup quest finisher for quest <id>" },
        {"quest item",                    "2", wrap(&ChatHandler::HandleQuestItemCommand),              "Lookup itemid necessary for quest <id>" },
        {"quest list",                    "2", wrap(&ChatHandler::HandleQuestListCommand),              "Lists the quests for the npc <id>" },
        {"quest load",                    "2", wrap(&ChatHandler::HandleQuestLoadCommand),              "Loads quests from database" },
        {"quest giver",                   "2", wrap(&ChatHandler::HandleQuestGiverCommand),             "Lookup quest giver for quest <id>" },
        {"quest remove",                  "2", wrap(&ChatHandler::HandleQuestRemoveCommand),            "Removes the quest <id> from the targeted player" },
        {"quest reward",                  "2", wrap(&ChatHandler::HandleQuestRewardCommand),            "Shows reward for quest <id>" },
        {"quest status",                  "2", wrap(&ChatHandler::HandleQuestStatusCommand),            "Lists the status of quest <id>" },
        {"quest start",                   "2", wrap(&ChatHandler::HandleQuestStartCommand),             "Starts quest <id>" },
        {"quest startspawn",              "2", wrap(&ChatHandler::HandleQuestStarterSpawnCommand),      "Port to spawn location for quest <id> (starter)" },
        {"quest finishspawn",             "2", wrap(&ChatHandler::HandleQuestFinisherSpawnCommand),     "Port to spawn location for quest <id> (finisher)" },

        {"recall",                        "0" },
        {"recall list",                   "q", wrap(&ChatHandler::HandleRecallListCommand),             "List recall locations" },
        {"recall add",                    "q", wrap(&ChatHandler::HandleRecallAddCommand),              "Add a recall location" },
        {"recall del",                    "q", wrap(&ChatHandler::HandleRecallDelCommand),              "Remove a recall location" },
        {"recall port",                   "q", wrap(&ChatHandler::HandleRecallGoCommand),               "Ports you to recalled location" },
        {"recall portplayer",             "m", wrap(&ChatHandler::HandleRecallPortPlayerCommand),       "Ports specified player to a recalled location" },
        {"recall portus",                 "m", wrap(&ChatHandler::HandleRecallPortUsCommand),           "Ports you and the selected player to recalled location" },

        {"revive",                        "r", wrap(&ChatHandler::HandleReviveCommand),                 "Revives you or a selected target or player by name" },
        {"root",                          "b", wrap(&ChatHandler::HandleRootCommand),                   "Roots selected target." },

        {"server",                        "0" },
        {"server info",                   "0", wrap(&ChatHandler::HandleServerInfoCommand),             "Shows detailed Server info." },
        {"server rehash",                 "z", wrap(&ChatHandler::HandleServerRehashCommand),           "Reloads config file." },
        {"server save",                   "s", wrap(&ChatHandler::HandleServerSaveCommand),             "Save targeted or named player." },
        {"server saveall",                "s", wrap(&ChatHandler::HandleServerSaveAllCommand),          "Save all online player." },
        {"server setmotd",                "m", wrap(&ChatHandler::HandleServerSetMotdCommand),          "Sets server MessageOfTheDay." },
        {"server shutdown",               "z", wrap(&ChatHandler::HandleServerShutdownCommand),         "Initiates server shutdown in <x> seconds." },
        {"server cancelshutdown",         "z", wrap(&ChatHandler::HandleServerCancelShutdownCommand),   "Cancels a Server Restart/Shutdown." },
        {"server restart",                "z", wrap(&ChatHandler::HandleServerRestartCommand),          "Initiates server restart in <x> seconds." },

        {"server reloadtable",                      "m" },
        {"server reloadtable gameobjects",          "z", wrap(&ChatHandler::HandleReloadGameobjectsCommand),       "Reload gameobjets" },
        {"server reloadtable creatures",            "z", wrap(&ChatHandler::HandleReloadCreaturesCommand),         "Reload creatures" },
        {"server reloadtable areatriggers",         "z", wrap(&ChatHandler::HandleReloadAreaTriggersCommand),      "Reload areatriggers table" },
        {"server reloadtable command_overrides",    "z", wrap(&ChatHandler::HandleReloadCommandOverridesCommand),  "Reload command_overrides table" },
        {"server reloadtable fishing",              "z", wrap(&ChatHandler::HandleReloadFishingCommand),           "Reload fishing table" },
        {"server reloadtable gossip_menu_option",   "z", wrap(&ChatHandler::HandleReloadGossipMenuOptionCommand),  "Reload gossip_menu_option table" },
        {"server reloadtable graveyards",           "z", wrap(&ChatHandler::HandleReloadGraveyardsCommand),        "Reload graveyards table" },
        {"server reloadtable items",                "z", wrap(&ChatHandler::HandleReloadItemsCommand),             "Reload items table" },
        {"server reloadtable itempages",            "z", wrap(&ChatHandler::HandleReloadItempagesCommand),         "Reload itempages table" },
        {"server reloadtable npc_script_text",      "z", wrap(&ChatHandler::HandleReloadNpcScriptTextCommand),     "Reload npc_script_text table" },
        {"server reloadtable npc_gossip_text",      "z", wrap(&ChatHandler::HandleReloadNpcTextCommand),           "Reload npc_gossip_text table" },
        {"server reloadtable pet_level_abilities",  "z", wrap(&ChatHandler::HandleReloadPetLevelAbilitiesCommand), "Reload pet_level_abilities table" },
        {"server reloadtable player_xp_for_level",  "z", wrap(&ChatHandler::HandleReloadPlayerXpForLevelCommand),  "Reload player_xp_for_level table" },
        {"server reloadtable points_of_interest",   "z", wrap(&ChatHandler::HandleReloadPointsOfInterestCommand),  "Reload points_of_interest table" },
        {"server reloadtable quests",               "z", wrap(&ChatHandler::HandleReloadQuestsCommand),            "Reload quests table" },
        {"server reloadtable spell_teleport_coords","z", wrap(&ChatHandler::HandleReloadTeleportCoordsCommand),    "Reload teleport_coords table" },
        {"server reloadtable worldbroadcast",       "z", wrap(&ChatHandler::HandleReloadWorldbroadcastCommand),    "Reload worldbroadcast table" },
        {"server reloadtable worldmap_info",        "z", wrap(&ChatHandler::HandleReloadWorldmapInfoCommand),      "Reload worldmap_info table" },
        {"server reloadtable worldstring_tables",   "z", wrap(&ChatHandler::HandleReloadWorldstringTablesCommand), "Reload worldstring_tables table" },
        {"server reloadtable zoneguards",           "z", wrap(&ChatHandler::HandleReloadZoneguardsCommand),        "Reload zoneguards table" },

        {"server reloadscript",           "m", wrap(&ChatHandler::HandleServerReloadScriptsCommand),    "" },

        {"summon",                        "v", wrap(&ChatHandler::HandleSummonCommand),                 "Summons x to your position." },

        {"ticket",                        "0" },
        {"ticket list",                   "c", wrap(&ChatHandler::HandleTicketListCommand),             "Shows all active tickets" },
        {"ticket listall",                "c", wrap(&ChatHandler::HandleTicketListAllCommand),          "Shows all tickets in the database" },
        {"ticket get",                    "c", wrap(&ChatHandler::HandleTicketGetCommand),              "Returns the content of the specified ID" },
        {"ticket close",                  "c", wrap(&ChatHandler::HandleTicketCloseCommand),            "Close ticket with specified ID" },
        {"ticket delete",                 "a", wrap(&ChatHandler::HandleTicketDeleteCommand),           "Delete ticket by specified ID" },

        {"transport",                     "m" },
        {"transport info",                "m", wrap(&ChatHandler::HandleGetTransporterInfo),            "Displays the current transport info" },
        {"transport spawn",               "m", wrap(&ChatHandler::HandleSpawnInstanceTransport),        "Spawns transport with entry/period in current instance" },
        {"transport start",               "m", wrap(&ChatHandler::HandleStartTransport),                "Force starts the current transport" },
        {"transport stop",                "m", wrap(&ChatHandler::HandleStopTransport),                 "Force stops the current transport" },
        {"transport getperiod",           "m", wrap(&ChatHandler::HandleGetTransporterTime),            "Displays the current transport period in ms" },

        {"unban",                         "0" },
        {"unban ip",                      "m", wrap(&ChatHandler::HandleIPUnBanCommand),                "Deletes an address from the IP ban table: <address>" },
        {"unban character",               "b", wrap(&ChatHandler::HandleUnBanCharacterCommand),         "Unbans character x" },
        {"unroot",                        "b", wrap(&ChatHandler::HandleUnrootCommand),                 "Unroots selected target." },

        {"vehicle",                       "m" },
#ifdef FT_VEHICLES
        {"vehicle ejectpassenger",        "m", wrap(&ChatHandler::HandleVehicleEjectPassengerCommand),     "Ejects the passenger from the specified seat" },
        {"vehicle ejectallpassengers",    "m", wrap(&ChatHandler::HandleVehicleEjectAllPassengersCommand), "Ejects all passengers from the vehicle" },
        {"vehicle installaccessories",    "m", wrap(&ChatHandler::HandleVehicleInstallAccessoriesCommand), "Installs the accessories for the selected vehicle" },
        {"vehicle addpassenger",          "m", wrap(&ChatHandler::HandleVehicleAddPassengerCommand),       "Adds a new NPC passenger to the vehicle" },
#endif

        {"wannounce",                     "u", wrap(&ChatHandler::HandleWAnnounceCommand),              "Sends a widescreen announcement to all players." },

        {"waypoint",                      "0" },
        {"waypoint add",                  "w", wrap(&ChatHandler::HandleWayPointAddCommand),           "Add wp for selected creature at current pos." },
        {"waypoint delete",               "w", wrap(&ChatHandler::HandleWayPointDeleteCommand),        "Deletes selected wp." },
        {"waypoint deleteall",            "w", wrap(&ChatHandler::HandleWayPointDeleteAllCommand),     "Deletes all waypoints of selected creature." },
        {"waypoint hide",                 "w", wrap(&ChatHandler::HandleWayPointHideCommand),          "Hide wp's for selected creature." },
        {"waypoint show",                 "w", wrap(&ChatHandler::HandleWayPointShowCommand),          "Show wp's for selected creature <bool backwards>" },

        {"worldport",                     "v", wrap(&ChatHandler::HandleWorldPortCommand),             "Teleports you to a location with mapid x y z" }
    };
}

CommandTableStorage::~CommandTableStorage()
{}

CommandTableStorage& CommandTableStorage::getInstance()
{
    static CommandTableStorage mInstance;
    return mInstance;
}

void CommandTableStorage::loadOverridePermission()
{
    auto result = CharacterDatabase.Query("SELECT command_name, access_level FROM command_overrides");
    if (!result)
        return;

    do
    {
        const char* command = result->Fetch()[0].asCString();
        const char* level = result->Fetch()[1].asCString();
        overridePermission(command, level);
    } while (result->NextRow());
}

void CommandTableStorage::overridePermission(const char* command, const char* level)
{
    if (!command || !*command || !level || !*level)
        return;

    std::string strCommand = command;
    
    AscEmu::Util::Strings::toLowerCase(strCommand);

    bool exists = false;

    for (auto& cmd : m_commandRegistry)
    {
        if (cmd.command == strCommand)
        {
            cmd.commandPermission = level;
            exists = true;
        }
    }

    if (exists)
        sLogger.debug("Changing command level of .`{}` to {}.", strCommand, level);
}
