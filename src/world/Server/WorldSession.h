/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Server/Opcodes.hpp"
#include "Threading/ThreadSafeQueue.hpp"
#include "Server/CharacterErrors.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Data/Flags.hpp"
#include "Objects/MovementInfo.hpp"
#include "Utilities/CallBack.h"
#include "Management/AddonMgr.h"
#include <Utilities/utf8.hpp>
#include <memory>
#include <string>
#include "Logging/StringFormat.hpp"

struct QuestProperties;
class Player;
class WorldPacket;
class WorldSocket;
class WorldSession;
class MapMgr;
class Creature;
struct TrainerSpell;
class InstanceSaved;

struct LfgUpdateData; // forward declare
struct LfgJoinResultData;
struct LfgPlayerBoot;
struct LfgProposal;
struct LfgReward;
struct LfgRoleCheck;
struct Loot;
class WoWGuid;
class Query;
class QueryResult;

// Worldsocket related
#define WORLDSOCKET_TIMEOUT 120
#define PLAYER_LOGOUT_DELAY (20 * 1000) // 20 seconds should be more than enough.

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
#if VERSION_STRING > TBC
    NUM_ACCOUNT_DATA_TYPES          = 8
#else
    NUM_ACCOUNT_DATA_TYPES          = 32
#endif
};

const uint8_t GLOBAL_CACHE_MASK        = 0x15;
const uint8_t PER_CHARACTER_CACHE_MASK = 0xEA;

struct AccountDataEntry
{
    std::unique_ptr<char[]> data;
    uint32_t sz;
    bool bIsDirty;
};

extern CharacterErrorCodes VerifyName(utf8_string name);

class SERVER_DECL WorldSession
{
    friend class WorldSocket;

    public:
        WorldSession(uint32_t id, std::string name, WorldSocket* sock);
        ~WorldSession();

        Player* m_loggingInPlayer;

        void SendPacket(WorldPacket* packet);

        void OutPacket(uint16_t opcode);

        void SendChatPacket(WorldPacket* data, uint32_t langpos, int32_t lang, WorldSession* originator);

        uint32_t m_currMsTime;
        uint32_t m_lastPing;
        uint32_t m_loginTime;

        uint32_t GetAccountId() const { return _accountId; }
        Player* GetPlayer() { return _player; }
        Player* GetPlayerOrThrow();

        // Acct flags
        void SetAccountFlags(uint32_t /*flags*/)
        {
            // TODO: add a config to determine what flags are allowed on the server.
            // For now, override the db value depending on the AE Version.
            // _accountFlags = flags;

            switch (getAEVersion())
            {
                case 5875:
                    _accountFlags = 0;
                    break;
                case 8606:
                    _accountFlags = ACCOUNT_FLAG_XPACK_01;
                    break;
                case 12340:
                    _accountFlags = AF_FULL_WOTLK;
                    break;
                case 15595:
                    _accountFlags = AF_FULL_CATA;
                    break;
                case 18414:
                    _accountFlags = AF_FULL_MOP;
                    break;
            }
        }
        bool HasFlag(uint32_t flag) { return (_accountFlags & flag) != 0; }
        uint32_t GetFlags() { return _accountFlags; }

        // GM Permission System
        void LoadSecurity(std::string securitystring);
        std::unique_ptr<char[]> GetPermissions() const;

    //MIT
    bool hasPermissions() const;
    bool hasPermission(const char* requiredPermission) const;

        bool HasGMPermissions() const;

        bool CanUseCommand(char cmdstr);
        bool canUseCommand(const std::string& cmdstr) const;

        void SetSocket(WorldSocket* sock)
        {
            _socket = sock;
        }
        void SetPlayer(Player* plr) { _player = plr; }

        void SetAccountData(uint32_t index, std::unique_ptr<char[]> data, bool initial, uint32_t sz)
        {
            if (index >= 8)
                return;

            sAccountData[index].data = std::move(data);
            sAccountData[index].sz = sz;

            if (!initial && !sAccountData[index].bIsDirty)      // Mark as "changed" or "dirty"
                sAccountData[index].bIsDirty = true;
            else if (initial)
                sAccountData[index].bIsDirty = false;
        }

        AccountDataEntry* GetAccountData(uint32_t index);

        void SetLogoutTimer(uint32_t ms)
        {
            if (ms)
                _logoutTime = m_currMsTime + ms;
            else
                _logoutTime = 0;
        }

        void LogoutPlayer(bool Save);

        void QueuePacket(std::unique_ptr<WorldPacket> packet);

        void OutPacket(uint16_t opcode, uint16_t len, const void* data);

