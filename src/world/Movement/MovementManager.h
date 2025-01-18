/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Movement/MovementDefines.h"
#include "Movement/MovementGenerator.h"

#include <deque>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

class PathGenerator;
class Unit;
class LocationVector;
struct SplineChainLink;
struct SplineChainResumeInfo;
struct WaypointPath;

namespace MovementMgr
{
    class MoveSplineInit;
}

enum MovementManagerFlags : uint8_t
{
    MOTIONMASTER_FLAG_NONE = 0x0,
    MOTIONMASTER_FLAG_UPDATE = 0x1, // Update in progress
    MOTIONMASTER_FLAG_STATIC_INITIALIZATION_PENDING = 0x2, // Static movement (MOTION_SLOT_DEFAULT) hasn't been initialized
    MOTIONMASTER_FLAG_INITIALIZATION_PENDING = 0x4, // MovementManager is stalled until signaled
    MOTIONMASTER_FLAG_INITIALIZING = 0x8, // MovementManager is initializing

    MOTIONMASTER_FLAG_DELAYED = MOTIONMASTER_FLAG_UPDATE | MOTIONMASTER_FLAG_INITIALIZATION_PENDING
};

enum MovementManagerDelayedActionType : uint8_t
{
    MOTIONMASTER_DELAYED_CLEAR = 0,
    MOTIONMASTER_DELAYED_CLEAR_SLOT,
    MOTIONMASTER_DELAYED_CLEAR_MODE,
    MOTIONMASTER_DELAYED_CLEAR_PRIORITY,
    MOTIONMASTER_DELAYED_ADD,
    MOTIONMASTER_DELAYED_REMOVE,
    MOTIONMASTER_DELAYED_REMOVE_TYPE,
    MOTIONMASTER_DELAYED_INITIALIZE
};

struct MovementGeneratorDeleter
{
    void operator()(MovementGenerator* a);
};

struct MovementGeneratorComparator
{
public:
    bool operator()(MovementGenerator const* a, MovementGenerator const* b) const;
};

struct MovementGeneratorInformation
{
    MovementGeneratorInformation(MovementGeneratorType type, uint64_t targetGUID, std::string const& targetName);

    MovementGeneratorType Type;
    uint64_t TargetGUID;
    std::string TargetName;
};

static bool EmptyValidator()
{
    return true;
}

class SERVER_DECL MovementManager
{
public:
    typedef std::function<void()> DelayedActionDefine;
    typedef std::function<bool()> DelayedActionValidator;

    class DelayedAction
    {
    public:
        explicit DelayedAction(DelayedActionDefine&& action, DelayedActionValidator&& validator, MovementManagerDelayedActionType type) : Action(std::move(action)), Validator(std::move(validator)), Type(type) { }
        explicit DelayedAction(DelayedActionDefine&& action, MovementManagerDelayedActionType type) : Action(std::move(action)), Validator(EmptyValidator), Type(type) { }
        ~DelayedAction() = default;

        void resolve() { if (Validator()) Action(); }

        DelayedActionDefine Action;
        DelayedActionValidator Validator;
        uint8_t Type;
    };

    explicit MovementManager(Unit* unit);
    ~MovementManager();

    void initialize();
    void initializeDefault();
    void addToWorld();

