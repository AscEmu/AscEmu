/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Chat/ChatDefines.hpp"
#include "Server/WorldSession.h"

#include "Management/Guild/GuildDefinitions.h"
#include "GuildEmblemInfo.h"
#include "GuildBankRightsAndSlots.h"
#include "GuildLog.h"
#include "GuildEventLog.h"
#include "GuildNewsLog.h"
#include "GuildBankEventLog.h"
#include "GuildLogHolder.h"
#include "GuildRankInfo.h"
#include "GuildBankTab.h"

#include <string>


class Player;
class EmblemInfo;

typedef std::vector<GuildBankRightsAndSlots> GuildBankRightsAndSlotsVec;

typedef std::set<uint8_t> SlotIds;

class SERVER_DECL Guild
{
    class GuildMember
    {
        public:

            GuildMember(uint32_t guildId, uint64_t guid, uint8_t rankId);

            void setStats(Player* player);
            void setStats(std::string const& name, uint8_t level, uint8_t _class, uint32_t zoneId, uint32_t accountId, uint32_t reputation);
            bool checkStats() const;

            void setPublicNote(std::string const& publicNote);
            void setOfficerNote(std::string const& officerNote);

            void setZoneId(uint32_t id);
            void setAchievementPoints(uint32_t val);
            void setLevel(uint8_t var);

            void addFlag(uint8_t var);
            void removeFlag(uint8_t var);
            void resetFlags();

            bool loadGuildMembersFromDB(Field* fields, Field* fields2);
            void saveGuildMembersToDB(bool _delete) const;

            uint64_t getGUID() const;
            std::string const& getName() const;
            uint32_t getAccountId() const;
            uint8_t getRankId() const;

            uint64_t getLogoutTime() const;

            std::string getPublicNote() const;
            std::string getOfficerNote() const;

            uint8_t getClass() const;
            uint8_t getLevel() const;
            uint8_t getFlags() const;
            uint32_t getZoneId() const;

            uint32_t getAchievementPoints() const;

            uint64_t getTotalActivity() const;
            uint64_t getWeekActivity() const;
            uint32_t getTotalReputation() const;
            uint32_t getWeekReputation() const;

            bool isOnline();

            void changeRank(uint8_t newRank);

            void updateLogoutTime();
            bool isRank(uint8_t rankId) const;
            bool isRankNotLower(uint8_t rankId) const;
            bool isSamePlayer(uint64_t guid) const;

            void updateBankWithdrawValue(uint8_t tabId, uint32_t amount);
            int32_t getBankWithdrawValue(uint8_t tabId) const;
            void resetValues(bool weekly = false);

            Player* getPlayerByGuid(uint64_t guid);

        private:

            uint32_t mGuildId;

            uint64_t mGuid;
            std::string mName;
            uint32_t mZoneId;
            uint8_t mLevel;
            uint8_t mClass;
            uint8_t mFlags;
            uint64_t mLogoutTime;
            uint32_t mAccountId;

            uint8_t mRankId;
            std::string mPublicNote;
            std::string mOfficerNote;

            int32_t mBankWithdraw[MAX_GUILD_BANK_TABS + 1];
            uint32_t mAchievementPoints;
            uint64_t mTotalActivity;
            uint64_t mWeekActivity;
            uint32_t mTotalReputation;
            uint32_t mWeekReputation;
    };

    private:

        typedef std::vector<GuildRankInfo> GuildRankInfoStore;
        typedef std::vector<GuildBankTab*> GuildBankTabsStore;
        typedef std::map<uint32_t, class GuildMember*> GuildMembersStore;

    public:

        static void sendCommandResult(WorldSession* session, GuildCommandType type, GuildCommandError errCode, std::string const& param = "");
        static void sendSaveEmblemResult(WorldSession* session, GuildEmblemError errCode);

        Guild();
        ~Guild();

        bool create(Player* pLeader, std::string const& name);
        void disband();

        void saveGuildToDB();

        uint32_t getId() const { return mId; }
        uint32_t getGuildId() const { return getId(); }

        uint64_t getGUID() const { return MAKE_NEW_GUID(mId, 0, HIGHGUID_TYPE_GUILD); }
        uint64_t getLeaderGUID() const { return mLeaderGuid; }
        std::string const& getName() const { return mName; }
        std::string const& getMOTD() const { return mMotd; }
        std::string const& getInfo() const { return mInfo; }
        const char* getGuildName() const { return getName().c_str(); }