        WorldSocket* GetSocket() { return _socket; }

        void Disconnect();

        uint8_t Update(uint32_t InstanceID);

        void SendNotification(const char* message, ...);

        void SetInstance(uint32_t Instance) { instanceId = Instance; }
        uint32_t GetLatency() const { return _latency; }
        std::string GetAccountName() { return _accountName; }
        const char* GetAccountNameS() const { return _accountName.c_str(); }
        const char* LocalizedWorldSrv(uint32_t id);
        const char* LocalizedGossipOption(uint32_t id);
        const char* LocalizedMapName(uint32_t id);
        const char* LocalizedBroadCast(uint32_t id);

#if VERSION_STRING != Cata
        uint32_t GetClientBuild() { return client_build; }
        void SetClientBuild(uint32_t build) { client_build = build; }
#else
        uint16_t GetClientBuild() { return client_build; }
        void SetClientBuild(uint16_t build) { client_build = build; }
#endif

        bool bDeleted;
        uint32_t GetInstance() { return instanceId; }
        std::mutex deleteMutex;
        int32_t m_moveDelayTime;
        int32_t m_clientTimeDelay;

        
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

        void handleRequestRatedBgInfoOpcode(WorldPacket& recvPacket);       //>= Cata
        void handleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/);  //>= Cata
        void handleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/);    //>= Cata
        void handleRequestPvpOptionsOpcode(WorldPacket& /*recvPacket*/);    //>= Cata


        //////////////////////////////////////////////////////////////////////////////////////////
        // CalendarHandler.cpp
        // \todo handle it
        void handleCalendarGetCalendar(WorldPacket& /*recvPacket*/);    //> TBC
        void handleCalendarComplain(WorldPacket& recvPacket);           //> TBC
        void handleCalendarGetNumPending(WorldPacket& /*recvPacket*/);  //> TBC
        void handleCalendarAddEvent(WorldPacket& recvPacket);           //> TBC
        void handleCalendarGetEvent(WorldPacket& recvPacket);           //> TBC
        void handleCalendarGuildFilter(WorldPacket& recvPacket);        //> TBC
        void handleCalendarArenaTeam(WorldPacket& recvPacket);          //> TBC
        void handleCalendarUpdateEvent(WorldPacket& recvPacket);        //> TBC
        void handleCalendarRemoveEvent(WorldPacket& recvPacket);        //> TBC
        void handleCalendarCopyEvent(WorldPacket& recvPacket);          //> TBC
        void handleCalendarEventInvite(WorldPacket& recvPacket);        //> TBC
        void handleCalendarEventRsvp(WorldPacket& recvPacket);          //> TBC
        void handleCalendarEventRemoveInvite(WorldPacket& recvPacket);  //> TBC
        void handleCalendarEventStatus(WorldPacket& recvPacket);        //> TBC
        void handleCalendarEventModeratorStatus(WorldPacket& recvPacket);   //> TBC

public:
        void sendCalendarRaidLockout(InstanceSaved const* save, bool add);  //> TBC
        void sendCalendarRaidLockoutUpdated(InstanceSaved const* save);     //> TBC

