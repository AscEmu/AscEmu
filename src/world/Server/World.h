/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "EventableObject.h"
#include "IUpdatable.h"
//#include "Definitions.h"
//#include "Storage/DBC/DBCStores.h"
#include "WorldSession.h"
#include "WorldConfig.h"
#include "World.Legacy.h"

#include <set>
#include <string>
#include <vector>

class SERVER_DECL World : public Singleton<World>, public EventableObject, public IUpdatable
{
    public:

        World();
        ~World();

    //////////////////////////////////////////////////////////////////////////////////////////
    // WorldConfig
        WorldConfig settings;

        void loadWorldConfigValues(bool reload = false);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Player statistic
    private:
    
        uint32_t mHordePlayersCount;
        uint32_t mAlliancePlayersCount;

    public:

        uint32_t getPlayerCount();
        void resetPlayerCount();
        void incrementPlayerCount(uint32_t team);
        void decrementPlayerCount(uint32_t team);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Uptime
    private:

        uint32_t mStartTime;

    public:

        void setWorldStartTime(uint32_t start_time);
        uint32_t getWorldStartTime();
        uint32_t getWorldUptime();
        std::string getWorldUptimeString();

    //////////////////////////////////////////////////////////////////////////////////////////
    // InfoCore
    private:

        Arcemu::PerformanceCounter perfcounter;

        double mTotalTrafficInKB;
        double mTotalTrafficOutKB;
        double mLastTotalTrafficInKB;
        double mLastTotalTrafficOutKB;
        time_t mLastTrafficQuery;

        void updateAllTrafficTotals();

        uint32_t mAcceptedConnections;
        uint32_t mPeakSessionCount;

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

        typedef std::unordered_map<uint32_t, WorldSession*> activeSessionMap;
        activeSessionMap mActiveSessionMapStore;

        RWLock mSessionLock;

    public:

        void addSession(WorldSession* worldSession);

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
        Mutex globalSessionMutex;

    public:

        void addGlobalSession(WorldSession* worldSession);
        void updateGlobalSession(uint32_t diff);

    //////////////////////////////////////////////////////////////////////////////////////////
    // Session queue
    private:

        typedef std::list<WorldSocket*> QueuedWorldSocketList;
        QueuedWorldSocketList mQueuedSessions;

        Mutex queueMutex;

        uint32_t mQueueUpdateTimer;

    public:

        void updateQueuedSessions(uint32_t diff);
        uint32_t getQueuedSessions() { return static_cast<uint32_t>(mQueuedSessions.size()); };

        uint32_t addQueuedSocket(WorldSocket* socket);
        void removeQueuedSocket(WorldSocket* socket);

        uint32_t getQueueUpdateTimer() { return mQueueUpdateTimer; }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Send Messages
    public:

        void sendMessageToOnlineGms(const std::string& message, WorldSession* sendToSelf = nullptr);

        void sendMessageToAll(const std::string& message, WorldSession* sendToSelf = nullptr);
        void sendAreaTriggerMessage(const std::string& message, WorldSession* sendToSelf = nullptr);
        void sendGlobalMessage(WorldPacket* worldPacket, WorldSession* sendToSelf = nullptr);

        void sendZoneMessage(WorldPacket* worldPacket, uint32_t zoneId, WorldSession* sendToSelf = nullptr);
        void sendInstanceMessage(WorldPacket* worldPacket, uint32_t instanceId, WorldSession* sendToSelf = nullptr);
        void sendZoneUnderAttackMessage(uint32_t areaId, uint8_t teamId);

        void sendBroadcastMessageById(uint32_t broadcastId);
        
    //////////////////////////////////////////////////////////////////////////////////////////
    // General Functions
    private:

        EventableObjectHolder* mEventableObjectHolder;

    public:

        std::list<SpellInfo const*> dummySpellList;

        bool setInitialWorldSettings();
        void resetCharacterLoginBannState();
        bool loadDbcDb2Stores();

        void loadMySQLStores();
        void loadMySQLTablesByTask();
        void logEntitySize();

        void Update(unsigned long timePassed);

        void saveAllPlayersToDb();
        void playSoundToAllPlayers(uint32_t soundId);
        void logoutAllPlayers();

        void checkForExpiredInstances();

        void deleteObject(Object* object);

    //////////////////////////////////////////////////////////////////////////////////////////
    // GM Ticket System
    private:

        bool mGmTicketSystemEnabled;

    public:

        bool getGmTicketStatus();
        void toggleGmTicketStatus();
};

#define sWorld World::getSingleton()
#define worldConfig sWorld.settings