        void handleRoster(WorldSession* session = nullptr);
        void handleQuery(WorldSession* session);
        void handleSetMOTD(WorldSession* session, std::string const& motd);
        void handleSetInfo(WorldSession* session, std::string const& info);
        void handleSetEmblem(WorldSession* session, const EmblemInfo& emblemInfo);
        void handleSetNewGuildMaster(WorldSession* session, std::string const& name);
        void handleSetBankTabInfo(WorldSession* session, uint8_t tabId, std::string const& name, std::string const& icon);
        void handleSetMemberNote(WorldSession* session, std::string const& note, uint64_t guid, bool isPublic);
        void handleSetRankInfo(WorldSession* session, uint8_t rankId, std::string const& name, uint32_t rights, uint32_t moneyPerDay, GuildBankRightsAndSlotsVec rightsAndSlots);
        void handleBuyBankTab(WorldSession* session, uint8_t tabId);
        void handleInviteMember(WorldSession* session, std::string const& name);
        void handleAcceptMember(WorldSession* session);
        void handleLeaveMember(WorldSession* session);
        void handleRemoveMember(WorldSession* session, uint64_t guid);
        void handleUpdateMemberRank(WorldSession* session, uint64_t guid, bool demote);
        void handleSetMemberRank(WorldSession* session, uint64_t guid, uint64_t setterGuid, uint32_t rank);
        void handleAddNewRank(WorldSession* session, std::string const& name);
        void handleRemoveRank(WorldSession* session, uint8_t rankId);
        void handleMemberDepositMoney(WorldSession* session, uint64_t amount, bool cashFlow = false);
        bool handleMemberWithdrawMoney(WorldSession* session, uint64_t amount, bool repair = false);
        void handleMemberLogout(WorldSession* session);
        void handleDisband(WorldSession* session);
        void handleGuildPartyRequest(WorldSession* session);
        void handleNewsSetSticky(WorldSession* session, uint32_t newsId, bool sticky);
        void handleGuildRequestChallengeUpdate(WorldSession* session);

        void updateMemberData(Player* player, uint8_t dataid, uint32_t value);
        void onPlayerStatusChange(Player* player, uint32_t flag, bool state);

        void sendGuildRankInfo(WorldSession* session) const;
        void sendEventLog(WorldSession* session) const;
        void sendBankLog(WorldSession* session, uint8_t tabId) const;
        void sendBankList(WorldSession* session, uint8_t tabId, bool withContent, bool withTabInfo) const;
        void sendGuildXP(WorldSession* session = nullptr) const;
        void sendBankTabText(WorldSession* session, uint8_t tabId) const;
        void sendPermissions(WorldSession* session) const;
        void sendMoneyInfo(WorldSession* session) const;
        void sendLoginInfo(WorldSession* session);
        void sendNewsUpdate(WorldSession* session);
        static void sendTurnInPetitionResult(WorldSession* pClient, uint32_t result);
        void _sendBankContentUpdate(uint8_t tabId, SlotIds slots) const;

        bool loadGuildFromDB(Field* fields);
        void loadGuildNewsLogFromDB(Field* fields);
        void loadRankFromDB(Field* fields);
        bool loadMemberFromDB(Field* fields, Field* fields2);
        bool loadEventLogFromDB(Field* fields);
        void loadBankRightFromDB(Field* fields);
        void loadBankTabFromDB(Field* fields);
        bool loadBankEventLogFromDB(Field* fields);
        bool loadBankItemFromDB(Field* fields);
        bool validate();

        void broadcastToGuild(WorldSession* session, bool officerOnly, std::string const& msg, uint32_t language = LANG_UNIVERSAL) const;
        void broadcastAddonToGuild(WorldSession* session, bool officerOnly, std::string const& msg, std::string const& prefix) const;
        void broadcastPacketToRank(WorldPacket* packet, uint8_t rankId) const;
        void broadcastPacket(WorldPacket* packet) const;
        void sendPacket(WorldPacket* packet) const { broadcastPacket(packet); }

        void massInviteToEvent(WorldSession* session, uint32_t minLevel, uint32_t maxLevel, uint32_t minRank);

