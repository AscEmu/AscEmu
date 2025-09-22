/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CommandTableStorage.hpp"

#include <sstream>

#include "ChatCommand.hpp"
#include "ChatCommandHandler.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Strings.hpp"

CommandTableStorage::CommandTableStorage()
{
    m_commandRegistry = {

        {"account",                       "0", 0 },
        {"account create",                "a", 2, wrap(&ChatCommandHandler::handleAccountCreate),               "Creates an account with name and password" },
        {"account setgm",                 "z", 2, wrap(&ChatCommandHandler::handleAccountSetGMCommand),         "Sets gm level on account. Pass it username and 0,1,2,3,az, etc." },
        {"account mute",                  "a", 2, wrap(&ChatCommandHandler::handleAccountMuteCommand),          "Mutes account for <timeperiod>." },
        {"account unmute",                "a", 2, wrap(&ChatCommandHandler::handleAccountUnmuteCommand),        "Unmutes account <x>" },
        {"account ban",                   "a", 1, wrap(&ChatCommandHandler::handleAccountBannedCommand),        "Bans account: .ban account <name> [duration] [reason]" },
        {"account unban",                 "z", 1, wrap(&ChatCommandHandler::handleAccountUnbanCommand),         "Unbans account x." },
        {"account changepw",              "0", 1, wrap(&ChatCommandHandler::handleAccountChangePassword),       "Change the password of your account." },
        {"account getid",                 "1", 1, wrap(&ChatCommandHandler::handleAccountGetAccountID),         "Get Account ID for account name X" },

        {"achieve",                       "0", 0 },
#if VERSION_STRING > TBC
        {"achieve complete",              "m", 1, wrap(&ChatCommandHandler::handleAchievementCompleteCommand),  "Completes the specified achievement." },
        {"achieve criteria",              "m", 1, wrap(&ChatCommandHandler::handleAchievementCriteriaCommand),  "Completes the specified achievement criteria." },
        {"achieve reset",                 "m", 1, wrap(&ChatCommandHandler::handleAchievementResetCommand),     "Resets achievement data from the target." },
#endif

        {"admin",                         "0", 0 },
        {"admin castall",                 "z", 1, wrap(&ChatCommandHandler::HandleAdminCastAllCommand),         "Makes all players online cast spell <x>." },
        {"admin dispelall",               "z", 1, wrap(&ChatCommandHandler::HandleAdminDispelAllCommand),       "Dispels all negative (or positive w/ 1) auras on all players." },
        {"admin masssummon",              "z", 0, wrap(&ChatCommandHandler::HandleAdminMassSummonCommand),      "Summons all online players to you, use a/h for alliance/horde." },
        {"admin playall",                 "z", 1, wrap(&ChatCommandHandler::HandleAdminPlayGlobalSoundCommand), "Plays a sound to everyone on the realm." },

        {"announce",                      "u", 1, wrap(&ChatCommandHandler::HandleAnnounceCommand),             "Sends a normal chat message to all players." },
        {"appear",                        "v", 1, wrap(&ChatCommandHandler::HandleAppearCommand),               "Teleports to x's position." },

        {"arena",                         "0", 0 },
        {"arena createteam",              "e", 2, wrap(&ChatCommandHandler::HandleArenaCreateTeam),             "Creates arena team with <type> <name>" },
        {"arena setteamleader",           "e", 0, wrap(&ChatCommandHandler::HandleArenaSetTeamLeader),          "Sets the arena team leader for <type>" },
        {"arena resetallratings",         "z", 0, wrap(&ChatCommandHandler::HandleArenaTeamResetAllRatings),    "Resets all arena teams to their default rating" },

        {"ban",                           "0", 0 },
        {"ban ip",                        "m", 1, wrap(&ChatCommandHandler::HandleIPBanCommand),                "Bans IP by <address> [duration]" },
        {"ban character",                 "b", 1, wrap(&ChatCommandHandler::HandleBanCharacterCommand),         "Bans character by <charname> [duration] [reason]" },
        {"ban all",                       "a", 1, wrap(&ChatCommandHandler::HandleBanAllCommand),               "Bans all by <charname> [duration] [reason]" },

        {"battleground",                  "0", 0 },
        {"battleground forceinitqueue",   "z", 0, wrap(&ChatCommandHandler::HandleBGForceInitQueueCommand),     "Forces init of all bgs with in queue." },
        {"battleground getqueue",         "z", 0, wrap(&ChatCommandHandler::HandleBGGetQueueCommand),           "Gets common battleground queue information." },
        {"battleground info",             "e", 0, wrap(&ChatCommandHandler::HandleBGInfoCommand),               "Displays information about current bg." },
        {"battleground leave",            "e", 0, wrap(&ChatCommandHandler::HandleBGLeaveCommand),              "Leaves the current battleground." },
        {"battleground menu",             "e", 1, wrap(&ChatCommandHandler::HandleBGMenuCommand),               "Shows BG Menu for selected player by type <x>" },
        {"battleground pause",            "e", 0, wrap(&ChatCommandHandler::HandleBGPauseCommand),              "Pauses current battleground match." },
        {"battleground playsound",        "e", 1, wrap(&ChatCommandHandler::HandleBGPlaySoundCommand),          "Plays sound to all players in bg <sound_id>" },
        {"battleground sendstatus",       "e", 1, wrap(&ChatCommandHandler::HandleBGSendStatusCommand),         "Sends status of bg by type <x>" },
        {"battleground setscore",         "e", 1, wrap(&ChatCommandHandler::HandleBGSetScoreCommand),           "Sets bg score <Teamid> <Score>." },
        {"battleground setworldstate",    "e", 1, wrap(&ChatCommandHandler::HandleBGSetWorldStateCommand),      "Sets singe worldsate value." },
        {"battleground setworldstates",   "e", 1, wrap(&ChatCommandHandler::HandleBGSetWorldStatesCommand),     "Sets multipe worldstate values for start/end id" },
        {"battleground start",            "e", 0, wrap(&ChatCommandHandler::HandleBGStartCommand),              "Starts current battleground match." },

        {"blockappear",                   "v", 0, wrap(&ChatCommandHandler::HandleBlockAppearCommand),          "Blocks appearance to your position." },
        {"blocksummon",                   "v", 0, wrap(&ChatCommandHandler::HandleBlockSummonCommand),          "Blocks summons to others position." },

        {"character",                     "0", 0 },
        
        {"character add",                 "m", 0 },
        {"character add copper",          "m", 1, wrap(&ChatCommandHandler::HandleCharAddCopperCommand),        "Adds x copper to character." },
        {"character add silver",          "m", 1, wrap(&ChatCommandHandler::HandleCharAddSilverCommand),        "Adds x silver to character." },
        {"character add gold",            "m", 1, wrap(&ChatCommandHandler::HandleCharAddGoldCommand),          "Adds x gold to character." },
        {"character add honorpoints",     "m", 1, wrap(&ChatCommandHandler::HandleCharAddHonorPointsCommand),   "Adds x amount of honor points/currency" },
        {"character add honorkills",      "m", 1, wrap(&ChatCommandHandler::HandleCharAddHonorKillCommand),     "Adds x amount of honor kills" },
        {"character add item",            "m", 1, wrap(&ChatCommandHandler::HandleCharAddItemCommand),          "Adds item x count y" },
        {"character add itemset",         "m", 1, wrap(&ChatCommandHandler::HandleCharAddItemSetCommand),       "Adds item set to inv." },
        
        {"character set",                 "m", 0 },
        {"character set allexplored",     "m", 0, wrap(&ChatCommandHandler::HandleCharSetAllExploredCommand),   "Reveals the unexplored parts of the map." },
        {"character set gender",          "m", 1, wrap(&ChatCommandHandler::HandleCharSetGenderCommand),        "Changes gender of target. 0=male, 1=female." },
        {"character set itemsrepaired",   "n", 0, wrap(&ChatCommandHandler::HandleCharSetItemsRepairedCommand), "Sets all items repaired for selected player" },
        {"character set level",           "m", 1, wrap(&ChatCommandHandler::HandleCharSetLevelCommand),         "Sets level of selected target to <x>." },
        {"character set name",            "m", 2, wrap(&ChatCommandHandler::HandleCharSetNameCommand),          "Renames character x to y." },
        {"character set phase",           "m", 1, wrap(&ChatCommandHandler::HandleCharSetPhaseCommand),         "Sets phase of selected player" },
        {"character set speed",           "m", 1, wrap(&ChatCommandHandler::HandleCharSetSpeedCommand),         "Sets speed of the selected target to <x>." },
        {"character set standing",        "m", 2, wrap(&ChatCommandHandler::HandleCharSetStandingCommand),      "Sets standing of faction x to y." },
        {"character set talentpoints",    "m", 1, wrap(&ChatCommandHandler::HandleCharSetTalentpointsCommand),  "Sets available talent points of the target." },
        {"character set title",           "m", 1, wrap(&ChatCommandHandler::HandleCharSetTitleCommand),         "Sets pvp title for target" },
        {"character set forcerename",     "m", 1, wrap(&ChatCommandHandler::HandleCharSetForceRenameCommand),   "Forces char x to rename on next login" },
        {"character set customize",       "m", 1, wrap(&ChatCommandHandler::HandleCharSetCustomizeCommand),     "Allows char x to customize on next login" },
        {"character set factionchange",   "m", 1, wrap(&ChatCommandHandler::HandleCharSetFactionChangeCommand), "Allows char x to change the faction on next login" },
        {"character set racechange",      "m", 1, wrap(&ChatCommandHandler::HandleCharSetCustomizeCommand),     "Allows char x to change the race on next login" },
        
        {"character list",                "m", 0 },
        {"character list skills",         "m", 0, wrap(&ChatCommandHandler::HandleCharListSkillsCommand),       "Lists all the skills from a player" },
        {"character list spells",         "m", 0, wrap(&ChatCommandHandler::handleCharListSpellsCommand),       "Lists all the spells from a player" },
        {"character list standing",       "m", 1, wrap(&ChatCommandHandler::HandleCharListStandingCommand),     "Lists standing of faction x." },
        {"character list items",          "m", 0, wrap(&ChatCommandHandler::HandleCharListItemsCommand),        "Lists items of selected Player" },
        {"character list kills",          "m", 0, wrap(&ChatCommandHandler::HandleCharListKillsCommand),        "Lists all kills of selected Player" },
        {"character list instances",      "z", 0, wrap(&ChatCommandHandler::HandleCharListInstanceCommand),     "Lists persistent instances of selected Player" },
        
        {"character clearcooldowns",      "m", 0, wrap(&ChatCommandHandler::HandleCharClearCooldownsCommand),   "Clears all cooldowns for your class." },
        {"character demorph",             "m", 0, wrap(&ChatCommandHandler::HandleCharDeMorphCommand),          "Demorphs from morphed model." },
        {"character levelup",             "m", 1, wrap(&ChatCommandHandler::HandleCharLevelUpCommand),          "Player target will be levelup x levels" },
        {"character removeauras",         "m", 0, wrap(&ChatCommandHandler::HandleCharRemoveAurasCommand),      "Removes all auras from target" },
        {"character removesickness",      "m", 0, wrap(&ChatCommandHandler::HandleCharRemoveSickessCommand),    "Removes ressurrection sickness from target" },
        {"character learn",               "m", 1, wrap(&ChatCommandHandler::HandleCharLearnCommand),            "Learns spell <x> or all available spells by race" },
        {"character unlearn",             "m", 1, wrap(&ChatCommandHandler::HandleCharUnlearnCommand),          "Unlearns spell" },
        {"character learnskill",          "m", 1, wrap(&ChatCommandHandler::HandleCharLearnSkillCommand),       "Learns skill id skillid opt: min max." },
        {"character advanceskill",        "m", 1, wrap(&ChatCommandHandler::HandleCharAdvanceSkillCommand),     "Advances skill line x y times." },
        {"character removeskill",         "m", 1, wrap(&ChatCommandHandler::HandleCharRemoveSkillCommand),      "Removes skill." },
        {"character increaseweaponskill", "m", 0, wrap(&ChatCommandHandler::HandleCharIncreaseWeaponSkill),     "Increase equipped weapon skill x times." },
        {"character resetreputation",     "n", 0, wrap(&ChatCommandHandler::HandleCharResetReputationCommand),  "Resets reputation to start levels." },
        {"character resetspells",         "n", 0, wrap(&ChatCommandHandler::HandleCharResetSpellsCommand),      "Resets all spells of selected player." },
        {"character resettalents",        "n", 0, wrap(&ChatCommandHandler::HandleCharResetTalentsCommand),     "Resets all talents of selected player." },
#if VERSION_STRING >= TBC
        {"character resetskills",         "n", 0, wrap(&ChatCommandHandler::HandleCharResetSkillsCommand),      "Resets all skills." },
#endif
        {"character removeitem",          "m", 1, wrap(&ChatCommandHandler::HandleCharRemoveItemCommand),       "Removes item x count y." },
        {"character advanceallskills",    "m", 0, wrap(&ChatCommandHandler::HandleAdvanceAllSkillsCommand),     "Advances all skills <x> points." },

        {"cheat",                         "0", 0 },
        {"cheat list",                    "m", 0, wrap(&ChatCommandHandler::HandleCheatListCommand),            "Shows active cheats." },
        {"cheat taxi",                    "m", 0, wrap(&ChatCommandHandler::HandleCheatTaxiCommand),            "Toggles TaxiCheat." },
        {"cheat cooldown",                "m", 0, wrap(&ChatCommandHandler::HandleCheatCooldownCommand),        "Toggles CooldownCheat." },
        {"cheat casttime",                "m", 0, wrap(&ChatCommandHandler::HandleCheatCastTimeCommand),        "Toggles CastTimeCheat." },
        {"cheat power",                   "m", 0, wrap(&ChatCommandHandler::HandleCheatPowerCommand),           "Toggles PowerCheat. Disables mana consumption." },
        {"cheat god",                     "m", 0, wrap(&ChatCommandHandler::HandleCheatGodCommand),             "Toggles GodCheat." },
        {"cheat fly",                     "m", 0, wrap(&ChatCommandHandler::HandleCheatFlyCommand),             "Toggles FlyCheat." },
        {"cheat aurastack",               "m", 0, wrap(&ChatCommandHandler::HandleCheatAuraStackCommand),       "Toggles AuraStackCheat." },
        {"cheat itemstack",               "m", 0, wrap(&ChatCommandHandler::HandleCheatItemStackCommand),       "Toggles ItemStackCheat." },
        {"cheat triggerpass",             "m", 0, wrap(&ChatCommandHandler::HandleCheatTriggerpassCommand),     "Ignores area trigger prerequisites." },

        {"commands",                      "0", 0, wrap(&ChatCommandHandler::handleCommandsCommand),             "Shows commands" },

        {"debug",                         "0", 0 },
        {"debug dumpitemset",             "d", 0, wrap(&ChatCommandHandler::HandleMoveDBCItemSetsToDB),             "Dumps DBC itemset bonus to database" },
        {"debug dumpitems",               "d", 0, wrap(&ChatCommandHandler::HandleMoveDB2ItemsToDB),                "Dumps DB2 items to database" },
        {"debug dumpscripts",             "d", 0, wrap(&ChatCommandHandler::HandleMoveHardcodedScriptsToDBCommand), "Dumps hardcoded aispells to cmdline for creatures on map X" },
        {"debug sendcreaturemove",        "d", 0, wrap(&ChatCommandHandler::HandleDebugSendCreatureMove),           "Requests the target creature moves to you using movement manager." },
        {"debug dopctdamage",             "z", 0, wrap(&ChatCommandHandler::HandleDoPercentDamageCommand),          "Do percent damage to creature target" },
        {"debug setscriptphase",          "z", 0, wrap(&ChatCommandHandler::HandleSetScriptPhaseCommand),           "ScriptPhase test" },
        {"debug aicharge",                "z", 0, wrap(&ChatCommandHandler::HandleAiChargeCommand),                 "AiCharge test" },
        {"debug aiknockback",             "z", 0, wrap(&ChatCommandHandler::HandleAiKnockbackCommand),              "AiKnockBack test" },
        {"debug aijump",                  "z", 0, wrap(&ChatCommandHandler::HandleAiJumpCommand),                   "AiJump test" },
        {"debug aifalling",               "z", 0, wrap(&ChatCommandHandler::HandleAiFallingCommand),                "AiFalling test" },
        {"debug movetospawn",             "z", 0, wrap(&ChatCommandHandler::HandleMoveToSpawnCommand),              "Move target to spwn" },
        {"debug position",                "z", 0, wrap(&ChatCommandHandler::HandlePositionCommand),                 "Show position" },
        {"debug setorientation",          "z", 0, wrap(&ChatCommandHandler::HandleSetOrientationCommand),           "Sets orientation on npc" },
        {"debug dumpmovement",            "d", 0, wrap(&ChatCommandHandler::HandleDebugDumpMovementCommand),        "Dumps the player's movement information to chat" },
        {"debug infront",                 "d", 0, wrap(&ChatCommandHandler::HandleDebugInFrontCommand),             "" },
        {"debug showreact",               "d", 0, wrap(&ChatCommandHandler::HandleShowReactionCommand),             "" },
        {"debug aimove",                  "d", 0, wrap(&ChatCommandHandler::HandleAIMoveCommand),                   "" },
        {"debug dist",                    "d", 0, wrap(&ChatCommandHandler::HandleDistanceCommand),                 "" },
        {"debug face",                    "d", 0, wrap(&ChatCommandHandler::HandleFaceCommand),                     "" },
        {"debug dumpstate",               "d", 0, wrap(&ChatCommandHandler::HandleDebugDumpState),                  "" },
        {"debug moveinfo",                "d", 0, wrap(&ChatCommandHandler::HandleDebugMoveInfo),                   "" },
        {"debug landwalk",                "d", 0, wrap(&ChatCommandHandler::HandleDebugLandWalk),                   "Sets landwalk move for unit" },
        {"debug waterwalk",               "d", 0, wrap(&ChatCommandHandler::HandleDebugWaterWalk),                  "Sets waterwal move for unit" },
        {"debug hover",                   "d", 0, wrap(&ChatCommandHandler::HandleDebugHover),                      "Toggles hover move on/off for unit" },
        {"debug state",                   "d", 0, wrap(&ChatCommandHandler::HandleDebugState),                      "Display MovementFlags for unit" },
        {"debug swim",                    "d", 0, wrap(&ChatCommandHandler::HandleDebugSwim),                       "Toggles swim move for unit" },
        {"debug fly",                     "d", 0, wrap(&ChatCommandHandler::HandleDebugFly),                        "Toggles fly move for unit" },
        {"debug disablegravity",          "d", 0, wrap(&ChatCommandHandler::HandleDebugDisableGravity),             "Toggles disablegravitiy move for unit" },
        {"debug featherfall",             "d", 0, wrap(&ChatCommandHandler::HandleDebugFeatherFall),                "Toggles featherfall move for unit" },
        {"debug speed",                   "d", 0, wrap(&ChatCommandHandler::HandleDebugSpeed),                      "Sets move speed for unit. Usage: .debug speed <value>" },
        {"debug castspell",               "d", 0, wrap(&ChatCommandHandler::HandleCastSpellCommand),                "Casts spell on target." },
        {"debug castself",                "d", 0, wrap(&ChatCommandHandler::HandleCastSelfCommand),                 "Target casts spell <spellId> on itself." },
        {"debug castspellne",             "d", 0, wrap(&ChatCommandHandler::HandleCastSpellNECommand),              "Casts spell by spellid on target (only plays animations)" },
        {"debug aggrorange",              "d", 0, wrap(&ChatCommandHandler::HandleAggroRangeCommand),               "Shows aggro Range of the selected Creature." },
        {"debug knockback",               "d", 0, wrap(&ChatCommandHandler::HandleKnockBackCommand),                "Knocks you back by <value>." },
        {"debug fade",                    "d", 0, wrap(&ChatCommandHandler::HandleFadeCommand),                     "Calls ModThreatModifyer() with <value>." },
        {"debug threatMod",               "d", 0, wrap(&ChatCommandHandler::HandleThreatModCommand),                "Calls ModGeneratedThreatModifyer() with <value>." },
        {"debug movefall",                "d", 0, wrap(&ChatCommandHandler::HandleMoveFallCommand),                 "Makes the creature fall to the ground" },
        {"debug threatList",              "d", 0, wrap(&ChatCommandHandler::HandleThreatListCommand),               "Returns all AI_Targets of the selected Creature." },
        {"debug gettptime",               "d", 0, wrap(&ChatCommandHandler::HandleGetTransporterTime),              "Grabs transporter travel time" },
        {"debug dumpcoords",              "d", 0, wrap(&ChatCommandHandler::HandleDebugDumpCoordsCommmand),         "" },
        {"debug rangecheck",              "d", 0, wrap(&ChatCommandHandler::HandleRangeCheckCommand),               "Checks the range between the player and the target." },
        {"debug testlos",                 "d", 0, wrap(&ChatCommandHandler::HandleCollisionTestLOS),                "Tests LoS" },
        {"debug testindoor",              "d", 0, wrap(&ChatCommandHandler::HandleCollisionTestIndoor),             "Tests indoor" },
        {"debug getheight",               "d", 0, wrap(&ChatCommandHandler::HandleCollisionGetHeight),              "Gets height" },
        {"debug deathstate",              "d", 0, wrap(&ChatCommandHandler::HandleGetDeathState),                   "Returns current deathstate for target" },
        {"debug sendfailed",              "d", 0, wrap(&ChatCommandHandler::HandleSendCastFailed),                  "Sends failed cast result <x>" },
        {"debug playmovie",               "d", 0, wrap(&ChatCommandHandler::HandlePlayMovie),                       "Triggers a movie for selected player" },
        {"debug auraupdate",              "d", 0, wrap(&ChatCommandHandler::HandleAuraUpdateAdd),                   "<SpellID> <Flags> <StackCount>" },
        {"debug auraremove",              "d", 0, wrap(&ChatCommandHandler::HandleAuraUpdateRemove),                "Remove Auras in visual slot" },
        {"debug spawnwar",                "d", 0, wrap(&ChatCommandHandler::HandleDebugSpawnWarCommand),            "Spawns desired amount of npcs to fight with eachother" },
        {"debug updateworldstate",        "d", 0, wrap(&ChatCommandHandler::HandleUpdateWorldStateCommand),         "Sets the worldstate field to the specified value" },
        {"debug initworldstates",         "d", 0, wrap(&ChatCommandHandler::HandleInitWorldStatesCommand),          "(Re)initializes the worldstates." },
        {"debug clearworldstates",        "d", 0, wrap(&ChatCommandHandler::HandleClearWorldStatesCommand),         "Clears the worldstates" },
        {"debug pvpcredit",               "m", 0, wrap(&ChatCommandHandler::HandleDebugPVPCreditCommand),           "Sends PVP credit packet, with specified rank and points" },
        {"debug calcdist",                "d", 0, wrap(&ChatCommandHandler::HandleSimpleDistanceCommand),           "Displays distance between your position and x y z" },
        {"debug setunitbyte",             "d", 0, wrap(&ChatCommandHandler::HandleDebugSetUnitByteCommand),         "Set value z for unit byte x with offset y." },
        {"debug setplayerflags",          "d", 0, wrap(&ChatCommandHandler::HandleDebugSetPlayerFlagsCommand),      "Add player flags x to selected player" },
        {"debug getplayerflags",          "d", 0, wrap(&ChatCommandHandler::HandleDebugGetPlayerFlagsCommand),      "Display current player flags of selected player x" },
        {"debug setweather",              "d", 0, wrap(&ChatCommandHandler::HandleDebugSetWeatherCommand),          "Change zone weather <type> <densitiy>" },

        {"dismount",                      "h", 0, wrap(&ChatCommandHandler::HandleDismountCommand),                "Dismounts targeted unit." },

        {"event",                         "0", 0 },
        {"event list",                    "m", 0, wrap(&ChatCommandHandler::HandleEventListEvents),               "Shows list of currently active events" },
        {"event start",                   "m", 1, wrap(&ChatCommandHandler::HandleEventStartEvent),               "Force start an event" },
        {"event stop",                    "m", 1, wrap(&ChatCommandHandler::HandleEventStopEvent),                "Force stop an event" },
        {"event reset",                   "m", 1, wrap(&ChatCommandHandler::HandleEventResetEvent),               "Resets force flags for an event" },
        {"event reload",                  "a", 0, wrap(&ChatCommandHandler::HandleEventReloadAllEvents),          "Reloads all events from the database" },

        {"gm",                            "0", 0 },
        {"gm active",                     "t", 0, wrap(&ChatCommandHandler::HandleGMActiveCommand),               "Activate/Deactivate <GM> tag" },
        {"gm allowwhispers",              "c", 1, wrap(&ChatCommandHandler::HandleGMAllowWhispersCommand),        "Allows whispers from player <s>." },
        {"gm announce",                   "u", 1, wrap(&ChatCommandHandler::HandleGMAnnounceCommand),             "Sends announce to all online GMs" },
        {"gm blockwhispers",              "c", 1, wrap(&ChatCommandHandler::HandleGMBlockWhispersCommand),        "Blocks whispers from player <s>." },
        {"gm devtag",                     "1", 0, wrap(&ChatCommandHandler::HandleGMDevTagCommand),               "Activate/Deactivate <DEV> tag" },
        {"gm list",                       "0", 0, wrap(&ChatCommandHandler::HandleGMListCommand),                 "Shows active GM's" },
        {"gm logcomment",                 "1", 1, wrap(&ChatCommandHandler::HandleGMLogCommentCommand),           "Adds a comment to the GM log." },

        {"gmTicket",                      "0", 0 },
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        {"gmTicket get",                  "c", 0, wrap(&ChatCommandHandler::HandleGMTicketListCommand),           "Gets GM Ticket list." },
        {"gmTicket getId",                "c", 1, wrap(&ChatCommandHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket by player name." },
        {"gmTicket delId",                "c", 1, wrap(&ChatCommandHandler::HandleGMTicketRemoveByIdCommand),     "Deletes GM Ticket by player name." },
#else
        {"gmTicket list",                 "c", 0, wrap(&ChatCommandHandler::HandleGMTicketListCommand),           "Lists all active GM Tickets." },
        {"gmTicket get",                  "c", 1, wrap(&ChatCommandHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket with ID x." },
        {"gmTicket remove",               "c", 1, wrap(&ChatCommandHandler::HandleGMTicketRemoveByIdCommand),     "Removes GM Ticket with ID x." },
        {"gmTicket deletepermanent",      "z", 1, wrap(&ChatCommandHandler::HandleGMTicketDeletePermanentCommand),"Deletes GM Ticket with ID x permanently." },
        {"gmTicket assign",               "c", 2, wrap(&ChatCommandHandler::HandleGMTicketAssignToCommand),       "Assigns GM Ticket with id x to GM y." },
        {"gmTicket release",              "c", 1, wrap(&ChatCommandHandler::HandleGMTicketReleaseCommand),        "Releases assigned GM Ticket with ID x." },
        {"gmTicket comment",              "c", 2, wrap(&ChatCommandHandler::HandleGMTicketCommentCommand),        "Sets comment x to GM Ticket with ID y." },
#endif
        {"gmTicket toggle",               "z", 0, wrap(&ChatCommandHandler::HandleGMTicketToggleTicketSystemStatusCommand), "Toggles the ticket system status." },

        {"gobject",                       "0", 0 },
        {"gobject damage",                "o", 1, wrap(&ChatCommandHandler::HandleGODamageCommand),               "Damages the GO for the specified hitpoints" },
        {"gobject delete",                "o", 0, wrap(&ChatCommandHandler::HandleGODeleteCommand),               "Deletes selected GameObject" },
        {"gobject enable",                "o", 0, wrap(&ChatCommandHandler::HandleGOEnableCommand),               "Enables the selected GO for use." },
        {"gobject export",                "o", 0, wrap(&ChatCommandHandler::HandleGOExportCommand),               "Exports the selected GO to .sql file" },
        {"gobject info",                  "o", 0, wrap(&ChatCommandHandler::HandleGOInfoCommand),                 "Gives you information about selected GO" },
        {"gobject movehere",              "g", 0, wrap(&ChatCommandHandler::HandleGOMoveHereCommand),             "Moves gameobject to your position" },
        {"gobject open",                  "o", 0, wrap(&ChatCommandHandler::HandleGOOpenCommand),                 "Toggles open/close (state) of selected GO." },
        {"gobject rebuild",               "o", 0, wrap(&ChatCommandHandler::HandleGORebuildCommand),              "Rebuilds the GO." },
        {"gobject rotate",                "g", 0, wrap(&ChatCommandHandler::HandleGORotateCommand),               "Rotates the object. <Axis> x,y, Default o." },
        {"gobject select",                "o", 0, wrap(&ChatCommandHandler::HandleGOSelectCommand),               "Selects the nearest GameObject to you" },
        {"gobject selectguid",            "o", 1, wrap(&ChatCommandHandler::HandleGOSelectGuidCommand),           "Selects GO with <guid>" },

        {"gobject set",                   "o", 0 },
        {"gobject set animprogress",      "o", 1, wrap(&ChatCommandHandler::HandleGOSetAnimProgressCommand),      "Sets anim progress of selected GO" },
        {"gobject set faction",           "o", 1, wrap(&ChatCommandHandler::HandleGOSetFactionCommand),           "Sets the faction of the GO" },
        {"gobject set flags",             "o", 1, wrap(&ChatCommandHandler::HandleGOSetFlagsCommand),             "Sets the flags of the GO" },
        {"gobject set overrides",         "o", 1, wrap(&ChatCommandHandler::HandleGOSetOverridesCommand),         "Sets override of selected GO" },
        {"gobject set phase",             "o", 1, wrap(&ChatCommandHandler::HandleGOSetPhaseCommand),             "Sets phase of selected GO" },
        {"gobject set scale",             "o", 1, wrap(&ChatCommandHandler::HandleGOSetScaleCommand),             "Sets scale of selected GO" },
        {"gobject set state",             "o", 1, wrap(&ChatCommandHandler::HandleGOSetStateCommand),             "Sets the state byte of the GO" },
        {"gobject spawn",                 "o", 1, wrap(&ChatCommandHandler::HandleGOSpawnCommand),               "Spawns a GameObject by ID" },

        {"gocreature",                    "v", 1, wrap(&ChatCommandHandler::HandleGoCreatureSpawnCommand),        "Teleports you to the creature with <spwn_id>." },
        {"gogameobject",                  "v", 1, wrap(&ChatCommandHandler::HandleGoGameObjectSpawnCommand),      "Teleports you to the gameobject with <spawn_id>." },
        {"gostartlocation",               "m", 1, wrap(&ChatCommandHandler::HandleGoStartLocationCommand),        "Teleports you to a starting location" },
        {"gotrig",                        "v", 1, wrap(&ChatCommandHandler::HandleGoTriggerCommand),              "Teleports you to the areatrigger with <id>." },
        {"gps",                           "0", 0, wrap(&ChatCommandHandler::HandleGPSCommand),                    "Shows position of targeted unit" },

        {"guild",                         "0", 0 },
        {"guild create",                  "m", 1, wrap(&ChatCommandHandler::HandleGuildCreateCommand),            "Creates a guild." },
        {"guild disband",                 "m", 0, wrap(&ChatCommandHandler::HandleGuildDisbandCommand),           "Disbands the guild of your target." },
#if VERSION_STRING >= Cata
        {"guild info",                    "m", 0, wrap(&ChatCommandHandler::HandleGuildInfoCommand),              "Shows guild info of your target." },
#endif
        {"guild join",                    "m", 1, wrap(&ChatCommandHandler::HandleGuildJoinCommand),              "Force selected player to join a guild by name" },
        {"guild listmembers",             "m", 1, wrap(&ChatCommandHandler::HandleGuildListMembersCommand),       "Lists guildmembers with ranks by guild name." },
        {"guild rename",                  "m", 1, wrap(&ChatCommandHandler::HandleRenameGuildCommand),            "Renames a guild." },
        {"guild removeplayer",            "m", 0, wrap(&ChatCommandHandler::HandleGuildRemovePlayerCommand),      "Removes a player from a guild." },

        {"help",                          "0", 0, wrap(&ChatCommandHandler::handleHelpCommand),                   "Shows help for command" },

        {"instance",                      "0", 0 },
        {"instance create",               "z", 4, wrap(&ChatCommandHandler::HandleCreateInstanceCommand),         "Creates instance by mapid x y z" },
        {"instance countcreature",        "z", 1, wrap(&ChatCommandHandler::HandleCountCreaturesCommand),         "Returns number of creatures with entry x" },
        {"instance exit",                 "m", 0, wrap(&ChatCommandHandler::HandleExitInstanceCommand),           "Exits current instance, return to entry point." },
        {"instance info",                 "m", 0, wrap(&ChatCommandHandler::HandleGetInstanceInfoCommand),        "Gets info about instance with ID x (default current instance)." },
        {"instance reset",                "z", 1, wrap(&ChatCommandHandler::HandleResetInstanceCommand),          "Removes instance ID x from target player." },
        {"instance resetall",             "m", 0, wrap(&ChatCommandHandler::HandleResetAllInstancesCommand),      "Removes all instance IDs from target player." },
        {"instance shutdown",             "z", 0, wrap(&ChatCommandHandler::HandleShutdownInstanceCommand),       "Shutdown instance with ID x (default is current instance)." },
        {"instance showtimers",           "m", 0, wrap(&ChatCommandHandler::HandleShowTimersCommand),             "Show timers for current instance." },

        {"invincible",                    "j", 0, wrap(&ChatCommandHandler::HandleInvincibleCommand),             "Toggles invincibility on/off" },
        {"invisible",                     "i", 0, wrap(&ChatCommandHandler::HandleInvisibleCommand),              "Toggles invisibility and invincibility on/off" },

        {"kick",                          "0", 0 },
        {"kick player",                   "f", 1, wrap(&ChatCommandHandler::HandleKickByNameCommand),             "Disconnects the player with name <s>." },
        {"kick account",                  "f", 1, wrap(&ChatCommandHandler::HandleKKickBySessionCommand),         "Disconnects the session with account name <s>." },
        {"kick ip",                       "f", 1, wrap(&ChatCommandHandler::HandleKickByIPCommand),               "Disconnects the session with the ip <s>." },
        {"kill",                          "r", 0, wrap(&ChatCommandHandler::HandleKillCommand),                   "Kills selected unit or player by name" },

        {"lookup",                        "0", 0 },
        {"lookup achievement",            "l", 1, wrap(&ChatCommandHandler::HandleLookupAchievementCommand),      "Looks up achievement string x." },
        {"lookup creature",               "l", 1, wrap(&ChatCommandHandler::HandleLookupCreatureCommand),         "Looks up creature string x." },
        {"lookup faction",                "l", 1, wrap(&ChatCommandHandler::HandleLookupFactionCommand),          "Looks up faction string x." },
        {"lookup item",                   "l", 1, wrap(&ChatCommandHandler::HandleLookupItemCommand),             "Looks up item string x." },
        {"lookup object",                 "l", 1, wrap(&ChatCommandHandler::HandleLookupObjectCommand),           "Looks up gameobject string x." },
        {"lookup quest",                  "l", 1, wrap(&ChatCommandHandler::HandleLookupQuestCommand),            "Looks up quest string x." },
        {"lookup spell",                  "l", 1, wrap(&ChatCommandHandler::HandleLookupSpellCommand),            "Looks up spell string x." },
        {"lookup skill",                  "l", 1, wrap(&ChatCommandHandler::HandleLookupSkillCommand),            "Looks up skill string x." },

        {"modify",                        "0", 0 },
        {"modify hp",                     "m", 1, wrap(&ChatCommandHandler::HandleModifyHp),                      "Mods health points (HP) of selected target" },
        {"modify mana",                   "m", 1, wrap(&ChatCommandHandler::HandleModifyMana),                    "Mods mana points (MP) of selected target." },
        {"modify rage",                   "m", 1, wrap(&ChatCommandHandler::HandleModifyRage),                    "Mods rage points of selected target." },
        {"modify energy",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyEnergy),                  "Mods energy points of selected target." },
#if VERSION_STRING >= WotLK
        {"modify runicpower",             "m", 1, wrap(&ChatCommandHandler::HandleModifyRunicpower),              "Mods runic power points of selected target." },
#endif
        {"modify strength",               "m", 1, wrap(&ChatCommandHandler::HandleModifyStrength),                "Mods strength value of the selected target." },
        {"modify agility",                "m", 1, wrap(&ChatCommandHandler::HandleModifyAgility),                 "Mods agility value of the selected target." },
        {"modify intelligence",           "m", 1, wrap(&ChatCommandHandler::HandleModifyIntelligence),            "Mods intelligence value of the selected target." },
        {"modify spirit",                 "m", 1, wrap(&ChatCommandHandler::HandleModifySpirit),                  "Mods spirit value of the selected target." },
        {"modify armor",                  "m", 1, wrap(&ChatCommandHandler::HandleModifyArmor),                   "Mods armor of selected target." },
        {"modify holy",                   "m", 1, wrap(&ChatCommandHandler::HandleModifyHoly),                    "Mods holy resistance of selected target." },
        {"modify fire",                   "m", 1, wrap(&ChatCommandHandler::HandleModifyFire),                    "Mods fire resistance of selected target." },
        {"modify nature",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyNature),                  "Mods nature resistance of selected target." },
        {"modify frost",                  "m", 1, wrap(&ChatCommandHandler::HandleModifyFrost),                   "Mods frost resistance of selected target." },
        {"modify shadow",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyShadow),                  "Mods shadow resistance of selected target." },
        {"modify arcane",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyArcane),                  "Mods arcane resistance of selected target." },
        {"modify damage",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyDamage),                  "Mods damage done by the selected target." },
        {"modify ap",                     "m", 1, wrap(&ChatCommandHandler::HandleModifyAp),                      "Mods attack power of the selected target." },
        {"modify rangeap",                "m", 1, wrap(&ChatCommandHandler::HandleModifyRangeap),                 "Mods range attack power of the selected target." },
        {"modify scale",                  "m", 1, wrap(&ChatCommandHandler::HandleModifyScale),                   "Mods scale of the selected target." },
        {"modify nativedisplayid",        "m", 1, wrap(&ChatCommandHandler::HandleModifyNativedisplayid),         "Mods native display identifier of the target." },
        {"modify displayid",              "m", 1, wrap(&ChatCommandHandler::HandleModifyDisplayid),               "Mods display identifier (DisplayID) of the target." },
        {"modify flags",                  "m", 1, wrap(&ChatCommandHandler::HandleModifyFlags),                   "Mods flags of the selected target." },
        {"modify faction",                "m", 1, wrap(&ChatCommandHandler::HandleModifyFaction),                 "Mods faction template of the selected target." },
        {"modify dynamicflags",           "m", 1, wrap(&ChatCommandHandler::HandleModifyDynamicflags),            "Mods dynamic flags of the selected target." },
#if VERSION_STRING < Cata
        {"modify happiness",              "m", 1, wrap(&ChatCommandHandler::HandleModifyHappiness),               "Mods happiness value of the selected target." },
#endif
        {"modify boundingradius",         "m", 1, wrap(&ChatCommandHandler::HandleModifyBoundingradius),          "Mods bounding radius of the selected target." },
        {"modify combatreach",            "m", 1, wrap(&ChatCommandHandler::HandleModifyCombatreach),             "Mods combat reach of the selected target." },
        {"modify emotestate",             "m", 1, wrap(&ChatCommandHandler::HandleModifyEmotestate),              "Mods Unit emote state of the selected target." },
        {"modify bytes0",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyBytes0),                  "Mods bytes0 entry of selected target." },
        {"modify bytes1",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyBytes1),                  "Mods bytes1 entry of selected target." },
        {"modify bytes2",                 "m", 1, wrap(&ChatCommandHandler::HandleModifyBytes2),                  "Mods bytes2 entry of selected target." },

        {"mount",                         "m", 1, wrap(&ChatCommandHandler::HandleMountCommand),                  "Mounts targeted unit with modelid x." },

        {"npc",                           "0", 0 },
        {"npc addagent",                  "n", 10, wrap(&ChatCommandHandler::HandleNpcAddAgentCommand),            "Add ai agents to npc." },
        {"npc addtrainerspell",           "m", 1, wrap(&ChatCommandHandler::HandleNpcAddTrainerSpellCommand),     "Add spells to trainer learn list." },
        {"npc appear",                    "n", 0, wrap(&ChatCommandHandler::HandleNpcAppearCommand),              "Teleports you to the target NPC's location." },
        {"npc cast",                      "n", 1, wrap(&ChatCommandHandler::HandleNpcCastCommand),                "Makes NPC cast <spellid>." },
        {"npc come",                      "n", 0, wrap(&ChatCommandHandler::HandleNpcComeCommand),                "Makes NPC move to your position" },
        {"npc delete",                    "n", 0, wrap(&ChatCommandHandler::HandleNpcDeleteCommand),              "Deletes mob from world optional from DB" },
        {"npc info",                      "n", 0, wrap(&ChatCommandHandler::HandleNpcInfoCommand),                "Displays NPC information" },
        {"npc listAgent",                 "n", 0, wrap(&ChatCommandHandler::HandleNpcListAIAgentCommand),         "List AIAgents of selected target." },
        {"npc listloot",                  "m", 0, wrap(&ChatCommandHandler::HandleNpcListLootCommand),            "Displays possible loot for the selected NPC." },
        {"npc follow",                    "m", 0, wrap(&ChatCommandHandler::HandleNpcFollowCommand),              "Sets NPC to follow you" },
        {"npc stopfollow",                "m", 0, wrap(&ChatCommandHandler::HandleNpcStopFollowCommand),          "Sets NPC to not follow anything" },
        {"npc possess",                   "n", 0, wrap(&ChatCommandHandler::HandlePossessCommand),                "Possess targeted NPC (mind control)" },
        {"npc unpossess",                 "n", 0, wrap(&ChatCommandHandler::HandleUnPossessCommand),              "Unpossess any currently possessed npc." },
        {"npc return",                    "n", 0, wrap(&ChatCommandHandler::HandleNpcReturnCommand),              "Returns NPC to spawnpoint." },
        {"npc respawn",                   "n", 0, wrap(&ChatCommandHandler::HandleNpcRespawnCommand),             "Respawns a dead NPC from its corpse." },
        {"npc say",                       "n", 1, wrap(&ChatCommandHandler::HandleNpcSayCommand),                 "Makes selected NPC say <text>." },
        {"npc select",                    "n", 0, wrap(&ChatCommandHandler::HandleNpcSelectCommand),              "Selects closest NPC" },

        {"npc set",                       "0", 0 },
        {"npc set canfly",                "n", 0, wrap(&ChatCommandHandler::HandleNpcSetCanFlyCommand),           "Toggles CanFly state" },
        {"npc set emote",                 "n", 1, wrap(&ChatCommandHandler::HandleNpcSetEmoteCommand),            "Sets emote state" },
        {"npc set equip",                 "m", 1, wrap(&ChatCommandHandler::HandleNpcSetEquipCommand),            "Sets equipment itemt" },
        {"npc set flags",                 "n", 1, wrap(&ChatCommandHandler::HandleNpcSetFlagsCommand),            "Sets NPC flags" },
        {"npc set formationmaster",       "m", 0, wrap(&ChatCommandHandler::HandleNpcSetFormationMasterCommand),  "Sets formation master." },
        {"npc set formationslave",        "m", 0, wrap(&ChatCommandHandler::HandleNpcSetFormationSlaveCommand),   "Sets formation slave with distance and angle" },
        {"npc set formationclear",        "m", 0, wrap(&ChatCommandHandler::HandleNpcSetFormationClearCommand),   "Removes formation from creature" },
        {"npc set phase",                 "n", 1, wrap(&ChatCommandHandler::HandleNpcSetPhaseCommand),            "Sets phase for selected creature" },
        {"npc set standstate",            "m", 1, wrap(&ChatCommandHandler::HandleNpcSetStandstateCommand),       "Sets standstate for selected creature" },
        {"npc set entry",                 "m", 1, wrap(&ChatCommandHandler::HandleNpcChangeEntry),                "Sets a New Entry for selected creature" },

        {"npc spawn",                     "n", 1, wrap(&ChatCommandHandler::HandleNpcSpawnCommand),               "Spawns NPC of entry <id>" },
        {"npc showtimers",                "m", 0, wrap(&ChatCommandHandler::HandleNpcShowTimersCommand),          "Shows timers for selected creature" },
        {"npc vendoradditem",             "n", 1, wrap(&ChatCommandHandler::HandleNpcVendorAddItemCommand),       "Adds item to vendor" },
        {"npc vendorremoveitem",          "n", 1, wrap(&ChatCommandHandler::HandleNpcVendorRemoveItemCommand),    "Removes item from vendor." },
        {"npc yell",                      "n", 1, wrap(&ChatCommandHandler::HandleNpcYellCommand),                "Makes selected NPC yell <text>." },

        {"pet",                           "0", 0 },
        {"pet create",                    "m", 1, wrap(&ChatCommandHandler::HandlePetCreateCommand),              "Creates a pet with <entry>." },
        {"pet dismiss",                   "m", 0, wrap(&ChatCommandHandler::HandlePetDismissCommand),             "Dismisses a pet by for selected player or selected pet." },
        {"pet rename",                    "m", 1, wrap(&ChatCommandHandler::HandlePetRenameCommand),              "Renames a pet to <name>." },
        {"pet addspell",                  "m", 1, wrap(&ChatCommandHandler::HandlePetAddSpellCommand),            "Teaches pet <spell>." },
        {"pet removespell",               "m", 1, wrap(&ChatCommandHandler::HandlePetRemoveSpellCommand),         "Removes pet spell <spell>." },
        {"pet setlevel",                  "m", 1, wrap(&ChatCommandHandler::HandlePetSetLevelCommand),            "Sets pet level to <level>." },

        {"playerinfo",                    "m", 0, wrap(&ChatCommandHandler::HandlePlayerInfo),                    "Displays info for selected character or <charname>" },

        {"quest",                         "0", 0 },
        {"quest addboth",                 "2", 1, wrap(&ChatCommandHandler::HandleQuestAddBothCommand),           "Add quest <id> to the targeted NPC as start & finish" },
        {"quest addfinish",               "2", 1, wrap(&ChatCommandHandler::HandleQuestAddFinishCommand),         "Add quest <id> to the targeted NPC as finisher" },
        {"quest addstart",                "2", 1, wrap(&ChatCommandHandler::HandleQuestAddStartCommand),          "Add quest <id> to the targeted NPC as starter" },
        {"quest delboth",                 "2", 1, wrap(&ChatCommandHandler::HandleQuestDelBothCommand),           "Delete quest <id> from the targeted NPC as start & finish" },
        {"quest delfinish",               "2", 1, wrap(&ChatCommandHandler::HandleQuestDelFinishCommand),         "Delete quest <id> from the targeted NPC as finisher" },
        {"quest delstart",                "2", 1, wrap(&ChatCommandHandler::HandleQuestDelStartCommand),          "Delete quest <id> from the targeted NPC as starter" },
        {"quest complete",                "2", 1, wrap(&ChatCommandHandler::HandleQuestFinishCommand),            "Complete/Finish quest <id>" },
        {"quest fail",                    "2", 1, wrap(&ChatCommandHandler::HandleQuestFailCommand),              "Fail quest <id>" },
        {"quest finisher",                "2", 1, wrap(&ChatCommandHandler::HandleQuestFinisherCommand),          "Lookup quest finisher for quest <id>" },
        {"quest item",                    "2", 1, wrap(&ChatCommandHandler::HandleQuestItemCommand),              "Lookup itemid necessary for quest <id>" },
        {"quest list",                    "2", 1, wrap(&ChatCommandHandler::HandleQuestListCommand),              "Lists the quests for the npc <id>" },
        {"quest load",                    "2", 0, wrap(&ChatCommandHandler::HandleQuestLoadCommand),              "Loads quests from database" },
        {"quest giver",                   "2", 1, wrap(&ChatCommandHandler::HandleQuestGiverCommand),             "Lookup quest giver for quest <id>" },
        {"quest remove",                  "2", 1, wrap(&ChatCommandHandler::HandleQuestRemoveCommand),            "Removes the quest <id> from the targeted player" },
        {"quest reward",                  "2", 1, wrap(&ChatCommandHandler::HandleQuestRewardCommand),            "Shows reward for quest <id>" },
        {"quest status",                  "2", 1, wrap(&ChatCommandHandler::HandleQuestStatusCommand),            "Lists the status of quest <id>" },
        {"quest start",                   "2", 1, wrap(&ChatCommandHandler::HandleQuestStartCommand),             "Starts quest <id>" },
        {"quest startspawn",              "2", 1, wrap(&ChatCommandHandler::HandleQuestStarterSpawnCommand),      "Port to spawn location for quest <id> (starter)" },
        {"quest finishspawn",             "2", 1, wrap(&ChatCommandHandler::HandleQuestFinisherSpawnCommand),     "Port to spawn location for quest <id> (finisher)" },

        {"recall",                        "0", 0 },
        {"recall list",                   "q", 0, wrap(&ChatCommandHandler::HandleRecallListCommand),             "List recall locations" },
        {"recall add",                    "q", 1, wrap(&ChatCommandHandler::HandleRecallAddCommand),              "Add a recall location" },
        {"recall del",                    "q", 1, wrap(&ChatCommandHandler::HandleRecallDelCommand),              "Remove a recall location" },
        {"recall port",                   "q", 1, wrap(&ChatCommandHandler::HandleRecallGoCommand),               "Ports you to recalled location" },
        {"recall portplayer",             "m", 2, wrap(&ChatCommandHandler::HandleRecallPortPlayerCommand),       "Ports specified player to a recalled location" },
        {"recall portus",                 "m", 1, wrap(&ChatCommandHandler::HandleRecallPortUsCommand),           "Ports you and the selected player to recalled location" },

        {"revive",                        "r", 0, wrap(&ChatCommandHandler::HandleReviveCommand),                 "Revives you or a selected target or player by name" },
        {"root",                          "b", 0, wrap(&ChatCommandHandler::HandleRootCommand),                   "Roots selected target." },

        {"server",                        "0", 0 },
        {"server info",                   "0", 0, wrap(&ChatCommandHandler::HandleServerInfoCommand),             "Shows detailed Server info." },
        {"server rehash",                 "z", 0, wrap(&ChatCommandHandler::HandleServerRehashCommand),           "Reloads config file." },
        {"server save",                   "s", 0, wrap(&ChatCommandHandler::HandleServerSaveCommand),             "Save targeted or named player." },
        {"server saveall",                "s", 0, wrap(&ChatCommandHandler::HandleServerSaveAllCommand),          "Save all online player." },
        {"server setmotd",                "m", 1, wrap(&ChatCommandHandler::HandleServerSetMotdCommand),          "Sets server MessageOfTheDay." },
        {"server shutdown",               "z", 0, wrap(&ChatCommandHandler::HandleServerShutdownCommand),         "Initiates server shutdown in <x> seconds." },
        {"server cancelshutdown",         "z", 0, wrap(&ChatCommandHandler::HandleServerCancelShutdownCommand),   "Cancels a Server Restart/Shutdown." },
        {"server restart",                "z", 0, wrap(&ChatCommandHandler::HandleServerRestartCommand),          "Initiates server restart in <x> seconds." },

        {"server reloadtable",                      "m", 0 },
        {"server reloadtable gameobjects",          "z", 0, wrap(&ChatCommandHandler::HandleReloadGameobjectsCommand),       "Reload gameobjets" },
        {"server reloadtable creatures",            "z", 0, wrap(&ChatCommandHandler::HandleReloadCreaturesCommand),         "Reload creatures" },
        {"server reloadtable areatriggers",         "z", 0, wrap(&ChatCommandHandler::HandleReloadAreaTriggersCommand),      "Reload areatriggers table" },
        {"server reloadtable command_overrides",    "z", 0, wrap(&ChatCommandHandler::HandleReloadCommandOverridesCommand),  "Reload command_overrides table" },
        {"server reloadtable fishing",              "z", 0, wrap(&ChatCommandHandler::HandleReloadFishingCommand),           "Reload fishing table" },
        {"server reloadtable gossip_menu_option",   "z", 0, wrap(&ChatCommandHandler::HandleReloadGossipMenuOptionCommand),  "Reload gossip_menu_option table" },
        {"server reloadtable graveyards",           "z", 0, wrap(&ChatCommandHandler::HandleReloadGraveyardsCommand),        "Reload graveyards table" },
        {"server reloadtable items",                "z", 0, wrap(&ChatCommandHandler::HandleReloadItemsCommand),             "Reload items table" },
        {"server reloadtable itempages",            "z", 0, wrap(&ChatCommandHandler::HandleReloadItempagesCommand),         "Reload itempages table" },
        {"server reloadtable npc_script_text",      "z", 0, wrap(&ChatCommandHandler::HandleReloadNpcScriptTextCommand),     "Reload npc_script_text table" },
        {"server reloadtable npc_gossip_text",      "z", 0, wrap(&ChatCommandHandler::HandleReloadNpcTextCommand),           "Reload npc_gossip_text table" },
        {"server reloadtable pet_level_abilities",  "z", 0, wrap(&ChatCommandHandler::HandleReloadPetLevelAbilitiesCommand), "Reload pet_level_abilities table" },
        {"server reloadtable player_xp_for_level",  "z", 0, wrap(&ChatCommandHandler::HandleReloadPlayerXpForLevelCommand),  "Reload player_xp_for_level table" },
        {"server reloadtable points_of_interest",   "z", 0, wrap(&ChatCommandHandler::HandleReloadPointsOfInterestCommand),  "Reload points_of_interest table" },
        {"server reloadtable quests",               "z", 0, wrap(&ChatCommandHandler::HandleReloadQuestsCommand),            "Reload quests table" },
        {"server reloadtable spell_teleport_coords","z", 0, wrap(&ChatCommandHandler::HandleReloadTeleportCoordsCommand),    "Reload teleport_coords table" },
        {"server reloadtable worldbroadcast",       "z", 0, wrap(&ChatCommandHandler::HandleReloadWorldbroadcastCommand),    "Reload worldbroadcast table" },
        {"server reloadtable worldmap_info",        "z", 0, wrap(&ChatCommandHandler::HandleReloadWorldmapInfoCommand),      "Reload worldmap_info table" },
        {"server reloadtable worldstring_tables",   "z", 0, wrap(&ChatCommandHandler::HandleReloadWorldstringTablesCommand), "Reload worldstring_tables table" },
        {"server reloadtable zoneguards",           "z", 0, wrap(&ChatCommandHandler::HandleReloadZoneguardsCommand),        "Reload zoneguards table" },

        {"server reloadscript",           "m", 0, wrap(&ChatCommandHandler::HandleServerReloadScriptsCommand),    "" },

        {"summon",                        "v", 1, wrap(&ChatCommandHandler::HandleSummonCommand),                 "Summons x to your position." },

        {"ticket",                        "0", 0 },
        {"ticket list",                   "c", 0, wrap(&ChatCommandHandler::HandleTicketListCommand),             "Shows all active tickets" },
        {"ticket listall",                "c", 0, wrap(&ChatCommandHandler::HandleTicketListAllCommand),          "Shows all tickets in the database" },
        {"ticket get",                    "c", 1, wrap(&ChatCommandHandler::HandleTicketGetCommand),              "Returns the content of the specified ID" },
        {"ticket close",                  "c", 1, wrap(&ChatCommandHandler::HandleTicketCloseCommand),            "Close ticket with specified ID" },
        {"ticket delete",                 "a", 1, wrap(&ChatCommandHandler::HandleTicketDeleteCommand),           "Delete ticket by specified ID" },

        {"transport",                     "m", 0 },
        {"transport info",                "m", 0, wrap(&ChatCommandHandler::HandleGetTransporterInfo),            "Displays the current transport info" },
        {"transport spawn",               "m", 1, wrap(&ChatCommandHandler::HandleSpawnInstanceTransport),        "Spawns transport with entry/period in current instance" },
        {"transport start",               "m", 0, wrap(&ChatCommandHandler::HandleStartTransport),                "Force starts the current transport" },
        {"transport stop",                "m", 0, wrap(&ChatCommandHandler::HandleStopTransport),                 "Force stops the current transport" },
        {"transport getperiod",           "m", 0, wrap(&ChatCommandHandler::HandleGetTransporterTime),            "Displays the current transport period in ms" },

        {"unban",                         "0", 0 },
        {"unban ip",                      "m", 1, wrap(&ChatCommandHandler::HandleIPUnBanCommand),                "Deletes an address from the IP ban table: <address>" },
        {"unban character",               "b", 1, wrap(&ChatCommandHandler::HandleUnBanCharacterCommand),         "Unbans character x" },
        {"unroot",                        "b", 0, wrap(&ChatCommandHandler::HandleUnrootCommand),                 "Unroots selected target." },

        {"vehicle",                       "m", 0 },
