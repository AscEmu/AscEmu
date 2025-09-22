/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ChatDefines.hpp"
#include "CommonTypes.hpp"
#include "AEVersion.hpp"
#include "Logging/StringFormat.hpp"

#include <string>
#include <optional>

class SkillNameMgr;
class Creature;
class WorldSession;
class Player;
class Unit;
struct ItemProperties;

class SERVER_DECL ChatCommandHandler
{
    friend class CommandTableStorage;

private:
    ChatCommandHandler();
    ~ChatCommandHandler();

public:
    static ChatCommandHandler& getInstance();
    void initialize();
    void finalize();

    ChatCommandHandler(ChatCommandHandler&&) = delete;
    ChatCommandHandler(ChatCommandHandler const&) = delete;
    ChatCommandHandler& operator=(ChatCommandHandler&&) = delete;
    ChatCommandHandler& operator=(ChatCommandHandler const&) = delete;

    int ParseCommands(const char* text, WorldSession* session);

    void sendSystemMessagePacket(WorldSession* _session, std::string& _message);

    // Variadic template version of systemMessage
    template<typename... Args>
    void systemMessage(WorldSession* _session, const std::string& _format, Args&&... _args)
    {
        // Use the custom StringFormat function to format the string
        std::string formattedMessage = AscEmu::StringFormat(_format, std::forward<Args>(_args)...);

        // Send the formatted message via packet
        sendSystemMessagePacket(_session, formattedMessage);
    }

    // Variadic template version of redSystemMessage
    template<typename... Args>
    void colorSystemMessage(WorldSession* _session, const std::string _colorCode, const std::string& _format, Args&&... _args)
    {
        // Use the custom StringFormat function to format the string
        const std::string formattedMessage = AscEmu::StringFormat(_format, std::forward<Args>(_args)...);
        std::string coloredMessage = _colorCode + formattedMessage + "|r";

        // Send the formatted message via packet
        sendSystemMessagePacket(_session, coloredMessage);
    }

    // Variadic template version of redSystemMessage
    template<typename... Args>
    void redSystemMessage(WorldSession* _session, const std::string& _format, Args&&... _args)
    {
        colorSystemMessage(_session, MSG_COLOR_LIGHTRED, _format, std::forward<Args>(_args)...);
    }

    // Variadic template version of greenSystemMessage
    template<typename... Args>
    void greenSystemMessage(WorldSession* _session, const std::string& _format, Args&&... _args)
    {
        colorSystemMessage(_session, MSG_COLOR_GREEN, _format, std::forward<Args>(_args)...);
    }

    // Variadic template version of blueSystemMessage
    template<typename... Args>
    void blueSystemMessage(WorldSession* _session, const std::string& _format, Args&&... _args)
    {
        colorSystemMessage(_session, MSG_COLOR_LIGHTBLUE, _format, std::forward<Args>(_args)...);
    }

    void SendMultilineMessage(WorldSession* m_session, const char* str);

    std::optional<std::string_view> normalizeCommandInput(const char* raw);
    bool resolveTopLevelAbbrev(std::string_view tok0, WorldSession* s, std::string& outTop) const;
    bool executeCommandFlat(std::string_view text, WorldSession* m_session);
    bool executeCommand(std::string_view text, WorldSession* m_session);

    void SendHighlightedName(WorldSession* m_session, const char* prefix, const char* full_name, std::string & lowercase_name, std::string & highlight, uint32_t id);

    // Helper
    static Player* GetSelectedPlayer(WorldSession* m_session, bool showerror = true, bool auto_self = false);
    Creature* GetSelectedCreature(WorldSession* m_session, bool showerror = true);
    Unit* GetSelectedUnit(WorldSession* m_session, bool showerror = true);
    uint32_t GetSelectedWayPointId(WorldSession* m_session);

    const char* GetMapTypeString(uint8_t type);
    const char* GetDifficultyString(uint8_t difficulty);
    const char* GetRaidDifficultyString(uint8_t diff);

    // For skill related GM commands
    std::unique_ptr<SkillNameMgr> SkillNameManager;

