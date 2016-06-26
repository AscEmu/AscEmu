/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"

bool FillTransporterPathVector(uint32 PathID, TransportPath & Path)
{
    // Store dbc values into current Path array
    Path.Resize(sTaxiPathNodeStore.GetNumRows());

    uint32 i = 0;
    for (uint32 j = 0; j < sTaxiPathNodeStore.GetNumRows(); ++j)
    {
        auto pathnode = sTaxiPathNodeStore.LookupEntry(j);
        if (pathnode == nullptr)
            continue;

        if (pathnode->path == PathID)
        {
            Path[i].mapid = pathnode->mapid;
            Path[i].x = pathnode->x;
            Path[i].y = pathnode->y;
            Path[i].z = pathnode->z;
            Path[i].actionFlag = pathnode->flags;
            Path[i].delay = pathnode->waittime;
            ++i;
        }
    }

    Path.Resize(i);
    return (i > 0 ? true : false);
}

Transporter* ObjectMgr::LoadTransportInInstance(MapMgr *instance, uint32 goEntry, uint32 period)
{
    auto gameobject_info = sMySQLStore.GetGameObjectProperties(goEntry);
    if (gameobject_info == nullptr)
    {
        Log.Error("Transport Handler", "Transport ID:%u, will not be loaded, gameobject_properties missing", goEntry);
        return NULL;
    }

    if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
    {
        Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_properties type wrong", goEntry, gameobject_info->name.c_str());
        return NULL;
    }

    std::set<uint32> mapsUsed;

    Transporter* t = new Transporter((uint64)HIGHGUID_TYPE_TRANSPORTER << 32 | goEntry);
    if (!t->Create(goEntry, period))
    {
        delete t;
        return NULL;
    }

    m_Transporters.insert(t);
    m_TransportersByInstanceIdMap[instance->GetInstanceID()].insert(t);
    AddTransport(t);

    // AddObject To World
    t->AddToWorld(instance);

    // correct incorrect instance id's
    t->SetInstanceID(instance->GetInstanceID());
    t->SetMapId(t->GetMapId());

    t->BuildStartMovePacket(instance);
    t->BuildStopMovePacket(instance);
    t->m_WayPoints.clear(); // Make transport stopped at server-side, movement will be handled by scripts

    Log.Success("Transport Handler", "Spawned Transport Entry %u, map %u, instance id %u", goEntry, t->GetMapId(), t->GetInstanceID());
    return t;
}

void ObjectMgr::UnloadTransportFromInstance(Transporter *t)
{
    m_creatureSetMutex.Acquire();
    for (Transporter::CreatureSet::iterator itr = t->m_NPCPassengerSet.begin(); itr != t->m_NPCPassengerSet.end();)
    {
        if (Creature* npc = *itr)
        {
            npc->RemoveFromWorld(true);
        }
        ++itr;
    }

    t->m_NPCPassengerSet.clear();
    m_creatureSetMutex.Release();
    m_TransportersByInstanceIdMap[t->GetInstanceID()].erase(t);
    m_Transporters.erase(t);
    t->m_WayPoints.clear();
    t->RemoveFromWorld(true);
}