protected:
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

		// declined names (Cyrillic client)
		void handleDeclinedPlayerNameOpcode(WorldPacket& recvPacket);   //> TBC
        
		void handleCharEnumOpcode(WorldPacket& /*recvPacket*/);

        void handleCharFactionOrRaceChange(WorldPacket& recvPacket);    //> TBC
        void handleCharCustomizeLooksOpcode(WorldPacket& recvPacket);   //> TBC
        
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
    public:
        void sendEmptyGroupList(Player* player);                                //>= Cata

    private:
        void handleGroupInviteResponseOpcode(WorldPacket& recvPacket);          //>= Cata
        void handleGroupSetRolesOpcode(WorldPacket& recvPacket);                //>= Cata
        void handleGroupRequestJoinUpdatesOpcode(WorldPacket& /*recvPacket*/);  //>= Cata
        void handleGroupRoleCheckBeginOpcode(WorldPacket& recvPacket);          //>= Cata

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

        void handleGuildInfo(WorldPacket& /*recvPacket*/);          //< Cata

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
        void handleGuildSetPublicNote(WorldPacket& recvPacket);     //<Cata
        void handleGuildSetOfficerNote(WorldPacket& recvPacket);    //<Cata

        void handleGuildSetNoteOpcode(WorldPacket& recvPacket);     //>=Cata

        //\brief this was an empty opcodes on versions < Cata.
        //       now it has some content since cata.
        void handleGuildDelRank(WorldPacket& recvPacket);

        //\brief this was an MSG opcode on versions < Cata.
        //       now it is split into CMSG and SMSG packets since cata.
        void handleGuildBankQueryText(WorldPacket& recvPacket); //<Cata
        void handleQueryGuildBankTabText(WorldPacket& recvPacket);  //>=Cata

        void handleCharterShowSignatures(WorldPacket& recvPacket);
        void handleCharterOffer(WorldPacket& recvPacket);
        void handleCharterSign(WorldPacket& recvPacket);
        void handleCharterDecline(WorldPacket& recvPacket);
        void handleCharterRename(WorldPacket& recvPacket);
        void handleCharterTurnInCharter(WorldPacket& recvPacket);
        void handleCharterQuery(WorldPacket& recvPacket);
        void handleCharterBuy(WorldPacket& recvPacket);

        // Guild
        void handleGuildAssignRankOpcode(WorldPacket& recvPacket);              //>= Cata
        void handleGuildQueryRanksOpcode(WorldPacket& recvPacket);              //>= Cata
        void handleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/);    //>= Cata
        void handleGuildQueryXPOpcode(WorldPacket& recvPacket);                 //>= Cata
        void handleGuildRequestPartyState(WorldPacket& recvPacket);             //>= Cata
        void handleGuildRequestMaxDailyXP(WorldPacket& recvPacket);             //>= Cata
        void handleAutoDeclineGuildInvites(WorldPacket& recvPacket);            //>= Cata
        void handleGuildRewardsQueryOpcode(WorldPacket& recvPacket);            //>= Cata
        void handleGuildQueryNewsOpcode(WorldPacket& recvPacket);               //>= Cata
        void handleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket);        //>= Cata
        void handleGuildSetGuildMaster(WorldPacket& recvPacket);                //>= Cata

        // GuildFinder
        void handleGuildFinderAddRecruit(WorldPacket& recvPacket);              //>= Cata
        void handleGuildFinderBrowse(WorldPacket& recvPacket);                  //>= Cata
        void handleGuildFinderDeclineRecruit(WorldPacket& recvPacket);          //>= Cata
        void handleGuildFinderGetApplications(WorldPacket& /*recvPacket*/);     //>= Cata
        void handleGuildFinderGetRecruits(WorldPacket& recvPacket);             //>= Cata
        void handleGuildFinderPostRequest(WorldPacket& /*recvPacket*/);         //>= Cata
        void handleGuildFinderRemoveRecruit(WorldPacket& recvPacket);           //>= Cata
        void handleGuildFinderSetGuildPost(WorldPacket& recvPacket);            //>= Cata

        //////////////////////////////////////////////////////////////////////////////////////////
        // ItemHandler.cpp
    public:
        void sendInventoryList(Creature* pCreature);
        void sendBuyFailed(uint64_t guid, uint32_t itemid, uint8_t error);
        void sendSellItem(uint64_t vendorguid, uint64_t itemid, uint8_t error);

        // Void Storage
        void handleVoidStorageUnlock(WorldPacket& recvData);        //>= Cata
        void handleVoidStorageQuery(WorldPacket& recvData);         //>= Cata
        void handleVoidStorageTransfer(WorldPacket& recvData);      //>= Cata
        void handleVoidSwapItem(WorldPacket& recvData);             //>= Cata
        void sendVoidStorageTransferResult(uint8_t result);         //>= Cata

        // Transmogrification
        void handleTransmogrifyItems(WorldPacket& recvData);        //>= Cata

        // Reforge
        void handleReforgeItemOpcode(WorldPacket& recvData);        //>= Cata
        void sendReforgeResult(bool success);                       //>= Cata

        void sendRefundInfo(uint64_t guid);                             //>= WotLK

    protected:
        void handleItemRefundInfoOpcode(WorldPacket& recvPacket);       //>= WotLK
        void handleItemRefundRequestOpcode(WorldPacket& recvPacket);    //>= WotLK

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

        void handleInsertGemOpcode(WorldPacket& recvPacket);        //> Classic

        void handleWrapItemOpcode(WorldPacket& recvPacket);
        void handleEquipmentSetUse(WorldPacket& recvPacket);        //> TBC
        void handleEquipmentSetSave(WorldPacket& recvPacket);       //> TBC
        void handleEquipmentSetDelete(WorldPacket& recvPacket);     //> TBC

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
        void handleLfgLockInfoOpcode(WorldPacket& recvPacket);      //>= Cata

        void handleLfgJoinOpcode(WorldPacket& recvPacket);          //> TBC
        void handleLfgLeaveOpcode(WorldPacket& recvPacket);         //> TBC
        void handleLfgSearchOpcode(WorldPacket& recvPacket);        //> TBC
        void handleLfgSearchLeaveOpcode(WorldPacket& recvPacket);   //> TBC
        void handleLfgProposalResultOpcode(WorldPacket& recvPacket);//> TBC
        void handleLfgSetRolesOpcode(WorldPacket& recvPacket);      //> TBC
        void handleLfgSetBootVoteOpcode(WorldPacket& recvPacket);   //> TBC
        void handleLfgPlayerLockInfoRequestOpcode(WorldPacket& recvPacket); //> TBC
        void handleLfgTeleportOpcode(WorldPacket& recvPacket);      //> TBC
        void handleLfgPartyLockInfoRequestOpcode(WorldPacket& recvPacket);  //> TBC

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

        void doLootRelease(WoWGuid lguid);

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
        void sendAccountDataTimes(uint32_t mask);
        void sendMOTD();
        void sendClientCacheVersion(uint32_t version);    //> TBC

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
        void handleInstanceLockResponse(WorldPacket& recvPacket);
        void handleViolenceLevel(WorldPacket& recvPacket);                  //>= Cata
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

        void handleSuggestionOpcode(WorldPacket& recvPacket);               //>= Cata
        void handleReturnToGraveyardOpcode(WorldPacket& /*recvPacket*/);    //>= Cata
        void handleLogDisconnectOpcode(WorldPacket& recvPacket);            //>= Cata

        void handleCompleteCinematic(WorldPacket& /*recvPacket*/);
        void handleNextCinematic(WorldPacket& /*recvPacket*/);
        void handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/);
        void handleSummonResponseOpcode(WorldPacket& recvPacket);
        void handleLogoutCancelOpcode(WorldPacket& /*recvPacket*/);
        void handlePlayerLogoutOpcode(WorldPacket& /*recvPacket*/);
        void handleCorpseReclaimOpcode(WorldPacket& recvPacket);

        void handleLoadScreenOpcode(WorldPacket& recvPacket);               //>= Cata
        void handleUITimeRequestOpcode(WorldPacket& /*recvPacket*/);        //>= Cata
        void handleTimeSyncRespOpcode(WorldPacket& recvPacket);             //>= Cata
        void handleObjectUpdateFailedOpcode(WorldPacket& recvPacket);       //>= Cata
        void handleRequestHotfix(WorldPacket& recvPacket);                  //>= Cata
        void handleRequestCemeteryListOpcode(WorldPacket& /*recvPacket*/);  //>= Cata
        void sendItemDb2Reply(uint32_t entry);                              //>= Cata
        void sendItemSparseDb2Reply(uint32_t entry);                        //>= Cata

        void handleRemoveGlyph(WorldPacket& recvPacket);                    //> TBC
        void handleBarberShopResult(WorldPacket& recvPacket);               //> TBC
        void handleRepopRequestOpcode(WorldPacket& /*recvPacket*/);
        void handleWhoIsOpcode(WorldPacket& recvPacket);
        void handleAmmoSetOpcode(WorldPacket& recvPacket);
        void handleGameObjectUse(WorldPacket& recvPacket);
        void handleInspectOpcode(WorldPacket& recvPacket);

    //\todo move to seperated file
    private:
        bool isAddonMessageFiltered;                                        //>= Cata
        std::vector<std::string> mRegisteredAddonPrefixesVector;            //>= Cata
        typedef std::list<AddonEntry> AddonsList;                           //>= Cata
        AddonsList m_addonList;                                             //>= Cata

    public:
        bool isAddonRegistered(const std::string& addon_name) const;        //>= Cata
        void readAddonInfoPacket(ByteBuffer& recvPacket);                   //>= Cata
        void sendAddonInfo();                                               //>= Cata

    protected:
        void handleUnregisterAddonPrefixesOpcode(WorldPacket& /*recvPacket*/);  //>= Cata
        void handleAddonRegisteredPrefixesOpcode(WorldPacket& recvPacket);  //>= Cata
        void handleReportOpcode(WorldPacket& recvPacket);                   //>= Cata
        void handleReportPlayerOpcode(WorldPacket& recvPacket);             //>= Cata

        void HandleMirrorImageOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // MovementHandler.cpp
        void handleSetActiveMoverOpcode(WorldPacket& recvPacket);
        void updatePlayerMovementVars(uint16_t opcode);
        bool isHackDetectedInMovementData(uint16_t opcode);

        void handleMovementOpcodes(WorldPacket& recvPacket);
        void handleAcknowledgementOpcodes(WorldPacket& recvPacket);
        void handleForceSpeedChangeAck(WorldPacket& recvPacket);
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

        TrainerSpellState trainerGetSpellStatus(TrainerSpell const* trainerSpell) const;

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
        std::unique_ptr<WorldPacket> buildQuestQueryResponse(QuestProperties const* qst);

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
        void handleQuestPOIQueryOpcode(WorldPacket& recvPacket);        //> TBC

        //////////////////////////////////////////////////////////////////////////////////////////
        // SkillHandler.cpp
        void handleUnlearnSkillOpcode(WorldPacket& recvPacket);
        void handleLearnTalentOpcode(WorldPacket& recvPacket);
        void handleUnlearnTalents(WorldPacket& recvPacket);
        void handleLearnMultipleTalentsOpcode(WorldPacket& recvPacket); //< Cata
        void handleLearnPreviewTalentsOpcode(WorldPacket& recvPacket);  //>=Cata

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
        void sendTaxiStatus(WoWGuid guid);
        void sendTaxiMenu(Creature* unit);
        void sendDoFlight(uint32_t mountDisplayId, uint32_t path, uint32_t pathNode = 0);
        bool sendLearnNewTaxiNode(Creature* unit);
        void sendDiscoverNewTaxiNode(uint32_t nodeid);

    protected:
        void handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
        void handleEnabletaxiOpcode(WorldPacket& recvPacket);
        void handleActivateTaxiOpcode(WorldPacket& recvPacket);
        void handleMultipleActivateTaxiOpcode(WorldPacket& recvPacket);
        void handleMoveSplineDoneOpcode(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // TradeHandler.cpp
    public:
        void sendTradeUpdate(bool tradeState = true);
        void sendTradeResult(TradeStatus result, uint64_t guid = 0);

    protected:
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
        void handleDismissVehicle(WorldPacket& /*recvPacket*/);                 //> TBC
        void handleRequestVehiclePreviousSeat(WorldPacket& /*recvPacket*/);     //> TBC
        void handleRequestVehicleNextSeat(WorldPacket& /*recvPacket*/);         //> TBC
        void handleRequestVehicleSwitchSeat(WorldPacket& recvPacket);           //> TBC
        void handleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket);     //> TBC
        void handleRemoveVehiclePassenger(WorldPacket& recvPacket);             //> TBC
        void handleLeaveVehicle(WorldPacket& /*recvPacket*/);                   //> TBC
        void handleEnterVehicle(WorldPacket& recvPacket);                       //> TBC

        //////////////////////////////////////////////////////////////////////////////////////////
        // VoiceChatHandler.cpp
        // Zyres: this feature will be not implemented in the near future!
        //void handleEnableMicrophoneOpcode(WorldPacket& recvPacket);
        //void handleChannelVoiceQueryOpcode(WorldPacket& recvPacket);
        //void handleVoiceChatQueryOpcode(WorldPacket& recvPacket);

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
        MovementInfo sessionMovementInfo;

        uint32_t _accountId;
        uint32_t _accountFlags;
        std::string _accountName;
