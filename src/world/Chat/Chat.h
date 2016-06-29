/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org>
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
 */

#ifndef _WOWSERVER_CHAT_H
#define _WOWSERVER_CHAT_H

#include "SkillNameMgr.h"

class ChatHandler;
class WorldSession;
class Player;
class Unit;

enum ChatMsg
{
    CHAT_MSG_ADDON                      = -1,
    CHAT_MSG_SYSTEM                     = 0,    // 28, CHAT_MSG_SYSTEM = 0x00
    CHAT_MSG_SAY                        = 1,
    CHAT_MSG_PARTY                      = 2,
    CHAT_MSG_RAID                       = 3,
    CHAT_MSG_GUILD                      = 4,
    CHAT_MSG_OFFICER                    = 5,
    CHAT_MSG_YELL                       = 6,
    CHAT_MSG_WHISPER                    = 7,
    CHAT_MSG_WHISPER_MOB                = 8,    // CHAT_MSG_WHISPER_INFORM
    CHAT_MSG_WHISPER_INFORM             = 9,    // CHAT_MSG_REPLY
    CHAT_MSG_EMOTE                      = 10,
    CHAT_MSG_TEXT_EMOTE                 = 11,
    CHAT_MSG_MONSTER_SAY                = 12,
    CHAT_MSG_MONSTER_PARTY              = 13,
    CHAT_MSG_MONSTER_YELL               = 14,
    CHAT_MSG_MONSTER_WHISPER            = 15,
    CHAT_MSG_MONSTER_EMOTE              = 16,
    CHAT_MSG_CHANNEL                    = 17,
    CHAT_MSG_CHANNEL_JOIN               = 18,
    CHAT_MSG_CHANNEL_LEAVE              = 19,
    CHAT_MSG_CHANNEL_LIST               = 20,
    CHAT_MSG_CHANNEL_NOTICE             = 21,
    CHAT_MSG_CHANNEL_NOTICE_USER        = 22,
    CHAT_MSG_AFK                        = 23,
    CHAT_MSG_DND                        = 24,
    CHAT_MSG_IGNORED                    = 25,
    CHAT_MSG_SKILL                      = 26,
    CHAT_MSG_LOOT                       = 27,
    CHAT_MSG_MONEY                      = 28,
    CHAT_MSG_OPENING                    = 29,
    CHAT_MSG_TRADESKILLS                = 30,
    CHAT_MSG_PET_INFO                   = 31,
    CHAT_MSG_COMBAT_MISC_INFO           = 32,
    CHAT_MSG_COMBAT_XP_GAIN             = 33,
    CHAT_MSG_COMBAT_HONOR_GAIN          = 34,
    CHAT_MSG_COMBAT_FACTION_CHANGE      = 35,
    CHAT_MSG_BG_EVENT_NEUTRAL           = 36,
    CHAT_MSG_BG_EVENT_ALLIANCE          = 37,
    CHAT_MSG_BG_EVENT_HORDE             = 38,
    CHAT_MSG_RAID_LEADER                = 39,
    CHAT_MSG_RAID_WARNING               = 40,
    CHAT_MSG_RAID_WARNING_WIDESCREEN    = 41,
    CHAT_MSG_RAID_BOSS_EMOTE            = 42,
    CHAT_MSG_FILTERED                   = 43,
    CHAT_MSG_BATTLEGROUND               = 44,
    CHAT_MSG_BATTLEGROUND_LEADER        = 45,
    CHAT_MSG_RESTRICTED                 = 46,
    CHAT_MSG_ACHIEVEMENT                = 48,
    CHAT_MSG_GUILD_ACHIEVEMENT          = 49,
    CHAT_MSG_PARTY_LEADER               = 51
};

enum Languages
{
    LANG_UNIVERSAL      = 0x00,
    LANG_ORCISH         = 0x01,
    LANG_DARNASSIAN     = 0x02,
    LANG_TAURAHE        = 0x03,
    LANG_DWARVISH       = 0x06,
    LANG_COMMON         = 0x07,
    LANG_DEMONIC        = 0x08,
    LANG_TITAN          = 0x09,
    LANG_THELASSIAN     = 0x0A,
    LANG_DRACONIC       = 0x0B,
    LANG_KALIMAG        = 0x0C,
    LANG_GNOMISH        = 0x0D,
    LANG_TROLL          = 0x0E,
    LANG_GUTTERSPEAK    = 0x21,
    LANG_DRAENEI        = 0x23,
    NUM_LANGUAGES       = 0x24
};


