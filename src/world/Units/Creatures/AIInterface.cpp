/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "StdAfx.h"
#include "VMapFactory.h"
#include "MMapManager.h"
#include "MMapFactory.h"
#include "Units/Stats.h"
#include "Server/Packets/Movement/CreatureMovement.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Objects/Faction.h"
#include "Spell/SpellMgr.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Map/WorldCreator.h"
#include "scripts/Battlegrounds/AlteracValley.h"
#include "Spell/Definitions/SpellCastTargetFlags.h"
#include "Spell/Definitions/SpellRanged.h"
#include "Spell/Definitions/LockTypes.h"
#include "Spell/Definitions/SpellIsFlags.h"
#include "Spell/Definitions/PowerType.h"
#include "Spell/SpellHelpers.h"
#include "Pet.h"
#include "Spell/SpellEffects.h"

#ifndef UNIX
#include <cmath>
#endif

AIInterface::AIInterface()
    :
    m_canMove(true),
    mShowWayPoints(false),
    mShowWayPointsBackwards(false),
    mCurrentWaypoint(0),
    mMoveWaypointsBackwards(false),

    mWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE),

    mNextPoint(-1),
    mWaitTimerSetOnWP(false),

    mCreatureState(STOPPED),

    onGameobject(false),
    m_canFlee(false),
    m_canCallForHelp(false),
    m_canRangedAttack(false),
    m_FleeHealth(0.0f),
    m_FleeDuration(0),
    m_CallForHelpHealth(0.0f),
    m_totemspelltimer(0),
    m_totemspelltime(0),
    totemspell(nullptr),
    m_totalMoveTime(0),
    m_formationLinkTarget(0),
    m_formationFollowDistance(0.0f),
    m_formationFollowAngle(0.0f),
    m_formationLinkSqlId(0),
    timed_emotes(nullptr),

    mIsCombatDisabled(false),
    mIsMeleeDisabled(false),
    mIsRangedDisabled(false),
    mIsCastDisabled(false),
    mIsTargetingDisabled(false),

    waiting_for_cooldown(false),
    next_spell_time(0),
    m_isGuard(false),
    m_isNeutralGuard(false),
    m_AllowedToEnterCombat(true),
    m_updateAssist(false),
    m_updateTargets(false),
    m_updateAssistTimer(1),
    m_updateTargetsTimer(TARGET_UPDATE_INTERVAL_ON_PLAYER),
    m_updateTargetsTimer2(0),
    m_nextSpell(nullptr),
    m_nextTarget(0),
    m_fleeTimer(0),
    m_hasFleed(false),
    m_hasCalledForHelp(false),
    m_outOfCombatRange(50 * 50), // Where did u get this value?
    m_Unit(nullptr),
    m_PetOwner(nullptr),
    FollowDistance(0.0f),
    m_fallowAngle(M_PI_FLOAT / 2),
    mAiState(AI_STATE_IDLE),
    m_aiCurrentAgent(AGENT_NULL),
    tauntedBy(nullptr),
    isTaunted(false),
    soullinkedWith(nullptr),
    isSoulLinked(false),
    m_runSpeed(0.0f),
    m_flySpeed(0.0f),
    m_last_target_x(0),
    m_last_target_y(0),
    mSplinePriority(SPLINE_PRIORITY_MOVEMENT),
    m_returnX(0),
    m_returnY(0),
    m_returnZ(0),
    m_combatResetX(0),
    m_combatResetY(0),
    m_combatResetZ(0),
    m_lastFollowX(0),
    m_lastFollowY(0),
    m_UnitToFollow(0),
    m_UnitToFollow_backup(0),
    m_UnitToFear(0),
    m_timeToMove(0),
    m_timeMoved(0),
    m_moveTimer(0),
    m_FearTimer(0),
    m_WanderTimer(0),
    m_MovementState(MOVEMENTSTATE_STOP),
    m_currentHighestThreat(0),
    timed_emote_expire(0xFFFFFFFF),
    mWaypointMapIsLoadedFromDB(false),
    mWayPointMap(nullptr),
    m_is_in_instance(false),
    skip_reset_hp(false),

    faction_visibility(0),

    mWalkMode(0),
    FollowDistance_backup(0),
    mAiScriptType(AI_SCRIPT_LONER),
    m_walkSpeed(0),
    m_guardTimer(0)
{
    m_aiTargets.clear();
    m_assistTargets.clear();
    m_spells.clear();
}

AIInterface::~AIInterface()
{
    for (std::list<AI_Spell*>::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        delete(*itr);
    }

    m_spells.clear();

    deleteAllWayPoints();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Waypoint / movement functions
void AIInterface::setWaypointScriptType(Movement::WaypointMovementScript wp_script)
{
    mWaypointScriptType = wp_script;
    m_moveTimer = 0;
}

Movement::WaypointMovementScript AIInterface::getWaypointScriptType()
{
    return mWaypointScriptType;
}

bool AIInterface::isWaypointScriptType(Movement::WaypointMovementScript wp_script)
{
    return wp_script == mWaypointScriptType;
}

void AIInterface::setupAndMoveToNextWaypoint()
{
    if (!m_moveTimer)
    {
        mWaitTimerSetOnWP = false;

        if (mNextPoint != -1)
        {
            Movement::WayPoint* wayPoint = getWayPoint(mNextPoint);
            if (wayPoint)
            {
                if (!mMoveWaypointsBackwards)
                {
                    if ((wayPoint->forwardskinid != 0) && (GetUnit()->GetDisplayId() != wayPoint->forwardskinid))
                    {
                        GetUnit()->SetDisplayId(wayPoint->forwardskinid);
                        GetUnit()->EventModelChange();
                    }
                }
                else
                {
                    if ((wayPoint->backwardskinid != 0) && (GetUnit()->GetDisplayId() != wayPoint->backwardskinid))
                    {
                        GetUnit()->SetDisplayId(wayPoint->backwardskinid);
                        GetUnit()->EventModelChange();
                    }
                }

                switch (wayPoint->flags)
                {
                    case Movement::WP_MOVE_TYPE_FLY:
                    {
                        setSplineFlying();
                    } break;
                    case Movement::WP_MOVE_TYPE_RUN:
                    {
                        setSplineRun();
                    } break;
                    default:
                    {
                        setSplineWalk();
                    } break;
                }

                MoveTo(wayPoint->x, wayPoint->y, wayPoint->z);
            }
        }
    }
}

void AIInterface::generateWaypointScriptCircle()
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(m_Unit->GetEntry());
    if (creatureProperties != nullptr)
    {
        //LOG_DEBUG("%s (%u) called new Circle Generator!", creatureProperties->Name.c_str(), creatureProperties->Id);

        if (MoveDone())
        {
            if (!m_moveTimer)
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //init destination point
                if (mWaitTimerSetOnWP == false)
                {
                    mNextPoint = -1;
                    bool isLastWP = false;

                    // 1 -> 2 ... -> 10 then 10 -> 1 -> 2 ... -> 10
                    {
                        ++mCurrentWaypoint;
                        if (mCurrentWaypoint > getWayPointsCount())
                        {
                            mCurrentWaypoint = 1;
                            isLastWP = true;
                        }

                        mNextPoint = mCurrentWaypoint;
                        mMoveWaypointsBackwards = false;
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////
                    // calc on reach wp script call
                    if (mNextPoint != -1 && (mCurrentWaypoint > 1 || isLastWP))
                    {
                        Movement::WayPoint* wayPoint = getWayPoint(isLastWP ? getWayPointsCount() : mNextPoint - 1);
                        if (wayPoint)
                        {
                            CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wayPoint->id, !mMoveWaypointsBackwards);

                            if (wayPoint->waittime > 0)
                            {
                                mWaitTimerSetOnWP = true;
                                m_moveTimer = wayPoint->waittime;
                            }
                        }
                    }
                }

                //////////////////////////////////////////////////////////////////////////////////////////
                // get next point to move
                setupAndMoveToNextWaypoint();
            }
        }
        else
        {
            //LOG_DEBUG("%s (%u) MOVE NOT DONE!", creatureProperties->Name.c_str(), creatureProperties->Id);
        }
    }
}

void AIInterface::generateWaypointScriptRandom()
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(m_Unit->GetEntry());
    if (creatureProperties != nullptr)
    {
        //LOG_DEBUG("%s (%u) called new Random Generator!", creatureProperties->Name.c_str(), creatureProperties->Id);

        if (getWayPointsCount())
        {
            if (MoveDone())
            {
                if (!m_moveTimer)
                {
                    //////////////////////////////////////////////////////////////////////////////////////////
                    //init destination point
                    if (mWaitTimerSetOnWP == false)
                    {
                        bool isFirstWP = false;

                        // 5 -> 7 then 7 -> 8 then 8 -> 2 then ....
                        {
                            if (mCurrentWaypoint == 0)
                            {
                                mCurrentWaypoint = RandomUInt(1, (uint32)getWayPointsCount());
                                isFirstWP = true;
                            }
                            else
                            {
                                mCurrentWaypoint = mNextPoint;
                            }

                            mNextPoint = RandomUInt(1, (uint32)getWayPointsCount());
                        }

                        //////////////////////////////////////////////////////////////////////////////////////////
                        // calc on reach wp script call
                        if (mCurrentWaypoint > 0 && isFirstWP == false)
                        {
                            Movement::WayPoint* wayPoint = getWayPoint(mCurrentWaypoint);
                            if (wayPoint)
                            {
                                CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wayPoint->id, !mMoveWaypointsBackwards);
                                static_cast<Creature*>(m_Unit)->HandleMonsterSayEvent(MONSTER_SAY_EVENT_RANDOM_WAYPOINT);

                                if (wayPoint->waittime > 0)
                                {
                                    mWaitTimerSetOnWP = true;
                                    m_moveTimer = wayPoint->waittime;
                                }
                            }
                        }
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////
                    // get next point to move
                    setupAndMoveToNextWaypoint();
                }
            }
            else
            {
                //LOG_DEBUG("%s (%u) MOVE NOT DONE!", creatureProperties->Name.c_str(), creatureProperties->Id);
            }
        }
        else
        {
            if (!m_moveTimer)
            {
                if (MoveDone())
                {
                    uint32_t randomMoveTime = RandomUInt(300, 6000);

                    LocationVector pos = m_Unit->GetPosition();

                    float distance = RandomFloat(4.0f) + 2.0f;
                    float orientation = RandomFloat(6.283f);

                    LocationVector randPos;
                    randPos.x = pos.x + distance * cosf(orientation);
                    randPos.y = pos.y + distance * sinf(orientation);
                    randPos.z = m_Unit->GetMapMgr()->GetLandHeight(randPos.x, randPos.y, pos.z + 2);

                    VMAP::IVMapManager* vmapMgr = VMAP::VMapFactory::createOrGetVMapManager();

                    bool isHittingObject = vmapMgr->getObjectHitPos(m_Unit->GetMapId(), pos.x, pos.y, pos.z + 2, randPos.x, randPos.y, randPos.z, randPos.x, randPos.y, randPos.z, -1);

                    MoveTo(randPos.x, randPos.y, randPos.z);

                    m_moveTimer = randomMoveTime;
                }
            }
        }
    }
}

void AIInterface::generateWaypointScriptForwad()
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(m_Unit->GetEntry());
    if (creatureProperties != nullptr)
    {
        //LOG_DEBUG("%s (%u) called new Forwad Generator!", creatureProperties->Name.c_str(), creatureProperties->Id);

        if (MoveDone())
        {
            if (!m_moveTimer)
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //init destination point
                if (mWaitTimerSetOnWP == false)
                {
                    mNextPoint = -1;
                    bool isLastWP = false;

                    // 1 -> 10 then stop
                    {
                        ++mCurrentWaypoint;
                        if (mCurrentWaypoint > getWayPointsCount())
                        {
                            mCurrentWaypoint = getWayPointsCount();
                            isLastWP = true;
                        }

                        mNextPoint = mCurrentWaypoint;
                        mMoveWaypointsBackwards = false;
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////
                    // calc on reach wp script call
                    if (mNextPoint != -1 && (mCurrentWaypoint > 1 || isLastWP))
                    {
                        Movement::WayPoint* wayPoint = getWayPoint(isLastWP ? getWayPointsCount() : mNextPoint - 1);
                        if (wayPoint)
                        {
                            if (isLastWP)
                                setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

                            CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wayPoint->id, !mMoveWaypointsBackwards);

                            if (wayPoint->waittime > 0 && !isLastWP)
                            {
                                mWaitTimerSetOnWP = true;
                                m_moveTimer = wayPoint->waittime;
                            }
                        }
                    }
                }

                //////////////////////////////////////////////////////////////////////////////////////////
                // get next point to move
                setupAndMoveToNextWaypoint();
            }
        }
        else
        {
            //LOG_DEBUG("%s (%u) MOVE NOT DONE!", creatureProperties->Name.c_str(), creatureProperties->Id);
        }
    }
}

void AIInterface::generateWaypointScriptWantedWP()
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(m_Unit->GetEntry());
    if (creatureProperties != nullptr)
    {
        //LOG_DEBUG("%s (%u) called new WantedWP Generator!", creatureProperties->Name.c_str(), creatureProperties->Id);

        if (mCurrentWaypoint > 0 && mCurrentWaypoint < getWayPointsCount())
        {
            if (!m_moveTimer)
            {
                mNextPoint = mCurrentWaypoint;
                setupAndMoveToNextWaypoint();

                if (MoveDone())
                {
                    Movement::WayPoint* wayPoint = getWayPoint(mNextPoint);
                    if (wayPoint != nullptr)
                    {
                        m_Unit->GetAIInterface()->setWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_NONE);

                        CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wayPoint->id, !mMoveWaypointsBackwards);

                        m_moveTimer = wayPoint->waittime;
                    }
                }
                else
                {
                    //LOG_DEBUG("%s (%u) MOVE NOT DONE!", creatureProperties->Name.c_str(), creatureProperties->Id);
                }
            }
        }
    }
}

void AIInterface::generateWaypointScriptPatrol()
{
    CreatureProperties const* creatureProperties = sMySQLStore.getCreatureProperties(m_Unit->GetEntry());
    if (creatureProperties != nullptr)
    {
        //LOG_DEBUG("%s (%u) called new Patrol Generator!", creatureProperties->Name.c_str(), creatureProperties->Id);

        if (MoveDone())
        {
            if (!m_moveTimer)
            {
                //////////////////////////////////////////////////////////////////////////////////////////
                //init destination point
                if (mWaitTimerSetOnWP == false)
                {
                    mNextPoint = -1;
                    bool isLastWP = false;

                    // 1 -> 2 ... -> 10 then 10 -> 9 ... -> 1
                    {
                        if (mCurrentWaypoint > getWayPointsCount())
                            mCurrentWaypoint = 1;

                        if (mCurrentWaypoint == getWayPointsCount())
                        {
                            mMoveWaypointsBackwards = true;
                            isLastWP = true;
                        }

                        if (mCurrentWaypoint == 1)
                            mMoveWaypointsBackwards = false;

                        if (mMoveWaypointsBackwards == false)
                            mNextPoint = ++mCurrentWaypoint;
                        else
                            mNextPoint = --mCurrentWaypoint;
                    }

                    //////////////////////////////////////////////////////////////////////////////////////////
                    // calc on reach wp script call
                    if (mNextPoint != -1 && (mCurrentWaypoint > 0 || isLastWP))
                    {
                        Movement::WayPoint* wayPoint = getWayPoint(isLastWP ? getWayPointsCount() : mMoveWaypointsBackwards ? mNextPoint + 1 : mNextPoint - 1);
                        if (wayPoint)
                        {
                            CALL_SCRIPT_EVENT(m_Unit, OnReachWP)(wayPoint->id, !mMoveWaypointsBackwards);

                            if (wayPoint->waittime > 0)
                            {
                                mWaitTimerSetOnWP = true;
                                m_moveTimer = wayPoint->waittime;
                            }
                        }
                    }
                }

                //////////////////////////////////////////////////////////////////////////////////////////
                // get next point to move
                setupAndMoveToNextWaypoint();
            }
        }
        else
        {
            //LOG_DEBUG("%s (%u) MOVE NOT DONE!", creatureProperties->Name.c_str(), creatureProperties->Id);
        }
    }
}

void AIInterface::updateOrientation()
{
    if (MoveDone())
    {
        setFacing(m_Unit->GetOrientation());
    }
}

