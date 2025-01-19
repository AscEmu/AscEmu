/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementGenerator.h"
#include "PathMovementBase.h"
#include "Movement/MovementDefines.h"
#include "Objects/Units/UnitDefines.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Utilities/Util.hpp"

class Player;
class Creature;
class Unit;

class FlightPathMovementGenerator : public MovementGeneratorMedium< Player, FlightPathMovementGenerator >,
    public PathMovementBase<Player, TaxiPathNodeList>
{
public:
    explicit FlightPathMovementGenerator(uint32_t startNode = 0)
    {
        _currentNode = startNode;
        _endGridX = 0.0f;
        _endGridY = 0.0f;
        _endMapId = 0;
        _preloadTargetNode = 0;

        Mode = MOTION_MODE_DEFAULT;
        Priority = MOTION_PRIORITY_HIGHEST;
        Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
        BaseUnitState = UNIT_STATE_IN_FLIGHT;
    }
    void loadPath(Player* player);
    void doInitialize(Player*);
    void doReset(Player*);
    void doFinalize(Player*, bool, bool);
    void doDeactivate(Player*);
    bool doUpdate(Player*, uint32_t);
    MovementGeneratorType getMovementGeneratorType() const override { return FLIGHT_MOTION_TYPE; }

    TaxiPathNodeList const& getPath() { return _path; }
    uint32_t getPathAtMapEnd() const;
    bool hasArrived() const { return (_currentNode >= _path.size()); }
    void setCurrentNodeAfterTeleport();
    void skipCurrentNode() { ++_currentNode; }
    void setToLastNode() { _currentNode = static_cast<uint32_t>(_path.size()) - 1; }
    void doEventIfAny(Player* player, WDB::Structures::TaxiPathNodeEntry const* node, bool departure);

    bool getResetPos(Player*, float& x, float& y, float& z);

    void initEndGridInfo();
    void preloadEndGrid();

private:
    float _endGridX;                                        //! X coord of last node location
    float _endGridY;                                        //! Y coord of last node location
    uint32_t _endMapId;                                     //! map Id of last node location
    uint32_t _preloadTargetNode;                            //! node index where preloading starts

    struct TaxiNodeChangeInfo
    {
        uint32_t PathIndex;
        int32_t Cost;
    };

    std::deque<TaxiNodeChangeInfo> _pointsForPathSwitch;    //! node indexes and costs where TaxiPath changes
};