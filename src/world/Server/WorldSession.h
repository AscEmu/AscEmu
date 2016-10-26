/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H


#include <Threading/Mutex.h>
#include "Server/Packets/Opcodes.h"
#include "Management/Quest.hpp"
#include "FastQueue.h"
#include "Units/Unit.h"
#include <stddef.h>

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

#define DB2_REPLY_SPARSE 2442913102
#define DB2_REPLY_ITEM   1344507586

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

struct OpcodeHandler
{
    uint16 status;
    void (WorldSession::*handler)(WorldPacket& recvPacket);
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE                 = 0x0000,
    UPDATEFLAG_SELF                 = 0x0001,
    UPDATEFLAG_TRANSPORT            = 0x0002,
    UPDATEFLAG_HAS_ATTACKING_TARGET = 0x0004,
    UPDATEFLAG_UNK                  = 0x0008,
    UPDATEFLAG_LOW                  = 0x0010,
    UPDATEFLAG_LIVING               = 0x0020,
    UPDATEFLAG_HAS_POSITION         = 0x0040,
    UPDATEFLAG_VEHICLE              = 0x0080,
    UPDATEFLAG_POSITION             = 0x0100,
    UPDATEFLAG_ROTATION             = 0x0200,
    UPDATEFLAG_UNK1                 = 0x0400,
    UPDATEFLAG_ANIM_KITS            = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR        = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS       = 0x2000,
    UPDATEFLAG_UNK2                 = 0x4000,
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

typedef struct Cords
{
    float x, y, z;
} Cords;

enum TrainerSpellState
{
    TRAINER_SPELL_GRAY = 0,
    TRAINER_SPELL_GREEN = 1,
    TRAINER_SPELL_RED = 2,
    TRAINER_SPELL_GREEN_DISABLED = 10   // custom value, not send to client: formally green but learn not allowed
};

extern OpcodeHandler WorldPacketHandlers[NUM_MSG_TYPES];
void CapitalizeString(std::string & arg);

class SERVER_DECL WorldSession
{
    friend class WorldSocket;

    public:

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
        uint8 GetFlags() { return _accountFlags; }

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
                delete [] sAccountData[index].data;
            sAccountData[index].data = data;
            sAccountData[index].sz = sz;
            if (!initial && !sAccountData[index].bIsDirty)      // Mark as "changed" or "dirty"
                sAccountData[index].bIsDirty = true;
            else if (initial)
                sAccountData[index].bIsDirty = false;
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
        void SendAuctionPlaceBidResultPacket(uint32 itemId, uint32 error);
        void SendRefundInfo(uint64 GUID);
        void SendNotInArenaTeamPacket(uint8 type);

        void SetInstance(uint32 Instance) { instanceId = Instance; }
        uint32 GetLatency() const { return _latency; }
        std::string GetAccountName() { return _accountName; }
        const char* GetAccountNameS() const { return _accountName.c_str(); }
        const char* LocalizedWorldSrv(uint32 id);
        const char* LocalizedCreatureTexts(uint32 id);
        const char* LocalizedGossipOption(uint32 id);
        const char* LocalizedMapName(uint32 id);
        const char* LocalizedBroadCast(uint32 id);

        uint16 GetClientBuild() { return client_build; }
        void SetClientBuild(uint16 build) { client_build = build; }
        bool bDeleted;
        uint32 GetInstance() { return instanceId; }
        Mutex deleteMutex;
        void _HandleAreaTriggerOpcode(uint32 id); //real handle
        int32 m_moveDelayTime;
        int32 m_clientTimeDelay;

        void CharacterEnumProc(QueryResult* result);
        void LoadAccountDataProc(QueryResult* result);
        bool IsLoggingOut() { return _loggingOut; }

    protected:

        /// Login screen opcodes (PlayerHandler.cpp):
        void HandleCharEnumOpcode(WorldPacket& recvPacket);
        void HandleCharDeleteOpcode(WorldPacket& recvPacket);
        uint8 DeleteCharacter(uint32 guid);
        void HandleCharCreateOpcode(WorldPacket& recvPacket);
        void HandlePlayerLoginOpcode(WorldPacket& recvPacket);
        void HandleRealmSplitOpcode(WorldPacket& recvPacket);
        void HandleObjectUpdateFailedOpcode(WorldPacket& recv_data);
        //void HandleTimeSyncResp(WorldPacket& recv_data);

