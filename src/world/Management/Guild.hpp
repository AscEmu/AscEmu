/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#ifndef _GUILD_HPP
#define _GUILD_HPP

#define MAX_GUILD_LEVEL         25
extern uint64                   GUILD_LEVEL_XP_REQ[MAX_GUILD_LEVEL + 2];
#define GUILD_DAILY_XP_LIMIT    6246000
#define MAX_GUILD_NEWS          200     //bliz sent up to 400 values here

class Player;

#define ITEM_ENTRY_GUILD_CHARTER    5863

#define ARENA_TEAM_CHARTER_2v2      23560
#define ARENA_TEAM_CHARTER_2v2_COST 800000  // 80 G
#define ARENA_TEAM_CHARTER_3v3      23561
#define ARENA_TEAM_CHARTER_3v3_COST 1200000 // 120 G
#define ARENA_TEAM_CHARTER_5v5      23562
#define ARENA_TEAM_CHARTER_5v5_COST 2000000 // 200 G

#define MAX_GUILD_BANK_SLOTS        98
#define MAX_GUILD_BANK_TABS         6

enum PETITION_TURNIN_ERRORS
{
    ERR_PETITION_OK,
    ERR_PETITION_ALREADY_SIGNED,
    ERR_PETITION_IN_GUILD,
    ERR_PETITION_CREATOR,
    ERR_PETITION_NOT_ENOUGH_SIGNATURES,
};

enum GuildMisc
{
    GUILD_BANK_MAX_TABS             = 8,            // send by client for money log also
    GUILD_BANK_MAX_SLOTS            = 98,
    GUILD_BANK_MONEY_LOGS_TAB       = 100,          // used for money log in DB
    GUILD_RANKS_MIN_COUNT           = 2,
    GUILD_RANKS_MAX_COUNT           = 10,
    GUILD_RANK_NONE                 = 0xFF,
    GUILD_WITHDRAW_MONEY_UNLIMITED  = 0xFFFFFFFF,
    GUILD_WITHDRAW_SLOT_UNLIMITED   = 0xFFFFFFFF,
    GUILD_EVENT_LOG_GUID_UNDEFINED  = 0xFFFFFFFF,
    GUILD_EXPERIENCE_UNCAPPED_LEVEL = 20,           // Hardcoded in client, starting from this level, guild daily experience gain is unlimited.
    TAB_UNDEFINED = 0xFF,
};

enum GuildMemberData
{
    GUILD_MEMBER_DATA_ZONEID,
    GUILD_MEMBER_DATA_ACHIEVEMENT_POINTS,
    GUILD_MEMBER_DATA_LEVEL,
};


enum GuildDefaultRanks
{
    // These ranks can be modified, but they cannot be deleted
    GR_GUILDMASTER  = 0,
    GR_OFFICER      = 1,
    GR_VETERAN      = 2,
    GR_MEMBER       = 3,
    GR_INITIATE     = 4
    // When promoting member server does: rank--
    // When demoting member server does: rank++
};

enum GuildRankRights
{
    GR_RIGHT_EMPTY                  = 0x00000040,
    GR_RIGHT_GCHATLISTEN            = GR_RIGHT_EMPTY | 0x00000001,
    GR_RIGHT_GCHATSPEAK             = GR_RIGHT_EMPTY | 0x00000002,
    GR_RIGHT_OFFCHATLISTEN          = GR_RIGHT_EMPTY | 0x00000004,
    GR_RIGHT_OFFCHATSPEAK           = GR_RIGHT_EMPTY | 0x00000008,
    GR_RIGHT_INVITE                 = GR_RIGHT_EMPTY | 0x00000010,
    GR_RIGHT_REMOVE                 = GR_RIGHT_EMPTY | 0x00000020,
    GR_RIGHT_PROMOTE                = GR_RIGHT_EMPTY | 0x00000080,
    GR_RIGHT_DEMOTE                 = GR_RIGHT_EMPTY | 0x00000100,
    GR_RIGHT_SETMOTD                = GR_RIGHT_EMPTY | 0x00001000,
    GR_RIGHT_EPNOTE                 = GR_RIGHT_EMPTY | 0x00002000,
    GR_RIGHT_VIEWOFFNOTE            = GR_RIGHT_EMPTY | 0x00004000,
    GR_RIGHT_EOFFNOTE               = GR_RIGHT_EMPTY | 0x00008000,
    GR_RIGHT_MODIFY_GUILD_INFO      = GR_RIGHT_EMPTY | 0x00010000,
    GR_RIGHT_WITHDRAW_GOLD_LOCK     = 0x00020000,                   // remove money withdraw capacity
    GR_RIGHT_WITHDRAW_REPAIR        = 0x00040000,                   // withdraw for repair
    GR_RIGHT_WITHDRAW_GOLD          = 0x00080000,                   // withdraw gold
    GR_RIGHT_CREATE_GUILD_EVENT     = 0x00100000,                   // wotlk
    GR_RIGHT_ALL                    = 0x00DDFFBF
};