void AIInterface::setFormationMovement()
{
    if (m_formationLinkSqlId != 0)
    {
        if (m_formationLinkTarget == 0)
        {
            Creature* creature = static_cast<Creature*>(m_Unit);
            if (!creature->haslinkupevent)
            {
                creature->haslinkupevent = true;
                sEventMgr.AddEvent(creature, &Creature::FormationLinkUp, m_formationLinkSqlId, EVENT_CREATURE_FORMATION_LINKUP, 1000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            }
        }
        else
        {
            SetUnitToFollow(m_formationLinkTarget);
            FollowDistance = m_formationFollowDistance;
            m_fallowAngle = m_formationFollowAngle;
        }
    }
}

void AIInterface::setFearRandomMovement()
{
    Unit* unitToFear = getUnitToFear();
    if (unitToFear != nullptr && isAiState(AI_STATE_FEAR))          // check fear aura instead?
    {
        if (Util::getMSTime() > m_FearTimer)
        {
            if (MoveDone())
            {
                LocationVector pos = unitToFear->GetPosition();     // current position

                float distance = RandomFloat(15.0f) + 5.0f;
                float orientation = RandomFloat(6.283f);

                LocationVector randPos;
                randPos.x = pos.x + distance * cosf(orientation);
                randPos.y = pos.y + distance * sinf(orientation);
                randPos.z = unitToFear->GetMapMgr()->GetLandHeight(randPos.x, randPos.y, pos.z + 2);

                VMAP::IVMapManager* vmapMgr = VMAP::VMapFactory::createOrGetVMapManager();

                // change generated x, y, z to a position before hitting the object.
                bool isHittingObject = vmapMgr->getObjectHitPos(m_Unit->GetMapId(), pos.x, pos.y, pos.z + 2, randPos.x, randPos.y, randPos.z, randPos.x, randPos.y, randPos.z, -1);

                MoveTo(randPos.x, randPos.y, randPos.z);

                m_FearTimer = Util::getMSTime() + RandomUInt(500, 1700);
            }
        }
    }
}

void AIInterface::setPetFollowMovement()
{
    Unit* unitToFollow = getUnitToFollow();
    if (unitToFollow != nullptr)
    {
        if (unitToFollow->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId())
        {
            m_UnitToFollow = 0;
        }
        else
        {
            if (isAiState(AI_STATE_IDLE) || isAiState(AI_STATE_FOLLOWING))
            {
                LocationVector followPos = unitToFollow->GetPosition();
                LocationVector pos = m_Unit->GetPosition();

                if (m_lastFollowX != followPos.x || m_lastFollowY != followPos.y)
                {
                    float distanceX = followPos.x - pos.x;
                    float distanceY = followPos.y - pos.y;

                    if (distanceY != 0.0f)
                    {
                        float angle = atan2(distanceX, distanceY);
                        m_Unit->SetOrientation(angle);
                    }

                    m_lastFollowX = followPos.x;
                    m_lastFollowY = followPos.y;
                }

                float distanceToTarget = m_Unit->getDistanceSq(unitToFollow);
                if (distanceToTarget > (FollowDistance * FollowDistance))
                {
                    setAiState(AI_STATE_FOLLOWING);

                    if (distanceToTarget > 100.0f)
                        setSplineSprint();
                    else if (distanceToTarget > 30.0f && distanceToTarget < 100.0f)
                        setSplineRun();
                    else
                        setSplineWalk();

                    if (isAiScriptType(AI_SCRIPT_PET) || (m_UnitToFollow == m_formationLinkTarget))
                    {
                        float followDistance = 3.0f;
                        float deltaX = followPos.x + (followDistance * (cosf(m_fallowAngle + followPos.o)));
                        float deltaY = followPos.y + (followDistance * (sinf(m_fallowAngle + followPos.o)));

                        if (m_formationLinkTarget != 0)
                            followDistance = m_formationFollowDistance;

                        MoveTo(deltaX, deltaY, followPos.z);
                    }
                    else
                    {
                        _CalcDestinationAndMove(unitToFollow, FollowDistance);
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Waypoint functions
void AIInterface::addWayPoint(Movement::WayPoint* waypoint)
{
    if (waypoint == nullptr)
        return;

    if (addWayPointUnsafe(waypoint) == false)
    {
        LOG_ERROR("WayPoint ID %u wasn't added to Unit ID %x.", waypoint->id, GetUnit()->GetGUID());
        delete waypoint;
    }
}

bool AIInterface::addWayPointUnsafe(Movement::WayPoint* waypoint)
{
    if (mWayPointMap == nullptr)
        mWayPointMap = new Movement::WayPointMap;

    if (waypoint == nullptr)
        return false;

    if (waypoint->id == 0)
        return false;

    if (mWayPointMap->size() <= waypoint->id)
        mWayPointMap->resize(waypoint->id + 1);

    if ((*mWayPointMap)[waypoint->id] == nullptr)
    {
        (*mWayPointMap)[waypoint->id] = waypoint;
        return true;
    }

    return false;
}

Movement::WayPoint* AIInterface::getWayPoint(uint32_t waypointId)
{
    if (mWayPointMap == nullptr)
        return nullptr;

    if (waypointId >= mWayPointMap->size())
        return nullptr;

    return mWayPointMap->at(waypointId);
}

bool AIInterface::saveWayPoints()
{
    if (mWayPointMap == nullptr)
        return false;

    if (GetUnit() == nullptr)
        return false;

    if (GetUnit()->IsCreature() == false)
        return false;

    WorldDatabase.Execute("DELETE FROM creature_waypoints WHERE spawnid = %u", static_cast<Creature*>(GetUnit())->GetSQL_id());

    for (Movement::WayPointMap::const_iterator itr = mWayPointMap->begin(); itr != mWayPointMap->end(); ++itr)
    {
        if ((*itr) == nullptr)
            continue;

        Movement::WayPoint* wp = (*itr);

        //Save
        std::stringstream ss;
        ss.rdbuf()->str("");

        ss << "INSERT INTO creature_waypoints ";
        ss << "(spawnid,waypointid,position_x,position_y,position_z,waittime,flags,forwardemoteoneshot,forwardemoteid,backwardemoteoneshot,backwardemoteid,forwardskinid,backwardskinid) VALUES (";
        ss << static_cast<Creature*>(GetUnit())->GetSQL_id() << ", ";
        ss << wp->id << ", ";
        ss << wp->x << ", ";
        ss << wp->y << ", ";
        ss << wp->z << ", ";
        ss << wp->waittime << ", ";
        ss << wp->flags << ", ";
        ss << wp->forwardemoteoneshot << ", ";
        ss << wp->forwardemoteid << ", ";
        ss << wp->backwardemoteoneshot << ", ";
        ss << wp->backwardemoteid << ", ";
        ss << wp->forwardskinid << ", ";
        ss << wp->backwardskinid << ")\0";

        WorldDatabase.Execute(ss.str().c_str());
    }

    return true;
}

void AIInterface::deleteWayPointById(uint32_t waypointId)
{
    if (mWayPointMap == nullptr)
        return;

    if (waypointId <= 0)
        return;

    if (waypointId > mWayPointMap->size())
        return;

    Movement::WayPointMap new_waypoints;

    for (auto wayPoint : *mWayPointMap)
    {
        if (wayPoint == nullptr || wayPoint->id == waypointId)
        {
            if (wayPoint != nullptr)
                delete wayPoint;

            continue;
        }

        new_waypoints.push_back(wayPoint);
    }

    mWayPointMap->clear();

    uint32_t newWpId = 1;
    for (auto newWayPoint : new_waypoints)
    {
        if (newWayPoint != nullptr)
        {
            newWayPoint->id = newWpId++;
            mWayPointMap->push_back(newWayPoint);
        }
    }

    saveWayPoints();
}

void AIInterface::deleteAllWayPoints()
{
    //if mWayPointMap was loaded from DB, then it's shared between other instances deleted  by ObjectMgr::~ObjectMgr()
    if (mWayPointMap == nullptr || mWaypointMapIsLoadedFromDB)
        return;

    for (auto wayPoint : *mWayPointMap)
        delete wayPoint;

    mWayPointMap->clear();
    delete mWayPointMap;
    mWayPointMap = nullptr;
}

bool AIInterface::hasWayPoints()
{
    return mWayPointMap != nullptr;
}

uint32_t AIInterface::getCurrentWayPointId()
{
    return mCurrentWaypoint;
}

void AIInterface::changeWayPointId(uint32_t oldWaypointId, uint32_t newWaypointId)
{
    if (!mWayPointMap)
        return;

    if (newWaypointId == 0)
        return;

    if (newWaypointId > mWayPointMap->size())
        return;

    if (oldWaypointId > mWayPointMap->size())
        return;

    if (newWaypointId == oldWaypointId)
        return;

    Movement::WayPoint* newWaypoint = getWayPoint(newWaypointId);
    if (newWaypoint == nullptr)
        return;

    Movement::WayPoint* oldWaypoint = getWayPoint(oldWaypointId);
    if (oldWaypoint == nullptr)
        return;

    oldWaypoint->id = newWaypointId;
    newWaypoint->id = oldWaypointId;
    (*mWayPointMap)[oldWaypoint->id] = oldWaypoint;
    (*mWayPointMap)[newWaypoint->id] = newWaypoint;

    saveWayPoints();
}

size_t AIInterface::getWayPointsCount()
{
    if (mWayPointMap && !mWayPointMap->empty())
        return mWayPointMap->size() - 1;
    else
        return 0;
}

void AIInterface::setWayPointToMove(uint32_t waypointId)
{
    mCurrentWaypoint = waypointId;
}

bool AIInterface::activateShowWayPoints(Player* player, bool showBackwards)
{
    if (mWayPointMap == nullptr)
        return false;

    Movement::WayPointMap::const_iterator itr;
    if (mShowWayPoints == true)
        return false;

    mShowWayPoints = true;

    for (auto wayPoint : *mWayPointMap)
    {
        if (wayPoint != nullptr)
        {
            Creature* targetCreature = static_cast<Creature*>(GetUnit());

            Creature* wpCreature = new Creature((uint64)HIGHGUID_TYPE_WAYPOINT << 32 | wayPoint->id);
            wpCreature->CreateWayPoint(wayPoint->id, player->GetMapId(), wayPoint->x, wayPoint->y, wayPoint->z, 0);
            wpCreature->SetCreatureProperties(targetCreature->GetCreatureProperties());
            wpCreature->SetEntry(1);
            wpCreature->SetScale(0.5f);

            uint32_t displayId = 0;
            if (showBackwards)
                displayId = (wayPoint->backwardskinid == 0) ? GetUnit()->GetNativeDisplayId() : wayPoint->backwardskinid;
            else
                displayId = (wayPoint->forwardskinid == 0) ? GetUnit()->GetNativeDisplayId() : wayPoint->forwardskinid;

            wpCreature->SetDisplayId(displayId);
            wpCreature->SetEmoteState(wayPoint->backwardemoteid);

            wpCreature->setLevel(wayPoint->id);
            wpCreature->setUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            wpCreature->SetFaction(player->GetFaction());
            wpCreature->SetHealth(1);
            wpCreature->SetMaxHealth(1);
            wpCreature->SetStat(STAT_STRENGTH, wayPoint->flags);

            ByteBuffer buf(3000);
            uint32_t count = wpCreature->BuildCreateUpdateBlockForPlayer(&buf, player);
            player->PushCreationData(&buf, count);

            wpCreature->setMoveRoot(true);

            delete wpCreature;
        }
    }
    return true;
}

void AIInterface::activateShowWayPointsBackwards(bool set)
{
    mShowWayPointsBackwards = set;
}

bool AIInterface::isShowWayPointsActive()
{
    return mShowWayPoints;
}

bool AIInterface::isShowWayPointsBackwardsActive()
{
    return mShowWayPointsBackwards;
}

bool AIInterface::hideWayPoints(Player* player)
{
    if (mWayPointMap == nullptr)
        return false;

    if (mShowWayPoints != true)
        return false;

    mShowWayPoints = false;

    for (auto wayPoint : *mWayPointMap)
    {
        if (wayPoint != nullptr)
        {
            uint64_t guid = ((uint64_t)HIGHGUID_TYPE_WAYPOINT << 32) | wayPoint->id;
            WoWGuid wowguid(guid);
            player->PushOutOfRange(wowguid);
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Spline functions
void AIInterface::setFacing(float orientation)
{
    m_Unit->m_movementManager.m_spline.SetFacing(orientation);

    LocationVector pos = LocationVector(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), orientation);

    sendSplineMoveToPoint(pos);
}

void AIInterface::setWalkMode(uint32_t mode)
{
    mWalkMode = mode;
}

bool AIInterface::hasWalkMode(uint32_t mode) const
{
    return mWalkMode == mode;
}

uint32_t AIInterface::getWalkMode() const
{
    return mWalkMode;
}

void AIInterface::setSplineFlying() const
{
    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.flying = true;
}

bool AIInterface::isFlying()
{
    if (m_Unit->IsCreature())
        return m_Unit->m_movementManager.isFlying();

    if (m_Unit->IsPlayer())
        return static_cast<Player*>(m_Unit)->FlyCheat;

    return false;
}

void AIInterface::unsetSplineFlying()
{
    if (isFlying())
    {
        m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.flying = false;
        setSplineWalk();
    }
}

void AIInterface::setSplineSprint()
{
    if (!isFlying())
    {
        m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.walkmode = true;
        setWalkMode(WALKMODE_SPRINT);
        UpdateSpeeds();
    }
}

void AIInterface::setSplineRun()
{
    if (!isFlying())
    {
        m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.walkmode = true;
        setWalkMode(WALKMODE_RUN);
        UpdateSpeeds();
    }
}

void AIInterface::setSplineWalk()
{
    if (!isFlying())
    {
        m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.walkmode = true;
        setWalkMode(WALKMODE_WALK);
        UpdateSpeeds();
    }
}

void AIInterface::unsetSpline()
{
    m_Unit->m_movementManager.m_spline.ClearSpline();
    m_Unit->m_movementManager.ForceUpdate();

    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.trajectory = false;

#if VERSION_STRING != Cata
    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.knockback = false;
#else
    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.falling = false;
#endif
}

void AIInterface::splineMoveKnockback(float x, float y, float z, float horizontal, float vertical)
{
    mSplinePriority = SPLINE_PRIORITY_REDIRECTION;

    unsetSpline();

    m_Unit->m_movementManager.m_spline.m_splineTrajectoryTime = 0;
    m_Unit->m_movementManager.m_spline.m_splineTrajectoryVertical = vertical;

    setSplineRun();
    m_runSpeed *= 3;

    float speedmod = float(vertical / 7.5);
    m_runSpeed /= speedmod;

    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.trajectory = true;
#if VERSION_STRING != Cata
    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.knockback = true;
#else
    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.falling = true;
#endif

    sendSplineMoveToPoint(LocationVector(x, y, z, 0));
}

void AIInterface::splineMoveJump(float x, float y, float z, float o /*= 0*/, float speedZ /*= 5.0f */, bool hugearc /*= false*/)
{
    mSplinePriority = SPLINE_PRIORITY_REDIRECTION;

    unsetSpline();

    m_Unit->m_movementManager.m_spline.SetFacing(o);

    m_Unit->m_movementManager.m_spline.m_splineTrajectoryTime = 0;

    if (hugearc)
        m_Unit->m_movementManager.m_spline.m_splineTrajectoryVertical = 250;
    else
        m_Unit->m_movementManager.m_spline.m_splineTrajectoryVertical = speedZ;

    setSplineRun();
    m_runSpeed *= 3;

    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.trajectory = true;

    float orientation = o ? o : m_Unit->calcRadAngle(x, y, m_Unit->GetPositionX(), m_Unit->GetPositionY());

    sendSplineMoveToPoint(LocationVector(x, y, z, orientation));
}

void AIInterface::splineMoveFalling(float x, float y, float z, float o /*= 0*/)
{
    unsetSpline();

    m_Unit->m_movementManager.m_spline.SetFacing(o);

    m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.done = true;

    sendSplineMoveToPoint(LocationVector(x, y, z, o));
}

void AIInterface::splineMoveCharge(Unit* targetUnit, float distance /*= 3.0f*/)
{
    if (targetUnit == nullptr)
        return;

    mSplinePriority = SPLINE_PRIORITY_MOVEMENT;

    unsetSpline();

    setSplineSprint();

    m_runSpeed *= 7.0f;

    generateSplinePathToTarget(targetUnit, distance);

    LocationVector targetPos = targetUnit->GetPosition();

    float orientation = m_Unit->calcRadAngle(targetPos.x, targetPos.y, m_Unit->GetPositionX(), m_Unit->GetPositionY());

    //reset run speed
    UpdateSpeeds();

    unsetSpline();

    m_Unit->SetPosition(targetPos.x, targetPos.y, targetPos.z, orientation);
}

void AIInterface::generateSplinePathToTarget(Unit* targetUnit, float distance)
{
    if (!m_canMove || m_Unit->IsStunned())
    {
        StopMovement(0);
        return;
    }

    if (targetUnit == nullptr)
        return;

    LocationVector targetPos = targetUnit->GetPosition();

    if (abs(m_last_target_x - targetPos.x) < minWalkDistance && abs(m_last_target_y - targetPos.y) < minWalkDistance && isCreatureState(MOVING))
        return;

    m_last_target_x = targetPos.x;
    m_last_target_y = targetPos.y;

    float angle = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), targetPos.x, targetPos.y) * M_PI_FLOAT / 180.0f;
    float x = distance * cosf(angle);
    float y = distance * sinf(angle);

    if (targetUnit->IsPlayer() && static_cast<Player*>(targetUnit)->m_isMoving)
    {
        x -= cosf(targetPos.o);
        y -= sinf(targetPos.o);
    }

    targetPos.x -= x;
    targetPos.y -= y;

    generateAndSendSplinePath(targetPos.x, targetPos.y, targetPos.z);
}

void AIInterface::sendSplineMoveToPoint(LocationVector pos)
{
    AddSpline(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ());
    AddSpline(pos.x, pos.y, pos.z);

    SendMoveToPacket();

    UpdateSpeeds();

    unsetSpline();

    m_Unit->SetPosition(pos.x, pos.y, pos.z, pos.o);
}

bool AIInterface::generateAndSendSplinePath(float x, float y, float z, float o /*= 0.0f*/)
{
    if (mSplinePriority > SPLINE_PRIORITY_MOVEMENT)
        return false;

    //Make sure our position is up to date
    UpdateMovementSpline();

    //Clear current spline
    m_Unit->m_movementManager.m_spline.ClearSpline();
    m_Unit->m_movementManager.ForceUpdate();


    //Add new points
    if (worldConfig.terrainCollision.isPathfindingEnabled)
    {
        if (!isFlying())
        {
            if (CreatePath(x, y, z))
            {
                SendMoveToPacket();
                return true;
            }
            else
            {
                StopMovement(0);
                return false;
            }
        }
        else
        {
            sendSplineMoveToPoint(LocationVector(x, y, z, o));
            return true;
        }
    }
    else
    {
        sendSplineMoveToPoint(LocationVector(x, y, z, o));
        return true;
    }

    return true;
}


void AIInterface::EventAiInterfaceParamsetFinish()
{
    if (timed_emotes && timed_emotes->begin() != timed_emotes->end())
    {
        next_timed_emote = timed_emotes->begin();
        timed_emote_expire = (*next_timed_emote)->expire_after;
    }
}

void AIInterface::Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt)
{
    ARCEMU_ASSERT(at != AI_SCRIPT_PET);

    setAiScriptType(at);
    setWaypointScriptType(mt);

    setAiState(AI_STATE_IDLE);
    m_MovementState = MOVEMENTSTATE_STOP;

    m_Unit = un;

    m_walkSpeed = m_Unit->m_currentSpeedWalk * 0.001f; //move distance per ms time
    m_runSpeed = m_Unit->m_currentSpeedRun * 0.001f; //move distance per ms time
    m_flySpeed = m_Unit->m_currentSpeedFly * 0.001f;

    m_guardTimer = Util::getMSTime();
}

void AIInterface::Init(Unit* un, AiScriptTypes at, Movement::WaypointMovementScript mt, Unit* owner)
{
    ARCEMU_ASSERT(at == AI_SCRIPT_PET || at == AI_SCRIPT_TOTEM);

    setAiScriptType(at);
    setWaypointScriptType(mt);

    setAiState(AI_STATE_IDLE);
    m_MovementState = MOVEMENTSTATE_STOP;

    m_Unit = un;
    m_PetOwner = owner;

    m_walkSpeed = m_Unit->m_currentSpeedWalk * 0.001f; //move distance per ms time
    m_runSpeed = m_Unit->m_currentSpeedRun * 0.001f; //move/ms
    m_flySpeed = m_Unit->m_currentSpeedFly * 0.001f;
}

Unit* AIInterface::GetUnit() const
{
    return m_Unit;
}

Unit* AIInterface::GetPetOwner() const
{
    return m_PetOwner;
}

void AIInterface::HandleEvent(uint32 event, Unit* pUnit, uint32 misc1)
{
    if (m_Unit == nullptr)
        return;

    // Passive NPCs (like target dummies) shouldn't do anything.
    if (isAiScriptType(AI_SCRIPT_PASSIVE))
        return;

    if (event < NUM_AI_EVENTS && AIEventHandlers[event] != NULL)
        (*this.*AIEventHandlers[event])(pUnit, misc1);
}

void AIInterface::Update(unsigned long time_passed)
{
    float tdist;
    if (isAiScriptType(AI_SCRIPT_TOTEM))
    {
        _UpdateTotem(time_passed);
        return;
    }

    _UpdateTimer(time_passed);
    _UpdateTargets();

    if (m_Unit->isAlive() && !isAiState(AI_STATE_IDLE)
        && !isAiState(AI_STATE_FOLLOWING) && !isAiState(AI_STATE_FEAR)
        && !isAiState(AI_STATE_WANDER) && !isAiState(AI_STATE_SCRIPTMOVE))
    {
        if (isAiScriptType(AI_SCRIPT_PET))
        {
            if (!m_Unit->bInvincible && m_Unit->IsPet())
            {
                Pet* pPet = static_cast<Pet*>(m_Unit);
                if (pPet->GetPetAction() == PET_ACTION_ATTACK || pPet->GetPetState() != PET_STATE_PASSIVE)
                {
                    _UpdateCombat(time_passed);
                }
            }
            else if (!m_Unit->IsPet())      //we just use any creature as a pet guardian
            {
                _UpdateCombat(time_passed);
            }
        }
        else
        {
            _UpdateCombat(time_passed);
        }
    }

    UpdateMovementSpline();
    _UpdateMovement(time_passed);

    if (isAiState(AI_STATE_EVADE))
    {
        tdist = m_Unit->getDistanceSq(m_returnX, m_returnY, m_returnZ);
        if (tdist <= 4.0f)
        {
            setAiState(AI_STATE_IDLE);
            m_returnX = m_returnY = m_returnZ = 0.0f;
            m_combatResetX = m_combatResetY = m_combatResetZ = 0.0f;
            setSplineWalk();

            if (!isAiScriptType(AI_SCRIPT_PET) && !skip_reset_hp)
                m_Unit->SetHealth(m_Unit->GetMaxHealth());
        }
        else
        {
            if (isCreatureState(STOPPED))
            {
                // return to the home
                if (m_returnX == 0.0f && m_returnY == 0.0f)
                {
                    SetReturnPosition();
                }

                MoveEvadeReturn();
            }
        }
    }

    if (m_fleeTimer)
    {
        if (m_fleeTimer > time_passed)
        {
            m_fleeTimer -= time_passed;
            _CalcDestinationAndMove(getNextTarget(), 5.0f);
        }
        else
        {
            m_fleeTimer = 0;
            setNextTarget(FindTargetForSpell(m_nextSpell));
        }
    }

    if (!getNextTarget() && !m_fleeTimer && isCreatureState(STOPPED) && isAiState(AI_STATE_IDLE) && m_Unit->isAlive())
    {
        if (timed_emote_expire <= time_passed)    // note that creature might go idle and time_passed might get big next time ...We do not skip emotes because of lost time
        {
            if ((*next_timed_emote)->type == 1)   //standstate
            {
                m_Unit->SetStandState(static_cast<uint8>((*next_timed_emote)->value));
                m_Unit->setUInt32Value(UNIT_NPC_EMOTESTATE, 0);
            }
            else if ((*next_timed_emote)->type == 2)   //emotestate
            {
                m_Unit->setUInt32Value(UNIT_NPC_EMOTESTATE, (*next_timed_emote)->value);
                m_Unit->SetStandState(STANDSTATE_STAND);
            }
            else if ((*next_timed_emote)->type == 3)   //oneshot emote
            {
                m_Unit->setUInt32Value(UNIT_NPC_EMOTESTATE, 0);
                m_Unit->SetStandState(STANDSTATE_STAND);
                m_Unit->Emote((EmoteType)(*next_timed_emote)->value);           // Animation
            }

            if ((*next_timed_emote)->msg)
                m_Unit->SendChatMessage((*next_timed_emote)->msg_type, (*next_timed_emote)->msg_lang, (*next_timed_emote)->msg);

            timed_emote_expire = (*next_timed_emote)->expire_after; //should we keep lost time ? I think not
            ++next_timed_emote;

            if (next_timed_emote == timed_emotes->end())
                next_timed_emote = timed_emotes->begin();
        }
        else
            timed_emote_expire -= time_passed;
    }
}

void AIInterface::_UpdateTimer(uint32 p_time)
{
    if (m_updateAssistTimer > p_time)
    {
        m_updateAssistTimer -= p_time;
    }
    else
    {
        m_updateAssist = true;
        m_updateAssistTimer = TARGET_UPDATE_INTERVAL_ON_PLAYER * 2 - m_updateAssistTimer - p_time;
    }

    if (m_updateTargetsTimer > p_time)
    {
        m_updateTargetsTimer -= p_time;
    }
    else
    {
        m_updateTargets = true;
        m_updateTargetsTimer = TARGET_UPDATE_INTERVAL_ON_PLAYER * 2 - m_updateTargetsTimer - p_time;
    }
}

void AIInterface::_UpdateTargets()
{
    if (m_Unit->IsPlayer() || (!isAiScriptType(AI_SCRIPT_PET) && mIsTargetingDisabled))
        return;

    if (static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Type == UNIT_TYPE_CRITTER && static_cast<Creature*>(m_Unit)->GetType() != CREATURE_TYPE_GUARDIAN)
        return;

    if (m_Unit->GetMapMgr() == nullptr)
        return;

    // Find new Assist Targets and remove old ones
    if (isAiState(AI_STATE_FLEEING))
    {
        FindFriends(100.0f/*10.0*/);
    }
    else if (!isAiState(AI_STATE_IDLE) && !isAiState(AI_STATE_SCRIPTIDLE))
    {
        FindFriends(64.0f/*8.0f*/);
    }

    if (m_updateAssist)
    {
        m_updateAssist = false;

        for (AssistTargetSet::iterator i = m_assistTargets.begin(); i != m_assistTargets.end();)
        {
            AssistTargetSet::iterator i2 = i++;
            if ((*i2) == NULL || (*i2)->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() ||
                !(*i2)->isAlive() || m_Unit->getDistanceSq((*i2)) >= 2500.0f || !(*i2)->CombatStatus.IsInCombat() || !((*i2)->m_phase & m_Unit->m_phase))
            {
                m_assistTargets.erase(i2);
            }
        }
    }

    if (m_updateTargets)
    {
        m_updateTargets = false;

        LockAITargets(true);

        for (TargetMap::iterator itr = m_aiTargets.begin(); itr != m_aiTargets.end();)
        {
            TargetMap::iterator it2 = itr++;

            Unit* ai_t = m_Unit->GetMapMgr()->GetUnit(it2->first);
            if (ai_t == nullptr)
            {
                m_aiTargets.erase(it2);
            }
            else
            {
                bool instance = false;
                if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo())
                {
                    switch (m_Unit->GetMapMgr()->GetMapInfo()->type)
                    {
                        case INSTANCE_RAID:
                        case INSTANCE_NONRAID:
                        case INSTANCE_MULTIMODE:
                            instance = true;
                            break;
                    }
                }

                if (ai_t->event_GetCurrentInstanceId() != m_Unit->event_GetCurrentInstanceId() || !ai_t->isAlive() || ((!instance && m_Unit->getDistanceSq(ai_t) >= 6400.0f) || !(ai_t->m_phase & m_Unit->m_phase)))
                {
                    m_aiTargets.erase(it2);
                }
            }
        }

        LockAITargets(false);

        if (isCombatDisabled())
            return;

        if (m_aiTargets.size() == 0
            && !isAiState(AI_STATE_IDLE) && !isAiState(AI_STATE_FOLLOWING)
            && !isAiState(AI_STATE_EVADE) && !isAiState(AI_STATE_FEAR)
            && !isAiState(AI_STATE_WANDER) && !isAiState(AI_STATE_SCRIPTIDLE))
        {
            if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo())
            {
                Unit* target = nullptr;
                switch (m_Unit->GetMapMgr()->GetMapInfo()->type)
                {
                    case INSTANCE_RAID:
                    case INSTANCE_NONRAID:
                    case INSTANCE_MULTIMODE:
                        target = FindTarget();
                        break;

                    default:
                        if (m_outOfCombatRange && _CalcDistanceFromHome() < m_outOfCombatRange)
                            target = FindTarget();
                        break;
                }

                if (target != nullptr)
                    AttackReaction(target, 1, 0);
            }
        }
        else if (m_aiTargets.size() == 0 && ((isAiScriptType(AI_SCRIPT_PET) && (m_Unit->IsPet() && static_cast< Pet* >(m_Unit)->GetPetState() == PET_STATE_AGGRESSIVE)) || (!m_Unit->IsPet() && mIsMeleeDisabled == false)))
        {
            Unit* target = FindTarget();
            if (target)
            {
                AttackReaction(target, 1, 0);
            }
        }
    }
    // Find new Targets when we are ooc
    if ((isAiState(AI_STATE_IDLE) || isAiState(AI_STATE_SCRIPTIDLE)) && m_assistTargets.size() == 0)
    {
        Unit* target = FindTarget();
        if (target)
        {
            AttackReaction(target, 1, 0);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
/// Updates Combat Status of m_Unit
void AIInterface::_UpdateCombat(uint32 p_time)
{
    if (!isAiScriptType(AI_SCRIPT_PET) && isCombatDisabled())
        return;

    //just make sure we are not hitting self.
    // This was reported as an exploit.Should never occur anyway
    if (getNextTarget() == m_Unit)
        setNextTarget(GetMostHated());

    uint16 agent = static_cast<uint16>(m_aiCurrentAgent);

    // If creature is very far from spawn point return to spawnpoint
    // If at instance don't return -- this is wrong ... instance creatures always returns to spawnpoint, dunno how do you got this idea.
    // If at instance returns to spawnpoint after empty agrolist
    Unit* nextTarget = getNextTarget();
    if (!isAiScriptType(AI_SCRIPT_PET)
        && !isAiState(AI_STATE_EVADE)
        && !isAiState(AI_STATE_SCRIPTMOVE)
        && !m_is_in_instance
        && (m_outOfCombatRange && m_Unit->getDistanceSq(m_combatResetX, m_combatResetY, m_combatResetZ) > m_outOfCombatRange))
    {
        HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
    }
    else if (nextTarget == NULL && !isAiState(AI_STATE_FOLLOWING) && !isAiState(AI_STATE_SCRIPTMOVE))
    {
        //        SetNextTarget(FindTargetForSpell(m_nextSpell));
        if (m_is_in_instance)
            setNextTarget(FindTarget());
        else
            setNextTarget(GetMostHated());

        if (getNextTarget() == NULL)
        {
            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }
    else if (nextTarget != NULL && !(nextTarget->m_phase & m_Unit->m_phase))     // the target or we changed phase, stop attacking
    {
        if (m_is_in_instance)
            setNextTarget(FindTarget());
        else
            setNextTarget(GetMostHated());

        if (getNextTarget() == NULL)
        {
            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }

    if (worldConfig.terrainCollision.isCollisionEnabled)
    {
        if (m_Unit->GetMapMgr() != NULL && getNextTarget() != NULL)
        {
            if (!isFlying())
            {
                float target_land_z = m_Unit->GetMapMgr()->GetLandHeight(getNextTarget()->GetPositionX(), getNextTarget()->GetPositionY(), getNextTarget()->GetPositionZ());

                if (fabs(getNextTarget()->GetPositionZ() - target_land_z) > _CalcCombatRange(getNextTarget(), false))
                {
                    if (!getNextTarget()->IsPlayer())
                    {
                        if (target_land_z > m_Unit->GetMapMgr()->GetLiquidHeight(getNextTarget()->GetPositionX(), getNextTarget()->GetPositionY()))
                            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);  //bugged npcs, probably db fault
                    }
                    else if (static_cast< Player* >(getNextTarget())->GetSession() != NULL)
                    {
                        MovementInfo* mi = static_cast<Player*>(getNextTarget())->GetSession()->GetMovementInfo();

#if VERSION_STRING != Cata
                        if (mi->flags & MOVEFLAG_FLYING)
                            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
#else
                        if (mi->hasMovementFlag(MOVEFLAG_FLYING))
                            HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
#endif
                    }
                }
            }
        }
    }

    if (getNextTarget() != NULL && getNextTarget()->IsCreature() && isAiState(AI_STATE_EVADE))
        HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);

    bool cansee = false;
    if (getNextTarget())
    {
        if (getNextTarget()->event_GetCurrentInstanceId() == m_Unit->event_GetCurrentInstanceId())
        {
            if (m_Unit->IsCreature())
                cansee = static_cast< Creature* >(m_Unit)->CanSee(getNextTarget());
            else
                cansee = static_cast< Player* >(m_Unit)->CanSee(getNextTarget());
        }
        else
        {
            resetNextTarget();
        }
    }

    if (cansee && getNextTarget() && getNextTarget()->isAlive() && !isAiState(AI_STATE_EVADE) && !m_Unit->IsCasting())
    {
        if (agent == AGENT_NULL || (isAiScriptType(AI_SCRIPT_PET) && !m_nextSpell))     // allow pets autocast
        {
            if (!m_nextSpell)
                m_nextSpell = this->getSpell();

            if (m_canFlee && !m_hasFleed && ((static_cast<float>(m_Unit->GetHealth()) / static_cast<float>(m_Unit->GetMaxHealth()) < m_FleeHealth)))
            {
                agent = AGENT_FLEE;
            }
            else if (m_canCallForHelp && !m_hasCalledForHelp)
            {
                agent = AGENT_CALLFORHELP;
            }
            else if (m_nextSpell)
            {
                if (m_nextSpell->agent != AGENT_NULL)
                {
                    agent = m_nextSpell->agent;
                }
                else
                {
                    agent = AGENT_MELEE;
                }
            }
            else
            {
                agent = AGENT_MELEE;
            }
        }
        if (agent == AGENT_RANGED || agent == AGENT_MELEE)
        {
            if (m_canRangedAttack)
            {
                agent = AGENT_MELEE;
                if (getNextTarget()->IsPlayer())
                {
                    float dist = m_Unit->getDistanceSq(getNextTarget());
                    if (static_cast< Player* >(getNextTarget())->HasUnitMovementFlag(MOVEFLAG_ROOTED) || dist >= 64.0f)
                    {
                        agent = AGENT_RANGED;
                    }
                }
                else if (getNextTarget()->m_canMove == false || m_Unit->getDistanceSq(getNextTarget()) >= 64.0f)
                {
                    agent = AGENT_RANGED;
                }
            }
            else
            {
                agent = AGENT_MELEE;
            }
        }

        if (this->isMeleeDisabled() && agent == AGENT_MELEE)
            agent = AGENT_NULL;

        if (this->isRangedDisabled() && agent == AGENT_RANGED)
            agent = AGENT_NULL;

        if (this->isCastDisabled() && agent == AGENT_SPELL)
            agent = AGENT_NULL;

        switch (agent)
        {
            case AGENT_MELEE:
            {
                float combatReach[2]; // Calculate Combat Reach
                float distance = m_Unit->CalcDistance(getNextTarget());

                combatReach[0] = getNextTarget()->GetModelHalfSize();
                combatReach[1] = _CalcCombatRange(getNextTarget(), false);

                if (distance <= combatReach[1] + minWalkDistance) // Target is in Range -> Attack
                {
                    //FIX ME: offhand shit
                    if (m_Unit->isAttackReady(false) && !m_fleeTimer)
                    {
                        setCreatureState(ATTACKING);
                        bool infront = m_Unit->isInFront(getNextTarget());

                        if (!infront) // set InFront
                        {
                            //prevent mob from rotating while stunned
                            if (!m_Unit->IsStunned())
                            {
                                setInFront(getNextTarget());
                                infront = true;
                            }
                        }
                        if (infront)
                        {
                            m_Unit->setAttackTimer(0, false);
#ifdef ENABLE_CREATURE_DAZE
                            //we require to know if strike was successful. If there was no dmg then target cannot be dazed by it
                            Unit* t_unit = getNextTarget();
                            if (t_unit == nullptr)
                                return;

                            uint32 health_before_strike = t_unit->GetHealth();
#endif
                            if (m_Unit->GetOnMeleeSpell() != 0)
                            {
                                m_Unit->CastOnMeleeSpell();
                            }
                            else
                                m_Unit->Strike(getNextTarget(), MELEE, NULL, 0, 0, 0, false, false);

#ifdef ENABLE_CREATURE_DAZE
                            //now if the target is facing his back to us then we could just cast dazed on him :P
                            //as far as i know dazed is casted by most of the creatures but feel free to remove this code if you think otherwise
                            if (getNextTarget() && m_Unit->m_factionDBC &&
                                !(m_Unit->m_factionDBC->RepListId == -1 && m_Unit->m_faction->FriendlyMask == 0 && m_Unit->m_faction->HostileMask == 0) /* neutral creature */
                                && getNextTarget()->IsPlayer() && !m_Unit->IsPet() && health_before_strike > getNextTarget()->GetHealth()
                                && Rand(m_Unit->get_chance_to_daze(getNextTarget())))
                            {
                                float our_facing = m_Unit->calcRadAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), getNextTarget()->GetPositionX(), getNextTarget()->GetPositionY());
                                float his_facing = getNextTarget()->GetOrientation();
                                if (fabs(our_facing - his_facing) < CREATURE_DAZE_TRIGGER_ANGLE && !getNextTarget()->HasAura(CREATURE_SPELL_TO_DAZE))
                                {
                                    SpellInfo* info = sSpellCustomizations.GetSpellInfo(CREATURE_SPELL_TO_DAZE);
                                    Spell* sp = sSpellFactoryMgr.NewSpell(m_Unit, info, false, NULL);
                                    SpellCastTargets targets;
                                    targets.m_unitTarget = getNextTarget()->GetGUID();
                                    sp->prepare(&targets);
                                }
                            }
#endif
                        }
                    }
                }
                else // Target out of Range -> Run to it
                {
                    float dist;
                    if (m_Unit->GetModelHalfSize() > getNextTarget()->GetModelHalfSize())
                        dist = m_Unit->GetModelHalfSize();
                    else
                        dist = getNextTarget()->GetModelHalfSize();

                    setSplineRun();
                    _CalcDestinationAndMove(getNextTarget(), dist);
                }
            }
            break;
            case AGENT_RANGED:
            {
                float combatReach[2]; // Calculate Combat Reach
                float distance = m_Unit->CalcDistance(getNextTarget());

                combatReach[0] = 8.0f;
                combatReach[1] = 30.0f;

                if (distance >= combatReach[0] && distance <= combatReach[1]) // Target is in Range -> Attack
                {
                    //FIX ME: offhand shit
                    if (m_Unit->isAttackReady(false) && !m_fleeTimer)
                    {
                        setCreatureState(ATTACKING);
                        bool infront = m_Unit->isInFront(getNextTarget());

                        if (!infront) // set InFront
                        {
                            //prevent mob from rotating while stunned
                            if (!m_Unit->IsStunned())
                            {
                                setInFront(getNextTarget());
                                infront = true;
                            }
                        }

                        if (infront)
                        {
                            m_Unit->setAttackTimer(0, false);
                            SpellInfo* info = sSpellCustomizations.GetSpellInfo(SPELL_RANGED_GENERAL);
                            if (info)
                            {
                                Spell* sp = sSpellFactoryMgr.NewSpell(m_Unit, info, false, NULL);
                                SpellCastTargets targets;
                                targets.m_unitTarget = getNextTarget()->GetGUID();
                                sp->prepare(&targets);
                                //Lets make spell handle this
                                //m_Unit->Strike(GetNextTarget(), (agent == AGENT_MELEE ? MELEE : RANGED), NULL, 0, 0, 0);
                            }
                        }
                    }
                }
                else // Target out of Range -> Run to it
                {
                    //calculate next move
                    float dist;

                    if (distance < combatReach[0])// Target is too near
                        dist = 9.0f;
                    else
                        dist = 20.0f;

                    setSplineRun();
                    _CalcDestinationAndMove(getNextTarget(), dist);
                }
            }
            break;
            case AGENT_SPELL:
            {
                if (!m_nextSpell || !getNextTarget())
                    return;

                bool los = true;

                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                    los = mgr->isInLineOfSight(m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), getNextTarget()->GetPositionX(), getNextTarget()->GetPositionY(), getNextTarget()->GetPositionZ());
                }

                float distance = m_Unit->CalcDistance(getNextTarget());
                if (los && ((distance <= m_nextSpell->maxrange + m_Unit->GetModelHalfSize()) || m_nextSpell->maxrange == 0))  // Target is in Range -> Attack
                {
                    SpellInfo* spellInfo = m_nextSpell->spell;

                    /* if in range stop moving so we don't interrupt the spell */
                    //do not stop for instant spells
                    DBC::Structures::SpellCastTimesEntry const* spell_cast_time = sSpellCastTimesStore.LookupEntry(m_nextSpell->spell->getCastingTimeIndex());
                    if (spell_cast_time && GetCastTime(spell_cast_time) != 0)
                        StopMovement(0);

                    SpellCastTargets targets = setSpellTargets(spellInfo, getNextTarget());
                    uint32 targettype = 0;
                    if (m_nextSpell)
                        targettype = m_nextSpell->spelltargetType;

                    switch (targettype)
                    {
                        case TTYPE_CASTER:
                        case TTYPE_SINGLETARGET:
                        {
                            CastSpell(m_Unit, spellInfo, targets);
                            break;
                        }
                        case TTYPE_SOURCE:
                        {
                            m_Unit->CastSpellAoF(targets.source(), spellInfo, true);
                            break;
                        }
                        case TTYPE_DESTINATION:
                        {
                            m_Unit->CastSpellAoF(targets.destination(), spellInfo, true);
                            break;
                        }
                        default:
                            LOG_ERROR("AI Agents: Targettype of AI agent spell %u for creature %u not set", spellInfo->getId(), static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Id);
                    }

                    // CastSpell(m_Unit, spellInfo, targets);
                    if (m_nextSpell && m_nextSpell->cooldown)
                        m_nextSpell->cooldowntime = Util::getMSTime() + m_nextSpell->cooldown;

                    next_spell_time = (uint32)UNIXTIME + MOB_SPELLCAST_GLOBAL_COOLDOWN;

                    m_nextSpell = NULL;
                }
                else // Target out of Range -> Run to it
                {
                    //calculate next move
                    setSplineRun();
                    float close_to_enemy = 0.0f;
                    if (distance > m_nextSpell->maxrange)
                        close_to_enemy = m_nextSpell->maxrange - minWalkDistance;
                    else if (distance < m_nextSpell->minrange)
                        close_to_enemy = m_nextSpell->minrange + minWalkDistance;

                    if (close_to_enemy < 0)
                        close_to_enemy = 0;

                    _CalcDestinationAndMove(getNextTarget(), close_to_enemy);  //if we make exact movement we will never position perfectly
                }
            }
            break;
            case AGENT_FLEE:
            {
                setSplineWalk();

                if (m_fleeTimer == 0)
                    m_fleeTimer = m_FleeDuration;

                _CalcDestinationAndMove(getNextTarget(), 5.0f);
                if (!m_hasFleed)
                    CALL_SCRIPT_EVENT(m_Unit, OnFlee)(getNextTarget());

                setAiState(AI_STATE_FLEEING);
                resetNextTarget();

                WorldPacket data(SMSG_MESSAGECHAT, 100);
                std::string msg = "%s attempts to run away in fear!";
                data << uint8(CHAT_MSG_CHANNEL);
                data << uint32(LANG_UNIVERSAL);
                data << uint32(static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Name.size());
                data << static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Name;
                data << uint64(0);
                data << uint32(msg.size() + 1);
                data << msg;
                data << uint8(0);

                m_Unit->SendMessageToSet(&data, false);

                m_hasFleed = true;
            }
            break;
            case AGENT_CALLFORHELP:
            {
                FindFriends(64.0f /*8.0f*/);
                m_hasCalledForHelp = true; // We only want to call for Help once in a Fight.
                if (m_Unit->IsCreature())
                    static_cast< Creature* >(m_Unit)->HandleMonsterSayEvent(MONSTER_SAY_EVENT_CALL_HELP);
                CALL_SCRIPT_EVENT(m_Unit, OnCallForHelp)();
            }
            break;
        }
    }
    else if (!getNextTarget() || getNextTarget()->GetInstanceID() != m_Unit->GetInstanceID() || !getNextTarget()->isAlive() || !cansee)
    {
        resetNextTarget();
        // no more target
        //m_Unit->setAttackTarget(NULL);
    }
}

void AIInterface::DismissPet()
{
    /*
    if (m_AIType != AITYPE_PET)
    return;

    if (!m_PetOwner)
    return;

    if (m_PetOwner->GetTypeId() != TYPEID_PLAYER)
    return;

    if (m_Unit->GetCreatedBySpell() == 0)
    TO< Player* >(m_PetOwner)->SetFreePetNo(false, (int)m_Unit->GetUInt32Value(UNIT_FIELD_PETNUMBER));
    TO< Player* >(m_PetOwner)->SetPet(NULL);
    TO< Player* >(m_PetOwner)->SetPetName("");

    //FIXME:Check hunter pet or not
    //FIXME:Check enslaved creature
    m_PetOwner->SetUInt64Value(UNIT_FIELD_SUMMON, 0);

    WorldPacket data;
    data.Initialize(SMSG_PET_SPELLS);
    data << (uint64)0;
    TO< Player* >(m_PetOwner)->GetSession()->SendPacket(&data);

    sEventMgr.RemoveEvents(((Creature*)m_Unit));
    if (m_Unit->IsInWorld())
    {
    m_Unit->RemoveFromWorld();
    }
    //setup an event to delete the Creature
    sEventMgr.AddEvent(((Creature*)this->m_Unit), &Creature::DeleteMe, EVENT_DELETE_TIMER, 1, 1);*/
}

void AIInterface::SetUnitToFollow(Unit* un)
{
    if (un == nullptr)
        m_UnitToFollow = 0;
    else
        m_UnitToFollow = un->GetGUID();
}

void AIInterface::SetUnitToFear(Unit* un)
{
    if (un == nullptr)
        m_UnitToFear = 0;
    else
        m_UnitToFear = un->GetGUID();
}

void AIInterface::SetUnitToFollowBackup(Unit* un)
{
    if (un == nullptr)
        m_UnitToFollow_backup = 0;
    else
        m_UnitToFollow_backup = un->GetGUID();
}

void AIInterface::AttackReaction(Unit* pUnit, uint32 damage_dealt, uint32 spellId)
{
    if (isAiState(AI_STATE_EVADE) || !pUnit || !pUnit->isAlive() || m_Unit->IsDead() || (m_Unit == pUnit) || isAiScriptType(AI_SCRIPT_PASSIVE) || isCombatDisabled())
        return;

    if (worldConfig.terrainCollision.isCollisionEnabled && pUnit->IsPlayer())
    {
        if (m_Unit->GetMapMgr() != nullptr)
        {
            if (!isFlying())
            {
                float target_land_z = m_Unit->GetMapMgr()->GetLandHeight(pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ());

                if (fabs(pUnit->GetPositionZ() - target_land_z) > _CalcCombatRange(pUnit, false))
                {
                    if (!pUnit->IsPlayer() && target_land_z > m_Unit->GetMapMgr()->GetLiquidHeight(pUnit->GetPositionX(), pUnit->GetPositionY()))
                    {
                        return;
                    }
                    else if (static_cast< Player* >(pUnit)->GetSession() != nullptr)
                    {
                        MovementInfo* mi = static_cast< Player* >(pUnit)->GetSession()->GetMovementInfo();

#if VERSION_STRING != Cata
                        if (mi != nullptr && !(mi->flags & MOVEFLAG_FALLING) && !(mi->flags & MOVEFLAG_SWIMMING) && !(mi->flags & MOVEFLAG_HOVER))
                            return;
#else
                        if (mi != NULL && !(mi->hasMovementFlag(MOVEFLAG_FALLING)) && !(mi->hasMovementFlag(MOVEFLAG_SWIMMING)) && !(mi->hasMovementFlag(MOVEFLAG_HOVER)))
                            return;
#endif
                    }
                }
            }
        }
    }

    if (pUnit->IsPlayer() && static_cast< Player* >(pUnit)->GetMisdirectionTarget() != 0)
    {
        Unit* mTarget = m_Unit->GetMapMgr()->GetUnit(static_cast< Player* >(pUnit)->GetMisdirectionTarget());
        if (mTarget != nullptr && mTarget->isAlive())
            pUnit = mTarget;
    }

    if ((isAiState(AI_STATE_IDLE) || isAiState(AI_STATE_FOLLOWING)) && m_Unit->GetAIInterface()->GetAllowedToEnterCombat())
    {
        WipeTargetList();

        HandleEvent(EVENT_ENTERCOMBAT, pUnit, 0);
    }

    if (isAiState(AI_STATE_UNFEARED))
    {
        //we're unfeared resume combat
        HandleEvent(EVENT_ENTERCOMBAT, pUnit, 1);
        removeAiState(AI_STATE_UNFEARED);
    }

    HandleEvent(EVENT_DAMAGETAKEN, pUnit, _CalcThreat(damage_dealt, spellId ? sSpellCustomizations.GetSpellInfo(spellId) : NULL, pUnit));
}

void AIInterface::HealReaction(Unit* caster, Unit* victim, SpellInfo* sp, uint32 amount)
{
    if (!caster || !victim)
        return;

    bool casterInList = false;
    bool victimInList = false;

    if (m_aiTargets.find(caster->GetGUID()) != m_aiTargets.end())
        casterInList = true;

    if (m_aiTargets.find(victim->GetGUID()) != m_aiTargets.end())
        victimInList = true;

    if (!victimInList && !casterInList) // none of the Casters is in the Creatures Threat list
        return;

    int32 threat = int32(amount / 2);
    if (caster->getClass() == PALADIN)
        threat = threat / 2; //Paladins only get 50% threat per heal than other classes

    if (sp != nullptr)
        threat += (threat * caster->GetGeneratedThreatModifyer(sp->getSchool()) / 100);

    if (threat < 1)
        threat = 1;

    if (!casterInList && victimInList) // caster is not yet in Combat but victim is
    {
        // get caster into combat if he's hostile
        if (isHostile(m_Unit, caster))
            m_aiTargets.insert(TargetMap::value_type(caster->GetGUID(), threat));
    }
    else if (casterInList && victimInList) // both are in combat already
    {
        modThreatByPtr(caster, threat);
    }
    else // caster is in Combat already but victim is not
    {
        modThreatByPtr(caster, threat);
        // both are players so they might be in the same group
        if (caster->IsPlayer() && victim->IsPlayer())
        {
            if (static_cast< Player* >(caster)->GetGroup() == static_cast< Player* >(victim)->GetGroup())
            {
                // get victim into combat since they are both
                // in the same party
                if (isHostile(m_Unit, victim))
                    m_aiTargets.insert(TargetMap::value_type(victim->GetGUID(), 1));
            }
        }
    }
}

void AIInterface::OnDeath(Object* pKiller)
{
    if (pKiller->IsUnit())
        HandleEvent(EVENT_UNITDIED, static_cast< Unit* >(pKiller), 0);
    else
        HandleEvent(EVENT_UNITDIED, m_Unit, 0);
}

//function is designed to make a quick check on target to decide if we can attack it
bool AIInterface::UnsafeCanOwnerAttackUnit(Unit* pUnit)
{
    if (!isHostile(m_Unit, pUnit))
        return false;

    if (!pUnit->isAlive())
        return false;

    if (!(pUnit->m_phase & m_Unit->m_phase))   //Not in the same phase
        return false;

    //do not agro units that are faking death. Should this be based on chance ?
    if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
        return false;

    //don't attack owner
    if (m_Unit->GetCreatedByGUID() == pUnit->GetGUID())
        return false;

    //don't agro neutrals
    if ((pUnit->IsPlayer() || pUnit->IsPet())
        && m_Unit->m_factionDBC
        && m_Unit->m_factionDBC->RepListId == -1
        && m_Unit->m_faction->HostileMask == 0
        && m_Unit->m_faction->FriendlyMask == 0
        )
    {
        return false;
    }
    else if ((m_Unit->IsPlayer() || m_Unit->IsPet())
        && pUnit->m_factionDBC
        && pUnit->m_factionDBC->RepListId == -1
        && pUnit->m_faction->HostileMask == 0
        && pUnit->m_faction->FriendlyMask == 0
        )
    {
        return false;
    }

    //make sure we do not agro flying stuff
    if (abs(pUnit->GetPositionZ() - m_Unit->GetPositionZ()) > _CalcCombatRange(pUnit, false))
        return false; //blizz has this set to 250 but uses pathfinding

    return true;
}

//this function might be slow but so it should not be spammed
//!!!this function has been reported the biggest bottleneck on emu in 2008 07 04
Unit* AIInterface::FindTarget()
{
    // find nearest hostile Target to attack
    if (!m_AllowedToEnterCombat)
        return nullptr;

    if (m_Unit->GetMapMgr() == nullptr)
        return nullptr;

    Unit* target = nullptr;

    float distance = 999999.0f; // that should do it.. :p

    //target is immune to all form of attacks, cant attack either.
    // not attackable creatures sometimes fight enemies in scripted fights though
    if (m_Unit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NOT_ATTACKABLE_2))
    {
        return nullptr;
    }

    // Start of neutralguard snippet
    if (m_isNeutralGuard)
    {
        for (std::set< Object*>::iterator itrPlr = m_Unit->GetInRangePlayerSetBegin(); itrPlr != m_Unit->GetInRangePlayerSetEnd(); ++itrPlr)
        {
            Player* tmpPlr = static_cast< Player* >(*itrPlr);

            if (tmpPlr == nullptr)
                continue;

            if (tmpPlr->IsDead())
                continue;

            if (tmpPlr->GetTaxiState())
                continue;

            if (tmpPlr->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_FEIGN_DEATH))
                continue;

            if (tmpPlr->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9))
                continue;

            if (tmpPlr->m_invisible)
                continue;

            if (!tmpPlr->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_CONT_PVP))    //PvP Guard Attackable.
                continue;

            if (!(tmpPlr->m_phase & m_Unit->m_phase))   //Not in the same phase, skip this target
                continue;

            float dist = m_Unit->getDistanceSq(tmpPlr);
            if (dist > 2500.0f)
                continue;

            if (distance > dist)
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    VMAP::IVMapManager* mgr = VMAP::VMapFactory::createOrGetVMapManager();
                    bool los = mgr->isInLineOfSight(m_Unit->GetMapId(), m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), tmpPlr->GetPositionX(), tmpPlr->GetPositionY(), tmpPlr->GetPositionZ());
                    if (los)
                    {
                        distance = dist;
                        target = static_cast<Unit*>(tmpPlr);
                    }
                }
                else
                {
                    distance = dist;
                    target = static_cast<Unit*>(tmpPlr);
                }
            }
        }

        if (target)
        {
            m_Unit->m_currentSpeedRun = m_Unit->m_basicSpeedRun * 2.0f;
            AttackReaction(target, 1, 0);

            m_Unit->SendAIReaction();

            return target;
        }
        distance = 999999.0f; //Reset Distance for normal check
    }
    // End of neutralguard snippet

    //we have a high chance that we will agro a player
    //this is slower then oppfaction list BUT it has a lower chance that contains invalid pointers
    for (std::set<Object*>::iterator pitr2 = m_Unit->GetInRangePlayerSetBegin(); pitr2 != m_Unit->GetInRangePlayerSetEnd();)
    {
        std::set<Object*>::iterator pitr = pitr2;
        ++pitr2;

        Unit* pUnit = static_cast< Player* >(*pitr);

        if (UnsafeCanOwnerAttackUnit(pUnit) == false)
            continue;

        //on blizz there is no Z limit check
        float dist = m_Unit->GetDistance2dSq(pUnit);
        if (dist > distance)     // we want to find the CLOSEST target
            continue;

        if (dist <= _CalcAggroRange(pUnit))
        {
            if (worldConfig.terrainCollision.isCollisionEnabled)
            {
                if (m_Unit->GetMapMgr()->isInLineOfSight(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ() + 2, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ() + 2))
                {
                    distance = dist;
                    target = pUnit;
                }
            }
            else
            {
                distance = dist;
                target = pUnit;
            }
        }
    }

    Unit* critterTarget = nullptr;

    //a lot less times are check inter faction mob wars :)
    if (m_updateTargetsTimer2 < Util::getMSTime())
    {
        m_updateTargetsTimer2 = Util::getMSTime() + TARGET_UPDATE_INTERVAL;

        for (std::set<Object*>::iterator itr2 = m_Unit->GetInRangeSetBegin(); itr2 != m_Unit->GetInRangeSetEnd();)
        {
            std::set<Object*>::iterator itr = itr2;
            ++itr2;

            if (!(*itr)->IsUnit())
                continue;

            Unit* pUnit = static_cast< Unit* >(*itr);

            if (UnsafeCanOwnerAttackUnit(pUnit) == false)
                continue;

            //on blizz there is no Z limit check
            float dist = m_Unit->GetDistance2dSq(pUnit);

            if (pUnit->m_faction && pUnit->m_faction->Faction == 28)// only Attack a critter if there is no other Enemy in range
            {
                if (dist < 225.0f)    // was 10
                    critterTarget = pUnit;

                continue;
            }

            if (dist > distance)     // we want to find the CLOSEST target
                continue;

            if (dist <= _CalcAggroRange(pUnit))
            {
                if (worldConfig.terrainCollision.isCollisionEnabled)
                {
                    if (m_Unit->GetMapMgr()->isInLineOfSight(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ() + 2, pUnit->GetPositionX(), pUnit->GetPositionY(), pUnit->GetPositionZ() + 2))
                    {
                        distance = dist;
                        target = pUnit;
                    }
                }
                else
                {
                    distance = dist;
                    target = pUnit;
                }
            }
        }
    }

    if (!target)
    {
        target = critterTarget;
    }

    if (target)
    {
        AttackReaction(target, 1, 0);

        m_Unit->SendAIReaction();

        if (target->GetCreatedByGUID() != 0)
        {
            uint64 charmer = target->GetCharmedByGUID();

            Unit* target2 = m_Unit->GetMapMgr()->GetPlayer(Arcemu::Util::GUID_LOPART(charmer));

            if (target2)
            {
                AttackReaction(target2, 1, 0);
            }
        }
    }
    return target;
}

