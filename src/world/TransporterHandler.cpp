/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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
    for (uint32 j = 0; j < sTaxiPathNodeStore.GetNumRows(); j++)
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
    auto gameobject_info = GameObjectNameStorage.LookupEntry(goEntry);

    if (!gameobject_info)
    {
        Log.Error("Transport Handler", "Transport ID:%u, will not be loaded, gameobject_names missing", goEntry);
        return NULL;
    }

    if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
    {
        Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_names type wrong", goEntry, gameobject_info->name);
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
    for (Transporter::CreatureSet::iterator itr = t->m_NPCPassengerSet.begin(); itr != t->m_NPCPassengerSet.end();)
    {
        if (Creature* npc = *itr)
        {
            npc->SetTransport(NULL);
            npc->RemoveFromWorld(true);
        }
        ++itr;
    }

    t->m_NPCPassengerSet.clear();
    m_TransportersByInstanceIdMap[t->GetInstanceID()].erase(t);
    m_Transporters.erase(t);
    t->m_WayPoints.clear();
    t->RemoveFromWorld(true);
}

void ObjectMgr::LoadTransports()
{
    uint32 oldMSTime = getMSTime();

    QueryResult* result = WorldDatabase.Query("SELECT entry, name, period FROM transport_data");

    if (!result)
    {
        Log.Error("Transporter Handler", ">> Loaded 0 transports. DB table `transport_data` is empty!");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        std::string name = fields[1].GetString();
        uint32 period = fields[2].GetUInt32();

        auto gameobject_info = GameObjectNameStorage.LookupEntry(entry);

        if (!gameobject_info)
        {
            Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_names missing", entry, name.c_str());
            continue;
        }

        if (gameobject_info->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            Log.Error("Transporter Handler", "Transport ID:%u, Name: %s, will not be loaded, gameobject_names type wrong", entry, name.c_str());
            continue;
        }

        std::set<uint32> mapsUsed;

        Transporter* pTransporter = new Transporter((uint64)HIGHGUID_TYPE_TRANSPORTER << 32 | entry);
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

        ++count;
    } 
    while (result->NextRow());

    delete result;

    Log.Success("Transporter Handler", ">> Loaded %u transports in %u ms", count, getMSTime() - oldMSTime);
}

void ObjectMgr::LoadTransportNPCs()
{
    uint32 oldMSTime = getMSTime();

    QueryResult* result = WorldDatabase.Query("SELECT guid, npc_entry, transport_entry, TransOffsetX, TransOffsetY, TransOffsetZ, TransOffsetO, emote FROM creature_transport");

    if (!result)
    {
        Log.Error("Transport Handler", ">> Loaded 0 transport NPCs. DB table `creature_transport` is empty!");
        return;
    }

    uint32 count = 0;

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
                (*itr)->AddNPCPassenger(guid, entry, tX, tY, tZ, tO, anim);
                break;
            }
        }

        ++count;
    } while (result->NextRow());

    Log.Success("Transport Handler", ">> Loaded %u Transport Npcs in %u ms", count, getMSTime() - oldMSTime);
}

Transporter::Transporter(uint64 guid) : GameObject(guid)
{
    m_pathTime = 0;
    m_timer = 0;
    m_period = 0;
}

Transporter::~Transporter()
{
    sEventMgr.RemoveEvents(this);

    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* passenger = *itr;
        MapMgr* map = passenger->GetMapMgr();
        passenger->SetTransport(NULL);
    }

    m_NPCPassengerSet.clear();
    m_WayPoints.clear();
    m_passengers.clear();
}

