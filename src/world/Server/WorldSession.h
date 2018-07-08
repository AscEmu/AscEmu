/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#include "AuthCodes.h"
#include "Data/Flags.h"
#if VERSION_STRING == Cata
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

//#define SESSION_CAP 5
#define CHECK_INWORLD_RETURN if (_player == NULL || !_player->IsInWorld()) { return; }


// Does nothing on release builds
#ifdef _DEBUG
#define CHECK_INWORLD_ASSERT ARCEMU_ASSERT(_player != NULL && _player->IsInWorld())
#else
#define CHECK_INWORLD_ASSERT CHECK_INWORLD_RETURN
#endif

#define CHECK_GUID_EXISTS(guidx) if (_player == NULL || _player->GetMapMgr() == NULL || _player->GetMapMgr()->GetUnit((guidx)) == NULL) { return; }
#define CHECK_PACKET_SIZE(pckp, ssize) if (ssize && pckp.size() < ssize) { Disconnect(); return; }

// Worldsocket related
#define WORLDSOCKET_TIMEOUT 120
#define PLAYER_LOGOUT_DELAY (20 * 1000) // 20 seconds should be more than enough to gank ya.

#define NOTIFICATION_MESSAGE_NO_PERMISSION "You do not have permission to perform that function."
//#define CHECK_PACKET_SIZE(x, y) if (y > 0 && x.size() < y) { _socket->Disconnect(); return; }

#define REGISTERED_ADDON_PREFIX_SOFTCAP 64

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
extern LoginErrorCode VerifyName(const char* name, size_t nlen);

class SERVER_DECL WorldSession
{
    friend class WorldSocket;

    public:
        //MIT

        //\brief: Used for LuAEngine
        void sendGuildCommandResult(uint32_t guildCommand, std::string text, uint32_t error);
        void sendGuildInvitePacket(std::string invitedName);
        //MIT END

        WorldSession(uint32 id, std::string Name, WorldSocket* sock);
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

            return (strchr(permissions, 'a') != NULL) ? true : false;
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

            if (initial == false && sAccountData[index].bIsDirty == false)      // Mark as "changed" or "dirty"
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

        void SendBuyFailed(uint64 guid, uint32 itemid, uint8 error);
        void SendSellItem(uint64 vendorguid, uint64 itemid, uint8 error);
        void SendNotification(const char* message, ...);
#if VERSION_STRING > TBC
        void SendRefundInfo(uint64_t guid);
#endif

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

        void CharacterEnumProc(QueryResult* result);
        void LoadAccountDataProc(QueryResult* result);
        bool IsLoggingOut() { return _loggingOut; }

    protected:

        // Login screen opcodes (CharacterHandler.cpp):
        void handleCharEnumOpcode(WorldPacket& /*recvPacket*/);
        void handleCharDeleteOpcode(WorldPacket& recvPacket);
        uint8_t deleteCharacter(WoWGuid guid);
        void handleCharCreateOpcode(WorldPacket& recvPacket);
        void handlePlayerLoginOpcode(WorldPacket& recvPacket);
        void handleRealmSplitOpcode(WorldPacket& recvPacket);
        void HandleObjectUpdateFailedOpcode(WorldPacket& recv_data);
        void handleDeclinedPlayerNameOpcode(WorldPacket& recvPacket); // declined names (Cyrillic client)