Unit* AIInterface::FindTargetForSpell(AI_Spell* sp)
{
    if (sp)
    {
        if (sp->spellType == STYPE_HEAL)
        {
            if (m_Unit->GetHealthPct() / 100.0f <= sp->floatMisc1) // Heal ourselves cause we got too low HP
            {
                m_Unit->SetTargetGUID(0);
                return m_Unit;
            }
            for (AssistTargetSet::iterator i = m_assistTargets.begin(); i != m_assistTargets.end(); ++i)
            {
                if (!(*i)->isAlive())
                {
                    continue;
                }
                if ((*i)->GetHealthPct() / 100.0f <= sp->floatMisc1) // Heal ourselves cause we got too low HP
                {
                    m_Unit->SetTargetGUID((*i)->GetGUID());
                    return (*i); // heal Assist Target which has low HP
                }
            }
        }

        if (sp->spellType == STYPE_BUFF)
        {
            m_Unit->SetTargetGUID(0);
            return m_Unit;
        }
    }

    return GetMostHated();
}

bool AIInterface::FindFriends(float dist)
{
    if (m_Unit->GetMapMgr() == nullptr)
        return false;

    bool result = false;

    for (std::set<Object*>::iterator itr = m_Unit->GetInRangeSetBegin(); itr != m_Unit->GetInRangeSetEnd(); ++itr)
    {
        if (!(*itr)->IsUnit())
            continue;

        Unit* pUnit = static_cast< Unit* >(*itr);
        if (!pUnit->isAlive())
            continue;

        if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE))
            continue;

        if (pUnit->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_9))
            continue;

        if (!(pUnit->m_phase & m_Unit->m_phase))   //We can't help a friendly unit if it is not in our phase
            continue;

        if (isCombatSupport(m_Unit, pUnit) && (pUnit->GetAIInterface()->isAiState(AI_STATE_IDLE) || pUnit->GetAIInterface()->isAiState(AI_STATE_SCRIPTIDLE)))      //Not sure
        {
            if (m_Unit->getDistanceSq(pUnit) < dist)
            {
                if (m_assistTargets.count(pUnit) == 0)
                {
                    result = true;
                    m_assistTargets.insert(pUnit);
                }

                LockAITargets(true);

                for (TargetMap::iterator it = m_aiTargets.begin(); it != m_aiTargets.end(); ++it)
                {
                    Unit* ai_t = m_Unit->GetMapMgr()->GetUnit(it->first);
                    if (ai_t && pUnit->GetAIInterface() && isHostile(ai_t, pUnit))
                        pUnit->GetAIInterface()->AttackReaction(ai_t, 1, 0);
                }

                LockAITargets(false);
            }
        }
    }

    uint32 family = static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Type;

    CreatureProperties const* pt = static_cast< Creature* >(m_Unit)->GetCreatureProperties();

    uint32 summonguard = 0;

    summonguard = pt->summonguard;

    if (family == UNIT_TYPE_HUMANOID && summonguard > 0 && Util::getMSTime() > m_guardTimer && !IS_INSTANCE(m_Unit->GetMapId()))
    {
        m_guardTimer = Util::getMSTime() + 15000;
        DBC::Structures::AreaTableEntry const* at = m_Unit->GetArea();
        if (!at)
            return result;

        MySQLStructure::ZoneGuards const* zoneSpawn;
        if (at->zone != 0)
            zoneSpawn = sMySQLStore.getZoneGuard(at->zone);
        else
            zoneSpawn = sMySQLStore.getZoneGuard(at->id);

        if (!zoneSpawn)
            return result;

        uint32 team = 1; // horde default
        if (isAlliance(m_Unit))
            team = 0;

        uint32 guardid = zoneSpawn->allianceEntry;
        if (team == 1)
            guardid = zoneSpawn->hordeEntry;

        if (!guardid)
            return result;

        CreatureProperties const* cp = sMySQLStore.getCreatureProperties(guardid);
        if (cp == nullptr)
            return result;

        float x = m_Unit->GetPositionX() + ((RandomFloat(150.f) + 100.f) / 1000.0f);
        float y = m_Unit->GetPositionY() + ((RandomFloat(150.f) + 100.f) / 1000.0f);
        float z = m_Unit->GetMapMgr()->GetLandHeight(x, y, m_Unit->GetPositionZ() + 2);

        if (fabs(z - m_Unit->GetPositionZ()) > 10.0f)
            z = m_Unit->GetPositionZ();

        uint8 spawned = 0;

        for (std::set<Object*>::iterator hostileItr = m_Unit->GetInRangePlayerSetBegin(); hostileItr != m_Unit->GetInRangePlayerSetEnd(); ++hostileItr)
        {
            Player* player = static_cast<Player*>(*hostileItr);

            if (spawned >= 3)
                break;

            if (!isHostile(player, m_Unit))
                continue;

            if (spawned == 0)
            {
                uint32 languageid = (team == 0) ? LANG_COMMON : LANG_ORCISH;
                m_Unit->SendChatMessage(CHAT_MSG_MONSTER_SAY, languageid, "Guards!");
            }

            Creature* guard = m_Unit->GetMapMgr()->CreateCreature(guardid);
            guard->Load(cp, x, y, z);
            guard->SetZoneId(m_Unit->GetZoneId());
            guard->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP); /* shitty DBs */
            guard->m_noRespawn = true;

            if (guard->CanAddToWorld())
            {
                uint32 t = RandomUInt(8) * 1000;
                if (t == 0)
                    guard->PushToWorld(m_Unit->GetMapMgr());
                else
                    sEventMgr.AddEvent(guard, &Creature::AddToWorld, m_Unit->GetMapMgr(), EVENT_UNK, t, 1, 0);
            }
            else
            {
                guard->DeleteMe();
                return result;
            }

            sEventMgr.AddEvent(guard, &Creature::SetGuardWaypoints, EVENT_UNK, 10000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            sEventMgr.AddEvent(guard, &Creature::DeleteMe, EVENT_CREATURE_SAFE_DELETE, 60 * 5 * 1000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
            spawned++;
        }
    }

    return result;
}

