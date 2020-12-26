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
    if (map)
        _TransportersByInstanceIdMap[map->GetInstanceID()].insert(trans);
    else
        _TransportersByInstanceIdMap[trans->GetInstanceID()].insert(trans);

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

void TransportHandler::LoadTransportForPlayers(Player* player)
{
    auto tset = _TransportersByInstanceIdMap[player->GetInstanceID()];
    ByteBuffer transData(500);
    uint32 count = 0;

    if (tset.size() == 0)
        return;

    for (auto i = tset.begin(); i != tset.end(); ++i)
        count = (*i)->buildCreateUpdateBlockForPlayer(&transData, player);

    player->getUpdateMgr().pushCreationData(&transData, count);
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
                k.InitialOrientation = NormalizeOrientation(std::atan2(node_i.y, node_i.x) + float(M_PI));

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
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if ((keyFrames[i].IsTeleportFrame()) || (keyFrames[i].Node.mapid != keyFrames[i - 1].Node.mapid))
        {
            keyFrames[i].Index = i + 1;
            keyFrames[i].DistFromPrev = 0;
        }
        else
        {
            keyFrames[i].Index = i + 1;
            keyFrames[i].DistFromPrev =
                std::sqrt(pow(keyFrames[i].Node.x - keyFrames[i - 1].Node.x, 2) +
                    pow(keyFrames[i].Node.y - keyFrames[i - 1].Node.y, 2) +
                    pow(keyFrames[i].Node.z - keyFrames[i - 1].Node.z, 2));
            if (i > 0)
                keyFrames[i - 1].NextDistFromPrev = keyFrames[i].DistFromPrev;
        }

        if (keyFrames[i].IsStopFrame())
        {
            // remember first stop frame
            if (firstStop == -1)
                firstStop = i;

            lastStop = i;
        }
    }

    keyFrames.back().NextDistFromPrev = keyFrames.front().DistFromPrev;

    if (firstStop == -1 || lastStop == -1)
        firstStop = lastStop = 0;

    // at stopping keyframes, we define distSinceStop == 0,
    // and distUntilStop is to the next stopping keyframe.
    // this is required to properly handle cases of two stopping frames in a row (yes they do exist)
    float tmpDist = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame() || j == lastStop)
            tmpDist = 0.0f;
        else
            tmpDist += keyFrames[j].DistFromPrev;
        keyFrames[j].DistSinceStop = tmpDist;
    }

    tmpDist = 0.0f;
    for (int32 i = int32(keyFrames.size()) - 1; i >= 0; i--)
    {
        int32 j = (i + firstStop) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].DistFromPrev;
        keyFrames[j].DistUntilStop = tmpDist;
        if (keyFrames[j].IsStopFrame() || j == firstStop)
            tmpDist = 0.0f;
    }

    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        float total_dist = keyFrames[i].DistSinceStop + keyFrames[i].DistUntilStop;
        if (total_dist < 2 * accel_dist) // won't reach full speed
        {
            if (keyFrames[i].DistSinceStop < keyFrames[i].DistUntilStop) // is still accelerating
            {
                // calculate accel+brake time for this short segment
                float segment_time = 2.0f * std::sqrt((keyFrames[i].DistUntilStop + keyFrames[i].DistSinceStop) / accel);
                // substract acceleration time
                keyFrames[i].TimeTo = segment_time - std::sqrt(2 * keyFrames[i].DistSinceStop / accel);
            }
            else // slowing down
                keyFrames[i].TimeTo = std::sqrt(2 * keyFrames[i].DistUntilStop / accel);
        }
        else if (keyFrames[i].DistSinceStop < accel_dist) // still accelerating (but will reach full speed)
        {
            // calculate accel + cruise + brake time for this long segment
            float segment_time = (keyFrames[i].DistUntilStop + keyFrames[i].DistSinceStop) / speed + (speed / accel);
            // substract acceleration time
            keyFrames[i].TimeTo = segment_time - std::sqrt(2 * keyFrames[i].DistSinceStop / accel);
        }
        else if (keyFrames[i].DistUntilStop < accel_dist) // already slowing down (but reached full speed)
            keyFrames[i].TimeTo = std::sqrt(2 * keyFrames[i].DistUntilStop / accel);
        else // at full speed
            keyFrames[i].TimeTo = (keyFrames[i].DistUntilStop / speed) + (0.5f * speed / accel);
    }

    // calculate tFrom times from tTo times
    float segmentTime = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame() || j == lastStop)
            segmentTime = keyFrames[j].TimeTo;
        keyFrames[j].TimeFrom = segmentTime - keyFrames[j].TimeTo;
    }

    // calculate path times
    keyFrames[0].ArriveTime = 0;
    float curPathTime = 0.0f;
    if (keyFrames[0].IsStopFrame())
    {
        curPathTime = float(keyFrames[0].Node.delay);
        keyFrames[0].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
    }

    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        curPathTime += keyFrames[i - 1].TimeTo;
        if (keyFrames[i].IsStopFrame())
        {
            keyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            curPathTime += float(keyFrames[i].Node.delay);
            keyFrames[i].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
        }
        else
        {
            curPathTime -= keyFrames[i].TimeTo;
            keyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            keyFrames[i].DepartureTime = keyFrames[i].ArriveTime;
        }
    }

    keyFrames.back().NextArriveTime = keyFrames.back().DepartureTime;

    transport->pathTime = keyFrames.back().DepartureTime;
    LogDebug("TransportHandler: total time %u at transport %u \n", transport->pathTime, transport->entry);
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
    auto itr = _Transporters.find(guid);
    rv = (itr != _Transporters.end()) ? itr->second : 0;
    _TransportLock.Release();
    return rv;
}

void TransportHandler::AddTransport(Transporter* transport)
{
    _TransportLock.Acquire();
    _Transporters[transport->GetUIdFromGUID()] = transport;
    _TransportLock.Release();
}

void TransportHandler::LoadTransportAnimationAndRotation()
{
    for (uint32 i = 0; i < sTransportAnimationStore.GetNumRows(); ++i)
        if (DBC::Structures::TransportAnimationEntry const* anim = sTransportAnimationStore.LookupEntry(i))
            AddPathNodeToTransport(anim->TransportID, anim->TimeIndex, anim);

    for (uint32 i = 0; i < sTransportRotationStore.GetNumRows(); ++i)
        if (DBC::Structures::TransportRotationEntry const* rot = sTransportRotationStore.LookupEntry(i))
            AddPathRotationToTransport(rot->GameObjectsID, rot->TimeIndex, rot);
}

void TransportHandler::AddPathNodeToTransport(uint32 transportEntry, uint32 timeSeg, DBC::Structures::TransportAnimationEntry const* node)
{
    TransportAnimation& animNode = _transportAnimations[transportEntry];
    if (animNode.TotalTime < timeSeg)
        animNode.TotalTime = timeSeg;

    animNode.Path[timeSeg] = node;
}

DBC::Structures::TransportAnimationEntry const* TransportAnimation::GetAnimNode(uint32 time) const
{
    auto itr = Path.lower_bound(time);
    if (itr != Path.end())
        return itr->second;

    return nullptr;
}

DBC::Structures::TransportRotationEntry const* TransportAnimation::GetAnimRotation(uint32 time) const
{
    auto itr = Rotations.lower_bound(time);
    if (itr != Rotations.end())
        return itr->second;

    return nullptr;
}