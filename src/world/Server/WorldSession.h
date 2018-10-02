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
#include "Server/CharacterErrors.h"
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
#define PLAYER_LOGOUT_DELAY (20 * 1000) // 20 seconds should be more than enough.

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
extern CharacterErrorCodes VerifyName(const char* name, size_t nlen);

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
        void HandleObjectUpdateFailedOpcode(WorldPacket& recvPacket);
        void handleDeclinedPlayerNameOpcode(WorldPacket& recvPacket); // declined names (Cyrillic client)

        // Authentification and misc opcodes (MiscHandler.cpp):
        void HandlePingOpcode(WorldPacket& recvPacket);
        void HandleAuthSessionOpcode(WorldPacket& recvPacket);
        void HandleRepopRequestOpcode(WorldPacket& recvPacket);
        void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);
        void HandleLootMoneyOpcode(WorldPacket& recvPacket);
        void HandleLootOpcode(WorldPacket& recvPacket);
        void HandleLootReleaseOpcode(WorldPacket& recvPacket);
        void HandleLootMasterGiveOpcode(WorldPacket& recvPacket);
        void handleLootRollOpcode(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        Loot* getLootFromHighGuidType(HighGuid highGuid);
        void HandleSuggestionOpcode(WorldPacket& recvPacket);
#endif
        void handleWhoOpcode(WorldPacket& recvPacket);
        void HandleWhoIsOpcode(WorldPacket& recvPacket);
        void handleLogoutRequestOpcode(WorldPacket& recvPacket);
        void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
        void HandleLogoutCancelOpcode(WorldPacket& recvPacket);
        void handleZoneupdate(WorldPacket& recvPacket);
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

        void HandleBugOpcode(WorldPacket& recvPacket);
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
        void HandleCompleteCinematic(WorldPacket& recvPacket);
        void HandleNextCinematic(WorldPacket& recvPacket);
        void HandleInspectOpcode(WorldPacket& recvPacket);
        void handleGameobjReportUseOpCode(WorldPacket& recvPacket);

        // Gm Ticket System in GMTicket.cpp:
        void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketUpdateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketDeleteOpcode(WorldPacket& /*recvPacket*/);
        void HandleGMTicketGetTicketOpcode(WorldPacket& /*recvPacket*/);
        void HandleGMTicketSystemStatusOpcode(WorldPacket& /*recvPacket*/);
        void HandleGMTicketToggleSystemStatusOpcode(WorldPacket& recvPacket);

        // Lag report
        void HandleReportLag(WorldPacket& recvPacket);

        void HandleGMSurveySubmitOpcode(WorldPacket& recvPacket);

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

        // Opcodes implemented in MovementHandler.cpp
        void HandleMoveWorldportAckOpcode(WorldPacket& recvPacket);
        void HandleMovementOpcodes(WorldPacket& recvPacket);
        void HandleMoveNotActiveMoverOpcode(WorldPacket& recvPacket);
        void handleSetActiveMoverOpcode(WorldPacket& recvPacket);
        void HandleMoveTeleportAckOpcode(WorldPacket& recvPacket);

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
        void HandleGroupRoleCheckBeginOpcode(WorldPacket& recvPacket);
#endif

        //LFG
        void handleLfgSetCommentOpcode(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        void HandleLfgLockInfoOpcode(WorldPacket& recvPacket);
#endif

#if VERSION_STRING > TBC
        void HandleLfgJoinOpcode(WorldPacket& recvPacket);
        void HandleLfgLeaveOpcode(WorldPacket& recvPacket);
        void HandleLfrSearchOpcode(WorldPacket& recvPacket);
        void HandleLfrLeaveOpcode(WorldPacket& recvPacket);
        void HandleLfgProposalResultOpcode(WorldPacket& recvPacket);
        void HandleLfgSetRolesOpcode(WorldPacket& recvPacket);
        void HandleLfgSetBootVoteOpcode(WorldPacket& recvPacket);
        void HandleLfgPlayerLockInfoRequestOpcode(WorldPacket& recvPacket);
        void HandleLfgTeleportOpcode(WorldPacket& recvPacket);
        void HandleLfgPartyLockInfoRequestOpcode(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // Taxi helper and handlers (TaxiHandler.cpp)
    public:
        void sendTaxiList(Creature* creature);

    protected:
        void handleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void handleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
        void handleEnabletaxiOpcode(WorldPacket& recvPacket);
        void handleActivateTaxiOpcode(WorldPacket& recvPacket);
        void handleMultipleActivateTaxiOpcode(WorldPacket& recvPacket);

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

        // Item opcodes (ItemHandler.cpp)
        void HandleSwapInvItemOpcode(WorldPacket& recvPacket);
        void handleSwapItemOpcode(WorldPacket& recvPacket);
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
        void HandleEquipmentSetUse(WorldPacket& recvPacket);
        void HandleEquipmentSetSave(WorldPacket& recvPacket);
        void HandleEquipmentSetDelete(WorldPacket& recvPacket);
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
        // Combat handler (CombatHandler.cpp)
        void handleAttackSwingOpcode(WorldPacket& recvPacket);
        void handleAttackStopOpcode(WorldPacket& /*recvPacket*/);

        //////////////////////////////////////////////////////////////////////////////////////////
        // Duel handler (DuelHandler.cpp)
        void handleDuelAccepted(WorldPacket& /*recvPacket*/);
        void handleDuelCancelled(WorldPacket& /*recvPacket*/);

        // Spell opcodes (SpellHandler.cpp)
        void HandleUseItemOpcode(WorldPacket& recvPacket);
        void HandleCastSpellOpcode(WorldPacket& recvPacket);
        void HandleSpellClick(WorldPacket& recvPacket);
        void HandleCancelCastOpcode(WorldPacket& recvPacket);
        void HandleCancelAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelChannellingOpcode(WorldPacket& recvPacket);
        void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recvPacket);
        void HandlePetCastSpell(WorldPacket& recvPacket);
        void HandleCancelTotem(WorldPacket& recvPacket);
        void HandleUpdateProjectilePosition(WorldPacket& recvPacket);

        // Skill opcodes (SkillHandler.spp)
#if VERSION_STRING == Cata
        void HandleLearnPreviewTalentsOpcode(WorldPacket& recvPacket);
#endif
        //void HandleSkillLevelUpOpcode(WorldPacket& recvPacket);
        void HandleLearnTalentOpcode(WorldPacket& recvPacket);
        void HandleLearnMultipleTalentsOpcode(WorldPacket& recvPacket);
        void HandleUnlearnTalents(WorldPacket& recvPacket);

        // Quest opcodes (QuestHandler.cpp)
#if VERSION_STRING == TBC
        void HandleInrangeQuestgiverQuery(WorldPacket& /*recvPacket*/);
#endif
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
        void handleQuestPushResultOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void HandleQuestPOIQueryOpcode(WorldPacket& recvPacket);
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

        // Chat opcodes (Chat.cpp)
        bool isSessionMuted();
        bool isFloodProtectionTriggered();

        void handleMessageChatOpcode(WorldPacket& recvPacket);
        void handleTextEmoteOpcode(WorldPacket& recvPacket);
        void handleEmoteOpcode(WorldPacket& recvPacket);

        void handleReportSpamOpcode(WorldPacket& recvPacket);
        void handleChatIgnoredOpcode(WorldPacket& recvPacket);
        void handleChatChannelWatchOpcode(WorldPacket& recvPacket);

        // Corpse opcodes (Corpse.cpp)
        void HandleCorpseReclaimOpcode(WorldPacket& recvPacket);
        
        void handleResurrectResponse(WorldPacket& recvPacket);

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
#if VERSION_STRING != Cata
        void handleGuildSetPublicNote(WorldPacket& recvPacket);
        void handleGuildSetOfficerNote(WorldPacket& recvPacket);
#else
        void handleGuildSetNoteOpcode(WorldPacket& recvPacket);
#endif

    //\brief this was an empty opcodes on versions < Cata.
    //       now it has some content since cata.
#if VERSION_STRING != Cata
        void handleGuildDelRank(WorldPacket& /*recvPacket*/);
#else
        void handleGuildDelRank(WorldPacket& recvPacket);
#endif

    //\brief this was an MSG opcode on versions < Cata.
    //       now it is split into CMSG and SMSG packets since cata.
#if VERSION_STRING != Cata
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


#if VERSION_STRING != Cata
        //void HandleCreateGuild(WorldPacket& recvPacket);
        void handleGuildInfo(WorldPacket& /*recvPacket*/);
#else
    public:

        //////////////////////////////////////////////////////////////////////////////////////////
        // Guild
        void HandleGuildAssignRankOpcode(WorldPacket& recvPacket);
        
        void HandleGuildQueryRanksOpcode(WorldPacket& recvPacket);
        
        void HandleGuildRequestChallengeUpdate(WorldPacket& /*recvPacket*/);
        void HandleGuildQueryXPOpcode(WorldPacket& recvPacket);
        void HandleGuildRequestPartyState(WorldPacket& recvPacket);
        void HandleGuildRequestMaxDailyXP(WorldPacket& recvPacket);
        void HandleAutoDeclineGuildInvites(WorldPacket& recvPacket);
        void HandleGuildRewardsQueryOpcode(WorldPacket& recvPacket);
        void HandleGuildQueryNewsOpcode(WorldPacket& recvPacket);
        void HandleGuildNewsUpdateStickyOpcode(WorldPacket& recvPacket);
        void HandleGuildSetGuildMaster(WorldPacket& recvPacket);

        //////////////////////////////////////////////////////////////////////////////////////////
        // GuildFinder
        void HandleGuildFinderAddRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderBrowse(WorldPacket& recvPacket);
        void HandleGuildFinderDeclineRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderGetApplications(WorldPacket& /*recvPacket*/);
        void HandleGuildFinderGetRecruits(WorldPacket& recvPacket);
        void HandleGuildFinderPostRequest(WorldPacket& /*recvPacket*/);
        void HandleGuildFinderRemoveRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderSetGuildPost(WorldPacket& recvPacket);
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

        //////////////////////////////////////////////////////////////////////////////////////////
        // Battleground helper and handlers (BattlegroundHandler.cpp)
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

#if VERSION_STRING == Cata
        void HandleRequestRatedBgInfoOpcode(WorldPacket& recvPacket);
        void HandleRequestRatedBgStatsOpcode(WorldPacket& /*recvPacket*/);
        void HandleRequestPvPRewardsOpcode(WorldPacket& /*recvPacket*/);
        void HandleRequestPvpOptionsOpcode(WorldPacket& /*recvPacket*/);
#endif

        // Helper functions
        //void SetNpcFlagsForTalkToQuest(const uint64& guid, const uint64& targetGuid);

        // Tutorials
        void handleTutorialFlag(WorldPacket& recvPacket);
        void handleTutorialClear(WorldPacket& recvPacket);
        void handleTutorialReset(WorldPacket& recvPacket);

        // Acknowledgements
        void HandleAcknowledgementOpcodes(WorldPacket& recvPacket);
        void handleMountSpecialAnimOpcode(WorldPacket& recvPacket);

        void handleSelfResurrect(WorldPacket& /*recvPacket*/);
        void HandleUnlearnSkillOpcode(WorldPacket& recvPacket);
        void handleRandomRollOpcode(WorldPacket& recvPacket);
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
        // At Login
        void handleCharRenameOpcode(WorldPacket& recvPacket);
#if VERSION_STRING > TBC
        void handleCharCustomizeLooksOpcode(WorldPacket& recvPacket);
        void handleCharFactionOrRaceChange(WorldPacket& recvPacket);
#endif
        void handleReadyForAccountDataTimes(WorldPacket& /*recvPacket*/);

        void handlePartyMemberStatsOpcode(WorldPacket& recvPacket);
        void HandleSummonResponseOpcode(WorldPacket& recvPacket);

        void HandleArenaTeamAddMemberOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamRemoveMemberOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamInviteAcceptOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamInviteDenyOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamLeaveOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamDisbandOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamPromoteOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamQueryOpcode(WorldPacket& recvPacket);
        void HandleArenaTeamRosterOpcode(WorldPacket& recvPacket);
        void handleInspectArenaStatsOpcode(WorldPacket& recvPacket);

        void handleWorldTeleportOpcode(WorldPacket& recvPacket);
        void HandleWrapItemOpcode(WorldPacket& recvPacket);

        // VoicChat
        // Zyres: this feature will be not implemented in the near future!
        //void HandleEnableMicrophoneOpcode(WorldPacket& recvPacket);
        //void HandleVoiceChatQueryOpcode(WorldPacket& recvPacket);
        //void HandleChannelVoiceQueryOpcode(WorldPacket& recvPacket);
        void handleSetAutoLootPassOpcode(WorldPacket& recvPacket);


        // Misc
        void handleWorldStateUITimerUpdate(WorldPacket& recvPacket);
        void handleSetTaxiBenchmarkOpcode(WorldPacket& recvPacket);
        void HandleMirrorImageOpcode(WorldPacket& recvPacket);
        void HandleRemoveGlyph(WorldPacket& recvPacket);
        void handleSetFactionInactiveOpcode(WorldPacket& recvPacket);
#if VERSION_STRING == Cata
        void HandleLogDisconnectOpcode(WorldPacket& recvPacket);
#endif

        // Calendar \todo handle it
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
#if VERSION_STRING == Cata
        void HandleReadyForAccountDataTimesOpcode(WorldPacket& recvPacket);
        void HandleLoadScreenOpcode(WorldPacket& recvPacket);
        void HandleUITimeRequestOpcode(WorldPacket& recvPacket);
        void HandleTimeSyncRespOpcode(WorldPacket& recvPacket);
        void HandleRequestHotfix(WorldPacket& recvPacket);
        void HandleRequestCemeteryListOpcode(WorldPacket& recvPacket);
        void HandleForceSpeedAckOpcodes(WorldPacket& recvPacket);
        void HandleReturnToGraveyardOpcode(WorldPacket& /*recvPacket*/);

        // Reports
        void HandleReportOpcode(WorldPacket& recvPacket);
        void HandleReportPlayerOpcode(WorldPacket& recvPacket);

    private:
        typedef std::list<AddonEntry> AddonsList;
        AddonsList m_addonList;


    public:
        void readAddonInfoPacket(ByteBuffer& recvPacket);
        void sendAddonInfo();
#endif

        void Unhandled(WorldPacket& recvPacket);
        void nothingToHandle(WorldPacket& recvPacket);

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
        void HandleUnregisterAddonPrefixesOpcode(WorldPacket& /*recvPacket*/);
        void HandleAddonRegisteredPrefixesOpcode(WorldPacket& recvPacket);
#endif

};

#endif // WORLDSESSION_H