float AIInterface::_CalcAggroRange(Unit* target)
{
    //float baseAR = 15.0f; // Base Aggro Range
    //                    -8     -7     -6      -5    -4      -3     -2     -1     0      +1     +2     +3    +4     +5     +6     +7    +8
    //float baseAR[17] = {29.0f, 27.5f, 26.0f, 24.5f, 23.0f, 21.5f, 20.0f, 18.5f, 17.0f, 15.5f, 14.0f, 12.5f, 11.0f,  9.5f,  8.0f,  6.5f, 5.0f};
    float baseAR[17] = { 19.0f, 18.5f, 18.0f, 17.5f, 17.0f, 16.5f, 16.0f, 15.5f, 15.0f, 14.5f, 12.0f, 10.5f, 8.5f, 7.5f, 6.5f, 6.5f, 5.0f };
    // Lvl Diff -8 -7 -6 -5 -4 -3 -2 -1 +0 +1 +2  +3  +4  +5  +6  +7  +8
    // Arr Pos   0  1  2  3  4  5  6  7  8  9 10  11  12  13  14  15  16
    int8 lvlDiff = static_cast<int8>(target->getLevel() - m_Unit->getLevel());
    uint8 realLvlDiff = lvlDiff;
    if (lvlDiff > 8)
    {
        lvlDiff = 8;
    }

    if (lvlDiff < -8)
    {
        lvlDiff = -8;
    }

    if (!static_cast<Creature*>(m_Unit)->CanSee(target))
        return 0;

    // Retrieve aggrorange from table
    float AggroRange = baseAR[lvlDiff + 8];

    // Check to see if the target is a player mining a node
    bool isMining = false;
    if (target->IsPlayer())
    {
        if (target->IsCasting())
        {
            // If nearby miners weren't spotted already we'll give them a little surprise.
            Spell* sp = target->GetCurrentSpell();
            if (sp->GetSpellInfo()->getEffect(0) == SPELL_EFFECT_OPEN_LOCK && sp->GetSpellInfo()->getEffectMiscValue(0) == LOCKTYPE_MINING)
            {
                isMining = true;
            }
        }
    }

    // If the target is of a much higher level the aggro range must be scaled down, unless the target is mining a nearby resource node
    if (realLvlDiff > 8 && !isMining)
    {
        AggroRange += AggroRange * ((lvlDiff - 8) * 5 / 100);
    }

    // Multiply by elite value
    if (m_Unit->IsCreature() && static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank > 0)
    {
        AggroRange *= (static_cast<Creature*>(m_Unit)->GetCreatureProperties()->Rank) * 1.50f;
    }

    // Cap Aggro range at 40.0f
    if (AggroRange > 40.0f)
    {
        AggroRange = 40.0f;
    }

    // SPELL_AURA_MOD_DETECT_RANGE
    int32 modDetectRange = target->getDetectRangeMod(m_Unit->GetGUID());
    AggroRange += modDetectRange;
    if (target->IsPlayer())
    {
        AggroRange += static_cast< Player* >(target)->DetectedRange;
    }

    // Re-check if aggro range exceeds Minimum/Maximum caps
    if (AggroRange < 3.0f)
    {
        AggroRange = 3.0f;
    }

    if (AggroRange > 40.0f)
    {
        AggroRange = 40.0f;
    }

    return (AggroRange * AggroRange);
}

