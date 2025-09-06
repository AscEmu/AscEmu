/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Logging/Logger.hpp"
#include "Movement/MovementManager.h"
#include "Movement/WaypointManager.h"
#include "Objects/Units/Creatures/AIInterface.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/WorldSession.h"

//.waypoint add
bool ChatCommandHandler::HandleWayPointAddCommand(const char* args, WorldSession* m_session)
{
    AIInterface* ai = nullptr;
    Creature* creature_target = nullptr;
    Player* player = m_session->GetPlayer();

    if (player->m_aiInterfaceWaypoint != nullptr)
    {
        systemMessage(m_session, "Using Previous Unit.");
        ai = player->m_aiInterfaceWaypoint;
        if (!ai)
        {
            systemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        creature_target = static_cast<Creature*>(ai->getUnit());
        if (creature_target == nullptr || creature_target->isPet())
        {
            systemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }
    }
    else
    {
        creature_target = GetSelectedCreature(m_session, true);
        if (creature_target == nullptr)
            return true;

        ai = creature_target->getAIInterface();
    }

    uint32_t pathId = creature_target->getWaypointPath();

    char* pWaitTime = strtok((char*)args, " ");
    uint32_t WaitTime = (pWaitTime) ? atoi(pWaitTime) : 10000;

    char* pFlags = strtok(nullptr, " ");
    uint32_t Flags = (pFlags) ? atoi(pFlags) : 0;

    bool showing = ai->isShowWayPointsActive();

    if (!pathId)
    {
        pathId = sWaypointMgr->generateWaypointPathId();
        creature_target->loadPath(pathId);
        sLogger.debug("New Waypoint Path {} Startet for Creature {}.", pathId, creature_target->getSpawnId());

        // Start Movement
        creature_target->setDefaultMovementType(WAYPOINT_MOTION_TYPE);
        creature_target->getMovementManager()->movePath(pathId, true);

        WorldDatabase.Execute("UPDATE creature_spawns SET movetype = %u, waypoint_group = %u WHERE id = %u AND min_build <= %u AND max_build >= %u", WAYPOINT_MOTION_TYPE, pathId, creature_target->spawnid, VERSION_STRING, VERSION_STRING);
    }

    WaypointNode waypoint;
    waypoint.id = uint32_t(ai->getWayPointsCount() + 1);
    waypoint.x = player->GetPositionX();
    waypoint.y = player->GetPositionY();
    waypoint.z = player->GetPositionZ();
    waypoint.orientation = player->GetOrientation();
    waypoint.moveType = Flags;

    if (waypoint.moveType >= WAYPOINT_MOVE_TYPE_MAX)
    {
        sLogger.failure("Waypoint {} has invalid move_type, setting default", waypoint.id);
        waypoint.moveType = WAYPOINT_MOVE_TYPE_WALK;
    }

    waypoint.delay = WaitTime;
    waypoint.eventId = 0;
    waypoint.eventChance = 0;

    if (showing)
        ai->hideWayPoints(player);

    // Save Our New Waypoint
    sWaypointMgr->addWayPoint(pathId, waypoint, true);
    systemMessage(m_session, "Waypoint {} added to Creature {}.", waypoint.id, creature_target->GetCreatureProperties()->Name);

    if (showing)
        ai->activateShowWayPoints(player, false);

    return true;
}

//.waypoint delete
bool ChatCommandHandler::HandleWayPointDeleteCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32_t wpid = GetSelectedWayPointId(m_session);
    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->m_aiInterfaceWaypoint;
    if (ai == nullptr || !ai->getUnit())
    {
        systemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    if (wpid)
    {
        bool show = ai->isShowWayPointsActive();
        if (show == true)
            ai->hideWayPoints(player);

        sWaypointMgr->deleteWayPointById(ai->getUnit()->ToCreature()->getWaypointPath(), wpid);

        systemMessage(m_session, "Waypoint {} deleted.", wpid);
    }
    else
    {
        systemMessage(m_session, "Invalid Waypoint.");
    }
    return true;
}

//.waypoint deleteall
bool ChatCommandHandler::HandleWayPointDeleteAllCommand(const char* /*args*/, WorldSession* m_session)
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
bool ChatCommandHandler::HandleWayPointHideCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    AIInterface* ai = creature_target->getAIInterface();
    Player* player = m_session->GetPlayer();

    if (player->m_aiInterfaceWaypoint == ai)
    {
        if (ai->isShowWayPointsActive() == true)
            player->m_aiInterfaceWaypoint->hideWayPoints(player);

        player->m_aiInterfaceWaypoint = nullptr;
    }
    else
    {
        systemMessage(m_session, "Waypoints for that Unit are not Visible.");
        return true;
    }

    systemMessage(m_session, "Hiding Waypoints for creature_spawn {}", creature_target->GetSQL_id());
    return true;
}

//.waypoint show
bool ChatCommandHandler::HandleWayPointShowCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    char* pBackwards = strtok((char*)args, " ");
    bool Backwards = (pBackwards) ? ((atoi(pBackwards) > 0) ? true : false) : false;

    AIInterface* ai = creature_target->getAIInterface();
    Player* player = m_session->GetPlayer();
    if (player->m_aiInterfaceWaypoint != ai)
    {
        if (ai->isShowWayPointsActive() == true)
        {
            redSystemMessage(m_session, "Some one else is also Viewing this Creatures WayPoints.");
            redSystemMessage(m_session, "Viewing WayPoints at the same time as some one else can cause undesireble results.");
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
            systemMessage(m_session, "Waypoints Already Showing.");
        else
            ai->activateShowWayPoints(m_session->GetPlayer(), Backwards);
    }

    systemMessage(m_session, "Showing waypoints for creature {}", creature_target->GetSQL_id());
    return true;
}
