/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

#ifndef WORLDSESSION_H
#define WORLDSESSION_H

#include <Threading/Mutex.h>
#include "Server/Packets/Opcode.h"
#include "Management/Quest.h"
#include "FastQueue.h"
#include "Units/Unit.h"
#include "Server/CharacterErrors.h"
#include "Data/Flags.h"
#if VERSION_STRING >= Cata
    #include "Management/AddonMgr.h"
    #include "Units/Players/PlayerDefines.hpp"
    #include "Units/Players/Player.h"
    struct AddonEntry;
#endif

#include <stddef.h>
#include <string>

class Player;
class WorldPacket;
class WorldSocket;
class WorldSession;
class MapMgr;
class Creature;
struct TrainerSpell;

template<class T, class LOCK>

class FastQueue;
class Mutex;

struct LfgUpdateData;       // forward declare
struct LfgJoinResultData;
struct LfgPlayerBoot;
struct LfgProposal;
struct LfgReward;
struct LfgRoleCheck;
struct AddonEntry;

#define CHECK_INWORLD_RETURN if (_player == NULL || !_player->IsInWorld()) { return; }

// Worldsocket related
#define WORLDSOCKET_TIMEOUT 120
#define PLAYER_LOGOUT_DELAY (20 * 1000) // 20 seconds should be more than enough.

struct OpcodeHandler
{
    uint16 status;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

enum SessionStatus
{
    STATUS_AUTHED = 0,
    STATUS_LOGGEDIN,
    //STATUS_LOGGEDIN_RECENTLY_LOGGOUT = 3,
};

///\todo refactoring these types. In use? Implement it!
enum AccountDataType
{
    GLOBAL_CONFIG_CACHE             = 0,                    // 0x01 g
    PER_CHARACTER_CONFIG_CACHE      = 1,                    // 0x02 p
    GLOBAL_BINDINGS_CACHE           = 2,                    // 0x04 g
    PER_CHARACTER_BINDINGS_CACHE    = 3,                    // 0x08 p
    GLOBAL_MACROS_CACHE             = 4,                    // 0x10 g
    PER_CHARACTER_MACROS_CACHE      = 5,                    // 0x20 p
    PER_CHARACTER_LAYOUT_CACHE      = 6,                    // 0x40 p
    PER_CHARACTER_CHAT_CACHE        = 7,                    // 0x80 p
    NUM_ACCOUNT_DATA_TYPES          = 8
};

const uint8 GLOBAL_CACHE_MASK        = 0x15;
const uint8 PER_CHARACTER_CACHE_MASK = 0xEA;

struct AccountDataEntry
{
    char* data;
    uint32 sz;
    bool bIsDirty;
};

struct CharCreate
{
    std::string name;
    uint8_t _race;
    uint8_t _class;
    uint8_t gender;
    uint8_t skin;
    uint8_t face;
    uint8_t hairStyle;
    uint8_t hairColor;
    uint8_t facialHair;
    uint8_t outfitId;
};

extern OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];
extern CharacterErrorCodes VerifyName(std::string name);

class SERVER_DECL WorldSession
{
    friend class WorldSocket;

    public:
        WorldSession(uint32 id, std::string name, WorldSocket* sock);
        ~WorldSession();

        Player* m_loggingInPlayer;

        void SendPacket(WorldPacket* packet);

        void SendPacket(StackBufferBase* packet);

        void OutPacket(uint16 opcode);

        void Delete();

        void SendChatPacket(WorldPacket* data, uint32 langpos, int32 lang, WorldSession* originator);

        uint32 m_currMsTime;
        uint32 m_lastPing;

        uint32 GetAccountId() const { return _accountId; }
        Player* GetPlayer() { return _player; }
        Player* GetPlayerOrThrow();

        // Acct flags
        void SetAccountFlags(uint32 flags) { _accountFlags = flags; }
        bool HasFlag(uint32 flag) { return (_accountFlags & flag) != 0; }
        uint32 GetFlags() { return _accountFlags; }

        // GM Permission System
        void LoadSecurity(std::string securitystring);
        void SetSecurity(std::string securitystring);
        char* GetPermissions() const { return permissions; }
        ptrdiff_t GetPermissionCount() const { return permissioncount; }
        bool HasPermissions() const { return (permissioncount > 0) ? true : false; }
        bool HasGMPermissions() const
        {
            if (!permissioncount)
                return false;

            return (strchr(permissions, 'a') != nullptr) ? true : false;
        }

        bool CanUseCommand(char cmdstr);

        void SetSocket(WorldSocket* sock)
        {
            _socket = sock;
        }
        void SetPlayer(Player* plr) { _player = plr; }

        void SetAccountData(uint32 index, char* data, bool initial, uint32 sz)
        {
            ARCEMU_ASSERT(index < 8);
            if (sAccountData[index].data)
            {
                delete[] sAccountData[index].data;
            }

            sAccountData[index].data = data;
            sAccountData[index].sz = sz;

            if (!initial && !sAccountData[index].bIsDirty)      // Mark as "changed" or "dirty"
            {
                sAccountData[index].bIsDirty = true;
            }
            else if (initial)
            {
                sAccountData[index].bIsDirty = false;
            }
        }

        AccountDataEntry* GetAccountData(uint32 index)
        {
            ARCEMU_ASSERT(index < 8);
            return &sAccountData[index];
        }

