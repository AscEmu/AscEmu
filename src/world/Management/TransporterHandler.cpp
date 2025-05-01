/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include <Movement/Spline/Spline.h>
#include <Movement/Spline/MoveSplineInitArgs.h>

#include "Logging/Logger.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Server/Definitions.h"
#include "Objects/Transporter.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

TransportHandler& TransportHandler::getInstance()
{
    static TransportHandler mInstance;
    return mInstance;
}

void TransportHandler::unload()
{
    _transportTemplates.clear();
}

void TransportHandler::loadTransportTemplates()
{
    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "TransportHandler : Start Loading TransportTemplates...");

    uint32_t createCount = 0;

    for (auto& it : sMySQLStore._transportEntryStore)
    {
        uint32_t entry = it.first;

        GameObjectProperties const* gameobject_info = sMySQLStore.getGameObjectProperties(entry);
        if (gameobject_info == nullptr)
        {
            sLogger.failure("Transport {} has no associated GameObjectProperties from `gameobject_properities` , skipped.", entry);
            continue;
        }

        // paths are generated per template, saves us from generating it again in case of instanced transports
        TransportTemplate& transport = _transportTemplates[entry];
        transport.entry = entry;
        generatePath(gameobject_info, &transport);

        // transports in instance are only on one map
        if (transport.inInstance)
            _instanceTransports[*transport.mapsUsed.begin()].insert(entry);

        ++createCount;
    }

    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "Transporter Handler : Loaded {} transport templates", createCount);
}

void TransportHandler::spawnContinentTransports()
{
    if (_transportTemplates.empty())
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "TransportHandler : Start Spawning Continent Transports...");

    uint32_t createCount = 0;

    for (auto& it : sMySQLStore._transportDataStore)
    {
        uint32_t entry = it.first;
        if (TransportTemplate const* tInfo = getTransportTemplate(entry))
            if (!tInfo->inInstance)
                if (const auto trans = createTransport(entry))
                    ++createCount;
    }

    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "Transporter Handler : Spawned {} Continent Transports", createCount);
}

Transporter* TransportHandler::createTransport(uint32_t entry, WorldMap* map /*= nullptr*/)
{
    TransportTemplate const* tInfo = getTransportTemplate(entry);
    if (!tInfo)
    {
        sLogger.failure("Transport {} will not be loaded, `transport_template` missing", entry);
        return nullptr;
    }

    // create transport...
    Transporter* trans = new Transporter((uint64_t)HIGHGUID_TYPE_TRANSPORTER << 32 | entry);

    // ...at first waypoint
#if VERSION_STRING == Classic
    if (tInfo->keyFrames.size() == 0)
        return nullptr;

#endif
    PathNode startNode = tInfo->keyFrames.begin()->Node;
    uint32_t mapId = startNode.mapid;
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
        _TransportersByInstanceIdMap[map->getInstanceId()].insert(trans);
    else
        _TransportersByInstanceIdMap[trans->GetInstanceID()].insert(trans);

    addTransport(trans);

    if (const auto mapEntry = sMapStore.lookupEntry(mapId))
    {
        if (mapEntry->instanceable() != tInfo->inInstance)
        {
            sLogger.failure("Transport {} attempted creation in instance map (id: {}) but it is not an instanced transport!", entry, mapId);
            delete trans;
            return nullptr;
        }
    }

    // Instance Case
    if (map)
    {
        trans->AddToWorld(map);
        trans->SetInstanceID(map->getInstanceId());
    }      
    else
        trans->AddToWorld();

    trans->getWorldMap()->addToMapMgr(trans);

    // Load Creatures
    trans->LoadStaticPassengers();

    return trans;
}

void TransportHandler::loadTransportForPlayers(Player* player)
{
    auto& tset = _TransportersByInstanceIdMap[player->GetInstanceID()];
    ByteBuffer transData(500);
    uint32_t count = 0;

    if (tset.size() == 0)
        return;

    for (auto i = tset.begin(); i != tset.end(); ++i)
        count = (*i)->buildCreateUpdateBlockForPlayer(&transData, player);

    player->getUpdateMgr().pushCreationData(&transData, count);
}

void TransportHandler::removeInstancedTransport(Transporter* transport, uint32_t instanceID)
{
    auto itr = _TransportersByInstanceIdMap[instanceID].find(transport);
    if (itr != _TransportersByInstanceIdMap[instanceID].end())
        _TransportersByInstanceIdMap[instanceID].erase(transport);
}

