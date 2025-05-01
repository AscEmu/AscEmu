/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "CommonTypes.hpp"

#include <cstdint>
#include <ctime>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>

#if VERSION_STRING > TBC

class WorldSession;
class Object;
class QueryResult;

namespace WDB::Structures
{
    struct AchievementEntry;
    struct AchievementCriteriaEntry;
}

class QueryBuffer;
struct AchievementEntry;
struct AchievementCriteriaEntry;

enum AchievementRewardTypes
{
    ACHIEVEMENT_REWARDTYPE_NONE     = 0,
    ACHIEVEMENT_REWARDTYPE_ITEM     = 1,
    ACHIEVEMENT_REWARDTYPE_TITLE    = 2,
    ACHIEVEMENT_REWARDTYPE_SPELL    = 4
};

enum AchievementFlags
{
    ACHIEVEMENT_FLAG_COUNTER                = 0x00000001,    // Just count statistic (never stop and complete)
    ACHIEVEMENT_FLAG_HIDDEN                 = 0x00000002,    // Not sent to client - internal use only
    ACHIEVEMENT_FLAG_PLAY_NO_VISUAL         = 0x00000004,    // Client does not play achievement earned visual
    ACHIEVEMENT_FLAG_SUMM                   = 0x00000008,    // Use summ criteria value from all requirements (and calculate max value)
    ACHIEVEMENT_FLAG_MAX_USED               = 0x00000010,    // Show max criteria (and calculate max value ??)
    ACHIEVEMENT_FLAG_REQ_COUNT              = 0x00000020,    // Use not zero req count (and calculate max value)
    ACHIEVEMENT_FLAG_AVERAGE                = 0x00000040,    // Show as average value (value / time_in_days) depend from other flag (by def use last criteria value)
    ACHIEVEMENT_FLAG_BAR                    = 0x00000080,    // Show as progress bar (value / max vale) depend from other flag (by def use last criteria value)
    ACHIEVEMENT_FLAG_REALM_FIRST_REACH      = 0x00000100,    //
    ACHIEVEMENT_FLAG_REALM_FIRST_KILL       = 0x00000200,    //
    ACHIEVEMENT_FLAG_UNK3                   = 0x00000400,    // ACHIEVEMENT_FLAG_HIDE_NAME_IN_TIE
    ACHIEVEMENT_FLAG_REALM_FIRST_GUILD      = 0x00000800,    // first guild on realm done something
    ACHIEVEMENT_FLAG_SHOW_IN_GUILD_NEWS     = 0x00001000,    // Shows in guild news
    ACHIEVEMENT_FLAG_SHOW_IN_GUILD_HEADER   = 0x00002000,    // Shows in guild news header
    ACHIEVEMENT_FLAG_GUILD                  = 0x00004000,    //
    ACHIEVEMENT_FLAG_SHOW_GUILD_MEMBERS     = 0x00008000,    //
    ACHIEVEMENT_FLAG_SHOW_CRITERIA_MEMBERS  = 0x00010000     //
};

inline uint32_t secsToTimeBitFields(time_t secs)
{
    tm* lt = localtime(&secs);
    return static_cast<uint32_t>((lt->tm_year - 100) << 24 | lt->tm_mon << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min);
}

struct CriteriaProgress
{
    CriteriaProgress(uint32_t iid, uint32_t icounter, time_t tdate = time(nullptr))
        :
        id(iid),
        counter(icounter),
        date(tdate)
    { }

    uint32_t id;     ///< Criteria ID
    uint32_t counter; ///< Completed count: how many times the criteria has been completed
    time_t date;   ///< Date/time
};

struct AchievementReward
{
    uint8_t gender;
    uint32_t titel_A;
    uint32_t titel_H;
    uint32_t itemId;
    uint32_t sender;
    std::string subject;
    std::string text;
};

