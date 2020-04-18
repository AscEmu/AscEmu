/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/TransportSpawn.hpp"
#include "Objects/GameObject.h"

class TransportPath
{
    public:
        struct PathNode
        {
            uint32_t mapid;
            float x, y, z;
            uint32_t actionFlag;
            uint32_t delay;
        };

        inline void SetLength(const unsigned int sz)
        {
            i_nodes.resize(sz);
        }

        inline size_t Size(void) const { return i_nodes.size(); }
        inline void Resize(unsigned int sz) { i_nodes.resize(sz); }
        inline void Clear(void) { i_nodes.clear(); }
        inline PathNode* GetNodes(void) { return static_cast< PathNode* >(&i_nodes[0]); }
        float GetTotalLength(void)
        {
            float len = 0, xd, yd, zd;
            for (unsigned int idx = 1; idx < i_nodes.size(); ++idx)
            {
                xd = i_nodes[ idx ].x - i_nodes[ idx - 1 ].x;
                yd = i_nodes[ idx ].y - i_nodes[ idx - 1 ].y;
                zd = i_nodes[ idx ].z - i_nodes[ idx - 1 ].z;
                len += (float)std::sqrt(xd * xd + yd * yd + zd * zd);
            }
            return len;
        }

        PathNode & operator[](const unsigned int idx) { return i_nodes[idx]; }
        const PathNode & operator()(const unsigned int idx) const { return i_nodes[idx]; }

    protected:
        std::vector<PathNode> i_nodes;
};

struct keyFrame
{
    keyFrame(float _x, float _y, float _z, uint32_t _mapid, int _actionflag, int _delay)
    {
        x = _x;
        y = _y;
        z = _z;
        mapid = _mapid;
        actionflag = _actionflag;
        delay = _delay;
        distFromPrev = -1;
        distSinceStop = -1;
        distUntilStop = -1;
        tFrom = 0;
        tTo = 0;
    }

    float x;
    float y;
    float z;
    uint32_t mapid;
    int actionflag;
    int delay;
    float distSinceStop;
    float distUntilStop;
    float distFromPrev;
    float tFrom, tTo;
};

struct TWayPoint
{
    TWayPoint() : mapid(0), x(0), y(0), z(0), o(0), teleport(0), delayed(false) {}
    TWayPoint(uint32_t _mapid, float _x, float _y, float _z, bool _teleport) :
        mapid(_mapid), x(_x), y(_y), z(_z), o(0), teleport(_teleport), delayed(false) {}
    uint32_t mapid;
    float x;
    float y;
    float z;
    float o;
    bool teleport;
    bool delayed;
};

bool FillTransporterPathVector(uint32_t PathID, TransportPath & Path);

class SERVER_DECL Transporter : public GameObject
{
protected:

    std::vector<TransportSpawn> m_creatureSpawns;

public:

    Transporter(uint64_t guid);
    ~Transporter();
    
    void AddCreature(TransportSpawn creature);
    void RespawnCreaturePassengers();

    // Creates The Transporter
    bool Create(uint32_t entry, int32_t Time);

    // Start Generating of the Waypoints
    bool GenerateWaypoints(uint32_t pathid);

    // Update Transporter Position and Transport Passengers
    void Update();

    // Add Passenger to Transporter
    bool AddPassenger(Player* passenger);

    // Remove Passenger from Transporter
    bool RemovePassenger(Player* passenger);

    // Start Update EVent on Transporter push to World
    void OnPushToWorld();

    // Build Update for Player
    uint32_t  buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);

    std::set<uint32_t> const& GetPassengers() const { return m_passengers; }

    typedef std::set<Creature*> CreatureSet;
    CreatureSet m_NPCPassengerSet;

    // Spawning of the Creatures on Continent Transports
    uint32_t AddNPCPassenger(uint32_t tguid, uint32_t entry, float x, float y, float z, float o, uint32_t anim = 0);

    // Spawning of the Creatures in Instance Transports
    Creature* AddNPCPassengerInInstance(uint32_t entry, float x, float y, float z, float o, uint32_t anim = 0);

    // Removes NPC Passenger
    void RemovePassenger(Creature* passenger) { m_NPCPassengerSet.erase(passenger); }

    // Update NPC Position
    void UpdateNPCPositions(float x, float y, float z, float o);

    // Update Player POsition
    void UpdatePlayerPositions(float x, float y, float z, float o);

    // Builds Start Move Packet
    void BuildStartMovePacket(MapMgr* targetMap);

    // Builds Stop Move Packet
    void BuildStopMovePacket(MapMgr* targetMap);

    // Transport Gossip
    void TransportGossip(uint32_t route);

    void SetPeriod(int32_t val);
    int32_t GetPeriod();

    uint32_t m_pathTime;
    uint32_t m_timer;

    std::set<uint32_t> m_passengers;

    uint32_t currenttguid;

private:

    typedef std::map<uint32_t, TWayPoint> WaypointMap;

    WaypointMap::const_iterator mCurrentWaypoint;
    WaypointMap::const_iterator mNextWaypoint;

public:

    WaypointMap m_WayPoints;

private:

    void TeleportTransport(uint32_t newMapid, uint32_t oldmap, float x, float y, float z);
    void GetNextWaypoint();
    int32_t m_period;

protected:

    Mutex m_creatureSetMutex;
};
