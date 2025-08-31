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

        {"account",                       "0", 0 },
        {"account create",                "a", 2, wrap(&ChatHandler::handleAccountCreate),               "Creates an account with name and password" },
        {"account setgm",                 "z", 2, wrap(&ChatHandler::handleAccountSetGMCommand),         "Sets gm level on account. Pass it username and 0,1,2,3,az, etc." },
        {"account mute",                  "a", 2, wrap(&ChatHandler::handleAccountMuteCommand),          "Mutes account for <timeperiod>." },
        {"account unmute",                "a", 2, wrap(&ChatHandler::handleAccountUnmuteCommand),        "Unmutes account <x>" },
        {"account ban",                   "a", 1, wrap(&ChatHandler::handleAccountBannedCommand),        "Bans account: .ban account <name> [duration] [reason]" },
        {"account unban",                 "z", 1, wrap(&ChatHandler::handleAccountUnbanCommand),         "Unbans account x." },
        {"account changepw",              "0", 1, wrap(&ChatHandler::handleAccountChangePassword),       "Change the password of your account." },
        {"account getid",                 "1", 1, wrap(&ChatHandler::handleAccountGetAccountID),         "Get Account ID for account name X" },

        {"achieve",                       "0", 0 },
#if VERSION_STRING > TBC
        {"achieve complete",              "m", 1, wrap(&ChatHandler::handleAchievementCompleteCommand),  "Completes the specified achievement." },
        {"achieve criteria",              "m", 1, wrap(&ChatHandler::handleAchievementCriteriaCommand),  "Completes the specified achievement criteria." },
        {"achieve reset",                 "m", 1, wrap(&ChatHandler::handleAchievementResetCommand),     "Resets achievement data from the target." },
#endif

        {"admin",                         "0", 0 },
        {"admin castall",                 "z", 1, wrap(&ChatHandler::HandleAdminCastAllCommand),         "Makes all players online cast spell <x>." },
        {"admin dispelall",               "z", 1, wrap(&ChatHandler::HandleAdminDispelAllCommand),       "Dispels all negative (or positive w/ 1) auras on all players." },
        {"admin masssummon",              "z", 0, wrap(&ChatHandler::HandleAdminMassSummonCommand),      "Summons all online players to you, use a/h for alliance/horde." },
        {"admin playall",                 "z", 1, wrap(&ChatHandler::HandleAdminPlayGlobalSoundCommand), "Plays a sound to everyone on the realm." },

        {"announce",                      "u", 1, wrap(&ChatHandler::HandleAnnounceCommand),             "Sends a normal chat message to all players." },
        {"appear",                        "v", 1, wrap(&ChatHandler::HandleAppearCommand),               "Teleports to x's position." },

        {"arena",                         "0", 0 },
        {"arena createteam",              "e", 2, wrap(&ChatHandler::HandleArenaCreateTeam),             "Creates arena team with <type> <name>" },
        {"arena setteamleader",           "e", 0, wrap(&ChatHandler::HandleArenaSetTeamLeader),          "Sets the arena team leader for <type>" },
        {"arena resetallratings",         "z", 0, wrap(&ChatHandler::HandleArenaTeamResetAllRatings),    "Resets all arena teams to their default rating" },

        {"ban",                           "0", 0 },
        {"ban ip",                        "m", 1, wrap(&ChatHandler::HandleIPBanCommand),                "Bans IP by <address> [duration]" },
        {"ban character",                 "b", 1, wrap(&ChatHandler::HandleBanCharacterCommand),         "Bans character by <charname> [duration] [reason]" },
        {"ban all",                       "a", 1, wrap(&ChatHandler::HandleBanAllCommand),               "Bans all by <charname> [duration] [reason]" },

        {"battleground",                  "0", 0 },
        {"battleground forceinitqueue",   "z", 0, wrap(&ChatHandler::HandleBGForceInitQueueCommand),     "Forces init of all bgs with in queue." },
        {"battleground getqueue",         "z", 0, wrap(&ChatHandler::HandleBGGetQueueCommand),           "Gets common battleground queue information." },
        {"battleground info",             "e", 0, wrap(&ChatHandler::HandleBGInfoCommand),               "Displays information about current bg." },
        {"battleground leave",            "e", 0, wrap(&ChatHandler::HandleBGLeaveCommand),              "Leaves the current battleground." },
        {"battleground menu",             "e", 1, wrap(&ChatHandler::HandleBGMenuCommand),               "Shows BG Menu for selected player by type <x>" },
        {"battleground pause",            "e", 0, wrap(&ChatHandler::HandleBGPauseCommand),              "Pauses current battleground match." },
        {"battleground playsound",        "e", 1, wrap(&ChatHandler::HandleBGPlaySoundCommand),          "Plays sound to all players in bg <sound_id>" },
        {"battleground sendstatus",       "e", 1, wrap(&ChatHandler::HandleBGSendStatusCommand),         "Sends status of bg by type <x>" },
        {"battleground setscore",         "e", 1, wrap(&ChatHandler::HandleBGSetScoreCommand),           "Sets bg score <Teamid> <Score>." },
        {"battleground setworldstate",    "e", 1, wrap(&ChatHandler::HandleBGSetWorldStateCommand),      "Sets singe worldsate value." },
        {"battleground setworldstates",   "e", 1, wrap(&ChatHandler::HandleBGSetWorldStatesCommand),     "Sets multipe worldstate values for start/end id" },
        {"battleground start",            "e", 0, wrap(&ChatHandler::HandleBGStartCommand),              "Starts current battleground match." },

        {"blockappear",                   "v", 0, wrap(&ChatHandler::HandleBlockAppearCommand),          "Blocks appearance to your position." },
        {"blocksummon",                   "v", 0, wrap(&ChatHandler::HandleBlockSummonCommand),          "Blocks summons to others position." },

        {"character",                     "0", 0 },
        
        {"character add",                 "m", 0 },
        {"character add copper",          "m", 1, wrap(&ChatHandler::HandleCharAddCopperCommand),        "Adds x copper to character." },
        {"character add silver",          "m", 1, wrap(&ChatHandler::HandleCharAddSilverCommand),        "Adds x silver to character." },
        {"character add gold",            "m", 1, wrap(&ChatHandler::HandleCharAddGoldCommand),          "Adds x gold to character." },
        {"character add honorpoints",     "m", 1, wrap(&ChatHandler::HandleCharAddHonorPointsCommand),   "Adds x amount of honor points/currency" },
        {"character add honorkills",      "m", 1, wrap(&ChatHandler::HandleCharAddHonorKillCommand),     "Adds x amount of honor kills" },
        {"character add item",            "m", 1, wrap(&ChatHandler::HandleCharAddItemCommand),          "Adds item x count y" },
        {"character add itemset",         "m", 1, wrap(&ChatHandler::HandleCharAddItemSetCommand),       "Adds item set to inv." },
        
        {"character set",                 "m", 0 },
        {"character set allexplored",     "m", 0, wrap(&ChatHandler::HandleCharSetAllExploredCommand),   "Reveals the unexplored parts of the map." },
        {"character set gender",          "m", 1, wrap(&ChatHandler::HandleCharSetGenderCommand),        "Changes gender of target. 0=male, 1=female." },
        {"character set itemsrepaired",   "n", 0, wrap(&ChatHandler::HandleCharSetItemsRepairedCommand), "Sets all items repaired for selected player" },
        {"character set level",           "m", 1, wrap(&ChatHandler::HandleCharSetLevelCommand),         "Sets level of selected target to <x>." },
        {"character set name",            "m", 2, wrap(&ChatHandler::HandleCharSetNameCommand),          "Renames character x to y." },
        {"character set phase",           "m", 1, wrap(&ChatHandler::HandleCharSetPhaseCommand),         "Sets phase of selected player" },
        {"character set speed",           "m", 1, wrap(&ChatHandler::HandleCharSetSpeedCommand),         "Sets speed of the selected target to <x>." },
        {"character set standing",        "m", 2, wrap(&ChatHandler::HandleCharSetStandingCommand),      "Sets standing of faction x to y." },
        {"character set talentpoints",    "m", 1, wrap(&ChatHandler::HandleCharSetTalentpointsCommand),  "Sets available talent points of the target." },
        {"character set title",           "m", 1, wrap(&ChatHandler::HandleCharSetTitleCommand),         "Sets pvp title for target" },
        {"character set forcerename",     "m", 1, wrap(&ChatHandler::HandleCharSetForceRenameCommand),   "Forces char x to rename on next login" },
        {"character set customize",       "m", 1, wrap(&ChatHandler::HandleCharSetCustomizeCommand),     "Allows char x to customize on next login" },
        {"character set factionchange",   "m", 1, wrap(&ChatHandler::HandleCharSetFactionChangeCommand), "Allows char x to change the faction on next login" },
        {"character set racechange",      "m", 1, wrap(&ChatHandler::HandleCharSetCustomizeCommand),     "Allows char x to change the race on next login" },
        
        {"character list",                "m", 0 },
        {"character list skills",         "m", 0, wrap(&ChatHandler::HandleCharListSkillsCommand),       "Lists all the skills from a player" },
        {"character list spells",         "m", 0, wrap(&ChatHandler::handleCharListSpellsCommand),       "Lists all the spells from a player" },
        {"character list standing",       "m", 1, wrap(&ChatHandler::HandleCharListStandingCommand),     "Lists standing of faction x." },
        {"character list items",          "m", 0, wrap(&ChatHandler::HandleCharListItemsCommand),        "Lists items of selected Player" },
        {"character list kills",          "m", 0, wrap(&ChatHandler::HandleCharListKillsCommand),        "Lists all kills of selected Player" },
        {"character list instances",      "z", 0, wrap(&ChatHandler::HandleCharListInstanceCommand),     "Lists persistent instances of selected Player" },
        
        {"character clearcooldowns",      "m", 0, wrap(&ChatHandler::HandleCharClearCooldownsCommand),   "Clears all cooldowns for your class." },
        {"character demorph",             "m", 0, wrap(&ChatHandler::HandleCharDeMorphCommand),          "Demorphs from morphed model." },
        {"character levelup",             "m", 1, wrap(&ChatHandler::HandleCharLevelUpCommand),          "Player target will be levelup x levels" },
        {"character removeauras",         "m", 0, wrap(&ChatHandler::HandleCharRemoveAurasCommand),      "Removes all auras from target" },
        {"character removesickness",      "m", 0, wrap(&ChatHandler::HandleCharRemoveSickessCommand),    "Removes ressurrection sickness from target" },
        {"character learn",               "m", 1, wrap(&ChatHandler::HandleCharLearnCommand),            "Learns spell <x> or all available spells by race" },
        {"character unlearn",             "m", 1, wrap(&ChatHandler::HandleCharUnlearnCommand),          "Unlearns spell" },
        {"character learnskill",          "m", 1, wrap(&ChatHandler::HandleCharLearnSkillCommand),       "Learns skill id skillid opt: min max." },
        {"character advanceskill",        "m", 1, wrap(&ChatHandler::HandleCharAdvanceSkillCommand),     "Advances skill line x y times." },
        {"character removeskill",         "m", 1, wrap(&ChatHandler::HandleCharRemoveSkillCommand),      "Removes skill." },
        {"character increaseweaponskill", "m", 0, wrap(&ChatHandler::HandleCharIncreaseWeaponSkill),     "Increase equipped weapon skill x times." },
        {"character resetreputation",     "n", 0, wrap(&ChatHandler::HandleCharResetReputationCommand),  "Resets reputation to start levels." },
        {"character resetspells",         "n", 0, wrap(&ChatHandler::HandleCharResetSpellsCommand),      "Resets all spells of selected player." },
        {"character resettalents",        "n", 0, wrap(&ChatHandler::HandleCharResetTalentsCommand),     "Resets all talents of selected player." },