    bool empty() const;
    uint32_t size() const;
    std::vector<MovementGeneratorInformation> getMovementGeneratorsInformation() const;
    MovementSlot getCurrentSlot() const;
    MovementGenerator* getCurrentMovementGenerator() const;
    MovementGeneratorType getCurrentMovementGeneratorType() const;
    MovementGeneratorType getCurrentMovementGeneratorType(MovementSlot slot) const;
    MovementGenerator* getCurrentMovementGenerator(MovementSlot slot) const;
    // Returns first found MovementGenerator that matches the given criteria
    MovementGenerator* getMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot = MOTION_SLOT_ACTIVE) const;
    bool hasMovementGenerator(std::function<bool(MovementGenerator const*)> const& filter, MovementSlot slot = MOTION_SLOT_ACTIVE) const;

    void update(uint32_t diff);
    void add(MovementGenerator* movement, MovementSlot slot = MOTION_SLOT_ACTIVE);
    // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
    void remove(MovementGenerator* movement, MovementSlot slot = MOTION_SLOT_ACTIVE);
    // Removes first found movement
    // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
    void remove(MovementGeneratorType type, MovementSlot slot = MOTION_SLOT_ACTIVE);
    // NOTE: MOTION_SLOT_DEFAULT wont be affected
    void clear();
    // Removes all movements for the given MovementSlot
    // NOTE: MOTION_SLOT_DEFAULT will be autofilled with IDLE_MOTION_TYPE
    void clear(MovementSlot slot);
    // Removes all movements with the given MovementGeneratorMode
    // NOTE: MOTION_SLOT_DEFAULT wont be affected
    void clear(MovementGeneratorMode mode);
    // Removes all movements with the given MovementGeneratorPriority
    // NOTE: MOTION_SLOT_DEFAULT wont be affected
    void clear(MovementGeneratorPriority priority);
    void propagateSpeedChange();
    bool getDestination(float &x, float &y, float &z);

    void moveIdle();
    void moveTargetedHome();
    void moveRandom(float wanderDistance = 0.0f);
    void moveFollow(Unit* target, float dist, ChaseAngle angle, MovementSlot slot = MOTION_SLOT_ACTIVE);
    void moveChase(Unit* target, Optional<ChaseRange> dist = {}, Optional<ChaseAngle> angle = {});
    void moveChase(Unit* target, float dist, float angle) { moveChase(target, ChaseRange(dist), ChaseAngle(angle)); }
    void moveChase(Unit* target, float dist) { moveChase(target, ChaseRange(dist)); }
    void moveConfused();
    void moveFleeing(Unit* enemy, uint32_t time = 0);
    void movePoint(uint32_t id, LocationVector const& pos, bool generatePath = true, Optional<float> finalOrient = {});
    void movePoint(uint32_t id, float x, float y, float z, bool generatePath = true, Optional<float> finalOrient = {});
    /*
     *  Makes the unit move toward the target until it is at a certain distance from it. The unit then stops.
     *  Only works in 2D.
     *  This method doesn't account for any movement done by the target. in other words, it only works if the target is stationary.
     */
    void moveCloserAndStop(uint32_t id, Unit* target, float distance);
    // These two movement types should only be used with creatures having landing/takeoff animations
    void moveLand(uint32_t id, LocationVector const& pos, Optional<float> velocity = {});
    void moveTakeoff(uint32_t id, LocationVector const& pos, Optional<float> velocity = {});
    void moveCharge(float x, float y, float z, float speed = SPEED_CHARGE, uint32_t id = EVENT_CHARGE, bool generatePath = false);
    void moveCharge(PathGenerator const& path, float speed = SPEED_CHARGE);
    void moveKnockbackFrom(float srcX, float srcY, float speedXY, float speedZ);
    void moveJumpTo(float angle, float speedXY, float speedZ);
    void moveJump(LocationVector const& pos, float speedXY, float speedZ, uint32_t id = EVENT_JUMP, bool hasOrientation = false);
    void moveJump(float x, float y, float z, float o, float speedXY, float speedZ, uint32_t id = EVENT_JUMP, bool hasOrientation = false);
    void moveCirclePath(float x, float y, float z, float radius, bool clockwise, uint8_t stepCount);
    void moveSmoothPath(uint32_t pointId, LocationVector const* pathPoints, size_t pathSize, bool walk);
    // Walk along spline chain stored in DB (script_spline_chain_meta and script_spline_chain_waypoints)
    void moveAlongSplineChain(uint32_t pointId, uint16_t dbChainId, bool walk);
    void moveAlongSplineChain(uint32_t pointId, std::vector<SplineChainLink> const& chain, bool walk);
    void resumeSplineChain(SplineChainResumeInfo const& info);
    void moveFall(uint32_t id = 0);
    void moveSeekAssistance(float x, float y, float z);
    void moveSeekAssistanceDistract(uint32_t timer);
    void moveTaxiFlight(uint32_t path, uint32_t pathnode);
    void moveDistract(uint32_t time, float orientation);
    void movePath(uint32_t pathId, bool repeatable);
    void movePath(WaypointPath& path, bool repeatable);
    void moveRotate(uint32_t id, uint32_t time, RotateDirection direction);
    void moveFormation(Unit* leader, float range, float angle, uint32_t point1, uint32_t point2);

    void launchMoveSpline(MovementMgr::MoveSplineInit&& init, uint32_t id = 0, MovementGeneratorPriority priority = MOTION_PRIORITY_NORMAL, MovementGeneratorType type = EFFECT_MOTION_TYPE);

private:
    typedef std::unique_ptr<MovementGenerator, MovementGeneratorDeleter> MovementGeneratorPointer;
    typedef std::multiset<MovementGenerator*, MovementGeneratorComparator> MovementManagerContainer;
    typedef std::unordered_multimap<uint32_t, MovementGenerator const*> MotionMasterUnitStatesContainer;

    void addFlag(uint8_t const flag) { _flags |= flag; }
    bool hasFlag(uint8_t const flag) const { return (_flags & flag) != 0; }
    void removeFlag(uint8_t const flag) { _flags &= ~flag; }

    void resolveDelayedActions();
    void remove(MovementManagerContainer::iterator iterator, bool active, bool movementInform);
    void pop(bool active, bool movementInform);
    void directInitialize();
    void directClear();
    void directClearDefault();
    void directClear(std::function<bool(MovementGenerator*)> const& filter);
    void directAdd(MovementGenerator* movement, MovementSlot slot);

    void Delete(MovementGenerator* movement, bool active, bool movementInform);
    void deleteDefault(bool active, bool movementInform);
    void addBaseUnitState(MovementGenerator const* movement);
    void clearBaseUnitState(MovementGenerator const* movement);
    void clearBaseUnitStates();

    Unit* _owner;
    MovementGeneratorPointer _defaultGenerator;
    MovementManagerContainer _generators;
    MotionMasterUnitStatesContainer _baseUnitStatesMap;
    std::deque<DelayedAction> _delayedActions;
    uint8_t _flags;
};