typedef std::unordered_map<uint32_t, std::unique_ptr<CriteriaProgress>> CriteriaProgressMap;
typedef std::unordered_map<uint32_t, time_t> CompletedAchievementMap;
typedef std::multimap<uint32_t, AchievementReward> AchievementRewardsMap;
typedef std::pair<AchievementRewardsMap::const_iterator, AchievementRewardsMap::const_iterator> AchievementRewardsMapBounds;
typedef std::set<uint32_t> AchievementSet;
typedef std::list<WDB::Structures::AchievementCriteriaEntry const*> AchievementCriteriaEntryList;

class Player;
class WorldPacket;
class ObjectMgr;

enum AchievementCompletionState
{
    ACHIEVEMENT_COMPLETED_NONE,                 ///< #0# Achievement is not completed
    ACHIEVEMENT_COMPLETED_COMPLETED_NOT_STORED, ///< #1# Achievement is completed, but not stored yet
    ACHIEVEMENT_COMPLETED_COMPLETED_STORED,     ///< #2# Achievement is completed and has been stored
};

/// \note Currently these are not being used at all.
enum AchievementCriteriaCondition
{
    ACHIEVEMENT_CRITERIA_CONDITION_NONE      = 0,  ///< #0# No condition
    ACHIEVEMENT_CRITERIA_CONDITION_NO_DEATH  = 1,  ///< #1# Must not die?
    ACHIEVEMENT_CRITERIA_CONDITION_UNK1      = 2,  ///< #2# only used in "Complete a daily quest every day for five consecutive days"
    ACHIEVEMENT_CRITERIA_CONDITION_MAP       = 3,  ///< #3# requires you to be on specific map
    ACHIEVEMENT_CRITERIA_CONDITION_NO_LOOSE  = 4,  ///< #4# only used in "Win 10 arenas without losing"
    ACHIEVEMENT_CRITERIA_CONDITION_UNK2      = 9,  ///< #9# unk
    ACHIEVEMENT_CRITERIA_CONDITION_UNK3      = 13, ///< #13# unk
};

