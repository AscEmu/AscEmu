/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "EventableObject.h"
#include "WorldConfig.h"

#include <set>
#include <string>
#include <unordered_map>

#include "PerformanceCounter.hpp"
#include "Utilities/CallBack.h"
#include "Chat/CommandTableStorage.hpp"

class WorldPacket;
class WorldSocket;
class WorldSession;

// Values based on ServerMessages.dbc
enum ServerMessageType
{
    // Vanilla
    SERVER_MSG_SHUTDOWN_TIME = 1,                               // [SERVER] Shutdown in %s
    SERVER_MSG_RESTART_TIME = 2,                                // [SERVER] Restart in %s
    SERVER_MSG_STRING = 3,                                      // %s
    SERVER_MSG_SHUTDOWN_CANCELLED = 4,                          // [SERVER] Shutdown cancelled
    SERVER_MSG_RESTART_CANCELLED = 5,                           // [SERVER] Restart cancelled
    // TBC
    SERVER_MSG_BATTLEGROUND_SHUTDOWN = 6,                       // [SERVER] Battleground shutdown in %s
    SERVER_MSG_BATTLEGROUND_RESTART = 7,                        // [SERVER] Battleground restart in %s
    SERVER_MSG_INSTANCE_SHUTDOWN = 8,                           // [SERVER] Instance shutdown in %s
    SERVER_MSG_INSTANCE_RESTART = 9,                            // [SERVER] Instance restart in %s
    // Cataclysm
    SERVER_MSG_CATACLYSM_CONTENT_AVAILABLE = 10,                // Cataclysm content is now available. Please completely quit and restart World of Warcraft, then enjoy the game.
    SERVER_MSG_TICKET_WILL_BE_SERVICED_SOON = 11,               // Your ticket will be serviced soon.
    SERVER_MSG_WAIT_TIME_CURRENTLY_UNAVAILABLE = 12,            // Wait time currently unavailable.
    SERVER_MSG_AVERAGE_TICKET_WAIT_TIME = 13,                   // Average ticket wait time:\n %s
    // MOP
    SERVER_MSG_MOP_HAS_LAUNCHED = 14,                           // Mists of Pandaria has launched!
    SERVER_MSG_MOP_HAS_LAUNCHED_VISIT_ORGRIMMAR = 15,           // Mists of Pandaria has launched! Visit Orgrimmar to begin your adventure!
    SERVER_MSG_MOP_HAS_LAUNCHED_VISIT_STORMWIND = 16,           // Mists of Pandaria has launched! Visit Stormwind to begin your adventure!
    SERVER_MSG_CROSS_REALM_SHUTDOWN = 17,                       // [SERVER] Cross realm shutdown in %s
    SERVER_MSG_CROSS_REALM_RESTART = 18,                        // [SERVER] Cross realm restart in %s
};

class SpellInfo;
class Object;

typedef std::set<WorldSession*> SessionSet;

class SERVER_DECL World : public EventableObject
{
private:
    World();
    ~World();

public:
    static World& getInstance();
    void initialize();
    void finalize();

    World(World&&) = delete;
    World(World const&) = delete;
    World& operator=(World&&) = delete;
    World& operator=(World const&) = delete;

    //////////////////////////////////////////////////////////////////////////////////////////
    // WorldConfig
    WorldConfig settings;