        /// Authentification and misc opcodes (MiscHandler.cpp):
        void HandleRepopRequestOpcode(WorldPacket& recvPacket);
        void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);

        //Loot
        void HandleLootMoneyOpcode(WorldPacket& recvPacket);
        void HandleLootOpcode(WorldPacket& recvPacket);
        void HandleLootReleaseOpcode(WorldPacket& recvPacket);
        void HandleLootMasterGiveOpcode(WorldPacket& recv_data);
        void HandleLootRollOpcode(WorldPacket& recv_data);

        void HandleWhoOpcode(WorldPacket& recvPacket);
        void HandleWhoIsOpcode(WorldPacket& recvPacket);
        void HandleLogoutRequestOpcode(WorldPacket& recvPacket);
        //void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
        void HandleLogoutCancelOpcode(WorldPacket& recvPacket);
        void HandleZoneUpdateOpcode(WorldPacket& recvPacket);
        //void HandleSetTargetOpcode(WorldPacket& recvPacket);
        void HandleSetSelectionOpcode(WorldPacket& recvPacket);
        void HandleStandStateChangeOpcode(WorldPacket& recvPacket);
        //void HandleDismountOpcode(WorldPacket& recvPacket);
        void HandleFriendListOpcode(WorldPacket& recvPacket);
        //void HandleAddFriendOpcode(WorldPacket& recvPacket);
        //void HandleDelFriendOpcode(WorldPacket& recvPacket);
        //void HandleAddIgnoreOpcode(WorldPacket& recvPacket);
        //void HandleDelIgnoreOpcode(WorldPacket& recvPacket);
        //void HandleBugOpcode(WorldPacket& recvPacket);
        void HandleAreaTriggerOpcode(WorldPacket& recvPacket);
        //void HandleUpdateAccountData(WorldPacket& recvPacket);
        //void HandleRequestAccountData(WorldPacket& recvPacket);
        void HandleSetActionButtonOpcode(WorldPacket& recvPacket);
        //void HandleSetAtWarOpcode(WorldPacket& recvPacket);
        //void HandleSetWatchedFactionIndexOpcode(WorldPacket& recvPacket);
        void HandleTogglePVPOpcode(WorldPacket& recvPacket);
        //void HandleAmmoSetOpcode(WorldPacket& recvPacket);
        void HandleGameObjectUse(WorldPacket& recvPacket);
        void HandleBarberShopResult(WorldPacket& recvPacket);
        //void HandleLeaveChannelOpcode(WorldPacket& recvPacket);
        void HandlePlayedTimeOpcode(WorldPacket& recv_data);
        void HandleSetSheathedOpcode(WorldPacket& recv_data);
        void HandleCompleteCinematic(WorldPacket& recv_data);
        void HandleNextCinematic(WorldPacket& recv_data);
        void HandleInspectOpcode(WorldPacket& recv_data);
        void HandleGameobjReportUseOpCode(WorldPacket& recv_data);

        /// Gm Ticket System in GMTicket.cpp:
        //void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        //void HandleGMTicketUpdateOpcode(WorldPacket& recvPacket);
        //void HandleGMTicketDeleteOpcode(WorldPacket& recvPacket);
        //void HandleGMTicketGetTicketOpcode(WorldPacket& recvPacket);
        //void HandleGMTicketSystemStatusOpcode(WorldPacket& recvPacket);
        //void HandleGMTicketToggleSystemStatusOpcode(WorldPacket& recvPacket);

        /// Lag report
        //void HandleReportLag(WorldPacket& recvPacket);

        //void HandleGMSurveySubmitOpcode(WorldPacket& recv_data);

        /// Opcodes implemented in QueryHandler.cpp:
        void HandleNameQueryOpcode(WorldPacket& recvPacket);
        void HandleQueryTimeOpcode(WorldPacket& recvPacket);
        void HandleCreatureQueryOpcode(WorldPacket& recvPacket);
        void HandleGameObjectQueryOpcode(WorldPacket& recvPacket);
        //void HandleItemNameQueryOpcode(WorldPacket& recv_data);
        //void HandlePageTextQueryOpcode(WorldPacket& recv_data);
        //void HandleAchievmentQueryOpcode(WorldPacket& recv_data);