void ObjectMgr::LoadTransports()
{
    Log.Success("TransportHandler", "Starting loading transport data...");
    {
        QueryResult* result = WorldDatabase.Query("SELECT entry, name, period FROM transport_data");

        if (!result)
        {
            Log.Error("Transporter Handler", ">> Loaded 0 transports. DB table `transport_data` is empty!");
            return;
        }

        uint32 pCount = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            std::string name = fields[1].GetString();
            uint32 period = fields[2].GetUInt32();

            auto gameobject_info = sMySQLStore.GetGameObjectProperties(entry);
            if (gameobject_info == nullptr)
            {
                Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_properties missing", entry, name.c_str());
                continue;
            }

            if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
            {
                Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_properties type wrong", entry, name.c_str());
                continue;
            }

            std::set<uint32> mapsUsed;

            Transporter* pTransporter = new Transporter((uint64)HIGHGUID_TYPE_TRANSPORTER << 32 | entry);

            // Generate waypoints
            if (!pTransporter->GenerateWaypoints(gameobject_info->mo_transport.taxi_path_id))
            {
                Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, failed to create waypoints", entry, name.c_str());
                delete pTransporter;
                continue;
            }

            if (!pTransporter->Create(entry, period))
            {
                delete pTransporter;
                continue;
            }

            // AddObject To World
            pTransporter->AddToWorld();

            m_Transporters.insert(pTransporter);
            AddTransport(pTransporter);

            for (std::set<uint32>::const_iterator i = mapsUsed.begin(); i != mapsUsed.end(); ++i)
                m_TransportersByMap[*i].insert(pTransporter);

            ++pCount;
        } while (result->NextRow());

        delete result;

        Log.Success("Transporter Handler", ">> Loaded %u transports", pCount);
    }
    Log.Success("TransportHandler", "Starting loading transport creatures...");
    {
        QueryResult* result = WorldDatabase.Query("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM transport_creatures");

        if (!result)
        {
            Log.Error("Transport Handler", ">> Loaded 0 transport NPCs. DB table `transport_creatures` is empty!");
            return;
        }

        uint32 pCount = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 guid = fields[0].GetInt32();
            uint32 entry = fields[1].GetInt32();
            uint32 transportEntry = fields[2].GetInt32();
            float tX = fields[3].GetFloat();
            float tY = fields[4].GetFloat();
            float tZ = fields[5].GetFloat();
            float tO = fields[6].GetFloat();
            uint32 anim = fields[7].GetInt32();

            for (ObjectMgr::TransporterSet::iterator itr = m_Transporters.begin(); itr != m_Transporters.end(); ++itr)
            {
                if ((*itr)->GetEntry() == transportEntry)
                {
                    TransportSpawn spawn{ guid, entry, transportEntry, tX, tY, tZ, tO, anim };
                    (*itr)->AddCreature(spawn);
                    break;
                }
            }

            ++pCount;
        } while (result->NextRow());
        delete result;

        for (auto transport : m_Transporters)
        {
            transport->RespawnCreaturePassengers();
        }

        Log.Success("Transport Handler", ">> Loaded %u Transport Npcs", pCount);
    }
}

Transporter::Transporter(uint64 guid) : GameObject(guid), currenttguid(0)
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_HIGHGUID | UPDATEFLAG_HAS_POSITION | UPDATEFLAG_ROTATION);

    m_pathTime = 0;
    m_timer = 0;
    m_period = 0;
}

Transporter::~Transporter()
{
    sEventMgr.RemoveEvents(this);

    m_creatureSetMutex.Acquire();
    m_NPCPassengerSet.clear();
    m_creatureSetMutex.Release();

    m_WayPoints.clear();
    m_passengers.clear();
}

void Transporter::AddCreature(TransportSpawn creature)
{
    this->m_creatureSpawns.push_back(creature);
}

void Transporter::RespawnCreaturePassengers()
{
    // TODO: Respect existing creature positions
    m_creatureSetMutex.Acquire();
    for (auto existing_passenger : m_NPCPassengerSet)
    {
        if (existing_passenger->IsInWorld())
            existing_passenger->DeleteMe();
    }

    m_NPCPassengerSet.clear();
    m_creatureSetMutex.Release();
    for (auto spawn : this->m_creatureSpawns)
    {
        if (this->AddNPCPassenger(spawn.transport_guid, spawn.entry, spawn.x, spawn.y, spawn.z, spawn.o, spawn.animation) == 0)
            Log.Error("Transporter::RespawnCreaturePassengers", "Failed to add npc entry: %u to transport: %u", spawn.entry, spawn.transport_guid);
    }
}