        /// Authentification and misc opcodes (MiscHandler.cpp):
        void HandlePingOpcode(WorldPacket& recvPacket);
        void HandleAuthSessionOpcode(WorldPacket& recvPacket);
        void HandleRepopRequestOpcode(WorldPacket& recvPacket);
        void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);
        void HandleLootMoneyOpcode(WorldPacket& recvPacket);
        void HandleLootOpcode(WorldPacket& recvPacket);
        void HandleLootReleaseOpcode(WorldPacket& recvPacket);
        void HandleLootMasterGiveOpcode(WorldPacket& recv_data);
        void handleLootRollOpcode(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        Loot* getLootFromHighGuidType(uint32_t highGuid);
        void HandleSuggestionOpcode(WorldPacket& recv_data);
#endif
        void handleWhoOpcode(WorldPacket& recvPacket);
        void HandleWhoIsOpcode(WorldPacket& recvPacket);
        void handleLogoutRequestOpcode(WorldPacket& recvPacket);
        void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
        void HandleLogoutCancelOpcode(WorldPacket& recvPacket);
        void HandleZoneUpdateOpcode(WorldPacket& recvPacket);
        //void HandleSetTargetOpcode(WorldPacket& recvPacket);
        void handleSetSelectionOpcode(WorldPacket& recvPacket);
        void handleStandStateChangeOpcode(WorldPacket& recvPacket);
        void handleDismountOpcode(WorldPacket& /*recvPacket*/);

        void handleFriendListOpcode(WorldPacket& recvPacket);
        void handleAddFriendOpcode(WorldPacket& recvPacket);
        void handleDelFriendOpcode(WorldPacket& recvPacket);
        void handleAddIgnoreOpcode(WorldPacket& recvPacket);
        void handleDelIgnoreOpcode(WorldPacket& recvPacket);

        //\todo not used for WotLK nor TBC maybe classic?
        void HandleSetFriendNote(WorldPacket& recvPacket);

        void HandleBugOpcode(WorldPacket& recv_data);
        void handleAreaTriggerOpcode(WorldPacket& recvPacket);
        void HandleUpdateAccountData(WorldPacket& recvPacket);
        void HandleRequestAccountData(WorldPacket& recvPacket);

        void handleSetActionButtonOpcode(WorldPacket& recvPacket);

        void handleSetFactionAtWarOpcode(WorldPacket& recvPacket);
        void handleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket);
        void handleTogglePVPOpcode(WorldPacket& recvPacket);
        void HandleAmmoSetOpcode(WorldPacket& recvPacket);
        void HandleGameObjectUse(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void HandleBarberShopResult(WorldPacket& recvPacket);
#endif
        //void HandleJoinChannelOpcode(WorldPacket& recvPacket);
        //void HandleLeaveChannelOpcode(WorldPacket& recvPacket);
        void handlePlayedTimeOpcode(WorldPacket& recvPacket);
        void handleSetSheathedOpcode(WorldPacket& recvPacket);
        void HandleCompleteCinematic(WorldPacket& recv_data);
        void HandleNextCinematic(WorldPacket& recv_data);
        void HandleInspectOpcode(WorldPacket& recv_data);
        void handleGameobjReportUseOpCode(WorldPacket& recvPacket);

        /// Gm Ticket System in GMTicket.cpp:
        void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketUpdateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketDeleteOpcode(WorldPacket& /*recv_data*/);
        void HandleGMTicketGetTicketOpcode(WorldPacket& /*recv_data*/);
        void HandleGMTicketSystemStatusOpcode(WorldPacket& /*recv_data*/);
        void HandleGMTicketToggleSystemStatusOpcode(WorldPacket& recvPacket);

        /// Lag report
        void HandleReportLag(WorldPacket& recvPacket);

        void HandleGMSurveySubmitOpcode(WorldPacket& recv_data);

        // Opcodes implemented in QueryHandler.cpp:
        void handleGameObjectQueryOpcode(WorldPacket& recvPacket);
        void handleQueryTimeOpcode(WorldPacket& recvPacket);
        void handleCreatureQueryOpcode(WorldPacket& recvPacket);
        void handleNameQueryOpcode(WorldPacket& recvPacket);
        void handleInrangeQuestgiverQuery(WorldPacket& /*recvPacket*/);
        void handleCorpseQueryOpcode(WorldPacket& /*recvPacket*/);
        void handleItemNameQueryOpcode(WorldPacket& recvPacket);
        void handlePageTextQueryOpcode(WorldPacket& recvPacket);
        void handleAchievmentQueryOpcode(WorldPacket& recvPacket);

        /// Opcodes implemented in MovementHandler.cpp
        void HandleMoveWorldportAckOpcode(WorldPacket& recvPacket);
        void HandleMovementOpcodes(WorldPacket& recvPacket);
        void HandleMoveTimeSkippedOpcode(WorldPacket& recv_data);
        void HandleMoveNotActiveMoverOpcode(WorldPacket& recv_data);
        void handleSetActiveMoverOpcode(WorldPacket& recv_data);
        void HandleMoveTeleportAckOpcode(WorldPacket& recv_data);

        // Opcodes implemented in GroupHandler.cpp:
#if VERSION_STRING == Cata
    public:
        void SendEmptyGroupList(Player* player);

    private:
        void HandleGroupInviteResponseOpcode(WorldPacket& recvPacket);
        void HandleGroupSetRolesOpcode(WorldPacket& recvPacket);
        void HandleGroupRequestJoinUpdatesOpcode(WorldPacket& recvPacket);

#endif

        void handleGroupInviteOpcode(WorldPacket& recvPacket);
        //void HandleGroupCancelOpcode(WorldPacket& recvPacket);
        void handleGroupAcceptOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupDeclineOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupUninviteOpcode(WorldPacket& recvPacket);
        void handleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        void handleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        void handleGroupDisbandOpcode(WorldPacket& /*recvPacket*/);
        void handleLootMethodOpcode(WorldPacket& recvPacket);
        void handleMinimapPingOpcode(WorldPacket& recvPacket);
        void handleSetPlayerIconOpcode(WorldPacket& recvPacket);


        // Raid
        void handleConvertGroupToRaidOpcode(WorldPacket& /*recvPacket*/);
        void handleGroupChangeSubGroup(WorldPacket& recvPacket);
        void handleGroupAssistantLeader(WorldPacket& recvPacket);
        void handleRequestRaidInfoOpcode(WorldPacket& /*recvPacket*/);
        void handleReadyCheckOpcode(WorldPacket& recvPacket);
        void handleGroupPromote(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        void HandleGroupRoleCheckBeginOpcode(WorldPacket& recv_data);
#endif

        //LFG
        void HandleLfgSetCommentOpcode(WorldPacket& recv_data);
#if VERSION_STRING == Cata
        void HandleLfgLockInfoOpcode(WorldPacket& recv_data);
#endif

#if VERSION_STRING > TBC
        void HandleLfgJoinOpcode(WorldPacket& recv_data);
        void HandleLfgLeaveOpcode(WorldPacket& recv_data);
        void HandleLfrSearchOpcode(WorldPacket& recv_data);
        void HandleLfrLeaveOpcode(WorldPacket& recv_data);
        void HandleLfgProposalResultOpcode(WorldPacket& recv_data);
        void HandleLfgSetRolesOpcode(WorldPacket& recv_data);
        void HandleLfgSetBootVoteOpcode(WorldPacket& recv_data);
        void HandleLfgPlayerLockInfoRequestOpcode(WorldPacket& recv_data);
        void HandleLfgTeleportOpcode(WorldPacket& recv_data);
        void HandleLfgPartyLockInfoRequestOpcode(WorldPacket& recv_data);
#endif

        /// Taxi opcodes (TaxiHandler.cpp)
        void HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
        void HandleActivateTaxiOpcode(WorldPacket& recvPacket);
        void HandleMultipleActivateTaxiOpcode(WorldPacket& recvPacket);

        // NPC opcodes (NPCHandler.cpp)
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

        // Auction House opcodes
        void handleAuctionHelloOpcode(WorldPacket& recvPacket);
        void handleAuctionListItems(WorldPacket& recvPacket);
        void handleAuctionListBidderItems(WorldPacket& recvPacket);
        void handleAuctionSellItem(WorldPacket& recvPacket);
        void handleAuctionListOwnerItems(WorldPacket& recvPacket);
        void handleAuctionPlaceBid(WorldPacket& recvPacket);
        void handleCancelAuction(WorldPacket& recvPacket);
        void handleAuctionListPendingSales(WorldPacket& recvPacket);

        // Mail opcodes
        void HandleGetMail(WorldPacket& recv_data);
        void HandleSendMail(WorldPacket& recv_data);
        void HandleTakeMoney(WorldPacket& recv_data);
        void HandleTakeItem(WorldPacket& recv_data);
        void HandleMarkAsRead(WorldPacket& recv_data);
        void HandleReturnToSender(WorldPacket& recv_data);
        void HandleMailDelete(WorldPacket& recv_data);
        void HandleItemTextQuery(WorldPacket& recv_data);
        void HandleMailTime(WorldPacket& recv_data);
        void HandleMailCreateTextItem(WorldPacket& recv_data);

        /// Item opcodes (ItemHandler.cpp)
        void HandleSwapInvItemOpcode(WorldPacket& recvPacket);
        void HandleSwapItemOpcode(WorldPacket& recvPacket);
        void HandleDestroyItemOpcode(WorldPacket& recvPacket);
        void HandleAutoEquipItemOpcode(WorldPacket& recvPacket);
        void HandleAutoEquipItemSlotOpcode(WorldPacket& recvPacket);
        void HandleItemQuerySingleOpcode(WorldPacket& recvPacket);
        void HandleSellItemOpcode(WorldPacket& recvPacket);
        void HandleBuyItemInSlotOpcode(WorldPacket& recvPacket);
        void HandleBuyItemOpcode(WorldPacket& recvPacket);
        void HandleListInventoryOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
        void HandleBuyBackOpcode(WorldPacket& recvPacket);
        void HandleSplitOpcode(WorldPacket& recvPacket);
        void HandleReadItemOpcode(WorldPacket& recvPacket);
        void HandleRepairItemOpcode(WorldPacket& recvPacket);
        void HandleAutoBankItemOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket);
        void HandleCancelTemporaryEnchantmentOpcode(WorldPacket& recvPacket);
        void HandleInsertGemOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void HandleItemRefundInfoOpcode(WorldPacket& recvPacket);
        void HandleItemRefundRequestOpcode(WorldPacket& recvPacket);
#endif

        // Equipment set opcode
#if VERSION_STRING > TBC
        void HandleEquipmentSetUse(WorldPacket& data);
        void HandleEquipmentSetSave(WorldPacket& data);
        void HandleEquipmentSetDelete(WorldPacket& data);
#endif

        // Combat opcodes (CombatHandler.cpp)
        void handleAttackSwingOpcode(WorldPacket& recvPacket);
        void handleAttackStopOpcode(WorldPacket& /*recvPacket*/);

        /// Spell opcodes (SpellHandler.cpp)
        void HandleUseItemOpcode(WorldPacket& recvPacket);
        void HandleCastSpellOpcode(WorldPacket& recvPacket);
        void HandleSpellClick(WorldPacket& recvPacket);
        void HandleCancelCastOpcode(WorldPacket& recvPacket);
        void HandleCancelAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelChannellingOpcode(WorldPacket& recvPacket);
        void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data);
        void HandlePetCastSpell(WorldPacket& recvPacket);
        void HandleCancelTotem(WorldPacket& recv_data);
        void HandleUpdateProjectilePosition(WorldPacket& recv_data);

        /// Skill opcodes (SkillHandler.spp)