        /// Opcodes implemented in MovementHandler.cpp
        void HandleMoveWorldportAckOpcode(WorldPacket& recvPacket);
        void HandleMovementOpcodes(WorldPacket& recvPacket);
        void HandleMoveTimeSkippedOpcode(WorldPacket& recv_data);
        void HandleMoveNotActiveMoverOpcode(WorldPacket& recv_data);
        void HandleSetActiveMoverOpcode(WorldPacket& recv_data);
        void HandleMoveTeleportAckOpcode(WorldPacket& recv_data);

        /// Opcodes implemented in GroupHandler.cpp:
        //void HandleGroupInviteOpcode(WorldPacket& recvPacket);
        //void HandleGroupCancelOpcode(WorldPacket& recvPacket);
        //void HandleGroupAcceptOpcode(WorldPacket& recvPacket);
        //void HandleGroupDeclineOpcode(WorldPacket& recvPacket);
        //void HandleGroupUninviteOpcode(WorldPacket& recvPacket);
        //void HandleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        //void HandleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        //void HandleGroupDisbandOpcode(WorldPacket& recvPacket);
        //void HandleLootMethodOpcode(WorldPacket& recvPacket);
        //void HandleMinimapPingOpcode(WorldPacket& recvPacket);
        //void HandleSetPlayerIconOpcode(WorldPacket& recv_data);
        void SendPartyCommandResult(Player* pPlayer, uint32 p1, std::string name, uint32 err);

        // Raid
        //void HandleConvertGroupToRaidOpcode(WorldPacket& recvPacket);
        //void HandleGroupChangeSubGroup(WorldPacket& recvPacket);
        //void HandleGroupAssistantLeader(WorldPacket& recvPacket);
        void HandleRequestRaidInfoOpcode(WorldPacket& recvPacket);
        //void HandleReadyCheckOpcode(WorldPacket& recv_data);
        //void HandleGroupPromote(WorldPacket& recv_data);

        //LFG
        //void HandleLfgSetCommentOpcode(WorldPacket& recv_data);
        //void HandleLfgJoinOpcode(WorldPacket& recv_data);
        //void HandleLfgLeaveOpcode(WorldPacket& recv_data);
        //void HandleLfrSearchOpcode(WorldPacket& recv_data);
        //void HandleLfrLeaveOpcode(WorldPacket& recv_data);
        //void HandleLfgProposalResultOpcode(WorldPacket& recv_data);
        //void HandleLfgSetRolesOpcode(WorldPacket& recv_data);
        //void HandleLfgSetBootVoteOpcode(WorldPacket& recv_data);
        void HandleLfgPlayerLockInfoRequestOpcode(WorldPacket& recv_data);
        //void HandleLfgTeleportOpcode(WorldPacket& recv_data);
        //void HandleLfgPartyLockInfoRequestOpcode(WorldPacket& recv_data);

        /// Taxi opcodes (TaxiHandler.cpp)
        void HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleTaxiQueryAvaibleNodesOpcode(WorldPacket& recvPacket);
        void HandleActivateTaxiOpcode(WorldPacket& recvPacket);
        //void HandleMultipleActivateTaxiOpcode(WorldPacket& recvPacket);

        /// NPC opcodes (NPCHandler.cpp)
        //void HandleTabardVendorActivateOpcode(WorldPacket& recvPacket);
        //void HandleBankerActivateOpcode(WorldPacket& recvPacket);
        //void HandleBuyBankSlotOpcode(WorldPacket& recvPacket);
        void HandleTrainerListOpcode(WorldPacket& recvPacket);
        void HandleTrainerBuySpellOpcode(WorldPacket& recvPacket);
        //void HandleCharterShowListOpcode(WorldPacket& recvPacket);
        void HandleGossipHelloOpcode(WorldPacket& recvPacket);
        void HandleGossipSelectOptionOpcode(WorldPacket& recvPacket);
        void HandleSpiritHealerActivateOpcode(WorldPacket& recvPacket);
        void HandleNpcTextQueryOpcode(WorldPacket& recvPacket);
        void HandleBinderActivateOpcode(WorldPacket& recvPacket);

