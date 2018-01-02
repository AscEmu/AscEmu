/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"

//.waypoint add
bool ChatHandler::HandleWayPointAddCommand(const char* args, WorldSession* m_session)
{
    AIInterface* ai = nullptr;
    Creature* creature_target = nullptr;
    Player* player = m_session->GetPlayer();

    if (player->waypointunit != nullptr)
    {
        SystemMessage(m_session, "Using Previous Unit.");
        ai = player->waypointunit;
        if (!ai)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        creature_target = static_cast<Creature*>(ai->GetUnit());
        if (creature_target == nullptr || creature_target->IsPet())
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

    char* pForwardEmoteId = strtok(NULL, " ");
    uint32 ForwardEmoteId = (pForwardEmoteId) ? atoi(pForwardEmoteId) : 0;

    char* pBackwardEmoteId = strtok(NULL, " ");
    uint32 BackwardEmoteId = (pBackwardEmoteId) ? atoi(pBackwardEmoteId) : 0;

    char* pForwardSkinId = strtok(NULL, " ");
    uint32 ForwardSkinId = (pForwardSkinId) ? atoi(pForwardSkinId) : creature_target->GetNativeDisplayId();

    char* pBackwardSkinId = strtok(NULL, " ");
    uint32 BackwardSkinId = (pBackwardSkinId) ? atoi(pBackwardSkinId) : creature_target->GetNativeDisplayId();

    char* pForwardEmoteOneShot = strtok(NULL, " ");
    uint32 ForwardEmoteOneShot = (pForwardEmoteOneShot) ? atoi(pForwardEmoteOneShot) : 1;

    char* pBackwardEmoteOneShot = strtok(NULL, " ");
    uint32 BackwardEmoteOneShot = (pBackwardEmoteOneShot) ? atoi(pBackwardEmoteOneShot) : 1;

    Movement::WayPoint* waypoint = new Movement::WayPoint;
    bool showing = ai->isShowWayPointsActive();
    waypoint->id = uint32(ai->getWayPointsCount() + 1);
    waypoint->x = player->GetPositionX();
    waypoint->y = player->GetPositionY();
    waypoint->z = player->GetPositionZ();
    waypoint->waittime = WaitTime;
    waypoint->flags = Flags;
    waypoint->forwardemoteoneshot = ForwardEmoteOneShot > 0 ? true : false;
    waypoint->forwardemoteid = ForwardEmoteId;
    waypoint->backwardemoteoneshot = BackwardEmoteOneShot > 0 ? true : false;
    waypoint->backwardemoteid = BackwardEmoteId;
    waypoint->forwardskinid = ForwardSkinId;
    waypoint->backwardskinid = BackwardSkinId;

    if (showing)
        ai->hideWayPoints(player);

    if (ai->addWayPointUnsafe(waypoint))
    {
        ai->saveWayPoints();
        SystemMessage(m_session, "Waypoint %u added to Creature %s.", waypoint->id, creature_target->GetCreatureProperties()->Name.c_str());
    }
    else
    {
        SystemMessage(m_session, "An error occurred while adding the Waypoint");
        delete waypoint;
    }

    if (showing)
        ai->activateShowWayPoints(player, ai->isShowWayPointsBackwardsActive());

    return true;
}

//.waypoint addfly
bool ChatHandler::HandleWayPointAddFlyCommand(const char* args, WorldSession* m_session)
{
    AIInterface* ai = nullptr;
    Creature* creature_target = nullptr;
    Player* player = m_session->GetPlayer();
    if (player->waypointunit != nullptr)
    {
        SystemMessage(m_session, "Using Previous Unit.");
        ai = player->waypointunit;
        if (!ai)
        {
            SystemMessage(m_session, "Invalid Creature, please select another one.");
            return true;
        }

        creature_target = static_cast<Creature*>(ai->GetUnit());
        if (creature_target == nullptr || creature_target->IsPet())
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

    char* pForwardEmoteId = strtok(NULL, " ");
    uint32 ForwardEmoteId = (pForwardEmoteId) ? atoi(pForwardEmoteId) : 0;

    char* pBackwardEmoteId = strtok(NULL, " ");
    uint32 BackwardEmoteId = (pBackwardEmoteId) ? atoi(pBackwardEmoteId) : 0;

    char* pForwardSkinId = strtok(NULL, " ");
    uint32 ForwardSkinId = (pForwardSkinId) ? atoi(pForwardSkinId) : creature_target->GetNativeDisplayId();

    char* pBackwardSkinId = strtok(NULL, " ");
    uint32 BackwardSkinId = (pBackwardSkinId) ? atoi(pBackwardSkinId) : creature_target->GetNativeDisplayId();

    char* pForwardEmoteOneShot = strtok(NULL, " ");
    uint32 ForwardEmoteOneShot = (pForwardEmoteOneShot) ? atoi(pForwardEmoteOneShot) : 1;

    char* pBackwardEmoteOneShot = strtok(NULL, " ");
    uint32 BackwardEmoteOneShot = (pBackwardEmoteOneShot) ? atoi(pBackwardEmoteOneShot) : 1;

    Movement::WayPoint* waypoint = new Movement::WayPoint;
    bool showing = ai->isShowWayPointsActive();
    waypoint->id = (uint32)ai->getWayPointsCount() + 1;
    waypoint->x = player->GetPositionX();
    waypoint->y = player->GetPositionY();
    waypoint->z = player->GetPositionZ();
    waypoint->waittime = WaitTime;
    waypoint->flags = 768;
    waypoint->forwardemoteoneshot = (ForwardEmoteOneShot > 0) ? true : false;
    waypoint->forwardemoteid = ForwardEmoteId;
    waypoint->backwardemoteoneshot = (BackwardEmoteOneShot > 0) ? true : false;
    waypoint->backwardemoteid = BackwardEmoteId;
    waypoint->forwardskinid = ForwardSkinId;
    waypoint->backwardskinid = BackwardSkinId;

    if (showing)
        ai->hideWayPoints(player);

    if (ai->addWayPointUnsafe(waypoint))
    {
        ai->saveWayPoints();
        SystemMessage(m_session, "Fly waypoint %u added to Creature %s.", waypoint->id, creature_target->GetCreatureProperties()->Name.c_str());
    }
    else
    {
        SystemMessage(m_session, "An error occurred while adding the Waypoint");
        delete waypoint;
    }

    if (showing)
        ai->activateShowWayPoints(player, ai->isShowWayPointsBackwardsActive());

    return true;
}

//.waypoint change
bool ChatHandler::HandleWayPointChangeNumberCommand(const char* args, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);
    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    char* pNewID = strtok((char*)args, " ");
    uint32 NewID = (pNewID) ? atoi(pNewID) : 0;

    if (NewID == wpid)
        return true;

    if (wpid)
    {
        bool show = ai->isShowWayPointsActive();
        if (show == true)
            ai->hideWayPoints(player);

        ai->changeWayPointId(wpid, NewID);

        if (show == true)
            ai->activateShowWayPoints(player, ai->isShowWayPointsBackwardsActive());

        SystemMessage(m_session, "Waypoint %u changed to %u", wpid, NewID);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint selection.");
        return true;
    }

    return true;
}

//.waypoint delete
bool ChatHandler::HandleWayPointDeleteCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);
    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    if (wpid)
    {
        bool show = ai->isShowWayPointsActive();
        if (show == true)
            ai->hideWayPoints(player);

        ai->deleteWayPointById(wpid);

        if (show == true)
            ai->activateShowWayPoints(player, ai->isShowWayPointsBackwardsActive());

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
    AIInterface* ai = player->waypointunit;
    if (creature_target == nullptr || !creature_target->GetSQL_id())
        return true;

    bool show = ai->isShowWayPointsActive();
    if (show == true)
        ai->hideWayPoints(player);

    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid=%u", creature_target->GetSQL_id());

    creature_target->GetAIInterface()->deleteAllWayPoints();
    SystemMessage(m_session, "Deleted waypoints for creature_spawn %u", creature_target->GetSQL_id());

    return true;
}

