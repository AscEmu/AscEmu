/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/


#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Movement/WaypointManager.h"

//.waypoint add
bool ChatHandler::HandleWayPointAddCommand(const char* args, WorldSession* m_session)
{

    AIInterface* ai = nullptr;
    Creature* creature_target = nullptr;
    Player* player = m_session->GetPlayer();

    if (player->m_aiInterfaceWaypoint != nullptr)
    {
        SystemMessage(m_session, "Using Previous Unit.");
        ai = player->m_aiInterfaceWaypoint;
        if (!ai)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        creature_target = static_cast<Creature*>(ai->getUnit());
        if (creature_target == nullptr || creature_target->isPet())
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }
    }
    else
    {
        creature_target = GetSelectedCreature(m_session, true);
        if (creature_target == nullptr)
            return true;

        ai = creature_target->GetAIInterface();
    }

    char* pWaitTime = strtok((char*)args, " ");
    uint32 WaitTime = (pWaitTime) ? atoi(pWaitTime) : 10000;

    char* pFlags = strtok(NULL, " ");
    uint32 Flags = (pFlags) ? atoi(pFlags) : 0;

    bool showing = ai->isShowWayPointsActive();

    WaypointNode waypoint;
    waypoint.id = uint32(ai->getWayPointsCount() + 1);
    waypoint.x = player->GetPositionX();
    waypoint.y = player->GetPositionY();
    waypoint.z = player->GetPositionZ();
    waypoint.orientation = player->GetOrientation();
    waypoint.moveType = Flags;

    if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
    {
        sLogger.failure("Waypoint %u has invalid move_type, setting default", waypoint.id);
        waypoint.moveType = WAYPOINT_MOVE_TYPE_WALK;
    }

    waypoint.delay = WaitTime;
    waypoint.eventId = 0;
    waypoint.eventChance = 0;

    if (showing)
        ai->hideWayPoints(player);

    sWaypointMgr->addWayPoint(creature_target->getWaypointPath() ,waypoint);
    SystemMessage(m_session, "Waypoint %u added to Creature %s.", waypoint.id, creature_target->GetCreatureProperties()->Name.c_str());


    if (showing)
        ai->activateShowWayPoints(player, false);

    return true;
}

//.waypoint delete
bool ChatHandler::HandleWayPointDeleteCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);
    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->m_aiInterfaceWaypoint;
    if (ai == nullptr || !ai->getUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    if (wpid)
    {
        bool show = ai->isShowWayPointsActive();
        if (show == true)
            ai->hideWayPoints(player);

        sWaypointMgr->deleteWayPointById(ai->getUnit()->ToCreature()->getWaypointPath(), wpid);

        SystemMessage(m_session, "Waypoint %u deleted.", wpid);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
    }
    return true;
}

//.waypoint deleteall
bool ChatHandler::HandleWayPointDeleteAllCommand(const char* /*args*/, WorldSession* m_session)
{
    Creature* creature_target = GetSelectedCreature(m_session, true);
    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->m_aiInterfaceWaypoint;
    if (creature_target == nullptr || !creature_target->GetSQL_id())
        return true;

    bool show = ai->isShowWayPointsActive();
    if (show == true)
        ai->hideWayPoints(player);

    sWaypointMgr->deleteAllWayPoints(creature_target->getWaypointPath());
    return true;
}
//.waypoint hide
bool ChatHandler::HandleWayPointHideCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    AIInterface* ai = creature_target->GetAIInterface();
    Player* player = m_session->GetPlayer();

    if (player->m_aiInterfaceWaypoint == ai)
    {
        if (ai->isShowWayPointsActive() == true)
            player->m_aiInterfaceWaypoint->hideWayPoints(player);

        player->m_aiInterfaceWaypoint = nullptr;
    }
    else
    {
        SystemMessage(m_session, "Waypoints for that Unit are not Visible.");
        return true;
    }

    SystemMessage(m_session, "Hiding Waypoints for creature_spawn %u", creature_target->GetSQL_id());
    return true;
}

//.waypoint show
bool ChatHandler::HandleWayPointShowCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    char* pBackwards = strtok((char*)args, " ");
    bool Backwards = (pBackwards) ? ((atoi(pBackwards) > 0) ? true : false) : false;

    AIInterface* ai = creature_target->GetAIInterface();
    Player* player = m_session->GetPlayer();
    if (player->m_aiInterfaceWaypoint != ai)
    {
        if (ai->isShowWayPointsActive() == true)
        {
            RedSystemMessage(m_session, "Some one else is also Viewing this Creatures WayPoints.");
            RedSystemMessage(m_session, "Viewing WayPoints at the same time as some one else can cause undesireble results.");
            return true;
        }

        if (player->m_aiInterfaceWaypoint != nullptr)
            player->m_aiInterfaceWaypoint->hideWayPoints(player);

        player->m_aiInterfaceWaypoint = ai;
        ai->activateShowWayPoints(player, Backwards);
    }
    else
    {
        if (ai->isShowWayPointsActive() == true)
        {
            SystemMessage(m_session, "Waypoints Already Showing.");
        }
        else
            ai->activateShowWayPoints(m_session->GetPlayer(), Backwards);
    }

    SystemMessage(m_session, "Showing waypoints for creature %u", creature_target->GetSQL_id());
    return true;
}
