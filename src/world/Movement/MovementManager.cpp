/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <algorithm>
#include <iterator>

#include "MovementManager.h"
#include "Objects/Units/Unit.hpp"
#include "AbstractFollower.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Object.hpp"
#include "Management/G3DPosition.hpp"
#include "Spline/MoveSpline.h"
#include "Spline/MoveSplineInit.h"
#include "Objects/Units/Players/Player.hpp"
#include "PathGenerator.h"
#include "WaypointDefines.h"
#include "Logging/Logger.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "MovementGenerators/ChaseMovementGenerator.h"
#include "MovementGenerators/ConfusedMovementGenerator.h"
#include "MovementGenerators/FleeingMovementGenerator.h"
#include "MovementGenerators/FollowMovementGenerator.h"
#include "MovementGenerators/FormationMovementGenerator.h"
#include "MovementGenerators/GenericMovementGenerator.h"
#include "MovementGenerators/HomeMovementGenerator.h"
#include "MovementGenerators/IdleMovementGenerator.h"
#include "MovementGenerators/PointMovementGenerator.h"
#include "MovementGenerators/RandomMovementGenerator.h"
#include "MovementGenerators/SplineChainMovementGenerator.h"
#include "MovementGenerators/WaypointMovementGenerator.h"
#include "MovementGenerators/FlightPathMovementGenerator.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/TimeTracker.hpp"

namespace FactorySelector
{
    MovementGenerator* selectMovementGenerator(Unit* unit)
    {
        MovementGeneratorType type = unit->getDefaultMovementType();
        if (Creature* creature = unit->ToCreature())
            if (!creature->m_playerControler)
                type = creature->getDefaultMovementType();

        MovementGeneratorCreator const* mv_factory = sMovementGeneratorRegistry->getRegistryItem(type);
        if (mv_factory)
            return mv_factory->create(unit);

        return nullptr;
    }
}

inline MovementGenerator* getIdleMovementGenerator()
{
    return sMovementGeneratorRegistry->getRegistryItem(IDLE_MOTION_TYPE)->create();
}

inline bool isStatic(MovementGenerator* movement)
{
    return (movement == getIdleMovementGenerator());
}

inline void movementGeneratorPointerDeleter(MovementGenerator* a)
{
    if (a != nullptr && !isStatic(a))
        delete a;
}

void MovementGeneratorDeleter::operator()(MovementGenerator* a)
{
    movementGeneratorPointerDeleter(a);
}

bool MovementGeneratorComparator::operator()(MovementGenerator const* a, MovementGenerator const* b) const
{
    if (a->Mode > b->Mode)
        return true;
    else if (a->Mode == b->Mode)
        return a->Priority > b->Priority;

    return false;
}

MovementGeneratorInformation::MovementGeneratorInformation(MovementGeneratorType type, uint64_t targetGUID, std::string const& targetName) : Type(type), TargetGUID(targetGUID), TargetName(targetName) { }

MovementManager::MovementManager(Unit* unit) : _owner(unit), _defaultGenerator(nullptr), _flags(MOTIONMASTER_FLAG_INITIALIZATION_PENDING) { }

MovementManager::~MovementManager()
{
    _delayedActions.clear();

    for (auto itr = _generators.begin(); itr != _generators.end(); itr = _generators.erase(itr))
        movementGeneratorPointerDeleter(*itr);
}

void MovementManager::initialize()
{
    if (hasFlag(MOTIONMASTER_FLAG_INITIALIZATION_PENDING))
        return;

    if (hasFlag(MOTIONMASTER_FLAG_UPDATE))
    {
        DelayedActionDefine action = [this]()
        {
            initialize();
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_INITIALIZE);
        return;
    }

    directInitialize();
}

void MovementManager::initializeDefault()
{
    add(FactorySelector::selectMovementGenerator(_owner), MOTION_SLOT_DEFAULT);
}

void MovementManager::addToWorld()
{
    if (!hasFlag(MOTIONMASTER_FLAG_INITIALIZATION_PENDING))
        return;

    addFlag(MOTIONMASTER_FLAG_INITIALIZING);
    removeFlag(MOTIONMASTER_FLAG_INITIALIZATION_PENDING);

    directInitialize();
    resolveDelayedActions();

    removeFlag(MOTIONMASTER_FLAG_INITIALIZING);
}