void AIInterface::_CalcDestinationAndMove(Unit* target, float dist)
{
    if (!m_canMove || m_Unit->IsStunned())
    {
        StopMovement(0); //Just Stop
        return;
    }

    float newx, newy, newz;

    if (target != nullptr)
    {
        newx = target->GetPositionX();
        newy = target->GetPositionY();
        newz = target->GetPositionZ();

        //avoid eating bandwidth with useless movement packets when target did not move since last position
        //this will work since it turned into a common myth that when you pull mob you should not move :D
        if (abs(m_last_target_x - newx) < minWalkDistance
            && abs(m_last_target_y - newy) < minWalkDistance && isCreatureState(MOVING))
            return;

        m_last_target_x = newx;
        m_last_target_y = newy;

        float angle = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), newx, newy) * M_PI_FLOAT / 180.0f;
        float x = dist * cosf(angle);
        float y = dist * sinf(angle);

        if (target->IsPlayer() && static_cast<Player*>(target)->m_isMoving)
        {
            // cater for moving player vector based on orientation
            x -= cosf(target->GetOrientation());
            y -= sinf(target->GetOrientation());
        }

        newx -= x;
        newy -= y;
    }
    else
    {
        newx = m_Unit->GetPositionX();
        newy = m_Unit->GetPositionY();
        newz = m_Unit->GetPositionZ();
    }

    if (!generateAndSendSplinePath(newx, newy, newz))
    {
        ///\todo enter evade mode if creature, not pet, not totem
    }
}

float AIInterface::_CalcCombatRange(Unit* target, bool ranged)
{
    if (!target)
        return 0.0f;

    float rang = 0.0f;
    if (ranged)
        rang = 5.0f;

    float selfreach = m_Unit->GetCombatReach();
    float targetradius;
    //    targetradius = target->GetBoundingRadius(); //this is plain wrong. Represents i have no idea what :)
    targetradius = target->GetModelHalfSize();
    float selfradius;
    //    selfradius = m_Unit->GetBoundingRadius(); //this is plain wrong. Represents i have no idea what :)
    selfradius = m_Unit->GetModelHalfSize();
    //    float targetscale = target->GetScale();
    //    float selfscale = m_Unit->GetScale();

    //    range = ((((targetradius*targetradius)*targetscale) + selfreach) + ((selfradius*selfscale) + rang));
    float range = targetradius + selfreach + selfradius + rang;
    //    if (range > 28.29f) range = 28.29f;
    return range;
}

float AIInterface::_CalcDistanceFromHome()
{
    if (isAiScriptType(AI_SCRIPT_PET))
    {
        return m_Unit->getDistanceSq(m_PetOwner);
    }
    else if (m_Unit->IsCreature())
    {
        if (m_combatResetX != 0 && m_combatResetY != 0)
            return m_Unit->getDistanceSq(m_combatResetX, m_combatResetY, m_combatResetZ);

        if (m_returnX != 0.0f && m_returnY != 0.0f)
            return m_Unit->getDistanceSq(m_returnX, m_returnY, m_returnZ);
    }

    return 0.0f;
}

/************************************************************************************************************
SendMoveToPacket:
Comments: Some comments on the SMSG_MONSTER_MOVE packet:
the uint8 field:
0: Default                                                            known
1: Don't move                                                        known
2: there is an extra 3 floats, also known as a vector                unknown
3: there is an extra uint64 most likely a guid.                        unknown
4: there is an extra float that causes the orientation to be set.    known

note:    when this field is 1.
there is no need to send  the next 3 uint32's as they aren't used by the client

the MoveFlags:
0x00000000 - Walk
0x00000100 - Teleport
0x00001000 - Run
0x00000200 - Fly - OLD FLAG, IS THIS STILL VALID?
0x00003000 - Fly
some comments on that 0x00000300 - Fly = 0x00000100 | 0x00000200

waypoint's:
\todo as they somehow seemed to be changed long time ago..

*************************************************************************************************************/

void AIInterface::SendMoveToPacket()
{
    ::Packets::Movement::SendMoveToPacket(m_Unit);
}

bool AIInterface::StopMovement(uint32 time)
{
    if (m_Unit->GetCurrentVehicle() != nullptr)
    {
        return true;
    }

    mSplinePriority = SPLINE_PRIORITY_MOVEMENT;

    if (m_Unit->GetMapMgr() != nullptr)
    {
        UpdateMovementSpline();
    }

    m_moveTimer = time; //set pause after stopping

    //Clear current spline
    m_Unit->m_movementManager.m_spline.ClearSpline();
    m_Unit->m_movementManager.ForceUpdate();

    if (m_Unit->GetMapMgr() != nullptr)
    {
        SendMoveToPacket();
    }

    return true;
}

bool AIInterface::MoveTo(float x, float y, float z, float o /*= 0.0f*/)
{
    if (!m_canMove || m_Unit->IsStunned())
    {
        StopMovement(0); //Just Stop
        return false;
    }

    if (!generateAndSendSplinePath(x, y, z, o))
        return false;

    if (!isCreatureState(MOVING))
        setCreatureState(MOVING);

    return true;
}

void AIInterface::UpdateSpeeds()
{
    if (hasWalkMode(WALKMODE_SPRINT))
        m_runSpeed = (m_Unit->m_currentSpeedRun + 5.0f) * 0.001f;

    if (hasWalkMode(WALKMODE_RUN))
        m_runSpeed = m_Unit->m_currentSpeedRun * 0.001f;

    m_walkSpeed = m_Unit->m_currentSpeedWalk * 0.001f;
    m_flySpeed = m_Unit->m_currentSpeedFly * 0.001f;
}

void AIInterface::SendCurrentMove(Player* plyr)
{
    if (m_Unit->m_movementManager.m_spline.IsSplineMoveDone())
        return;

    auto start = m_Unit->m_movementManager.m_spline.GetFirstSplinePoint();
    uint32 timepassed = Util::getMSTime() - start.setoff;

    ByteBuffer* splineBuf = new ByteBuffer(20 * 4);
    *splineBuf << uint32(0); // spline flags
    *splineBuf << uint32(timepassed); //Time Passed (start Position)
    *splineBuf << uint32(m_Unit->m_movementManager.m_spline.m_currentSplineTotalMoveTime); //Total Time
    *splineBuf << uint32(0); //Unknown
    *splineBuf << float(0); //unk
    *splineBuf << float(0); //unk

    *splineBuf << float(0); //trajectory parabolic soeed
    *splineBuf << uint32(0); //trajectory time

    if (m_Unit->m_movementManager.m_spline.GetSplinePoints()->size() < 4)  //client requires 4, lets generate shit for it
    {
        *splineBuf << uint32(m_Unit->m_movementManager.m_spline.GetSplinePoints()->size() + 1 /* 1 fake start */ + 2 /* 2 fake ends */); //Spline Count
        auto end = m_Unit->m_movementManager.m_spline.GetLastSplinePoint();

        *splineBuf << start.pos.x << start.pos.y << start.pos.z;
        auto splinePoints = *m_Unit->m_movementManager.m_spline.GetSplinePoints();
        for (auto point : splinePoints)
        {
            *splineBuf << point.pos.x;
            *splineBuf << point.pos.y;
            *splineBuf << point.pos.z;
        }

        *splineBuf << end.pos.x << end.pos.y << end.pos.z + 0.1f;
        *splineBuf << end.pos.x << end.pos.y << end.pos.z + 0.2f;
        *splineBuf << uint8(0);
        *splineBuf << end.pos.x << end.pos.y << end.pos.z;
    }
    else
    {
        *splineBuf << uint32(m_Unit->m_movementManager.m_spline.GetSplinePoints()->size());
        auto splinePoints = *m_Unit->m_movementManager.m_spline.GetSplinePoints();
        for (auto point : splinePoints)
        {
            *splineBuf << point.pos.x;
            *splineBuf << point.pos.y;
            *splineBuf << point.pos.z;
        }
        auto end = m_Unit->m_movementManager.m_spline.GetLastSplinePoint();
        *splineBuf << uint8(0);
        *splineBuf << end.pos.x << end.pos.y << end.pos.z;
    }

    plyr->AddSplinePacket(m_Unit->GetGUID(), splineBuf);
}

bool AIInterface::setInFront(Unit* target) // not the best way to do it, though
{
    //angle the object has to face
    float angle = m_Unit->calcAngle(m_Unit->GetPositionX(), m_Unit->GetPositionY(), target->GetPositionX(), target->GetPositionY());
    //Change angle slowly 2000ms to turn 180 deg around
    if (angle > 180)
        angle += 90;
    else
        angle -= 90; //angle < 180

    //m_Unit->getEasyAngle(angle); These things happen with shitty style. CID53249 (Useless call)
    //Convert from degrees to radians (180 deg = PI rad)
    float orientation = angle / (180 / M_PI_FLOAT);
    //Update Orientation Server Side
    m_Unit->SetPosition(m_Unit->GetPositionX(), m_Unit->GetPositionY(), m_Unit->GetPositionZ(), orientation);

    return m_Unit->isInFront(target);
}