//.waypoint emote
bool ChatHandler::HandleWayPointEmoteCommand(const char* args, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    uint32 EmoteId = 0;
    bool OneShot = true;

    if (wpid)
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint != nullptr)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards) ? atoi(pBackwards) : 0;
            char* pEmoteId = strtok(NULL, " ");
            EmoteId = (pEmoteId) ? atoi(pEmoteId) : 0;
            char* pOneShot = strtok(NULL, " ");
            OneShot = (pOneShot) ? ((atoi(pOneShot) > 0) ? true : false) : 1;
            if (Backwards)
            {
                waypoint->backwardemoteid = EmoteId;
                waypoint->backwardemoteoneshot = OneShot;
            }
            else
            {
                waypoint->forwardemoteid = EmoteId;
                waypoint->forwardemoteoneshot = OneShot;
            }

            //save wp
            ai->saveWayPoints();
        }

        SystemMessage(m_session, "EmoteID for Waypoint %u is now %u amd oneshot is set to %u", wpid, EmoteId, uint32(OneShot));
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
    }

    return true;
}

//.waypoint flags
bool ChatHandler::HandleWayPointFlagsCommand(const char* args, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    if (wpid)
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint == nullptr)
        {
            SystemMessage(m_session, "Invalid Waypoint.");
            return true;
        }

        uint32 flags = waypoint->flags;

        char* pNewFlags = strtok((char*)args, " ");
        uint32 NewFlags = (pNewFlags) ? atoi(pNewFlags) : 0;

        waypoint->flags = NewFlags;

        ai->saveWayPoints();

        SystemMessage(m_session, "Waypoint %u changed flags from %u to %u", wpid, flags, NewFlags);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");

    }

    return true;
}