        void SetLogoutTimer(uint32 ms)
        {
            if (ms)
                _logoutTime = m_currMsTime + ms;
            else
                _logoutTime = 0;
        }

        void LogoutPlayer(bool Save);

        void QueuePacket(WorldPacket* packet);

        void OutPacket(uint16 opcode, uint16 len, const void* data);

        WorldSocket* GetSocket() { return _socket; }

        void Disconnect();

        uint8 Update(uint32 InstanceID);

        
        void SendNotification(const char* message, ...);


        void SetInstance(uint32 Instance) { instanceId = Instance; }
        uint32 GetLatency() const { return _latency; }
        std::string GetAccountName() { return _accountName; }
        const char* GetAccountNameS() const { return _accountName.c_str(); }
        const char* LocalizedWorldSrv(uint32 id);
        const char* LocalizedGossipOption(uint32 id);
        const char* LocalizedMapName(uint32 id);
        const char* LocalizedBroadCast(uint32 id);

#if VERSION_STRING != Cata
        uint32_t GetClientBuild() { return client_build; }
        void SetClientBuild(uint32_t build) { client_build = build; }
#else
        uint16_t GetClientBuild() { return client_build; }
        void SetClientBuild(uint16_t build) { client_build = build; }
#endif

        bool bDeleted;
        uint32 GetInstance() { return instanceId; }
        Mutex deleteMutex;
        int32 m_moveDelayTime;
        int32 m_clientTimeDelay;

        
        bool IsLoggingOut() { return _loggingOut; }

        //////////////////////////////////////////////////////////////////////////////////////////
        // Handlers (Already rewritten)

    protected:

        //////////////////////////////////////////////////////////////////////////////////////////
        // AreaTriggerHandler.cpp
        void handleAreaTriggerOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // ArenaTeamHandler.cpp
        void handleArenaTeamQueryOpcode(WorldPacket& recvPacket);
        void handleArenaTeamAddMemberOpcode(WorldPacket& recvPacket);
        void handleArenaTeamRemoveMemberOpcode(WorldPacket& recvPacket);
        void handleArenaTeamInviteAcceptOpcode(WorldPacket& /*recvPacket*/);
        void handleArenaTeamInviteDenyOpcode(WorldPacket& /*recvPacket*/);
        void handleArenaTeamLeaveOpcode(WorldPacket& recvPacket);
        void handleArenaTeamDisbandOpcode(WorldPacket& recvPacket);
        void handleArenaTeamPromoteOpcode(WorldPacket& recvPacket);
        void handleArenaTeamRosterOpcode(WorldPacket& recvPacket);
        void handleInspectArenaStatsOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // AuctionHandler.cpp
        void handleAuctionHelloOpcode(WorldPacket& recvPacket);
        void handleAuctionListItems(WorldPacket& recvPacket);
        void handleAuctionListBidderItems(WorldPacket& recvPacket);
        void handleAuctionSellItem(WorldPacket& recvPacket);
        void handleAuctionListOwnerItems(WorldPacket& recvPacket);
        void handleAuctionPlaceBid(WorldPacket& recvPacket);
        void handleCancelAuction(WorldPacket& recvPacket);
        void handleAuctionListPendingSales(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // BattlegroundHandler.cpp
    public:
        void sendBattlegroundList(Creature* creature, uint32_t mapId);

    protected:
        void handleBattlefieldPortOpcode(WorldPacket& recvPacket);
        void handleBattlefieldStatusOpcode(WorldPacket& /*recvPacket*/);
        void handleBattleMasterHelloOpcode(WorldPacket& recvPacket);
        void handleLeaveBattlefieldOpcode(WorldPacket& /*recvPacket*/);
        void handleAreaSpiritHealerQueryOpcode(WorldPacket& recvPacket);
        void handleAreaSpiritHealerQueueOpcode(WorldPacket& recvPacket);
        void handleBattlegroundPlayerPositionsOpcode(WorldPacket& /*recvPacket*/);
        void handleArenaJoinOpcode(WorldPacket& recvPacket);
        void handleBattleMasterJoinOpcode(WorldPacket& recvPacket);
        void handleInspectHonorStatsOpcode(WorldPacket& recvPacket);
        void handlePVPLogDataOpcode(WorldPacket& /*recvPacket*/);
        void handleBattlefieldListOpcode(WorldPacket& recvPacket);
#if VERSION_STRING >= Cata
        void handleRequestRatedBgInfoOpcode(WorldPacket& recvPacket);
        void handleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/);
        void handleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/);
        void handleRequestPvpOptionsOpcode(WorldPacket& /*recvPacket*/);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // CalendarHandler.cpp
        // \todo handle it
#if VERSION_STRING > TBC
        void handleCalendarGetCalendar(WorldPacket& /*recvPacket*/);
        void handleCalendarComplain(WorldPacket& recvPacket);
        void handleCalendarGetNumPending(WorldPacket& /*recvPacket*/);
        void handleCalendarAddEvent(WorldPacket& recvPacket);
        void handleCalendarGetEvent(WorldPacket& recvPacket);
        void handleCalendarGuildFilter(WorldPacket& recvPacket);
        void handleCalendarArenaTeam(WorldPacket& recvPacket);
        void handleCalendarUpdateEvent(WorldPacket& recvPacket);
        void handleCalendarRemoveEvent(WorldPacket& recvPacket);
        void handleCalendarCopyEvent(WorldPacket& recvPacket);
        void handleCalendarEventInvite(WorldPacket& recvPacket);
        void handleCalendarEventRsvp(WorldPacket& recvPacket);
        void handleCalendarEventRemoveInvite(WorldPacket& recvPacket);
        void handleCalendarEventStatus(WorldPacket& recvPacket);
        void handleCalendarEventModeratorStatus(WorldPacket& recvPacket);