#if VERSION_STRING == Cata
        void HandleLearnPreviewTalentsOpcode(WorldPacket& recvPacket);
#endif
        //void HandleSkillLevelUpOpcode(WorldPacket& recvPacket);
        void HandleLearnTalentOpcode(WorldPacket& recvPacket);
        void HandleLearnMultipleTalentsOpcode(WorldPacket& recvPacket);
        void HandleUnlearnTalents(WorldPacket& recv_data);

        /// Quest opcodes (QuestHandler.cpp)
        void HandleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverHelloOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverCancelOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestGiverQueryQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverCompleteQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestlogRemoveQuestOpcode(WorldPacket& recvPacket);
        void HandlePushQuestToPartyOpcode(WorldPacket& recvPacket);
        void HandleQuestPushResult(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void HandleQuestPOIQueryOpcode(WorldPacket& recv_data);
#endif

        // Vehicles
#if VERSION_STRING > TBC
        void handleDismissVehicle(WorldPacket& recvPacket);
        void handleRequestVehiclePreviousSeat(WorldPacket& /*recvPacket*/);
        void handleRequestVehicleNextSeat(WorldPacket& /*recvPacket*/);
        void handleRequestVehicleSwitchSeat(WorldPacket& recvPacket);
        void handleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket);
        void handleRemoveVehiclePassenger(WorldPacket& recvPacket);
        void handleLeaveVehicle(WorldPacket& /*recvPacket*/);
        void handleEnterVehicle(WorldPacket& recvPacket);
        void handleVehicleDismiss(WorldPacket& /*recvPacket*/);
