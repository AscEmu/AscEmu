/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "FlightPathMovementGenerator.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Debugging/Errors.h"
#include "Map/Management/MapMgr.hpp"
#include "Movement/MovementDefines.h"
#include "Movement/Spline/MoveSpline.h"
#include "Movement/Spline/MoveSplineInit.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Transporter.hpp"
#include "Movement/WaypointManager.h"
#include "Objects/Units/Creatures/AIInterface.h"

uint32_t FlightPathMovementGenerator::getPathAtMapEnd() const
{
    if (_currentNode >= _path.size())
        return _path.size();

    uint32_t curMapId = _path[_currentNode]->mapid;
    for (uint32_t i = _currentNode; i < _path.size(); ++i)
        if (_path[i]->mapid != curMapId)
            return i;

    return _path.size();
}

bool isNodeIncludedInShortenedPath(WDB::Structures::TaxiPathNodeEntry const* path1, WDB::Structures::TaxiPathNodeEntry const* path2)
{
    return path1->mapid != path2->mapid || std::pow(path1->x - path2->x, 2) + std::pow(path1->y - path2->y, 2) > (40.0f * 40.0f);
}

void FlightPathMovementGenerator::loadPath(Player* player)
{
    _pointsForPathSwitch.clear();
    std::deque<uint32_t> const& taxi = player->m_taxi->getPath();
    for (uint32 src = 0, dst = 1; dst < taxi.size(); src = dst++)
    {
        uint32_t path, cost;
        sTaxiMgr.getTaxiPath(taxi[src], taxi[dst], path, cost);
        if (path > sTaxiPathNodesByPath.size())
            return;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[path];
        if (!nodes.empty())
        {
            WDB::Structures::TaxiPathNodeEntry const* start = nodes[0];
            WDB::Structures::TaxiPathNodeEntry const* end = nodes[nodes.size() - 1];
            bool passedPreviousSegmentProximityCheck = false;
            for (uint32 i = 0; i < nodes.size(); ++i)
            {
                if (passedPreviousSegmentProximityCheck || !src || _path.empty() || isNodeIncludedInShortenedPath(_path[_path.size() - 1], nodes[i]))
                {
                    if ((!src || (isNodeIncludedInShortenedPath(start, nodes[i]) && i >= 2)) &&
                        (dst == taxi.size() - 1 || (isNodeIncludedInShortenedPath(end, nodes[i]) && i < nodes.size() - 1)))
                    {
                        passedPreviousSegmentProximityCheck = true;
                        _path.push_back(nodes[i]);
                    }
                }
                else
                {
                    _path.pop_back();
                    --_pointsForPathSwitch.back().PathIndex;
                }
            }
        }

        _pointsForPathSwitch.push_back({ uint32_t(_path.size() - 1), int32_t(cost) });
    }
}

void FlightPathMovementGenerator::doInitialize(Player* player)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    addFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED);

    doReset(player);
    initEndGridInfo();
}

void FlightPathMovementGenerator::doDeactivate(Player* /*owner*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
}

void FlightPathMovementGenerator::doFinalize(Player* player, bool active, bool /*movementInform*/)
{
    addFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (!active)
        return;

    // remove flag to prevent send object build movement packets for flight state
    player->removeUnitStateFlag(UNIT_STATE_IN_FLIGHT);

    if (!player->m_taxi->nodeAfterTeleport)
    {
        player->cleanupAfterTaxiFlight();
    }
    else
    {
        player->dismount();
        player->removeUnitFlags(UNIT_FLAG_LOCK_PLAYER | UNIT_FLAG_MOUNTED_TAXI);
    }

    if (player->m_taxi->empty())
    {
        // update position and orientation for landing point
        // this prevent cheating with landing point at lags
        player->stopMoving();
        float mapHeight = player->getWorldMap()->getHeight(player->GetPhase(), LocationVector(_path[getCurrentNode()]->x, _path[getCurrentNode()]->y, _path[getCurrentNode()]->z));
        // When the player reaches the last flight point, teleport to destination
        player->safeTeleport(_path[getCurrentNode()]->mapid, 0, LocationVector(_path[getCurrentNode()]->x, _path[getCurrentNode()]->y, mapHeight, player->GetOrientation()));
    }

    player->removePlayerFlags(PLAYER_FLAG_UNK2);
}