#define MSG_COLOR_LIGHTRED          "|cffff6060"
#define MSG_COLOR_LIGHTBLUE         "|cff00ccff"
#define MSG_COLOR_TORQUISEBLUE      "|cff00C78C"
#define MSG_COLOR_SPRINGGREEN       "|cff00FF7F"
#define MSG_COLOR_GREENYELLOW       "|cffADFF2F"
#define MSG_COLOR_BLUE              "|cff0000ff"
#define MSG_COLOR_PURPLE            "|cffDA70D6"
#define MSG_COLOR_GREEN             "|cff00ff00"
#define MSG_COLOR_RED               "|cffff0000"
#define MSG_COLOR_GOLD              "|cffffcc00"
#define MSG_COLOR_GOLD2             "|cffFFC125"
#define MSG_COLOR_GREY              "|cff888888"
#define MSG_COLOR_WHITE             "|cffffffff"
#define MSG_COLOR_SUBWHITE          "|cffbbbbbb"
#define MSG_COLOR_MAGENTA           "|cffff00ff"
#define MSG_COLOR_YELLOW            "|cffffff00"
#define MSG_COLOR_ORANGEY           "|cffFF4500"
#define MSG_COLOR_CHOCOLATE         "|cffCD661D"
#define MSG_COLOR_CYAN              "|cff00ffff"
#define MSG_COLOR_IVORY             "|cff8B8B83"
#define MSG_COLOR_LIGHTYELLOW       "|cffFFFFE0"
#define MSG_COLOR_SEXGREEN          "|cff71C671"
#define MSG_COLOR_SEXTEAL           "|cff388E8E"
#define MSG_COLOR_SEXPINK           "|cffC67171"
#define MSG_COLOR_SEXBLUE           "|cff00E5EE"
#define MSG_COLOR_SEXHOTPINK        "|cffFF6EB4"

int32 GetSpellIDFromLink(const char* spelllink);
uint16 GetItemIDFromLink(const char* itemlink, uint32* itemid);

class ChatCommand
{
    public:

        const char* Name;

        char CommandGroup;

        bool (ChatHandler::*Handler)(const char* args, WorldSession* m_session) ;

        std::string Help;

        ChatCommand* ChildCommands;

        uint32 NormalValueField;
        uint32 MaxValueField;

        /// ValueType: 0 = nothing, 1 = uint, 2 = float
        uint16 ValueType;
};

class SERVER_DECL CommandTableStorage : public Singleton<CommandTableStorage>
{
    // List command containers ex. .character is a container of .character additem
    ChatCommand* _modifyCommandTable;
    ChatCommand* _debugCommandTable;
    ChatCommand* _eventCommandTable;
    ChatCommand* _waypointCommandTable;
    ChatCommand* _GMTicketCommandTable;
    ChatCommand* _TicketCommandTable;
    ChatCommand* _GuildCommandTable;
    ChatCommand* _GameObjectSetCommandTable;
    ChatCommand* _GameObjectCommandTable;
    ChatCommand* _BattlegroundCommandTable;
    ChatCommand* _NPCSetCommandTable;
    ChatCommand* _NPCCommandTable;
    ChatCommand* _CheatCommandTable;
    ChatCommand* _accountCommandTable;
    ChatCommand* _petCommandTable;
    ChatCommand* _recallCommandTable;
    ChatCommand* _questCommandTable;
    ChatCommand* _serverCommandTable;
    ChatCommand* _reloadTableCommandTable;
    ChatCommand* _gmCommandTable;
    ChatCommand* _characterAddCommandTable;
    ChatCommand* _characterSetCommandTable;
    ChatCommand* _characterListCommandTable;
    ChatCommand* _characterCommandTable;
    ChatCommand* _lookupCommandTable;
    ChatCommand* _adminCommandTable;
    ChatCommand* _kickCommandTable;
    ChatCommand* _banCommandTable;
    ChatCommand* _unbanCommandTable;
    ChatCommand* _instanceCommandTable;
    ChatCommand* _arenaCommandTable;
    ChatCommand* _achievementCommandTable;
    ChatCommand* _vehicleCommandTable;
    ChatCommand* _transportCommandTable;
    ChatCommand* _commandTable;