#endif
        //////////////////////////////////////////////////////////////////////////////////////////
        // ChannelHandler.cpp
        void handleChannelJoin(WorldPacket& recvPacket);
        void handleChannelLeave(WorldPacket& recvPacket);
        void handleChannelList(WorldPacket& recvPacket);
        void handleChannelPassword(WorldPacket& recvPacket);
        void handleChannelSetOwner(WorldPacket& recvPacket);
        void handleChannelOwner(WorldPacket& recvPacket);
        void handleChannelModerator(WorldPacket& recvPacket);
        void handleChannelUnmoderator(WorldPacket& recvPacket);
        void handleChannelMute(WorldPacket& recvPacket);
        void handleChannelUnmute(WorldPacket& recvPacket);
        void handleChannelInvite(WorldPacket& recvPacket);
        void handleChannelKick(WorldPacket& recvPacket);
        void handleChannelBan(WorldPacket& recvPacket);
        void handleChannelUnban(WorldPacket& recvPacket);
        void handleChannelAnnounce(WorldPacket& recvPacket);
        void handleChannelModerate(WorldPacket& recvPacket);
        void handleGetChannelMemberCount(WorldPacket& recvPacket);
        void handleChannelRosterQuery(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // CharacterHandler.cpp
    public:
        void loadPlayerFromDBProc(QueryResultVector& results);
        uint8_t deleteCharacter(WoWGuid guid);

        void initGMMyMaster();
        void sendServerStats();
        void fullLogin(Player* player);
        void characterEnumProc(QueryResult* result);
        void loadAccountDataProc(QueryResult* result);

    protected:
        void handleSetFactionAtWarOpcode(WorldPacket& recvPacket);
        void handleSetFactionInactiveOpcode(WorldPacket& recvPacket);
        void handleCharDeleteOpcode(WorldPacket& recvPacket);
        void handlePlayerLoginOpcode(WorldPacket& recvPacket);
        void handleCharRenameOpcode(WorldPacket& recvPacket);
        void handleCharCreateOpcode(WorldPacket& recvPacket);
        void handleDeclinedPlayerNameOpcode(WorldPacket& recvPacket); // declined names (Cyrillic client)
        void handleCharEnumOpcode(WorldPacket& /*recvPacket*/);
#if VERSION_STRING > TBC
        void handleCharFactionOrRaceChange(WorldPacket& recvPacket);
        void handleCharCustomizeLooksOpcode(WorldPacket& recvPacket);
#endif
        
        //////////////////////////////////////////////////////////////////////////////////////////
        // ChatHandler.cpp
    public:
        bool isSessionMuted();
        bool isFloodProtectionTriggered();

    protected:
        void handleMessageChatOpcode(WorldPacket& recvPacket);
        void handleTextEmoteOpcode(WorldPacket& recvPacket);
        void handleEmoteOpcode(WorldPacket& recvPacket);
        void handleReportSpamOpcode(WorldPacket& recvPacket);
        void handleChatIgnoredOpcode(WorldPacket& recvPacket);
        void handleChatChannelWatchOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // CombatHandler.cpp
        void handleAttackSwingOpcode(WorldPacket& recvPacket);
        void handleAttackStopOpcode(WorldPacket& /*recvPacket*/);

        //////////////////////////////////////////////////////////////////////////////////////////
        // DuelHandler.cpp
        void handleDuelAccepted(WorldPacket& /*recvPacket*/);
        void handleDuelCancelled(WorldPacket& /*recvPacket*/);

        //////////////////////////////////////////////////////////////////////////////////////////
        // GMTicketHandler.cpp
        void handleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void handleGMTicketUpdateOpcode(WorldPacket& recvPacket);
        void handleGMTicketDeleteOpcode(WorldPacket& /*recvPacket*/);
        void handleGMSurveySubmitOpcode(WorldPacket& recvPacket);
        void handleReportLag(WorldPacket& recvPacket);
        void handleGMTicketGetTicketOpcode(WorldPacket& /*recvPacket*/);
        void handleGMTicketSystemStatusOpcode(WorldPacket& /*recvPacket*/);
        void handleGMTicketToggleSystemStatusOpcode(WorldPacket& /*recvPacket*/);

        //////////////////////////////////////////////////////////////////////////////////////////
        // GroupHandler.cpp
#if VERSION_STRING >= Cata
    public:
        void sendEmptyGroupList(Player* player);

    private:
        void handleGroupInviteResponseOpcode(WorldPacket& recvPacket);
        void handleGroupSetRolesOpcode(WorldPacket& recvPacket);
        void handleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupRoleCheckBeginOpcode(WorldPacket& recvPacket);
#endif

        void handleGroupInviteOpcode(WorldPacket& recvPacket);
        void handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupAcceptOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupUninviteOpcode(WorldPacket& recvPacket);
        void handleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        void handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/);
        void handleMinimapPingOpcode(WorldPacket& recvPacket);
        void handleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        void handleLootMethodOpcode(WorldPacket& recvPacket);
        void handleSetPlayerIconOpcode(WorldPacket& recvPacket);
        void handlePartyMemberStatsOpcode(WorldPacket& recvPacket);
        void handleConvertGroupToRaidOpcode(WorldPacket& /*recvPacket*/);
        void handleRequestRaidInfoOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupChangeSubGroup(WorldPacket& recvPacket);
        void handleGroupAssistantLeader(WorldPacket& recvPacket);
        void handleGroupPromote(WorldPacket& recvPacket);
        void handleReadyCheckOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // GuildHandler.cpp
        void handleGuildQuery(WorldPacket& recvPacket);
        void handleInviteToGuild(WorldPacket& recvPacket);