#endif
        void handleSetActionBarTogglesOpcode(WorldPacket& recvPacket);
        void HandleMoveSplineCompleteOpcode(WorldPacket& recvPacket);

        // Chat opcodes (Chat.cpp)
        bool isSessionMuted();
        bool isFloodProtectionTriggered();

        void handleMessageChatOpcode(WorldPacket& recvPacket);
        void handleTextEmoteOpcode(WorldPacket& recvPacket);
        void handleEmoteOpcode(WorldPacket& recvPacket);

        void handleReportSpamOpcode(WorldPacket& recvPacket);
        void handleChatIgnoredOpcode(WorldPacket& recvPacket);
        void handleChatChannelWatchOpcode(WorldPacket& recvPacket);

        /// Corpse opcodes (Corpse.cpp)
        void HandleCorpseReclaimOpcode(WorldPacket& recvPacket);
        
        void HandleResurrectResponseOpcode(WorldPacket& recvPacket);

        // Channel Opcodes (ChannelHandler.cpp)
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

        // Duel
        void handleDuelAccepted(WorldPacket& /*recvPacket*/);
        void handleDuelCancelled(WorldPacket& /*recvPacket*/);

        // Trade
#if VERSION_STRING == Cata
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

        // Guild
        void handleGuildQuery(WorldPacket& recvPacket);
        void handleInviteToGuild(WorldPacket& recvPacket);