bool MovementManager::empty() const
{
    return !_defaultGenerator && _generators.empty();
}

uint32_t MovementManager::size() const
{
    return (_defaultGenerator ? 1 : 0) + uint32_t(_generators.size());
}

std::vector<MovementGeneratorInformation> MovementManager::getMovementGeneratorsInformation() const
{
    std::vector<MovementGeneratorInformation> list;

    if (_defaultGenerator)
        list.emplace_back(_defaultGenerator->getMovementGeneratorType(), 0, std::string());

    for (auto itr = _generators.begin(); itr != _generators.end(); ++itr)
    {
        MovementGenerator* movement = *itr;
        MovementGeneratorType const type = movement->getMovementGeneratorType();
        switch (type)
        {
        case CHASE_MOTION_TYPE:
        case FOLLOW_MOTION_TYPE:
            if (AbstractFollower* followInformation = dynamic_cast<AbstractFollower*>(movement))
            {
                if (Unit* target = followInformation->getTarget())
                    list.emplace_back(type, target->getGuid(), target->ToCreature()->GetCreatureProperties()->Name.c_str());
                else
                    list.emplace_back(type, 0, std::string());
            }
            else
                list.emplace_back(type, 0, std::string());
            break;
        default:
            list.emplace_back(type, 0, std::string());
            break;
        }
    }

    return list;
}

MovementSlot MovementManager::getCurrentSlot() const
{
    if (!_generators.empty())
        return MOTION_SLOT_ACTIVE;

    if (_defaultGenerator)
        return MOTION_SLOT_DEFAULT;

    return MAX_MOTION_SLOT;
}

MovementGenerator* MovementManager::getCurrentMovementGenerator() const
{
    if (!_generators.empty())
        return *_generators.begin();

    if (_defaultGenerator)
        return _defaultGenerator.get();

    return nullptr;
}

MovementGeneratorType MovementManager::getCurrentMovementGeneratorType() const
{
    if (empty())
        return MAX_MOTION_TYPE;

    MovementGenerator const* movement = getCurrentMovementGenerator();
    if (!movement)
        return MAX_MOTION_TYPE;

    return movement->getMovementGeneratorType();
}

MovementGeneratorType MovementManager::getCurrentMovementGeneratorType(MovementSlot slot) const
{
    if (empty() || isInvalidMovementSlot(slot))
        return MAX_MOTION_TYPE;

    if (slot == MOTION_SLOT_ACTIVE && !_generators.empty())
        return (*_generators.begin())->getMovementGeneratorType();

    if (slot == MOTION_SLOT_DEFAULT && _defaultGenerator)
        return _defaultGenerator->getMovementGeneratorType();

    return MAX_MOTION_TYPE;
}

MovementGenerator* MovementManager::getCurrentMovementGenerator(MovementSlot slot) const
{
    if (empty() || isInvalidMovementSlot(slot))
        return nullptr;

    if (slot == MOTION_SLOT_ACTIVE && !_generators.empty())
        return *_generators.begin();

    if (slot == MOTION_SLOT_DEFAULT && _defaultGenerator)
        return _defaultGenerator.get();

    return nullptr;
}

MovementGenerator* MovementManager::getMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot) const
{
    if (empty() || isInvalidMovementSlot(slot))
        return nullptr;

    MovementGenerator* movement = nullptr;
    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && filter(_defaultGenerator.get()))
            movement = _defaultGenerator.get();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), std::ref(filter));
            if (itr != _generators.end())
                movement = *itr;
        }
        break;
    default:
        break;
    }

    return movement;
}

bool MovementManager::hasMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot) const
{
    if (empty() || isInvalidMovementSlot(slot))
        return false;

    bool value = false;
    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && filter(_defaultGenerator.get()))
            value = true;
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), std::ref(filter));
            value = itr != _generators.end();
        }
        break;
    default:
        break;
    }

    return value;
}