    ChatCommand* GetSubCommandTable(const char* name);
    ChatCommand* GetCharSubCommandTable(const char* name);
    ChatCommand* GetNPCSubCommandTable(const char* name);
    ChatCommand* GetGOSubCommandTable(const char* name);
    ChatCommand* GetReloadCommandTable(const char* name);

    public:

        void Init();
        void Dealloc();
        void Load();
        void Override(const char* command, const char* level);
        inline ChatCommand* Get() { return _commandTable; }
};

class SERVER_DECL ChatHandler : public Singleton<ChatHandler>
{
    friend class CommandTableStorage;

    public:

        ChatHandler();
        ~ChatHandler();

        WorldPacket* FillMessageData(uint32 type, uint32 language,  const char* message, uint64 guid, uint8 flag = 0) const;
        WorldPacket* FillSystemMessageData(const char* message) const;

        int ParseCommands(const char* text, WorldSession* session);

        void SystemMessage(WorldSession* m_session, const char* message, ...);
        void ColorSystemMessage(WorldSession* m_session, const char* colorcode, const char* message, ...);
        void RedSystemMessage(WorldSession* m_session, const char* message, ...);
        void GreenSystemMessage(WorldSession* m_session, const char* message, ...);
        void BlueSystemMessage(WorldSession* m_session, const char* message, ...);
        bool hasStringAbbr(const char* s1, const char* s2);
        void SendMultilineMessage(WorldSession* m_session, const char* str);

        bool ExecuteCommandInTable(ChatCommand* table, const char* text, WorldSession* m_session);
        bool ShowHelpForCommand(WorldSession* m_session, ChatCommand* table, const char* cmd);
        void SendHighlightedName(WorldSession* m_session, const char* prefix, const char* full_name, std::string & lowercase_name, std::string & highlight, uint32 id);
        void SendItemLinkToPlayer(ItemProperties const* iProto, WorldSession* pSession, bool ItemCount, Player* owner, uint32 language = LANG_UNIVERSAL);

        ChatCommand* getCommandTable();

        //helper
        Player* GetSelectedPlayer(WorldSession* m_session, bool showerror = true, bool auto_self = false);
        Creature* GetSelectedCreature(WorldSession* m_session, bool showerror = true);
        Unit* GetSelectedUnit(WorldSession* m_session, bool showerror = true);
        uint32 GetSelectedWayPointId(WorldSession* m_session);
        std::string GetNpcFlagString(Creature* creature);
        const char* GetMapTypeString(uint8 type);
        const char* GetDifficultyString(uint8 difficulty);
        const char* GetRaidDifficultyString(uint8 diff);
        std::string MyConvertIntToString(const int arg);
        std::string MyConvertFloatToString(const float arg);
        // For skill related GM commands
        SkillNameMgr* SkillNameManager;

        // AccountCommands
        bool HandleAccountCreate(const char* args, WorldSession* m_session);
        bool HandleAccountChangePassword(const char* args, WorldSession* m_session);
        bool HandleAccountBannedCommand(const char* args, WorldSession* m_session);
        bool HandleAccountSetGMCommand(const char* args, WorldSession* m_session);
        bool HandleAccountUnbanCommand(const char* args, WorldSession* m_session);
        bool HandleAccountMuteCommand(const char* args, WorldSession* m_session);
        bool HandleAccountUnmuteCommand(const char* args, WorldSession* m_session);

        // Arena commands
        uint8 GetArenaTeamInternalType(uint32 type, WorldSession* m_session);
        bool HandleArenaCreateTeam(const char* args, WorldSession* m_session);
        bool HandleArenaSetTeamLeader(const char* args, WorldSession* m_session);
        bool HandleArenaTeamResetAllRatings(const char* /*args*/, WorldSession* /*m_session*/);

        // Cheat
        bool HandleCheatListCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatTaxiCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatCooldownCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatCastTimeCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatPowerCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatGodCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatFlyCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatAuraStackCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatItemStackCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCheatTriggerpassCommand(const char* /*args*/, WorldSession* m_session);

        //Character
        bool HandleCharClearCooldownsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharLevelUpCommand(const char* args, WorldSession* m_session);
        bool HandleCharUnlearnCommand(const char* args, WorldSession* m_session);
        bool HandleCharLearnSkillCommand(const char* args, WorldSession* m_session);
        bool HandleCharAdvanceSkillCommand(const char* args, WorldSession* m_session);
        bool HandleCharRemoveSkillCommand(const char* args, WorldSession* m_session);
        bool HandleCharRemoveAurasCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharRemoveSickessCommand(const char* /*args*/, WorldSession* m_session);