void Transporter::OnPushToWorld()
{
    // Create waypoint event
    sEventMgr.AddEvent(this, &Transporter::Update, EVENT_TRANSPORTER_NEXT_WAYPOINT, 100, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

bool Transporter::Create(uint32 entry, int32 Time)
{
    // Lookup GameobjectInfo
    if (!CreateFromProto(entry, 0, 0, 0, 0, 0))
        return false;

    // Override these flags to avoid mistakes in proto
    SetFlags(40);
    SetAnimProgress(100);

    auto gameobject_info = GameObjectNameStorage.LookupEntry(entry);

    pInfo = gameobject_info;

    if (pInfo)
        pInfo->type = GAMEOBJECT_TYPE_MO_TRANSPORT;
    else
        LOG_ERROR("Transporter id[%i] - can't set GAMEOBJECT_TYPE - it will behave badly!", entry);

    m_overrides = GAMEOBJECT_INFVIS | GAMEOBJECT_ONMOVEWIDE; //Make it forever visible on the same map

    // Set period
    m_period = Time;

    // Generate waypoints
    if (!GenerateWaypoints(pInfo->raw.parameter_0))
        return false;

    // Set position
    SetMapId(m_WayPoints[0].mapid);
    SetPosition(m_WayPoints[0].x, m_WayPoints[0].y, m_WayPoints[0].z, 0);
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
    for (int i = 1; i < (int)path.Size() - 1; i++)
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
            mapChange--;
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
    for (size_t i = 1; i < keyFrames.size(); i++)
    {
        if ((keyFrames[i - 1].actionflag == 1) || (keyFrames[i].mapid != keyFrames[i - 1].mapid))
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
            if (firstStop < 0)
                firstStop = (int)i;

            lastStop = (int)i;
        }
    }

    float tmpDist = 0;
    for (int i = 0; i < (int)keyFrames.size(); i++)
    {
        int j = (i + lastStop) % (int)keyFrames.size();
        if (keyFrames[j].actionflag == 2)
            tmpDist = 0;
        else
            tmpDist += keyFrames[j].distFromPrev;
        keyFrames[j].distSinceStop = tmpDist;
    }

    for (int i = int(keyFrames.size()) - 1; i >= 0; i--)
    {
        int j = (i + (firstStop + 1)) % (int)keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].distFromPrev;
        keyFrames[j].distUntilStop = tmpDist;
        if (keyFrames[j].actionflag == 2)
            tmpDist = 0;
    }

    for (size_t i = 0; i < keyFrames.size(); i++)
    {
        if (keyFrames[i].distSinceStop < (30 * 30 * 0.5))
            keyFrames[i].tFrom = sqrt(2 * keyFrames[i].distSinceStop);
        else
            keyFrames[i].tFrom = ((keyFrames[i].distSinceStop - (30 * 30 * 0.5f)) / 30) + 30;

        if (keyFrames[i].distUntilStop < (30 * 30 * 0.5))
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
    uint32 last_t = 0;
    m_WayPoints[0] = pos;
    t += keyFrames[0].delay * 1000;

    int cM = keyFrames[0].mapid;
    for (size_t i = 0; i < keyFrames.size() - 1; i++)        //
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

                    teleport = false;
                    if ((int)keyFrames[i].mapid != cM)
                    {
                        teleport = true;
                        cM = keyFrames[i].mapid;
                    }

                    //                    sLog.outString("T: %d, D: %f, x: %f, y: %f, z: %f", t, d, newX, newY, newZ);
                    TWayPoint pos2(keyFrames[i].mapid, newX, newY, newZ, teleport);
                    if (teleport || ((t - last_t) >= 1000))
                    {
                        m_WayPoints[t] = pos2;
                        last_t = t;
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

        teleport = false;
        if ((keyFrames[i + 1].actionflag == 1) || (keyFrames[i + 1].mapid != keyFrames[i].mapid))
        {
            teleport = true;
            cM = keyFrames[i + 1].mapid;
        }

        TWayPoint pos2(keyFrames[i + 1].mapid, keyFrames[i + 1].x, keyFrames[i + 1].y, keyFrames[i + 1].z, teleport);

        //        sLog.outString("T: %d, x: %f, y: %f, z: %f, t:%d", t, pos.x, pos.y, pos.z, teleport);

        //if (teleport)
        //m_WayPoints[t] = pos;
        if (keyFrames[i + 1].delay > 5)
            pos2.delayed = true;

        m_WayPoints.insert(WaypointMap::value_type(t, pos2));
        last_t = t;

        t += keyFrames[i + 1].delay * 1000;
        //        sLog.outString("------");
    }

    uint32 timer = t;

    mCurrentWaypoint = m_WayPoints.begin();
    mNextWaypoint = GetNextWaypoint();
    m_pathTime = timer;
    m_timer = 0;
    return true;
}

WaypointIterator Transporter::GetNextWaypoint()
{
    WaypointIterator iter = mCurrentWaypoint;
    iter++;
    if (iter == m_WayPoints.end())
        iter = m_WayPoints.begin();
    return iter;
}

void Transporter::TeleportTransport(uint32 newMapid, uint32 oldmap, float x, float y, float z)
{
    sEventMgr.RemoveEvents(this, EVENT_TRANSPORTER_NEXT_WAYPOINT);

    if(m_passengers.size() > 0)
    {
        WorldPacket Pending(SMSG_TRANSFER_PENDING, 12);
        Pending << newMapid << GetEntry() << oldmap;

        WorldPacket NewWorld;
        LocationVector v;

        for (PlayerSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
        {
            Player* player = *itr;
            ++itr;

            v.x = x + player->movement_info.transporter_info.position.x;
            v.y = y + player->movement_info.transporter_info.position.y;
            v.z = z + player->movement_info.transporter_info.position.z;
            v.o = player->GetOrientation();

            if (newMapid == 530 && !player->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_01))
            {
                // player does not have BC content, repop at graveyard
                player->RepopAtGraveyard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
                continue;
            }

            if (newMapid == 571 && !player->GetSession()->HasFlag(ACCOUNT_FLAG_XPACK_02))
            {
                player->RepopAtGraveyard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId());
                continue;
            }

            player->GetSession()->SendPacket(&Pending);
            player->_Relocate(newMapid, v, false, true, 0);

            // Lucky bitch. Do it like on official.
            if (player->IsDead())
            {
                player->ResurrectPlayer();
                player->SetHealth(player->GetMaxHealth());
                player->SetPower(POWER_TYPE_MANA, player->GetMaxPower(POWER_TYPE_MANA));
            }
        }
    }

    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
            (*itr)->TeleportFar(newMapid, x, y, z, (*itr)->GetOrientation());

    // Set our position
    RemoveFromWorld(false);
    SetMapId(newMapid);
    SetPosition(x, y, z, m_position.o, false);
    AddToWorld();
}