enum AchievementCriteriaTypes : uint8_t
{
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE                     = 0,    ///< #0# Kill creature x
    ACHIEVEMENT_CRITERIA_TYPE_WIN_BG                            = 1,    ///< #1# Win battleground
    ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL                       = 5,    ///< #5# Reach level x
    ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL                 = 7,    ///< #7#  Reach skill level x
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT              = 8,    ///< #8#  Complete an achievement
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT              = 9,    ///< #9#  Complete x quests
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST_DAILY        = 10,   ///< #10# Complete daily quest x days in a row
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE           = 11,   ///< #11# Complete quests in zone x
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_DAILY_QUEST              = 14,   ///< #14# Complete daily quest
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_BATTLEGROUND             = 15,   ///< #15# Complete battleground
    ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP                      = 16,   ///< #16# Death at map
    ACHIEVEMENT_CRITERIA_TYPE_DEATH                             = 17,   ///< #17# Death
    ACHIEVEMENT_CRITERIA_TYPE_DEATH_IN_DUNGEON                  = 18,   ///< #18# Death in dungeon
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_RAID                     = 19,   ///< #19# Complete raid
    ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE                = 20,   ///< #20# Killed by creature
    ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER                  = 23,   ///< #23# Killed by player
    ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING                = 24,   ///< #24# Fall without dying
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST                    = 27,   ///< #27# Complete quest
    ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET                   = 28,   ///< #28# Have spell x cast on you
    ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL                        = 29,   ///< #29# Cast spell x
    ACHIEVEMENT_CRITERIA_TYPE_BG_OBJECTIVE_CAPTURE              = 30,   ///< #30# Capture a battleground objective
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA            = 31,   ///< #31# Get an honorable kill at area
    ACHIEVEMENT_CRITERIA_TYPE_WIN_ARENA                         = 32,   ///< #32# Win arena
    ACHIEVEMENT_CRITERIA_TYPE_PLAY_ARENA                        = 33,   ///< #33# Play arena
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL                       = 34,   ///< #34# Learn spell x
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL                    = 35,   ///< #35# Honorable kill
    ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM                          = 36,   ///< #36# Own item x
    ///\todo the achievements 1162 and 1163 requires a special raing which can't be found in the dbc
    ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_ARENA                   = 37,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_TEAM_RATING               = 38,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_TEAM_RATING                 = 39,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL                 = 40,
    ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM                          = 41,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM                         = 42,
    ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA                      = 43,
    ACHIEVEMENT_CRITERIA_TYPE_OWN_RANK                          = 44,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT                     = 45,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION                   = 46,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION           = 47,
    ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP                 = 48,
    ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM                   = 49,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT                 = 50,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT                = 51,
    ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS                          = 52,
    ACHIEVEMENT_CRITERIA_TYPE_HK_RACE                           = 53,
    ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE                          = 54,
    ACHIEVEMENT_CRITERIA_TYPE_HEALING_DONE                      = 55,
    ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW                      = 56,
    ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM                        = 57,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TALENTS            = 60,
    ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_TALENT_RESETS           = 61,
    ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD                 = 62,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING         = 63,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER              = 65,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_MAIL               = 66,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY                        = 67,
    ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT                    = 68,
    ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2                  = 69,
    ACHIEVEMENT_CRITERIA_TYPE_SPECIAL_PVP_KILL                  = 70,
    ACHIEVEMENT_CRITERIA_TYPE_FISH_IN_GAMEOBJECT                = 72,
    ///\todo title id is not mentioned in dbc
    ACHIEVEMENT_CRITERIA_TYPE_EARNED_PVP_TITLE                  = 74,
    ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS                  = 75,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_DUEL                          = 76,
    ACHIEVEMENT_CRITERIA_TYPE_LOSE_DUEL                         = 77,
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE                = 78,
    ACHIEVEMENT_CRITERIA_TYPE_GOLD_EARNED_BY_AUCTIONS           = 80,
    ACHIEVEMENT_CRITERIA_TYPE_CREATE_AUCTION                    = 82,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_BID               = 83,
    ACHIEVEMENT_CRITERIA_TYPE_WON_AUCTIONS                      = 84,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_AUCTION_SOLD              = 85,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_GOLD_VALUE_OWNED          = 86,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_REVERED_REPUTATION           = 87,
    ACHIEVEMENT_CRITERIA_TYPE_GAIN_HONORED_REPUTATION           = 88,
    ACHIEVEMENT_CRITERIA_TYPE_KNOWN_FACTIONS                    = 89,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_EPIC_ITEM                    = 90,
    ACHIEVEMENT_CRITERIA_TYPE_RECEIVE_EPIC_ITEM                 = 91,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED                         = 93,
    ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED                        = 94,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALTH                    = 95,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_POWER                     = 96,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_STAT                      = 97,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_SPELLPOWER                = 98,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_ARMOR                     = 99,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_RATING                    = 100,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_DEALT                 = 101,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HIT_RECEIVED              = 102,
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL_DAMAGE_RECEIVED             = 103,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEAL_CASTED               = 104,
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL_HEALING_RECEIVED            = 105,
    ACHIEVEMENT_CRITERIA_TYPE_HIGHEST_HEALING_RECEIVED          = 106,
    ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED                   = 107,
    ACHIEVEMENT_CRITERIA_TYPE_FLIGHT_PATHS_TAKEN                = 108,
    ACHIEVEMENT_CRITERIA_TYPE_LOOT_TYPE                         = 109,
    ///\todo target entry is missing
    ACHIEVEMENT_CRITERIA_TYPE_CAST_SPELL2                       = 110,
    ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LINE                  = 112,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL               = 113,
    ACHIEVEMENT_CRITERIA_TYPE_ACCEPTED_SUMMONINGS               = 114,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_ACHIEVEMENT_POINTS           = 115,
    ACHIEVEMENT_CRITERIA_TYPE_USE_LFD_TO_GROUP_WITH_PLAYERS     = 119,
#if VERSION_STRING == WotLK
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL                             = 124,
#endif
#if VERSION_STRING > WotLK
    ACHIEVEMENT_CRITERIA_TYPE_SPENT_GOLD_GUILD_REPAIRS          = 124,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_GUILD_LEVEL                 = 125,
    ACHIEVEMENT_CRITERIA_TYPE_CRAFT_ITEMS_GUILD                 = 126,
    ACHIEVEMENT_CRITERIA_TYPE_CATCH_FROM_POOL                   = 127,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_BANK_SLOTS              = 128,
    ACHIEVEMENT_CRITERIA_TYPE_EARN_GUILD_ACHIEVEMENT_POINTS     = 129,
    ACHIEVEMENT_CRITERIA_TYPE_WIN_RATED_BATTLEGROUND            = 130,
    ACHIEVEMENT_CRITERIA_TYPE_REACH_BG_RATING                   = 132,
    ACHIEVEMENT_CRITERIA_TYPE_BUY_GUILD_TABARD                  = 133,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_GUILD             = 134,
    ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILLS_GUILD             = 135,
    ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE_GUILD          = 136,
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE_TYPE     = 138,  //struct { Flag flag; uint32_t count; } 1: Guild Dungeon, 2:Guild Challenge, 3:Guild battlefield
    ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_GUILD_CHALLENGE          = 139,  //struct { uint32_t count; } Guild Challenge
    ACHIEVEMENT_CRITERIA_TYPE_TOTAL                             = 140,
#endif
};