    // AccountCommands
    bool handleAccountCreate(const char* args, WorldSession* m_session);
    bool handleAccountChangePassword(const char* args, WorldSession* m_session);
    bool handleAccountBannedCommand(const char* args, WorldSession* m_session);
    bool handleAccountSetGMCommand(const char* args, WorldSession* m_session);
    bool handleAccountUnbanCommand(const char* args, WorldSession* m_session);
    bool handleAccountMuteCommand(const char* args, WorldSession* m_session);
    bool handleAccountUnmuteCommand(const char* args, WorldSession* m_session);
    bool handleAccountGetAccountID(const char* args, WorldSession* m_session);

#if VERSION_STRING > TBC
    // Achievement
    bool handleAchievementCompleteCommand(const char* args, WorldSession* m_session);
    bool handleAchievementCriteriaCommand(const char* args, WorldSession* m_session);
    bool handleAchievementResetCommand(const char* args, WorldSession* m_session);
#endif

    // Admin commands
    bool HandleAdminCastAllCommand(const char* args, WorldSession* m_session);
    bool HandleAdminDispelAllCommand(const char* args, WorldSession* m_session);
    bool HandleAdminMassSummonCommand(const char* args, WorldSession* m_session);
    bool HandleAdminPlayGlobalSoundCommand(const char* args, WorldSession* m_session);

    // Arena commands
    uint8_t GetArenaTeamInternalType(uint32_t type, WorldSession* m_session);
    bool HandleArenaCreateTeam(const char* args, WorldSession* m_session);
    bool HandleArenaSetTeamLeader(const char* args, WorldSession* m_session);
    bool HandleArenaTeamResetAllRatings(const char* /*args*/, WorldSession* /*m_session*/);

    // Battleground commands
    bool HandleBGForceInitQueueCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGGetQueueCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGInfoCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGLeaveCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGMenuCommand(const char* args, WorldSession* m_session);
    bool HandleBGPauseCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGPlaySoundCommand(const char* args, WorldSession* m_session);
    bool HandleBGStartCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleBGSendStatusCommand(const char* args, WorldSession* m_session);
    bool HandleBGSetScoreCommand(const char* args, WorldSession* m_session);
    bool HandleBGSetWorldStateCommand(const char* args, WorldSession* m_session);
    bool HandleBGSetWorldStatesCommand(const char* args, WorldSession* m_session);

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

    // Character
    int32_t getSpellIDFromLink(const char* spelllink);
    uint16_t getItemIDFromLink(const char* itemlink, uint32_t* itemid);

    bool HandleCharClearCooldownsCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharLevelUpCommand(const char* args, WorldSession* m_session);
    bool HandleCharUnlearnCommand(const char* args, WorldSession* m_session);
    bool HandleCharLearnSkillCommand(const char* args, WorldSession* m_session);
    bool HandleCharAdvanceSkillCommand(const char* args, WorldSession* m_session);
    bool HandleCharRemoveSkillCommand(const char* args, WorldSession* m_session);
    bool HandleCharRemoveAurasCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharRemoveSickessCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleAdvanceAllSkillsCommand(const char* args, WorldSession* m_session);
    //\todo Rewrite this commands
    bool HandleCharLearnCommand(const char* args, WorldSession* m_session);

    // Character add commands
    bool HandleCharAddCopperCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddSilverCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddGoldCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddItemCommand(const char* args, WorldSession* m_session);
    bool HandleCharAddItemSetCommand(const char* args, WorldSession* m_session);

    // Character set commands
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
    bool HandleCharResetTalentsCommand(const char* /*args*/, WorldSession* m_session);

#if VERSION_STRING >= TBC // support classic
    bool HandleCharResetSkillsCommand(const char* /*args*/, WorldSession* m_session);
#endif

    bool HandleCharRemoveItemCommand(const char* args, WorldSession* m_session);
    bool HandleCharIncreaseWeaponSkill(const char* args, WorldSession* m_session);
    bool HandleCharResetReputationCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharResetSpellsCommand(const char* args, WorldSession* m_session);

    // Character list commands
    bool HandleCharListSkillsCommand(const char* /*args*/, WorldSession* m_session);
    bool handleCharListSpellsCommand(const char* args, WorldSession* m_session);
    bool HandleCharListStandingCommand(const char* args, WorldSession* m_session);
    bool HandleCharListItemsCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharListKillsCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleCharListInstanceCommand(const char* /*args*/, WorldSession* m_session);