enum GuildCommandType
{
    GUILD_COMMAND_CREATE            = 0,
    GUILD_COMMAND_INVITE            = 1,
    GUILD_COMMAND_QUIT              = 3,
    GUILD_COMMAND_ROSTER            = 5,
    GUILD_COMMAND_PROMOTE           = 6,
    GUILD_COMMAND_DEMOTE            = 7,
    GUILD_COMMAND_REMOVE            = 8,
    GUILD_COMMAND_CHANGE_LEADER     = 10,
    GUILD_COMMAND_EDIT_MOTD         = 11,
    GUILD_COMMAND_GUILD_CHAT        = 13,
    GUILD_COMMAND_FOUNDER           = 14,
    GUILD_COMMAND_CHANGE_RANK       = 16,
    GUILD_COMMAND_PUBLIC_NOTE       = 19,
    GUILD_COMMAND_VIEW_TAB          = 21,
    GUILD_COMMAND_MOVE_ITEM         = 22,
    GUILD_COMMAND_REPAIR            = 25,
};

enum GuildCommandError
{
    ERR_GUILD_COMMAND_SUCCESS           = 0,
    ERR_GUILD_INTERNAL                  = 1,
    ERR_ALREADY_IN_GUILD                = 2,
    ERR_ALREADY_IN_GUILD_S              = 3,
    ERR_INVITED_TO_GUILD                = 4,
    ERR_ALREADY_INVITED_TO_GUILD_S      = 5,
    ERR_GUILD_NAME_INVALID              = 6,
    ERR_GUILD_NAME_EXISTS_S             = 7,
    ERR_GUILD_LEADER_LEAVE              = 8,
    ERR_GUILD_PERMISSIONS               = 8,
    ERR_GUILD_PLAYER_NOT_IN_GUILD       = 9,
    ERR_GUILD_PLAYER_NOT_IN_GUILD_S     = 10,
    ERR_GUILD_PLAYER_NOT_FOUND_S        = 11,
    ERR_GUILD_NOT_ALLIED                = 12,
    ERR_GUILD_RANK_TOO_HIGH_S           = 13,
    ERR_GUILD_RANK_TOO_LOW_S            = 14,
    ERR_GUILD_RANKS_LOCKED              = 17,
    ERR_GUILD_RANK_IN_USE               = 18,
    ERR_GUILD_IGNORING_YOU_S            = 19,
    ERR_GUILD_UNK1                      = 20, // Forces roster update
    ERR_GUILD_WITHDRAW_LIMIT            = 25,
    ERR_GUILD_NOT_ENOUGH_MONEY          = 26,
    ERR_GUILD_BANK_FULL                 = 28,
    ERR_GUILD_ITEM_NOT_FOUND            = 29,
    ERR_GUILD_TOO_MUCH_MONEY            = 31,
    ERR_GUILD_BANK_WRONG_TAB            = 32,
    ERR_RANK_REQUIRES_AUTHENTICATOR     = 34,
    ERR_GUILD_BANK_VOUCHER_FAILED       = 35,
    ERR_GUILD_TRIAL_ACCOUNT             = 36,
    ERR_GUILD_UNDELETABLE_DUE_TO_LEVEL  = 37,
    ERR_GUILD_MOVE_STARTING             = 38,
    ERR_GUILD_REP_TOO_LOW               = 39
};

enum GuildEvents
{
    GE_PROMOTION                    = 1,
    GE_DEMOTION                     = 2,
    GE_MOTD                         = 3,
    GE_JOINED                       = 4,
    GE_LEFT                         = 5,
    GE_REMOVED                      = 6,
    GE_LEADER_IS                    = 7,
    GE_LEADER_CHANGED               = 8,
    GE_DISBANDED                    = 9,
    GE_TABARDCHANGE                 = 10,
    GE_RANK_UPDATED                 = 11,
    GE_RANK_CREATED                 = 12,
    GE_RANK_DELETED                 = 13,
    GE_RANK_ORDER_CHANGED           = 14,
    GE_FOUNDER                      = 15,
    GE_SIGNED_ON                    = 16,
    GE_SIGNED_OFF                   = 17,
    GE_GUILDBANKBAGSLOTS_CHANGED    = 18,
    GE_BANK_TAB_PURCHASED           = 19,
    GE_BANK_TAB_UPDATED             = 20,
    GE_BANK_MONEY_SET               = 21,
    GE_BANK_MONEY_CHANGED           = 22,
    GE_BANK_TEXT_CHANGED            = 23,
    // 24 - error 795
    GE_SIGNED_ON_MOBILE             = 25,
    GE_SIGNED_Off_MOBILE            = 26
};

enum PetitionTurns
{
    PETITION_TURN_OK                    = 0,
    PETITION_TURN_ALREADY_IN_GUILD      = 2,
    PETITION_TURN_NEED_MORE_SIGNATURES  = 4,
    PETITION_TURN_GUILD_PERMISSIONS     = 11,
    PETITION_TURN_GUILD_NAME_INVALID    = 12
};

enum PetitionSigns
{
    PETITION_SIGN_OK                    = 0,
    PETITION_SIGN_ALREADY_SIGNED        = 1,
    PETITION_SIGN_ALREADY_IN_GUILD      = 2,
    PETITION_SIGN_CANT_SIGN_OWN         = 3,
    PETITION_SIGN_NOT_SERVER            = 4,
    PETITION_SIGN_FULL                  = 5,
    PETITION_SIGN_ALREADY_SIGNED_OTHER  = 6,
    PETITION_SIGN_RESTRICTED_ACCOUNT    = 7
};

