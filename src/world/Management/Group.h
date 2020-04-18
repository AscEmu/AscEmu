/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../world/WorldConf.h"
#include "Units/Players/Player.h"
#include "Server/Packets/CmsgMessageChat.h"

enum PartyErrors
{
    ERR_PARTY_NO_ERROR              = 0,
    ERR_PARTY_CANNOT_FIND           = 1,
    ERR_PARTY_IS_NOT_IN_YOUR_PARTY  = 2,
    ERR_PARTY_UNK                   = 3,
    ERR_PARTY_IS_FULL               = 4,
    ERR_PARTY_ALREADY_IN_GROUP      = 5,
    ERR_PARTY_YOU_ARENT_IN_A_PARTY  = 6,
    ERR_PARTY_YOU_ARE_NOT_LEADER    = 7,
    ERR_PARTY_WRONG_FACTION         = 8,
    ERR_PARTY_IS_IGNORING_YOU       = 9
};

enum GroupTypes
{
    GROUP_TYPE_PARTY    = 0x0,
    GROUP_TYPE_BG       = 0x1,
    GROUP_TYPE_RAID     = 0x2,
    GROUP_TYPE_BGRAID   = GROUP_TYPE_BG | GROUP_TYPE_RAID,
    GROUP_TYPE_UNK1     = 0x4,
    GROUP_TYPE_LFD      = 0x8
};

enum MaxGroupCount
{
    MAX_GROUP_SIZE_PARTY    = 5,
    MAX_GROUP_SIZE_RAID     = 40
};

enum QuickGroupUpdateFlags
{
    PARTY_UPDATE_FLAG_POSITION  = 1,
    PARTY_UPDATE_FLAG_ZONEID    = 2
};

enum PartyUpdateFlags
{
    GROUP_UPDATE_FLAG_NONE              = 0x00000000,       // nothing
    GROUP_UPDATE_FLAG_STATUS            = 0x00000001,       // uint16_t, flags
    GROUP_UPDATE_FLAG_CUR_HP            = 0x00000002,       // uint32_t
    GROUP_UPDATE_FLAG_MAX_HP            = 0x00000004,       // uint32_t
    GROUP_UPDATE_FLAG_POWER_TYPE        = 0x00000008,       // uint8_t
    GROUP_UPDATE_FLAG_CUR_POWER         = 0x00000010,       // uint16_t
    GROUP_UPDATE_FLAG_MAX_POWER         = 0x00000020,       // uint16_t
    GROUP_UPDATE_FLAG_LEVEL             = 0x00000040,       // uint16_t
    GROUP_UPDATE_FLAG_ZONE              = 0x00000080,       // uint16_t
    GROUP_UPDATE_FLAG_POSITION          = 0x00000100,       // uint16_t, uint16_t
    GROUP_UPDATE_FLAG_AURAS             = 0x00000200,       // uint64_t mask, for each bit set uint32_t spellid + uint8_t unk
    GROUP_UPDATE_FLAG_PET_GUID          = 0x00000400,       // uint64_t pet guid
    GROUP_UPDATE_FLAG_PET_NAME          = 0x00000800,       // pet name, NULL terminated string
    GROUP_UPDATE_FLAG_PET_MODEL_ID      = 0x00001000,       // uint16_t, model id
    GROUP_UPDATE_FLAG_PET_CUR_HP        = 0x00002000,       // uint32_t pet cur health
    GROUP_UPDATE_FLAG_PET_MAX_HP        = 0x00004000,       // uint32_t pet max health
    GROUP_UPDATE_FLAG_PET_POWER_TYPE    = 0x00008000,       // uint8_t pet power type
    GROUP_UPDATE_FLAG_PET_CUR_POWER     = 0x00010000,       // uint16_t pet cur power
    GROUP_UPDATE_FLAG_PET_MAX_POWER     = 0x00020000,       // uint16_t pet max power
    GROUP_UPDATE_FLAG_PET_AURAS         = 0x00040000,       // uint64_t mask, for each bit set uint32_t spellid + uint8_t unk, pet auras...
    GROUP_UPDATE_FLAG_VEHICLE_SEAT      = 0x00080000,       // uint32_t vehicle_seat_id (index from VehicleSeat.dbc)
    GROUP_UPDATE_PET                    = 0x0007FC00,       // all pet flags
    GROUP_UPDATE_FULL                   = 0x0007FFFF        // all known flags
};