void MovementManager::update(uint32_t diff)
{
    if (!_owner)
        return;

    if (hasFlag(MOTIONMASTER_FLAG_INITIALIZATION_PENDING | MOTIONMASTER_FLAG_INITIALIZING))
        return;

    if (empty())
    {
        sLogger.failure("MovementManager: update called without Initializing! ({})", _owner->getGuid());
        return;
    }

    addFlag(MOTIONMASTER_FLAG_UPDATE);

    MovementGenerator* top = getCurrentMovementGenerator();
    if (hasFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING) && isStatic(top))
    {
        removeFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
        top->initialize(_owner);
    }
    if (top->hasFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING))
        top->initialize(_owner);
    if (top->hasFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED))
        top->reset(_owner);

    if (!top->update(_owner, diff))
    {
        // Since all the actions that modify any slot are delayed, this movement is guaranteed to be top
        pop(true, true); // Natural, and only, call to MovementInform
    }

    removeFlag(MOTIONMASTER_FLAG_UPDATE);

    resolveDelayedActions();
}

void MovementManager::add(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (!movement)
        return;

    if (isInvalidMovementSlot(slot))
    {
        delete movement;
        return;
    }

    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, movement, slot]()
        {
            add(movement, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_ADD);
    }
    else
        directAdd(movement, slot);
}

void MovementManager::remove(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (!movement || isInvalidMovementSlot(slot))
        return;

    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, movement, slot]()
        {
            remove(movement, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_REMOVE);
        return;
    }

    if (empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && _defaultGenerator.get() == movement)
            directClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto bounds = _generators.equal_range(movement);
            auto itr = std::find(bounds.first, bounds.second, movement);
            if (itr != _generators.end())
                remove(itr, getCurrentMovementGenerator() == *itr, false);
        }
        break;
    default:
        break;
    }
}

void MovementManager::remove(MovementGeneratorType type, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    if (isInvalidMovementGeneratorType(type) || isInvalidMovementSlot(slot))
        return;

    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, type, slot]()
        {
            remove(type, slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_REMOVE_TYPE);
        return;
    }

    if (empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator && _defaultGenerator->getMovementGeneratorType() == type)
            directClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            auto itr = std::find_if(_generators.begin(), _generators.end(), [type](MovementGenerator const* a) -> bool
                {
                    return a->getMovementGeneratorType() == type;
                });

            if (itr != _generators.end())
                remove(itr, getCurrentMovementGenerator() == *itr, false);
        }
        break;
    default:
        break;
    }
}

void MovementManager::clear()
{
    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this]()
        {
            clear();
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR);
        return;
    }

    if (!empty())
        directClear();
}

void MovementManager::clear(MovementSlot slot)
{
    if (isInvalidMovementSlot(slot))
        return;

    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, slot]()
        {
            clear(slot);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_SLOT);
        return;
    }

    if (empty())
        return;

    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        directClearDefault();
        break;
    case MOTION_SLOT_ACTIVE:
        directClear();
        break;
    default:
        break;
    }
}

void MovementManager::clear(MovementGeneratorMode mode)
{
    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, mode]()
        {
            clear(mode);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_MODE);
        return;
    }

    if (empty())
        return;

    std::function<bool(MovementGenerator*)> criteria = [mode](MovementGenerator* a) -> bool
    {
        return a->Mode == mode;
    };
    directClear(criteria);
}

void MovementManager::clear(MovementGeneratorPriority priority)
{
    if (hasFlag(MOTIONMASTER_FLAG_DELAYED))
    {
        DelayedActionDefine action = [this, priority]()
        {
            clear(priority);
        };
        _delayedActions.emplace_back(std::move(action), MOTIONMASTER_DELAYED_CLEAR_PRIORITY);
        return;
    }

    if (empty())
        return;

    std::function<bool(MovementGenerator*)> criteria = [priority](MovementGenerator* a) -> bool
    {
        return a->Priority == priority;
    };
    directClear(criteria);
}

void MovementManager::propagateSpeedChange()
{
    if (empty())
        return;

    MovementGenerator* movement = getCurrentMovementGenerator();
    if (!movement)
        return;

    movement->unitSpeedChanged();
}

bool MovementManager::getDestination(float &x, float &y, float &z)
{
    if (_owner->movespline->Finalized())
        return false;

    G3D::Vector3 const& dest = _owner->movespline->FinalDestination();
    x = dest.x;
    y = dest.y;
    z = dest.z;
    return true;
}