        // Auction House opcodes
        void HandleAuctionHelloOpcode(WorldPacket& recvPacket);
        void HandleAuctionListItems(WorldPacket& recv_data);
        void HandleAuctionListBidderItems(WorldPacket& recv_data);
        void HandleAuctionSellItem(WorldPacket& recv_data);
        void HandleAuctionListOwnerItems(WorldPacket& recv_data);
        void HandleAuctionPlaceBid(WorldPacket& recv_data);
        void HandleCancelAuction(WorldPacket& recv_data);
        void HandleAuctionListPendingSales(WorldPacket& recv_data);

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
        //void HandleItemQuerySingleOpcode(WorldPacket& recvPacket);
        void HandleSellItemOpcode(WorldPacket& recvPacket);
        void HandleBuyItemOpcode(WorldPacket& recvPacket);
        void HandleListInventoryOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
        void HandleBuyBackOpcode(WorldPacket& recvPacket);
        void HandleSplitItemOpcode(WorldPacket& recvPacket);
        void HandleReadItemOpcode(WorldPacket& recvPacket);
        void HandleRepairItemOpcode(WorldPacket& recvPacket);
        //void HandleAutoBankItemOpcode(WorldPacket& recvPacket);
        //void HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket);
        void HandleCancelTemporaryEnchantmentOpcode(WorldPacket& recvPacket);
        void HandleInsertGemOpcode(WorldPacket& recvPacket);
        void HandleItemRefundInfoOpcode(WorldPacket& recvPacket);
        //void HandleItemRefundRequestOpcode(WorldPacket& recvPacket);

        // Equipment set opcode
        //void HandleEquipmentSetUse(WorldPacket& data);
        //void HandleEquipmentSetSave(WorldPacket& data);
        //void HandleEquipmentSetDelete(WorldPacket& data);

        /// Combat opcodes (CombatHandler.cpp)
        void HandleAttackSwingOpcode(WorldPacket& recvPacket);
        void HandleAttackStopOpcode(WorldPacket& recvPacket);