        //Character add commands
        bool HandleCharAddCopperCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddSilverCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddGoldCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddItemCommand(const char* args, WorldSession* m_session);
        bool HandleCharAddItemSetCommand(const char* args, WorldSession* m_session);

        //Character set commands
        bool HandleCharSetAllExploredCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharSetGenderCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetItemsRepairedCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetLevelCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetNameCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetPhaseCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetSpeedCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetStandingCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetTalentpointsCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetTitleCommand(const char* args, WorldSession* m_session);

        bool HandleCharSetForceRenameCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetCustomizeCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetFactionChangeCommand(const char* args, WorldSession* m_session);
        bool HandleCharSetRaceChangeCommand(const char* args, WorldSession* m_session);

        //Character list commands
        bool HandleCharListSkillsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharListStandingCommand(const char* args, WorldSession* m_session);
        bool HandleCharListItemsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharListKillsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleCharListInstanceCommand(const char* /*args*/, WorldSession* m_session);

        // Debug
        bool HandleDebugMoveInfo(const char* /*args*/, WorldSession* m_session);
        bool HandleDebugPVPCreditCommand(const char* args, WorldSession* m_session);

        // GameEventMgr
        bool HandleEventListEvents(const char* args, WorldSession* m_session);
        bool HandleEventStartEvent(const char* args, WorldSession* m_session);
        bool HandleEventStopEvent(const char* args, WorldSession* m_session);
        bool HandleEventResetEvent(const char* args, WorldSession* m_session);
        bool HandleEventReloadAllEvents(const char* args, WorldSession* m_session);

        //GameMasterCommands
        bool HandleGMActiveCommand(const char* args, WorldSession* m_session);
        bool HandleGMAllowWhispersCommand(const char* args, WorldSession* m_session);
        bool HandleGMAnnounceCommand(const char* args, WorldSession* m_session);
        bool HandleGMBlockWhispersCommand(const char* args, WorldSession* m_session);
        bool HandleGMDevTagCommand(const char* args, WorldSession* m_session);
        bool HandleGMListCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleGMLogCommentCommand(const char* args, WorldSession* m_session);

        //go* commands
        bool HandleGoTriggerCommand(const char* args, WorldSession* m_session);
        bool HandleGoCreatureSpawnCommand(const char* args, WorldSession* m_session);
        bool HandleGoGameObjectSpawnCommand(const char* args, WorldSession* m_session);

        //GameObjectCommands
        bool HandleGOSelectGuidCommand(const char* args, WorldSession* m_session);
        bool HandleGODamageCommand(const char* args, WorldSession* session);
        bool HandleGORebuildCommand(const char* /*args*/, WorldSession* session);
        bool HandleGOMoveHereCommand(const char* args, WorldSession* m_session);
        bool HandleGOOpenCommand(const char* /*args*/, WorldSession* m_session);

        bool HandleGOSetStateCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetFlagsCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetFactionCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetPhaseCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetScaleCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetAnimProgressCommand(const char* args, WorldSession* m_session);
        bool HandleGOSetOverridesCommand(const char* args, WorldSession* m_session);

        // Lookups
#ifdef ENABLE_ACHIEVEMENTS
        bool HandleLookupAchievementCommand(const char* args, WorldSession* m_session);
#endif
        bool HandleLookupCreatureCommand(const char* args, WorldSession* m_session);
        bool HandleLookupFactionCommand(const char* args, WorldSession* m_session);
        bool HandleLookupItemCommand(const char* args, WorldSession* m_session);
        bool HandleLookupObjectCommand(const char* args, WorldSession* m_session);
        bool HandleLookupQuestCommand(const char* args, WorldSession* m_session);
        bool HandleLookupSpellCommand(const char* args, WorldSession* m_session);
        bool HandleLookupSkillCommand(const char* args, WorldSession* m_session);