#if VERSION_STRING != Cata
        //void HandleCreateGuild(WorldPacket& recv_data);
        void HandleGuildAccept(WorldPacket& /*recv_data*/);
        void HandleGuildDecline(WorldPacket& /*recv_data*/);
        void HandleGuildInfo(WorldPacket& /*recv_data*/);
        void HandleGuildRoster(WorldPacket& /*recv_data*/);
        void HandleGuildPromote(WorldPacket& recv_data);
        void HandleGuildDemote(WorldPacket& recv_data);
        void HandleGuildLeave(WorldPacket& /*recv_data*/);
        void HandleGuildRemove(WorldPacket& recv_data);
        void HandleGuildDisband(WorldPacket& /*recv_data*/);
        void HandleGuildLeader(WorldPacket& recv_data);
        void HandleGuildMotd(WorldPacket& recv_data);
        void HandleGuildRank(WorldPacket& recv_data);
        void HandleGuildAddRank(WorldPacket& recv_data);
        void HandleGuildDelRank(WorldPacket& /*recv_data*/);
        void HandleGuildSetPublicNote(WorldPacket& recv_data);
        void HandleGuildSetOfficerNote(WorldPacket& recv_data);
        void HandleSaveGuildEmblem(WorldPacket& recv_data);
        void HandleCharterBuy(WorldPacket& recv_data);
        void HandleCharterShowSignatures(WorldPacket& recv_data);
        void HandleCharterTurnInCharter(WorldPacket& recv_data);
        void HandleCharterQuery(WorldPacket& recv_data);
        void HandleCharterOffer(WorldPacket& recv_data);
        void HandleCharterSign(WorldPacket& recv_data);
        void HandleCharterDecline(WorldPacket& recv_data);
        void HandleCharterRename(WorldPacket& recv_data);
        void HandleSetGuildInformation(WorldPacket& recv_data);
        void HandleGuildLog(WorldPacket& /*recv_data*/);
        void HandleGuildBankViewTab(WorldPacket& recv_data);
        void HandleGuildBankViewLog(WorldPacket& recv_data);
        void HandleGuildBankQueryText(WorldPacket& recv_data);
        void HandleSetGuildBankText(WorldPacket& recv_data);
        void HandleGuildBankOpenVault(WorldPacket& recv_data);
        void HandleGuildBankBuyTab(WorldPacket& recv_data);
        void HandleGuildBankDepositMoney(WorldPacket& recv_data);
        void HandleGuildBankWithdrawMoney(WorldPacket& recv_data);
        void HandleGuildBankDepositItem(WorldPacket& recv_data);
        void HandleGuildBankGetAvailableAmount(WorldPacket& recv_data);
        void HandleGuildBankModifyTab(WorldPacket& recv_data);
        void HandleGuildGetFullPermissions(WorldPacket& /*recv_data*/);
