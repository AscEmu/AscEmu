/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"
#include "FactoryHolder.h"
#include "ObjectRegistry.h"

class Creature;
class Unit;

enum MovementGeneratorType : uint8_t;

enum MovementGeneratorFlags : uint16_t
{
    MOVEMENTGENERATOR_FLAG_NONE                   = 0x000,
    MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING = 0x001,
    MOVEMENTGENERATOR_FLAG_INITIALIZED            = 0x002,
    MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING   = 0x004,
    MOVEMENTGENERATOR_FLAG_INTERRUPTED            = 0x008,
    MOVEMENTGENERATOR_FLAG_PAUSED                 = 0x010,
    MOVEMENTGENERATOR_FLAG_TIMED_PAUSED           = 0x020,
    MOVEMENTGENERATOR_FLAG_DEACTIVATED            = 0x040,
    MOVEMENTGENERATOR_FLAG_INFORM_ENABLED         = 0x080,
    MOVEMENTGENERATOR_FLAG_FINALIZED              = 0x100,

    MOVEMENTGENERATOR_FLAG_TRANSITORY = MOVEMENTGENERATOR_FLAG_SPEED_UPDATE_PENDING | MOVEMENTGENERATOR_FLAG_INTERRUPTED
};

class SERVER_DECL MovementGenerator
{
public:
    MovementGenerator() : Mode(0), Priority(0), Flags(MOVEMENTGENERATOR_FLAG_NONE), BaseUnitState(0) { }
    virtual ~MovementGenerator();

    // on top first update
    virtual void initialize(Unit*) = 0;
    // on top reassign
    virtual void reset(Unit*) = 0;
    // on top on MovementManager::Update
    virtual bool update(Unit*, uint32_t diff) = 0;
    // on current top if another movement replaces
    virtual void deactivate(Unit*) = 0;
    // on movement delete
    virtual void finalize(Unit*, bool, bool) = 0;
    virtual MovementGeneratorType getMovementGeneratorType() const = 0;

    virtual void unitSpeedChanged() { }
    // timer in ms
    virtual void pause(uint32_t/* timer = 0*/) { }
    // timer in ms
    virtual void resume(uint32_t/* overrideTimer = 0*/) { }
    // used by Evade code for select point to evade with expected restart default movement
    virtual bool getResetPosition(Unit*, float&/* x*/, float&/* y*/, float&/* z*/) { return false; }

    virtual void notifyAIOnFinalize(Unit*);

    void addFlag(uint16_t const flag) { Flags |= flag; }
    bool hasFlag(uint16_t const flag) const { return (Flags & flag) != 0; }
    void removeFlag(uint16_t const flag) { Flags &= ~flag; }

    virtual std::string getDebugInfo() const;

    uint8_t Mode;
    uint8_t Priority;
    uint16_t Flags;
    uint32_t BaseUnitState;
};

template<class T, class D>
class MovementGeneratorMedium : public MovementGenerator
{
public:
    void initialize(Unit* owner) override
    {
        (static_cast<D*>(this))->doInitialize(static_cast<T*>(owner));
    }

    void reset(Unit* owner) override
    {
        (static_cast<D*>(this))->doReset(static_cast<T*>(owner));
    }

    bool update(Unit* owner, uint32_t diff) override
    {
        return (static_cast<D*>(this))->doUpdate(static_cast<T*>(owner), diff);
    }

    void deactivate(Unit* owner) override
    {
        (static_cast<D*>(this))->doDeactivate(static_cast<T*>(owner));
    }

    void finalize(Unit* owner, bool active, bool movementInform) override
    {
        (static_cast<D*>(this))->doFinalize(static_cast<T*>(owner), active, movementInform);
    }
};

typedef FactoryHolder<MovementGenerator, Unit, MovementGeneratorType> MovementGeneratorCreator;

template<class Movement>
struct MovementGeneratorFactory : public MovementGeneratorCreator
{
    MovementGeneratorFactory(MovementGeneratorType movementGeneratorType) : MovementGeneratorCreator(movementGeneratorType) { }

    MovementGenerator* create(Unit* /*object*/) const override
    {
        return new Movement();
    }
};

struct IdleMovementFactory : public MovementGeneratorCreator
{
    IdleMovementFactory();

    MovementGenerator* create(Unit* object) const override;
};

struct RandomMovementFactory : public MovementGeneratorCreator
{
    RandomMovementFactory();

    MovementGenerator* create(Unit* object) const override;
};

struct WaypointMovementFactory : public MovementGeneratorCreator
{
    WaypointMovementFactory();

    MovementGenerator* create(Unit* object) const override;
};

typedef MovementGeneratorCreator::FactoryHolderRegistry MovementGeneratorRegistry;

#define sMovementGeneratorRegistry MovementGeneratorRegistry::getInstance()