    // Debug
    bool HandleMoveDBCItemSetsToDB(const char* args, WorldSession* session);
    bool HandleMoveDB2ItemsToDB(const char* args, WorldSession* session);
    bool HandleDebugDumpState(const char* args, WorldSession* session);
    bool HandleDebugMoveInfo(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugHover(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugState(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugSwim(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugFly(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugDisableGravity(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugFeatherFall(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugWaterWalk(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugSpeed(const char* args, WorldSession* m_session);
    bool HandleDebugPVPCreditCommand(const char* args, WorldSession* m_session);
    bool HandleSendCastFailed(const char* args, WorldSession* m_session);
    bool HandleDebugSendCreatureMove(const char* args, WorldSession* m_session);
    bool HandleDebugSetUnitByteCommand(const char* args, WorldSession* m_session);
    bool HandleDebugSetPlayerFlagsCommand(const char* args, WorldSession* m_session);
    bool HandleDebugGetPlayerFlagsCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleDebugSetWeatherCommand(const char* args, WorldSession* m_session);

    // old debugcmds.cpp
    //\todo Rewrite these commands
    bool HandleMoveHardcodedScriptsToDBCommand(const char* args, WorldSession* session);
    bool HandleDoPercentDamageCommand(const char* args, WorldSession* session);
    bool HandleSetScriptPhaseCommand(const char* args, WorldSession* session);
    bool HandleAiChargeCommand(const char* /*args*/, WorldSession* session);
    bool HandleAiKnockbackCommand(const char* /*args*/, WorldSession* session);
    bool HandleAiJumpCommand(const char* /*args*/, WorldSession* session);
    bool HandleAiFallingCommand(const char* /*args*/, WorldSession* session);
    bool HandleMoveToSpawnCommand(const char* /*args*/, WorldSession* session);
    bool HandlePositionCommand(const char* /*args*/, WorldSession* session);
    bool HandleSetOrientationCommand(const char* args, WorldSession* session);

    bool HandleDebugDumpMovementCommand(const char* args, WorldSession* session);
    bool HandleDebugInFrontCommand(const char* args, WorldSession* m_session);
    bool HandleShowReactionCommand(const char* args, WorldSession* m_session);
    bool HandleDistanceCommand(const char* args, WorldSession* m_session);
    bool HandleAIMoveCommand(const char* args, WorldSession* m_session);
    bool HandleFaceCommand(const char* args, WorldSession* m_session);
    bool HandleDebugLandWalk(const char* args, WorldSession* m_session);
    bool HandleAggroRangeCommand(const char* args, WorldSession* m_session);
    bool HandleKnockBackCommand(const char* args, WorldSession* m_session);
    bool HandleFadeCommand(const char* args, WorldSession* m_session);
    bool HandleThreatModCommand(const char* args, WorldSession* m_session);
    bool HandleMoveFallCommand(const char* args, WorldSession* m_session);
    bool HandleThreatListCommand(const char* args, WorldSession* m_session);
    bool HandlePlayMovie(const char* args, WorldSession* m_session);
    bool HandleDebugDumpCoordsCommmand(const char* args, WorldSession* m_session);
    bool HandleDebugSpawnWarCommand(const char* args, WorldSession* m_session);
    bool HandleUpdateWorldStateCommand(const char* args, WorldSession* session);
    bool HandleInitWorldStatesCommand(const char* args, WorldSession* session);
    bool HandleClearWorldStatesCommand(const char* args, WorldSession* session);
    bool HandleAuraUpdateAdd(const char* args, WorldSession* m_session);
    bool HandleAuraUpdateRemove(const char* args, WorldSession* m_session);
    bool HandleRangeCheckCommand(const char* args, WorldSession* m_session);
    bool HandleSimpleDistanceCommand(const char* args, WorldSession* m_session);
    bool HandleCollisionTestIndoor(const char* args, WorldSession* m_session);
    bool HandleCollisionTestLOS(const char* args, WorldSession* m_session);
    bool HandleCollisionGetHeight(const char* args, WorldSession* m_session);
    bool HandleGetDeathState(const char* args, WorldSession* m_session);
    bool HandleCastSpellCommand(const char* args, WorldSession* m_session);
    bool HandleCastSpellNECommand(const char* args, WorldSession* m_session);
    bool HandleCastSelfCommand(const char* args, WorldSession* m_session);

    // GameEventMgr
    bool HandleEventListEvents(const char* args, WorldSession* m_session);
    bool HandleEventStartEvent(const char* args, WorldSession* m_session);
    bool HandleEventStopEvent(const char* args, WorldSession* m_session);
    bool HandleEventResetEvent(const char* args, WorldSession* m_session);
    bool HandleEventReloadAllEvents(const char* args, WorldSession* m_session);

    // GameMasterCommands
    bool HandleGMActiveCommand(const char* args, WorldSession* m_session);
    bool HandleGMAllowWhispersCommand(const char* args, WorldSession* m_session);
    bool HandleGMAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleGMBlockWhispersCommand(const char* args, WorldSession* m_session);
    bool HandleGMDevTagCommand(const char* args, WorldSession* m_session);
    bool HandleGMListCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleGMLogCommentCommand(const char* args, WorldSession* m_session);

    // go* commands
    bool HandleGoTriggerCommand(const char* args, WorldSession* m_session);
    bool HandleGoStartLocationCommand(const char* args, WorldSession* m_session);
    bool HandleGoCreatureSpawnCommand(const char* args, WorldSession* m_session);
    bool HandleGoGameObjectSpawnCommand(const char* args, WorldSession* m_session);

    // GameObjectCommands
    bool HandleGODamageCommand(const char* args, WorldSession* session);
    bool HandleGODeleteCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleGOEnableCommand(const char*  /*args*/, WorldSession* m_session);
    bool HandleGOExportCommand(const char* args, WorldSession* m_session);
    bool HandleGOInfoCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleGORotateCommand(const char* args, WorldSession* m_session);
    bool HandleGORebuildCommand(const char* /*args*/, WorldSession* session);
    bool HandleGOSelectGuidCommand(const char* args, WorldSession* m_session);
    bool HandleGOSelectCommand(const char* args, WorldSession* m_session);
    bool HandleGOMoveHereCommand(const char* args, WorldSession* m_session);
    bool HandleGOOpenCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleGOSpawnCommand(const char* args, WorldSession* m_session);
    // GameObject set commands
    bool HandleGOSetStateCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetFlagsCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetFactionCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetPhaseCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetScaleCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetAnimProgressCommand(const char* args, WorldSession* m_session);
    bool HandleGOSetOverridesCommand(const char* args, WorldSession* m_session);

    //old GMTicket commands
    //\todo Rewrite these commands
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

    //Guild Command
    bool HandleGuildCreateCommand(const char* args, WorldSession* m_session);
    bool HandleGuildDisbandCommand(const char* /*args*/, WorldSession* m_session);
#if VERSION_STRING >= Cata
    bool HandleGuildInfoCommand(const char* /*args*/, WorldSession* m_session);
#endif
    bool HandleGuildJoinCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleGuildListMembersCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleRenameGuildCommand(const char* args, WorldSession* m_session);
    bool HandleGuildRemovePlayerCommand(const char* /*args*/, WorldSession* m_session);

    // Instance
    bool HandleCreateInstanceCommand(const char* args, WorldSession* m_session);
    bool HandleCountCreaturesCommand(const char* args, WorldSession* m_session);
    bool HandleExitInstanceCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleResetAllInstancesCommand(const char* args, WorldSession* m_session);
    bool HandleResetInstanceCommand(const char* args, WorldSession* m_session);
    bool HandleShutdownInstanceCommand(const char* args, WorldSession* m_session);
    bool HandleGetInstanceInfoCommand(const char* args, WorldSession* m_session);
    bool HandleShowTimersCommand(const char* /*args*/, WorldSession* m_session);

    // Lookups
    bool HandleLookupAchievementCommand(const char* args, WorldSession* m_session);
    bool HandleLookupCreatureCommand(const char* args, WorldSession* m_session);
    bool HandleLookupFactionCommand(const char* args, WorldSession* m_session);

    void sendItemLinkToPlayer(ItemProperties const* iProto, WorldSession* pSession, bool ItemCount, Player* owner, uint32_t language = 0/*LANG_UNIVERSAL*/);

    bool HandleLookupItemCommand(const char* args, WorldSession* m_session);
    bool HandleLookupObjectCommand(const char* args, WorldSession* m_session);
    bool HandleLookupQuestCommand(const char* args, WorldSession* m_session);
    bool HandleLookupSpellCommand(const char* args, WorldSession* m_session);
    bool HandleLookupSkillCommand(const char* args, WorldSession* m_session);

    // Modify Commands
    bool HandleModifyHp(const char* args, WorldSession* session);
    bool HandleModifyMana(const char* args, WorldSession* session);
    bool HandleModifyRage(const char* args, WorldSession* session);
    bool HandleModifyEnergy(const char* args, WorldSession* session);
    bool HandleModifyRunicpower(const char* args, WorldSession* session);
    bool HandleModifyStrength(const char* args, WorldSession* session);
    bool HandleModifyAgility(const char* args, WorldSession* session);
    bool HandleModifyIntelligence(const char* args, WorldSession* session);
    bool HandleModifySpirit(const char* args, WorldSession* session);
    bool HandleModifyArmor(const char* args, WorldSession* session);
    bool HandleModifyHoly(const char* args, WorldSession* session);
    bool HandleModifyFire(const char* args, WorldSession* session);
    bool HandleModifyNature(const char* args, WorldSession* session);
    bool HandleModifyFrost(const char* args, WorldSession* session);
    bool HandleModifyShadow(const char* args, WorldSession* session);
    bool HandleModifyArcane(const char* args, WorldSession* session);
    bool HandleModifyDamage(const char* args, WorldSession* session);
    bool HandleModifyAp(const char* args, WorldSession* session);
    bool HandleModifyRangeap(const char* args, WorldSession* session);
    bool HandleModifyScale(const char* args, WorldSession* session);
    bool HandleModifyNativedisplayid(const char* args, WorldSession* session);
    bool HandleModifyDisplayid(const char* args, WorldSession* session);
    bool HandleModifyFlags(const char* args, WorldSession* session);
    bool HandleModifyFaction(const char* args, WorldSession* session);
    bool HandleModifyDynamicflags(const char* args, WorldSession* session);
#if VERSION_STRING < Cata
    bool HandleModifyHappiness(const char* args, WorldSession* session);
#endif
    bool HandleModifyBoundingradius(const char* args, WorldSession* session);
    bool HandleModifyCombatreach(const char* args, WorldSession* session);
    bool HandleModifyEmotestate(const char* args, WorldSession* session);
    bool HandleModifyBytes0(const char* args, WorldSession* session);
    bool HandleModifyBytes1(const char* args, WorldSession* session);
    bool HandleModifyBytes2(const char* args, WorldSession* session);

    void sendModifySystemMessage(WorldSession* session, Unit* unitTarget, std::string modType, uint32_t newValue, uint32_t oldValue);
    void sendModifySystemMessage(WorldSession* session, Unit* unitTarget, std::string modType, float newValue, float oldValue);

    // NPC Commands
    std::string getNpcFlagString(Creature* creature);

    bool HandleNpcAddAgentCommand(const char* args, WorldSession* m_session);
    bool HandleNpcAppearCommand(const char * _, WorldSession * __);
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
    bool HandleNpcVendorAddItemCommand(const char* args, WorldSession* m_session);
    bool HandleNpcVendorRemoveItemCommand(const char* args, WorldSession* m_session);
    bool HandleNpcChangeEntry(const char* args, WorldSession* m_session);
    //Zyres: not only for selected creature... players too!
    bool HandlePossessCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleUnPossessCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleNpcShowTimersCommand(const char* /*args*/, WorldSession* m_session);

    // NPC set commands
    bool HandleNpcSetCanFlyCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSetEquipCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSetEmoteCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSetFlagsCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSetFormationMasterCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleNpcSetFormationSlaveCommand(const char* args, WorldSession* m_session);
    bool HandleNpcSetFormationClearCommand(const char* args, WorldSession* m_session);
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
    bool HandleReloadPetLevelAbilitiesCommand(const char* /*args*/, WorldSession* m_session);
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
    bool HandleGetTransporterTime(const char* args, WorldSession* m_session);
    bool HandleGetTransporterInfo(const char* args, WorldSession* m_session);
    bool HandleSpawnInstanceTransport(const char* args, WorldSession* m_session);
    bool HandleStartTransport(const char* args, WorldSession* m_session);
    bool HandleStopTransport(const char* args, WorldSession* m_session);

    // MiscCommands
    bool handleCommandsCommand(const char* args, WorldSession* m_session);
    bool showHelpForCommand(WorldSession* m_session, const char* args);
    bool handleHelpCommand(const char* args, WorldSession* m_session);
    bool HandleKillCommand(const char* args, WorldSession* m_session);
    bool HandleReviveCommand(const char* args, WorldSession* m_session);
    bool HandleUnrootCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleRootCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleDismountCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleMountCommand(const char* args, WorldSession* m_session);
    bool HandleWorldPortCommand(const char* args, WorldSession* m_session);
    bool HandleGPSCommand(const char* args, WorldSession* m_session);
    bool HandleInvincibleCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleInvisibleCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleSummonCommand(const char* args, WorldSession* m_session);
    bool HandleBlockSummonCommand(const char* args, WorldSession* m_session);
    bool HandleAppearCommand(const char* args, WorldSession* m_session);
    bool HandleBlockAppearCommand(const char* args, WorldSession* m_session);
    bool HandleAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandleWAnnounceCommand(const char* args, WorldSession* m_session);
    bool HandlePlayerInfo(const char* args, WorldSession* m_session);
    bool HandleIPUnBanCommand(const char* args, WorldSession* m_session);
    bool HandleUnBanCharacterCommand(const char* args, WorldSession* m_session);
    bool HandleIPBanCommand(const char* args, WorldSession* m_session);
    bool HandleBanCharacterCommand(const char* args, WorldSession* m_session);
    bool HandleBanAllCommand(const char* args, WorldSession* m_session);
    //.kick
    bool HandleKickByNameCommand(const char* args, WorldSession* m_session);
    bool HandleKKickBySessionCommand(const char* args, WorldSession* m_session);
    bool HandleKickByIPCommand(const char* args, WorldSession* m_session);

    // Pet Commands
    bool HandlePetCreateCommand(const char* args, WorldSession* m_session);
    bool HandlePetDismissCommand(const char* args, WorldSession* m_session);
    bool HandlePetRenameCommand(const char* args, WorldSession* m_session);
    bool HandlePetAddSpellCommand(const char* args, WorldSession* m_session);
    bool HandlePetRemoveSpellCommand(const char* args, WorldSession* m_session);
    bool HandlePetSetLevelCommand(const char* args, WorldSession* m_session);

    //old QuestCommands.cpp
    //\todo Rewrite these commands
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

    //old RecallCommands.cpp
    //\todo Rewrite these commands
    bool HandleRecallListCommand(const char* args, WorldSession* m_session);
    bool HandleRecallGoCommand(const char* args, WorldSession* m_session);
    bool HandleRecallAddCommand(const char* args, WorldSession* m_session);
    bool HandleRecallDelCommand(const char* args, WorldSession* m_session);
    bool HandleRecallPortPlayerCommand(const char* args, WorldSession* m_session);
    bool HandleRecallPortUsCommand(const char* args, WorldSession* m_session);

#ifdef FT_VEHICLES
    // Vehicle
    bool HandleVehicleEjectPassengerCommand(const char* args, WorldSession* session);
    bool HandleVehicleEjectAllPassengersCommand(const char* /*args*/, WorldSession* session);
    bool HandleVehicleInstallAccessoriesCommand(const char* /*args*/, WorldSession* session);
    bool HandleVehicleAddPassengerCommand(const char* args, WorldSession* session);
#endif

    // Waypoint
    bool HandleWayPointAddCommand(const char* args, WorldSession* m_session);
    bool HandleWayPointDeleteCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleWayPointDeleteAllCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleWayPointHideCommand(const char* /*args*/, WorldSession* m_session);
    bool HandleWayPointShowCommand(const char* args, WorldSession* m_session);
};

#define sChatHandler ChatCommandHandler::getInstance()
