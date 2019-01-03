/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LocationVector.h"

class Unit;

class MovementAI
{
    LocationVector m_destination;
    uint32_t m_destination_time;
    unsigned long m_diff;
    Unit* m_follow_target;
    bool m_moving;
    LocationVector m_origin;
    uint32_t m_origin_time;
    Unit* m_owner;
    unsigned long m_update_frequency;

    LocationVector calculateCurrentPosition() const;
    uint32_t calculateExpectedMoveTime() const;
    uint32_t calculateTimeToReachPoint(float speed, LocationVector point) const;
    bool isMoving() const;
    void sendMovePacket();
public:
    MovementAI(Unit* owner);

    void moveTo(LocationVector destination);
    void startFollowing(Unit* followTarget);
    void stopFollowing();
    void stopMoving(bool interrupt);
    void updateMovement(unsigned long diff);
};