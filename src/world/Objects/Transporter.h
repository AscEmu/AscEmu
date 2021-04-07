/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Objects/GameObject.h"
#include "Management/TransporterHandler.h"

class SERVER_DECL TransportBase
{
protected:
    TransportBase() { }
    virtual ~TransportBase() { }

public:
    // This method transforms supplied transport offsets into global coordinates
    virtual void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const = 0;

    // This method transforms supplied global coordinates into local offsets
    virtual void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const = 0;

protected:
    static void CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
    {
        float inx = x, iny = y, inz = z;
        if (o)
            *o = sTransportHandler.normalizeOrientation(transO + *o);

        x = transX + inx * std::cos(transO) - iny * std::sin(transO);
        y = transY + iny * std::cos(transO) + inx * std::sin(transO);
        z = transZ + inz;
    }

    static void CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
    {
        if (o)
            *o = sTransportHandler.normalizeOrientation(*o - transO);

        z -= transZ;
        y -= transY;
        x -= transX;
        float inx = x, iny = y;
        y = (iny - inx * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
        x = (inx + iny * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
    }
};

class SERVER_DECL Transporter : public GameObject, public TransportBase
{
    friend Transporter* TransportHandler::createTransport(uint32, MapMgr*);
    Transporter(uint64 guid);

public:
    typedef std::set<Object*> PassengerSet;
    ~Transporter();

    // Creates The Transporter
    bool Create(uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress);

    // Update Transporter Position and Transport Passengers
    void Update(unsigned long /*time_passed*/);

    // Populate Transporters with Creatures and Gameobjects
    void LoadStaticPassengers();

    // Remove Creatures and Gameobjects from Transporter
    void UnloadStaticPassengers();

    // Add Passenger to Transporter
    void AddPassenger(Player* passenger);

    // Remove Passenger from Transporter ( could be an Creature or Gameobject or even a Player)
    void RemovePassenger(Object* passenger);

    PassengerSet const& GetPassengers() const { return _passengers; }
    Creature* createNPCPassenger(MySQLStructure::CreatureSpawn* data);
    GameObject* createGOPassenger(MySQLStructure::GameobjectSpawn* data);

    // Build Update for Player
    uint32  buildCreateUpdateBlockForPlayer(ByteBuffer* data, Player* target);
    void UpdateForMap(MapMgr* map);

    // This method transforms supplied transport offsets into global coordinates
    void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const override
    {
        TransportBase::CalculatePassengerPosition(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    }

    // This method transforms supplied global coordinates into local offsets
    void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const override
    {
        TransportBase::CalculatePassengerOffset(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
    }

    uint32_t getTransportPeriod() const override { return getLevel(); }
    void SetPeriod(uint32 period) { setLevel(period); }
    uint32 GetTimer() const { return mTransValues.PathProgress; }

    KeyFrameVec const& GetKeyFrames() const { return _transportInfo->keyFrames; }

    void UpdatePosition(float x, float y, float z, float o);

    void EnableMovement(bool enabled, MapMgr* instance);

    TransportTemplate const* GetTransportTemplate() const { return _transportInfo; }

    uint32_t getCurrentFrame() { return _currentFrame->Index; }

private:
    void MoveToNextWaypoint();
    float CalculateSegmentPos(float perc);

    // Occours when Transport reaches Teleport Frame
    bool TeleportTransport(uint32_t newMapId, float x, float y, float z, float o);
    void DelayedTeleportTransport(MapMgr* oldMap);

    // Helper to Port Players
    void TeleportPlayers(float x, float y, float z, float o, uint32_t newMapId, uint32_t oldMapId, bool newMap);

    // Update all Passenger Positions
    void UpdatePassengerPositions(PassengerSet& passengers);

    void DoEventIfAny(KeyFrame const& node, bool departure);

    // Helpers to know if stop frame was reached
    bool IsMoving() const { return _isMoving; }
    void SetMoving(bool val) { _isMoving = val; }

    TransportTemplate const* _transportInfo;

    KeyFrameVec::const_iterator _currentFrame;
    KeyFrameVec::const_iterator _nextFrame;
    bool _isMoving;
    bool _pendingStop;

    // These are needed to properly control events triggering only once for each frame
    bool _triggeredArrivalEvent;
    bool _triggeredDepartureEvent;

    PassengerSet _passengers;
    PassengerSet::iterator _passengerTeleportItr;
    PassengerSet _staticPassengers;

    int32_t _positionChangeTimer = 0;
    int32_t _mapUpdateTimer = 0;

    uint32_t positionUpdateDelay;
    bool _delayedTeleport = false;
};