#if VERSION_STRING < Cata
        void handleGuildInfo(WorldPacket& /*recvPacket*/);
#endif
        void handleSaveGuildEmblem(WorldPacket& recvPacket);
        void handleGuildAccept(WorldPacket& /*recvPacket*/);
        void handleGuildDecline(WorldPacket& /*recvPacket*/);
        void handleGuildRoster(WorldPacket& /*recvPacket*/);
        void handleGuildLeave(WorldPacket& /*recvPacket*/);
        void handleGuildDisband(WorldPacket& /*recvPacket*/);
        void handleGuildLog(WorldPacket& /*recvPacket*/);
        void handleGuildPermissions(WorldPacket& /*recvPacket*/);
        void handleGuildBankBuyTab(WorldPacket& recvPacket);
        void handleGuildBankLogQuery(WorldPacket& recvPacket);
        void handleSetGuildBankText(WorldPacket& recvPacket);
        void handleGuildLeader(WorldPacket& recvPacket);
        void handleGuildMotd(WorldPacket& recvPacket);
        void handleGuildAddRank(WorldPacket& recvPacket);
        void handleSetGuildInfo(WorldPacket& recvPacket);
        void handleGuildRemove(WorldPacket& recvPacket);
        void handleGuildPromote(WorldPacket& recvPacket);
        void handleGuildDemote(WorldPacket& recvPacket);
        void handleGuildBankWithdrawMoney(WorldPacket& recvPacket);
        void handleGuildBankDepositMoney(WorldPacket& recvPacket);
        void handleGuildBankUpdateTab(WorldPacket& recvPacket);
        void handleGuildBankSwapItems(WorldPacket& recvPacket);
        void handleGuildBankQueryTab(WorldPacket& recvPacket);
        void handleGuildBankerActivate(WorldPacket& recvPacket);
        void handleGuildSetRank(WorldPacket& recvPacket);

        //\brief this was an MSG opcode on versions < Cata.
        //       now it is split into CMSG and SMSG packets since cata.
        void handleGuildBankMoneyWithdrawn(WorldPacket& /*recvPacket*/);

        //\brief this was two seperated opcodes on versions < Cata.
        //       now it is one since cata.
#if VERSION_STRING < Cata
        void handleGuildSetPublicNote(WorldPacket& recvPacket);
        void handleGuildSetOfficerNote(WorldPacket& recvPacket);
#else
        void handleGuildSetNoteOpcode(WorldPacket& recvPacket);
#endif

        //\brief this was an empty opcodes on versions < Cata.
        //       now it has some content since cata.
#if VERSION_STRING < Cata
        void handleGuildDelRank(WorldPacket& /*recvPacket*/);
#else
        void handleGuildDelRank(WorldPacket& recvPacket);
#endif

        //\brief this was an MSG opcode on versions < Cata.
        //       now it is split into CMSG and SMSG packets since cata.
#if VERSION_STRING < Cata
        void handleGuildBankQueryText(WorldPacket& recvPacket);
#else
        void handleQueryGuildBankTabText(WorldPacket& recvPacket);
#endif

        void handleCharterShowSignatures(WorldPacket& recvPacket);
        void handleCharterOffer(WorldPacket& recvPacket);
        void handleCharterSign(WorldPacket& recvPacket);
        void handleCharterDecline(WorldPacket& recvPacket);
        void handleCharterRename(WorldPacket& recvPacket);
        void handleCharterTurnInCharter(WorldPacket& recvPacket);
        void handleCharterQuery(WorldPacket& recvPacket);
        void handleCharterBuy(WorldPacket& recvPacket);