void MovementManager::moveIdle()
{
    add(getIdleMovementGenerator(), MOTION_SLOT_DEFAULT);
}

void MovementManager::moveTargetedHome()
{
    Creature* owner = _owner->ToCreature();
    if (!owner)
    {
        return;
    }

    clear();

    uint64_t ownerGuid = owner->isCharmed() ? owner->getCharmGuid() : owner->getCreatedByGuid();
    Unit* target = owner->getWorldMapCreature(ownerGuid);
    if (!target)
    {
        add(new HomeMovementGenerator<Creature>());
    }
    else
    {
        add(new FollowMovementGenerator(target, PET_FOLLOW_DIST, PET_FOLLOW_ANGLE));
    }
}

void MovementManager::moveRandom(float wanderDistance)
{
    if (_owner->getObjectTypeId() == TYPEID_UNIT)
    {
        add(new RandomMovementGenerator<Creature>(wanderDistance), MOTION_SLOT_DEFAULT);
    }
}

void MovementManager::moveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    // Ignore movement request if target not exist
    if (!target || target == _owner)
        return;

    add(new FollowMovementGenerator(target, dist, angle), slot);
}

void MovementManager::moveChase(Unit* target, Optional<ChaseRange> dist, Optional<ChaseAngle> angle)
{
    // Ignore movement request if target not exist
    if (!target || target == _owner || !_owner->isAIEnabled())
        return;

    add(new ChaseMovementGenerator(target, dist, angle));
}

void MovementManager::moveConfused()
{
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
    {
        add(new ConfusedMovementGenerator<Player>());
    }
    else
    {
        add(new ConfusedMovementGenerator<Creature>());
    }
}

void MovementManager::moveFleeing(Unit* enemy, uint32_t time)
{
    if (!enemy)
        return;

    if (_owner->getObjectTypeId() == TYPEID_UNIT)
    {
        if (time)
            add(new TimedFleeingMovementGenerator(enemy->getGuid(), time));
        else
            add(new FleeingMovementGenerator<Creature>(enemy->getGuid()));
    }
    else
        add(new FleeingMovementGenerator<Player>(enemy->getGuid()));
}

void MovementManager::movePoint(uint32_t id, LocationVector const& pos, bool generatePath/* = true*/, Optional<float> finalOrient/* = {}*/)
{
    movePoint(id, pos.x, pos.y, pos.z, generatePath, finalOrient);
}

void MovementManager::movePoint(uint32_t id, float x, float y, float z, bool generatePath, Optional<float> finalOrient)
{
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
    {
        add(new PointMovementGenerator<Player>(id, x, y, z, generatePath, 0.0f, finalOrient));
    }
    else
    {
        add(new PointMovementGenerator<Creature>(id, x, y, z, generatePath, 0.0f, finalOrient));
    }
}

void MovementManager::moveCloserAndStop(uint32_t id, Unit* target, float distance)
{
    float distanceToTravel = _owner->getDistanceSq(target) - distance;
    if (distanceToTravel > 0.0f)
    {
        float angle = _owner->getAbsoluteAngle(target);
        float destx = _owner->GetPositionX() + distanceToTravel * std::cos(angle);
        float desty = _owner->GetPositionY() + distanceToTravel * std::sin(angle);
        movePoint(id, destx, desty, target->GetPositionZ());
    }
    else
    {
        // We are already close enough. We just need to turn toward the target without changing position.
        MovementMgr::MoveSplineInit init(_owner);
        init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ());
        init.SetFacing(target);
        add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
    }
}

void MovementManager::moveLand(uint32_t id, LocationVector const& pos, Optional<float> velocity /*= {}*/)
{
    MovementMgr::MoveSplineInit init(_owner);
    init.MoveTo(positionToVector3(pos), false);
#if VERSION_STRING >= WotLK
    init.SetAnimation(ANIMATION_FLAG_GROUND);
#endif
    if (velocity)
        init.SetVelocity(*velocity);
    add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
}

void MovementManager::moveTakeoff(uint32_t id, LocationVector const& pos, Optional<float> velocity /*= {}*/)
{
    MovementMgr::MoveSplineInit init(_owner);
    init.MoveTo(positionToVector3(pos), false);
#if VERSION_STRING >= WotLK
    init.SetAnimation(ANIMATION_FLAG_HOVER);
#endif
    if (velocity)
        init.SetVelocity(*velocity);
    add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id));
}