#if VERSION_STRING > TBC
        bool has_level_55_char; // death knights
        bool has_dk;
#endif
        // uint16_t _TEMP_ERR_CREATE_CODE; // increments
        uint8_t _side;

        WoWGuid m_MoverWoWGuid;

        uint32_t _logoutTime; // time we received a logout request -- wait 20 seconds, and quit

        std::array<AccountDataEntry, 8> sAccountData{};

        ThreadSafeQueue<std::unique_ptr<WorldPacket>> _recvQueue;
        std::string permissions;

        bool _loggingOut; // Player is being removed from the game.
        bool LoggingOut; // Player requesting to be logged out

        uint32_t _latency;
#if VERSION_STRING < Cata
        uint32_t client_build;
#else
        uint16_t client_build;
#endif
        uint32_t instanceId;
        uint8_t _updatecount;

public:
    static void registerOpcodeHandler();

        uint32_t floodLines;
        time_t floodTime;

        void SystemMessage(const char* format, ...);

    void sendSystemMessagePacket(std::string& _message);

    // Variadic template version of systemMessage
    template<typename... Args>
    void systemMessage(const std::string& format, Args&&... args)
    {
        // Use the custom StringFormat function to format the string
        std::string formattedMessage = AscEmu::StringFormat(format, std::forward<Args>(args)...);

        // Send the formatted message via packet
        sendSystemMessagePacket(formattedMessage);
    }

        uint32_t language;
        uint32_t m_muted;
};

#endif // WORLDSESSION_H