enum GuildBankRights
{
    GUILD_BANK_RIGHT_VIEW_TAB       = 0x01,
    GUILD_BANK_RIGHT_PUT_ITEM       = 0x02,
    GUILD_BANK_RIGHT_UPDATE_TEXT    = 0x04,

    GUILD_BANK_RIGHT_DEPOSIT_ITEM   = GUILD_BANK_RIGHT_VIEW_TAB | GUILD_BANK_RIGHT_PUT_ITEM,
    GUILD_BANK_RIGHT_FULL           = 0xFF
};

enum GuildBankEventLogTypes
{
    GUILD_BANK_LOG_DEPOSIT_ITEM         = 1,
    GUILD_BANK_LOG_WITHDRAW_ITEM        = 2,
    GUILD_BANK_LOG_MOVE_ITEM            = 3,
    GUILD_BANK_LOG_DEPOSIT_MONEY        = 4,
    GUILD_BANK_LOG_WITHDRAW_MONEY       = 5,
    GUILD_BANK_LOG_REPAIR_MONEY         = 6,
    GUILD_BANK_LOG_MOVE_ITEM2           = 7,
    GUILD_BANK_LOG_UNK1                 = 8,
    GUILD_BANK_LOG_BUY_SLOT             = 9,
    GUILD_BANK_LOG_CASH_FLOW_DEPOSIT    = 10
};

enum GuildEventLogTypes
{
    GUILD_EVENT_LOG_INVITE_PLAYER       = 1,
    GUILD_EVENT_LOG_JOIN_GUILD          = 2,
    GUILD_EVENT_LOG_PROMOTE_PLAYER      = 3,
    GUILD_EVENT_LOG_DEMOTE_PLAYER       = 4,
    GUILD_EVENT_LOG_UNINVITE_PLAYER     = 5,
    GUILD_EVENT_LOG_LEAVE_GUILD         = 6
};

enum GuildEmblemError
{
    ERR_GUILDEMBLEM_SUCCESS                 = 0,
    ERR_GUILDEMBLEM_INVALID_TABARD_COLORS   = 1,
    ERR_GUILDEMBLEM_NOGUILD                 = 2,
    ERR_GUILDEMBLEM_NOTGUILDMASTER          = 3,
    ERR_GUILDEMBLEM_NOTENOUGHMONEY          = 4,
    ERR_GUILDEMBLEM_INVALIDVENDOR           = 5
};

enum GuildMemberFlags
{
    GUILDMEMBER_STATUS_NONE         = 0x0000,
    GUILDMEMBER_STATUS_ONLINE       = 0x0001,
    GUILDMEMBER_STATUS_AFK          = 0x0002,
    GUILDMEMBER_STATUS_DND          = 0x0004,
    GUILDMEMBER_STATUS_MOBILE       = 0x0008    // remote chat from mobile app
};

enum GuildNews
{
    GUILD_NEWS_GUILD_ACHIEVEMENT    = 0,
    GUILD_NEWS_PLAYER_ACHIEVEMENT   = 1,
    GUILD_NEWS_DUNGEON_ENCOUNTER    = 2,        // todo Implement
    GUILD_NEWS_ITEM_LOOTED          = 3,
    GUILD_NEWS_ITEM_CRAFTED         = 4,
    GUILD_NEWS_ITEM_PURCHASED       = 5,
    GUILD_NEWS_LEVEL_UP             = 6
};

struct GuildReward
{
    uint32 Entry;
    int32 Racemask;
    uint64 Price;
    uint32 AchievementId;
    uint8 Standing;
};

uint32 const MinNewsItemLevel[4] = { 61, 90, 200, 353 };

class EmblemInfo
{
    public:

        EmblemInfo() : m_style(0), m_color(0), m_borderStyle(0), m_borderColor(0), m_backgroundColor(0) { }

        void LoadFromDB(Field* fields);
        void SaveToDB(uint32 guildId) const;
        void ReadPacket(WorldPacket& recv);
        void WritePacket(WorldPacket& data) const;

        uint32 GetStyle() const { return m_style; }
        uint32 GetColor() const { return m_color; }
        uint32 GetBorderStyle() const { return m_borderStyle; }
        uint32 GetBorderColor() const { return m_borderColor; }
        uint32 GetBackgroundColor() const { return m_backgroundColor; }

    private:

        uint32 m_style;
        uint32 m_color;
        uint32 m_borderStyle;
        uint32 m_borderColor;
        uint32 m_backgroundColor;
};

// Structure for storing guild bank rights and remaining slots together.
class GuildBankRightsAndSlots
{
    public:

        GuildBankRightsAndSlots() : tabId(TAB_UNDEFINED), rights(0), slots(0) { }
        GuildBankRightsAndSlots(uint8 _tabId) : tabId(_tabId), rights(0), slots(0) { }
        GuildBankRightsAndSlots(uint8 _tabId, uint8 _rights, uint32 _slots) : tabId(_tabId), rights(_rights), slots(_slots) { }

        void SetGuildMasterValues()
        {
            rights = GUILD_BANK_RIGHT_FULL;
            slots = uint32(GUILD_WITHDRAW_SLOT_UNLIMITED);
        }

        void SetTabId(uint8 _tabId) { tabId = _tabId; }
        uint8 GetTabId() const { return tabId; }

        void SetSlots(uint32 _slots) { slots = _slots; }
        uint32 GetSlots() const { return slots; }