#if VERSION_STRING >= Cata
        // Guild
        void handleGuildAssignRankOpcode(WorldPacket& recvPacket);
        void handleGuildQueryRanksOpcode(WorldPacket& recvPacket);
        void handleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/);
        void handleGuildQueryXPOpcode(WorldPacket& recvPacket);
        void handleGuildRequestPartyState(WorldPacket& recvPacket);
        void handleGuildRequestMaxDailyXP(WorldPacket& recvPacket);
        void handleAutoDeclineGuildInvites(WorldPacket& recvPacket);
        void handleGuildRewardsQueryOpcode(WorldPacket& recvPacket);
        void handleGuildQueryNewsOpcode(WorldPacket& recvPacket);
        void handleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket);
        void handleGuildSetGuildMaster(WorldPacket& recvPacket);

        // GuildFinder
        void handleGuildFinderAddRecruit(WorldPacket& recvPacket);
        void handleGuildFinderBrowse(WorldPacket& recvPacket);
        void handleGuildFinderDeclineRecruit(WorldPacket& recvPacket);
        void handleGuildFinderGetApplications(WorldPacket& /*recvPacket*/);
        void handleGuildFinderGetRecruits(WorldPacket& recvPacket);
        void handleGuildFinderPostRequest(WorldPacket& /*recvPacket*/);
        void handleGuildFinderRemoveRecruit(WorldPacket& recvPacket);
        void handleGuildFinderSetGuildPost(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // ItemHandler.cpp
    public:
        void sendInventoryList(Creature* pCreature);
        void sendBuyFailed(uint64_t guid, uint32_t itemid, uint8_t error);
        void sendSellItem(uint64_t vendorguid, uint64_t itemid, uint8_t error);

#if VERSION_STRING >= WotLK
        void sendRefundInfo(uint64_t guid);

    protected:
        void handleItemRefundInfoOpcode(WorldPacket& recvPacket);
        void handleItemRefundRequestOpcode(WorldPacket& recvPacket);
#endif
        void handleUseItemOpcode(WorldPacket& recvPacket);
        void handleSwapItemOpcode(WorldPacket& recvPacket);
        void handleSplitOpcode(WorldPacket& recvPacket);
        void handleSwapInvItemOpcode(WorldPacket& recvPacket);
        void handleDestroyItemOpcode(WorldPacket& recvPacket);
        void handleAutoEquipItemOpcode(WorldPacket& recvPacket);
        void handleAutoEquipItemSlotOpcode(WorldPacket& recvPacket);
        void handleItemQuerySingleOpcode(WorldPacket& recvPacket);
        void handleBuyBackOpcode(WorldPacket& recvPacket);
        void handleSellItemOpcode(WorldPacket& recvPacket);
        void handleBuyItemInSlotOpcode(WorldPacket& recvPacket);
        void handleBuyItemOpcode(WorldPacket& recvPacket);
        void handleListInventoryOpcode(WorldPacket& recvPacket);
        void handleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
        void handleReadItemOpcode(WorldPacket& recvPacket);
        void handleRepairItemOpcode(WorldPacket& recvPacket);
        void handleAutoBankItemOpcode(WorldPacket& recvPacket);
        void handleAutoStoreBankItemOpcode(WorldPacket& recvPacket);
        void handleCancelTemporaryEnchantmentOpcode(WorldPacket& recvPacket);
        void handleInsertGemOpcode(WorldPacket& recvPacket);
        void handleWrapItemOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void handleEquipmentSetUse(WorldPacket& recvPacket);
        void handleEquipmentSetSave(WorldPacket& recvPacket);
        void handleEquipmentSetDelete(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // LfgHandler.cpp
    public:
        void sendLfgUpdateSearch(bool update);
        void sendLfgDisabled();
        void sendLfgOfferContinue(uint32_t dungeonEntry);
        void sendLfgTeleportError(uint8_t error);
        void sendLfgJoinResult(const LfgJoinResultData& joinData);
        void sendLfgUpdatePlayer(const LfgUpdateData& updateData);
        void sendLfgUpdateParty(const LfgUpdateData& updateData);
        void sendLfgRoleChosen(uint64_t guid, uint8_t roles);
        void sendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck);
        void sendLfgQueueStatus(uint32_t dungeon, int32_t waitTime, int32_t avgWaitTime, int32_t waitTimeTanks, int32_t waitTimeHealer, int32_t waitTimeDps, uint32_t queuedTime, uint8_t tanks, uint8_t healers, uint8_t dps);
        void sendLfgPlayerReward(uint32_t RandomDungeonEntry, uint32_t DungeonEntry, uint8_t done, const LfgReward* reward, QuestProperties const* qReward);
        void sendLfgBootPlayer(const LfgPlayerBoot* pBoot);
        void sendLfgUpdateProposal(uint32_t proposalId, const LfgProposal *pProp);

    protected:
        void handleLfgSetCommentOpcode(WorldPacket& recvPacket);
#if VERSION_STRING >= Cata
        void handleLfgLockInfoOpcode(WorldPacket& recvPacket);
#endif
#if VERSION_STRING > TBC
        void handleLfgJoinOpcode(WorldPacket& recvPacket);
        void handleLfgLeaveOpcode(WorldPacket& recvPacket);
        void handleLfgSearchOpcode(WorldPacket& recvPacket);
        void handleLfgSearchLeaveOpcode(WorldPacket& recvPacket);
        void handleLfgProposalResultOpcode(WorldPacket& recvPacket);
        void handleLfgSetRolesOpcode(WorldPacket& recvPacket);
        void handleLfgSetBootVoteOpcode(WorldPacket& recvPacket);
        void handleLfgPlayerLockInfoRequestOpcode(WorldPacket& recvPacket);
        void handleLfgTeleportOpcode(WorldPacket& recvPacket);
        void handleLfgPartyLockInfoRequestOpcode(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // LootHandler.cpp
    public:
        Loot* getItemLootFromHighGuidType(WoWGuid wowGuid);
        Loot* getMoneyLootFromHighGuidType(WoWGuid wowGuid);

    protected:
        void handleAutostoreLootItemOpcode(WorldPacket& recvPacket);
        void handleLootMoneyOpcode(WorldPacket& /*recvPacket*/);
        void handleLootOpcode(WorldPacket& recvPacket);
        void handleLootReleaseOpcode(WorldPacket& recvPacket);
        void handleLootMasterGiveOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // MailHandler.cpp
        void handleGetMailOpcode(WorldPacket& /*recvPacket*/);
        void handleSendMailOpcode(WorldPacket& recvPacket);
        void handleTakeMoneyOpcode(WorldPacket& recvPacket);
        void handleTakeItemOpcode(WorldPacket& recvPacket);
        void handleMarkAsReadOpcode(WorldPacket& recvPacket);
        void handleReturnToSenderOpcode(WorldPacket& recvPacket);
        void handleMailDeleteOpcode(WorldPacket& recvPacket);
        void handleItemTextQueryOpcode(WorldPacket& recvPacket);
        void handleMailTimeOpcode(WorldPacket& /*recvPacket*/);
        void handleMailCreateTextItemOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // MiscHandler.cpp
    public:
        void sendAccountDataTimes(uint32 mask);
        void sendMOTD();
#if VERSION_STRING > TBC
        void sendClientCacheVersion(uint32 version);
#endif

    protected:
        void handleStandStateChangeOpcode(WorldPacket& recvPacket);
        void handleWhoOpcode(WorldPacket& recvPacket);
        void handleSetSelectionOpcode(WorldPacket& recvPacket);
        void handleTogglePVPOpcode(WorldPacket& /*recvPacket*/);
        void handleTutorialFlag(WorldPacket& recvPacket);
        void handleTutorialClear(WorldPacket& /*recvPacket*/);
        void handleTutorialReset(WorldPacket& /*recvPacket*/);
        void handleLogoutRequestOpcode(WorldPacket& /*recvPacket*/);
        void handleSetSheathedOpcode(WorldPacket& recvPacket);
        void handlePlayedTimeOpcode(WorldPacket& recvPacket);
        void handleSetActionButtonOpcode(WorldPacket& recvPacket);
        void handleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket);
        void handleRandomRollOpcode(WorldPacket& recvPacket);
        void handleRealmSplitOpcode(WorldPacket& recvPacket);
        void handleSetTaxiBenchmarkOpcode(WorldPacket& recvPacket);
        void handleWorldStateUITimerUpdate(WorldPacket& /*recvPacket*/);
        void handleGameobjReportUseOpCode(WorldPacket& recvPacket);
        void handleDungeonDifficultyOpcode(WorldPacket& recvPacket);
        void handleRaidDifficultyOpcode(WorldPacket& recvPacket);
        void handleSetAutoLootPassOpcode(WorldPacket& recvPacket);
        void handleSetActionBarTogglesOpcode(WorldPacket& recvPacket);
        void handleLootRollOpcode(WorldPacket& recvPacket);
        void handleOpenItemOpcode(WorldPacket& recvPacket);
        void handleDismountOpcode(WorldPacket& /*recvPacket*/);
        void handleToggleHelmOpcode(WorldPacket& /*recvPacket*/);
        void handleToggleCloakOpcode(WorldPacket& /*recvPacket*/);
        void handleResetInstanceOpcode(WorldPacket& /*recvPacket*/);
        void handleSetTitle(WorldPacket& recvPacket);
        void handleZoneupdate(WorldPacket& recvPacket);
        void handleResurrectResponse(WorldPacket& recvPacket);
        void handleSelfResurrect(WorldPacket& /*recvPacket*/);
        void handleUpdateAccountData(WorldPacket& recvPacket);
        void handleRequestAccountData(WorldPacket& recvPacket);
        void handleBugOpcode(WorldPacket& recvPacket);
#if VERSION_STRING >= Cata
        void handleSuggestionOpcode(WorldPacket& recvPacket);
        void handleReturnToGraveyardOpcode(WorldPacket& /*recvPacket*/);
        void handleLogDisconnectOpcode(WorldPacket& recvPacket);
#endif
        void handleCompleteCinematic(WorldPacket& /*recvPacket*/);
        void handleNextCinematic(WorldPacket& /*recvPacket*/);
        void handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/);
        void handleSummonResponseOpcode(WorldPacket& recvPacket);
        void handleLogoutCancelOpcode(WorldPacket& /*recvPacket*/);
        void handlePlayerLogoutOpcode(WorldPacket& /*recvPacket*/);
        void handleCorpseReclaimOpcode(WorldPacket& recvPacket);