        /// Spell opcodes (SpellHandler.cpp)
        void HandleUseItemOpcode(WorldPacket& recvPacket);
        void HandleCastSpellOpcode(WorldPacket& recvPacket);
        void HandleSpellClick(WorldPacket& recvPacket);
        void HandleCancelCastOpcode(WorldPacket& recvPacket);
        //void HandleCancelAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelChannellingOpcode(WorldPacket& recvPacket);
        void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recv_data);
        void HandlePetCastSpell(WorldPacket& recvPacket);
        //void HandleCancelTotem(WorldPacket& recv_data);
        //void HandleUpdateProjectilePosition(WorldPacket& recv_data);

        /// Skill opcodes (SkillHandler.spp)
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
        void HandleQuestPOIQueryOpcode(WorldPacket& recv_data);
    
        /// Vehicles
        //void HandleDismissVehicle(WorldPacket& recv_data);
        //void HandleChangeVehicleSeat(WorldPacket& recv_data);
        //void HandleRemoveVehiclePassenger(WorldPacket& recv_data);
        //void HandleLeaveVehicle(WorldPacket& recv_data);
        //void HandleEnterVehicle(WorldPacket& recv_data);
        void HandleSetActionBarTogglesOpcode(WorldPacket& recvPacket);
        //void HandleMoveSplineCompleteOpcode(WorldPacket& recvPacket);

        /// Chat opcodes (Chat.cpp)
        void HandleMessagechatOpcode(WorldPacket& recvPacket);
        void HandleEmoteOpcode(WorldPacket& recvPacket);
        void HandleTextEmoteOpcode(WorldPacket& recvPacket);
        //void HandleReportSpamOpcode(WorldPacket& recvPacket);
        void HandleChatIgnoredOpcode(WorldPacket& recvPacket);
        void HandleChatChannelWatchOpcode(WorldPacket& recvPacket);

        /// Corpse opcodes (Corpse.cpp)
        void HandleCorpseReclaimOpcode(WorldPacket& recvPacket);
        void HandleCorpseQueryOpcode(WorldPacket& recvPacket);
        //void HandleResurrectResponseOpcode(WorldPacket& recvPacket);

        /// Channel Opcodes (ChannelHandler.cpp)
        void HandleJoinChannelOpcode(WorldPacket& recvPacket);
        void HandleChannelLeave(WorldPacket& recvPacket);
        void HandleChannelList(WorldPacket& recvPacket);
        void HandleChannelPassword(WorldPacket& recvPacket);
        void HandleChannelSetOwner(WorldPacket& recvPacket);
        void HandleChannelOwner(WorldPacket& recvPacket);
        void HandleChannelModerator(WorldPacket& recvPacket);
        void HandleChannelUnmoderator(WorldPacket& recvPacket);
        void HandleChannelMute(WorldPacket& recvPacket);
        void HandleChannelUnmute(WorldPacket& recvPacket);
        void HandleChannelInvite(WorldPacket& recvPacket);
        void HandleChannelKick(WorldPacket& recvPacket);
        void HandleChannelBan(WorldPacket& recvPacket);
        void HandleChannelUnban(WorldPacket& recvPacket);
        void HandleChannelAnnounce(WorldPacket& recvPacket);
        void HandleChannelModerate(WorldPacket& recvPacket);
        void HandleChannelNumMembersQuery(WorldPacket& recvPacket);
        void HandleChannelRosterQuery(WorldPacket& recvPacket);

        // Duel
        //void HandleDuelAccepted(WorldPacket& recv_data);
        //void HandleDuelCancelled(WorldPacket& recv_data);

        // Trade
        //void HandleInitiateTrade(WorldPacket& recv_data);
        //void HandleBeginTrade(WorldPacket& recv_data);
        //void HandleBusyTrade(WorldPacket& recv_data);
        //void HandleIgnoreTrade(WorldPacket& recv_data);
        //void HandleAcceptTrade(WorldPacket& recv_data);
        //void HandleUnacceptTrade(WorldPacket& recv_data);
        void HandleCancelTrade(WorldPacket& recv_data);
        //void HandleSetTradeItem(WorldPacket& recv_data);
        //void HandleClearTradeItem(WorldPacket& recv_data);
        //void HandleSetTradeGold(WorldPacket& recv_data);

        // Guild
        void HandleGuildQueryOpcode(WorldPacket& recv_data);
        //void HandleCreateGuild(WorldPacket& recv_data);
        void HandleInviteToGuildOpcode(WorldPacket& recv_data);
        void HandleGuildRemoveOpcode(WorldPacket& recv_data);
        void HandleGuildAcceptOpcode(WorldPacket& recv_data);
        void HandleGuildDeclineOpcode(WorldPacket& recv_data);
        //void HandleSetGuildInformation(WorldPacket& recv_data);
        //void HandleGuildInfo(WorldPacket& recv_data);
        void HandleGuildRosterOpcode(WorldPacket& recv_data);
        void HandleGuildPromoteOpcode(WorldPacket& recv_data);
        void HandleGuildAssignRankOpcode(WorldPacket& recv_data);
        void HandleGuildDemoteOpcode(WorldPacket& recv_data);
        void HandleGuildLeaveOpcode(WorldPacket& recv_data);
        void HandleGuildDisbandOpcode(WorldPacket& recv_data);
        //void HandleGuildLeaderOpcode(WorldPacket& recv_data);
        void HandleGuildMotdOpcode(WorldPacket& recv_data);
        void HandleGuildSetNoteOpcode(WorldPacket& recv_data);
        void HandleGuildQueryRanksOpcode(WorldPacket& recv_data);
        void HandleGuildAddRankOpcode(WorldPacket& recv_data);
        void HandleGuildDelRankOpcode(WorldPacket& recv_data);
        void HandleGuildChangeInfoTextOpcode(WorldPacket& recv_data);
        void HandleGuildQueryNewsOpcode(WorldPacket& recv_data);
        void HandleGuildNewsUpdateStickyOpcode(WorldPacket& recv_data);
        void HandleGuildSetGuildMaster(WorldPacket& recv_data);
        //void HandleGuildSetPublicNoteOpcode(WorldPacket& recv_data);
        //void HandleGuildSetOfficerNoteOpcode(WorldPacket& recv_data);
        void HandleSaveGuildEmblemOpcode(WorldPacket& recv_data);
        void HandleGuildEventLogQueryOpcode(WorldPacket& recv_data);
        void HandleGuildQueryXPOpcode(WorldPacket& recv_data);
        void HandleGuildSetRankPermissionsOpcode(WorldPacket& recv_data);
        void HandleGuildRequestPartyState(WorldPacket& recv_data);
        void HandleGuildRequestMaxDailyXP(WorldPacket& recv_data);
        void HandleAutoDeclineGuildInvites(WorldPacket& recv_data);
        void HandleGuildRewardsQueryOpcode(WorldPacket& recv_data);
        
        // Guild Bank
        void HandleGuildPermissions(WorldPacket& recv_data);
        void HandleGuildBankMoneyWithdrawn(WorldPacket& recv_data);
        void HandleGuildBankerActivate(WorldPacket& recv_data);
        void HandleGuildBankQueryTab(WorldPacket& recv_data);
        void HandleGuildBankLogQuery(WorldPacket& recv_data);
        void HandleGuildBankDepositMoney(WorldPacket& recv_data);
        void HandleGuildBankWithdrawMoney(WorldPacket& recv_data);
        void HandleGuildBankSwapItems(WorldPacket& recv_data);
        void HandleGuildBankUpdateTab(WorldPacket& recv_data);
        void HandleGuildBankBuyTab(WorldPacket& recv_data);
        void HandleQueryGuildBankTabText(WorldPacket& recv_data);
        void HandleSetGuildBankTabText(WorldPacket& recv_data);
        
        // Charter
        void HandleCharterBuyOpcode(WorldPacket& recv_data);
        void HandleCharterShowSignaturesOpcode(WorldPacket& recv_data);
        void HandleCharterTurnInCharterOpcode(WorldPacket& recv_data);
        void HandleCharterQueryOpcode(WorldPacket& recv_data);
        void HandleCharterOfferOpcode(WorldPacket& recv_data);
        void HandleCharterSignOpcode(WorldPacket& recv_data);
        void HandleCharterDeclineOpcode(WorldPacket& recv_data);
        void HandleCharterRenameOpcode(WorldPacket& recv_data);
        
        //GuildFinder
        void HandleGuildFinderAddRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderBrowse(WorldPacket& recvPacket);
        void HandleGuildFinderDeclineRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderGetApplications(WorldPacket& recvPacket);
        void HandleGuildFinderGetRecruits(WorldPacket& recvPacket);
        void HandleGuildFinderPostRequest(WorldPacket& recvPacket);
        void HandleGuildFinderRemoveRecruit(WorldPacket& recvPacket);
        void HandleGuildFinderSetGuildPost(WorldPacket& recvPacket);

        // Pet
        //void HandlePetAction(WorldPacket& recv_data);
        //void HandlePetInfo(WorldPacket& recv_data);
        //void HandlePetNameQuery(WorldPacket& recv_data);
        //void HandleBuyStableSlot(WorldPacket& recv_data);
        //void HandleStablePet(WorldPacket& recv_data);
        //void HandleUnstablePet(WorldPacket& recv_data);
        //void HandleStabledPetList(WorldPacket& recv_data);
        //void HandleStableSwapPet(WorldPacket& recv_data);
        //void HandlePetRename(WorldPacket& recv_data);
        //void HandlePetAbandon(WorldPacket& recv_data);
        //void HandlePetUnlearn(WorldPacket& recv_data);
        void HandlePetSpellAutocast(WorldPacket& recv_data);
        //void HandlePetCancelAura(WorldPacket& recv_data);
        //void HandlePetLearnTalent(WorldPacket& recv_data);
        //void HandleDismissCritter(WorldPacket& recv_data);

        // Battleground
        //void HandleBattlefieldPortOpcode(WorldPacket& recv_data);
        void HandleBattlefieldStatusOpcode(WorldPacket& recv_data);
        //void HandleBattleMasterHelloOpcode(WorldPacket& recv_data);
        //void HandleLeaveBattlefieldOpcode(WorldPacket& recv_data);
        void HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data);
        void HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data);
        //void HandleBattlegroundPlayerPositionsOpcode(WorldPacket& recv_data);
        //void HandleArenaJoinOpcode(WorldPacket& recv_data);
        //void HandleBattleMasterJoinOpcode(WorldPacket& recv_data);
        void HandleInspectHonorStatsOpcode(WorldPacket& recv_data);
        //void HandlePVPLogDataOpcode(WorldPacket& recv_data);
        void HandleBattlefieldListOpcode(WorldPacket& recv_data);
        ///\todo unknown packet
        //void HandleBgInviteResponse(WorldPacket& recv_data);

        /// Helper functions
        //void SetNpcFlagsForTalkToQuest(const uint64& guid, const uint64& targetGuid);

        // Tutorials
        void HandleTutorialFlag(WorldPacket& recv_data);
        void HandleTutorialClear(WorldPacket& recv_data);
        void HandleTutorialReset(WorldPacket& recv_data);

        // Acknowledgements
        void HandleAcknowledgementOpcodes(WorldPacket& recv_data);
        //void HandleMountSpecialAnimOpcode(WorldPacket& recv_data);

        //void HandleSelfResurrectOpcode(WorldPacket& recv_data);
        //void HandleUnlearnSkillOpcode(WorldPacket& recv_data);
        //void HandleRandomRollOpcode(WorldPacket& recv_data);
        //void HandleOpenItemOpcode(WorldPacket& recv_data);
        void HandleRequestHotfix(WorldPacket& recv_data);
        void SendItemDb2Reply(uint32 entry);
        void SendItemSparseDb2Reply(uint32 entry);

        //void HandleToggleHelmOpcode(WorldPacket& recv_data);
        //void HandleToggleCloakOpcode(WorldPacket& recv_data);
        //void HandleSetVisibleRankOpcode(WorldPacket& recv_data);
        //void HandlePetSetActionOpcode(WorldPacket& recv_data);

        // Instances
        void HandleResetInstanceOpcode(WorldPacket& recv_data);
        void HandleDungeonDifficultyOpcode(WorldPacket& recv_data);
        void HandleRaidDifficultyOpcode(WorldPacket& recv_data);

        TrainerSpellState TrainerGetSpellStatus(TrainerSpell* pSpell);
        void SendMailError(uint32 error);

        // At Login
        void HandleCharRenameOpcode(WorldPacket& recv_data);
        void HandleCharCustomizeLooksOpcode(WorldPacket& recv_data);
        //void HandleCharFactionOrRaceChange(WorldPacket& recv_data);
        void HandleReadyForAccountDataTimes(WorldPacket& /*recvData*/);

        //void HandlePartyMemberStatsOpcode(WorldPacket& recv_data);
        //void HandleSummonResponseOpcode(WorldPacket& recv_data);

        //void HandleArenaTeamAddMemberOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamRemoveMemberOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamInviteAcceptOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamInviteDenyOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamLeaveOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamDisbandOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamPromoteOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamQueryOpcode(WorldPacket& recv_data);
        //void HandleArenaTeamRosterOpcode(WorldPacket& recv_data);
        //void HandleInspectArenaStatsOpcode(WorldPacket& recv_data);

        //void HandleTeleportCheatOpcode(WorldPacket& recv_data);
        //void HandleTeleportToUnitOpcode(WorldPacket& recv_data);
        //void HandleWorldportOpcode(WorldPacket& recv_data);
        //void HandleWrapItemOpcode(WorldPacket& recv_data);

        // VoicChat
        // Zyres: this feature will be not implemented in the near future!
        //void HandleEnableMicrophoneOpcode(WorldPacket& recv_data);
        //void HandleVoiceChatQueryOpcode(WorldPacket& recv_data);
        //void HandleChannelVoiceQueryOpcode(WorldPacket& recv_data);
        //void HandleSetAutoLootPassOpcode(WorldPacket& recv_data);


        // Misc
        void HandleWorldStateUITimerUpdate(WorldPacket& recv_data);
        void HandleSetTaxiBenchmarkOpcode(WorldPacket& recv_data);
        //void HandleMirrorImageOpcode(WorldPacket& recv_data);
        //void HandleSetFriendNote(WorldPacket& recv_data);
        void HandleInrangeQuestgiverQuery(WorldPacket& recv_data);
        //void HandleRemoveGlyph(WorldPacket& recv_data);
        //void HandleSetFactionInactiveOpcode(WorldPacket& recv_data);
        void HandleRequestCemeteryListOpcode(WorldPacket& recv_data);

        // Calendar \todo handle it
        void HandleCalendarGetCalendar(WorldPacket& /*recv_data*/);
        //void HandleCalendarComplain(WorldPacket& recv_data);
        void HandleCalendarGetNumPending(WorldPacket& /*recv_data*/);
        //void HandleCalendarAddEvent(WorldPacket& recv_data);
        //void HandleCalendarGetEvent(WorldPacket& recv_data);
        //void HandleCalendarGuildFilter(WorldPacket& recv_data);
        //void HandleCalendarArenaTeam(WorldPacket& recv_data);
        //void HandleCalendarUpdateEvent(WorldPacket& recv_data);
        //void HandleCalendarRemoveEvent(WorldPacket& recv_data);
        //void HandleCalendarCopyEvent(WorldPacket& recv_data);
        //void HandleCalendarEventInvite(WorldPacket& recv_data);
        //void HandleCalendarEventRsvp(WorldPacket& recv_data);
        //void HandleCalendarEventRemoveInvite(WorldPacket& recv_data);
        //void HandleCalendarEventStatus(WorldPacket& recv_data);
        //void HandleCalendarEventModeratorStatus(WorldPacket& recv_data);

        // 4.3.4
        void HandleReadyForAccountDataTimesOpcode(WorldPacket& recv_data);
        void HandleLoadScreenOpcode(WorldPacket& recv_data);
        void HandleUITimeRequestOpcode(WorldPacket& recv_data); // empty opcode
        void HandleTimeSyncRespOpcode(WorldPacket& recv_data);
        void HandleForceSpeedAckOpcodes(WorldPacket& recv_data);

        void Unhandled(WorldPacket& recv_data);
        void NothingToHandle(WorldPacket& recv_data);

    public:

        void SendInventoryList(Creature* pCreature);
        void SendTrainerList(Creature* pCreature);
        void SendCharterRequest(Creature* pCreature);
        void SendTaxiList(Creature* pCreature);
        void SendInnkeeperBind(Creature* pCreature);
        void SendBattlegroundList(Creature* pCreature, uint32 mapid);
        void SendBankerList(Creature* pCreature);
        void SendTabardHelp(Creature* pCreature);
        void SendAuctionList(Creature* pCreature);
        void SendSpiritHealerRequest(Creature* pCreature);
        void SendAccountDataTimes(uint32 mask);
        void SendStabledPetList(uint64 npcguid);
        void FullLogin(Player* plr);
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
        void LoadPlayerFromDBProc(QueryResultVector & results);

        // Preallocated buffers for movement handlers
        MovementInfo movement_info;

        uint32 _accountId;
        uint32 _accountFlags;
        std::string _accountName;

        bool has_level_55_char; // death knights
        bool has_dk;

        //uint16 _TEMP_ERR_CREATE_CODE; // increments
        int8 _side;

        WoWGuid m_MoverWoWGuid;
        //New Movement System
        uint64 MoverGuid;

        uint32 _logoutTime; // time we received a logout request -- wait 20 seconds, and quit

        AccountDataEntry sAccountData[8];

        FastQueue<WorldPacket*, Mutex> _recvQueue;
        char* permissions;
        int permissioncount;

        bool _loggingOut; //Player is being removed from the game.
        bool LoggingOut; //Player requesting to be logged out

        uint32 _latency;
        uint16 client_build;
        uint32 instanceId;
        uint8 _updatecount;

    public:

        MovementInfo* GetMovementInfo() { return &movement_info; }

        const MovementInfo* GetMovementInfo() const { return &movement_info; }
        static void InitPacketHandlerTable();

        uint32 floodLines;
        time_t floodTime;

        void SystemMessage(const char* format, ...);

        uint32 language;
        void BuildQuestQueryResponse(QuestProperties const* qst);
        uint32 m_muted;

        void SendClientCacheVersion(uint32 version);

};

typedef std::set<WorldSession*> SessionSet;

#endif // _WORLDSESSION_H