        void SetRights(uint8 _rights) { rights = _rights; }
        uint8 GetRights() const { return rights; }

    private:

        uint8 tabId;
        uint8 rights;
        uint32 slots;
};

typedef std::vector <GuildBankRightsAndSlots> GuildBankRightsAndSlotsVec;

typedef std::set <uint8> SlotIds;

class SERVER_DECL Guild
{
    private:

        class GuildMember
        {
            public:

                GuildMember(uint32 guildId, uint64 guid, uint8 rankId) :
                    m_guildId(guildId),
                    m_guid(guid),
                    m_zoneId(0),
                    m_level(0),
                    m_class(0),
                    m_flags(GUILDMEMBER_STATUS_NONE),
                    m_logoutTime(::time(NULL)),
                    m_accountId(0),
                    m_rankId(rankId),
                    m_achievementPoints(0),
                    m_totalActivity(0),
                    m_weekActivity(0),
                    m_totalReputation(0),
                    m_weekReputation(0)
                {
                    memset(m_bankWithdraw, 0, (GUILD_BANK_MAX_TABS + 1) * sizeof(int32));
                }

                void SetStats(Player* player);
                void SetStats(std::string const& name, uint8 level, uint8 _class, uint32 zoneId, uint32 accountId, uint32 reputation);
                bool CheckStats() const;

                void SetPublicNote(std::string const& publicNote);
                void SetOfficerNote(std::string const& officerNote);
                void SetZoneId(uint32 id) { m_zoneId = id; }
                void SetAchievementPoints(uint32 val) { m_achievementPoints = val; }
                void SetLevel(uint8 var) { m_level = var; }
                void AddReputation(uint32& reputation);
                void AddActivity(uint64 activity);

                void AddFlag(uint8 var) { m_flags |= var; }
                void RemFlag(uint8 var) {  m_flags &= ~var; }
                void ResetFlags() {  m_flags = GUILDMEMBER_STATUS_NONE; }

                bool LoadFromDB(Field* fields, Field* fields2);
                void SaveToDB(bool _delete) const;

                uint64 GetGUID() const { return m_guid; }
                std::string const& GetName() const { return m_name; }
                uint32 GetAccountId() const { return m_accountId; }
                uint8 GetRankId() const { return m_rankId; }
                uint64 GetLogoutTime() const { return m_logoutTime; }
                std::string GetPublicNote() const { return m_publicNote; }
                std::string GetOfficerNote() const { return m_officerNote; }
                uint8 GetClass() const { return m_class; }
                uint8 GetLevel() const { return m_level; }
                uint8 GetFlags() const { return m_flags; }
                uint32 GetZoneId() const { return m_zoneId; }
                uint32 GetAchievementPoints() const { return m_achievementPoints; }
                uint64 GetTotalActivity() const { return m_totalActivity; }
                uint64 GetWeekActivity() const { return m_weekActivity; }
                uint32 GetTotalReputation() const { return m_totalReputation; }
                uint32 GetWeekReputation() const { return m_weekReputation; }

                bool IsOnline() { return (m_flags & GUILDMEMBER_STATUS_ONLINE); }

                void ChangeRank(uint8 newRank);

                inline void UpdateLogoutTime() { m_logoutTime = ::time(NULL); }
                inline bool IsRank(uint8 rankId) const { return m_rankId == rankId; }
                inline bool IsRankNotLower(uint8 rankId) const {return m_rankId <= rankId; }
                inline bool IsSamePlayer(uint64 guid) const { return m_guid == guid; }

                void UpdateBankWithdrawValue(uint8 tabId, uint32 amount);
                int32 GetBankWithdrawValue(uint8 tabId) const;
                void ResetValues(bool weekly = false);

                Player* FindPlayer(uint64 guid);

            private:

                uint32 m_guildId;
                // Fields from characters table
                uint64 m_guid;
                std::string m_name;
                uint32 m_zoneId;
                uint8 m_level;
                uint8 m_class;
                uint8 m_flags;
                uint64 m_logoutTime;
                uint32 m_accountId;
                // Fields from guild_member table
                uint8 m_rankId;
                std::string m_publicNote;
                std::string m_officerNote;

                int32 m_bankWithdraw[GUILD_BANK_MAX_TABS + 1];
                uint32 m_achievementPoints;
                uint64 m_totalActivity;
                uint64 m_weekActivity;
                uint32 m_totalReputation;
                uint32 m_weekReputation;
        };


        class GuildLogEntry
        {
            public:

                GuildLogEntry(uint32 guildId, uint32 guid) :
                    m_guildId(guildId),
                    m_guid(guid),
                    m_timestamp(::time(NULL)) { }

                GuildLogEntry(uint32 guildId, uint32 guid, time_t timestamp) :
                    m_guildId(guildId),
                    m_guid(guid),
                    m_timestamp(timestamp) { }

                virtual ~GuildLogEntry() { }

                uint32 GetGUID() const { return m_guid; }
                uint64 GetTimestamp() const { return m_timestamp; }

                virtual void SaveToDB() const = 0;
                virtual void WritePacket(WorldPacket& data, ByteBuffer& content) const = 0;

            protected:

                uint32 m_guildId;
                uint32 m_guid;
                uint64 m_timestamp;
        };


        class GuildEventLogEntry : public GuildLogEntry
        {
            public:

