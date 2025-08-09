/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Server/EventableObject.h"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Logging/Log.hpp"

#include <mutex>
#include <atomic>

class Object;
class Spell;
class Unit;
class LocationVector;
class GameObject;
class Creature;
class WorldMap;

class SERVER_DECL Battleground : public EventableObject
{
public:
    // Creating a battleground requires a pre-existing map manager
    Battleground(WorldMap* worldMap, uint32_t id, uint32_t levelGroup, uint32_t type);
    virtual ~Battleground();

protected:
    WorldMap* m_mapMgr = nullptr;
    uint32_t m_id = 0;
    uint32_t m_type = 0;
    uint32_t m_levelGroup = 0;
    std::atomic<uint32_t> m_invisGMs = 0;

    bool m_hasEnded = false;
    bool m_hasStarted = false;

    uint8_t m_winningTeam = 0;

    time_t m_nextPvPUpdateTime = 0;
    uint32_t m_countdownStage = 0;

    uint32_t m_startTime = static_cast<uint32_t>(UNIXTIME);
    uint32_t m_lastResurrectTime = static_cast<uint32_t>(UNIXTIME);

    uint32_t m_zoneId = 0;

    Group* m_groups[2] = { nullptr };

    uint32_t m_deltaRating[2] = { 0 };

    uint32_t m_honorPerKill = 0;

    std::recursive_mutex m_mutex;

    // PvP Log Data Map
    std::map<uint32_t, BGScore> m_pvpData;

    // Player count per team 
    uint32_t m_playerCountPerTeam = 0;

    // "pending" players
    std::set<uint32_t> m_pendPlayers[2];

    std::map<Creature*, std::set<uint32_t> > m_resurrectMap;

    bool m_isWeekend = false;

    friend class AVNode;

public:
    // Team->Player Map
    std::set<Player*> m_players[2];

    void addInvisGM();
    void removeInvisGM();
    std::recursive_mutex& GetMutex();

    void startBattleground();
    void endBattleground(PlayerTeam winningTeam);
    bool hasStarted();
    bool hasEnded();

    void addHonorToTeam(uint32_t team, uint32_t amount);

    void castSpellOnTeam(uint32_t team, uint32_t spell);

    void removeAuraFromTeam(uint32_t team, uint32_t aura);

    void sendChatMessage(uint8_t Type, uint64_t Guid, const char* Format, ...);

    // Retrieval Functions
    uint32_t getId();
    uint32_t getLevelGroup();
    WorldMap* getWorldMap();

    // Send the pvp log data of all players to this player
    void sendPVPData(Player* plr);

    // Send a packet to the entire battleground
    void distributePacketToAll(WorldPacket* packet);

    // send a packet to only this team
    void distributePacketToTeam(WorldPacket* packet, uint32_t Team);
    void playSoundToTeam(uint32_t Team, uint32_t Sound);
    void playSoundToAll(uint32_t Sound);

    bool isFull();

    // Are we full?
    bool hasFreeSlots(uint32_t Team, uint32_t type);

    void addPlayer(Player* plr, uint32_t team);
    void removePlayer(Player* plr, bool logout);
    void portPlayer(Player* plr, bool skip_teleport = false);
    void removePendingPlayer(Player* plr);
    uint32_t getFreeSlots(uint32_t t, uint32_t type);

    GameObject* spawnGameObject(uint32_t entry, LocationVector const& v, uint32_t flags, uint32_t faction, float scale);
    Creature* spawnCreature(uint32_t entry, float x, float y, float z, float o, uint32_t faction = 0);
    Creature* spawnCreature(uint32_t entry, LocationVector& v, uint32_t faction = 0);
    void updatePvPData();

    uint32_t getStartTime();
    uint32_t getType();

    // events should execute in the correct context
    int32_t event_GetInstanceID() override;
    void eventCreate();
    void eventCountdown();
    void close();

    void setWorldState(uint32_t Index, uint32_t Value);
    Creature* spawnSpiritGuide(float x, float y, float z, float o, uint32_t horde);
    Creature* spawnSpiritGuide(LocationVector& v, uint32_t faction);

    uint32_t getLastResurrect();
    void addSpiritGuide(Creature* pCreature);
    void removeSpiritGuide(Creature* pCreature);
    void queuePlayerForResurrect(Player* plr, Creature* spirit_healer);
    void removePlayerFromResurrect(Player* plr, Creature* spirit_healer);
    void eventResurrectPlayers();

    void buildPvPUpdateDataPacket(WorldPacket* data);
    void onPlayerPushed(Player* plr);

    void queueAtNearestSpiritGuide(Player* plr, Creature* old);

    static bool isTypeArena(uint32_t x);
    bool isArena();

    uint32_t getFieldCount(uint32_t BGType);

    // Hook Functions
    virtual bool HandleFinishBattlegroundRewardCalculation(PlayerTeam winningTeam);
    virtual void HookOnPlayerResurrect(Player* player);
    virtual void HookOnUnitDied(Unit* victim);
    virtual void OnStart();
    virtual void OnClose();
    virtual bool HookSlowLockOpen(GameObject* pGo, Player* pPlayer, Spell* pSpell);
    virtual bool HookQuickLockOpen(GameObject* go, Player* player, Spell* spell);

    // Pure Hooks
    virtual void HookOnPlayerDeath(Player* plr) = 0;

    // Repopping - different battlegrounds have different ways of handling this
    virtual bool HookHandleRepop(Player* plr) = 0;

    // In CTF battlegrounds mounting will cause you to lose your flag.
    virtual void HookOnMount(Player* plr) = 0;

    // Only used in CTF (as far as I know)
    virtual void HookFlagDrop(Player* plr, GameObject* obj) = 0;
    virtual void HookFlagStand(Player* plr, GameObject* obj) = 0;
    virtual void HookOnFlagDrop(Player* plr) = 0;

    // Used when a player kills a player
    virtual void HookOnPlayerKill(Player* plr, Player* pVictim) = 0;
    virtual void HookOnHK(Player* plr) = 0;

    // On Area Trigger
    virtual void HookOnAreaTrigger(Player* plr, uint32_t id) = 0;

    // On Shadow Sight
    virtual void HookOnShadowSight() = 0;

    // On Loot Generating
    virtual void HookGenerateLoot(Player* plr, Object* pCorpse) = 0;

    // On Unit Killing
    virtual void HookOnUnitKill(Player* plr, Unit* pVictim) = 0;
    virtual void OnAddPlayer(Player* plr) = 0;
    virtual void OnCreate() = 0;
    virtual void OnRemovePlayer(Player* plr) = 0;

    // Get the starting position for this team.
    virtual LocationVector GetStartingCoords(uint32_t Team) = 0;

    virtual uint32_t GetNameID();
    virtual bool CanPlayerJoin(Player* plr, uint32_t type);
    virtual bool CreateCorpse(Player* plr);
    virtual uint8_t Rated();
    virtual void SetIsWeekend(bool isweekend);
    virtual uint64_t GetFlagHolderGUID(uint32_t faction) const;
};