#ifdef FT_VEHICLES
        {"vehicle ejectpassenger",        "m", 1, wrap(&ChatCommandHandler::HandleVehicleEjectPassengerCommand),     "Ejects the passenger from the specified seat" },
        {"vehicle ejectallpassengers",    "m", 0, wrap(&ChatCommandHandler::HandleVehicleEjectAllPassengersCommand), "Ejects all passengers from the vehicle" },
        {"vehicle installaccessories",    "m", 0, wrap(&ChatCommandHandler::HandleVehicleInstallAccessoriesCommand), "Installs the accessories for the selected vehicle" },
        {"vehicle addpassenger",          "m", 1, wrap(&ChatCommandHandler::HandleVehicleAddPassengerCommand),       "Adds a new NPC passenger to the vehicle" },
#endif

        {"wannounce",                     "u", 1, wrap(&ChatCommandHandler::HandleWAnnounceCommand),              "Sends a widescreen announcement to all players." },

        {"waypoint",                      "0", 0 },
        {"waypoint add",                  "w", 0, wrap(&ChatCommandHandler::HandleWayPointAddCommand),           "Add wp for selected creature at current pos." },
        {"waypoint delete",               "w", 0, wrap(&ChatCommandHandler::HandleWayPointDeleteCommand),        "Deletes selected wp." },
        {"waypoint deleteall",            "w", 0, wrap(&ChatCommandHandler::HandleWayPointDeleteAllCommand),     "Deletes all waypoints of selected creature." },
        {"waypoint hide",                 "w", 0, wrap(&ChatCommandHandler::HandleWayPointHideCommand),          "Hide wp's for selected creature." },
        {"waypoint show",                 "w", 0, wrap(&ChatCommandHandler::HandleWayPointShowCommand),          "Show wp's for selected creature <bool backwards>" },

        {"worldport",                     "v", 4, wrap(&ChatCommandHandler::HandleWorldPortCommand),             "Teleports you to a location with mapid x y z" }
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