void MovementManager::moveCharge(float x, float y, float z, float speed /*= SPEED_CHARGE*/, uint32_t id /*= EVENT_CHARGE*/, bool generatePath /*= false*/)
{
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
    {
        PointMovementGenerator<Player>* movement = new PointMovementGenerator<Player>(id, x, y, z, generatePath, speed);
        movement->Priority = MOTION_PRIORITY_HIGHEST;
        movement->BaseUnitState = UNIT_STATE_CHARGING;
        add(movement);
    }
    else
    {
        PointMovementGenerator<Creature>* movement = new PointMovementGenerator<Creature>(id, x, y, z, generatePath, speed);
        movement->Priority = MOTION_PRIORITY_HIGHEST;
        movement->BaseUnitState = UNIT_STATE_CHARGING;
        add(movement);
    }
}

void MovementManager::moveCharge(PathGenerator const& path, float speed /*= SPEED_CHARGE*/)
{
    G3D::Vector3 dest = path.getActualEndPosition();

    moveCharge(dest.x, dest.y, dest.z, speed, EVENT_CHARGE_PREPATH);

    // Charge movement is not started when using EVENT_CHARGE_PREPATH
    MovementMgr::MoveSplineInit init(_owner);
    init.MovebyPath(path.getPath());
    init.SetVelocity(speed);
    init.Launch();
}

void MovementManager::moveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ)
{
    // This function may make players fall below map
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
        return;

    if (speedXY < 0.01f)
        return;

    LocationVector dest = _owner->GetPosition();
    float moveTimeHalf = speedZ / MovementMgr::gravity;
    float dist = 2 * moveTimeHalf * speedXY;
    float max_height = -MovementMgr::computeFallElevation(moveTimeHalf, false, -speedZ);

    // Use a mmap raycast to get a valid destination.
    _owner->movePositionToFirstCollision(dest, dist, _owner->getRelativeAngle(srcX, srcY) + float(M_PI));

    MovementMgr::MoveSplineInit init(_owner);
    init.MoveTo(dest.getPositionX(), dest.getPositionY(), dest.getPositionZ(), false);
    init.SetParabolic(max_height, 0);
#if VERSION_STRING >= WotLK
    init.SetOrientationFixed(true);
#endif
    init.SetVelocity(speedXY);

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, 0);
    movement->Priority = MOTION_PRIORITY_HIGHEST;
    add(movement);
}

void MovementManager::moveJumpTo(float angle, float speedXY, float speedZ)
{
    // This function may make players fall below map
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
        return;

    float x, y, z = _owner->GetPositionZ();

    float moveTimeHalf = speedZ / MovementMgr::gravity;
    float dist = 2 * moveTimeHalf * speedXY;

    _owner->getNearPoint2D(nullptr, x, y, dist, _owner->GetOrientation() + angle);
    _owner->updateAllowedPositionZ(x, y, z);

    moveJump(x, y, z, 0.0f, speedXY, speedZ);
}

void MovementManager::moveJump(LocationVector const& pos, float speedXY, float speedZ, uint32_t id/* = EVENT_JUMP*/, bool hasOrientation/* = false*/)
{
    moveJump(pos.getPositionX(), pos.getPositionY(), pos.getPositionZ(), pos.getOrientation(), speedXY, speedZ, id, hasOrientation);
}

void MovementManager::moveJump(float x, float y, float z, float o, float speedXY, float speedZ, uint32_t id, bool hasOrientation /* = false*/)
{
    if (speedXY < 0.01f)
        return;

    float moveTimeHalf = speedZ / MovementMgr::gravity;
    float max_height = -MovementMgr::computeFallElevation(moveTimeHalf, false, -speedZ);

    MovementMgr::MoveSplineInit init(_owner);
    init.MoveTo(x, y, z, false);
    init.SetParabolic(max_height, 0);
    init.SetVelocity(speedXY);
    if (hasOrientation)
        init.SetFacing(o);

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id);
    movement->Priority = MOTION_PRIORITY_HIGHEST;
    movement->BaseUnitState = UNIT_STATE_JUMPING;
    add(movement);
}