/**
AchievementMgr class
Achievement Working List:
- ACHIEVEMENT_CRITERIA_TYPE_REACH_LEVEL
- ACHIEVEMENT_CRITERIA_TYPE_LOOT_ITEM
- ACHIEVEMENT_CRITERIA_TYPE_OWN_ITEM
- ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA
- ACHIEVEMENT_CRITERIA_TYPE_LOOT_MONEY
- ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST_COUNT
- ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUESTS_IN_ZONE
- ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_QUEST
- ACHIEVEMENT_CRITERIA_TYPE_QUEST_REWARD_GOLD
- ACHIEVEMENT_CRITERIA_TYPE_GAIN_REPUTATION
- ACHIEVEMENT_CRITERIA_TYPE_GAIN_EXALTED_REPUTATION
- ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT
- ACHIEVEMENT_CRITERIA_TYPE_LEARN_SPELL
- ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE
- ACHIEVEMENT_CRITERIA_TYPE_REACH_SKILL_LEVEL
- ACHIEVEMENT_CRITERIA_TYPE_LEARN_SKILL_LEVEL
- ACHIEVEMENT_CRITERIA_TYPE_EQUIP_ITEM
- ACHIEVEMENT_CRITERIA_TYPE_EQUIP_EPIC_ITEM
- ACHIEVEMENT_CRITERIA_TYPE_NUMBER_OF_MOUNTS
- ACHIEVEMENT_CRITERIA_TYPE_DO_EMOTE (partial)
- ACHIEVEMENT_CRITERIA_TYPE_KILLING_BLOW (some)
- ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE (some)
- ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE_TYPE
- ACHIEVEMENT_CRITERIA_TYPE_USE_ITEM
- ACHIEVEMENT_CRITERIA_TYPE_USE_GAMEOBJECT
- ACHIEVEMENT_CRITERIA_TYPE_BUY_BANK_SLOT
- ACHIEVEMENT_CRITERIA_TYPE_VISIT_BARBER_SHOP
- ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_AT_BARBER
- ACHIEVEMENT_CRITERIA_TYPE_FALL_WITHOUT_DYING
- ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL
- ACHIEVEMENT_CRITERIA_TYPE_HONORABLE_KILL_AT_AREA
- ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_PLAYER
- ACHIEVEMENT_CRITERIA_TYPE_KILLED_BY_CREATURE
- ACHIEVEMENT_CRITERIA_TYPE_HK_CLASS
- ACHIEVEMENT_CRITERIA_TYPE_HK_RACE
- ACHIEVEMENT_CRITERIA_TYPE_DEATH
- ACHIEVEMENT_CRITERIA_TYPE_DEATH_AT_MAP
- ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET
- ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2
- Realm-First achievements (most)
- Reward Titles
- Reward Spells
- Reward Items (goes to inventory?)
\todo Several achievement types
\todo Time limits on achievements
\todo Special conditions for achievements (group size, nobody in raid die during fight, etc.)
*/
class SERVER_DECL AchievementMgr
{
    friend Player;