bool FillTransporterPathVector(uint32_t PathID, TransportPath & Path)
{
    // Store dbc values into current Path array
    Path.resize(sTaxiPathNodeStore.getNumRows());

    uint32_t i = 0;
    for (uint32_t j = 0; j < sTaxiPathNodeStore.getNumRows(); ++j)
    {
        auto pathnode = sTaxiPathNodeStore.lookupEntry(j);
        if (pathnode == nullptr)
            continue;

        if (pathnode->pathId == PathID)
        {
            Path[i].mapid = pathnode->mapid;
            Path[i].x = pathnode->x;
            Path[i].y = pathnode->y;
            Path[i].z = pathnode->z;
            Path[i].flags = pathnode->flags;
            Path[i].delay = pathnode->waittime;
#if VERSION_STRING > Classic
            Path[i].ArrivalEventID = pathnode->arivalEventID;
            Path[i].DepartureEventID = pathnode->departureEventID;
#endif
            ++i;
        }
    }

    Path.resize(i);
    return (i > 0 ? true : false);
}

class SplineRawInitializer
{
public:
    SplineRawInitializer(MovementMgr::PointsArray& points) : _points(points) { }

    void operator()(uint8_t& mode, bool& cyclic, MovementMgr::PointsArray& points, int& lo, int& hi) const
    {
        mode = MovementMgr::SplineBase::ModeCatmullrom;
        cyclic = false;
        points.assign(_points.begin(), _points.end());
        lo = 1;
        hi = static_cast<int32_t>(points.size() - 2);
    }

    MovementMgr::PointsArray& _points;
};