                GuildEventLogEntry(uint32 guildId, uint32 guid, GuildEventLogTypes eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank) :
                    GuildLogEntry(guildId, guid),
                    m_eventType(eventType),
                    m_playerGuid1(playerGuid1),
                    m_playerGuid2(playerGuid2),
                    m_newRank(newRank) { }

                GuildEventLogEntry(uint32 guildId, uint32 guid, time_t timestamp, GuildEventLogTypes eventType, uint32 playerGuid1, uint32 playerGuid2, uint8 newRank) :
                    GuildLogEntry(guildId, guid, timestamp),
                    m_eventType(eventType),
                    m_playerGuid1(playerGuid1),
                    m_playerGuid2(playerGuid2),
                    m_newRank(newRank) { }

                ~GuildEventLogEntry() { }

                void SaveToDB() const;
                void WritePacket(WorldPacket& data, ByteBuffer& content) const;

            private:

                GuildEventLogTypes m_eventType;
                uint32 m_playerGuid1;
                uint32 m_playerGuid2;
                uint8  m_newRank;
        };


        class GuildBankEventLogEntry : public GuildLogEntry
        {
            public:

                static bool IsMoneyEvent(GuildBankEventLogTypes eventType)
                {
                    return
                        eventType == GUILD_BANK_LOG_DEPOSIT_MONEY ||
                        eventType == GUILD_BANK_LOG_WITHDRAW_MONEY ||
                        eventType == GUILD_BANK_LOG_REPAIR_MONEY ||
                        eventType == GUILD_BANK_LOG_CASH_FLOW_DEPOSIT;
                }

                bool IsMoneyEvent() const
                {
                    return IsMoneyEvent(m_eventType);
                }

                GuildBankEventLogEntry(uint32 guildId, uint32 guid, GuildBankEventLogTypes eventType, uint8 tabId, uint32 playerGuid, uint64 itemOrMoney, uint16 itemStackCount, uint8 destTabId) :
                    GuildLogEntry(guildId, guid),
                    m_eventType(eventType),
                    m_bankTabId(tabId),
                    m_playerGuid(playerGuid),
                    m_itemOrMoney(itemOrMoney),
                    m_itemStackCount(itemStackCount),
                    m_destTabId(destTabId) { }

                GuildBankEventLogEntry(uint32 guildId, uint32 guid, time_t timestamp, uint8 tabId, GuildBankEventLogTypes eventType, uint32 playerGuid, uint64 itemOrMoney, uint16 itemStackCount, uint8 destTabId) :
                    GuildLogEntry(guildId, guid, timestamp),
                    m_eventType(eventType),
                    m_bankTabId(tabId),
                    m_playerGuid(playerGuid),
                    m_itemOrMoney(itemOrMoney),
                    m_itemStackCount(itemStackCount),
                    m_destTabId(destTabId) { }

                ~GuildBankEventLogEntry() { }

                void SaveToDB() const;
                void WritePacket(WorldPacket& data, ByteBuffer& content) const;

            private:

                GuildBankEventLogTypes m_eventType;
                uint8 m_bankTabId;
                uint32 m_playerGuid;
                uint64 m_itemOrMoney;
                uint16 m_itemStackCount;
                uint8 m_destTabId;
        };


        class GuildNewsLogEntry : public GuildLogEntry
        {
            public:

                GuildNewsLogEntry(uint32 guildId, uint32 guid, GuildNews type, uint32 playerGuid, uint32 flags, uint32 value) :
                    GuildLogEntry(guildId, guid), m_type(type),
                    m_playerGuid(playerGuid),
                    m_flags(flags),
                    m_value(value) { }

                GuildNewsLogEntry(uint32 guildId, uint32 guid, time_t timestamp, GuildNews type, uint32 playerGuid, uint32 flags, uint32 value) :
                    GuildLogEntry(guildId, guid, timestamp),
                    m_type(type),
                    m_playerGuid(playerGuid),
                    m_flags(flags),
                    m_value(value) { }

                ~GuildNewsLogEntry() { }

                GuildNews GetType() const { return m_type; }
                uint64 GetPlayerGuid() const { return m_playerGuid ? MAKE_NEW_GUID(m_playerGuid, 0, HIGHGUID_TYPE_PLAYER) : 0; }
                uint32 GetValue() const { return m_value; }
                uint32 GetFlags() const { return m_flags; }
                void SetSticky(bool sticky)
                {
                    if (sticky)
                        m_flags |= 1;
                    else
                        m_flags &= ~1;
                }

                void SaveToDB() const;
                void WritePacket(WorldPacket& data, ByteBuffer& content) const;

            private:

                GuildNews m_type;
                uint32 m_playerGuid;
                uint32 m_flags;
                uint32 m_value;
        };

        typedef std::list<GuildLogEntry*> GuildLog;


        class GuildLogHolder
        {
            public:

                GuildLogHolder(uint32 guildId, uint32 maxRecords) :
                    m_guildId(guildId),
                    m_maxRecords(maxRecords),
                    m_nextGUID(uint32(GUILD_EVENT_LOG_GUID_UNDEFINED)) { }

                ~GuildLogHolder();

                uint8 GetSize() const { return uint8(m_log.size()); }