void Transporter::OnPushToWorld()
{
    // Create waypoint event
    sEventMgr.AddEvent(this, &Transporter::Update, EVENT_TRANSPORTER_NEXT_WAYPOINT, 100, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

bool Transporter::Create(uint32 entry, int32 Time)
{
    auto gameobject_info = sMySQLStore.GetGameObjectProperties(entry);
    if (gameobject_info == nullptr)
    {
        Log.Error("Transporter::Create", "Failed to create Transporter with go entry %u. Invalid gameobject!", entry);
        return false;
    }

    // Create transport
    float x, y, z, o;
    uint32 mapid;
    x = m_WayPoints[0].x;
    y = m_WayPoints[0].y;
    z = m_WayPoints[0].z;
    mapid = m_WayPoints[0].mapid;
    o = 1;

    if (!CreateFromProto(entry, mapid, x, y, z, o))
        return false;

    // Override these flags to avoid mistakes in proto
    SetFlags(40);
    SetAnimProgress(255);

    SetType(GAMEOBJECT_TYPE_MO_TRANSPORT);

    m_overrides = GAMEOBJECT_INFVIS | GAMEOBJECT_ONMOVEWIDE;

    m_period = Time;

    // Set position
    SetMapId(mapid);
    SetPosition(x, y, z, o);
    SetLevel(m_period);

    return true;
}

bool Transporter::GenerateWaypoints(uint32 pathid)
{
    TransportPath path;
    FillTransporterPathVector(pathid, path);

    if (path.Size() == 0)
    {
        Log.Error("Transport Handler", "path Size == 0 for path %u", pathid);
        return false;
    }

    std::vector<keyFrame> keyFrames;
    int mapChange = 0;
    for (size_t i = 1; i < path.Size() - 1; ++i)
    {
       if (mapChange == 0)
        {
            if ((path[i].mapid == path[i + 1].mapid))
            {
                keyFrame k(path[i].x, path[i].y, path[i].z, path[i].mapid, path[i].actionFlag, path[i].delay);
                keyFrames.push_back(k);
            }
            else
            {
                mapChange = 1;
            }
        }
        else
        {
            --mapChange;
        }
    }

    int lastStop = -1;
    int firstStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].distFromPrev = 0;
    if (keyFrames[0].actionflag == 2)
    {
        lastStop = 0;
    }

    // find the rest of the distances between key points
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if ((keyFrames[i].actionflag == 1) || (keyFrames[i].mapid != keyFrames[i - 1].mapid))
        {
            keyFrames[i].distFromPrev = 0;
        }
        else
        {
            keyFrames[i].distFromPrev =
                sqrt(pow(keyFrames[i].x - keyFrames[i - 1].x, 2) +
                pow(keyFrames[i].y - keyFrames[i - 1].y, 2) +
                pow(keyFrames[i].z - keyFrames[i - 1].z, 2));
        }
        if (keyFrames[i].actionflag == 2)
        {
            if (firstStop == -1)
                firstStop = i;

            lastStop = i;
        }
    }

    float tmpDist = 0;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].actionflag == 2)
            tmpDist = 0;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    for (int i = int(keyFrames.size()) - 1; i >= 0; --i)
    {
        int j = (i + (firstStop + 1)) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].actionflag == 2)
            tmpDist = 0;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i].distSinceStop < (30 * 30 * 0.5f))
            keyFrames[i].tFrom = sqrt(2 * keyFrames[i].distSinceStop);
        else
            keyFrames[i].tFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;

        if (keyFrames[i].distUntilStop < (30 * 30 * 0.5f))
            keyFrames[i].tTo = sqrt(2 * keyFrames[i].distUntilStop);
        else
            keyFrames[i].tTo = ((keyFrames[i].distUntilStop - (30 * 30 * 0.5f)) / 30) + 30;

        keyFrames[i].tFrom *= 1000;
        keyFrames[i].tTo *= 1000;
    }

    //    for (int i = 0; i < keyFrames.size(); i++) {
    //        sLog.outString("%f, %f, %f, %f, %f, %f, %f", keyFrames[i].x, keyFrames[i].y, keyFrames[i].distUntilStop, keyFrames[i].distSinceStop, keyFrames[i].distFromPrev, keyFrames[i].tFrom, keyFrames[i].tTo);
    //    }

    // Now we're completely set up; we can move along the length of each waypoint at 100 ms intervals
    // speed = max(30, t) (remember x = 0.5s^2, and when accelerating, a = 1 unit/s^2
    int t = 0;
    bool teleport = false;
    if (keyFrames[keyFrames.size() - 1].mapid != keyFrames[0].mapid)
        teleport = true;

    TWayPoint pos(keyFrames[0].mapid, keyFrames[0].x, keyFrames[0].y, keyFrames[0].z, teleport);
    m_WayPoints[0] = pos;
    t += keyFrames[0].delay * 1000;

    uint32 cM = keyFrames[0].mapid;
    for (size_t i = 0; i < keyFrames.size() - 1; ++i)        //
    {
        float d = 0;
        float tFrom = keyFrames[i].tFrom;
        float tTo = keyFrames[i].tTo;

        // keep the generation of all these points; we use only a few now, but may need the others later
        if (((d < keyFrames[i + 1].distFromPrev) && (tTo > 0)))
        {
            while ((d < keyFrames[i + 1].distFromPrev) && (tTo > 0))
            {
                tFrom += 100;
                tTo -= 100;

                if (d > 0)
                {
                    float newX, newY, newZ;
                    newX = keyFrames[i].x + (keyFrames[i + 1].x - keyFrames[i].x) * d / keyFrames[i + 1].distFromPrev;
                    newY = keyFrames[i].y + (keyFrames[i + 1].y - keyFrames[i].y) * d / keyFrames[i + 1].distFromPrev;
                    newZ = keyFrames[i].z + (keyFrames[i + 1].z - keyFrames[i].z) * d / keyFrames[i + 1].distFromPrev;

                    bool teleport = false;
                    if (keyFrames[i].mapid != cM)
                    {
                        teleport = true;
                        cM = keyFrames[i].mapid;
                    }

                    //                    sLog.outString("T: %d, D: %f, x: %f, y: %f, z: %f", t, d, newX, newY, newZ);
                    TWayPoint pos(keyFrames[i].mapid, newX, newY, newZ, teleport);
                    if (teleport)
                    {
                        m_WayPoints[t] = pos;
                    }
                }

                if (tFrom < tTo)                            // caught in tFrom dock's "gravitational pull"
                {
                    if (tFrom <= 30000)
                    {
                        d = 0.5f * (tFrom / 1000) * (tFrom / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tFrom - 30000) / 1000);
                    }
                    d = d - keyFrames[i].distSinceStop;
                }
                else
                {
                    if (tTo <= 30000)
                    {
                        d = 0.5f * (tTo / 1000) * (tTo / 1000);
                    }
                    else
                    {
                        d = 0.5f * 30 * 30 + 30 * ((tTo - 30000) / 1000);
                    }
                    d = keyFrames[i].distUntilStop - d;
                }
                t += 100;
            }
            t -= 100;
        }

        if (keyFrames[i + 1].tFrom > keyFrames[i + 1].tTo)
            t += 100 - ((long)keyFrames[i + 1].tTo % 100);
        else
            t += (long)keyFrames[i + 1].tTo % 100;

        bool teleport = false;
        if ((keyFrames[i + 1].actionflag == 1) || (keyFrames[i + 1].mapid != keyFrames[i].mapid))
        {
            teleport = true;
            cM = keyFrames[i + 1].mapid;
        }

        TWayPoint pos(keyFrames[i + 1].mapid, keyFrames[i + 1].x, keyFrames[i + 1].y, keyFrames[i + 1].z, teleport);

        //        sLog.outString("T: %d, x: %f, y: %f, z: %f, t:%d", t, pos.x, pos.y, pos.z, teleport);

        //if (teleport)
        m_WayPoints[t] = pos;
        //if (keyFrames[i + 1].delay > 5)
        //    pos2.delayed = true;

        //m_WayPoints.insert(WaypointMap::value_type(t, pos2));
        //last_t = t;

        t += keyFrames[i + 1].delay * 1000;
        //        sLog.outString("------");
    }

    uint32 timer = t;


    mNextWaypoint = m_WayPoints.begin();
    GetNextWaypoint();
    GetNextWaypoint();
    m_pathTime = timer;

    return true;
}