void TransportHandler::generatePath(GameObjectProperties const* goInfo, TransportTemplate* transport)
{
    uint32_t pathId = goInfo->mo_transport.taxi_path_id;
    TransportPath path;
    FillTransporterPathVector(pathId, path);

    if (path.size() == 0)
        return;

    std::vector<KeyFrame>& keyFrames = transport->keyFrames;
    MovementMgr::PointsArray splinePath, allPoints;
    bool mapChange = false;

    for (uint16_t i = 0; i < path.size(); ++i)
        allPoints.emplace_back(G3D::Vector3(path[i].x, path[i].y, path[i].z));

    // Add extra points to allow derivative calculations for all path nodes
    allPoints.insert(allPoints.begin(), allPoints.front().lerp(allPoints[1], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -1.0f));

    SplineRawInitializer initer(allPoints);
    TransportSpline orientationSpline;
    orientationSpline.init_spline_custom(initer);
    orientationSpline.initLengths();

    for (uint16_t i = 0; i < path.size(); ++i)
    {
        if (!mapChange)
        {
            PathNode node_i = path[i];
            if (i != path.size() - 1 && (node_i.flags & 1 || node_i.mapid != path[i + 1].mapid))
            {
                keyFrames.back().Teleport = true;
                mapChange = true;
            }
            else
            {
                KeyFrame k(node_i);
                G3D::Vector3 h;
                orientationSpline.evaluate_derivative(i + 1, 0.0f, h);
                k.InitialOrientation = normalizeOrientation(std::atan2(h.y, h.x) + float(M_PI));

                keyFrames.push_back(k);
                splinePath.emplace_back(G3D::Vector3(node_i.x, node_i.y, node_i.z));
                transport->mapsUsed.insert(k.Node.mapid);
            }
        }
        else
        {
            mapChange = false;
        }
    }

    // to calculate proper splines we need at least 4 spline points
    if (splinePath.size() >= 2)
    {
        // Remove special catmull-rom spline points
        if (!keyFrames.front().isStopFrame() && !keyFrames.front().Node.ArrivalEventID && !keyFrames.front().Node.DepartureEventID)
        {
            splinePath.erase(splinePath.begin());
            keyFrames.erase(keyFrames.begin());
        }
        if (!keyFrames.back().isStopFrame() && !keyFrames.back().Node.ArrivalEventID && !keyFrames.back().Node.DepartureEventID)
        {
            splinePath.pop_back();
            keyFrames.pop_back();
        }
    }

    if (keyFrames.empty())
    {
        sLogger.failure("TransportHandler::generatePath no keyFrames available for Transport {}", goInfo->entry);
        return;
    }

    if (transport->mapsUsed.size() > 1)
    {
        for (std::set<uint32_t>::const_iterator itr = transport->mapsUsed.begin(); itr != transport->mapsUsed.end(); ++itr)
        {
            if (const auto map = sMapStore.lookupEntry(*itr))
            {
                if (map->instanceable())
                {
                    sLogger.failure("TransportHandler::generatePath not allowed to create a path to a instance map!");
                    return;
                }
            }
        }

        transport->inInstance = false;
    }
    else
    {
        if (const auto map = sMapStore.lookupEntry(*transport->mapsUsed.begin()))
            transport->inInstance = map->instanceable();
    }

    // last to first is always "teleport", even for closed paths
    keyFrames.back().Teleport = true;

    const float speed = float(goInfo->mo_transport.move_speed);
    const float accel = float(goInfo->mo_transport.accel_rate);
    const float accel_dist = 0.5f * speed * speed / accel;

    transport->accelTime = speed / accel;
    transport->accelDist = accel_dist;

    int32_t firstStop = -1;
    int32_t lastStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].DistFromPrev = 0;
    keyFrames[0].Index = 1;
    if (keyFrames[0].isStopFrame())
    {
        lastStop = 0;
        firstStop = 0;
    }

    // find the rest of the distances between key points
    // Every path segment has its own spline
    uint16_t start = 0;
    for (uint16_t i = 1; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i - 1].Teleport || i + 1 == keyFrames.size())
        {
            auto extra = !keyFrames[i - 1].Teleport ? 1 : 0;
            std::shared_ptr<TransportSpline> spline = std::make_shared<TransportSpline>();
            spline->init_spline(&splinePath[start], i - start + extra, MovementMgr::SplineBase::ModeCatmullrom);
            spline->initLengths();
            for (auto j = start; j < i + extra; ++j)
            {
                keyFrames[j].Index = j - start + 1;
                keyFrames[j].DistFromPrev = float(spline->length(j - start, j + 1 - start));
                if (j > 0)
                    keyFrames[j - 1].NextDistFromPrev = keyFrames[j].DistFromPrev;
                keyFrames[j].Spline = spline;
            }

            if (keyFrames[i - 1].Teleport)
            {
                keyFrames[i].Index = i - start + 1;
                keyFrames[i].DistFromPrev = 0.0f;
                keyFrames[i - 1].NextDistFromPrev = 0.0f;
                keyFrames[i].Spline = spline;
            }

            start = i;
        }

        if (keyFrames[i].isStopFrame())
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
        auto j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].isStopFrame() || j == lastStop)
            tmpDist = 0.0f;
        else
            tmpDist += keyFrames[j].DistFromPrev;
        keyFrames[j].DistSinceStop = tmpDist;
    }

    tmpDist = 0.0f;
    for (int32_t i = static_cast<int32_t>(keyFrames.size()) - 1; i >= 0; i--)
    {
        int32_t j = (i + firstStop) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].DistFromPrev;
        keyFrames[j].DistUntilStop = tmpDist;
        if (keyFrames[j].isStopFrame() || j == firstStop)
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
            {
                keyFrames[i].TimeTo = std::sqrt(2 * keyFrames[i].DistUntilStop / accel);
            }
        }
        else if (keyFrames[i].DistSinceStop < accel_dist) // still accelerating (but will reach full speed)
        {
            // calculate accel + cruise + brake time for this long segment
            float segment_time = (keyFrames[i].DistUntilStop + keyFrames[i].DistSinceStop) / speed + (speed / accel);
            // substract acceleration time
            keyFrames[i].TimeTo = segment_time - std::sqrt(2 * keyFrames[i].DistSinceStop / accel);
        }
        else if (keyFrames[i].DistUntilStop < accel_dist) // already slowing down (but reached full speed)
        {
            keyFrames[i].TimeTo = std::sqrt(2 * keyFrames[i].DistUntilStop / accel);
        }
        else // at full speed
        {
            keyFrames[i].TimeTo = (keyFrames[i].DistUntilStop / speed) + (0.5f * speed / accel);
        }
    }

    // calculate tFrom times from tTo times
    float segmentTime = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        auto j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].isStopFrame() || j == lastStop)
            segmentTime = keyFrames[j].TimeTo;
        keyFrames[j].TimeFrom = segmentTime - keyFrames[j].TimeTo;
    }

    // calculate path times
    keyFrames[0].ArriveTime = 0;
    float curPathTime = 0.0f;
    if (keyFrames[0].isStopFrame())
    {
        curPathTime = float(keyFrames[0].Node.delay);
        keyFrames[0].DepartureTime = uint32_t(curPathTime * static_cast<float>(IN_MILLISECONDS));
    }

    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        curPathTime += keyFrames[i - 1].TimeTo;
        if (keyFrames[i].isStopFrame())
        {
            keyFrames[i].ArriveTime = uint32_t(curPathTime * static_cast<float>(IN_MILLISECONDS));
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            curPathTime += float(keyFrames[i].Node.delay);
            keyFrames[i].DepartureTime = uint32_t(curPathTime * static_cast<float>(IN_MILLISECONDS));
        }
        else
        {
            curPathTime -= keyFrames[i].TimeTo;
            keyFrames[i].ArriveTime = uint32_t(curPathTime * static_cast<float>(IN_MILLISECONDS));
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            keyFrames[i].DepartureTime = keyFrames[i].ArriveTime;
        }
    }

    keyFrames.back().NextArriveTime = keyFrames.back().DepartureTime;

    transport->pathTime = keyFrames.back().DepartureTime;
    sLogger.debugFlag(AscEmu::Logging::LF_MAP, "TransportHandler: total time {} at transport {} \n", transport->pathTime, transport->entry);
}