#if VERSION_STRING >= Cata
        void handleLoadScreenOpcode(WorldPacket& recvPacket);
        void handleUITimeRequestOpcode(WorldPacket& /*recvPacket*/);
        void handleTimeSyncRespOpcode(WorldPacket& recvPacket);
        void handleObjectUpdateFailedOpcode(WorldPacket& recvPacket);
        void handleRequestHotfix(WorldPacket& recvPacket);
        void handleRequestCemeteryListOpcode(WorldPacket& /*recvPacket*/);
#endif
#if VERSION_STRING > TBC
        void handleRemoveGlyph(WorldPacket& recvPacket);
        void handleBarberShopResult(WorldPacket& recvPacket);
#endif
        void handleRepopRequestOpcode(WorldPacket& /*recvPacket*/);
        void handleWhoIsOpcode(WorldPacket& recvPacket);
        void handleAmmoSetOpcode(WorldPacket& recvPacket);
        void handleGameObjectUse(WorldPacket& recvPacket);
        void handleInspectOpcode(WorldPacket& recvPacket);

    //\todo move to seperated file
#if VERSION_STRING >= Cata
    private:
        bool isAddonMessageFiltered;
        std::vector<std::string> mRegisteredAddonPrefixesVector;
        typedef std::list<AddonEntry> AddonsList;
        AddonsList m_addonList;

    public:
        bool isAddonRegistered(const std::string& addon_name) const;
        void readAddonInfoPacket(ByteBuffer& recvPacket);
        void sendAddonInfo();

    protected:
        void handleUnregisterAddonPrefixesOpcode(WorldPacket& /*recvPacket*/);
        void handleAddonRegisteredPrefixesOpcode(WorldPacket& recvPacket);
        void handleReportOpcode(WorldPacket& recvPacket);
        void handleReportPlayerOpcode(WorldPacket& recvPacket);