void Transporter::GetNextWaypoint()
{
    mCurrentWaypoint = mNextWaypoint;

    ++mNextWaypoint;
    if (mNextWaypoint == m_WayPoints.end())
        mNextWaypoint = m_WayPoints.begin();
}

void Transporter::TeleportTransport(uint32 newMapid, uint32 oldmap, float x, float y, float z)
{
    //sEventMgr.RemoveEvents(this, EVENT_TRANSPORTER_NEXT_WAYPOINT);

    RemoveFromWorld(false);
    SetMapId(newMapid);
    SetPosition(x, y, z, m_position.o, false);
    AddToWorld();

    WorldPacket packet(SMSG_TRANSFER_PENDING, 12);
    packet << newMapid;
    packet << GetEntry();
    packet << oldmap;

    for (auto passengerGuid : m_passengers)
    {
        auto passenger = objmgr.GetPlayer(passengerGuid);
        if (passenger == nullptr)
            continue;

        passenger->GetSession()->SendPacket(&packet);
        bool teleport_successful = passenger->Teleport(LocationVector(x, y, z, passenger->GetOrientation()), this->GetMapMgr());
        if (!teleport_successful)
        {
            passenger->RepopAtGraveyard(passenger->GetPositionX(), passenger->GetPositionY(), passenger->GetPositionZ(), passenger->GetMapId());
        }
        else
        {
            if (!passenger->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
            {
                passenger->AddUnitMovementFlag(MOVEFLAG_TRANSPORT);
            }
        }
    }

    this->RespawnCreaturePassengers();
}

bool Transporter::AddPassenger(Player* passenger)
{
    ARCEMU_ASSERT(passenger != nullptr);

    m_passengers.insert(passenger->GetLowGUID());
    Log.Debug("Transporter", "Player %s boarded transport %u.", passenger->GetName(), this->GetGameObjectProperties()->entry);

    if (!passenger->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        passenger->AddUnitMovementFlag(MOVEFLAG_TRANSPORT);
    }

    return true;
}

bool Transporter::RemovePassenger(Player* passenger)
{
    ARCEMU_ASSERT(passenger != nullptr);

    m_passengers.erase(passenger->GetLowGUID());
    Log.Debug("Transporter", "Player %s removed from transport %u.", passenger->GetName(), this->GetGameObjectProperties()->entry);

    if (passenger->HasUnitMovementFlag(MOVEFLAG_TRANSPORT))
    {
        passenger->RemoveUnitMovementFlag(MOVEFLAG_TRANSPORT);
    }

    return true;
}

uint32 Transporter::BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    uint32 cnt = Object::BuildCreateUpdateBlockForPlayer(data, target);

    // add all the npcs to the packet
    m_creatureSetMutex.Acquire();
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* npc = *itr;
        cnt += npc->BuildCreateUpdateBlockForPlayer(data, target);
    }
    m_creatureSetMutex.Release();
    return cnt;
}