                inline bool CanInsert() const { return m_log.size() < m_maxRecords; }
                void LoadEvent(GuildLogEntry* entry);
                void AddEvent(GuildLogEntry* entry);
                void WritePacket(WorldPacket& data) const;
                uint32 GetNextGUID();
                GuildLog* GetGuildLog() { return &m_log; }

            private:

                GuildLog m_log;
                uint32 m_guildId;
                uint32 m_maxRecords;
                uint32 m_nextGUID;
        };


        class GuildRankInfo
        {
            public:

                GuildRankInfo() :
                    m_guildId(0),
                    m_rankId(GUILD_RANK_NONE),
                    m_rights(GR_RIGHT_EMPTY),
                    m_bankMoneyPerDay(0) { }

                GuildRankInfo(uint32 guildId) :
                    m_guildId(guildId),
                    m_rankId(GUILD_RANK_NONE),
                    m_rights(GR_RIGHT_EMPTY),
                    m_bankMoneyPerDay(0) { }

                GuildRankInfo(uint32 guildId, uint8 rankId, std::string const& name, uint32 rights, uint32 money) :
                    m_guildId(guildId),
                    m_rankId(rankId),
                    m_name(name),
                    m_rights(rights),
                    m_bankMoneyPerDay(money) { }

                void LoadFromDB(Field* fields);
                void SaveToDB(bool _delete) const;

                uint8 GetId() const { return m_rankId; }

                std::string const& GetName() const { return m_name; }
                void SetName(std::string const& name);

                uint32 GetRights() const { return m_rights; }
                void SetRights(uint32 rights);

                int32 GetBankMoneyPerDay() const { return m_bankMoneyPerDay; }
                void SetBankMoneyPerDay(uint32 money);

                inline int8 GetBankTabRights(uint8 tabId) const
                {
                    return tabId < GUILD_BANK_MAX_TABS ? m_bankTabRightsAndSlots[tabId].GetRights() : 0;
                }

                inline int32 GetBankTabSlotsPerDay(uint8 tabId) const
                {
                    return tabId < GUILD_BANK_MAX_TABS ? m_bankTabRightsAndSlots[tabId].GetSlots() : 0;
                }

                void SetBankTabSlotsAndRights(GuildBankRightsAndSlots rightsAndSlots, bool saveToDB);
                void CreateMissingTabsIfNeeded(uint8 ranks, bool _delete, bool logOnCreate = false);

            private:

                uint32 m_guildId;
                uint8 m_rankId;
                std::string m_name;
                uint32 m_rights;
                uint32 m_bankMoneyPerDay;

                GuildBankRightsAndSlots m_bankTabRightsAndSlots[GUILD_BANK_MAX_TABS];
        };

        class GuildBankTab
        {
            public:

                GuildBankTab(uint32 guildId, uint8 tabId) : m_guildId(guildId), m_tabId(tabId)
                {
                    memset(m_items, 0, GUILD_BANK_MAX_SLOTS * sizeof(Item*));
                }

                void LoadFromDB(Field* fields);
                bool LoadItemFromDB(Field* fields);
                void Delete(bool removeItemsFromDB = false);

                void WritePacket(WorldPacket& data) const;
                bool WriteSlotPacket(WorldPacket& data, uint8 slotId, bool ignoreEmpty = true) const;
                void WriteInfoPacket(WorldPacket& data) const
                {
                    data << m_name;
                    data << m_icon;
                }

                void SetInfo(std::string const& name, std::string const& icon);
                void SetText(std::string const& text);
                void SendText(Guild const* guild, WorldSession* session) const;

                std::string const& GetName() const { return m_name; }
                std::string const& GetIcon() const { return m_icon; }
                std::string const& GetText() const { return m_text; }

                inline Item* GetItem(uint8 slotId) const { return slotId < GUILD_BANK_MAX_SLOTS ? m_items[slotId] : nullptr; }
                bool SetItem(uint8 slotId, Item* item);

            private:

                uint32 m_guildId;
                uint8 m_tabId;

                Item* m_items[GUILD_BANK_MAX_SLOTS];
                std::string m_name;
                std::string m_icon;
                std::string m_text;
        };


        typedef std::map<uint32, GuildMember*> Members;
        typedef std::vector<GuildRankInfo> Ranks;
        typedef std::vector<GuildBankTab*> BankTabs;

    public:

        static void SendCommandResult(WorldSession* session, GuildCommandType type, GuildCommandError errCode, std::string const& param = "");
        static void SendSaveEmblemResult(WorldSession* session, GuildEmblemError errCode);

        Guild();
        ~Guild();

        bool Create(Player* pLeader, std::string const& name);
        void Disband();

        void SaveToDB();

        // Getters
        uint32 GetId() const {  return m_id; }
        uint64 GetGUID() const { return MAKE_NEW_GUID(m_id, 0, HIGHGUID_TYPE_GUILD); }
        uint64 GetLeaderGUID() const { return m_leaderGuid; }
        std::string const& GetName() const { return m_name; }
        std::string const& GetMOTD() const { return m_motd; }
        std::string const& GetInfo() const { return m_info; }

