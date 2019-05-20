/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
            uint32 mapid;
            float x, y, z;
            uint32 actionFlag;
            uint32 delay;
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
    keyFrame(float _x, float _y, float _z, uint32 _mapid, int _actionflag, int _delay)
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
    uint32 mapid;
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
    TWayPoint(uint32 _mapid, float _x, float _y, float _z, bool _teleport) :
        mapid(_mapid), x(_x), y(_y), z(_z), o(0), teleport(_teleport), delayed(false) {}
    uint32 mapid;
    float x;
    float y;
    float z;
    float o;
    bool teleport;
    bool delayed;
};

bool FillTransporterPathVector(uint32 PathID, TransportPath & Path);

class SERVER_DECL Transporter : public GameObject
{
protected:

    std::vector<TransportSpawn> m_creatureSpawns;

public:

    Transporter(uint64 guid);
    ~Transporter();
    
    void AddCreature(TransportSpawn creature);
    void RespawnCreaturePassengers();

    // Creates The Transporter
    bool Create(uint32 entry, int32 Time);

    // Start Generating of the Waypoints
    bool GenerateWaypoints(uint32 pathid);

    // Update Transporter Position and Transport Passengers
    void Update();

    // Add Passenger to Transporter
    bool AddPassenger(Player* passenger);

    // Remove Passenger from Transporter
    bool RemovePassenger(Player* passenger);

    // Start Update EVent on Transporter push to World
    void OnPushToWorld();

    // Build Update for Player
    uint32  buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);

    std::set<uint32> const& GetPassengers() const { return m_passengers; }

    typedef std::set<Creature*> CreatureSet;
    CreatureSet m_NPCPassengerSet;

    // Spawning of the Creatures on Continent Transports
    uint32 AddNPCPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o, uint32 anim = 0);

    // Spawning of the Creatures in Instance Transports
    Creature* AddNPCPassengerInInstance(uint32 entry, float x, float y, float z, float o, uint32 anim = 0);

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
    void TransportGossip(uint32 route);

    void SetPeriod(int32 val);
    int32 GetPeriod();

    uint32 m_pathTime;
    uint32 m_timer;

    std::set<uint32> m_passengers;

    uint32 currenttguid;

private:

    typedef std::map<uint32, TWayPoint> WaypointMap;

    WaypointMap::const_iterator mCurrentWaypoint;
    WaypointMap::const_iterator mNextWaypoint;

public:

    WaypointMap m_WayPoints;

private:

    void TeleportTransport(uint32 newMapid, uint32 oldmap, float x, float y, float z);
    void GetNextWaypoint();
    int32 m_period;

protected:

    Mutex m_creatureSetMutex;
};