void FlightPathMovementGenerator::doReset(Player* player)
{
    removeFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    player->addUnitStateFlag(UNIT_STATE_IN_FLIGHT);
    player->setUnitFlags(UNIT_FLAG_LOCK_PLAYER | UNIT_FLAG_MOUNTED_TAXI);

    MovementMgr::MoveSplineInit init(player);
    for (uint32_t i = getCurrentNode(); i != getPathAtMapEnd(); ++i)
    {
        G3D::Vector3 vertice(_path[i]->x, _path[i]->y, _path[i]->z);
        init.Path().push_back(vertice);
    }
    init.SetFirstPointId(getCurrentNode());
    init.SetFly();
#if VERSION_STRING > WotLK
    init.SetSmooth();
    init.SetUncompressed();
    init.SetWalk(true);
#endif
    init.SetVelocity(32.0f);
    init.Launch();
}

bool FlightPathMovementGenerator::doUpdate(Player* player, uint32_t /*diff*/)
{
    if (!player)
        return false;

    uint32_t pointId = (uint32_t)player->movespline->currentPathIdx();
    if (pointId > _currentNode)
    {
        bool departureEvent = true;
        do
        {
            doEventIfAny(player, _path[_currentNode], departureEvent);
            while (!_pointsForPathSwitch.empty() && _pointsForPathSwitch.front().PathIndex <= _currentNode)
            {
                _pointsForPathSwitch.pop_front();
                player->m_taxi->nextTaxiDestination();
                if (!_pointsForPathSwitch.empty())
                {
#if VERSION_STRING > TBC
                    player->updateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_GOLD_SPENT_FOR_TRAVELLING, _pointsForPathSwitch.front().Cost);
#endif
                    player->modCoinage(-_pointsForPathSwitch.front().Cost);
                }
            }

            if (pointId == _currentNode)
                break;

            if (_currentNode == _preloadTargetNode)
                preloadEndGrid();

            _currentNode += (uint32_t)departureEvent;
            departureEvent = !departureEvent;
        } while (true);
    }

    return _currentNode < (_path.size() - 1);
}

void FlightPathMovementGenerator::setCurrentNodeAfterTeleport()
{
    if (_path.empty() || _currentNode >= _path.size())
        return;

    uint32_t map0 = _path[_currentNode]->mapid;
    for (size_t i = _currentNode + 1; i < _path.size(); ++i)
    {
        if (_path[i]->mapid != map0)
        {
            _currentNode = i;
            return;
        }
    }
}

void FlightPathMovementGenerator::doEventIfAny(Player* player, WDB::Structures::TaxiPathNodeEntry const* node, bool departure)
{
#if VERSION_STRING >= TBC
    if (uint32 eventid = departure ? node->departureEventID : node->arivalEventID)
    {
        sLogger.debug("FlightPathMovementGenerator:: Taxi %s event %u of node %u of path %u for player %s", departure ? "departure" : "arrival", eventid, node->NodeIndex, node->pathId, player->getName().c_str());
    }
#endif
}

bool FlightPathMovementGenerator::getResetPos(Player*, float& x, float& y, float& z)
{
    WDB::Structures::TaxiPathNodeEntry const* node = _path[_currentNode];
    x = node->x;
    y = node->y;
    z = node->z;
    return true;
}

void FlightPathMovementGenerator::initEndGridInfo()
{
    //  Storage to preload flightmaster cell at end of flight. For multi-stop flights, this will
    //  be reinitialized for each flightmaster at the end of each spline (or stop) in the flight.

    // Number of nodes in path.
    uint32_t nodeCount = _path.size();
    // MapId of last node
    _endMapId = _path[nodeCount - 1]->mapid; 
    // Node where Destination Cell gets Loaded
    _preloadTargetNode = nodeCount - 3;
    // EndCell Position
    _endGridX = _path[nodeCount - 1]->x;
    _endGridY = _path[nodeCount - 1]->y;
}

void FlightPathMovementGenerator::preloadEndGrid()
{
    // used to preload the final grid where the flightmaster is
    WorldMap* endMap = sMapMgr.findWorldMap(_endMapId);

    // Load the MapCell
    if (endMap)
    {
        sLogger.debug("FlightPathMovementGenerator:: Preloading of Cell(%f, %f) for map %u at node index %u/%u", _endGridX, _endGridY, _endMapId, _preloadTargetNode, (uint32_t)(_path.size() - 1));
        if (endMap->getCellByCoords(_endGridX, _endGridY))
            endMap->getCellByCoords(_endGridX, _endGridY)->setActivity(true);
    }
}