void MovementManager::moveCirclePath(float x, float y, float z, float radius, bool clockwise, uint8_t stepCount)
{
    float step = 2 * float(M_PI) / stepCount * (clockwise ? -1.0f : 1.0f);
    LocationVector const& pos = { x, y, z, 0.0f };
    float angle = pos.getAbsoluteAngle(_owner->GetPositionX(), _owner->GetPositionY());

    MovementMgr::MoveSplineInit init(_owner);

    // add the owner's current position as starting point as it gets removed after entering the cycle
    init.Path().push_back(G3D::Vector3(_owner->GetPositionX(), _owner->GetPositionY(), _owner->GetPositionZ()));

    for (uint8_t i = 0; i < stepCount; angle += step, ++i)
    {
        G3D::Vector3 point;
        point.x = x + radius * cosf(angle);
        point.y = y + radius * sinf(angle);

        if (_owner->IsFlying())
        {
            point.z = z;
        }
        else
        {
            point.z = _owner->getWorldMap()->getHeight(LocationVector(point.x, point.y, z));
#if VERSION_STRING >= WotLK
            point.z += _owner->getHoverHeight();
#endif
        }

        init.Path().push_back(point);
    }

    if (_owner->IsFlying())
    {
        init.SetFly();
        init.SetCyclic();
#if VERSION_STRING >= WotLK
        init.SetAnimation(ANIMATION_FLAG_HOVER);
#endif
    }
    else
    {
        init.SetWalk(true);
        init.SetCyclic();
    }

    add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, 0));
}

void MovementManager::moveSmoothPath(uint32_t pointId, LocationVector const* pathPoints, size_t pathSize, bool walk)
{
    MovementMgr::MoveSplineInit init(_owner);
    MovementMgr::PointsArray path;
    path.reserve(pathSize);
    std::transform(pathPoints, pathPoints + pathSize, std::back_inserter(path), [](LocationVector const& point)
        {
            return G3D::Vector3(point.getPositionX(), point.getPositionY(), point.getPositionZ());
        });

    init.MovebyPath(path);
#if VERSION_STRING >= WotLK
    init.SetSmooth();
#endif
    init.SetWalk(walk);

    // This code is not correct
    // GenericMovementGenerator does not affect UNIT_STATE_ROAMING_MOVE
    // need to call PointMovementGenerator with various pointIds
    add(new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, pointId));
}

void MovementManager::moveAlongSplineChain(uint32_t pointId, uint16_t dbChainId, bool walk)
{
    Creature* owner = _owner->ToCreature();
    if (!owner)
    {
        return;
    }

    std::vector<SplineChainLink> const* chain = sMySQLStore.getSplineChain(owner, dbChainId);
    if (!chain)
    {
        return;
    }

    moveAlongSplineChain(pointId, *chain, walk);
}

void MovementManager::moveAlongSplineChain(uint32_t pointId, std::vector<SplineChainLink> const& chain, bool walk)
{
    add(new SplineChainMovementGenerator(pointId, chain, walk));
}

void MovementManager::resumeSplineChain(SplineChainResumeInfo const& info)
{
    if (info.Empty())
    {
        return;
    }
    add(new SplineChainMovementGenerator(info));
}

void MovementManager::moveFall(uint32_t id/* = 0*/)
{
    // Use larger distance for vmap height search than in most other cases
    float tz = _owner->getWorldMap()->getHeight(_owner->GetPosition());
    if (tz <= INVALID_HEIGHT)
        return;

    // Abort too if the ground is very near
    if (std::fabs(_owner->GetPositionZ() - tz) < 0.1f)
        return;

    // rooted units don't move (also setting falling+root flag causes client freezes)
    if (_owner->hasUnitStateFlag(UNIT_STATE_ROOTED | UNIT_STATE_STUNNED))
        return;

    _owner->addUnitMovementFlag(MOVEFLAG_FALLING);
    _owner->obj_movement_info.setFallTime(0);

    // Don't run spline movement for players
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
        return;

#if VERSION_STRING >= WotLK
    tz += _owner->getHoverHeight();
#endif

    MovementMgr::MoveSplineInit init(_owner);
    init.MoveTo(_owner->GetPositionX(), _owner->GetPositionY(), tz, false);
    init.SetFall();

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), EFFECT_MOTION_TYPE, id);
    movement->Priority = MOTION_PRIORITY_HIGHEST;
    add(movement);
}