#define GROUP_UPDATE_FLAGS_COUNT 20
static const uint8_t GroupUpdateLength[GROUP_UPDATE_FLAGS_COUNT] = { 0, 2, 2, 2, 1, 2, 2, 2, 2, 4, 8, 8, 1, 2, 2, 2, 1, 2, 2, 8 };

enum GroupMemberOnlineStatus
{
    MEMBER_STATUS_OFFLINE   = 0x0000,
    MEMBER_STATUS_ONLINE    = 0x0001,       // Lua_UnitIsConnected
    MEMBER_STATUS_PVP       = 0x0002,       // Lua_UnitIsPVP
    MEMBER_STATUS_DEAD      = 0x0004,       // Lua_UnitIsDead
    MEMBER_STATUS_GHOST     = 0x0008,       // Lua_UnitIsGhost
    MEMBER_STATUS_PVP_FFA   = 0x0010,       // Lua_UnitIsPVPFreeForAll
    MEMBER_STATUS_UNK3      = 0x0020,       // used in calls from Lua_GetPlayerMapPosition/Lua_GetBattlefieldFlagPosition
    MEMBER_STATUS_AFK       = 0x0040,       // Lua_UnitIsAFK
    MEMBER_STATUS_DND       = 0x0080        // Lua_UnitIsDND
};

class PlayerInfo;

typedef struct
{
    PlayerInfo* player_info;
    Player* player;
} GroupMember;

class Group;
class Player;

typedef std::set<PlayerInfo*> GroupMembersSet;

class SERVER_DECL SubGroup // Most stuff will be done through here, not through the "Group" class.
{
public:
    friend class Group;

    SubGroup(Group* parent, uint32_t id) : m_Parent(parent), m_Id(id)  {}
    ~SubGroup();

    inline GroupMembersSet::iterator GetGroupMembersBegin(void) { return m_GroupMembers.begin(); }
    inline GroupMembersSet::iterator GetGroupMembersEnd(void)   { return m_GroupMembers.end(); }

    bool AddPlayer(PlayerInfo* info);
    void RemovePlayer(PlayerInfo* info);

    inline bool IsFull(void)                 { return m_GroupMembers.size() >= MAX_GROUP_SIZE_PARTY; }
    inline size_t GetMemberCount(void)       { return m_GroupMembers.size(); }

    inline uint32_t GetID(void)              { return m_Id; }
    inline void SetID(uint32_t newid)        { m_Id = newid; }

    inline void   SetParent(Group* parent)   { m_Parent = parent; }
    inline Group* GetParent(void)            { return m_Parent; }

    void Disband();
    bool HasMember(uint32_t guid);
    GroupMembersSet& getGroupMembers();

protected:
    GroupMembersSet m_GroupMembers;
    Group* m_Parent;
    uint32_t m_Id;
};

class Arena;
class SERVER_DECL Group
{
public:
    friend class SubGroup;

    static Group* Create();

    Group(bool Assign);
    ~Group();

    // Adding/Removal Management
    bool AddMember(PlayerInfo* info, int32_t subgroupid = -1);
    void RemovePlayer(PlayerInfo* info);

    // Leaders and Looting
    void SetLeader(Player* pPlayer, bool silent);
    void SetLooter(Player* pPlayer, uint8_t method, uint16_t threshold);

    // Transferring data to clients
    void Update();

    inline void SendPacketToAll(WorldPacket* packet) { SendPacketToAllButOne(packet, NULL); }
    void SendPacketToAllButOne(WorldPacket* packet, Player* pSkipTarget);

    inline void OutPacketToAll(uint16_t op, uint16_t len, const void* data) { OutPacketToAllButOne(op, len, data, NULL); }
    void OutPacketToAllButOne(uint16_t op, uint16_t len, const void* data, Player* pSkipTarget);

    void SendNullUpdate(Player* pPlayer);

    // Destroying/Converting
    void Disband();
    Player* FindFirstPlayer();

    // Accessing functions
    inline SubGroup* GetSubGroup(uint32_t Id)
    {
        if (Id >= 8)
            return 0;

        return m_SubGroups[Id];
    }

    inline uint32_t GetSubGroupCount(void) { return m_SubGroupCount; }