uint32 TimeStamp();
void Transporter::Update()
{
    if (m_WayPoints.size() <= 1)
        return;

    m_timer = getMSTime() % m_period;

    while (((m_timer - mCurrentWaypoint->first) % m_pathTime) > ((mNextWaypoint->first - mCurrentWaypoint->first) % m_pathTime))
    {
        GetNextWaypoint();

        // first check help in case client-server transport coordinates de-synchronization
        if (mCurrentWaypoint->second.mapid != GetMapId() || mCurrentWaypoint->second.teleport)
        {
            TeleportTransport(mCurrentWaypoint->second.mapid, GetMapId(), mCurrentWaypoint->second.x, mCurrentWaypoint->second.y, mCurrentWaypoint->second.z);
            break;
        }
        else
        {
            SetPosition(mCurrentWaypoint->second.x, mCurrentWaypoint->second.y, mCurrentWaypoint->second.z, std::atan2(mNextWaypoint->second.x, mNextWaypoint->second.y) + float(M_PI), false);
            UpdatePlayerPositions(mCurrentWaypoint->second.x, mCurrentWaypoint->second.y, mCurrentWaypoint->second.z, std::atan2(mNextWaypoint->second.x, mNextWaypoint->second.y) + float(M_PI));
            // After a few tests (Durotar<->Northrend we need this, otherwise npc disappear on entering new map/zone/area DankoDJ
            // Update Creature Position with Movement Info from Gameobject too prevent coord changes from Transporter Waypoint and Gameobject Position Aaron02
            UpdateNPCPositions(obj_movement_info.transporter_info.position.x, obj_movement_info.transporter_info.position.y, obj_movement_info.transporter_info.position.z, std::atan2(obj_movement_info.transporter_info.position.x, obj_movement_info.transporter_info.position.y) + float(M_PI));
        }
        if (mCurrentWaypoint->second.delayed)
        {
            switch (GetGameObjectProperties()->display_id)
            {
            case 3015:
            case 7087:
            {
                PlaySoundToSet(5154);        // ShipDocked LightHouseFogHorn.wav
            }
            break;
            case 3031:
            {
                PlaySoundToSet(11804);        // ZeppelinDocked    ZeppelinHorn.wav
            }
            break;
            default:
            {
                PlaySoundToSet(5495);        // BoatDockingWarning    BoatDockedWarning.wav
            }
            break;
            }
            TransportGossip(GetGameObjectProperties()->display_id);
        }
    }
}