#else
    public:

        //////////////////////////////////////////////////////////////////////////////////////////
        // Guild
        void HandleGuildRemoveOpcode(WorldPacket& recv_data);
        void HandleGuildAcceptOpcode(WorldPacket& /*recv_data*/);
        void HandleGuildDeclineOpcode(WorldPacket& /*recv_data*/);
        void HandleGuildRosterOpcode(WorldPacket& /*recv_data*/);
        void HandleGuildPromoteOpcode(WorldPacket& recv_data);
        void HandleGuildAssignRankOpcode(WorldPacket& recv_data);
        void HandleGuildDemoteOpcode(WorldPacket& recv_data);
        void HandleGuildLeaveOpcode(WorldPacket& /*recv_data*/);
        void HandleGuildDisbandOpcode(WorldPacket& /*recv_data*/);

        void HandleGuildLeaderOpcode(WorldPacket& recv_data);
        void HandleGuildMotdOpcode(WorldPacket& recv_data);

        void HandleGuildSetNoteOpcode(WorldPacket& recv_data);
        void HandleGuildQueryRanksOpcode(WorldPacket& recv_data);

        void HandleGuildAddRankOpcode(WorldPacket& recv_data);
        void HandleGuildDelRankOpcode(WorldPacket& recv_data);
        void HandleGuildChangeInfoTextOpcode(WorldPacket& recv_data);
        void HandleSaveGuildEmblemOpcode(WorldPacket& recv_data);
        void HandleGuildEventLogQueryOpcode(WorldPacket& /*recv_data*/);
        void HandleGuildRequestChallengeUpdate(WorldPacket& /*recv_data*/);
        void HandleGuildPermissions(WorldPacket& /*recv_data*/);
        void HandleGuildQueryXPOpcode(WorldPacket& recv_data);
        void HandleGuildSetRankPermissionsOpcode(WorldPacket& recv_data);
        void HandleGuildRequestPartyState(WorldPacket& recv_data);
        void HandleGuildRequestMaxDailyXP(WorldPacket& recv_data);
        void HandleAutoDeclineGuildInvites(WorldPacket& recv_data);
        void HandleGuildRewardsQueryOpcode(WorldPacket& recv_data);
        void HandleGuildQueryNewsOpcode(WorldPacket& recv_data);
        void HandleGuildNewsUpdateStickyOpcode(WorldPacket& recv_data);
        void HandleGuildSetGuildMaster(WorldPacket& recv_data);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Guild Bank
        void HandleGuildBankMoneyWithdrawn(WorldPacket& /*recv_data*/);
        void HandleGuildBankerActivate(WorldPacket& recv_data);
        void HandleGuildBankQueryTab(WorldPacket& recv_data);
        void HandleGuildBankDepositMoney(WorldPacket& recv_data);
        void HandleGuildBankWithdrawMoney(WorldPacket& recv_data);
        void HandleGuildBankSwapItems(WorldPacket& recv_data);
        void HandleGuildBankBuyTab(WorldPacket& recv_data);
        void HandleGuildBankUpdateTab(WorldPacket& recv_data);
        void HandleGuildBankLogQuery(WorldPacket& recv_data);
        void HandleQueryGuildBankTabText(WorldPacket& recv_data);
        void HandleSetGuildBankTabText(WorldPacket& recv_data);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Charter
        void HandleCharterBuyOpcode(WorldPacket& recv_data);
        void HandleCharterShowSignaturesOpcode(WorldPacket& recv_data);
        void HandleCharterQueryOpcode(WorldPacket& recv_data);
        void HandleCharterOfferOpcode(WorldPacket& recv_data);
        void HandleCharterSignOpcode(WorldPacket& recv_data);
        void HandleCharterDeclineOpcode(WorldPacket& recv_data);
        void HandleCharterTurnInCharterOpcode(WorldPacket& recv_data);
        void HandleCharterRenameOpcode(WorldPacket& recv_data);

        //////////////////////////////////////////////////////////////////////////////////////////
        // GuildFinder
        void HandleGuildFinderAddRecruit(WorldPacket& recv_data);
        void HandleGuildFinderBrowse(WorldPacket& recv_data);
        void HandleGuildFinderDeclineRecruit(WorldPacket& recv_data);
        void HandleGuildFinderGetApplications(WorldPacket& /*recv_data*/);
        void HandleGuildFinderGetRecruits(WorldPacket& recv_data);
        void HandleGuildFinderPostRequest(WorldPacket& /*recv_data*/);
        void HandleGuildFinderRemoveRecruit(WorldPacket& recv_data);
        void HandleGuildFinderSetGuildPost(WorldPacket& recv_data);
#endif

        // Pet
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

        //????
        void handleDismissCritter(WorldPacket& recvPacket);

        // Battleground
        void handleBattlefieldPortOpcode(WorldPacket& recvPacket);
        void handleBattlefieldStatusOpcode(WorldPacket& recvPacket);
        void handleBattleMasterHelloOpcode(WorldPacket& recvPacket);
        void handleLeaveBattlefieldOpcode(WorldPacket& recvPacket);
        void handleAreaSpiritHealerQueryOpcode(WorldPacket& recvPacket);
        void handleAreaSpiritHealerQueueOpcode(WorldPacket& recvPacket);
        void handleBattlegroundPlayerPositionsOpcode(WorldPacket& recvPacket);
        void handleArenaJoinOpcode(WorldPacket& recvPacket);
        void handleBattleMasterJoinOpcode(WorldPacket& recvPacket);
        void handleInspectHonorStatsOpcode(WorldPacket& recvPacket);
        void handlePVPLogDataOpcode(WorldPacket& /*recvPacket*/);
        void handleBattlefieldListOpcode(WorldPacket& recvPacket);

#if VERSION_STRING == Cata
        void HandleRequestRatedBgInfoOpcode(WorldPacket& recv_data);
        void HandleRequestRatedBgStatsOpcode(WorldPacket& /*recv_data*/);
        void HandleRequestPvPRewardsOpcode(WorldPacket& /*recv_data*/);
        void HandleRequestPvpOptionsOpcode(WorldPacket& /*recv_data*/);