//.waypoint generate
bool ChatHandler::HandleWayPointGenerateCommand(const char* args, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    if (creature_target->GetAIInterface()->getWayPointsCount())
    {
        SystemMessage(m_session, "The creature already has waypoints");
        return false;
    }

    if (m_session->GetPlayer()->waypointunit != nullptr)
    {
        SystemMessage(m_session, "You are already showing waypoints, hide them first.");
        return true;
    }

    if (!creature_target->GetSQL_id())
        return false;

    char* pR = strtok((char*)args, " ");
    if (!pR)
    {
        SystemMessage(m_session, "Randomly generate wps params: range count");
        return true;
    }

    uint32 range = atoi(pR);
    char* pC = strtok(NULL, " ");
    if (!pC)
    {
        SystemMessage(m_session, "Randomly generate wps params: waypoint count");
        return true;
    }
    uint8 wp_count = static_cast<uint8>(atol(pC));

    for (uint8 i = 0; i < wp_count; ++i)
    {
        if (range < 1)
        {
            SystemMessage(m_session, "Usage: waypoint range must be 1 or higher");
            return true;
        }
        float ang = Util::getRandomFloat(100.0f);
        float ran = (range < 2 ? 1.0f : Util::getRandomFloat(float(range)));
        while (ran < 1.0f)
        {
            ran = Util::getRandomFloat(float(range));
        }

        float x = creature_target->GetPositionX() + ran * sin(ang);
        float y = creature_target->GetPositionY() + ran * cos(ang);
        float z = creature_target->GetMapMgr()->GetLandHeight(x, y, creature_target->GetPositionZ() + 3);

        Movement::WayPoint* wp = new Movement::WayPoint;
        wp->id = (uint32)creature_target->GetAIInterface()->getWayPointsCount() + 1;
        wp->x = x;
        wp->y = y;
        wp->z = z;
        wp->waittime = 5000;
        wp->flags = 0;
        wp->forwardemoteoneshot = false;
        wp->forwardemoteid = 0;
        wp->backwardemoteoneshot = false;
        wp->backwardemoteid = 0;
        wp->forwardskinid = 0;
        wp->backwardskinid = 0;

        creature_target->GetAIInterface()->addWayPoint(wp);
    }

    if (creature_target->m_spawn && creature_target->m_spawn->movetype != 1)
    {
        creature_target->m_spawn->movetype = 1;
        creature_target->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_RANDOMWP);
        WorldDatabase.Execute("UPDATE creature_spawns SET movetype = 1 WHERE id = %u", creature_target->GetSQL_id());
    }

    m_session->GetPlayer()->waypointunit = creature_target->GetAIInterface();
    creature_target->GetAIInterface()->activateShowWayPoints(m_session->GetPlayer(), creature_target->GetAIInterface()->isShowWayPointsBackwardsActive());

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

    if (player->waypointunit == ai)
    {
        if (ai->isShowWayPointsActive() == true)
            player->waypointunit->hideWayPoints(player);

        player->waypointunit = nullptr;
    }
    else
    {
        SystemMessage(m_session, "Waypoints for that Unit are not Visible.");
        return true;
    }

    SystemMessage(m_session, "Hiding Waypoints for creature_spawn %u", creature_target->GetSQL_id());

    return true;
}

//.waypoint info
bool ChatHandler::HandleWayPointInfoCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    std::stringstream ss;

    if ((wpid > 0) && (wpid <= ai->getWayPointsCount()))
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint != nullptr)
        {
            ss << "Waypoint Number " << waypoint->id << ":\n";
            ss << "WaitTime: " << waypoint->waittime << "\n";
            ss << "Flags: " << waypoint->flags;
            if (waypoint->flags == 768)
                ss << " (Fly)\n";
            else if (waypoint->flags == 256)
                ss << " (Run)\n";
            else
                ss << " (Walk)\n";
            ss << "Backwards\n";
            ss << "   emoteid: " << waypoint->backwardemoteid << "\n";
            ss << "   oneshot: " << ((waypoint->backwardemoteoneshot == 1) ? "Yes" : "No") << "\n";
            ss << "   skinid: " << waypoint->backwardskinid << "\n";
            ss << "Forwards\n";
            ss << "   emoteid: " << waypoint->forwardemoteid << "\n";
            ss << "   oneshot: " << ((waypoint->forwardemoteoneshot == 1) ? "Yes" : "No") << "\n";
            ss << "   skinid: " << waypoint->forwardskinid << "\n";
            SendMultilineMessage(m_session, ss.str().c_str());
        }
    }
    else
    {
        SystemMessage(m_session, "Selected Waypoint is invalid!");
        return true;
    }
    return true;
}