void MovementManager::moveSeekAssistance(float x, float y, float z)
{
    if (Creature* creature = _owner->ToCreature())
    {
        // todo
        //creature->AttackStop();
        //creature->CastStop();
        //creature->DoNotReacquireSpellFocusTarget();
        creature->getAIInterface()->setReactState(REACT_PASSIVE);
        add(new AssistanceMovementGenerator(EVENT_ASSIST_MOVE, x, y, z));
    }
}

void MovementManager::moveSeekAssistanceDistract(uint32_t time)
{
    if (_owner->getObjectTypeId() == TYPEID_UNIT)
    {
        add(new AssistanceDistractMovementGenerator(time, _owner->GetOrientation()));
    }
}

void MovementManager::moveTaxiFlight(uint32_t path, uint32_t pathnode)
{
    if (_owner->getObjectTypeId() == TYPEID_PLAYER)
    {
        if (path < sTaxiPathNodesByPath.size())
        {
            bool hasExisting = hasMovementGenerator([](MovementGenerator const* gen) { return gen->getMovementGeneratorType() == FLIGHT_MOTION_TYPE; });
            if (hasExisting)
            {
                sLogger.failure("MoveTaxiFlight:: {} already has a Flightpath Movement Generator", _owner->ToPlayer()->getName());
                return;
            }

            sLogger.debug("MoveTaxiFlight:: {} taxi to (Path {} node {}).", _owner->ToPlayer()->getName(), path, pathnode);
            FlightPathMovementGenerator* movement = new FlightPathMovementGenerator(pathnode);
            movement->loadPath(_owner->ToPlayer());
            add(movement);
        }
        else
        {
            sLogger.failure("MoveTaxiFlight:: {} attempted taxi to (non-existing Path {} node {}).", _owner->ToPlayer()->getName(), path, pathnode);
        }
    }
}

void MovementManager::moveDistract(uint32_t timer, float orientation)
{
    add(new DistractMovementGenerator(timer, orientation));
}

void MovementManager::movePath(uint32_t pathId, bool repeatable)
{
    if (!pathId)
        return;

    add(new WaypointMovementGenerator<Creature>(pathId, repeatable), MOTION_SLOT_DEFAULT);
}

void MovementManager::movePath(WaypointPath& path, bool repeatable)
{
    add(new WaypointMovementGenerator<Creature>(path, repeatable), MOTION_SLOT_DEFAULT);
}

void MovementManager::moveRotate(uint32_t id, uint32_t time, RotateDirection direction)
{
    if (!time)
        return;

    add(new RotateMovementGenerator(id, time, direction));
}

void MovementManager::moveFormation(Unit* leader, float range, float angle, uint32_t point1, uint32_t point2)
{
    if (_owner->getObjectTypeId() == TYPEID_UNIT && leader)
    {
        add(new FormationMovementGenerator(leader, range, angle, point1, point2), MOTION_SLOT_DEFAULT);
    }
}