void AIInterface::_UpdateMovement(uint32 p_time)
{
    if (!m_Unit->isAlive())
    {
        StopMovement(0);
        return;
    }

    //move after finishing our current spell
    if (m_Unit->GetCurrentSpell() != NULL)
        return;

    uint32 timediff = 0;

    if (m_moveTimer > 0)
    {
        if (p_time >= m_moveTimer)
        {
            timediff = p_time - m_moveTimer;
            m_moveTimer = 0;
        }
        else
            m_moveTimer -= p_time;
    }

    if (m_timeToMove > 0)
    {
        m_timeMoved = m_timeToMove <= p_time + m_timeMoved ? m_timeToMove : p_time + m_timeMoved;
    }

    if (m_Unit->IsCreature())
    {
        if (isAiState(AI_STATE_IDLE))
        {
            if (getUnitToFollow() == nullptr)
            {
                switch (m_Unit->GetAIInterface()->getWaypointScriptType())
                {
                    case Movement::WP_MOVEMENT_SCRIPT_CIRCLEWP:
                        generateWaypointScriptCircle();
                        break;
                    case Movement::WP_MOVEMENT_SCRIPT_RANDOMWP:
                        generateWaypointScriptRandom();
                        break;
                    case Movement::WP_MOVEMENT_SCRIPT_FORWARDTHENSTOP:
                        generateWaypointScriptForwad();
                        break;
                    case Movement::WP_MOVEMENT_SCRIPT_WANTEDWP:
                        generateWaypointScriptWantedWP();
                        break;
                    case Movement::WP_MOVEMENT_SCRIPT_PATROL:
                        generateWaypointScriptPatrol();
                        break;
                    default:
                    {
                        updateOrientation();
                    } break;
                }
            }
            else
            {
                setFormationMovement();
            }
        }
    }
    else
    {
        LOG_DEBUG("Called new Waypoint Generator for Player!");
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // fear movement
    setFearRandomMovement();

    //////////////////////////////////////////////////////////////////////////////////////////
    // follow movement
    setPetFollowMovement();
}

void AIInterface::CastSpell(Unit* caster, SpellInfo* spellInfo, SpellCastTargets targets)
{
    ARCEMU_ASSERT(spellInfo != NULL);
    if (!isAiScriptType(AI_SCRIPT_PET) && isCastDisabled())
        return;

    // Stop movement while casting.
    setAiState(AI_STATE_CASTING);
#ifdef _AI_DEBUG
    LOG_DEBUG("AI DEBUG: Unit %u casting spell %s on target " I64FMT " ", caster->GetEntry(),
              sSpellStore.LookupString(spellInfo->Name), targets.m_unitTarget);
#endif

    //i wonder if this will lead to a memory leak :S
    Spell* nspell = sSpellFactoryMgr.NewSpell(caster, spellInfo, false, NULL);
    nspell->prepare(&targets);
}

SpellInfo* AIInterface::getSpellEntry(uint32 spellId)
{
    SpellInfo* spellInfo = sSpellCustomizations.GetSpellInfo(spellId);

    if (!spellInfo)
    {
        LOG_ERROR("WORLD: unknown spell id %i", spellId);
        return NULL;
    }

    return spellInfo;
}

SpellCastTargets AIInterface::setSpellTargets(SpellInfo* spellInfo, Unit* target) const
{
    SpellCastTargets targets;
    targets.m_unitTarget = target ? target->GetGUID() : 0;
    targets.m_itemTarget = 0;
    targets.setSource(m_Unit->GetPosition());
    targets.setDestination(m_Unit->GetPosition());

    if (m_nextSpell && m_nextSpell->spelltargetType == TTYPE_SINGLETARGET)
    {
        targets.m_targetMask = TARGET_FLAG_UNIT;
    }
    else if (m_nextSpell && m_nextSpell->spelltargetType == TTYPE_SOURCE)
    {
        targets.m_targetMask = TARGET_FLAG_SOURCE_LOCATION;
    }
    else if (m_nextSpell && m_nextSpell->spelltargetType == TTYPE_DESTINATION)
    {
        targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;
        if (target)
        {
            targets.setDestination(target->GetPosition());
        }
    }
    else if (m_nextSpell && m_nextSpell->spelltargetType == TTYPE_CASTER)
    {
        targets.m_targetMask = TARGET_FLAG_UNIT;
        targets.m_unitTarget = m_Unit->GetGUID();
    }

    return targets;
}

AI_Spell* AIInterface::getSpell()
{
    if (next_spell_time > (uint32)UNIXTIME)
        return nullptr;

    waiting_for_cooldown = false;

    // look at our spells
    AI_Spell* sp = nullptr;
    AI_Spell* def_spell = nullptr;
    uint32 cool_time = 0;
    uint32 cool_time2;
    uint32 nowtime = Util::getMSTime();

    if (m_Unit->IsPet())
    {
        sp = def_spell = static_cast<Pet*>(m_Unit)->HandleAutoCastEvent();
    }
    else
    {
        for (std::list<AI_Spell*>::iterator itr = m_spells.begin(); itr != m_spells.end();)
        {
            sp = *itr;
            ++itr;
            if (nowtime < sp->cooldowntime)
            {
                cool_time2 = sp->cooldowntime - nowtime;
                if (!cool_time || cool_time2 < cool_time)
                    cool_time = cool_time2;

                waiting_for_cooldown = true;
                continue;
            }

            if (sp->procCount && sp->procCounter >= sp->procCount)
                continue;

            if (sp->agent == AGENT_SPELL)
            {
                if (sp->spellType == STYPE_BUFF)
                {
                    // cast the buff at requested percent only if we don't have it already
                    if (sp->procChance >= 100 || Rand(sp->procChance))
                    {
                        if (!m_Unit->HasBuff(sp->spell->getId()))
                        {
                            return sp;
                        }
                    }
                }
                else
                {
                    // cast the spell at requested percent.
                    if (sp->procChance >= 100 || Rand(sp->procChance))
                    {
                        //focus/mana requirement
                        switch (sp->spell->getPowerType())
                        {
                            case POWER_TYPE_MANA:
                            {
                                if (m_Unit->GetPower(POWER_TYPE_MANA) < sp->spell->getManaCost())
                                    continue;
                            } break;
                            case POWER_TYPE_FOCUS:
                            {
                                if (m_Unit->GetPower(POWER_TYPE_FOCUS) < sp->spell->getManaCost())
                                    continue;
                            } break;
                        }

                        def_spell = sp;
                        //we got a selected spell, we can exit loop now
                        break;
                    }
                    else //we failed casting it due to given chance, we activate false cooldown on it to not spam searching this list
                    {
                        cool_time2 = 2000;
                        sp->cooldowntime = nowtime + cool_time2;
                        if (!cool_time || cool_time2 < cool_time)
                            cool_time = cool_time2;
                    }
                }
            }
        }
    }

    if (def_spell)
    {
        // set cooldown
        if (def_spell->procCount)
            def_spell->procCounter++;

        if (def_spell->cooldown)
            def_spell->cooldowntime = nowtime + def_spell->cooldown;

        waiting_for_cooldown = false;
        return def_spell;
    }

    // save some loops if waiting for cooldownz
    if (cool_time)
    {
        cool_time2 = cool_time / 1000;
        if (cool_time2)
            next_spell_time = (uint32)UNIXTIME + cool_time2;
    }
    else
    {
        next_spell_time = (uint32)UNIXTIME + MOB_SPELLCAST_REFRESH_COOLDOWN_INTERVAL;
        waiting_for_cooldown = false;
    }

#ifdef _AI_DEBUG
    LOG_DEBUG("AI DEBUG: Returning no spell for unit %u", m_Unit->GetEntry());
#endif
    return nullptr;
}

void AIInterface::addSpellToList(AI_Spell* sp)
{
    if (!sp || !sp->spell)
        return;

    AI_Spell* sp2 = new AI_Spell;
    memcpy(sp2, sp, sizeof(AI_Spell));
    m_spells.push_back(sp2);
}

uint32 AIInterface::getThreatByGUID(uint64 guid)
{
    if (m_Unit->GetMapMgr() == nullptr)
        return 0;

    Unit* obj = m_Unit->GetMapMgr()->GetUnit(guid);
    if (obj)
        return getThreatByPtr(obj);

    return 0;
}

uint32 AIInterface::getThreatByPtr(Unit* obj)
{
    if (!obj || m_Unit->GetMapMgr() == nullptr)
        return 0;

    TargetMap::iterator it = m_aiTargets.find(obj->GetGUID());
    if (it != m_aiTargets.end())
    {
        Unit* tempUnit = m_Unit->GetMapMgr()->GetUnit(it->first);
        if (tempUnit)
            return it->second + tempUnit->GetThreatModifyer();
        else
            return it->second;

    }
    return 0;
}

//should return a valid target
Unit* AIInterface::GetMostHated()
{
    if (m_Unit->GetMapMgr() == nullptr)
        return nullptr;

    Unit* ResultUnit = nullptr;

    //override mosthated with taunted target. Basic combat checks are made for it.
    //What happens if we can't see tauntedby unit ?
    ResultUnit = getTauntedBy();
    if (ResultUnit != nullptr)
        return ResultUnit;

    std::pair<Unit*, int32> currentTarget;
    currentTarget.first = 0;
    currentTarget.second = -1;

    LockAITargets(true);

    for (TargetMap::iterator it2 = m_aiTargets.begin(); it2 != m_aiTargets.end();)
    {
        TargetMap::iterator itr = it2;
        ++it2;

        /* check the target is valid */
        Unit* ai_t = m_Unit->GetMapMgr()->GetUnit(itr->first);
        if (!ai_t || ai_t->GetInstanceID() != m_Unit->GetInstanceID() || !ai_t->isAlive() || !isAttackable(m_Unit, ai_t))
        {
            if (getNextTarget() == ai_t)
                resetNextTarget();

            m_aiTargets.erase(itr);
            continue;
        }

        if ((itr->second + ai_t->GetThreatModifyer()) > currentTarget.second)
        {
            /* new target */
            currentTarget.first = ai_t;
            currentTarget.second = itr->second + ai_t->GetThreatModifyer();
            m_currentHighestThreat = currentTarget.second;
        }

        /* there are no more checks needed here... the needed checks are done by CheckTarget() */
    }

    LockAITargets(false);

    return currentTarget.first;
}

Unit* AIInterface::GetSecondHated()
{
    if (m_Unit->GetMapMgr() == nullptr)
        return nullptr;

    Unit* ResultUnit = GetMostHated();

    std::pair<Unit*, int32> currentTarget;
    currentTarget.first = 0;
    currentTarget.second = -1;

    LockAITargets(true);

    for (TargetMap::iterator it2 = m_aiTargets.begin(); it2 != m_aiTargets.end();)
    {
        TargetMap::iterator itr = it2;
        ++it2;

        /* check the target is valid */
        Unit* ai_t = m_Unit->GetMapMgr()->GetUnit(itr->first);
        if (!ai_t || ai_t->GetInstanceID() != m_Unit->GetInstanceID() || !ai_t->isAlive() || !isAttackable(m_Unit, ai_t))
        {
            m_aiTargets.erase(itr);
            continue;
        }

        if ((itr->second + ai_t->GetThreatModifyer()) > currentTarget.second && ai_t != ResultUnit)
        {
            /* new target */
            currentTarget.first = ai_t;
            currentTarget.second = itr->second + ai_t->GetThreatModifyer();
            m_currentHighestThreat = currentTarget.second;
        }
    }

    LockAITargets(false);

    return currentTarget.first;
}

bool AIInterface::modThreatByGUID(uint64 guid, int32 mod)
{
    if (!m_aiTargets.size())
        return false;

    if (m_Unit->GetMapMgr() == nullptr)
        return false;

    Unit* obj = m_Unit->GetMapMgr()->GetUnit(guid);
    if (obj)
        return modThreatByPtr(obj, mod);

    return false;
}

bool AIInterface::modThreatByPtr(Unit* obj, int32 mod)
{
    if (!obj)
        return false;

    LockAITargets(true);

    int32 tempthreat;
    TargetMap::iterator it = m_aiTargets.find(obj->GetGUID());
    if (it != m_aiTargets.end())
    {
        it->second += mod;
        if (it->second < 1)
            it->second = 1;

        tempthreat = it->second + obj->GetThreatModifyer();
        if (tempthreat < 1)
            tempthreat = 1;

        if (tempthreat > m_currentHighestThreat)
        {
            // new target!
            if (!isTaunted)
            {
                m_currentHighestThreat = tempthreat;
                setNextTarget(obj);
            }
        }
    }
    else
    {
        m_aiTargets.insert(std::make_pair(obj->GetGUID(), mod));

        tempthreat = mod + obj->GetThreatModifyer();
        if (tempthreat < 1)
            tempthreat = 1;

        if (tempthreat > m_currentHighestThreat)
        {
            if (!isTaunted)
            {
                m_currentHighestThreat = tempthreat;
                setNextTarget(obj);
            }
        }
    }

    LockAITargets(false);

    if (obj == getNextTarget())
    {
        // check for a possible decrease in threat.
        if (mod < 0)
        {
            setNextTarget(GetMostHated());
            //if there is no more new targets then we can walk back home ?
            if (!getNextTarget())
                HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }
    return true;
}

void AIInterface::RemoveThreatByGUID(uint64 guid)
{
    if (!m_aiTargets.size())
        return;

    if (m_Unit->GetMapMgr() == nullptr)
        return;

    Unit* obj = m_Unit->GetMapMgr()->GetUnit(guid);
    if (obj)
        RemoveThreatByPtr(obj);
}

void AIInterface::RemoveThreatByPtr(Unit* obj)
{
    if (!obj)
        return;

    LockAITargets(true);

    TargetMap::iterator it = m_aiTargets.find(obj->GetGUID());
    if (it != m_aiTargets.end())
    {
        m_aiTargets.erase(it);
        //check if we are in combat and need a new target
        if (obj == getNextTarget())
        {
            setNextTarget(GetMostHated());
            //if there is no more new targets then we can walk back home ?
            if (!getNextTarget())
                HandleEvent(EVENT_LEAVECOMBAT, m_Unit, 0);
        }
    }

    LockAITargets(false);
}

void AIInterface::addAssistTargets(Unit* Friend)
{
    // stop adding stuff that gives errors on linux!
    m_assistTargets.insert(Friend);
}

void AIInterface::WipeHateList()
{
    for (TargetMap::iterator itr = m_aiTargets.begin(); itr != m_aiTargets.end(); ++itr)
    {
        itr->second = 0;
    }

    m_currentHighestThreat = 0;
}
void AIInterface::ClearHateList() //without leaving combat
{
    for (TargetMap::iterator itr = m_aiTargets.begin(); itr != m_aiTargets.end(); ++itr)
    {
        itr->second = 1;
    }

    m_currentHighestThreat = 1;
}

void AIInterface::WipeTargetList()
{
    resetNextTarget();

    m_nextSpell = nullptr;
    m_currentHighestThreat = 0;
    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    m_Unit->CombatStatus.Vanished();
}

bool AIInterface::taunt(Unit* caster, bool apply)
{
    if (apply)
    {
        //wowwiki says that we cannot override this spell
        if (GetIsTaunted())
            return false;

        if (!caster)
        {
            isTaunted = false;
            return false;
        }

        //check if we can attack taunter. Maybe it's a hack or a bug if we fail this test
        if (isHostile(m_Unit, caster))
        {
            //check if we have to add him to our agro list
            //GetMostHated(); //update our most hated list/ Note that at this point we do not have a taunter yet. If we would have then this funtion will not give real mosthated
            int32 oldthreat = getThreatByPtr(caster);
            //make sure we rush the target anyway. Since we are not taunted yet, this will also set our target
            modThreatByPtr(caster, abs(m_currentHighestThreat - oldthreat) + 1); //we need to be the most hated at this moment
            //            SetNextTarget(caster);
        }
        isTaunted = true;
        tauntedBy = caster;
    }
    else
    {
        isTaunted = false;
        tauntedBy = nullptr;
        //taunt is over, we should get a new target based on most hated list
        setNextTarget(GetMostHated());
    }

    return true;
}

Unit* AIInterface::getTauntedBy()
{
    if (GetIsTaunted())
        return tauntedBy;
    else
        return nullptr;
}

bool AIInterface::GetIsTaunted()
{
    if (isTaunted)
    {
        if (!tauntedBy || !tauntedBy->isAlive())
        {
            isTaunted = false;
            tauntedBy = nullptr;
        }
    }
    return isTaunted;
}

void AIInterface::SetSoulLinkedWith(Unit* target)
{
    if (!target)
        return;

    soullinkedWith = target;
    isSoulLinked = true;
}

Unit* AIInterface::getSoullinkedWith()
{
    if (GetIsTaunted())
        return soullinkedWith;
    else
        return nullptr;
}

bool AIInterface::GetIsSoulLinked()
{
    if (isSoulLinked)
    {
        if (!soullinkedWith || !soullinkedWith->isAlive())
        {
            isSoulLinked = false;
            soullinkedWith = nullptr;
        }
    }
    return isSoulLinked;
}

void AIInterface::CheckTarget(Unit* target)
{
    if (target == nullptr)
        return;

    if (target->GetGUID() == getUnitToFollowGUID())            // fix for crash here
    {
        m_UnitToFollow = 0;
        m_lastFollowX = m_lastFollowY = 0;
        FollowDistance = 0;
    }

    if (target->GetGUID() == getUnitToFollowBackupGUID())
    {
        m_UnitToFollow_backup = 0;
    }

    AssistTargetSet::iterator  itr = m_assistTargets.find(target);
    if (itr != m_assistTargets.end())
        m_assistTargets.erase(itr);


    LockAITargets(true);

    TargetMap::iterator it2 = m_aiTargets.find(target->GetGUID());
    if (it2 != m_aiTargets.end() || target == getNextTarget())
    {
        target->CombatStatus.RemoveAttacker(m_Unit, m_Unit->GetGUID());
        m_Unit->CombatStatus.RemoveAttackTarget(target);

        if (it2 != m_aiTargets.end())
        {
            m_aiTargets.erase(it2);
        }

        if (target == getNextTarget())      // no need to cast on these.. mem addresses are still the same
        {
            resetNextTarget();
            m_nextSpell = nullptr;

            // find the one with the next highest threat
            GetMostHated();
        }
    }

    LockAITargets(false);

    if (target->IsCreature())
    {
        it2 = target->GetAIInterface()->m_aiTargets.find(m_Unit->GetGUID());
        if (it2 != target->GetAIInterface()->m_aiTargets.end())
        {
            target->GetAIInterface()->LockAITargets(true);
            target->GetAIInterface()->m_aiTargets.erase(it2);
            target->GetAIInterface()->LockAITargets(false);
        }

        if (target->GetAIInterface()->getNextTarget() == m_Unit)
        {
            target->GetAIInterface()->resetNextTarget();
            target->GetAIInterface()->m_nextSpell = nullptr;
            target->GetAIInterface()->GetMostHated();
        }

        if (target->GetAIInterface()->getUnitToFollowGUID() == m_Unit->GetGUID())
            target->GetAIInterface()->m_UnitToFollow = 0;
    }

    if (target->GetGUID() == getUnitToFearGUID())
        m_UnitToFear = 0;

    if (tauntedBy == target)
        tauntedBy = nullptr;
}

uint32 AIInterface::_CalcThreat(uint32 damage, SpellInfo* sp, Unit* Attacker)
{
    if (!Attacker)
        return 0; // No attacker means no threat and we prevent crashes this way

    if (m_Unit->m_faction != nullptr && Attacker->m_faction != nullptr)
        if (isSameFaction(m_Unit, Attacker))
            return 0;

    int32 mod = 0;
    if (sp != nullptr && sp->custom_ThreatForSpell != 0)
    {
        mod = sp->custom_ThreatForSpell;
    }
    if (sp != nullptr && sp->custom_ThreatForSpellCoef != 0.0f)
        mod += int32(damage * sp->custom_ThreatForSpellCoef);
    else
        mod += damage;

    if (sp != nullptr)
    {
        ascemu::World::Spell::Helpers::spellModFlatIntValue(Attacker->SM_FThreat, &mod, sp->getSpellGroupType());
        ascemu::World::Spell::Helpers::spellModPercentageIntValue(Attacker->SM_PThreat, &mod, sp->getSpellGroupType());
    }

    if (Attacker->getClass() == ROGUE)
        mod = int32(mod * 0.71); // Rogues generate 0.71x threat per damage.

    // modify threat by Buffs
    if (sp != nullptr)
        mod += (mod * Attacker->GetGeneratedThreatModifyer(sp->getSchool()) / 100);
    else
        mod += (mod * Attacker->GetGeneratedThreatModifyer(0) / 100);

    if (mod < 1)
        mod = 1;

    return mod;
}

void AIInterface::WipeReferences()
{
    m_nextSpell = 0;
    m_currentHighestThreat = 0;
    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    resetNextTarget();
    m_UnitToFear = 0;
    m_UnitToFollow = 0;
    tauntedBy = 0;

    //Clear targettable
    for (std::set<Object*>::iterator itr = m_Unit->GetInRangeSetBegin(); itr != m_Unit->GetInRangeSetEnd(); ++itr)
    {
        if ((*itr)->IsUnit() && static_cast<Unit*>(*itr)->GetAIInterface())
        {
            static_cast<Unit*>(*itr)->GetAIInterface()->RemoveThreatByPtr(m_Unit);
        }
    }
}

void AIInterface::ResetProcCounts()
{
    for (std::list<AI_Spell*>::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if ((*itr)->procCount)
        {
            (*itr)->procCounter = 0;
        }
    }
}

void AIInterface::EventChangeFaction(Unit* ForceAttackersToHateThisInstead)
{
    m_nextSpell = 0;
    m_currentHighestThreat = 0;
    //we need a new hatred list
    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    //we need a new assist list
    m_assistTargets.clear();
    //Clear targettable
    if (ForceAttackersToHateThisInstead == nullptr)
    {
        for (std::set<Object*>::iterator itr = m_Unit->GetInRangeSetBegin(); itr != m_Unit->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsUnit() && static_cast<Unit*>(*itr)->GetAIInterface())
            {
                static_cast<Unit*>(*itr)->GetAIInterface()->RemoveThreatByPtr(m_Unit);
            }
        }

        resetNextTarget();
    }
    else
    {
        for (std::set<Object*>::iterator itr = m_Unit->GetInRangeSetBegin(); itr != m_Unit->GetInRangeSetEnd(); ++itr)
        {
            if ((*itr)->IsUnit() && static_cast<Unit*>(*itr)->GetAIInterface()
                && static_cast<Unit*>(*itr)->GetAIInterface()->getThreatByPtr(m_Unit))   //this guy will join me in fight since I'm telling him "sorry i was controlled"
            {
                static_cast<Unit*>(*itr)->GetAIInterface()->modThreatByPtr(ForceAttackersToHateThisInstead, 10);   //just aping to be bale to hate him in case we got nothing else
                static_cast<Unit*>(*itr)->GetAIInterface()->RemoveThreatByPtr(m_Unit);
            }
        }

        modThreatByPtr(ForceAttackersToHateThisInstead, 1);
        setNextTarget(ForceAttackersToHateThisInstead);
    }
}

void AIInterface::WipeCurrentTarget()
{
    Unit* nextTarget = getNextTarget();
    if (nextTarget)
    {
        LockAITargets(true);
        TargetMap::iterator itr = m_aiTargets.find(nextTarget->GetGUID());
        if (itr != m_aiTargets.end())
            m_aiTargets.erase(itr);

        LockAITargets(false);

        if (nextTarget->GetGUID() == getUnitToFollowGUID())
            m_UnitToFollow = 0;

        if (nextTarget->GetGUID() == getUnitToFollowBackupGUID())
            m_UnitToFollow_backup = 0;
    }

    resetNextTarget();
}

Unit* AIInterface::getNextTarget()
{
    if (m_nextTarget && m_Unit && m_Unit->GetMapMgr())
        return m_Unit->GetMapMgr()->GetUnit(m_nextTarget);
    else
        return nullptr;
}

void AIInterface::setNextTarget(Unit* nextTarget)
{
    if (nextTarget)
        setNextTarget(nextTarget->GetGUID());
    else
        resetNextTarget();
}

void AIInterface::setNextTarget(uint64 nextTarget)
{
    m_nextTarget = nextTarget;
    m_Unit->SetTargetGUID(m_nextTarget);
}

void AIInterface::resetNextTarget()
{
    m_nextTarget = 0;
    m_Unit->SetTargetGUID(0);
}

Unit* AIInterface::getUnitToFollow()
{
    if (m_UnitToFollow == 0)
        return nullptr;

    Unit* unit = m_Unit->GetMapMgrUnit(m_UnitToFollow);
    if (unit == nullptr)
        m_UnitToFollow = 0;

    return unit;
}

Unit* AIInterface::getUnitToFollowBackup()
{
    if (m_UnitToFollow_backup == 0)
        return nullptr;

    Unit* unit = m_Unit->GetMapMgrUnit(m_UnitToFollow_backup);
    if (unit == nullptr)
        m_UnitToFollow_backup = 0;

    return unit;
}

Unit* AIInterface::getUnitToFear()
{
    if (m_UnitToFear == 0)
        return nullptr;

    Unit* unit = m_Unit->GetMapMgrUnit(m_UnitToFear);
    if (unit == nullptr)
        m_UnitToFear = 0;

    return unit;
}

Creature* AIInterface::getFormationLinkTarget()
{
    if (m_formationLinkTarget == 0)
        return nullptr;

    Creature* creature = m_Unit->GetMapMgrCreature(m_formationLinkTarget);
    if (creature == nullptr)
        m_formationLinkTarget = 0;

    return creature;
}

void AIInterface::LoadWaypointMapFromDB(uint32 spawnid)
{
    mWayPointMap = objmgr.GetWayPointMap(spawnid);
    if (mWayPointMap != nullptr && mWayPointMap->size() != 0)
        mWaypointMapIsLoadedFromDB = true;
}

void AIInterface::SetWaypointMap(Movement::WayPointMap* m, bool delete_old_map)
{
    if (mWayPointMap == m)
        return;

    if (delete_old_map)
        deleteAllWayPoints();

    mWayPointMap = m;
    mWaypointMapIsLoadedFromDB = false;
}

void AIInterface::_UpdateTotem(uint32 p_time)
{
    ARCEMU_ASSERT(totemspell != 0);
    if (p_time >= m_totemspelltimer)
    {
        Spell* pSpell = sSpellFactoryMgr.NewSpell(m_Unit, totemspell, true, 0);
        Unit* nextTarget = getNextTarget();
        if (nextTarget == NULL ||
            (!m_Unit->GetMapMgr()->GetUnit(nextTarget->GetGUID()) ||
            !nextTarget->isAlive() ||
            !(m_Unit->isInRange(nextTarget->GetPosition(), pSpell->GetSpellInfo()->custom_base_range_or_radius_sqr)) ||
            !isAttackable(m_Unit, nextTarget, !(pSpell->GetSpellInfo()->custom_c_is_flags & SPELL_FLAG_IS_TARGETINGSTEALTHED))
           )
           )
        {
            //we set no target and see if we managed to fid a new one
            resetNextTarget();
            //something happened to our target, pick another one
            SpellCastTargets targets(0);
            pSpell->GenerateTargets(&targets);
            if (targets.m_targetMask & TARGET_FLAG_UNIT)
                setNextTarget(targets.m_unitTarget);
        }
        nextTarget = getNextTarget();
        if (nextTarget)
        {
            SpellCastTargets targets(nextTarget->GetGUID());
            pSpell->prepare(&targets);
            // need proper cooldown time!
            m_totemspelltimer = m_totemspelltime;
        }
        else
        {
            delete pSpell;
            pSpell = nullptr;
        }
        // these will *almost always* be AoE, so no need to find a target here.
        //            SpellCastTargets targets(m_Unit->GetGUID());
        //            Spell* pSpell = sSpellFactoryMgr.NewSpell(m_Unit, totemspell, true, 0);
        //            pSpell->prepare(&targets);
        // need proper cooldown time!
        //            m_totemspelltimer = m_totemspelltime;
    }
    else
    {
        m_totemspelltimer -= p_time;
    }
}

void AIInterface::UpdateMovementSpline()
{
    if (!m_Unit->m_movementManager.CanUpdate(m_Unit->GetMapMgr()->mLoopCounter))
        return;

    if (m_Unit->m_movementManager.m_spline.IsSplineMoveDone())
    {
        setCreatureState(STOPPED);
        return;
    }

    m_Unit->m_movementManager.Update(m_Unit->GetMapMgr()->mLoopCounter);

    ::Movement::Spline::SplinePoint* current = m_Unit->m_movementManager.m_spline.GetCurrentSplinePoint();
    ::Movement::Spline::SplinePoint* prev = m_Unit->m_movementManager.m_spline.GetPreviousSplinePoint();

    G3D::Vector3 newpos;

    float o = atan2(current->pos.x - prev->pos.x, current->pos.y - prev->pos.y);

    uint32 curmstime = Util::getMSTime();

    if (curmstime >= current->arrive)
    {
        newpos = current->pos;
        m_Unit->m_movementManager.m_spline.IncrementCurrentSplineIndex();
        m_Unit->m_movementManager.ForceUpdate();
    }
    else
    {
        newpos = prev->pos - ((prev->pos - current->pos) * float(curmstime - current->setoff) / float(current->arrive - current->setoff));
    }


    m_Unit->SetPosition(newpos.x, newpos.y, newpos.z, o);

    //current spline is finished, attempt to move along next
    if (m_Unit->m_movementManager.IsMovementFinished())
    {
        if (MoveDone())
        {
            // If it is a random wp dont update spline movement immediately
            if (isWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_RANDOMWP))
            {
                OnMoveCompleted();
                return;
            }
            else
            {
                UpdateMovementSpline();
                OnMoveCompleted();
            }
        }
        else
        {
            UpdateMovementSpline();
        }
    }
}