        template<class Do>
        void broadcastWorker(Do& _do, Player* except = nullptr)
        {
            for (GuildMembersStore::iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
            {
                if (Player* player = itr->second->getPlayerByGuid(itr->second->getGUID()))
                {
                    if (player != except)
                    {
                        _do(player);
                    }
                }
            }
        }

        bool addMember(uint64_t guid, uint8_t rankId = GUILD_RANK_NONE);
        void deleteMember(uint64_t guid, bool isDisbanding = false, bool isKicked = false);
        bool changeMemberRank(uint64_t guid, uint8_t newRank);
        bool isMember(uint64_t guid) const;
        uint32_t getMembersCount() { return static_cast<uint32_t>(_guildMembersStore.size()); }

        void swapItems(Player* player, uint8_t tabId, uint8_t slotId, uint8_t destTabId, uint8_t destSlotId, uint32_t splitedAmount);
        void swapItemsWithInventory(Player* player, bool toChar, uint8_t tabId, uint8_t slotId, uint8_t playerBag, uint8_t playerSlotId, uint32_t splitedAmount);

        void setBankTabText(uint8_t tabId, std::string const& text);

        uint8_t getLevel() const { return _level; }
        void giveXP(uint32_t xp, Player* source);
        uint64_t getExperience() const { return _experience; }
        uint64_t getTodayExperience() const { return _todayExperience; }

        void addGuildNews(uint8_t type, uint64_t guid, uint32_t flags, uint32_t value);

        EmblemInfo const& getEmblemInfo() const { return mEmblemInfo; }
        void resetTimes(bool weekly);

        bool hasAchieved(uint32_t achievementId) const;

    protected:

        uint32_t mId;
        std::string mName;
        uint64_t mLeaderGuid;
        std::string mMotd;
        std::string mInfo;
        time_t mCreatedDate;

        EmblemInfo mEmblemInfo;
        uint32_t mAccountsNumber;
        uint64_t mBankMoney;

        GuildRankInfoStore _guildRankInfoStore;
        GuildMembersStore _guildMembersStore;
        GuildBankTabsStore _guildBankTabsStore;

        GuildLogHolder* mEventLog;
        GuildLogHolder* mBankEventLog[MAX_GUILD_BANK_TABS + 1];
        GuildLogHolder* mNewsLog;

        uint8_t _level;
        uint64_t _experience;
        uint64_t _todayExperience;

    private:

        inline uint8_t _getRanksSize() const { return uint8_t(_guildRankInfoStore.size()); }
        inline const GuildRankInfo* getRankInfo(uint8_t rankId) const { return rankId < _getRanksSize() ? &_guildRankInfoStore[rankId] : nullptr; }
        inline GuildRankInfo* getRankInfo(uint8_t rankId) { return rankId < _getRanksSize() ? &_guildRankInfoStore[rankId] : nullptr; }
        inline bool _hasRankRight(uint64_t playerGuid, uint32_t right) const
        {
            if (playerGuid)
            {
                if (GuildMember const* member = getMember(playerGuid))
                {
                    return (getRankRights(member->getRankId()) & right) != GR_RIGHT_EMPTY;
                }
            }

            return false;
        }

        inline uint8_t _getLowestRankId() const { return uint8_t(_guildRankInfoStore.size() - 1); }

        inline uint8_t _getPurchasedTabsSize() const { return uint8_t(_guildBankTabsStore.size()); }
        inline GuildBankTab* getBankTab(uint8_t tabId) { return tabId < _guildBankTabsStore.size() ? _guildBankTabsStore[tabId] : nullptr; }
        inline const GuildBankTab* getBankTab(uint8_t tabId) const { return tabId < _guildBankTabsStore.size() ? _guildBankTabsStore[tabId] : nullptr; }

        inline const GuildMember* getMember(uint64_t guid) const
        {
            GuildMembersStore::const_iterator itr = _guildMembersStore.find(Arcemu::Util::GUID_LOPART(guid));
            return itr != _guildMembersStore.end() ? itr->second : nullptr;
        }

        inline GuildMember* getMember(uint64_t guid)
        {
            GuildMembersStore::iterator itr = _guildMembersStore.find(Arcemu::Util::GUID_LOPART(guid));
            return itr != _guildMembersStore.end() ? itr->second : nullptr;
        }

        inline GuildMember* getMember(std::string const& name)
        {
            for (GuildMembersStore::iterator itr = _guildMembersStore.begin(); itr != _guildMembersStore.end(); ++itr)
            {
                if (itr->second->getName() == name)
                {
                    return itr->second;
                }
            }

            return nullptr;
        }

        void createLogHolders();
        void createNewBankTab();
        void createDefaultGuildRanks();
        bool createRank(std::string const& name, uint32_t rights);

        void updateAccountsNumber();
        bool isLeader(Player* player) const;
        void deleteBankItems(bool removeItemsFromDB = false);
        bool modifyBankMoney(uint64_t amount, bool add);
        void setLeaderGuid(GuildMember* leader);

        void setRankBankMoneyPerDay(uint8_t rankId, uint32_t moneyPerDay);
        void setRankBankTabRightsAndSlots(uint8_t rankId, GuildBankRightsAndSlots rightsAndSlots, bool saveToDB = true);
        int8_t getRankBankTabRights(uint8_t rankId, uint8_t tabId) const;
        uint32_t getRankRights(uint8_t rankId) const;
        int32_t getRankBankMoneyPerDay(uint8_t rankId) const;
        int32_t getRankBankTabSlotsPerDay(uint8_t rankId, uint8_t tabId) const;
        std::string getRankName(uint8_t rankId) const;

        int32_t getMemberRemainingSlots(GuildMember const* member, uint8_t tabId) const;
        int32_t getMemberRemainingMoney(GuildMember const* member) const;
        void updateMemberWithdrawSlots(uint64_t guid, uint8_t tabId);
        bool memberHasTabRights(uint64_t guid, uint8_t tabId, uint32_t rights) const;

        void logEvent(GuildEventLogTypes eventType, uint32_t playerGuid1, uint32_t playerGuid2 = 0, uint8_t newRank = 0);
        void logBankEvent(GuildBankEventLogTypes eventType, uint8_t tabId, uint32_t playerGuid, uint32_t itemOrMoney, uint16_t itemStackCount = 0, uint8_t destTabId = 0);

        void sendGuildReputationWeeklyCap(WorldSession* session, uint32_t reputation) const;
        void sendGuildRanksUpdate(uint64_t setterGuid, uint64_t targetGuid, uint32_t rank);

        void broadcastEvent(GuildEvents guildEvent, uint64_t guid, const char* var_1 = NULL, const char* var_2 = NULL, const char* var_3 = NULL) const;
};
