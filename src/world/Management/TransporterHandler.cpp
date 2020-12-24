/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Server/Packets/SmsgTransferPending.h"

using namespace AscEmu::Packets;

TransportTemplate::~TransportTemplate()
{
}

TransportHandler::TransportHandler()
{
}

TransportHandler::~TransportHandler()
{
}

TransportHandler& TransportHandler::getInstance()
{
    static TransportHandler mInstance;
    return mInstance;
}

void TransportHandler::Unload()
{
    _transportTemplates.clear();
}

void TransportHandler::LoadTransportTemplates()
{
    LogNotice("TransportHandler : Start Loading TransportTemplates...");
    {
        uint32_t createCount = 0;

        for (auto it : sMySQLStore._transportEntryStore)
        {
            uint32 entry = it.first;
            GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(entry);
            if (gameobject_info == nullptr)
            {
                LOG_ERROR("Transport %u has no associated GameObjectProperties from `gameobject_properities` , skipped.", entry);
                continue;
            }

            // paths are generated per template, saves us from generating it again in case of instanced transports
            TransportTemplate& transport = _transportTemplates[entry];
            transport.entry = entry;
            GeneratePath(gameobject_info, &transport);

            // transports in instance are only on one map
            if (transport.inInstance)
                _instanceTransports[*transport.mapsUsed.begin()].insert(entry);

            ++createCount;
        }

        LogDetail("Transporter Handler : Loaded %u transport templates", createCount);
    }
}

void TransportHandler::SpawnContinentTransports()
{
    if (_transportTemplates.empty())
        return;

    LogNotice("TransportHandler : Start Spawning Continent Transports...");
    {
        uint32_t createCount = 0;

        for (auto it : sMySQLStore._transportDataStore)
        {
            uint32 entry = it.first;
            if (TransportTemplate const* tInfo = GetTransportTemplate(entry))
                if (!tInfo->inInstance)
                    if (auto trans = CreateTransport(entry))
                        ++createCount;
        }

        LogDetail("Transporter Handler : Spawned %u Continent Transports", createCount);
    }
}

Transporter* TransportHandler::CreateTransport(uint32 entry, MapMgr* map /*= nullptr*/)
{
    TransportTemplate const* tInfo = GetTransportTemplate(entry);
    if (!tInfo)
    {
        LogError("Transport %u will not be loaded, `transport_template` missing", entry);
        return nullptr;
    }

    // create transport...
    Transporter* trans = new Transporter((uint64)HIGHGUID_TYPE_TRANSPORTER << 32 | entry);

    // ...at first waypoint
    PathNode startNode = tInfo->keyFrames.begin()->Node;
    uint32 mapId = startNode.mapid;
    float x = startNode.x;
    float y = startNode.y;
    float z = startNode.z;
    float o = tInfo->keyFrames.begin()->InitialOrientation;

    if (!trans->Create(entry, mapId, x, y, z, o, 255))
    {
        delete trans;
        return nullptr;
    }

    // Storage
    _Transports.insert(trans);
    AddTransport(trans);

    if (DBC::Structures::MapEntry const* mapEntry = sMapStore.LookupEntry(mapId))
    {
        if (mapEntry->Instanceable() != tInfo->inInstance)
        {
            LogError("Transport %u attempted creation in instance map (id: %u) but it is not an instanced transport!", entry, mapId);
            delete trans;
            return nullptr;
        }
    }

    // Instance Case
    if (map)
        trans->AddToWorld(map);
    else
        trans->AddToWorld();

    trans->GetMapMgr()->AddToMapMgr(trans);

    // Load Creatures
    trans->LoadStaticPassengers();

    return trans;
}

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
            Path[i].flags = pathnode->flags;
            Path[i].delay = pathnode->waittime;
            Path[i].ArrivalEventID = pathnode->arivalEventID;
            Path[i].DepartureEventID = pathnode->departureEventID;
            ++i;
        }
    }

    Path.Resize(i);
    return (i > 0 ? true : false);
}

void TransportHandler::GeneratePath(GameObjectProperties const* goInfo, TransportTemplate* transport)
{
    uint32 pathId = goInfo->mo_transport.taxi_path_id;
    TransportPath path;
    FillTransporterPathVector(pathId, path);
    std::vector<KeyFrame>& keyFrames = transport->keyFrames;
    bool mapChange = false;

    for (size_t i = 0; i < path.Size(); ++i)
    {
        if (!mapChange)
        {
            PathNode node_i = path[i];
            if (i != path.Size() - 1 && (node_i.flags & 1 || node_i.mapid != path[i + 1].mapid))
            {
                keyFrames.back().Teleport = true;
                mapChange = true;
            }
            else
            {
                KeyFrame k(node_i);
                G3D::Vector3 h;
                k.InitialOrientation = NormalizeOrientation(std::atan2(h.y, h.x) + float(M_PI));

                keyFrames.push_back(k);
                transport->mapsUsed.insert(k.Node.mapid);
            }
        }
        else
            mapChange = false;
    }

    ASSERT(!keyFrames.empty());

    if (transport->mapsUsed.size() > 1)
    {
        for (std::set<uint32>::const_iterator itr = transport->mapsUsed.begin(); itr != transport->mapsUsed.end(); ++itr)
            ASSERT(!sMapStore.LookupEntry(*itr)->Instanceable());

        transport->inInstance = false;
    }
    else
        transport->inInstance = sMapStore.LookupEntry(*transport->mapsUsed.begin())->Instanceable();

    // last to first is always "teleport", even for closed paths
    keyFrames.back().Teleport = true;

    const float speed = float(goInfo->mo_transport.move_speed);
    const float accel = float(goInfo->mo_transport.accel_rate);
    const float accel_dist = 0.5f * speed * speed / accel;

    transport->accelTime = speed / accel;
    transport->accelDist = accel_dist;

    int32 firstStop = -1;
    int32 lastStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].DistFromPrev = 0;
    keyFrames[0].Index = 1;
    if (keyFrames[0].IsStopFrame())
    {
        lastStop = 0;
        firstStop = 0;
    }

    // find the rest of the distances between key points
    size_t start = 0;
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
       
    }

    transport->pathTime = keyFrames.back().DepartureTime;
}

float TransportHandler::NormalizeOrientation(float o)
{
    if (o < 0)
    {
        float mod = o * -1;
        mod = std::fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }
    return std::fmod(o, 2.0f * static_cast<float>(M_PI));
}

Transporter* TransportHandler::GetTransporter(uint32 guid)
{
    Transporter* rv = nullptr;
    _TransportLock.Acquire();
    std::unordered_map<uint32, Transporter*>::const_iterator itr = _Transporters.find(guid);
    rv = (itr != _Transporters.end()) ? itr->second : nullptr;
    _TransportLock.Release();
    return rv;
}

void TransportHandler::AddTransport(Transporter* transport)
{
    _TransportLock.Acquire();
    _Transporters[transport->GetUIdFromGUID()] = transport;
    _TransportLock.Release();
}