bool AIInterface::MoveDone() const
{
    return m_Unit->m_movementManager.m_spline.IsSplineMoveDone();
}

void AIInterface::AddSpline(float x, float y, float z)
{
    ::Movement::Spline::SplinePoint p;
    p.pos = G3D::Vector3(x, y, z);

    if (m_Unit->m_movementManager.m_spline.GetSplinePoints()->size() == 0)
    {
        //this is first point just insert it, it's always our position for future points
        p.setoff = Util::getMSTime();
        p.arrive = Util::getMSTime(); //now
        m_Unit->m_movementManager.m_spline.AddSplinePoint(p);
        return;
    }

    auto prev = m_Unit->m_movementManager.m_spline.GetLastSplinePoint();

    float dx = x - prev.pos.x;
    float dy = y - prev.pos.y;
    float dz = z - prev.pos.z;
    float dist = sqrt(dx * dx + dy * dy + dz * dz);

    uint32 movetime;

    if (m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.flying)
    {
        movetime = (uint32)(dist / m_flySpeed);
    }
    else if (m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw.walkmode)
    {
        switch (getWalkMode())
        {
            case WALKMODE_SPRINT:
            case WALKMODE_RUN:
                movetime = (uint32)(dist / m_runSpeed);
                break;
            case WALKMODE_WALK:
                movetime = (uint32)(dist / m_walkSpeed);
                break;
            default:
                LOG_ERROR("Added a spline with unhandled spline flag: %X", m_Unit->m_movementManager.m_spline.GetSplineFlags());
                movetime = 1;
                break;
        }

    }
    else
    {
        LOG_ERROR("Added a spline with unhandled spline flag: %X", m_Unit->m_movementManager.m_spline.GetSplineFlags());
        //setting movetime to default value of 1 second. Change if to either a return; or something more meaningful
        //but don't leave movetime uninitialized...
        movetime = 1;
    }

    p.setoff = prev.arrive;
    p.arrive = prev.arrive + movetime;
    m_Unit->m_movementManager.m_spline.m_currentSplineTotalMoveTime += movetime;

    m_Unit->m_movementManager.m_spline.AddSplinePoint(p);
}

bool AIInterface::CreatePath(float x, float y, float z, bool onlytest /*= false*/)
{
    //make sure current spline is updated
    MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh* nav = const_cast<dtNavMesh*>(mmap->GetNavMesh(m_Unit->GetMapId()));
    dtNavMeshQuery* nav_query = const_cast<dtNavMeshQuery*>(mmap->GetNavMeshQuery(m_Unit->GetMapId(), m_Unit->GetInstanceID()));
    //NavMeshData* nav = CollideInterface.GetNavMesh(m_Unit->GetMapId());

    if (nav == nullptr)
        return false;

    float start[VERTEX_SIZE] = { m_Unit->GetPositionY(), m_Unit->GetPositionZ(), m_Unit->GetPositionX() };
    float end[VERTEX_SIZE] = { y, z, x };
    float extents[VERTEX_SIZE] = { 3.0f, 5.0f, 3.0f };
    float closest_point[VERTEX_SIZE] = { 0.0f, 0.0f, 0.0f };

    dtQueryFilter filter;
    filter.setIncludeFlags(NAV_GROUND | NAV_WATER | NAV_SLIME | NAV_MAGMA);

    dtPolyRef startref, endref;

    if (dtStatusFailed(nav_query->findNearestPoly(start, extents, &filter, &startref, closest_point)))
        return false;

    if (dtStatusFailed(nav_query->findNearestPoly(end, extents, &filter, &endref, closest_point)))
        return false;

    if (startref == 0 || endref == 0)
        return false;

    dtPolyRef path[256];
    int pathcount;

    if (dtStatusFailed(nav_query->findPath(startref, endref, start, end, &filter, path, &pathcount, 256)))
        return false;

    if (pathcount == 0 || path[pathcount - 1] != endref)
        return false;

    float points[MAX_PATH_LENGTH * 3];
    int32 pointcount;
    bool usedoffmesh;

    if (dtStatusFailed(findSmoothPath(start, end, path, pathcount, points, &pointcount, usedoffmesh, MAX_PATH_LENGTH, nav, nav_query, filter)))
        return false;

    //add to spline
    if (!onlytest)
    {
        for (int32 i = 0; i < pointcount; ++i)
        {
            AddSpline(points[i * 3 + 2], points[i * 3 + 0], points[i * 3 + 1]);
        }
    }
    return true;
}

dtStatus AIInterface::findSmoothPath(const float* startPos, const float* endPos, const dtPolyRef* polyPath, const uint32 polyPathSize, float* smoothPath, int* smoothPathSize, bool & usedOffmesh, const uint32 maxSmoothPathSize, dtNavMesh* mesh, dtNavMeshQuery* query, dtQueryFilter & filter)
{
    *smoothPathSize = 0;
    uint32 nsmoothPath = 0;
    usedOffmesh = false;

    dtPolyRef polys[MAX_PATH_LENGTH];
    memcpy(polys, polyPath, sizeof(dtPolyRef)*polyPathSize);
    uint32 npolys = polyPathSize;

    float iterPos[VERTEX_SIZE], targetPos[VERTEX_SIZE];
    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[0], startPos, iterPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    if (dtStatusFailed(query->closestPointOnPolyBoundary(polys[npolys - 1], endPos, targetPos)))
        return DT_FAILURE | DT_OUT_OF_MEMORY;

    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
    nsmoothPath++;

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && nsmoothPath < maxSmoothPathSize)
    {
        // Find location to steer towards.
        float steerPos[VERTEX_SIZE];
        unsigned char steerPosFlag;
        dtPolyRef steerPosRef = 0;

        if (!getSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, polys, npolys, steerPos, steerPosFlag, steerPosRef, query))
            break;

        bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
        bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

        // Find movement delta.
        float delta[VERTEX_SIZE];
        dtVsub(delta, steerPos, iterPos);
        float len = dtMathSqrtf(dtVdot(delta, delta));
        // If the steer target is end of path or off-mesh link, do not move past the location.
        if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
            len = 1.0f;
        else
            len = SMOOTH_PATH_STEP_SIZE / len;

        float moveTgt[VERTEX_SIZE];
        dtVmad(moveTgt, iterPos, delta, len);

        // Move
        float result[VERTEX_SIZE];
        const static uint32 MAX_VISIT_POLY = 16;
        dtPolyRef visited[MAX_VISIT_POLY];

        uint32 nvisited = 0;
        query->moveAlongSurface(polys[0], iterPos, moveTgt, &filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
        npolys = fixupCorridor(polys, npolys, MAX_PATH_LENGTH, visited, nvisited);

        query->getPolyHeight(visited[nvisited - 1], result, &result[1]);
        dtVcopy(iterPos, result);

        // Handle end of path and off-mesh links when close enough.
        if (endOfPath && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached end of path.
            dtVcopy(iterPos, targetPos);
            if (nsmoothPath < maxSmoothPathSize)
            {
                dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
                nsmoothPath++;
            }
            break;
        }
        else if (offMeshConnection && inRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 2.0f))
        {
            // Reached off-mesh connection.
            usedOffmesh = true;

            // Advance the path up to and over the off-mesh connection.
            dtPolyRef prevRef = 0;
            dtPolyRef polyRef = polys[0];
            uint32 npos = 0;
            while (npos < npolys && polyRef != steerPosRef)
            {
                prevRef = polyRef;
                polyRef = polys[npos];
                npos++;
            }

            for (uint32 i = npos; i < npolys; ++i)
            {
                polys[i - npos] = polys[i];
            }

            npolys -= npos;

            // Handle the connection.
            float startPos[VERTEX_SIZE], endPos[VERTEX_SIZE];
            if (!dtStatusFailed(mesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos)))
            {
                if (nsmoothPath < maxSmoothPathSize)
                {
                    dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], startPos);
                    nsmoothPath++;
                }
                // Move position at the other side of the off-mesh link.
                dtVcopy(iterPos, endPos);
                query->getPolyHeight(polys[0], iterPos, &iterPos[1]);
            }
        }

        // Store results.
        if (nsmoothPath < maxSmoothPathSize)
        {
            dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
            nsmoothPath++;
        }
    }

    *smoothPathSize = nsmoothPath;

    // this is most likely loop
    return nsmoothPath < maxSmoothPathSize ? DT_SUCCESS : DT_FAILURE;
}

bool AIInterface::getSteerTarget(const float* startPos, const float* endPos, const float minTargetDist, const dtPolyRef* path, const uint32 pathSize, float* steerPos, unsigned char & steerPosFlag, dtPolyRef & steerPosRef, dtNavMeshQuery* query)
{
    // Find steer target.
    static const uint32 MAX_STEER_POINTS = 3;
    float steerPath[MAX_STEER_POINTS * VERTEX_SIZE];
    unsigned char steerPathFlags[MAX_STEER_POINTS];
    dtPolyRef steerPathPolys[MAX_STEER_POINTS];
    uint32 nsteerPath = 0;
    dtStatus dtResult = query->findStraightPath(startPos, endPos, path, pathSize,
                                                steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
    if (!nsteerPath || dtStatusFailed(dtResult))
        return false;

    // Find vertex far enough to steer to.
    uint32 ns = 0;
    while (ns < nsteerPath)
    {
        // Stop at Off-Mesh link or when point is further than slop away.
        if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) || !inRangeYZX(&steerPath[ns * VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
        {
            break;
        }

        ns++;
    }
    // Failed to find good point to steer to.
    if (ns >= nsteerPath)
        return false;

    dtVcopy(steerPos, &steerPath[ns * VERTEX_SIZE]);
    steerPos[1] = startPos[1];  // keep Z value
    steerPosFlag = steerPathFlags[ns];
    steerPosRef = steerPathPolys[ns];

    return true;
}

uint32 AIInterface::fixupCorridor(dtPolyRef* path, const uint32 npath, const uint32 maxPath, const dtPolyRef* visited, const uint32 nvisited)
{
    int32 furthestPath = -1;
    int32 furthestVisited = -1;

    // Find furthest common polygon.
    for (int32 i = npath - 1; i >= 0; --i)
    {
        bool found = false;
        for (int32 j = nvisited - 1; j >= 0; --j)
        {
            if (path[i] == visited[j])
            {
                furthestPath = i;
                furthestVisited = j;
                found = true;
            }
        }

        if (found)
            break;
    }

    // If no intersection found just return current path.
    if (furthestPath == -1 || furthestVisited == -1)
        return npath;

    // Concatenate paths.

    // Adjust beginning of the buffer to include the visited.
    uint32 req = nvisited - furthestVisited;
    uint32 orig = uint32(furthestPath + 1) < npath ? furthestPath + 1 : npath;
    uint32 size = npath - orig > 0 ? npath - orig : 0;
    if (req + size > maxPath)
        size = maxPath - req;

    if (size)
        memmove(path + req, path + orig, size * sizeof(dtPolyRef));

    // Store visited
    for (uint32 i = 0; i < req; ++i)
    {
        path[i] = visited[(nvisited - 1) - i];
    }

    return req + size;
}

void AIInterface::EventEnterCombat(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    if (pUnit == nullptr || pUnit->IsDead() || m_Unit->IsDead())
        return;

    // set the target first
    if (pUnit->GetInstanceID() == m_Unit->GetInstanceID())
    {
        m_Unit->SetTargetGUID(pUnit->GetGUID());
    }

    /* send the message */
    if (m_Unit->IsCreature())
    {
        Creature* creature = static_cast<Creature*>(m_Unit);
        creature->HandleMonsterSayEvent(MONSTER_SAY_EVENT_ENTER_COMBAT);

        CALL_SCRIPT_EVENT(m_Unit, OnCombatStart)(pUnit);

        if (m_Unit->IsCreature())
        {
            // set encounter state = InProgress
            CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(static_cast<Creature*>(m_Unit)->GetEntry(), 1);
        }

        if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
        {
            m_Unit->SetChannelSpellId(0);
            m_Unit->SetChannelSpellTargetGUID(0);
        }
    }

    // Stop the emote - change to fight emote
    m_Unit->SetEmoteState(EMOTE_STATE_READY1H);

    if (misc1 == 0)
    {
        SetReturnPosition();

        m_combatResetX = pUnit->GetPositionX();
        m_combatResetY = pUnit->GetPositionY();
        m_combatResetZ = pUnit->GetPositionZ();
    }


    // dismount if mounted
    if (m_Unit->IsCreature() && !(static_cast<Creature*>(m_Unit)->GetCreatureProperties()->typeFlags & CREATURE_FLAG1_FIGHT_MOUNTED))
        m_Unit->SetMount(0);

    if (!isAiState(AI_STATE_ATTACKING))
        StopMovement(0);

    setAiState(AI_STATE_ATTACKING);
    if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo() && m_Unit->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID)
    {
        if (m_Unit->IsCreature())
        {
            if (static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Rank == 3)
            {
                m_Unit->GetMapMgr()->AddCombatInProgress(m_Unit->GetGUID());
            }
        }
    }

    if (pUnit->IsPlayer() && static_cast< Player* >(pUnit)->InGroup())
    {
        m_Unit->GetAIInterface()->modThreatByPtr(pUnit, 1);
        Group* pGroup = static_cast< Player* >(pUnit)->GetGroup();

        Player* pGroupGuy;
        GroupMembersSet::iterator itr;
        pGroup->Lock();
        for (uint32 i = 0; i < pGroup->GetSubGroupCount(); i++)
        {
            for (itr = pGroup->GetSubGroup(i)->GetGroupMembersBegin(); itr != pGroup->GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
            {
                pGroupGuy = (*itr)->m_loggedInPlayer;
                if (pGroupGuy && pGroupGuy->isAlive() && m_Unit->GetMapMgr() == pGroupGuy->GetMapMgr() && pGroupGuy->getDistanceSq(pUnit) <= 40 * 40) //50 yards for now. lets see if it works
                {
                    m_Unit->GetAIInterface()->AttackReaction(pGroupGuy, 1, 0);
                }
            }
        }
        pGroup->Unlock();
    }
    //Zack : Put mob into combat animation. Take out weapons and start to look serious :P
    m_Unit->smsg_AttackStart(pUnit);
}

void AIInterface::EventLeaveCombat(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    if (pUnit == nullptr)
        return;

    if (pUnit->IsCreature())
    {
        if (pUnit->IsDead())
            pUnit->RemoveAllAuras();
        else
            pUnit->RemoveNegativeAuras();
    }

    Unit* target = nullptr;
    if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo())
    {
        switch (m_Unit->GetMapMgr()->GetMapInfo()->type)
        {
            case INSTANCE_NULL:
            case INSTANCE_BATTLEGROUND:
                if (m_outOfCombatRange && _CalcDistanceFromHome() < m_outOfCombatRange)
                    target = FindTarget();
                break;

            case INSTANCE_RAID:
            case INSTANCE_NONRAID:
            case INSTANCE_MULTIMODE:
                target = FindTarget();
                break;
        }

        if (target != nullptr)
        {
            AttackReaction(target, 1, 0);
            return;
        }
    }

    // restart emote
    if (m_Unit->IsCreature())
    {
        Creature* creature = static_cast<Creature*>(m_Unit);
        creature->HandleMonsterSayEvent(MONSTER_SAY_EVENT_ON_COMBAT_STOP);

        if (creature->original_emotestate)
            m_Unit->SetEmoteState(creature->original_emotestate);
        else
            m_Unit->SetEmoteState(EMOTE_ONESHOT_NONE);

        if (creature->m_spawn && (creature->m_spawn->channel_target_go || creature->m_spawn->channel_target_creature))
        {
            if (creature->m_spawn->channel_target_go)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpGO, creature->m_spawn->channel_target_go, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);

            if (creature->m_spawn->channel_target_creature)
                sEventMgr.AddEvent(creature, &Creature::ChannelLinkUpCreature, creature->m_spawn->channel_target_creature, EVENT_CREATURE_CHANNEL_LINKUP, 1000, 5, 0);
        }

        if (isWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST))
        {
            Movement::WayPoint* waypoint = getWayPoint(getCurrentWayPointId());
            if (waypoint != nullptr)
            {
                m_returnX = waypoint->x;
                m_returnY = waypoint->y;
                m_returnZ = waypoint->z;
            }
        }
    }

    //reset ProcCount
    //ResetProcCounts();
    if (isWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST))
        setSplineWalk();
    else
        setSplineSprint();

    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    m_fleeTimer = 0;
    m_hasFleed = false;
    m_hasCalledForHelp = false;
    m_nextSpell = NULL;
    resetNextTarget();
    m_Unit->CombatStatus.Vanished();

    if (isAiScriptType(AI_SCRIPT_PET))
    {
        setAiState(AI_STATE_FOLLOWING);
        SetUnitToFollow(m_PetOwner);
        FollowDistance = 3.0f;
        m_lastFollowX = m_lastFollowY = 0;
        if (m_Unit->IsPet())
        {
            static_cast< Pet* >(m_Unit)->SetPetAction(PET_ACTION_FOLLOW);
            if (m_Unit->isAlive() && m_Unit->IsInWorld())
            {
                static_cast< Pet* >(m_Unit)->HandleAutoCastEvent(AUTOCAST_EVENT_LEAVE_COMBAT);
            }
        }
        HandleEvent(EVENT_FOLLOWOWNER, 0, 0);
    }
    else
    {
        setAiState(AI_STATE_EVADE);

        Unit* SavedFollow = getUnitToFollow();

        if (m_Unit->isAlive())
        {
            if (SavedFollow == nullptr)
            {
                SetReturnPosition();
                MoveEvadeReturn();
            }
            else
                setAiState(AI_STATE_FOLLOWING);

            Creature* aiowner = static_cast< Creature* >(m_Unit);
            //clear tagger.
            aiowner->UnTag();
            aiowner->setUInt32Value(UNIT_DYNAMIC_FLAGS, aiowner->getUInt32Value(UNIT_DYNAMIC_FLAGS) & ~(U_DYN_FLAG_TAGGED_BY_OTHER | U_DYN_FLAG_LOOTABLE));
        }

        CALL_SCRIPT_EVENT(m_Unit, OnCombatStop)(SavedFollow);
        if (m_Unit->IsCreature())
        {
            // set encounter state back to NotStarted
            CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(static_cast<Creature*>(m_Unit)->GetEntry(), 0);
        }
    }

    if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo() && m_Unit->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID)
    {
        if (m_Unit->IsCreature())
        {
            if (static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Rank == 3)
            {
                m_Unit->GetMapMgr()->RemoveCombatInProgress(m_Unit->GetGUID());
            }
        }
    }

    // Remount if mounted
    if (m_Unit->IsCreature())
    {
        Creature* creature = static_cast< Creature* >(m_Unit);
        if (creature->m_spawn)
            m_Unit->SetMount(creature->m_spawn->MountedDisplayID);
    }
    //Zack : not sure we need to send this. Did not see it in the dumps since mob died eventually but it seems logical to make this
    m_Unit->smsg_AttackStop(pUnit);
}