        // NPC Commands
        bool HandleNpcAddAgentCommand(const char* args, WorldSession* m_session);
        bool HandleNpcAddTrainerSpellCommand(const char* args, WorldSession* m_session);
        bool HandleNpcCastCommand(const char* args, WorldSession* m_session);
        bool HandleNpcComeCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcDeleteCommand(const char* args, WorldSession* m_session);
        bool HandleNpcFollowCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcInfoCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcListAIAgentCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcListLootCommand(const char* args, WorldSession* m_session);
        bool HandleNpcStopFollowCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcRespawnCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcReturnCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcSayCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSelectCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcSpawnCommand(const char* args, WorldSession* m_session);
        bool HandleNpcYellCommand(const char* args, WorldSession* m_session);
        //Zyres: not only for selected creature... players too!
        bool HandlePossessCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session);

        //NPC set commands
        bool HandleNpcSetCanFlyCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetEquipCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetEmoteCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetFlagsCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetFormationMasterCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleNpcSetFormationSlaveCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetFormationClearCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetOnGOCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetPhaseCommand(const char* args, WorldSession* m_session);
        bool HandleNpcSetStandstateCommand(const char* arg, WorldSession* m_session);

        // Server
        bool HandleServerInfoCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleServerRehashCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleServerSaveCommand(const char* args, WorldSession* m_session);
        bool HandleServerSaveAllCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleServerSetMotdCommand(const char* args, WorldSession* m_session);
        bool HandleServerShutdownCommand(const char* args, WorldSession* m_session);
        bool HandleServerCancelShutdownCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleServerRestartCommand(const char* args, WorldSession* m_session);
        bool HandleServerReloadScriptsCommand(const char* /*args*/, WorldSession* m_session);

        //Server reload commands
        bool HandleReloadGameobjectsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadCreaturesCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadAreaTriggersCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadCommandOverridesCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadFishingCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadGossipMenuOptionCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadGraveyardsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadItemsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadItempagesCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadNpcScriptTextCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadNpcTextCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadPlayerXpForLevelCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadPointsOfInterestCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadQuestsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadTeleportCoordsCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadWorldbroadcastCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadWorldmapInfoCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadWorldstringTablesCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleReloadZoneguardsCommand(const char* /*args*/, WorldSession* m_session);

        // Ticket
        bool HandleTicketListCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleTicketListAllCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleTicketGetCommand(const char* args, WorldSession* m_session);
        bool HandleTicketCloseCommand(const char* args, WorldSession* m_session);
        bool HandleTicketDeleteCommand(const char* args, WorldSession* m_session);

        // Transport
        bool HandleModPeriodCommand(const char* args, WorldSession* m_session);
        bool HandleGetTransporterTime(const char* args, WorldSession* m_session);
        bool HandleGetTransporterInfo(const char* args, WorldSession* m_session);
        bool HandleSpawnInstanceTransport(const char* args, WorldSession* m_session);
        bool HandleDespawnInstanceTransport(const char* args, WorldSession* m_session);
        bool HandleStartTransport(const char* args, WorldSession* m_session);
        bool HandleStopTransport(const char* args, WorldSession* m_session);

        // MiscCommands
        bool HandleKillCommand(const char* args, WorldSession* m_session);
        bool HandleReviveCommand(const char* args, WorldSession* m_session);
        bool HandleUnrootCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleRootCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleAutoSaveChangesCommand(const char* /*args*/, WorldSession* m_session);
        //.kick
        bool HandleKickByNameCommand(const char* args, WorldSession* m_session);
        bool HandleKKickBySessionCommand(const char* args, WorldSession* m_session);
        bool HandleKickByIPCommand(const char* args, WorldSession* m_session);

        //Waypoint
        bool HandleWayPointAddCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointAddFlyCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointChangeNumberCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointDeleteCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPointDeleteAllCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPointEmoteCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointFlagsCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointGenerateCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointHideCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPointInfoCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPpointMoveHereCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPointMoveTypeCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointSaveCommand(const char* /*args*/, WorldSession* m_session);
        bool HandleWayPointShowCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointSkinCommand(const char* args, WorldSession* m_session);
        bool HandleWayPointWaitCommand(const char* args, WorldSession* m_session);
        
        //////////////////////////////////////////////////////////////////////////////////////////
        // Everything under this line is untested/not rewritten.
        //\todo Rewrite these commands and move them to a proper file.
        //Unsorted
        bool HandleCastAllCommand(const char* args, WorldSession* m_session);
        bool HandleDispelAllCommand(const char* args, WorldSession* m_session);
        bool HandleMassSummonCommand(const char* args, WorldSession* m_session);
        bool HandleRenameAllCharacter(const char* args, WorldSession* m_session);
        bool HandleGlobalPlaySoundCommand(const char* args, WorldSession* m_session);
        bool HandleDebugDumpMovementCommand(const char* args, WorldSession* session);
        bool HandleDebugInFrontCommand(const char* args, WorldSession* m_session);
        bool HandleShowReactionCommand(const char* args, WorldSession* m_session);
        bool HandleAIMoveCommand(const char* args, WorldSession* m_session);
        bool HandleDistanceCommand(const char* args, WorldSession* m_session);
        bool HandleFaceCommand(const char* args, WorldSession* m_session);
        bool HandleSetBytesCommand(const char* args, WorldSession* m_session);
        bool HandleGetBytesCommand(const char* args, WorldSession* m_session);
        bool HandleDebugLandWalk(const char* args, WorldSession* m_session);
        bool HandleDebugWaterWalk(const char* args, WorldSession* m_session);
        bool HandleAggroRangeCommand(const char* args, WorldSession* m_session);
        bool HandleKnockBackCommand(const char* args, WorldSession* m_session);
        bool HandleFadeCommand(const char* args, WorldSession* m_session);
        bool HandleThreatModCommand(const char* args, WorldSession* m_session);
        bool HandleCalcThreatCommand(const char* args, WorldSession* m_session);
        bool HandleThreatListCommand(const char* args, WorldSession* m_session);
        bool HandleDebugDumpCoordsCommmand(const char* args, WorldSession* m_session);
        bool HandleSendpacket(const char* args, WorldSession* m_session);
        bool HandleSQLQueryCommand(const char* args, WorldSession* m_session);
        bool HandleRangeCheckCommand(const char* args, WorldSession* m_session);
        bool HandleSendFailed(const char* args, WorldSession* m_session);
        bool HandlePlayMovie(const char* args, WorldSession* m_session);
        bool HandleAuraUpdateAdd(const char* args, WorldSession* m_session);
        bool HandleAuraUpdateRemove(const char* args, WorldSession* m_session);
        bool HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session);
        bool HandleUpdateWorldStateCommand(const char* args, WorldSession* session);
        bool HandleInitWorldStatesCommand(const char* args, WorldSession* session);
        bool HandleClearWorldStatesCommand(const char* args, WorldSession* session);
        
        bool HandleGuildJoinCommand(const char* args, WorldSession* m_session);
        bool HandleGuildMembersCommand(const char* args, WorldSession* m_session);
        bool CreateGuildCommand(const char* args, WorldSession* m_session);
        bool HandleRenameGuildCommand(const char* args, WorldSession* m_session);
        bool HandleGuildRemovePlayerCommand(const char* args, WorldSession* m_session);
        bool HandleGuildDisbandCommand(const char* args, WorldSession* m_session);
        
        bool HandleItemCommand(const char* args, WorldSession* m_session);
        bool HandleItemRemoveCommand(const char* args, WorldSession* m_session);
        bool HandleStartBGCommand(const char* args, WorldSession* m_session);
        bool HandlePauseBGCommand(const char* args, WorldSession* m_session);
        bool HandleBGInfoCommnad(const char* args, WorldSession* m_session);
        bool HandleInvincibleCommand(const char* args, WorldSession* m_session);
        bool HandleInvisibleCommand(const char* args, WorldSession* m_session);
        bool HandleFixScaleCommand(const char* args, WorldSession* m_session);