#if VERSION_STRING >= TBC
        {"character resetskills",         "n", 0, wrap(&ChatHandler::HandleCharResetSkillsCommand),      "Resets all skills." },
#endif
        {"character removeitem",          "m", 1, wrap(&ChatHandler::HandleCharRemoveItemCommand),       "Removes item x count y." },
        {"character advanceallskills",    "m", 0, wrap(&ChatHandler::HandleAdvanceAllSkillsCommand),     "Advances all skills <x> points." },

        {"cheat",                         "0", 0 },
        {"cheat list",                    "m", 0, wrap(&ChatHandler::HandleCheatListCommand),            "Shows active cheats." },
        {"cheat taxi",                    "m", 0, wrap(&ChatHandler::HandleCheatTaxiCommand),            "Toggles TaxiCheat." },
        {"cheat cooldown",                "m", 0, wrap(&ChatHandler::HandleCheatCooldownCommand),        "Toggles CooldownCheat." },
        {"cheat casttime",                "m", 0, wrap(&ChatHandler::HandleCheatCastTimeCommand),        "Toggles CastTimeCheat." },
        {"cheat power",                   "m", 0, wrap(&ChatHandler::HandleCheatPowerCommand),           "Toggles PowerCheat. Disables mana consumption." },
        {"cheat god",                     "m", 0, wrap(&ChatHandler::HandleCheatGodCommand),             "Toggles GodCheat." },
        {"cheat fly",                     "m", 0, wrap(&ChatHandler::HandleCheatFlyCommand),             "Toggles FlyCheat." },
        {"cheat aurastack",               "m", 0, wrap(&ChatHandler::HandleCheatAuraStackCommand),       "Toggles AuraStackCheat." },
        {"cheat itemstack",               "m", 0, wrap(&ChatHandler::HandleCheatItemStackCommand),       "Toggles ItemStackCheat." },
        {"cheat triggerpass",             "m", 0, wrap(&ChatHandler::HandleCheatTriggerpassCommand),     "Ignores area trigger prerequisites." },

        {"commands",                      "0", 0, wrap(&ChatHandler::handleCommandsCommand),             "Shows commands" },

        {"debug",                         "0", 0 },
        {"debug dumpscripts",             "d", 0, wrap(&ChatHandler::HandleMoveHardcodedScriptsToDBCommand), "Dumps hardcoded aispells to cmdline for creatures on map X" },
        {"debug sendcreaturemove",        "d", 0, wrap(&ChatHandler::HandleDebugSendCreatureMove),           "Requests the target creature moves to you using movement manager." },
        {"debug dopctdamage",             "z", 0, wrap(&ChatHandler::HandleDoPercentDamageCommand),          "Do percent damage to creature target" },
        {"debug setscriptphase",          "z", 0, wrap(&ChatHandler::HandleSetScriptPhaseCommand),           "ScriptPhase test" },
        {"debug aicharge",                "z", 0, wrap(&ChatHandler::HandleAiChargeCommand),                 "AiCharge test" },
        {"debug aiknockback",             "z", 0, wrap(&ChatHandler::HandleAiKnockbackCommand),              "AiKnockBack test" },
        {"debug aijump",                  "z", 0, wrap(&ChatHandler::HandleAiJumpCommand),                   "AiJump test" },
        {"debug aifalling",               "z", 0, wrap(&ChatHandler::HandleAiFallingCommand),                "AiFalling test" },
        {"debug movetospawn",             "z", 0, wrap(&ChatHandler::HandleMoveToSpawnCommand),              "Move target to spwn" },
        {"debug position",                "z", 0, wrap(&ChatHandler::HandlePositionCommand),                 "Show position" },
        {"debug setorientation",          "z", 0, wrap(&ChatHandler::HandleSetOrientationCommand),           "Sets orientation on npc" },
        {"debug dumpmovement",            "d", 0, wrap(&ChatHandler::HandleDebugDumpMovementCommand),        "Dumps the player's movement information to chat" },
        {"debug infront",                 "d", 0, wrap(&ChatHandler::HandleDebugInFrontCommand),             "" },
        {"debug showreact",               "d", 0, wrap(&ChatHandler::HandleShowReactionCommand),             "" },
        {"debug aimove",                  "d", 0, wrap(&ChatHandler::HandleAIMoveCommand),                   "" },
        {"debug dist",                    "d", 0, wrap(&ChatHandler::HandleDistanceCommand),                 "" },
        {"debug face",                    "d", 0, wrap(&ChatHandler::HandleFaceCommand),                     "" },
        {"debug dumpstate",               "d", 0, wrap(&ChatHandler::HandleDebugDumpState),                  "" },
        {"debug moveinfo",                "d", 0, wrap(&ChatHandler::HandleDebugMoveInfo),                   "" },
        {"debug landwalk",                "d", 0, wrap(&ChatHandler::HandleDebugLandWalk),                   "Sets landwalk move for unit" },
        {"debug waterwalk",               "d", 0, wrap(&ChatHandler::HandleDebugWaterWalk),                  "Sets waterwal move for unit" },
        {"debug hover",                   "d", 0, wrap(&ChatHandler::HandleDebugHover),                      "Toggles hover move on/off for unit" },
        {"debug state",                   "d", 0, wrap(&ChatHandler::HandleDebugState),                      "Display MovementFlags for unit" },
        {"debug swim",                    "d", 0, wrap(&ChatHandler::HandleDebugSwim),                       "Toggles swim move for unit" },
        {"debug fly",                     "d", 0, wrap(&ChatHandler::HandleDebugFly),                        "Toggles fly move for unit" },
        {"debug disablegravity",          "d", 0, wrap(&ChatHandler::HandleDebugDisableGravity),             "Toggles disablegravitiy move for unit" },
        {"debug featherfall",             "d", 0, wrap(&ChatHandler::HandleDebugFeatherFall),                "Toggles featherfall move for unit" },
        {"debug speed",                   "d", 0, wrap(&ChatHandler::HandleDebugSpeed),                      "Sets move speed for unit. Usage: .debug speed <value>" },
        {"debug castspell",               "d", 0, wrap(&ChatHandler::HandleCastSpellCommand),                "Casts spell on target." },
        {"debug castself",                "d", 0, wrap(&ChatHandler::HandleCastSelfCommand),                 "Target casts spell <spellId> on itself." },
        {"debug castspellne",             "d", 0, wrap(&ChatHandler::HandleCastSpellNECommand),              "Casts spell by spellid on target (only plays animations)" },
        {"debug aggrorange",              "d", 0, wrap(&ChatHandler::HandleAggroRangeCommand),               "Shows aggro Range of the selected Creature." },
        {"debug knockback",               "d", 0, wrap(&ChatHandler::HandleKnockBackCommand),                "Knocks you back by <value>." },
        {"debug fade",                    "d", 0, wrap(&ChatHandler::HandleFadeCommand),                     "Calls ModThreatModifyer() with <value>." },
        {"debug threatMod",               "d", 0, wrap(&ChatHandler::HandleThreatModCommand),                "Calls ModGeneratedThreatModifyer() with <value>." },
        {"debug movefall",                "d", 0, wrap(&ChatHandler::HandleMoveFallCommand),                 "Makes the creature fall to the ground" },
        {"debug threatList",              "d", 0, wrap(&ChatHandler::HandleThreatListCommand),               "Returns all AI_Targets of the selected Creature." },
        {"debug gettptime",               "d", 0, wrap(&ChatHandler::HandleGetTransporterTime),              "Grabs transporter travel time" },
        {"debug dumpcoords",              "d", 0, wrap(&ChatHandler::HandleDebugDumpCoordsCommmand),         "" },
        {"debug rangecheck",              "d", 0, wrap(&ChatHandler::HandleRangeCheckCommand),               "Checks the range between the player and the target." },
        {"debug testlos",                 "d", 0, wrap(&ChatHandler::HandleCollisionTestLOS),                "Tests LoS" },
        {"debug testindoor",              "d", 0, wrap(&ChatHandler::HandleCollisionTestIndoor),             "Tests indoor" },
        {"debug getheight",               "d", 0, wrap(&ChatHandler::HandleCollisionGetHeight),              "Gets height" },
        {"debug deathstate",              "d", 0, wrap(&ChatHandler::HandleGetDeathState),                   "Returns current deathstate for target" },
        {"debug sendfailed",              "d", 0, wrap(&ChatHandler::HandleSendCastFailed),                  "Sends failed cast result <x>" },
        {"debug playmovie",               "d", 0, wrap(&ChatHandler::HandlePlayMovie),                       "Triggers a movie for selected player" },
        {"debug auraupdate",              "d", 0, wrap(&ChatHandler::HandleAuraUpdateAdd),                   "<SpellID> <Flags> <StackCount>" },
        {"debug auraremove",              "d", 0, wrap(&ChatHandler::HandleAuraUpdateRemove),                "Remove Auras in visual slot" },
        {"debug spawnwar",                "d", 0, wrap(&ChatHandler::HandleDebugSpawnWarCommand),            "Spawns desired amount of npcs to fight with eachother" },
        {"debug updateworldstate",        "d", 0, wrap(&ChatHandler::HandleUpdateWorldStateCommand),         "Sets the worldstate field to the specified value" },
        {"debug initworldstates",         "d", 0, wrap(&ChatHandler::HandleInitWorldStatesCommand),          "(Re)initializes the worldstates." },
        {"debug clearworldstates",        "d", 0, wrap(&ChatHandler::HandleClearWorldStatesCommand),         "Clears the worldstates" },
        {"debug pvpcredit",               "m", 0, wrap(&ChatHandler::HandleDebugPVPCreditCommand),           "Sends PVP credit packet, with specified rank and points" },
        {"debug calcdist",                "d", 0, wrap(&ChatHandler::HandleSimpleDistanceCommand),           "Displays distance between your position and x y z" },
        {"debug setunitbyte",             "d", 0, wrap(&ChatHandler::HandleDebugSetUnitByteCommand),         "Set value z for unit byte x with offset y." },
        {"debug setplayerflags",          "d", 0, wrap(&ChatHandler::HandleDebugSetPlayerFlagsCommand),      "Add player flags x to selected player" },
        {"debug getplayerflags",          "d", 0, wrap(&ChatHandler::HandleDebugGetPlayerFlagsCommand),      "Display current player flags of selected player x" },
        {"debug setweather",              "d", 0, wrap(&ChatHandler::HandleDebugSetWeatherCommand),          "Change zone weather <type> <densitiy>" },

        {"dismount",                      "h", 0, wrap(&ChatHandler::HandleDismountCommand),                "Dismounts targeted unit." },

        {"event",                         "0", 0 },
        {"event list",                    "m", 0, wrap(&ChatHandler::HandleEventListEvents),               "Shows list of currently active events" },
        {"event start",                   "m", 1, wrap(&ChatHandler::HandleEventStartEvent),               "Force start an event" },
        {"event stop",                    "m", 1, wrap(&ChatHandler::HandleEventStopEvent),                "Force stop an event" },
        {"event reset",                   "m", 1, wrap(&ChatHandler::HandleEventResetEvent),               "Resets force flags for an event" },
        {"event reload",                  "a", 0, wrap(&ChatHandler::HandleEventReloadAllEvents),          "Reloads all events from the database" },

        {"gm",                            "0", 0 },
        {"gm active",                     "t", 0, wrap(&ChatHandler::HandleGMActiveCommand),               "Activate/Deactivate <GM> tag" },
        {"gm allowwhispers",              "c", 1, wrap(&ChatHandler::HandleGMAllowWhispersCommand),        "Allows whispers from player <s>." },
        {"gm announce",                   "u", 1, wrap(&ChatHandler::HandleGMAnnounceCommand),             "Sends announce to all online GMs" },
        {"gm blockwhispers",              "c", 1, wrap(&ChatHandler::HandleGMBlockWhispersCommand),        "Blocks whispers from player <s>." },
        {"gm devtag",                     "1", 0, wrap(&ChatHandler::HandleGMDevTagCommand),               "Activate/Deactivate <DEV> tag" },
        {"gm list",                       "0", 0, wrap(&ChatHandler::HandleGMListCommand),                 "Shows active GM's" },
        {"gm logcomment",                 "1", 1, wrap(&ChatHandler::HandleGMLogCommentCommand),           "Adds a comment to the GM log." },

        {"gmTicket",                      "0", 0 },
