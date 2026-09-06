/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "TransportBase.hpp"
#include "Management/TransporterHandler.hpp"
#include "Objects/GameObject.h"

namespace MySQLStructure
{
    struct CreatureSpawn;
}

class SERVER_DECL Transporter : public GameObject, public TransportBase
{
    friend Transporter* ObjectFactory::createTransporter(uint32_t, uint32_t, const LocationVector&);
    Transporter(uint64_t guid);

public:
    typedef std::set<Object*> PassengerSet;
    ~Transporter();

    //////////////////////////////////////////////////////////////////////////////////////////
    // Essential functions
    //virtual void onPreAttachToWorld() override;
    virtual void onAttachToWorld() override;

    virtual void onPreDetachFromWorld() override;
    //virtual void onDetachFromWorld() override;

    // Creates The Transporter
    bool Create(uint32_t entry, uint32_t mapid, float x, float y, float z, float ang, uint8_t animprogress);

    // Update Transporter Position and Transport Passengers
    void Update(unsigned long /*time_passed*/);
    void delayedUpdate(unsigned long /*time_passed*/);

    void setLastUpdate(std::chrono::steady_clock::time_point tp) { lastUpdate_ = tp; }
    auto getLastUpdate() const { return lastUpdate_; }

    // Populate Transporters with Creatures and Gameobjects
    void LoadStaticPassengers();

    // Remove Creatures and Gameobjects from Transporter
    void UnloadStaticPassengers();

    // Add a dynamic passenger using transport-local coordinates.
    void AddPassenger(Object* passenger, const LocationVector& transportOffset);

    // Restore the transport relationship after a cross-map player teleport without
    // firing board/disembark script hooks a second time.
    void RestorePassengerAfterTeleport(Object* passenger, const LocationVector& transportOffset);

    // Remove Passenger from Transporter ( could be an Creature or Gameobject or even a Player)
    void RemovePassenger(Object* passenger);

    PassengerSet const& GetPassengers() const { return _passengers; }
    Creature* createNPCPassenger(MySQLStructure::CreatureSpawn* data);
    GameObject* createGOPassenger(MySQLStructure::GameobjectSpawn* data);

    // Build Update for Player
    void UpdateForMap(WorldMap* map);

    // Removes transport from map
    void removeFromMap();

    // This method transforms supplied transport offsets into global coordinates
    void calculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) override;

    // This method transforms supplied global coordinates into local offsets
    void calculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) override;

    uint32_t getTransportPeriod() const override { return getLevel(); }
    void SetPeriod(uint32_t period) { setLevel(period); }
    uint32_t GetTimer() const { return m_goValue.PathProgress; }

    KeyFrameVec const& GetKeyFrames() const { return _transportInfo->keyFrames; }

    void UpdatePosition(float x, float y, float z, float o);

    void EnableMovement(bool enabled, WorldMap* instance);

    void setDelayedAddModelToMap() { _delayedAddModel = true; }

    TransportTemplate const* GetTransportTemplate() const { return _transportInfo; }

    uint32_t getCurrentFrame() { return _currentFrame->Index; }

    void delayedTeleportTransport();

    // Helper to Port Players
    void TeleportPlayers(float x, float y, float z, float o, uint32_t newMapId, uint32_t oldMapId, bool newMap, WorldMap* targetMap = nullptr);

    bool isTransporter() const override { return true; }

private:
    void MoveToNextWaypoint();
    float CalculateSegmentPos(float perc);

    // Occours when Transport reaches Teleport Frame
    bool TeleportTransport(uint32_t newMapId, float x, float y, float z, float o);

    // Update all Passenger Positions
    void UpdatePassengerPositions(PassengerSet& passengers);
    void UpdatePlayerPositions(PassengerSet& passengers);

    void DoEventIfAny(KeyFrame const& node, bool departure);

    void updatePathProgress();

    // Helpers to know if stop frame was reached
    bool IsMoving() const { return _isMoving; }
    void SetMoving(bool val) { _isMoving = val; }

    bool _delayedAddModel = false;

    TransportTemplate const* _transportInfo = nullptr;

    KeyFrameVec::const_iterator _currentFrame;
    KeyFrameVec::const_iterator _nextFrame;
    bool _isMoving = true;
    bool _pendingStop = false;

    // These are needed to properly control events triggering only once for each frame
    bool _triggeredArrivalEvent = false;
    bool _triggeredDepartureEvent = false;

    PassengerSet _passengers;
    PassengerSet::iterator _passengerTeleportItr;
    PassengerSet _staticPassengers;

    int32_t _positionChangeTimer = 100;
    int32_t _mapUpdateTimer = 0;

    uint32_t positionUpdateDelay = 100;
    bool _delayedTeleport = false;

    std::chrono::steady_clock::time_point lastUpdate_{ std::chrono::steady_clock::now() };
};