#ifdef ENABLE_ACHIEVEMENTS
        bool HandleAchievementCompleteCommand(const char* args, WorldSession* m_session);
        bool HandleAchievementCriteriaCommand(const char* args, WorldSession* m_session);
        bool HandleAchievementResetCommand(const char* args, WorldSession* m_session);
#endif
        bool HandleHelpCommand(const char* args, WorldSession* m_session);
        bool HandleCommandsCommand(const char* args, WorldSession* m_session);

        bool HandleStartCommand(const char* args, WorldSession* m_session);
        bool HandleDismountCommand(const char* args, WorldSession* m_session);
        bool HandleRatingsCommand(const char* args, WorldSession* m_session);
        bool HandleSimpleDistanceCommand(const char* args, WorldSession* m_session);
        
        bool CmdSetValueField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args);
        bool CmdSetFloatField(WorldSession* m_session, uint32 field, uint32 fieldmax, const char* fieldname, const char* args);
        bool HandleSummonCommand(const char* args, WorldSession* m_session);
        bool HandleAppearCommand(const char* args, WorldSession* m_session);
        bool HandleAnnounceCommand(const char* args, WorldSession* m_session);
        bool HandleWAnnounceCommand(const char* args, WorldSession* m_session);
        bool HandleGPSCommand(const char* args, WorldSession* m_session);
        
        bool HandleCollisionTestIndoor(const char* args, WorldSession* m_session);
        bool HandleGetDeathState(const char* args, WorldSession* m_session);
        bool HandleCollisionTestLOS(const char* args, WorldSession* m_session);
        bool HandleCollisionGetHeight(const char* args, WorldSession* m_session);
        bool HandleMountCommand(const char* args, WorldSession* m_session);

        bool HandleSendItemPushResult(const char* args, WorldSession* m_session);
        
        bool HandleModifyValueCommand(const char* args, WorldSession* m_session);
        bool HandleModifyBitCommand(const char* args, WorldSession* m_session);
        bool HandleBattlegroundExitCommand(const char* args, WorldSession* m_session);
        bool HandleExitInstanceCommand(const char* args, WorldSession* m_session);
        
        bool HandleRecallListCommand(const char* args, WorldSession* m_session);
        bool HandleRecallGoCommand(const char* args, WorldSession* m_session);
        bool HandleRecallAddCommand(const char* args, WorldSession* m_session);
        bool HandleRecallDelCommand(const char* args, WorldSession* m_session);
        bool HandleRecallPortPlayerCommand(const char* args, WorldSession* m_session);
        bool HandleRecallPortUsCommand(const char* args, WorldSession* m_session);
        bool HandleIPBanCommand(const char* args, WorldSession* m_session);
        bool HandleIPUnBanCommand(const char* args, WorldSession* m_session);
        bool HandleRemoveItemCommand(const char* args, WorldSession* m_session);
        
        bool HandleCreatePetCommand(const char* args, WorldSession* m_session);
        bool HandleAddPetSpellCommand(const char* args, WorldSession* m_session);
        bool HandleRemovePetSpellCommand(const char* args, WorldSession* m_session);
        bool HandleRenamePetCommand(const char* args, WorldSession* m_session);
        bool HandleDismissPetCommand(const char* args, WorldSession* m_session);
        bool HandlePetLevelCommand(const char* args, WorldSession* m_session);