float TransportHandler::normalizeOrientation(float o)
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

Transporter* TransportHandler::getTransporter(uint32_t guid)
{
    std::lock_guard lock(_TransportLock);

    Transporter* rv = nullptr;
    auto itr = _Transporters.find(guid);
    rv = (itr != _Transporters.end()) ? itr->second : 0;
    return rv;
}

TransportTemplate const* TransportHandler::getTransportTemplate(uint32_t entry) const
{
    TransportTemplates::const_iterator itr = _transportTemplates.find(entry);
    if (itr != _transportTemplates.end())
        return &itr->second;

    return nullptr;
}

TransportAnimation const* TransportHandler::getTransportAnimInfo(uint32_t entry) const
{
    TransportAnimationContainer::const_iterator itr = _transportAnimations.find(entry);
    if (itr != _transportAnimations.end())
        return &itr->second;

    return nullptr;
}

void TransportHandler::addTransport(Transporter* transport)
{
    std::lock_guard lock(_TransportLock);

    _Transporters[transport->GetUIdFromGUID()] = transport;
}

void TransportHandler::loadTransportAnimationAndRotation()
{
    for (uint32_t i = 0; i < sTransportAnimationStore.getNumRows(); ++i)
        if (const auto anim = sTransportAnimationStore.lookupEntry(i))
            addPathNodeToTransport(anim->TransportID, anim->TimeIndex, anim);

#if VERSION_STRING >= WotLK
    for (uint32_t i = 0; i < sTransportRotationStore.getNumRows(); ++i)
        if (const auto rot = sTransportRotationStore.lookupEntry(i))
            addPathRotationToTransport(rot->GameObjectsID, rot->TimeIndex, rot);
#endif
}

void TransportHandler::addPathNodeToTransport(uint32_t transportEntry, uint32_t timeSeg, WDB::Structures::TransportAnimationEntry const* node)
{
    TransportAnimation& animNode = _transportAnimations[transportEntry];
    if (animNode.TotalTime < timeSeg)
        animNode.TotalTime = timeSeg;

    animNode.Path[timeSeg] = node;
}

#if VERSION_STRING >= WotLK
void TransportHandler::addPathRotationToTransport(uint32_t transportEntry, uint32_t timeSeg, WDB::Structures::TransportRotationEntry const* node)
{
    _transportAnimations[transportEntry].Rotations[timeSeg] = node;
}
#endif

WDB::Structures::TransportAnimationEntry const* TransportAnimation::getAnimNode(uint32_t time) const
{
    auto itr = Path.lower_bound(time);
    if (itr != Path.end())
        return itr->second;

    return nullptr;
}

#if VERSION_STRING >= WotLK
WDB::Structures::TransportRotationEntry const* TransportAnimation::getAnimRotation(uint32_t time) const
{
    auto itr = Rotations.lower_bound(time);
    if (itr != Rotations.end())
        return itr->second;

    return nullptr;
}
#endif