#endif

        void HandleMirrorImageOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // MovementHandler.cpp
        void handleSetActiveMoverOpcode(WorldPacket& recvPacket);
        void handleMovementOpcodes(WorldPacket& recvPacket);
        void handleAcknowledgementOpcodes(WorldPacket& recvPacket);
        void handleWorldTeleportOpcode(WorldPacket& recvPacket);
        void handleMountSpecialAnimOpcode(WorldPacket& /*recvPacket*/);
        void handleMoveWorldportAckOpcode(WorldPacket& /*recvPacket*/);
        void handleMoveTeleportAckOpcode(WorldPacket& recvPacket);
        void handleMoveNotActiveMoverOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // NPCHandler.cpp
    public:
        void sendTabardHelp(Creature* creature);
        void sendBankerList(Creature* creature);
        void sendAuctionList(Creature* creature);
        void sendSpiritHealerRequest(Creature* creature);
        void sendCharterRequest(Creature* creature);
        void sendInnkeeperBind(Creature* creature);
        void sendTrainerList(Creature* creature);
        void sendStabledPetList(uint64_t npcguid);
#if VERSION_STRING < Cata
        uint8_t trainerGetSpellStatus(TrainerSpell* trainerSpell);
#else
        TrainerSpellState trainerGetSpellStatus(TrainerSpell* trainerSpell);