    AchievementMgr(Player* _player);

public:
    // Need public for unique_ptr -Appled
    ~AchievementMgr();

    void loadFromDb(QueryResult* _achievementResult, QueryResult* _criteriaResult);
    void saveToDb(QueryBuffer* _buffer);

    bool canCompleteCriteria(WDB::Structures::AchievementCriteriaEntry const* _achievementCriteria, AchievementCriteriaTypes _type, Player* _player) const;
    bool canCompleteCriteria(WDB::Structures::AchievementCriteriaEntry const* _achievementCriteria, AchievementCriteriaTypes _type, int32_t _miscValue1, int32_t _miscValue2, Player* _player) const;

    void updateAllAchievementCriteria();

    void updateAchievementCriteria(AchievementCriteriaTypes _type, int32_t _miscvalue1, int32_t _miscvalue2, uint32_t _time, Object* _reference = nullptr);
    void updateAchievementCriteria(AchievementCriteriaTypes _type);
    bool updateAchievementCriteria(Player* _player, int32_t _criteriaId, uint32_t _count);

    uint32_t getCriteriaProgressCount();
    bool isGroupCriteriaType(AchievementCriteriaTypes _type) const;

    bool gmCompleteCriteria(WorldSession* _gmSession, uint32_t _criteriaId, bool _finishAll = false);
    void gmResetCriteria(uint32_t _criteriaId, bool _finishAll = false);

    void sendAllAchievementData(Player* _player);
#if VERSION_STRING >= Cata
    void sendRespondInspectAchievements(Player* _player);
#endif

    bool gmCompleteAchievement(WorldSession* _gmSession, uint32_t _achievementId, bool _finishAll = false);
    void gmResetAchievement(uint32_t _achievementId, bool _finishAll = false);

    time_t getCompletedTime(WDB::Structures::AchievementEntry const* _achievement);
    uint32_t getCompletedAchievementsCount() const;
    bool hasCompleted(uint32_t _achievementId) const;

    Player* getPlayer() const;

private:
    void completedAchievement(WDB::Structures::AchievementEntry const* _entry);
    bool showCompletedAchievement(uint32_t _achievementId, const Player* _player);

    void giveAchievementReward(WDB::Structures::AchievementEntry const* _entry);
    void sendAchievementEarned(WDB::Structures::AchievementEntry const* _entry);

    AchievementCompletionState getAchievementCompletionState(WDB::Structures::AchievementEntry const* _entry);

    bool canSendAchievementProgress(const CriteriaProgress* _criteriaProgress);
    bool canSaveAchievementProgressToDB(const CriteriaProgress* _criteriaProgress);
    void sendCriteriaUpdate(const CriteriaProgress* _criteriaProgress);
    void setCriteriaProgress(WDB::Structures::AchievementCriteriaEntry const* _entry, int32_t _newValue, bool _relative = false);
    void updateCriteriaProgress(WDB::Structures::AchievementCriteriaEntry const* _entry, int32_t _updateByValue);

    void completedCriteria(WDB::Structures::AchievementCriteriaEntry const* _entry);
    bool isCompletedCriteria(WDB::Structures::AchievementCriteriaEntry const* _entry);

    std::mutex m_lock;
    Player* m_player;
    CriteriaProgressMap m_criteriaProgress;
    CompletedAchievementMap m_completedAchievements;
    bool isCharacterLoading;
};
#endif
