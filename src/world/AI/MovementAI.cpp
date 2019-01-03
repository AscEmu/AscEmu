/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "MovementAI.h"
#include "WorldPacket.h"
#include "Units/Unit.h"

uint32_t MovementAI::calculateTimeToReachPoint(float speed, LocationVector point) const
{
    const auto distance = m_owner->GetPosition().Distance(point);
    return uint32_t((distance / speed) * 1000);
}

bool MovementAI::isMoving() const
{
    return m_moving || m_follow_target;
}

MovementAI::MovementAI(Unit * owner) :
    m_destination(),
    m_destination_time(0),
    m_diff(0),
    m_follow_target(nullptr),
    m_moving(false),
    m_origin(),
    m_origin_time(0),
    m_owner(owner),
    m_update_frequency(400)
{
}

void MovementAI::moveTo(LocationVector destination)
{
    m_origin = m_owner->GetPosition();
    m_origin_time = Util::getMSTime();
    m_destination = destination;
    m_destination_time = m_origin_time + calculateTimeToReachPoint(m_owner->getRunSpeed(), destination);
    m_diff = 0;
    m_moving = true;
    sendMovePacket();
}

void MovementAI::startFollowing(Unit * followTarget)
{
    m_follow_target = followTarget;
    moveTo(followTarget->GetPosition());
}

void MovementAI::stopFollowing()
{
    m_follow_target = nullptr;
}

void MovementAI::updateMovement(unsigned long diff)
{
    if (!m_moving && !m_follow_target)
    {
        return;
    }

    if (!m_moving)
    {
        if (m_follow_target->GetPosition().Distance(m_owner->GetPosition()) > 2.5f)
        {
            moveTo(m_follow_target->GetPosition());
            return;
        }
    }

    if (m_follow_target->GetPosition().Distance(m_destination) > 2.5f)
    {
        moveTo(m_follow_target->GetPosition());
        return;
    }

    m_diff += diff;
    if (m_diff < m_update_frequency)
    {
        return;
    }

    LocationVector currentPosition = calculateCurrentPosition();
    m_owner->setLocationWithoutUpdate(currentPosition);

    if (m_owner->GetPosition().Distance(m_destination) < 2.f)
    {
        stopMoving(false);
        // This is temporary, TODO: be able to update position without sending update packet
        m_owner->setLocationWithoutUpdate(m_destination);
        return;
    }
}

LocationVector MovementAI::calculateCurrentPosition() const
{
    if (!m_moving)
    {
        return m_owner->GetPosition();
    }

    const auto pct = float(m_diff) / float(calculateExpectedMoveTime());

    LOG_ERROR("Pct: %f, dest[%f, %f, %f], origin[%f, %f, %f]", pct, m_destination.x, m_destination.y, m_destination.z, m_origin.x, m_origin.y, m_origin.z);

    LocationVector current_position;
    current_position.x = m_destination.x - ((m_destination.x - m_origin.x) * (pct == 0 ? 0.00001f : pct));
    current_position.y = m_destination.y - ((m_destination.y - m_origin.y) * (pct == 0 ? 0.00001f : pct));
    current_position.z = m_destination.z - ((m_destination.z - m_origin.z) * (pct == 0 ? 0.00001f : pct));
    return current_position;
}

uint32_t MovementAI::calculateExpectedMoveTime() const
{
    const auto time = m_destination_time - m_origin_time;
    // We use this in a percentage calculation, so make sure we're not dividing by zero
    return time == 0 ? 1 : time;
}

void MovementAI::sendMovePacket()
{
    const auto start = m_owner->GetPosition();

    auto spline = m_owner->m_movementManager.m_spline;
    auto midpoints = m_owner->m_movementManager.m_spline.GetMidPoints();

    WorldPacket data(SMSG_MONSTER_MOVE, 60);
    data << m_owner->GetNewGUID();
    data << float(m_destination.x);
    data << float(m_destination.y);
    data << float(m_destination.z);
    data << uint32(m_origin_time);
    // Id of first spline, so always 0
    data << uint8(0);

    data << uint32(0x1000); //move flags: run
    data << uint32_t(calculateExpectedMoveTime()); //movetime
    data << uint32(1); //1 point
    data << float(m_destination.x);
    data << float(m_destination.y);
    data << float(m_destination.z);

    m_owner->SendMessageToSet(&data, true);
}

void MovementAI::stopMoving(bool /*interrupt*/)
{
    m_moving = false;
    m_origin_time = 0;
    m_destination_time = 0;
}