#ifdef USE_SPECIFIC_AIAGENTS
        bool HandlePetSpawnAIBot(const char* args, WorldSession* m_session);
#endif
        bool HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session);
        bool HandleQuestAddBothCommand(const char* args, WorldSession* m_session);
        bool HandleQuestAddFinishCommand(const char* args, WorldSession* m_session);
        bool HandleQuestAddStartCommand(const char* args, WorldSession* m_session);
        bool HandleQuestDelBothCommand(const char* args, WorldSession* m_session);
        bool HandleQuestDelFinishCommand(const char* args, WorldSession* m_session);
        bool HandleQuestFinisherCommand(const char* args, WorldSession* m_session);
        bool HandleQuestDelStartCommand(const char* args, WorldSession* m_session);
        bool HandleQuestFinishCommand(const char* args, WorldSession* m_session);
        bool HandleQuestFailCommand(const char* args, WorldSession* m_session);
        bool HandleQuestGiverCommand(const char* args, WorldSession* m_session);
        bool HandleQuestItemCommand(const char* args, WorldSession* m_session);
        bool HandleQuestListCommand(const char* args, WorldSession* m_session);
        bool HandleQuestLoadCommand(const char* args, WorldSession* m_session);
        
        bool HandleQuestRemoveCommand(const char* args, WorldSession* m_session);
        bool HandleQuestRewardCommand(const char* args, WorldSession* m_session);
        bool HandleQuestStarterSpawnCommand(const char* args, WorldSession* m_session);
        bool HandleQuestFinisherSpawnCommand(const char* args, WorldSession* m_session);
        bool HandleQuestStartCommand(const char* args, WorldSession* m_session);
        bool HandleQuestStatusCommand(const char* args, WorldSession* m_session);
        bool HandleCreateInstanceCommand(const char* args, WorldSession* m_session);
        bool HandleResetAllInstancesCommand(const char* args, WorldSession* m_session);
        bool HandleResetInstanceCommand(const char* args, WorldSession* m_session);
        bool HandleShutdownInstanceCommand(const char* args, WorldSession* m_session);
        
        bool HandleGetInstanceInfoCommand(const char* args, WorldSession* m_session);
        bool HandleBanCharacterCommand(const char* args, WorldSession* m_session);
        bool HandleBanAllCommand(const char* args, WorldSession* m_session);
        bool HandleUnBanCharacterCommand(const char* args, WorldSession* m_session);
        bool HandleSetBGScoreCommand(const char* args, WorldSession* m_session);
        bool HandleInitializeAllQueuedBattlegroundsCommand(const char* args, WorldSession* m_session);
        bool HandleGetBattlegroundQueueCommand(const char* args, WorldSession* m_session);
        bool HandleGOSelect(const char* args, WorldSession* m_session);
        bool HandleGODelete(const char* args, WorldSession* m_session);
        bool HandleGOSpawn(const char* args, WorldSession* m_session);
        
        bool HandleGOInfo(const char* args, WorldSession* m_session);
        bool HandleGOEnable(const char* args, WorldSession* m_session);
        
        bool HandleGORotate(const char* args, WorldSession* m_session);
        
        bool HandleGOExport(const char* args, WorldSession* m_session);

        bool HandleWorldPortCommand(const char* args, WorldSession* m_session);
        bool HandleLearnCommand(const char* args, WorldSession* m_session);
        
        bool HandleGMTicketListCommand(const char* args, WorldSession* m_session);
        bool HandleGMTicketGetByIdCommand(const char* args, WorldSession* m_session);
        bool HandleGMTicketRemoveByIdCommand(const char* args, WorldSession* m_session);