#ifdef GM_TICKET_MY_MASTER_COMPATIBLE
        {"gmTicket get",                  "c", 0, wrap(&ChatHandler::HandleGMTicketListCommand),           "Gets GM Ticket list." },
        {"gmTicket getId",                "c", 1, wrap(&ChatHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket by player name." },
        {"gmTicket delId",                "c", 1, wrap(&ChatHandler::HandleGMTicketRemoveByIdCommand),     "Deletes GM Ticket by player name." },
#else
        {"gmTicket list",                 "c", 0, wrap(&ChatHandler::HandleGMTicketListCommand),           "Lists all active GM Tickets." },
        {"gmTicket get",                  "c", 1, wrap(&ChatHandler::HandleGMTicketGetByIdCommand),        "Gets GM Ticket with ID x." },
        {"gmTicket remove",               "c", 1, wrap(&ChatHandler::HandleGMTicketRemoveByIdCommand),     "Removes GM Ticket with ID x." },
        {"gmTicket deletepermanent",      "z", 1, wrap(&ChatHandler::HandleGMTicketDeletePermanentCommand),"Deletes GM Ticket with ID x permanently." },
        {"gmTicket assign",               "c", 2, wrap(&ChatHandler::HandleGMTicketAssignToCommand),       "Assigns GM Ticket with id x to GM y." },
        {"gmTicket release",              "c", 1, wrap(&ChatHandler::HandleGMTicketReleaseCommand),        "Releases assigned GM Ticket with ID x." },
        {"gmTicket comment",              "c", 2, wrap(&ChatHandler::HandleGMTicketCommentCommand),        "Sets comment x to GM Ticket with ID y." },
#endif
        {"gmTicket toggle",               "z", 0, wrap(&ChatHandler::HandleGMTicketToggleTicketSystemStatusCommand), "Toggles the ticket system status." },

        {"gobject",                       "0", 0 },
        {"gobject damage",                "o", 1, wrap(&ChatHandler::HandleGODamageCommand),               "Damages the GO for the specified hitpoints" },
        {"gobject delete",                "o", 0, wrap(&ChatHandler::HandleGODeleteCommand),               "Deletes selected GameObject" },
        {"gobject enable",                "o", 0, wrap(&ChatHandler::HandleGOEnableCommand),               "Enables the selected GO for use." },
        {"gobject export",                "o", 0, wrap(&ChatHandler::HandleGOExportCommand),               "Exports the selected GO to .sql file" },
        {"gobject info",                  "o", 0, wrap(&ChatHandler::HandleGOInfoCommand),                 "Gives you information about selected GO" },
        {"gobject movehere",              "g", 0, wrap(&ChatHandler::HandleGOMoveHereCommand),             "Moves gameobject to your position" },
        {"gobject open",                  "o", 0, wrap(&ChatHandler::HandleGOOpenCommand),                 "Toggles open/close (state) of selected GO." },
        {"gobject rebuild",               "o", 0, wrap(&ChatHandler::HandleGORebuildCommand),              "Rebuilds the GO." },
        {"gobject rotate",                "g", 0, wrap(&ChatHandler::HandleGORotateCommand),               "Rotates the object. <Axis> x,y, Default o." },
        {"gobject select",                "o", 0, wrap(&ChatHandler::HandleGOSelectCommand),               "Selects the nearest GameObject to you" },
        {"gobject selectguid",            "o", 1, wrap(&ChatHandler::HandleGOSelectGuidCommand),           "Selects GO with <guid>" },

        {"gobject set",                   "o", 0 },
        {"gobject set animprogress",      "o", 1, wrap(&ChatHandler::HandleGOSetAnimProgressCommand),      "Sets anim progress of selected GO" },
        {"gobject set faction",           "o", 1, wrap(&ChatHandler::HandleGOSetFactionCommand),           "Sets the faction of the GO" },
        {"gobject set flags",             "o", 1, wrap(&ChatHandler::HandleGOSetFlagsCommand),             "Sets the flags of the GO" },
        {"gobject set overrides",         "o", 1, wrap(&ChatHandler::HandleGOSetOverridesCommand),         "Sets override of selected GO" },
        {"gobject set phase",             "o", 1, wrap(&ChatHandler::HandleGOSetPhaseCommand),             "Sets phase of selected GO" },
        {"gobject set scale",             "o", 1, wrap(&ChatHandler::HandleGOSetScaleCommand),             "Sets scale of selected GO" },
        {"gobject set state",             "o", 1, wrap(&ChatHandler::HandleGOSetStateCommand),             "Sets the state byte of the GO" },
        {"gobject spawn",                 "o", 1, wrap(&ChatHandler::HandleGOSpawnCommand),               "Spawns a GameObject by ID" },

        {"gocreature",                    "v", 1, wrap(&ChatHandler::HandleGoCreatureSpawnCommand),        "Teleports you to the creature with <spwn_id>." },
        {"gogameobject",                  "v", 1, wrap(&ChatHandler::HandleGoGameObjectSpawnCommand),      "Teleports you to the gameobject with <spawn_id>." },
        {"gostartlocation",               "m", 1, wrap(&ChatHandler::HandleGoStartLocationCommand),        "Teleports you to a starting location" },
        {"gotrig",                        "v", 1, wrap(&ChatHandler::HandleGoTriggerCommand),              "Teleports you to the areatrigger with <id>." },
        {"gps",                           "0", 0, wrap(&ChatHandler::HandleGPSCommand),                    "Shows position of targeted unit" },

        {"guild",                         "0", 0 },
        {"guild create",                  "m", 1, wrap(&ChatHandler::HandleGuildCreateCommand),            "Creates a guild." },
        {"guild disband",                 "m", 0, wrap(&ChatHandler::HandleGuildDisbandCommand),           "Disbands the guild of your target." },
#if VERSION_STRING >= Cata
        {"guild info",                    "m", 0, wrap(&ChatHandler::HandleGuildInfoCommand),              "Shows guild info of your target." },
#endif
        {"guild join",                    "m", 1, wrap(&ChatHandler::HandleGuildJoinCommand),              "Force selected player to join a guild by name" },
        {"guild listmembers",             "m", 1, wrap(&ChatHandler::HandleGuildListMembersCommand),       "Lists guildmembers with ranks by guild name." },
        {"guild rename",                  "m", 1, wrap(&ChatHandler::HandleRenameGuildCommand),            "Renames a guild." },
        {"guild removeplayer",            "m", 0, wrap(&ChatHandler::HandleGuildRemovePlayerCommand),      "Removes a player from a guild." },

        {"help",                          "0", 0, wrap(&ChatHandler::handleHelpCommand),                   "Shows help for command" },

        {"instance",                      "0", 0 },
        {"instance create",               "z", 4, wrap(&ChatHandler::HandleCreateInstanceCommand),         "Creates instance by mapid x y z" },
        {"instance countcreature",        "z", 1, wrap(&ChatHandler::HandleCountCreaturesCommand),         "Returns number of creatures with entry x" },
        {"instance exit",                 "m", 0, wrap(&ChatHandler::HandleExitInstanceCommand),           "Exits current instance, return to entry point." },
        {"instance info",                 "m", 0, wrap(&ChatHandler::HandleGetInstanceInfoCommand),        "Gets info about instance with ID x (default current instance)." },
        {"instance reset",                "z", 1, wrap(&ChatHandler::HandleResetInstanceCommand),          "Removes instance ID x from target player." },
        {"instance resetall",             "m", 0, wrap(&ChatHandler::HandleResetAllInstancesCommand),      "Removes all instance IDs from target player." },
        {"instance shutdown",             "z", 0, wrap(&ChatHandler::HandleShutdownInstanceCommand),       "Shutdown instance with ID x (default is current instance)." },
        {"instance showtimers",           "m", 0, wrap(&ChatHandler::HandleShowTimersCommand),             "Show timers for current instance." },

        {"invincible",                    "j", 0, wrap(&ChatHandler::HandleInvincibleCommand),             "Toggles invincibility on/off" },
        {"invisible",                     "i", 0, wrap(&ChatHandler::HandleInvisibleCommand),              "Toggles invisibility and invincibility on/off" },

        {"kick",                          "0", 0 },
        {"kick player",                   "f", 1, wrap(&ChatHandler::HandleKickByNameCommand),             "Disconnects the player with name <s>." },
        {"kick account",                  "f", 1, wrap(&ChatHandler::HandleKKickBySessionCommand),         "Disconnects the session with account name <s>." },
        {"kick ip",                       "f", 1, wrap(&ChatHandler::HandleKickByIPCommand),               "Disconnects the session with the ip <s>." },
        {"kill",                          "r", 0, wrap(&ChatHandler::HandleKillCommand),                   "Kills selected unit or player by name" },

        {"lookup",                        "0", 0 },
        {"lookup achievement",            "l", 1, wrap(&ChatHandler::HandleLookupAchievementCommand),      "Looks up achievement string x." },
        {"lookup creature",               "l", 1, wrap(&ChatHandler::HandleLookupCreatureCommand),         "Looks up creature string x." },
        {"lookup faction",                "l", 1, wrap(&ChatHandler::HandleLookupFactionCommand),          "Looks up faction string x." },
        {"lookup item",                   "l", 1, wrap(&ChatHandler::HandleLookupItemCommand),             "Looks up item string x." },
        {"lookup object",                 "l", 1, wrap(&ChatHandler::HandleLookupObjectCommand),           "Looks up gameobject string x." },
        {"lookup quest",                  "l", 1, wrap(&ChatHandler::HandleLookupQuestCommand),            "Looks up quest string x." },
        {"lookup spell",                  "l", 1, wrap(&ChatHandler::HandleLookupSpellCommand),            "Looks up spell string x." },
        {"lookup skill",                  "l", 1, wrap(&ChatHandler::HandleLookupSkillCommand),            "Looks up skill string x." },

        {"modify",                        "0", 0 },
        {"modify hp",                     "m", 1, wrap(&ChatHandler::HandleModifyHp),                      "Mods health points (HP) of selected target" },
        {"modify mana",                   "m", 1, wrap(&ChatHandler::HandleModifyMana),                    "Mods mana points (MP) of selected target." },
        {"modify rage",                   "m", 1, wrap(&ChatHandler::HandleModifyRage),                    "Mods rage points of selected target." },
        {"modify energy",                 "m", 1, wrap(&ChatHandler::HandleModifyEnergy),                  "Mods energy points of selected target." },
#if VERSION_STRING >= WotLK
        {"modify runicpower",             "m", 1, wrap(&ChatHandler::HandleModifyRunicpower),              "Mods runic power points of selected target." },
#endif
        {"modify strength",               "m", 1, wrap(&ChatHandler::HandleModifyStrength),                "Mods strength value of the selected target." },
        {"modify agility",                "m", 1, wrap(&ChatHandler::HandleModifyAgility),                 "Mods agility value of the selected target." },
        {"modify intelligence",           "m", 1, wrap(&ChatHandler::HandleModifyIntelligence),            "Mods intelligence value of the selected target." },
        {"modify spirit",                 "m", 1, wrap(&ChatHandler::HandleModifySpirit),                  "Mods spirit value of the selected target." },
        {"modify armor",                  "m", 1, wrap(&ChatHandler::HandleModifyArmor),                   "Mods armor of selected target." },
        {"modify holy",                   "m", 1, wrap(&ChatHandler::HandleModifyHoly),                    "Mods holy resistance of selected target." },
        {"modify fire",                   "m", 1, wrap(&ChatHandler::HandleModifyFire),                    "Mods fire resistance of selected target." },
        {"modify nature",                 "m", 1, wrap(&ChatHandler::HandleModifyNature),                  "Mods nature resistance of selected target." },
        {"modify frost",                  "m", 1, wrap(&ChatHandler::HandleModifyFrost),                   "Mods frost resistance of selected target." },
        {"modify shadow",                 "m", 1, wrap(&ChatHandler::HandleModifyShadow),                  "Mods shadow resistance of selected target." },
        {"modify arcane",                 "m", 1, wrap(&ChatHandler::HandleModifyArcane),                  "Mods arcane resistance of selected target." },
        {"modify damage",                 "m", 1, wrap(&ChatHandler::HandleModifyDamage),                  "Mods damage done by the selected target." },
        {"modify ap",                     "m", 1, wrap(&ChatHandler::HandleModifyAp),                      "Mods attack power of the selected target." },
        {"modify rangeap",                "m", 1, wrap(&ChatHandler::HandleModifyRangeap),                 "Mods range attack power of the selected target." },
        {"modify scale",                  "m", 1, wrap(&ChatHandler::HandleModifyScale),                   "Mods scale of the selected target." },
        {"modify nativedisplayid",        "m", 1, wrap(&ChatHandler::HandleModifyNativedisplayid),         "Mods native display identifier of the target." },
        {"modify displayid",              "m", 1, wrap(&ChatHandler::HandleModifyDisplayid),               "Mods display identifier (DisplayID) of the target." },
        {"modify flags",                  "m", 1, wrap(&ChatHandler::HandleModifyFlags),                   "Mods flags of the selected target." },
        {"modify faction",                "m", 1, wrap(&ChatHandler::HandleModifyFaction),                 "Mods faction template of the selected target." },
        {"modify dynamicflags",           "m", 1, wrap(&ChatHandler::HandleModifyDynamicflags),            "Mods dynamic flags of the selected target." },
#if VERSION_STRING < Cata
        {"modify happiness",              "m", 1, wrap(&ChatHandler::HandleModifyHappiness),               "Mods happiness value of the selected target." },
#endif
        {"modify boundingradius",         "m", 1, wrap(&ChatHandler::HandleModifyBoundingradius),          "Mods bounding radius of the selected target." },
        {"modify combatreach",            "m", 1, wrap(&ChatHandler::HandleModifyCombatreach),             "Mods combat reach of the selected target." },
        {"modify emotestate",             "m", 1, wrap(&ChatHandler::HandleModifyEmotestate),              "Mods Unit emote state of the selected target." },
        {"modify bytes0",                 "m", 1, wrap(&ChatHandler::HandleModifyBytes0),                  "Mods bytes0 entry of selected target." },
        {"modify bytes1",                 "m", 1, wrap(&ChatHandler::HandleModifyBytes1),                  "Mods bytes1 entry of selected target." },
        {"modify bytes2",                 "m", 1, wrap(&ChatHandler::HandleModifyBytes2),                  "Mods bytes2 entry of selected target." },

        {"mount",                         "m", 1, wrap(&ChatHandler::HandleMountCommand),                  "Mounts targeted unit with modelid x." },

        {"npc",                           "0", 0 },
        {"npc addagent",                  "n", 10, wrap(&ChatHandler::HandleNpcAddAgentCommand),            "Add ai agents to npc." },
        {"npc addtrainerspell",           "m", 1, wrap(&ChatHandler::HandleNpcAddTrainerSpellCommand),     "Add spells to trainer learn list." },
        {"npc appear",                    "n", 0, wrap(&ChatHandler::HandleNpcAppearCommand),              "Teleports you to the target NPC's location." },
        {"npc cast",                      "n", 1, wrap(&ChatHandler::HandleNpcCastCommand),                "Makes NPC cast <spellid>." },
        {"npc come",                      "n", 0, wrap(&ChatHandler::HandleNpcComeCommand),                "Makes NPC move to your position" },
        {"npc delete",                    "n", 0, wrap(&ChatHandler::HandleNpcDeleteCommand),              "Deletes mob from world optional from DB" },
        {"npc info",                      "n", 0, wrap(&ChatHandler::HandleNpcInfoCommand),                "Displays NPC information" },
        {"npc listAgent",                 "n", 0, wrap(&ChatHandler::HandleNpcListAIAgentCommand),         "List AIAgents of selected target." },
        {"npc listloot",                  "m", 0, wrap(&ChatHandler::HandleNpcListLootCommand),            "Displays possible loot for the selected NPC." },
        {"npc follow",                    "m", 0, wrap(&ChatHandler::HandleNpcFollowCommand),              "Sets NPC to follow you" },
        {"npc stopfollow",                "m", 0, wrap(&ChatHandler::HandleNpcStopFollowCommand),          "Sets NPC to not follow anything" },
        {"npc possess",                   "n", 0, wrap(&ChatHandler::HandlePossessCommand),                "Possess targeted NPC (mind control)" },
        {"npc unpossess",                 "n", 0, wrap(&ChatHandler::HandleUnPossessCommand),              "Unpossess any currently possessed npc." },
        {"npc return",                    "n", 0, wrap(&ChatHandler::HandleNpcReturnCommand),              "Returns NPC to spawnpoint." },
        {"npc respawn",                   "n", 0, wrap(&ChatHandler::HandleNpcRespawnCommand),             "Respawns a dead NPC from its corpse." },
        {"npc say",                       "n", 1, wrap(&ChatHandler::HandleNpcSayCommand),                 "Makes selected NPC say <text>." },
        {"npc select",                    "n", 0, wrap(&ChatHandler::HandleNpcSelectCommand),              "Selects closest NPC" },

        {"npc set",                       "0", 0 },
        {"npc set canfly",                "n", 0, wrap(&ChatHandler::HandleNpcSetCanFlyCommand),           "Toggles CanFly state" },
        {"npc set emote",                 "n", 1, wrap(&ChatHandler::HandleNpcSetEmoteCommand),            "Sets emote state" },
        {"npc set equip",                 "m", 1, wrap(&ChatHandler::HandleNpcSetEquipCommand),            "Sets equipment itemt" },
        {"npc set flags",                 "n", 1, wrap(&ChatHandler::HandleNpcSetFlagsCommand),            "Sets NPC flags" },
        {"npc set formationmaster",       "m", 0, wrap(&ChatHandler::HandleNpcSetFormationMasterCommand),  "Sets formation master." },
        {"npc set formationslave",        "m", 0, wrap(&ChatHandler::HandleNpcSetFormationSlaveCommand),   "Sets formation slave with distance and angle" },
        {"npc set formationclear",        "m", 0, wrap(&ChatHandler::HandleNpcSetFormationClearCommand),   "Removes formation from creature" },
        {"npc set phase",                 "n", 1, wrap(&ChatHandler::HandleNpcSetPhaseCommand),            "Sets phase for selected creature" },
        {"npc set standstate",            "m", 1, wrap(&ChatHandler::HandleNpcSetStandstateCommand),       "Sets standstate for selected creature" },
        {"npc set entry",                 "m", 1, wrap(&ChatHandler::HandleNpcChangeEntry),                "Sets a New Entry for selected creature" },

        {"npc spawn",                     "n", 1, wrap(&ChatHandler::HandleNpcSpawnCommand),               "Spawns NPC of entry <id>" },
        {"npc showtimers",                "m", 0, wrap(&ChatHandler::HandleNpcShowTimersCommand),          "Shows timers for selected creature" },
        {"npc vendoradditem",             "n", 1, wrap(&ChatHandler::HandleNpcVendorAddItemCommand),       "Adds item to vendor" },
        {"npc vendorremoveitem",          "n", 1, wrap(&ChatHandler::HandleNpcVendorRemoveItemCommand),    "Removes item from vendor." },
        {"npc yell",                      "n", 1, wrap(&ChatHandler::HandleNpcYellCommand),                "Makes selected NPC yell <text>." },

        {"pet",                           "0", 0 },
        {"pet create",                    "m", 1, wrap(&ChatHandler::HandlePetCreateCommand),              "Creates a pet with <entry>." },
        {"pet dismiss",                   "m", 0, wrap(&ChatHandler::HandlePetDismissCommand),             "Dismisses a pet by for selected player or selected pet." },
        {"pet rename",                    "m", 1, wrap(&ChatHandler::HandlePetRenameCommand),              "Renames a pet to <name>." },
        {"pet addspell",                  "m", 1, wrap(&ChatHandler::HandlePetAddSpellCommand),            "Teaches pet <spell>." },
        {"pet removespell",               "m", 1, wrap(&ChatHandler::HandlePetRemoveSpellCommand),         "Removes pet spell <spell>." },
        {"pet setlevel",                  "m", 1, wrap(&ChatHandler::HandlePetSetLevelCommand),            "Sets pet level to <level>." },

        {"playerinfo",                    "m", 0, wrap(&ChatHandler::HandlePlayerInfo),                    "Displays info for selected character or <charname>" },

        {"quest",                         "0", 0 },
        {"quest addboth",                 "2", 1, wrap(&ChatHandler::HandleQuestAddBothCommand),           "Add quest <id> to the targeted NPC as start & finish" },
        {"quest addfinish",               "2", 1, wrap(&ChatHandler::HandleQuestAddFinishCommand),         "Add quest <id> to the targeted NPC as finisher" },
        {"quest addstart",                "2", 1, wrap(&ChatHandler::HandleQuestAddStartCommand),          "Add quest <id> to the targeted NPC as starter" },
        {"quest delboth",                 "2", 1, wrap(&ChatHandler::HandleQuestDelBothCommand),           "Delete quest <id> from the targeted NPC as start & finish" },
        {"quest delfinish",               "2", 1, wrap(&ChatHandler::HandleQuestDelFinishCommand),         "Delete quest <id> from the targeted NPC as finisher" },
        {"quest delstart",                "2", 1, wrap(&ChatHandler::HandleQuestDelStartCommand),          "Delete quest <id> from the targeted NPC as starter" },
        {"quest complete",                "2", 1, wrap(&ChatHandler::HandleQuestFinishCommand),            "Complete/Finish quest <id>" },
        {"quest fail",                    "2", 1, wrap(&ChatHandler::HandleQuestFailCommand),              "Fail quest <id>" },
        {"quest finisher",                "2", 1, wrap(&ChatHandler::HandleQuestFinisherCommand),          "Lookup quest finisher for quest <id>" },
        {"quest item",                    "2", 1, wrap(&ChatHandler::HandleQuestItemCommand),              "Lookup itemid necessary for quest <id>" },
        {"quest list",                    "2", 1, wrap(&ChatHandler::HandleQuestListCommand),              "Lists the quests for the npc <id>" },
        {"quest load",                    "2", 0, wrap(&ChatHandler::HandleQuestLoadCommand),              "Loads quests from database" },
        {"quest giver",                   "2", 1, wrap(&ChatHandler::HandleQuestGiverCommand),             "Lookup quest giver for quest <id>" },
        {"quest remove",                  "2", 1, wrap(&ChatHandler::HandleQuestRemoveCommand),            "Removes the quest <id> from the targeted player" },
        {"quest reward",                  "2", 1, wrap(&ChatHandler::HandleQuestRewardCommand),            "Shows reward for quest <id>" },
        {"quest status",                  "2", 1, wrap(&ChatHandler::HandleQuestStatusCommand),            "Lists the status of quest <id>" },
        {"quest start",                   "2", 1, wrap(&ChatHandler::HandleQuestStartCommand),             "Starts quest <id>" },
        {"quest startspawn",              "2", 1, wrap(&ChatHandler::HandleQuestStarterSpawnCommand),      "Port to spawn location for quest <id> (starter)" },
        {"quest finishspawn",             "2", 1, wrap(&ChatHandler::HandleQuestFinisherSpawnCommand),     "Port to spawn location for quest <id> (finisher)" },

        {"recall",                        "0", 0 },
        {"recall list",                   "q", 0, wrap(&ChatHandler::HandleRecallListCommand),             "List recall locations" },
        {"recall add",                    "q", 1, wrap(&ChatHandler::HandleRecallAddCommand),              "Add a recall location" },
        {"recall del",                    "q", 1, wrap(&ChatHandler::HandleRecallDelCommand),              "Remove a recall location" },
        {"recall port",                   "q", 1, wrap(&ChatHandler::HandleRecallGoCommand),               "Ports you to recalled location" },
        {"recall portplayer",             "m", 2, wrap(&ChatHandler::HandleRecallPortPlayerCommand),       "Ports specified player to a recalled location" },
        {"recall portus",                 "m", 1, wrap(&ChatHandler::HandleRecallPortUsCommand),           "Ports you and the selected player to recalled location" },

        {"revive",                        "r", 0, wrap(&ChatHandler::HandleReviveCommand),                 "Revives you or a selected target or player by name" },
        {"root",                          "b", 0, wrap(&ChatHandler::HandleRootCommand),                   "Roots selected target." },

        {"server",                        "0", 0 },
        {"server info",                   "0", 0, wrap(&ChatHandler::HandleServerInfoCommand),             "Shows detailed Server info." },
        {"server rehash",                 "z", 0, wrap(&ChatHandler::HandleServerRehashCommand),           "Reloads config file." },
        {"server save",                   "s", 0, wrap(&ChatHandler::HandleServerSaveCommand),             "Save targeted or named player." },
        {"server saveall",                "s", 0, wrap(&ChatHandler::HandleServerSaveAllCommand),          "Save all online player." },
        {"server setmotd",                "m", 1, wrap(&ChatHandler::HandleServerSetMotdCommand),          "Sets server MessageOfTheDay." },
        {"server shutdown",               "z", 0, wrap(&ChatHandler::HandleServerShutdownCommand),         "Initiates server shutdown in <x> seconds." },
        {"server cancelshutdown",         "z", 0, wrap(&ChatHandler::HandleServerCancelShutdownCommand),   "Cancels a Server Restart/Shutdown." },
        {"server restart",                "z", 0, wrap(&ChatHandler::HandleServerRestartCommand),          "Initiates server restart in <x> seconds." },

        {"server reloadtable",                      "m", 0 },
        {"server reloadtable gameobjects",          "z", 0, wrap(&ChatHandler::HandleReloadGameobjectsCommand),       "Reload gameobjets" },
        {"server reloadtable creatures",            "z", 0, wrap(&ChatHandler::HandleReloadCreaturesCommand),         "Reload creatures" },
        {"server reloadtable areatriggers",         "z", 0, wrap(&ChatHandler::HandleReloadAreaTriggersCommand),      "Reload areatriggers table" },
        {"server reloadtable command_overrides",    "z", 0, wrap(&ChatHandler::HandleReloadCommandOverridesCommand),  "Reload command_overrides table" },
        {"server reloadtable fishing",              "z", 0, wrap(&ChatHandler::HandleReloadFishingCommand),           "Reload fishing table" },
        {"server reloadtable gossip_menu_option",   "z", 0, wrap(&ChatHandler::HandleReloadGossipMenuOptionCommand),  "Reload gossip_menu_option table" },
        {"server reloadtable graveyards",           "z", 0, wrap(&ChatHandler::HandleReloadGraveyardsCommand),        "Reload graveyards table" },
        {"server reloadtable items",                "z", 0, wrap(&ChatHandler::HandleReloadItemsCommand),             "Reload items table" },
        {"server reloadtable itempages",            "z", 0, wrap(&ChatHandler::HandleReloadItempagesCommand),         "Reload itempages table" },
        {"server reloadtable npc_script_text",      "z", 0, wrap(&ChatHandler::HandleReloadNpcScriptTextCommand),     "Reload npc_script_text table" },
        {"server reloadtable npc_gossip_text",      "z", 0, wrap(&ChatHandler::HandleReloadNpcTextCommand),           "Reload npc_gossip_text table" },
        {"server reloadtable pet_level_abilities",  "z", 0, wrap(&ChatHandler::HandleReloadPetLevelAbilitiesCommand), "Reload pet_level_abilities table" },
        {"server reloadtable player_xp_for_level",  "z", 0, wrap(&ChatHandler::HandleReloadPlayerXpForLevelCommand),  "Reload player_xp_for_level table" },
        {"server reloadtable points_of_interest",   "z", 0, wrap(&ChatHandler::HandleReloadPointsOfInterestCommand),  "Reload points_of_interest table" },
        {"server reloadtable quests",               "z", 0, wrap(&ChatHandler::HandleReloadQuestsCommand),            "Reload quests table" },
        {"server reloadtable spell_teleport_coords","z", 0, wrap(&ChatHandler::HandleReloadTeleportCoordsCommand),    "Reload teleport_coords table" },
        {"server reloadtable worldbroadcast",       "z", 0, wrap(&ChatHandler::HandleReloadWorldbroadcastCommand),    "Reload worldbroadcast table" },
        {"server reloadtable worldmap_info",        "z", 0, wrap(&ChatHandler::HandleReloadWorldmapInfoCommand),      "Reload worldmap_info table" },
        {"server reloadtable worldstring_tables",   "z", 0, wrap(&ChatHandler::HandleReloadWorldstringTablesCommand), "Reload worldstring_tables table" },
        {"server reloadtable zoneguards",           "z", 0, wrap(&ChatHandler::HandleReloadZoneguardsCommand),        "Reload zoneguards table" },

        {"server reloadscript",           "m", 0, wrap(&ChatHandler::HandleServerReloadScriptsCommand),    "" },

        {"summon",                        "v", 1, wrap(&ChatHandler::HandleSummonCommand),                 "Summons x to your position." },

        {"ticket",                        "0", 0 },
        {"ticket list",                   "c", 0, wrap(&ChatHandler::HandleTicketListCommand),             "Shows all active tickets" },
        {"ticket listall",                "c", 0, wrap(&ChatHandler::HandleTicketListAllCommand),          "Shows all tickets in the database" },
        {"ticket get",                    "c", 1, wrap(&ChatHandler::HandleTicketGetCommand),              "Returns the content of the specified ID" },
        {"ticket close",                  "c", 1, wrap(&ChatHandler::HandleTicketCloseCommand),            "Close ticket with specified ID" },
        {"ticket delete",                 "a", 1, wrap(&ChatHandler::HandleTicketDeleteCommand),           "Delete ticket by specified ID" },

        {"transport",                     "m", 0 },
        {"transport info",                "m", 0, wrap(&ChatHandler::HandleGetTransporterInfo),            "Displays the current transport info" },
        {"transport spawn",               "m", 1, wrap(&ChatHandler::HandleSpawnInstanceTransport),        "Spawns transport with entry/period in current instance" },
        {"transport start",               "m", 0, wrap(&ChatHandler::HandleStartTransport),                "Force starts the current transport" },
        {"transport stop",                "m", 0, wrap(&ChatHandler::HandleStopTransport),                 "Force stops the current transport" },
        {"transport getperiod",           "m", 0, wrap(&ChatHandler::HandleGetTransporterTime),            "Displays the current transport period in ms" },

        {"unban",                         "0", 0 },
        {"unban ip",                      "m", 1, wrap(&ChatHandler::HandleIPUnBanCommand),                "Deletes an address from the IP ban table: <address>" },
        {"unban character",               "b", 1, wrap(&ChatHandler::HandleUnBanCharacterCommand),         "Unbans character x" },
        {"unroot",                        "b", 0, wrap(&ChatHandler::HandleUnrootCommand),                 "Unroots selected target." },

        {"vehicle",                       "m", 0 },
#ifdef FT_VEHICLES
        {"vehicle ejectpassenger",        "m", 1, wrap(&ChatHandler::HandleVehicleEjectPassengerCommand),     "Ejects the passenger from the specified seat" },
        {"vehicle ejectallpassengers",    "m", 0, wrap(&ChatHandler::HandleVehicleEjectAllPassengersCommand), "Ejects all passengers from the vehicle" },
        {"vehicle installaccessories",    "m", 0, wrap(&ChatHandler::HandleVehicleInstallAccessoriesCommand), "Installs the accessories for the selected vehicle" },
        {"vehicle addpassenger",          "m", 1, wrap(&ChatHandler::HandleVehicleAddPassengerCommand),       "Adds a new NPC passenger to the vehicle" },
#endif

        {"wannounce",                     "u", 1, wrap(&ChatHandler::HandleWAnnounceCommand),              "Sends a widescreen announcement to all players." },

        {"waypoint",                      "0", 0 },
        {"waypoint add",                  "w", 0, wrap(&ChatHandler::HandleWayPointAddCommand),           "Add wp for selected creature at current pos." },
        {"waypoint delete",               "w", 0, wrap(&ChatHandler::HandleWayPointDeleteCommand),        "Deletes selected wp." },
        {"waypoint deleteall",            "w", 0, wrap(&ChatHandler::HandleWayPointDeleteAllCommand),     "Deletes all waypoints of selected creature." },
        {"waypoint hide",                 "w", 0, wrap(&ChatHandler::HandleWayPointHideCommand),          "Hide wp's for selected creature." },
        {"waypoint show",                 "w", 0, wrap(&ChatHandler::HandleWayPointShowCommand),          "Show wp's for selected creature <bool backwards>" },

        {"worldport",                     "v", 4, wrap(&ChatHandler::HandleWorldPortCommand),             "Teleports you to a location with mapid x y z" }
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