bool Transporter::AddPassenger(Player* passenger)
{
    if (m_passengers.insert(passenger).second)
        Log.Debug("Transporter", "Player %s boarded transport %u.", passenger->GetName(), this->GetInfo()->entry);

    AddPlayer(static_cast<Player*>(passenger));

    return true;
}

bool Transporter::RemovePassenger(Player* passenger)
{
    if (m_passengers.erase(passenger))
        Log.Debug("Transporter", "Player %s removed from transport %u.", passenger->GetName(), this->GetInfo()->entry);

    RemovePlayer(static_cast<Player*>(passenger));

    return true;
}

uint32 Transporter::BuildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target)
{
    uint32 cnt = Object::BuildCreateUpdateBlockForPlayer(data, target);

    // add all the npcs to the packet
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* npc = *itr;
        cnt += npc->BuildCreateUpdateBlockForPlayer(data, target);
    }

    return cnt;
    return 0;
}

uint32 TimeStamp();
void Transporter::Update()
{
    if (m_WayPoints.size() <= 1)
        return;

    m_timer = getMSTime() % m_period;

    while (((m_timer - mCurrentWaypoint->first) % m_pathTime) >= ((mNextWaypoint->first - mCurrentWaypoint->first) % m_pathTime))
    {
        mCurrentWaypoint = mNextWaypoint;
        mNextWaypoint = GetNextWaypoint();
        
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
            switch (GetInfo()->display_id)
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
            TransportGossip(GetInfo()->display_id);
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

    CreatureInfo* inf = CreatureNameStorage.LookupEntry(entry);
    CreatureProto* proto = CreatureProtoStorage.LookupEntry(entry);
    if (inf == nullptr || proto == nullptr)
        return 0;

    float transporter_x = obj_movement_info.transporter_info.position.x + x;
    float transporter_y = obj_movement_info.transporter_info.position.y + y;
    float transporter_z = obj_movement_info.transporter_info.position.z + z;

    Creature* pCreature = GetMapMgr()->CreateCreature(entry);
    pCreature->Create(inf->Name, GetMapMgr()->GetMapId(), transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->Load(proto, transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->AddToWorld(map);
    pCreature->SetUnitMovementFlags(MOVEFLAG_TRANSPORT);
    pCreature->obj_movement_info.transporter_info.position.x = x;
    pCreature->obj_movement_info.transporter_info.position.y = y;
    pCreature->obj_movement_info.transporter_info.position.z = z;
    pCreature->obj_movement_info.transporter_info.position.o = o;
    pCreature->obj_movement_info.transporter_info.guid = GetGUID();
    if (anim)
        pCreature->SetUInt32Value(UNIT_NPC_EMOTESTATE, anim);

    pCreature->SetTransport(this);
    m_NPCPassengerSet.insert(pCreature);

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

    CreatureInfo* inf = CreatureNameStorage.LookupEntry(entry);
    CreatureProto* proto = CreatureProtoStorage.LookupEntry(entry);
    if (inf == NULL || proto == NULL)
        return NULL;

    float transporter_x = obj_movement_info.transporter_info.position.x + x;
    float transporter_y = obj_movement_info.transporter_info.position.y + y;
    float transporter_z = obj_movement_info.transporter_info.position.z + z;

    Creature* pCreature = GetMapMgr()->CreateCreature(entry);
    pCreature->Create(inf->Name, GetMapMgr()->GetMapId(), transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->Load(proto, transporter_x, transporter_y, transporter_z, (std::atan2(transporter_x, transporter_y) + float(M_PI)) + o);
    pCreature->AddToWorld(map);
    pCreature->SetUnitMovementFlags(MOVEFLAG_TRANSPORT);
    pCreature->obj_movement_info.transporter_info.position.x = x;
    pCreature->obj_movement_info.transporter_info.position.y = y;
    pCreature->obj_movement_info.transporter_info.position.z = z;
    pCreature->obj_movement_info.transporter_info.position.o = o;
    pCreature->obj_movement_info.transporter_info.guid = GetGUID();

    pCreature->SetTransport(this);
    m_NPCPassengerSet.insert(pCreature);

    return pCreature;
}

void Transporter::UpdateNPCPositions(float x, float y, float z, float o)
{
    for (CreatureSet::iterator itr = m_NPCPassengerSet.begin(); itr != m_NPCPassengerSet.end(); ++itr)
    {
        Creature* npc = *itr;
        npc->SetPosition(x + npc->obj_movement_info.transporter_info.position.x, y + npc->obj_movement_info.transporter_info.position.y, z + npc->obj_movement_info.transporter_info.position.z, o + npc->obj_movement_info.transporter_info.position.o, false);
    }
}

void Transporter::UpdatePlayerPositions(float x, float y, float z, float o)
{
    for (PlayerSet::iterator itr = m_passengers.begin(); itr != m_passengers.end(); ++itr)
    {
        Player* plr = *itr;
        plr->SetPosition(x + plr->obj_movement_info.transporter_info.position.x, y + plr->obj_movement_info.transporter_info.position.y, z + plr->obj_movement_info.transporter_info.position.z, o + plr->obj_movement_info.transporter_info.position.o, false);

    }
}