        // Handle client commands
        void HandleRoster(WorldSession* session = NULL);
        void HandleQuery(WorldSession* session);
        void HandleSetMOTD(WorldSession* session, std::string const& motd);
        void HandleSetInfo(WorldSession* session, std::string const& info);
        void HandleSetEmblem(WorldSession* session, const EmblemInfo& emblemInfo);
        void HandleSetNewGuildMaster(WorldSession* session, std::string const& name);
        void HandleSetBankTabInfo(WorldSession* session, uint8 tabId, std::string const& name, std::string const& icon);
        void HandleSetMemberNote(WorldSession* session, std::string const& note, uint64 guid, bool isPublic);
        void HandleSetRankInfo(WorldSession* session, uint8 rankId, std::string const& name, uint32 rights, uint32 moneyPerDay, GuildBankRightsAndSlotsVec rightsAndSlots);
        void HandleBuyBankTab(WorldSession* session, uint8 tabId);
        void HandleInviteMember(WorldSession* session, std::string const& name);
        void HandleAcceptMember(WorldSession* session);
        void HandleLeaveMember(WorldSession* session);
        void HandleRemoveMember(WorldSession* session, uint64 guid);
        void HandleUpdateMemberRank(WorldSession* session, uint64 guid, bool demote);
        void HandleSetMemberRank(WorldSession* session, uint64 guid, uint64 setterGuid, uint32 rank);
        void HandleAddNewRank(WorldSession* session, std::string const& name);
        void HandleRemoveRank(WorldSession* session, uint8 rankId);
        void HandleMemberDepositMoney(WorldSession* session, uint64 amount, bool cashFlow = false);
        bool HandleMemberWithdrawMoney(WorldSession* session, uint64 amount, bool repair = false);
        void HandleMemberLogout(WorldSession* session);
        void HandleDisband(WorldSession* session);
        void HandleGuildPartyRequest(WorldSession* session);
        void HandleNewsSetSticky(WorldSession* session, uint32 newsId, bool sticky);

        void UpdateMemberData(Player* player, uint8 dataid, uint32 value);
        void OnPlayerStatusChange(Player* player, uint32 flag, bool state);

        // Send info to client
        void SendGuildRankInfo(WorldSession* session) const;
        void SendEventLog(WorldSession* session) const;
        void SendBankLog(WorldSession* session, uint8 tabId) const;
        void SendBankList(WorldSession* session, uint8 tabId, bool withContent, bool withTabInfo) const;
        void SendGuildXP(WorldSession* session = NULL) const;
        void SendBankTabText(WorldSession* session, uint8 tabId) const;
        void SendPermissions(WorldSession* session) const;
        void SendMoneyInfo(WorldSession* session) const;
        void SendLoginInfo(WorldSession* session);
        void SendNewsUpdate(WorldSession* session);
        static void SendTurnInPetitionResult(WorldSession * pClient, uint32 result);
        void _SendBankContentUpdate(uint8 tabId, SlotIds slots) const;

        // Load from DB
        bool LoadFromDB(Field* fields);
        void LoadGuildNewsLogFromDB(Field* fields);
        void LoadRankFromDB(Field* fields);
        bool LoadMemberFromDB(Field* fields, Field* fields2);
        bool LoadEventLogFromDB(Field* fields);
        void LoadBankRightFromDB(Field* fields);
        void LoadBankTabFromDB(Field* fields);
        bool LoadBankEventLogFromDB(Field* fields);
        bool LoadBankItemFromDB(Field* fields);
        bool Validate();

        // Broadcasts
        void BroadcastToGuild(WorldSession* session, bool officerOnly, std::string const& msg, uint32 language = LANG_UNIVERSAL) const;
        void BroadcastAddonToGuild(WorldSession* session, bool officerOnly, std::string const& msg, std::string const& prefix) const;
        void BroadcastPacketToRank(WorldPacket* packet, uint8 rankId) const;
        void BroadcastPacket(WorldPacket* packet) const;

        void MassInviteToEvent(WorldSession* session, uint32 minLevel, uint32 maxLevel, uint32 minRank);