    inline uint8_t GetMethod(void) { return m_LootMethod; }
    inline uint16_t GetThreshold(void) { return m_LootThreshold; }
    inline PlayerInfo* GetLeader(void) { return m_Leader; }
    inline PlayerInfo* GetLooter(void) { return m_Looter; }

    void MovePlayer(PlayerInfo* info, uint8_t subgroup);

    bool HasMember(Player* pPlayer);
    bool HasMember(PlayerInfo* info);
    inline uint32_t MemberCount(void) { return m_MemberCount; }
    inline bool IsFull() { return ((m_GroupType == GROUP_TYPE_PARTY && m_MemberCount >= MAX_GROUP_SIZE_PARTY) || (m_GroupType == GROUP_TYPE_RAID && m_MemberCount >= MAX_GROUP_SIZE_RAID)); }

    SubGroup* FindFreeSubGroup();

    void ExpandToRaid();

    void SaveToDB();
    void LoadFromDB(Field* fields);

    inline uint8_t getGroupType() const { return m_GroupType; }
    inline uint32_t GetID() { return m_Id; }
    uint64_t GetGUID() const;

    void UpdateOutOfRangePlayer(Player* pPlayer, bool Distribute, WorldPacket* Packet);
    void UpdateAllOutOfRangePlayersFor(Player* pPlayer);
    bool isRaid() const;

    uint64_t m_targetIcons[8];
    bool m_disbandOnNoMembers;
    inline Mutex & getLock() { return m_groupLock; }
    inline void Lock() { m_groupLock.Acquire(); }
    inline void Unlock() { return m_groupLock.Release(); }
    bool m_isqueued;

    void SetAssistantLeader(PlayerInfo* pMember);
    void SetMainTank(PlayerInfo* pMember);
    void SetMainAssist(PlayerInfo* pMember);

    inline PlayerInfo* GetAssistantLeader() { return m_assistantLeader; }
    inline PlayerInfo* GetMainTank() { return m_mainTank; }
    inline PlayerInfo* GetMainAssist() { return m_mainAssist; }

    uint32_t m_instanceIds[MAX_NUM_MAPS][NUM_INSTANCE_MODES];

    void SetDungeonDifficulty(uint8_t diff);
    void SetRaidDifficulty(uint8_t diff);
    void SendLootUpdates(Object* o);

    //////////////////////////////////////////////////////////////////////////////////////////
    /// Player* GetRandomPlayerInRangeButSkip(Player* plr, float range, Player* plr_skip)
    /// Return a random player in player's group that's in his range, skipping a desired player from result
    ///
    /// \param Player* plr - desired player
    /// \param float range - max. range
    /// \param Player* plr_skip - player to skip from result
    ///
    /// \returns NULL if there is not a member in range, returns reference to a Player otherwise.
    ///
    //////////////////////////////////////////////////////////////////////////////////////////
    Player* GetRandomPlayerInRangeButSkip(Player* plr, float range, Player* plr_skip);

#if VERSION_STRING > TBC
    void UpdateAchievementCriteriaForInrange(Object* o, AchievementCriteriaTypes type, int32_t miscvalue1, int32_t miscvalue2, uint32_t time);
#endif
    void Teleport(WorldSession* m_session);
    bool isLFGGroup()
    {
        if(m_GroupType & GROUP_TYPE_LFD)
            return true;
        return false;
    }
    void ExpandToLFG();
    uint32_t GetMembersCount() { return m_MemberCount; }

    uint64_t GetGUID() { return uint64_t(GetID()); }
    SubGroup* m_SubGroups[8];
    uint8_t m_SubGroupCount;
    void GoOffline(Player* p);

protected:
    PlayerInfo* m_Leader;
    PlayerInfo* m_Looter;
    PlayerInfo* m_assistantLeader;
    PlayerInfo* m_mainTank;
    PlayerInfo* m_mainAssist;

    uint8_t m_LootMethod;
    uint16_t m_LootThreshold;
    uint8_t m_GroupType;
    uint32_t m_Id;
    uint64_t m_guid;

    uint32_t m_MemberCount;
    Mutex m_groupLock;
    bool m_dirty;
    bool m_updateblock;
    uint32_t updatecounter;

public:
    uint8_t m_difficulty;
    uint8_t m_raiddifficulty;
};