void Transporter::TransportGossip(uint32 route)
{
    if (route == 241)
    {
        if (mCurrentWaypoint->second.mapid)
        {
            Log.Debug("Transporter", "Arrived in Ratchet at %u", m_timer);
        }

        else
        {
            Log.Debug("Transporter", "Arrived in Booty at %u", m_timer);
        }
    }
}

void Transporter::SetPeriod(int32 val)
{
    this->m_period = val;
}

int32 Transporter::GetPeriod()
{
    return this->m_period;
}

void Transporter::BuildStartMovePacket(MapMgr* targetMap)
{
    SetFlag(GAMEOBJECT_FLAGS, 1);
    SetState(GO_STATE_OPEN);
}

void Transporter::BuildStopMovePacket(MapMgr* targetMap)
{
    RemoveFlag(GAMEOBJECT_FLAGS, 1);
    SetState(GO_STATE_CLOSED);
}

uint32 Transporter::AddNPCPassenger(uint32 tguid, uint32 entry, float x, float y, float z, float o, uint32 anim)
{
    MapMgr* map = GetMapMgr();

    CreatureProperties const* creature_properties = sMySQLStore.GetCreatureProperties(entry);
    if (creature_properties == nullptr || map == nullptr)
        return 0;

    float transporter_x = obj_movement_info.transporter_info.position.x + x;
    float transporter_y = obj_movement_info.transporter_info.position.y + y;
    float transporter_z = obj_movement_info.transporter_info.position.z + z;

    Creature* pCreature = map->CreateCreature(entry);
    pCreature->Create(map->GetMapId(), transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->Load(creature_properties, transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->AddToWorld(map);
    pCreature->SetUnitMovementFlags(MOVEFLAG_TRANSPORT);
    pCreature->obj_movement_info.transporter_info.position.x = x;
    pCreature->obj_movement_info.transporter_info.position.y = y;
    pCreature->obj_movement_info.transporter_info.position.z = z;
    pCreature->obj_movement_info.transporter_info.position.o = o;
    pCreature->obj_movement_info.transporter_info.guid = GetGUID();

    pCreature->m_transportData.transportGuid = this->GetGUID();
    pCreature->m_transportData.relativePosition.x = x;
    pCreature->m_transportData.relativePosition.y = y;
    pCreature->m_transportData.relativePosition.z = z;
    pCreature->m_transportData.relativePosition.o = o;

    if (anim)
        pCreature->SetUInt32Value(UNIT_NPC_EMOTESTATE, anim);

    if (creature_properties->NPCFLags)
        pCreature->SetUInt32Value(UNIT_NPC_FLAGS, creature_properties->NPCFLags);

    m_creatureSetMutex.Acquire();
    m_NPCPassengerSet.insert(pCreature);
    m_creatureSetMutex.Release();
    if (tguid == 0)
    {
        ++currenttguid;
        tguid = currenttguid;
    }
    else
        currenttguid = std::max(tguid, currenttguid);

    return tguid;
}

Creature* Transporter::AddNPCPassengerInInstance(uint32 entry, float x, float y, float z, float o, uint32 anim)
{
    MapMgr* map = GetMapMgr();

    CreatureProperties const* creature_properties = sMySQLStore.GetCreatureProperties(entry);
    if (creature_properties == nullptr || map == nullptr)
        return nullptr;

    float transporter_x = obj_movement_info.transporter_info.position.x + x;
    float transporter_y = obj_movement_info.transporter_info.position.y + y;
    float transporter_z = obj_movement_info.transporter_info.position.z + z;

    Creature* pCreature = map->CreateCreature(entry);
    pCreature->Create(map->GetMapId(), transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->Load(creature_properties, transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->AddToWorld(map);
    pCreature->SetUnitMovementFlags(MOVEFLAG_TRANSPORT);
    pCreature->obj_movement_info.transporter_info.position.x = x;
    pCreature->obj_movement_info.transporter_info.position.y = y;
    pCreature->obj_movement_info.transporter_info.position.z = z;
    pCreature->obj_movement_info.transporter_info.position.o = o;
    pCreature->obj_movement_info.transporter_info.guid = GetGUID();

    pCreature->m_transportData.transportGuid = this->GetGUID();
    pCreature->m_transportData.relativePosition.x = x;
    pCreature->m_transportData.relativePosition.y = y;
    pCreature->m_transportData.relativePosition.z = z;
    pCreature->m_transportData.relativePosition.o = o;
    m_creatureSetMutex.Acquire();
    m_NPCPassengerSet.insert(pCreature);
    m_creatureSetMutex.Release();
    return pCreature;
}

void Transporter::UpdateNPCPositions(float x, float y, float z, float o)
{
    m_creatureSetMutex.Acquire();
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* npc = *itr;
        npc->SetPosition(x + npc->obj_movement_info.transporter_info.position.x, y + npc->obj_movement_info.transporter_info.position.y, z + npc->obj_movement_info.transporter_info.position.z, o + npc->obj_movement_info.transporter_info.position.o, false);
    }
    m_creatureSetMutex.Release();
}

void Transporter::UpdatePlayerPositions(float x, float y, float z, float o)
{
    for (auto playerGuid : m_passengers)
    {
        if (auto player = objmgr.GetPlayer(playerGuid))
        {
            player->SetPosition(
                x + player->obj_movement_info.transporter_info.position.x,
                y + player->obj_movement_info.transporter_info.position.y,
                z + player->obj_movement_info.transporter_info.position.z,
                o + player->obj_movement_info.transporter_info.position.o);
        }
    }
}