        template<class Do>
        void BroadcastWorker(Do& _do, Player* except = NULL)
        {
            for (Members::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
                if (Player* player = itr->second->FindPlayer())
                    if (player != except)
                        _do(player);
        }

        // Members
        // Adds member to guild. If rankId == GUILD_RANK_NONE, lowest rank is assigned.
        bool AddMember(uint64 guid, uint8 rankId = GUILD_RANK_NONE);
        void DeleteMember(uint64 guid, bool isDisbanding = false, bool isKicked = false);
        bool ChangeMemberRank(uint64 guid, uint8 newRank);
        bool IsMember(uint64 guid) const;
        uint32 GetMembersCount() { return m_members.size(); }

        // Bank
        void SwapItems(Player* player, uint8 tabId, uint8 slotId, uint8 destTabId, uint8 destSlotId, uint32 splitedAmount);
        void SwapItemsWithInventory(Player* player, bool toChar, uint8 tabId, uint8 slotId, uint8 playerBag, uint8 playerSlotId, uint32 splitedAmount);

        // Bank tabs
        void SetBankTabText(uint8 tabId, std::string const& text);

        //AchievementMgr<Guild>& GetAchievementMgr() { return m_achievementMgr; }
        //AchievementMgr<Guild> const& GetAchievementMgr() const { return m_achievementMgr; }

        // Guild leveling
        uint8 GetLevel() const { return _level; }
        void GiveXP(uint32 xp, Player* source);
        uint64 GetExperience() const { return _experience; }
        uint64 GetTodayExperience() const { return _todayExperience; }

        void AddGuildNews(uint8 type, uint64 guid, uint32 flags, uint32 value);

        EmblemInfo const& GetEmblemInfo() const { return m_emblemInfo; }
        void ResetTimes(bool weekly);

        bool HasAchieved(uint32 achievementId) const;
        //void UpdateAchievementCriteria(AchievementCriteriaTypes type, uint64 miscValue1, uint64 miscValue2, uint64 miscValue3, Unit* unit, Player* player);

    protected:

        uint32 m_id;
        std::string m_name;
        uint64 m_leaderGuid;
        std::string m_motd;
        std::string m_info;
        time_t m_createdDate;

        EmblemInfo m_emblemInfo;
        uint32 m_accountsNumber;
        uint64 m_bankMoney;

        Ranks m_ranks;
        Members m_members;
        BankTabs m_bankTabs;

        // These are actually ordered lists. The first element is the oldest entry.
        GuildLogHolder* m_eventLog;
        GuildLogHolder* m_bankEventLog[GUILD_BANK_MAX_TABS + 1];
        GuildLogHolder* m_newsLog;
        //AchievementMgr<Guild> m_achievementMgr;

        uint8 _level;
        uint64 _experience;
        uint64 _todayExperience;

    private:

        inline uint8 _GetRanksSize() const { return uint8(m_ranks.size()); }
        inline const GuildRankInfo* GetRankInfo(uint8 rankId) const { return rankId < _GetRanksSize() ? &m_ranks[rankId] : nullptr; }
        inline GuildRankInfo* GetRankInfo(uint8 rankId) { return rankId < _GetRanksSize() ? &m_ranks[rankId] : nullptr; }
        inline bool _HasRankRight(uint64 player_guid, uint32 right) const
        {
            if (player_guid)
                if (GuildMember const* member = GetMember(player_guid))
                    return (_GetRankRights(member->GetRankId()) & right) != GR_RIGHT_EMPTY;
            return false;
        }

        inline uint8 _GetLowestRankId() const { return uint8(m_ranks.size() - 1); }

        inline uint8 _GetPurchasedTabsSize() const { return uint8(m_bankTabs.size()); }
        inline GuildBankTab* GetBankTab(uint8 tabId) { return tabId < m_bankTabs.size() ? m_bankTabs[tabId] : nullptr; }
        inline const GuildBankTab* GetBankTab(uint8 tabId) const { return tabId < m_bankTabs.size() ? m_bankTabs[tabId] : nullptr; }

        inline const GuildMember* GetMember(uint64 guid) const
        {
            Members::const_iterator itr = m_members.find(Arcemu::Util::GUID_LOPART(guid));
            return itr != m_members.end() ? itr->second : nullptr;
        }

        inline GuildMember* GetMember(uint64 guid)
        {
            Members::iterator itr = m_members.find(Arcemu::Util::GUID_LOPART(guid));
            return itr != m_members.end() ? itr->second : nullptr;
        }

        inline GuildMember* GetMember(std::string const& name)
        {
            for (Members::iterator itr = m_members.begin(); itr != m_members.end(); ++itr)
                if (itr->second->GetName() == name)
                    return itr->second;

            return nullptr;
        }

        void _CreateLogHolders();
        void _CreateNewBankTab();
        void _CreateDefaultGuildRanks(/*LocaleConstant loc*/);
        bool _CreateRank(std::string const& name, uint32 rights);

        void _UpdateAccountsNumber();
        bool _IsLeader(Player* player) const;
        void _DeleteBankItems(bool removeItemsFromDB = false);
        bool _ModifyBankMoney(uint64 amount, bool add);
        void _SetLeaderGUID(GuildMember* pLeader);

        void _SetRankBankMoneyPerDay(uint8 rankId, uint32 moneyPerDay);
        void _SetRankBankTabRightsAndSlots(uint8 rankId, GuildBankRightsAndSlots rightsAndSlots, bool saveToDB = true);
        int8 _GetRankBankTabRights(uint8 rankId, uint8 tabId) const;
        uint32 _GetRankRights(uint8 rankId) const;
        int32 _GetRankBankMoneyPerDay(uint8 rankId) const;
        int32 _GetRankBankTabSlotsPerDay(uint8 rankId, uint8 tabId) const;
        std::string _GetRankName(uint8 rankId) const;

        int32 _GetMemberRemainingSlots(GuildMember const* member, uint8 tabId) const;
        int32 _GetMemberRemainingMoney(GuildMember const* member) const;
        void _UpdateMemberWithdrawSlots(uint64 guid, uint8 tabId);
        bool _MemberHasTabRights(uint64 guid, uint8 tabId, uint32 rights) const;

        void _LogEvent(GuildEventLogTypes eventType, uint32 playerGuid1, uint32 playerGuid2 = 0, uint8 newRank = 0);
        void _LogBankEvent(GuildBankEventLogTypes eventType, uint8 tabId, uint32 playerGuid, uint32 itemOrMoney, uint16 itemStackCount = 0, uint8 destTabId = 0);

        void SendGuildReputationWeeklyCap(WorldSession* session, uint32 reputation) const;
        void SendGuildRanksUpdate(uint64 setterGuid, uint64 targetGuid, uint32 rank);

        void _BroadcastEvent(GuildEvents guildEvent, uint64 guid, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL) const;
};


#endif // _GUILD_HPP