void AIInterface::EventDamageTaken(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    if (pUnit == nullptr)
        return;

    if (m_Unit->IsCreature())
        static_cast< Creature* >(m_Unit)->HandleMonsterSayEvent(MONSTER_SAY_EVENT_ON_DAMAGE_TAKEN);

    pUnit->RemoveAura(24575);

    CALL_SCRIPT_EVENT(m_Unit, OnDamageTaken)(pUnit, misc1);
    if (!modThreatByPtr(pUnit, misc1))
    {
        m_aiTargets.insert(TargetMap::value_type(pUnit->GetGUID(), misc1));
    }
    pUnit->CombatStatus.OnDamageDealt(m_Unit);
}

void AIInterface::EventFollowOwner(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    setAiState(AI_STATE_FOLLOWING);
    if (m_Unit->IsPet())
        static_cast< Pet* >(m_Unit)->SetPetAction(PET_ACTION_FOLLOW);

    SetUnitToFollow(m_PetOwner);
    m_lastFollowX = m_lastFollowY = 0;
    FollowDistance = 4.0f;

    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    m_fleeTimer = 0;
    m_hasFleed = false;
    m_hasCalledForHelp = false;
    m_nextSpell = nullptr;
    resetNextTarget();
    setSplineRun();
}

void AIInterface::EventFear(Unit* pUnit, uint32 misc1)
{
    if (pUnit == nullptr)
        return;

    m_FearTimer = 0;
    SetUnitToFear(pUnit);

    CALL_SCRIPT_EVENT(m_Unit, OnFear)(pUnit, 0);
    setAiState(AI_STATE_FEAR);
    StopMovement(1);

    m_UnitToFollow_backup = m_UnitToFollow;
    m_UnitToFollow = 0;
    m_lastFollowX = m_lastFollowY = 0;
    FollowDistance_backup = FollowDistance;
    FollowDistance = 0.0f;

    LockAITargets(true);
    m_aiTargets.clear(); // we'll get a new target after we are unfeared
    LockAITargets(false);
    m_fleeTimer = 0;
    m_hasFleed = false;
    m_hasCalledForHelp = false;

    // update speed
    setSplineRun();
    SetNextSpell(nullptr);
    resetNextTarget();
}

void AIInterface::EventUnfear(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    m_UnitToFollow = m_UnitToFollow_backup;
    FollowDistance = FollowDistance_backup;
    setAiState(AI_STATE_UNFEARED); // let future reactions put us back into combat without bugging return positions

    m_UnitToFear = 0;
    StopMovement(1);
}

void AIInterface::EventWander(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    if (pUnit == nullptr)
        return;

    m_WanderTimer = 0;

    //CALL_SCRIPT_EVENT(m_Unit, OnWander)(pUnit, 0); FIX ME
    setAiState(AI_STATE_WANDER);
    StopMovement(1);

    m_UnitToFollow_backup = m_UnitToFollow;
    m_UnitToFollow = 0;
    m_lastFollowX = m_lastFollowY = 0;
    FollowDistance_backup = FollowDistance;
    FollowDistance = 0.0f;

    LockAITargets(true);
    m_aiTargets.clear(); // we'll get a new target after we are unwandered
    LockAITargets(false);
    m_fleeTimer = 0;
    m_hasFleed = false;
    m_hasCalledForHelp = false;

    // update speed
    setSplineRun();

    SetNextSpell(nullptr);
    resetNextTarget();
}

void AIInterface::EventUnwander(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_EVADE))
        return;

    m_UnitToFollow = m_UnitToFollow_backup;
    FollowDistance = FollowDistance_backup;
    setAiState(AI_STATE_IDLE); // we need this to prevent permanent fear, wander, and other problems

    StopMovement(1);
}

void AIInterface::EventUnitDied(Unit* pUnit, uint32 misc1)
{
    if (pUnit == nullptr)
        return;

    if (m_Unit->IsCreature())
        static_cast< Creature* >(m_Unit)->HandleMonsterSayEvent(MONSTER_SAY_EVENT_ON_DIED);

    CALL_SCRIPT_EVENT(m_Unit, OnDied)(pUnit);
    if (m_Unit->IsCreature())
    {
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), OnCreatureDeath)(static_cast<Creature*>(m_Unit), pUnit);

        // set encounter state to finished
        CALL_INSTANCE_SCRIPT_EVENT(m_Unit->GetMapMgr(), setData)(static_cast<Creature*>(m_Unit)->GetEntry(), 2);    //2 = Finished
    }

    setAiState(AI_STATE_IDLE);

    StopMovement(0);
    LockAITargets(true);
    m_aiTargets.clear();
    LockAITargets(false);
    m_UnitToFollow = 0;
    m_lastFollowX = m_lastFollowY = 0;
    m_UnitToFear = 0;
    FollowDistance = 0.0f;
    m_fleeTimer = 0;
    m_hasFleed = false;
    m_hasCalledForHelp = false;
    m_nextSpell = nullptr;

    resetNextTarget();

    m_Unit->SetMount(0);

    mCurrentWaypoint = 0;

    Instance* pInstance = nullptr;
    if (m_Unit->GetMapMgr())
        pInstance = m_Unit->GetMapMgr()->pInstance;

    if (m_Unit->GetMapMgr()
        && m_Unit->IsCreature()
        && !m_Unit->IsPet()
        && pInstance
        && (pInstance->m_mapInfo->type == INSTANCE_RAID
        || pInstance->m_mapInfo->type == INSTANCE_NONRAID
        || pInstance->m_mapInfo->type == INSTANCE_MULTIMODE))
    {
        InstanceBossInfoMap* bossInfoMap = objmgr.m_InstanceBossInfoMap[m_Unit->GetMapMgr()->GetMapId()];
        Creature* pCreature = static_cast< Creature* >(m_Unit);
        bool found = false;

        if (IS_PERSISTENT_INSTANCE(pInstance) && bossInfoMap != NULL)
        {
            uint32 npcGuid = pCreature->GetCreatureProperties()->Id;
            InstanceBossInfoMap::const_iterator bossInfo = bossInfoMap->find(npcGuid);
            if (bossInfo != bossInfoMap->end())
            {
                found = true;
                m_Unit->GetMapMgr()->pInstance->m_killedNpcs.insert(npcGuid);
                m_Unit->GetMapMgr()->pInstance->SaveToDB();
                for (InstanceBossTrashList::iterator trash = bossInfo->second->trash.begin(); trash != bossInfo->second->trash.end(); ++trash)
                {
                    Creature* c = m_Unit->GetMapMgr()->GetSqlIdCreature((*trash));
                    if (c != nullptr)
                        c->m_noRespawn = true;
                }
                if (!pInstance->m_persistent)
                {
                    pInstance->m_persistent = true;
                    pInstance->SaveToDB();
                    for (PlayerStorageMap::iterator itr = m_Unit->GetMapMgr()->m_PlayerStorage.begin(); itr != m_Unit->GetMapMgr()->m_PlayerStorage.end(); ++itr)
                    {
                        (*itr).second->SetPersistentInstanceId(pInstance);
                    }
                }
            }
        }

        if (found == false)
        {
            // No instance boss information ... so fallback ...
            uint32 npcGuid = pCreature->GetSQL_id();
            m_Unit->GetMapMgr()->pInstance->m_killedNpcs.insert(npcGuid);
            m_Unit->GetMapMgr()->pInstance->SaveToDB();
        }
    }
    if (m_Unit->GetMapMgr() && m_Unit->GetMapMgr()->GetMapInfo() && m_Unit->GetMapMgr()->GetMapInfo()->type == INSTANCE_RAID)
    {
        if (m_Unit->IsCreature())
        {
            if (static_cast< Creature* >(m_Unit)->GetCreatureProperties()->Rank == 3)
            {
                m_Unit->GetMapMgr()->RemoveCombatInProgress(m_Unit->GetGUID());
            }
        }
    }
}

void AIInterface::EventHostileAction(Unit* pUnit, uint32 misc1)
{
    m_combatResetX = m_Unit->GetPositionX();
    m_combatResetY = m_Unit->GetPositionY();
    m_combatResetZ = m_Unit->GetPositionZ();
}

void AIInterface::OnMoveCompleted()
{
    auto splineFlags = m_Unit->m_movementManager.m_spline.GetSplineFlags()->m_splineFlagsRaw;

    //remove flags that are temporary
    splineFlags.done = false;
    splineFlags.trajectory = false;
#if VERSION_STRING != Cata
    splineFlags.knockback = false;
#else
    splineFlags.falling = false;
#endif

    //reset spline priority so other movements can happen
    mSplinePriority = SPLINE_PRIORITY_MOVEMENT;

    //we've been knocked somewhere without entering combat, move back
    if (isAiState(AI_STATE_IDLE) && m_returnX != 0.0f && m_returnY != 0.0f && m_returnZ != 0.0f)
    {
        setAiState(AI_STATE_EVADE);
        MoveEvadeReturn();
    }
}

void AIInterface::MoveEvadeReturn()
{
    generateAndSendSplinePath(m_returnX, m_returnY, m_returnZ);
}

void AIInterface::EventForceRedirected(Unit* pUnit, uint32 misc1)
{
    if (isAiState(AI_STATE_IDLE))
        SetReturnPosition();
}

void AIInterface::SetReturnPosition()
{
    if (m_returnX != 0.0f && m_returnY != 0.0f && m_returnZ != 0.0f)  //already returning somewhere
        return;

    if (isWaypointScriptType(Movement::WP_MOVEMENT_SCRIPT_QUEST))
    {
        Movement::WayPoint* waypoint = getWayPoint(getCurrentWayPointId());
        if (waypoint != nullptr)
        {
            m_returnX = waypoint->x;
            m_returnY = waypoint->y;
            m_returnZ = waypoint->z;
        }
    }
    else
    {
        m_returnX = m_Unit->GetSpawnX();
        m_returnY = m_Unit->GetSpawnY();
        m_returnZ = m_Unit->GetSpawnZ();
    }
}

void AIInterface::SetCreatureProtoDifficulty(uint32 entry)
{
    if (GetDifficultyType() != 0)
    {
        uint32 creature_difficulty_entry = sMySQLStore.getCreatureDifficulty(entry, GetDifficultyType());
        auto properties_difficulty = sMySQLStore.getCreatureProperties(creature_difficulty_entry);
        Creature* creature = static_cast<Creature*>(m_Unit);
        if (properties_difficulty != nullptr)
        {
            if (!properties_difficulty->isTrainingDummy && !m_Unit->IsVehicle())
            {
                m_Unit->GetAIInterface()->SetAllowedToEnterCombat(true);
            }
            else
            {
                m_Unit->GetAIInterface()->SetAllowedToEnterCombat(false);
                m_Unit->GetAIInterface()->setAiScriptType(AI_SCRIPT_PASSIVE);
            }

            m_walkSpeed = m_Unit->m_basicSpeedWalk = properties_difficulty->walk_speed;
            m_runSpeed = m_Unit->m_basicSpeedRun = properties_difficulty->run_speed;
            m_flySpeed = properties_difficulty->fly_speed;

            m_Unit->SetScale(properties_difficulty->Scale);

            uint32 health = properties_difficulty->MinHealth + RandomUInt(properties_difficulty->MaxHealth - properties_difficulty->MinHealth);

            m_Unit->SetHealth(health);
            m_Unit->SetMaxHealth(health);
            m_Unit->SetBaseHealth(health);

            m_Unit->SetMaxPower(POWER_TYPE_MANA, properties_difficulty->Mana);
            m_Unit->SetBaseMana(properties_difficulty->Mana);
            m_Unit->SetPower(POWER_TYPE_MANA, properties_difficulty->Mana);

            m_Unit->setLevel(properties_difficulty->MinLevel + (RandomUInt(properties_difficulty->MaxLevel - properties_difficulty->MinLevel)));

            for (uint8 i = 0; i < 7; ++i)
            {
                m_Unit->SetResistance(i, properties_difficulty->Resistances[i]);
            }

            m_Unit->SetBaseAttackTime(MELEE, properties_difficulty->AttackTime);

            m_Unit->SetMinDamage(properties_difficulty->MinDamage);
            m_Unit->SetMaxDamage(properties_difficulty->MaxDamage);

            m_Unit->SetBaseAttackTime(RANGED, properties_difficulty->RangedAttackTime);
            m_Unit->SetMinRangedDamage(properties_difficulty->RangedMinDamage);
            m_Unit->SetMaxRangedDamage(properties_difficulty->RangedMaxDamage);


            m_Unit->SetFaction(properties_difficulty->Faction);

            if (!(m_Unit->m_factionDBC->RepListId == -1 && m_Unit->m_faction->HostileMask == 0 && m_Unit->m_faction->FriendlyMask == 0))
            {
                m_Unit->GetAIInterface()->m_canCallForHelp = true;
            }

            if (properties_difficulty->CanRanged == 1)
                m_Unit->GetAIInterface()->m_canRangedAttack = true;
            else
                m_Unit->m_aiInterface->m_canRangedAttack = false;

            m_Unit->SetBoundingRadius(properties_difficulty->BoundingRadius);

            m_Unit->SetCombatReach(properties_difficulty->CombatReach);

            m_Unit->setUInt32Value(UNIT_NPC_FLAGS, properties_difficulty->NPCFLags);

            // resistances
            for (uint8 j = 0; j < 7; ++j)
            {
                m_Unit->BaseResistance[j] = m_Unit->GetResistance(j);
            }

            for (uint8 j = 0; j < 5; ++j)
            {
                m_Unit->BaseStats[j] = m_Unit->GetStat(j);
            }

            m_Unit->BaseDamage[0] = m_Unit->GetMinDamage();
            m_Unit->BaseDamage[1] = m_Unit->GetMaxDamage();
            m_Unit->BaseOffhandDamage[0] = m_Unit->GetMinOffhandDamage();
            m_Unit->BaseOffhandDamage[1] = m_Unit->GetMaxOffhandDamage();
            m_Unit->BaseRangedDamage[0] = m_Unit->GetMinRangedDamage();
            m_Unit->BaseRangedDamage[1] = m_Unit->GetMaxRangedDamage();

            creature->BaseAttackType = properties_difficulty->attackSchool;

            //guard
            if (properties_difficulty->guardtype == GUARDTYPE_CITY)
                m_Unit->m_aiInterface->m_isGuard = true;
            else
                m_Unit->m_aiInterface->m_isGuard = false;

            if (properties_difficulty->guardtype == GUARDTYPE_NEUTRAL)
                m_Unit->m_aiInterface->m_isNeutralGuard = true;
            else
                m_Unit->m_aiInterface->m_isNeutralGuard = false;

            m_Unit->m_aiInterface->UpdateSpeeds(); // use speed from creature_proto_difficulty.

            //invisibility
            m_Unit->m_invisFlag = static_cast<uint8>(properties_difficulty->invisibility_type);
            if (m_Unit->m_invisFlag > 0)
                m_Unit->m_invisible = true;
            else
                m_Unit->m_invisible = false;

            if (m_Unit->IsVehicle())
            {
                m_Unit->AddVehicleComponent(properties_difficulty->Id, properties_difficulty->vehicleid);
                m_Unit->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                m_Unit->setAItoUse(false);
            }

            if (properties_difficulty->rooted)
                m_Unit->setMoveRoot(true);
        }
    }
}

uint8 AIInterface::GetDifficultyType()
{
    uint8 difficulty_type;

    Instance* instance = sInstanceMgr.GetInstanceByIds(NUM_MAPS, m_Unit->GetInstanceID());
    if (instance != nullptr)
        difficulty_type = instance->m_difficulty;
    else
        difficulty_type = 0;    // standard MODE_NORMAL / MODE_NORMAL_10MEN

    return difficulty_type;
}