#endif

        /// Helper functions
        //void SetNpcFlagsForTalkToQuest(const uint64& guid, const uint64& targetGuid);

        // Tutorials
        void handleTutorialFlag(WorldPacket& recv_data);
        void handleTutorialClear(WorldPacket& recv_data);
        void handleTutorialReset(WorldPacket& recv_data);

        // Acknowledgements
        void HandleAcknowledgementOpcodes(WorldPacket& recv_data);
        void HandleMountSpecialAnimOpcode(WorldPacket& recv_data);

        void HandleSelfResurrectOpcode(WorldPacket& recv_data);
        void HandleUnlearnSkillOpcode(WorldPacket& recv_data);
        void handleRandomRollOpcode(WorldPacket& recv_data);
        void handleOpenItemOpcode(WorldPacket& recvPacket);

        void handleToggleHelmOpcode(WorldPacket& /*recvPacket*/);
        void handleToggleCloakOpcode(WorldPacket& /*recvPacket*/);
        void handleSetTitle(WorldPacket& recvPacket);

        // Instances
        void handleResetInstanceOpcode(WorldPacket& /*recvPacket*/);
        void handleDungeonDifficultyOpcode(WorldPacket& recvPacket);
        void handleRaidDifficultyOpcode(WorldPacket& recvPacket);

#if VERSION_STRING != Cata
        uint8_t trainerGetSpellStatus(TrainerSpell* trainerSpell);
#else
        TrainerSpellState trainerGetSpellStatus(TrainerSpell* trainerSpell);
#endif
        void SendMailError(uint32 error);

        // At Login
        void handleCharRenameOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void handleCharCustomizeLooksOpcode(WorldPacket& recvPacket);
        void handleCharFactionOrRaceChange(WorldPacket& recvPacket);
