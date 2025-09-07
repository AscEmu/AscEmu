/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlegroundDefines.hpp"
#include "Server/EventableObject.h"
#include <deque>
#include <vector>
#include <map>
#include <mutex>

class WorldSession;
class WorldPacket;
class Battleground;
class BattlegroundMap;
class Player;
class Group;
class Corpse;
class Arena;
class WoWGuid;

typedef Battleground* (*BattlegroundFactoryMethod)(BattlegroundMap* mgr, uint32_t iid, uint32_t group, uint32_t type);
typedef Battleground* (*ArenaFactoryMethod)(BattlegroundMap* mgr, uint32_t iid, uint32_t group, uint32_t type, uint32_t players_per_side);

class SERVER_DECL BattlegroundManager : public EventableObject
{
    BattlegroundManager() = default;
    ~BattlegroundManager() = default;

public:
    static BattlegroundManager& getInstance();
    void initialize();

    BattlegroundManager(BattlegroundManager&&) = delete;
    BattlegroundManager(BattlegroundManager const&) = delete;
    BattlegroundManager& operator=(BattlegroundManager&&) = delete;
    BattlegroundManager& operator=(BattlegroundManager const&) = delete;

    void registerBgFactory(uint32_t map, BattlegroundFactoryMethod method);
    void registerArenaFactory(uint32_t map, ArenaFactoryMethod method);
    void registerMapForBgType(uint32_t type, uint32_t map);

#if VERSION_STRING <= WotLK
    void handleBattlegroundListPacket(WorldSession* session, uint32_t battlegroundType, uint8_t from = 0);
#else
    void handleBattlegroundListPacket(WoWGuid& wowGuid, WorldSession* session, uint32_t battlegroundType);
#endif
    void handleArenaJoin(WorldSession* session, uint32_t battlegroundType, uint8_t asGroup, uint8_t ratedMatch);
    void handleGetBattlegroundQueueCommand(WorldSession* session);
    void handleBattlegroundJoin(WorldSession* session, WorldPacket& packet);

    void sendBattlefieldStatus(Player* player, BattlegroundDef::Status status, uint32_t type, uint32_t instanceId, uint32_t time, uint32_t mapId, uint8_t ratedMatch);

    void eventQueueUpdate();
    void eventQueueUpdate(bool forceStart);

    void removePlayerFromQueues(Player* player);
    void removeGroupFromQueues(uint32_t  group);

    Battleground* createInstance(uint32_t type, uint32_t levelGroup);

    bool canCreateInstance(uint32_t type, uint32_t levelGroup);

    void deleteBattleground(Battleground* battleground);

    uint32_t getArenaGroupQInfo(Group* group, uint8_t type, uint32_t* averageRating);

    int createArenaType(uint8_t type, Group* group1, Group* group2);

    void addPlayerToBgTeam(Battleground* battleground, std::deque<uint32_t>* playerVec, uint32_t type, uint32_t levelGroup, uint32_t team);

    void addPlayerToBg(Battleground* battleground, std::deque<uint32_t>* playerVec, uint32_t type, uint32_t levelGroup);

    void addGroupToArena(Battleground* battleground, Group* group, uint32_t team);

    uint32_t getMinimumPlayers(uint32_t dbcIndex);

    uint32_t getMaximumPlayers(uint32_t dbcIndex);

private:
    std::map<uint32_t, Battleground*> m_instances[BATTLEGROUND_NUM_TYPES];
    std::mutex m_instanceLock;

    uint32_t m_maxBattlegroundId[BATTLEGROUND_NUM_TYPES] = {0};

    std::list<uint32_t> m_queuedPlayers[BATTLEGROUND_NUM_TYPES][BattlegroundDef::MAX_LEVEL_GROUP];

    std::list<uint32_t> m_queuedGroups[BATTLEGROUND_NUM_TYPES];

    std::mutex m_queueLock;

    std::map<uint32_t, BattlegroundFactoryMethod> m_bgFactories;

    std::vector<ArenaFactoryMethod> m_arenaFactories;

    std::map<uint32_t, uint32_t> m_bgMaps;

    std::vector<uint32_t> m_arenaMaps;

    // All battlegrounds that are available for random BG queue
    std::vector<uint32_t> m_avalibleInRandom = { BattlegroundDef::TYPE_ALTERAC_VALLEY, BattlegroundDef::TYPE_WARSONG_GULCH,
        BattlegroundDef::TYPE_ARATHI_BASIN, BattlegroundDef::TYPE_EYE_OF_THE_STORM, BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT,
        BattlegroundDef::TYPE_ISLE_OF_CONQUEST };
};

#define sBattlegroundManager BattlegroundManager::getInstance()