void MovementManager::launchMoveSpline(MovementMgr::MoveSplineInit&& init, uint32_t id/*= 0*/, MovementGeneratorPriority priority/* = MOTION_PRIORITY_NORMAL*/, MovementGeneratorType type/*= EFFECT_MOTION_TYPE*/)
{
    if (isInvalidMovementGeneratorType(type))
    {
        return;
    }

    GenericMovementGenerator* movement = new GenericMovementGenerator(std::move(init), type, id);
    movement->Priority = priority;
    add(movement);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Private methods
void MovementManager::resolveDelayedActions()
{
    while (!_delayedActions.empty())
    {
        _delayedActions.front().resolve();
        _delayedActions.pop_front();
    }
}

void MovementManager::remove(MovementManagerContainer::iterator iterator, bool active, bool movementInform)
{
    MovementGenerator* pointer = *iterator;
    _generators.erase(iterator);
    Delete(pointer, active, movementInform);
}

void MovementManager::pop(bool active, bool movementInform)
{
    if (!_generators.empty())
        remove(_generators.begin(), active, movementInform);
}

void MovementManager::directInitialize()
{
    // clear ALL movement generators (including default)
    directClearDefault();
    directClear();

    initializeDefault();
}

void MovementManager::directClear()
{
    // First delete Top
    if (!_generators.empty())
        pop(true, false);

    // Then the rest
    while (!_generators.empty())
        pop(false, false);

    // Make sure the storage is empty
    clearBaseUnitStates();
}

void MovementManager::directClearDefault()
{
    if (_defaultGenerator)
        deleteDefault(_generators.empty(), false);
}

void MovementManager::directClear(std::function<bool(MovementGenerator*)> const& filter)
{
    if (_generators.empty())
        return;

    MovementGenerator const* top = getCurrentMovementGenerator();
    for (auto itr = _generators.begin(); itr != _generators.end();)
    {
        if (filter(*itr))
        {
            MovementGenerator* movement = *itr;
            itr = _generators.erase(itr);
            Delete(movement, movement == top, false);
        }
        else
            ++itr;
    }
}

void MovementManager::directAdd(MovementGenerator* movement, MovementSlot slot/* = MOTION_SLOT_ACTIVE*/)
{
    switch (slot)
    {
    case MOTION_SLOT_DEFAULT:
        if (_defaultGenerator)
        {
            _defaultGenerator->finalize(_owner, _generators.empty(), false);
            _defaultGenerator->notifyAIOnFinalize(_owner);
        }

        _defaultGenerator = MovementGeneratorPointer(movement);
        if (isStatic(movement))
            addFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
        break;
    case MOTION_SLOT_ACTIVE:
        if (!_generators.empty())
        {
            if (movement->Priority >= (*_generators.begin())->Priority)
            {
                auto itr = _generators.begin();
                if (movement->Priority == (*itr)->Priority)
                    remove(itr, true, false);
                else
                    (*itr)->deactivate(_owner);
            }
            else
            {
                auto itr = std::find_if(_generators.begin(), _generators.end(), [movement](MovementGenerator const* a) -> bool
                    {
                        return a->Priority == movement->Priority;
                    });

                if (itr != _generators.end())
                    remove(itr, false, false);
            }
        }
        else if (_defaultGenerator != nullptr)
        {
            _defaultGenerator->deactivate(_owner);
        }

        _generators.emplace(movement);
        addBaseUnitState(movement);
        break;
    default:
        break;
    }
}

void MovementManager::Delete(MovementGenerator* movement, bool active, bool movementInform)
{
    movement->finalize(_owner, active, movementInform);
    movement->notifyAIOnFinalize(_owner);
    clearBaseUnitState(movement);
    movementGeneratorPointerDeleter(movement);
}

void MovementManager::deleteDefault(bool active, bool movementInform)
{
    _defaultGenerator->finalize(_owner, active, movementInform);
    _defaultGenerator->notifyAIOnFinalize(_owner);
    _defaultGenerator = MovementGeneratorPointer(getIdleMovementGenerator());
    addFlag(MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING);
}

void MovementManager::addBaseUnitState(MovementGenerator const* movement)
{
    if (!movement || !movement->BaseUnitState)
        return;

    _baseUnitStatesMap.emplace(movement->BaseUnitState, movement);
    _owner->addUnitStateFlag(movement->BaseUnitState);
}

void MovementManager::clearBaseUnitState(MovementGenerator const* movement)
{
    if (!movement || !movement->BaseUnitState)
        return;

    auto range = _baseUnitStatesMap.equal_range(movement->BaseUnitState);
    for (auto itr = range.first; itr != range.second;)
    {
        if (itr->second == movement)
            itr = _baseUnitStatesMap.erase(itr);
        else
            ++itr;
    }

    if (_baseUnitStatesMap.count(movement->BaseUnitState) == 0)
        _owner->removeUnitStateFlag(movement->BaseUnitState);
}

void MovementManager::clearBaseUnitStates()
{
    uint32_t unitState = 0;
    for (auto itr = _baseUnitStatesMap.begin(); itr != _baseUnitStatesMap.end(); ++itr)
        unitState |= itr->first;

    _owner->removeUnitStateFlag(unitState);
    _baseUnitStatesMap.clear();
}