    void loadWorldConfigValues(bool reload = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player statistic
private:
    uint32_t mHordePlayersCount = 0;
    uint32_t mAlliancePlayersCount = 0;

public:
    uint32_t getPlayerCount();
    void resetPlayerCount();
    void incrementPlayerCount(uint32_t team);
    void decrementPlayerCount(uint32_t team);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Uptime
private:
    uint32_t mStartTime = 0;

public:
    void setWorldStartTime(uint32_t start_time);
    uint32_t getWorldStartTime();
    uint32_t getWorldUptime();
    std::string getWorldUptimeString();

    //////////////////////////////////////////////////////////////////////////////////////////
    // InfoCore
private:
    Ascemu::PerformanceCounter perfcounter;

    double mTotalTrafficInKB = 0;
    double mTotalTrafficOutKB = 0;
    double mLastTotalTrafficInKB = 0;
    double mLastTotalTrafficOutKB = 0;
    time_t mLastTrafficQuery = 0;

    void updateAllTrafficTotals();

    uint32_t mAcceptedConnections = 0;
    uint32_t mPeakSessionCount = 0;

public:
    void setTotalTraffic(double* totalin, double* totalout);
    void setLastTotalTraffic(double* totalin, double* totalout);

    float getCPUUsage();
    float getRAMUsage();

    uint32_t getAcceptedConnections() { return mAcceptedConnections; };
    void increaseAcceptedConnections() { ++mAcceptedConnections; }

    uint32_t getPeakSessionCount() { return mPeakSessionCount; };
    void setNewPeakSessionCount(uint32_t newCount) { mPeakSessionCount = newCount; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Session functions
private:
    typedef std::unordered_map<uint32_t, std::unique_ptr<WorldSession>> activeSessionMap;
    activeSessionMap mActiveSessionMapStore;

    std::mutex mSessionLock;

public:
    void addSession(std::unique_ptr<WorldSession> worldSession);

    WorldSession* getSessionByAccountId(uint32_t accountId);
    WorldSession* getSessionByAccountName(const std::string& accountName);

    void sendCharacterEnumToAccountSession(QueryResultVector& results, uint32_t accountId);
    void loadAccountDataProcForId(QueryResultVector& results, uint32_t accountId);

    size_t getSessionCount();

    void deleteSession(WorldSession* worldSession);
    void deleteSessions(std::list<WorldSession*> &slist);

    void disconnectSessionByAccountName(const std::string& accountName, WorldSession* worldSession);
    void disconnectSessionByIp(const std::string& ipString, WorldSession* worldSession);
    void disconnectSessionByPlayerName(const std::string& playerName, WorldSession* worldSession);

    //////////////////////////////////////////////////////////////////////////////////////////
    // GlobalSession functions
private:
    SessionSet globalSessionSet;

public:
    void addGlobalSession(WorldSession* worldSession);
    void updateGlobalSession(uint32_t diff);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Session queue
private:
    typedef std::list<std::pair<WorldSocket*, std::unique_ptr<WorldSession>>> QueuedWorldSocketList;
    QueuedWorldSocketList mQueuedSessions;

    std::mutex queueMutex;

    uint32_t mQueueUpdateTimer = 0;

public:
    void updateQueuedSessions(uint32_t diff);
    uint32_t getQueuedSessions() { return static_cast<uint32_t>(mQueuedSessions.size()); };

    uint32_t addQueuedSocket(WorldSocket* socket, std::unique_ptr<WorldSession> sessionHolder);
    void removeQueuedSocket(WorldSocket* socket);

    uint32_t getQueueUpdateTimer() { return mQueueUpdateTimer; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Send Messages
public:
    void sendMessageToOnlineGms(const std::string& message, WorldSession* sendToSelf = nullptr);

    void sendMessageToAll(const std::string& message, WorldSession* sendToSelf = nullptr);
    void sendAreaTriggerMessage(const std::string& message, WorldSession* sendToSelf = nullptr);
    void sendGlobalMessage(WorldPacket* worldPacket, WorldSession* sendToSelf = nullptr, uint32_t team = 3);

    void sendZoneMessage(WorldPacket* worldPacket, uint32_t zoneId, WorldSession* sendToSelf = nullptr);
    void sendInstanceMessage(WorldPacket* worldPacket, uint32_t instanceId, WorldSession* sendToSelf = nullptr);
    void sendZoneUnderAttackMessage(uint32_t areaId, uint8_t teamId);

    void sendBroadcastMessageById(uint32_t broadcastId);
    
    //////////////////////////////////////////////////////////////////////////////////////////
    // General Functions
private:
    std::unique_ptr<EventableObjectHolder> mEventableObjectHolder;
#if VERSION_STRING < Cata
    uint8_t mDbcLocaleId = 0;
#endif

public:
    std::list<std::unique_ptr<SpellInfo>> dummySpellList;

    bool setInitialWorldSettings();
    void resetCharacterLoginBannState();
    bool loadDbcDb2Stores();

    // TODO: figure out how to get it for cata/mop
    // although this is probably not needed anymore after wotlk
#if VERSION_STRING < Cata
    // Loads correct localization id used in name columns in DBC files
    void loadDbcLocaleLanguage();
    uint8_t getDbcLocaleLanguageId() const;
#endif

    void loadMySQLStores();
    void loadMySQLTablesByTask();
    void logEntitySize();

    void Update(unsigned long timePassed);

    void saveAllPlayersToDb();
    void playSoundToAllPlayers(uint32_t soundId);
    void logoutAllPlayers();

    void deleteObject(Object* object);

    //////////////////////////////////////////////////////////////////////////////////////////
    // GM Ticket System
private:
    bool mGmTicketSystemEnabled = false;

public:
    bool getGmTicketStatus();
    void toggleGmTicketStatus();
};

#define sWorld World::getInstance()
#define worldConfig sWorld.settings
