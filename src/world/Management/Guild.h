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

#pragma once

// AGPL END
// MIT START
#include "WorldConf.h"
#include "CommonTypes.hpp"
#include "Guild/GuildDefinitions.h"
#include "Management/Item.h"
#include "GuildEmblemInfo.h"


#if VERSION_STRING != Cata


// MIT END
//AGPL START
class PlayerInfo;


struct SERVER_DECL GuildRankTabPermissions
{
    uint32 iFlags;
    int32 iStacksPerDay;
};

struct SERVER_DECL GuildRank
{
    uint32 iId;
    uint32 iRights;
    int32 iGoldLimitPerDay;
    GuildRankTabPermissions iTabPermissions[MAX_GUILD_BANK_TABS];
    char* szRankName;
    bool CanPerformCommand(uint32 t);
    bool CanPerformBankCommand(uint32 t, uint32 tab);
};

struct SERVER_DECL GuildMember
{
    PlayerInfo* pPlayer;
    const char* szPublicNote;
    const char* szOfficerNote;
    GuildRank* pRank;
    uint32 uLastWithdrawReset;
    uint32 uWithdrawlsSinceLastReset;
    uint32 uLastItemWithdrawReset[MAX_GUILD_BANK_TABS];
    uint32 uItemWithdrawlsSinceLastReset[MAX_GUILD_BANK_TABS];

    uint32 CalculateAllowedItemWithdraws(uint32 tab);
    void OnItemWithdraw(uint32 tabid);

    uint32 CalculateAvailableAmount();
    bool RepairItem(uint32 cost);
    void OnMoneyWithdraw(uint32 amt);
};

struct SERVER_DECL GuildLogEvent
{
    uint32 iLogId;
    uint8 iEvent;
    uint32 iTimeStamp;
    uint32 iEventData[3];
};

struct SERVER_DECL GuildBankEvent
{
    uint32 iLogId;
    uint8 iAction;
    uint32 uPlayer;
    uint32 uEntry;
    uint8 iStack;
    uint32 uTimeStamp;
};

struct SERVER_DECL GuildBankTab
{
    uint8 iTabId;
    char* szTabName;
    char* szTabIcon;
    char* szTabInfo;
    Item* pSlots[MAX_GUILD_BANK_SLOTS];
    std::list<GuildBankEvent*> lLog;
};

class Charter;

typedef std::map<PlayerInfo*, GuildMember*> GuildMemberMap;

class SERVER_DECL Guild
{
    // MIT START
    protected:

        uint32_t m_id;
        std::string m_name;
        uint64_t m_leaderGuid;
        std::string m_motd;
        std::string m_info;
        uint32_t m_createdDate;

    public:
        //////////////////////////////////////////////////////////////////////////////////////////
        // new functions to serialize Guild classes

        uint32_t getId() const { return m_id; }

        uint64_t getGUID() const { return MAKE_NEW_GUID(m_id, 0, HIGHGUID_TYPE_GUILD); }
        uint64_t getLeaderGUID() const { return m_leaderGuid; }

        uint32 getLeaderGUIDLow() const { return Arcemu::Util::GUID_LOPART(m_leaderGuid); }

        std::string const& getName() const { return m_name; }
        std::string const& getMOTD() const { return m_motd; }
        std::string const& getInfo() const { return m_info; }

        //\brief: Used only in LuAEngine!
        const char* getNameChar() const { return m_name.c_str(); }
        const char* getMOTDChar() const { return m_motd.c_str(); }


        // misc
        uint32_t getMembersCount() const { return uint32_t(m_members.size()); }

        EmblemInfo m_emblemInfo;
        void setEmblemInfo(EmblemInfo emblem_info);
        EmblemInfo const& getEmblemInfo() const { return m_emblemInfo; }

        //////////////////////////////////////////////////////////////////////////////////////////
    // MIT END

        Guild();
        ~Guild();

        static Guild* Create();
        bool LoadFromDB(Field* f);


    protected:

        // Log entry processing
        uint32 m_hiLogId;

    public:

        uint32 GenerateGuildLogEventId();

        // guild bank logging calls
        void LogGuildBankActionMoney(uint8 iAction, uint32 uGuid, uint32 uAmount);
        void LogGuildBankAction(uint8 iAction, uint32 uGuid, uint32 uEntry, uint8 iStack, GuildBankTab* pTab);
        static void ClearOutOfDateLogEntries();

        // only call at first create/save 
        void CreateInDB();

        void SetMOTD(const char* szNewMotd, WorldSession* pClient);

        void SetGuildInformation(const char* szGuildInformation, WorldSession* pClient);

        void SendGuildRoster(WorldSession* pClient);

        void SendGuildQuery(WorldSession* pClient);

        void AddGuildMember(PlayerInfo* pMember, WorldSession* pClient, int32 ForcedRank = -1);

        // If this member is the guild master, the guild will be automatically handed down to the next highest member.
        void RemoveGuildMember(PlayerInfo* pMember, WorldSession* pClient);