//.waypoint movehere
bool ChatHandler::HandleWayPpointMoveHereCommand(const char* /*args*/, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    if (wpid)
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint != nullptr)
        {
            waypoint->x = player->GetPositionX();
            waypoint->y = player->GetPositionY();
            waypoint->z = player->GetPositionZ();

            ai->saveWayPoints();
        }

        if (ai->isShowWayPointsActive() == true)
        {
            ai->hideWayPoints(player);
            ai->activateShowWayPoints(player, ai->isShowWayPointsBackwardsActive());
        }

        SystemMessage(m_session, "Waypoint %u has been moved to your position.", wpid);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
    }

    return true;
}

//.waypoint movetype
bool ChatHandler::HandleWayPointMoveTypeCommand(const char* args, WorldSession* m_session)
{
    uint32 option = atoi((char*)args);

    if (option > 2 || !*args)
    {
        RedSystemMessage(m_session, "No movetype entered!");
        SystemMessage(m_session, "0 is Move from WP 1 ->  10 then 10 -> 1.");
        SystemMessage(m_session, "1 is Move from WP to a random WP.");
        SystemMessage(m_session, "2 is Move from WP 1 -> 10 then 1 -> 10.");
        return true;
    }

    Creature* creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr)
        return true;

    WorldDatabase.Execute("UPDATE creature_spawns SET movetype = '%u' WHERE id = '%u'", option, creature_target->spawnid);

    creature_target->GetAIInterface()->setWaypointScriptType((Movement::WaypointMovementScript)option);

    SystemMessage(m_session, "Value saved to database.");

    return true;
}

//.waypoint save
bool ChatHandler::HandleWayPointSaveCommand(const char* /*args*/, WorldSession* m_session)
{
    auto creature_target = GetSelectedCreature(m_session, true);
    if (creature_target == nullptr || !creature_target->GetSQL_id())
        return true;

    Player* player = m_session->GetPlayer();
    if (player->waypointunit == creature_target->GetAIInterface())
    {
        if (creature_target->GetAIInterface()->isShowWayPointsActive())
            player->waypointunit->hideWayPoints(player);

        player->waypointunit = nullptr;
    }

    creature_target->GetAIInterface()->saveWayPoints();

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
    if (player->waypointunit != ai)
    {
        if (ai->isShowWayPointsActive() == true)
        {
            RedSystemMessage(m_session, "Some one else is also Viewing this Creatures WayPoints.");
            RedSystemMessage(m_session, "Viewing WayPoints at the same time as some one else can cause undesireble results.");
            return true;
        }

        if (player->waypointunit != nullptr)
            player->waypointunit->hideWayPoints(player);

        player->waypointunit = ai;
        ai->activateShowWayPoints(player, Backwards);
        ai->activateShowWayPointsBackwards(Backwards);
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

//.waypoint skin
bool ChatHandler::HandleWayPointSkinCommand(const char* args, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }
    uint32 SkinId = 0;

    if (wpid)
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint)
        {
            char* pBackwards = strtok((char*)args, " ");
            uint32 Backwards = (pBackwards) ? atoi(pBackwards) : 0;
            char* pSkinId = strtok(NULL, " ");
            SkinId = (pSkinId) ? atoi(pSkinId) : 0;

            if (Backwards)
                waypoint->backwardskinid = SkinId;
            else
                waypoint->forwardskinid = SkinId;

            ai->saveWayPoints();
        }

        SystemMessage(m_session, "SkinID for Waypoint %u is now %u", wpid, SkinId);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
        return true;
    }

    return true;
}

//.waypoint wait
bool ChatHandler::HandleWayPointWaitCommand(const char* args, WorldSession* m_session)
{
    uint32 wpid = GetSelectedWayPointId(m_session);

    Player* player = m_session->GetPlayer();
    AIInterface* ai = player->waypointunit;
    if (ai == nullptr || !ai->GetUnit())
    {
        SystemMessage(m_session, "Invalid Creature, please select another one.");
        return true;
    }

    uint32 Wait = 10000;

    if (wpid)
    {
        Movement::WayPoint* waypoint = ai->getWayPoint(wpid);
        if (waypoint != nullptr)
        {
            char* pWait = strtok((char*)args, " ");
            Wait = (pWait) ? atoi(pWait) : 10000;

            if (Wait < 5000)
                RedSystemMessage(m_session, "A Wait Time of less then 5000ms can cause lag, consider extending it.");

            waypoint->waittime = Wait;

            ai->saveWayPoints();
        }

        SystemMessage(m_session, "Wait Time for Waypoint %u is now %u.", wpid, Wait);
    }
    else
    {
        SystemMessage(m_session, "Invalid Waypoint.");
    }

    return true;
}