#endif
        void handleReadyForAccountDataTimes(WorldPacket& /*recvData*/);

        void handlePartyMemberStatsOpcode(WorldPacket& recvPacket);
        void HandleSummonResponseOpcode(WorldPacket& recv_data);

        void HandleArenaTeamAddMemberOpcode(WorldPacket& recv_data);
        void HandleArenaTeamRemoveMemberOpcode(WorldPacket& recv_data);
        void HandleArenaTeamInviteAcceptOpcode(WorldPacket& recv_data);
        void HandleArenaTeamInviteDenyOpcode(WorldPacket& recv_data);
        void HandleArenaTeamLeaveOpcode(WorldPacket& recv_data);
        void HandleArenaTeamDisbandOpcode(WorldPacket& recv_data);
        void HandleArenaTeamPromoteOpcode(WorldPacket& recv_data);
        void HandleArenaTeamQueryOpcode(WorldPacket& recv_data);
        void HandleArenaTeamRosterOpcode(WorldPacket& recv_data);
        void handleInspectArenaStatsOpcode(WorldPacket& recvPacket);

        void HandleTeleportCheatOpcode(WorldPacket& recv_data);
        void HandleTeleportToUnitOpcode(WorldPacket& recv_data);
        void HandleWorldportOpcode(WorldPacket& recv_data);
        void HandleWrapItemOpcode(WorldPacket& recv_data);

        // VoicChat
        // Zyres: this feature will be not implemented in the near future!
        //void HandleEnableMicrophoneOpcode(WorldPacket& recv_data);
        //void HandleVoiceChatQueryOpcode(WorldPacket& recv_data);
        //void HandleChannelVoiceQueryOpcode(WorldPacket& recv_data);
        void handleSetAutoLootPassOpcode(WorldPacket& recvPacket);


        // Misc
        void handleWorldStateUITimerUpdate(WorldPacket& recvPacket);
        void handleSetTaxiBenchmarkOpcode(WorldPacket& recv_data);
        void HandleMirrorImageOpcode(WorldPacket& recv_data);
        void HandleRemoveGlyph(WorldPacket& recv_data);
        void handleSetFactionInactiveOpcode(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        void HandleLogDisconnectOpcode(WorldPacket& recv_data);
#endif

        // Calendar \todo handle it
#if VERSION_STRING > TBC
        void HandleCalendarGetCalendar(WorldPacket& /*recv_data*/);
        void HandleCalendarComplain(WorldPacket& recv_data);
        void HandleCalendarGetNumPending(WorldPacket& /*recv_data*/);
        void HandleCalendarAddEvent(WorldPacket& recv_data);
        void HandleCalendarGetEvent(WorldPacket& recv_data);
        void HandleCalendarGuildFilter(WorldPacket& recv_data);
        void HandleCalendarArenaTeam(WorldPacket& recv_data);
        void HandleCalendarUpdateEvent(WorldPacket& recv_data);
        void HandleCalendarRemoveEvent(WorldPacket& recv_data);
        void HandleCalendarCopyEvent(WorldPacket& recv_data);
        void HandleCalendarEventInvite(WorldPacket& recv_data);
        void HandleCalendarEventRsvp(WorldPacket& recv_data);
        void HandleCalendarEventRemoveInvite(WorldPacket& recv_data);
        void HandleCalendarEventStatus(WorldPacket& recv_data);
        void HandleCalendarEventModeratorStatus(WorldPacket& recv_data);
#endif
#if VERSION_STRING == Cata
        void HandleReadyForAccountDataTimesOpcode(WorldPacket& recv_data);
        void HandleLoadScreenOpcode(WorldPacket& recv_data);
        void HandleUITimeRequestOpcode(WorldPacket& recv_data);
        void HandleTimeSyncRespOpcode(WorldPacket& recv_data);
        void HandleRequestHotfix(WorldPacket& recv_data);
        void HandleRequestCemeteryListOpcode(WorldPacket& recv_data);
        void HandleForceSpeedAckOpcodes(WorldPacket& recv_data);
        void HandleReturnToGraveyardOpcode(WorldPacket& /*recv_data*/);

        // Reports
        void HandleReportOpcode(WorldPacket& recv_data);
        void HandleReportPlayerOpcode(WorldPacket& recv_data);

    private:
        typedef std::list<AddonEntry> AddonsList;
        AddonsList m_addonList;


    public:
        void readAddonInfoPacket(ByteBuffer& recv_data);
        void sendAddonInfo();
#endif

        void Unhandled(WorldPacket& recv_data);
        void nothingToHandle(WorldPacket& recv_data);

    public:

        //npc heler functions
        void sendTabardHelp(Creature* creature);
        void sendBankerList(Creature* creature);
        void sendAuctionList(Creature* creature);
        void sendSpiritHealerRequest(Creature* creature);
        void sendCharterRequest(Creature* creature);
        void sendInnkeeperBind(Creature* creature);
        void sendTrainerList(Creature* creature);
        void sendStabledPetList(uint64 npcguid);

        void SendInventoryList(Creature* pCreature);
        void SendTaxiList(Creature* pCreature);
        void SendBattlegroundList(Creature* pCreature, uint32_t mapId);
        void SendAccountDataTimes(uint32 mask);
        void initGMMyMaster();
        void sendServerStats();

    void fullLogin(Player* player);
        void SendMOTD();

        void SendLfgUpdatePlayer(const LfgUpdateData& updateData);
        void SendLfgUpdateParty(const LfgUpdateData& updateData);
        void SendLfgRoleChosen(uint64 guid, uint8 roles);
        void SendLfgRoleCheckUpdate(const LfgRoleCheck* pRoleCheck);
        void SendLfgUpdateSearch(bool update);
        void SendLfgJoinResult(const LfgJoinResultData& joinData);
        void SendLfgQueueStatus(uint32 dungeon, int32 waitTime, int32 avgWaitTime, int32 waitTimeTanks, int32 waitTimeHealer, int32 waitTimeDps, uint32 queuedTime, uint8 tanks, uint8 healers, uint8 dps);
        void SendLfgPlayerReward(uint32 RandomDungeonEntry, uint32 DungeonEntry, uint8 done, const LfgReward* reward, QuestProperties const* qReward);
        void SendLfgBootPlayer(const LfgPlayerBoot* pBoot);
        void SendLfgUpdateProposal(uint32 proposalId, const LfgProposal *pProp);
        void SendLfgDisabled();
        void SendLfgOfferContinue(uint32 dungeonEntry);
        void SendLfgTeleportError(uint8 err);

        float m_wLevel; // Level of water the player is currently in
        bool m_bIsWLevelSet; // Does the m_wLevel variable contain up-to-date information about water level?

    private:

        friend class Player;
        Player* _player;
        WorldSocket* _socket;

        // Used to know race on login
        void loadPlayerFromDBProc(QueryResultVector& results);

        // Preallocated buffers for movement handlers
        MovementInfo movement_info;
        uint8 movement_packet[90];

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

        AccountDataEntry sAccountData[8];

        FastQueue<WorldPacket*, Mutex> _recvQueue;
        char* permissions;
        int permissioncount;

        bool _loggingOut; //Player is being removed from the game.
        bool LoggingOut; //Player requesting to be logged out

        uint32 _latency;
#if VERSION_STRING != Cata
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
        WorldPacket* BuildQuestQueryResponse(QuestProperties const* qst);
        uint32 m_muted;
#if VERSION_STRING > TBC
        void SendClientCacheVersion(uint32 version);
#endif

#if VERSION_STRING == Cata
        bool isAddonMessageFiltered;
        std::vector<std::string> mRegisteredAddonPrefixesVector;

        bool isAddonRegistered(const std::string& addon_name) const;
        void HandleUnregisterAddonPrefixesOpcode(WorldPacket& /*recv_data*/);
        void HandleAddonRegisteredPrefixesOpcode(WorldPacket& recv_data);
#endif

};

#endif // WORLDSESSION_H