        // Do not use for changing guild master.Use ChangeGuildMaster() for that instead.
        void PromoteGuildMember(PlayerInfo* pMember, WorldSession* pClient);

        // Do not use for changing guild master. Use ChangeGuildMaster() for that instead.
        void DemoteGuildMember(PlayerInfo* pMember, WorldSession* pClient);

        void ChangeGuildMaster(PlayerInfo* pNewMaster, WorldSession* pClient);

        static void SendTurnInPetitionResult(WorldSession* pClient, uint32 result);

        void LogGuildEvent(uint8 iEvent, uint8 iStringCount, ...);

        void AddGuildLogEntry(uint8 iEvent, uint8 iParamCount, ...);

        void CreateFromCharter(Charter* pCharter, WorldSession* pTurnIn);

        void sendPacket(WorldPacket* data);

        // Sends a guild chat message.
        void GuildChat(const char* szMessage, WorldSession* pClient, uint32 iType);

        // Sends an officer chat message.
        void OfficerChat(const char* szMessage, WorldSession* pClient, uint32 iType);

        void SendGuildLog(WorldSession* pClient);
        void SendGuildBankLog(WorldSession* pClient, uint8 iSlot);

        void SetPublicNote(PlayerInfo* pMember, const char* szNewNote, WorldSession* pClient);

        void SetOfficerNote(PlayerInfo* pMember, const char* szNewNote, WorldSession* pClient);

        void disband();

        // creation time stuff
        uint32 creationDay;
        uint32 creationMonth;
        uint32 creationYear;

        inline const uint8  GetBankTabCount() const { return (uint8)m_bankTabs.size(); }
        inline const uint64 GetBankBalance() const { return m_bankBalance; }


        // Creates a guild rank with the specified permissions.
        GuildRank* CreateGuildRank(const char* szRankName, uint32 iPermissions, bool bFullGuildBankPermissions);

        // "Pops" or removes the bottom guild rank.
        void RemoveGuildRank(WorldSession* pClient);

        // Buys a new guild bank tab, usable only by guild master
        void BuyBankTab(WorldSession* pClient);

        // Deposits money into the guild bank, usable by any member.
        void DepositMoney(WorldSession* pClient, uint32 uAmount);

        // Withdraws money from the guild bank, usable by members with that permission.
        void WithdrawMoney(WorldSession* pClient, uint32 uAmount);

        // Decrease the guild balance of uAmount
        void SpendMoney(uint32 uAmount);

        // Retrieves a guild rank for editing
        inline GuildRank* GetGuildRank(uint32 Id)
        {
            if (Id >= MAX_GUILD_RANKS)
                return NULL;

            return m_ranks[Id];
        }

        // Gets a guild bank tab for editing/viewing
        inline GuildBankTab* GetBankTab(uint8 Id)
        {
            if (Id >= GetBankTabCount())
                return NULL;

            return m_bankTabs[Id];
        }

        // Gets a guild member struct
        inline GuildMember* GetGuildMember(PlayerInfo* pInfo)
        {
            GuildMemberMap::iterator itr;
            GuildMember* ret;
            m_lock.Acquire();
            itr = m_members.find(pInfo);
            ret = (itr != m_members.end()) ? itr->second : NULL;
            m_lock.Release();
            return ret;
        }

        // Get iterators
        inline GuildMemberMap::iterator GetGuildMembersBegin() { return m_members.begin(); }
        inline GuildMemberMap::iterator GetGuildMembersEnd() { return m_members.end(); }

        // Get, Lock, Unlock Mutex
        inline Mutex & getLock() { return m_lock; }
        inline void Lock() { m_lock.Acquire(); }
        inline void Unlock() { return m_lock.Release(); }

        // Sends the guild bank to this client.
        void SendGuildBank(WorldSession* pClient, GuildBankTab* pTab, int8 updated_slot1 = -1, int8 updated_slot2 = -1);
        void SendGuildBankInfo(WorldSession* pClient);

        // Sends the guild information packet to the specified client.
        void SendGuildInfo(WorldSession* pClient);

    protected:

        // Enables/disables command logging.
        // Use when performing mass events such as guild creation or destruction.
        bool m_commandLogging;

        // Internal variables
                
        uint64 m_bankBalance; //use a 64 bit int so we can store more gold in the gbank

        typedef std::vector<GuildBankTab*> GuildBankTabVector;
        GuildBankTabVector m_bankTabs;

        
        

        // Guild Member Map.
        //typedef map<PlayerInfo*, GuildMember*> GuildMemberMap;
        GuildMemberMap m_members;

        // Guild Rank Information.
        GuildRank* m_ranks[MAX_GUILD_RANKS];

        // Guild log. Ordered in first event -> last event.
        typedef std::list<GuildLogEvent*> GuildLogList;
        GuildLogList m_log;
        std::list<GuildBankEvent*> m_moneyLog;

        // Guild lock.
        Mutex m_lock;

        // finds the lowest rank
        GuildRank* FindLowestRank();
        GuildRank* FindHighestRank();
};

#endif
