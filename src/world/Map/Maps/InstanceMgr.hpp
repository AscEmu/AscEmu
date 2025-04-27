/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "InstanceDefines.hpp"
#include "CommonTypes.hpp"

#include <cstdint>
#include <ctime>
#include <list>
#include <map>
#include <mutex>
#include <unordered_map>

class Group;
class Player;
class InstanceSaved;

struct InstancePlayerBind
{
    InstanceSaved* save;
    bool perm;
    BindExtensionState extendState;

    InstancePlayerBind() : save(nullptr), perm(false), extendState(EXTEND_STATE_NORMAL) { }
};

class SERVER_DECL InstanceSaved
{
    friend class InstanceMgr;

public:
    InstanceSaved(uint32_t mapId, uint32_t instanceId, InstanceDifficulty::Difficulties difficulty, time_t resetTime, bool canReset);
    ~InstanceSaved();

    uint8_t getPlayerCount() const { return static_cast<uint8_t>(m_playerList.size()); }
    uint8_t getGroupCount() const { return static_cast<uint8_t>(m_groupList.size()); }

    // Map Related
    uint32_t getInstanceId() const { return m_instanceid; }
    uint32_t getMapId() const { return m_mapid; }

    void addPlayer(Player* player);
    bool removePlayer(Player* player);
    void addGroup(Group* group);
    bool removeGroup(Group* group);

    // Database
    void saveToDB();
    void deleteFromDB();

    // Time
    time_t getResetTime() const { return m_resetTime; }
    void setResetTime(time_t resetTime) { m_resetTime = resetTime; }
    time_t getResetTimeForDB();

    // Reset
    bool canReset() const { return m_canReset; }
    void setCanReset(bool canReset) { m_canReset = canReset; }

    InstanceDifficulty::Difficulties getDifficulty() const { return m_difficulty; }

    typedef std::list<Player*> PlayerList;
    typedef std::list<Group*> GroupList;

private:
    bool unloadIfEmpty();
  
    PlayerList m_playerList;
    GroupList m_groupList;
    time_t m_resetTime;
    uint32_t m_instanceid;
    uint32_t m_mapid;
    InstanceDifficulty::Difficulties m_difficulty;
    bool m_canReset;

    std::mutex _playerListLock;
};

struct DifficultyMap
{
    union
    {
        struct
        {
            uint16_t mapid;
            uint16_t difficulty;
        } MapPairParts;
        uint32_t MapPair;
    };
};

class SERVER_DECL InstanceMgr
{
    friend class InstanceSaved;

private:
    InstanceMgr() = default;
    ~InstanceMgr() = default;

public:
    InstanceMgr(InstanceMgr&&) = delete;
    InstanceMgr(InstanceMgr const&) = delete;
    InstanceMgr& operator=(InstanceMgr&&) = delete;
    InstanceMgr& operator=(InstanceMgr const&) = delete;

    static InstanceMgr& getInstance();

    struct InstResetEvent
    {
        uint8_t type;
        InstanceDifficulty::Difficulties difficulty : 8;
        uint16_t mapid;
        uint16_t instanceId;

        InstResetEvent() : type(0), difficulty(InstanceDifficulty::Difficulties::DUNGEON_NORMAL), mapid(0), instanceId(0) { }
        InstResetEvent(uint8_t t, uint32_t _mapid, InstanceDifficulty::Difficulties d, uint16_t _instanceid)
            : type(t), difficulty(d), mapid(static_cast<uint16_t>(_mapid)), instanceId(_instanceid) { }
        bool operator==(InstResetEvent const& e) const { return e.instanceId == instanceId; }
    };
    typedef std::multimap<time_t, InstResetEvent> ResetTimeQueue;
    typedef std::unordered_map<uint32_t, std::unique_ptr<InstanceSaved>> InstanceSavedMap;
    typedef std::unordered_map<uint32_t, time_t> ResetTimeByMapDifficultyMap;

    // Loading
    void loadInstances();
    void loadResetTimes();

    // Updates
    void update();

    // Reset Times
    time_t getResetTimeFor(uint16_t mapid, InstanceDifficulty::Difficulties d) const
    {
        DifficultyMap map = DifficultyMap();
        
        map.MapPairParts.mapid = mapid;
        map.MapPairParts.difficulty = d;

        ResetTimeByMapDifficultyMap::const_iterator itr = m_resetTimeByMapDifficulty.find(map.MapPair);
        return itr != m_resetTimeByMapDifficulty.end() ? itr->second : 0;
    }
    time_t getSubsequentResetTime(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, time_t resetTime) const;

    // Use this on startup when initializing reset times
    void initializeResetTimeFor(uint16_t mapid, InstanceDifficulty::Difficulties d, time_t t);

    // Use this only when updating existing reset times
    void setResetTimeFor(uint16_t mapid, InstanceDifficulty::Difficulties d, time_t t)
    {
        DifficultyMap map = DifficultyMap();

        map.MapPairParts.mapid = mapid;
        map.MapPairParts.difficulty = d;

        ResetTimeByMapDifficultyMap::iterator itr = m_resetTimeByMapDifficulty.find(map.MapPair);
        if (itr != m_resetTimeByMapDifficulty.end())
            itr->second = t;
    }

    ResetTimeByMapDifficultyMap const& GetResetTimeMap() const
    {
        return m_resetTimeByMapDifficulty;
    }

    // Events
    void addResetEvent(bool add, time_t time, InstResetEvent event);

    InstanceSaved* addInstanceSave(uint32_t mapId, uint32_t instanceId, InstanceDifficulty::Difficulties difficulty, time_t resetTime, bool canReset, bool load = false);
    void removeInstanceSave(uint32_t InstanceId);
    void unloadInstanceSave(uint32_t InstanceId);
    static void deleteInstanceFromDB(uint32_t instanceid);

    InstanceSaved* getInstanceSave(uint32_t InstanceId);

protected:
    static uint16_t ResetTimeDelay[];

private:
    void resetOrWarnAll(uint32_t mapid, InstanceDifficulty::Difficulties difficulty, bool warn, time_t resetTime);
    void resetInstance(uint32_t mapid, uint32_t instanceId);
    void resetSave(InstanceSavedMap::iterator& itr);
    bool lock_instanceLists = false;
    InstanceSavedMap m_instanceSaveById;
    ResetTimeByMapDifficultyMap m_resetTimeByMapDifficulty;
    ResetTimeQueue m_resetTimeQueue;
};

#define sInstanceMgr InstanceMgr::getInstance()