#endif

    protected:
        void handleTabardVendorActivateOpcode(WorldPacket& recvPacket);
        void handleBankerActivateOpcode(WorldPacket& recvPacket);
        void handleBuyBankSlotOpcode(WorldPacket& recvPacket);
        void handleTrainerListOpcode(WorldPacket& recvPacket);
        void handleTrainerBuySpellOpcode(WorldPacket& recvPacket);
        void handleCharterShowListOpcode(WorldPacket& recvPacket);
        void handleGossipHelloOpcode(WorldPacket& recvPacket);
        void handleGossipSelectOptionOpcode(WorldPacket& recvPacket);
        void handleSpiritHealerActivateOpcode(WorldPacket& /*recvPacket*/);
        void handleNpcTextQueryOpcode(WorldPacket& recvPacket);
        void handleBinderActivateOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // PetHandler.cpp
        void handleStabledPetList(WorldPacket& recvPacket);
        void handlePetAction(WorldPacket& recvPacket);
        void handlePetNameQuery(WorldPacket& recvPacket);
        void handleBuyStableSlot(WorldPacket& /*recvPacket*/);
        void handleStablePet(WorldPacket& /*recvPacket*/);
        void handleUnstablePet(WorldPacket& recvPacket);
        void handleStableSwapPet(WorldPacket& recvPacket);
        void handlePetRename(WorldPacket& recvPacket);
        void handlePetAbandon(WorldPacket& /*recvPacket*/);
        void handlePetUnlearn(WorldPacket& recvPacket);
        void handlePetSpellAutocast(WorldPacket& recvPacket);
        void handlePetCancelAura(WorldPacket& recvPacket);
        void handlePetLearnTalent(WorldPacket& recvPacket);
        void handlePetSetActionOpcode(WorldPacket& recvPacket);
        void handleDismissCritter(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // QueryHandler.cpp
        void handleGameObjectQueryOpcode(WorldPacket& recvPacket);
        void handleQueryTimeOpcode(WorldPacket& recvPacket);
        void handleCreatureQueryOpcode(WorldPacket& recvPacket);
        void handleNameQueryOpcode(WorldPacket& recvPacket);
        void handleInrangeQuestgiverQuery(WorldPacket& /*recvPacket*/);
        void handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/);
        void handleItemNameQueryOpcode(WorldPacket& recvPacket);
        void handlePageTextQueryOpcode(WorldPacket& recvPacket);
        void handleAchievmentQueryOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // QuestHandler.cpp
    public:
        WorldPacket* buildQuestQueryResponse(QuestProperties const* qst);

    protected:
        void handleQuestPushResultOpcode(WorldPacket& recvPacket);
        void handleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket);
        void handleQuestQueryOpcode(WorldPacket& recvPacket);
        void handleQuestgiverCancelOpcode(WorldPacket& recvPacket);
        void handleQuestgiverHelloOpcode(WorldPacket& recvPacket);
        void handleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket);
        void handleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket);
        void handleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket);
        void handleQuestGiverQueryQuestOpcode(WorldPacket& recvPacket);
        void handleQuestgiverCompleteQuestOpcode(WorldPacket& recvPacket);
        void handleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket);
        void handlePushQuestToPartyOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void handleQuestPOIQueryOpcode(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // SkillHandler.cpp
        void handleUnlearnSkillOpcode(WorldPacket& recvPacket);
        void handleLearnTalentOpcode(WorldPacket& recvPacket);
        void handleUnlearnTalents(WorldPacket& recvPacket);
#if VERSION_STRING < Cata
        void handleLearnMultipleTalentsOpcode(WorldPacket& recvPacket);
#else
        void handleLearnPreviewTalentsOpcode(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // SocialHandler.cpp
        void handleFriendListOpcode(WorldPacket& recvPacket);
        void handleAddFriendOpcode(WorldPacket& recvPacket);
        void handleDelFriendOpcode(WorldPacket& recvPacket);
        void handleAddIgnoreOpcode(WorldPacket& recvPacket);
        void handleDelIgnoreOpcode(WorldPacket& recvPacket);
        void handleSetFriendNote(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // SpellHandler.cpp
        void handleSpellClick(WorldPacket& recvPacket);
        void handleCastSpellOpcode(WorldPacket& recvPacket);
        void handleCancelCastOpcode(WorldPacket& recvPacket);
        void handleCancelAuraOpcode(WorldPacket& recvPacket);
        void handleCancelChannellingOpcode(WorldPacket& recvPacket);
        void handleCancelAutoRepeatSpellOpcode(WorldPacket& /*recvPacket*/);
        void handlePetCastSpell(WorldPacket& recvPacket);
        void handleCancelTotem(WorldPacket& recvPacket);
        void handleUpdateProjectilePosition(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // TaxiHandler.cpp
    public:
        void sendTaxiList(Creature* creature);

    protected:
        void handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
        void handleEnabletaxiOpcode(WorldPacket& recvPacket);
        void handleActivateTaxiOpcode(WorldPacket& recvPacket);
        void handleMultipleActivateTaxiOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // TradeHandler.cpp
#if VERSION_STRING >= Cata
    public:
        void sendTradeResult(TradeStatus result);
        void sendTradeUpdate(bool trade_state = true);
        void sendTradeCancel();

    protected:
#endif
        void handleInitiateTradeOpcode(WorldPacket& recvPacket);
        void handleBeginTradeOpcode(WorldPacket& /*recvPacket*/);
        void handleBusyTrade(WorldPacket& /*recvPacket*/);
        void handleIgnoreTrade(WorldPacket& /*recvPacket*/);
        void handleAcceptTrade(WorldPacket& /*recvPacket*/);
        void handleUnacceptTrade(WorldPacket& /*recvPacket*/);
        void handleCancelTrade(WorldPacket& recvPacket);
        void handleSetTradeItem(WorldPacket& recvPacket);
        void handleClearTradeItem(WorldPacket& recvPacket);
        void handleSetTradeGold(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // VehicleHandler.cpp
#if VERSION_STRING > TBC
        void handleDismissVehicle(WorldPacket& /*recvPacket*/);
        void handleRequestVehiclePreviousSeat(WorldPacket& /*recvPacket*/);
        void handleRequestVehicleNextSeat(WorldPacket& /*recvPacket*/);
        void handleRequestVehicleSwitchSeat(WorldPacket& recvPacket);
        void handleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket);
        void handleRemoveVehiclePassenger(WorldPacket& recvPacket);
        void handleLeaveVehicle(WorldPacket& /*recvPacket*/);
        void handleEnterVehicle(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // VoiceChatHandler.Legacy.cpp
        // Zyres: this feature will be not implemented in the near future!
        //void HandleEnableMicrophoneOpcode(WorldPacket& recvPacket);
        //void HandleVoiceChatQueryOpcode(WorldPacket& recvPacket);
        //void HandleChannelVoiceQueryOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // OtherFiles


        ////////////////////////////////////UNSORTED BELOW THIS LINE///////////////////////////////

        void Unhandled(WorldPacket& recvPacket);
        void nothingToHandle(WorldPacket& recvPacket);

    public:

        float m_wLevel; // Level of water the player is currently in
        bool m_bIsWLevelSet; // Does the m_wLevel variable contain up-to-date information about water level?

    private:

        friend class Player;
        Player* _player;
        WorldSocket* _socket;


        // Preallocated buffers for movement handlers
        MovementInfo movement_info;
        uint8 movement_packet[90]{};

        uint32 _accountId;
        uint32 _accountFlags;
        std::string _accountName;

        bool has_level_55_char; // death knights
        bool has_dk;

        //uint16 _TEMP_ERR_CREATE_CODE; // increments
        int8 _side;

        WoWGuid m_MoverWoWGuid;
        uint64 m_MoverGuid;

        uint32 _logoutTime; // time we received a logout request -- wait 20 seconds, and quit

        AccountDataEntry sAccountData[8]{};

        FastQueue<WorldPacket*, Mutex> _recvQueue;
        char* permissions;
        int permissioncount;

        bool _loggingOut; //Player is being removed from the game.
        bool LoggingOut; //Player requesting to be logged out

        uint32 _latency;
#if VERSION_STRING < Cata
        uint32_t client_build;
#else
        uint16_t client_build;
#endif
        uint32 instanceId;
        uint8 _updatecount;

    public:

        MovementInfo* GetMovementInfo() { return &movement_info; }

        const MovementInfo* GetMovementInfo() const { return &movement_info; }
        static void InitPacketHandlerTable();
        static void loadSpecificHandlers();

        uint32 floodLines;
        time_t floodTime;

        void SystemMessage(const char* format, ...);

        uint32 language;
        uint32 m_muted;
};

#endif // WORLDSESSION_H