#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
        void SendGMSurvey();
        bool HandleGMTicketAssignToCommand(const char* args, WorldSession* m_session);
        bool HandleGMTicketReleaseCommand(const char* args, WorldSession* m_session);
        bool HandleGMTicketCommentCommand(const char* args, WorldSession* m_session);
        bool HandleGMTicketDeletePermanentCommand(const char* args, WorldSession* m_session);
#endif
        bool HandleGMTicketToggleTicketSystemStatusCommand(const char* args, WorldSession* m_session);
        bool HandleAddSkillCommand(const char* args, WorldSession* m_session);
        
        bool HandleResetReputationCommand(const char* args, WorldSession* m_session);
        
        bool HandleIncreaseWeaponSkill(const char* args, WorldSession* m_session);
        bool HandleCastSpellCommand(const char* args, WorldSession* m_session);
        bool HandleCastSpellNECommand(const char* args, WorldSession* m_session);
        bool HandleCastSelfCommand(const char* args, WorldSession* m_session);

        bool HandleBattlegroundCommand(const char* args, WorldSession* m_session);
        bool HandleSetWorldStateCommand(const char* args, WorldSession* m_session);
        bool HandleSetWorldStatesCommand(const char* args, WorldSession* m_session);
        bool HandlePlaySoundCommand(const char* args, WorldSession* m_session);
        bool HandleSetBattlefieldStatusCommand(const char* args, WorldSession* m_session);
        bool HandleResetTalentsCommand(const char* args, WorldSession* m_session);
        bool HandleResetSpellsCommand(const char* args, WorldSession* m_session);
        bool HandleResetSkillsCommand(const char* args, WorldSession* m_session);
        bool HandleGetSkillLevelCommand(const char* args, WorldSession* m_session);
        
        bool HandlePlayerInfo(const char* args, WorldSession* m_session);
        bool HandleVehicleEjectPassengerCommand(const char* args, WorldSession* session);
        bool HandleVehicleEjectAllPassengersCommand(const char* args, WorldSession* session);
        bool HandleVehicleInstallAccessoriesCommand(const char* args, WorldSession* session);
        bool HandleVehicleRemoveAccessoriesCommand(const char* args, WorldSession* session);
        bool HandleVehicleAddPassengerCommand(const char* args, WorldSession* session);
        bool HandleAIAgentDebugBegin(const char* args, WorldSession* m_session);
        bool HandleAIAgentDebugContinue(const char* args, WorldSession* m_session);
        bool HandleAIAgentDebugSkip(const char* args, WorldSession* m_session);

};

#define sChatHandler ChatHandler::getSingleton()
#define sCommandTableStorag CommandTableStorage::getSingleton()

#endif // _WOWSERVER_CHAT_H
