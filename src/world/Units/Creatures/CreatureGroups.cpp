/*
Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureGroups.h"
#include "Creature.h"
#include "AIInterface.h"
#include "Map/MapMgr.h"

#include "Movement/MovementManager.h"
#include "Objects/ObjectMgr.h"

#define MAX_DESYNC 5.0f

FormationMgr::FormationMgr()
{
}

FormationMgr::~FormationMgr()
{
}

FormationMgr* FormationMgr::getInstance()
{
    static FormationMgr mInstance;
    return &mInstance;
}

void FormationMgr::addCreatureToGroup(uint32_t leaderSpawnId, Creature* creature)
{
    MapMgr* map = creature->GetMapMgr();

    auto itr = map->CreatureGroupHolder.find(leaderSpawnId);
    if (itr != map->CreatureGroupHolder.end())
    {
        //Add member to an existing group
        sLogger.debug("FormationMgr : Group found: %u, inserting creature %u, Group InstanceID %u", leaderSpawnId, creature->getGuid(), creature->GetInstanceID());

        // With dynamic spawn the creature may have just respawned
        // we need to find previous instance of creature and delete it from the formation, as it'll be invalidated
        for (auto pair : map->_sqlids_creatures)
        {
            if (pair.first == creature->getSpawnId())
            {
                Creature* other = pair.second;
                if (other == creature)
                    continue;

                if (itr->second->hasMember(other))
                    itr->second->removeMember(other);
            }
        }
    }
    else
    {
        //Create new group
        sLogger.debug("FormationMgr : Group not found: %u. Creating new group.", leaderSpawnId);
        CreatureGroup* group = new CreatureGroup(leaderSpawnId);
        std::tie(itr, std::ignore) = map->CreatureGroupHolder.emplace(leaderSpawnId, group);
    }

    itr->second->addMember(creature);
}

void FormationMgr::removeCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    sLogger.debug("FormationMgr : Deleting member pointer to GUID: %u from group %u", group->getLeaderSpawnId(), member->getSpawnId());
    group->removeMember(member);

    if (group->isEmpty())
    {
        MapMgr* map = member->GetMapMgr();

        sLogger.debug("FormationMgr : Deleting group with InstanceID %u", member->GetInstanceID());
        auto itr = map->CreatureGroupHolder.find(group->getLeaderSpawnId());
        ASSERT(itr != map->CreatureGroupHolder.end() && "Not registered group in map");
        map->CreatureGroupHolder.erase(itr);
        delete group;
    }
}

void FormationMgr::loadCreatureFormations()
{
    auto oldMSTime = Util::TimeNow();

    //Get group data
    QueryResult* result = WorldDatabase.Query("SELECT leaderGUID, memberGUID, dist, angle, groupAI, point_1, point_2 FROM creature_formations ORDER BY leaderGUID");
    if (!result)
    {
        sLogger.debug("FormationMgr : Loaded 0 creatures in formations. DB table `creature_formations` is empty!");
        return;
    }

    uint32_t count = 0;
    std::unordered_set<uint32_t> leaderSpawnIds;
    do
    {
        Field* fields = result->Fetch();

        //Load group member data
        FormationInfo member;
        member.LeaderSpawnId              = fields[0].GetUInt32();
        uint32_t memberSpawnId = fields[1].GetUInt32();
        member.FollowDist                 = 0.f;
        member.FollowAngle                = 0.f;

        //If creature is group leader we may skip loading of dist/angle
        if (member.LeaderSpawnId != memberSpawnId)
        {
            member.FollowDist             = fields[2].GetFloat();
            member.FollowAngle            = fields[3].GetFloat() * float(M_PI) / 180.0f;
        }

        member.GroupAI                    = fields[4].GetUInt32();
        for (uint8_t i = 0; i < 2; ++i)
            member.LeaderWaypointIDs[i]   = fields[5 + i].GetUInt16();

        // check data correctness
        {
            QueryResult* spawnResult = nullptr;
            spawnResult = WorldDatabase.Query("SELECT * FROM creature_spawns WHERE id = %u", member.LeaderSpawnId);
            if (spawnResult == nullptr)
            {
                sLogger.failure("FormationMgr : creature_formations table leader guid %u incorrect (not exist)", member.LeaderSpawnId);
                continue;
            }

            spawnResult = nullptr;
            spawnResult = WorldDatabase.Query("SELECT * FROM creature_spawns WHERE id = %u", memberSpawnId);
            if (spawnResult == nullptr)
            {
                sLogger.failure("FormationMgr : creature_formations table member guid %u incorrect (not exist)", memberSpawnId);
                continue;
            }

            leaderSpawnIds.insert(member.LeaderSpawnId);
        }

        _creatureGroupMap.emplace(memberSpawnId, std::move(member));
        ++count;
    } while (result->NextRow());

    for (uint32_t leaderSpawnId : leaderSpawnIds)
    {
        if (!_creatureGroupMap.count(leaderSpawnId))
        {
            sLogger.failure("FormationMgr : creature_formation contains leader spawn %u which is not included on its formation, removing", leaderSpawnId);
            for (auto itr = _creatureGroupMap.begin(); itr != _creatureGroupMap.end();)
            {
                if (itr->second.LeaderSpawnId == leaderSpawnId)
                {
                    itr = _creatureGroupMap.erase(itr);
                    continue;
                }

                ++itr;
            }
        }
    }

    sLogger.debug("FormationMgr : Loaded %u creatures in formations in %u ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

FormationInfo* FormationMgr::getFormationInfo(uint32_t spawnId)
{
    auto itr = _creatureGroupMap.find(spawnId);
    if (itr != _creatureGroupMap.end())
        return &itr->second;
    return nullptr;
}

void FormationMgr::addFormationMember(uint32_t spawnId, float followAng, float followDist, uint32_t leaderSpawnId, uint32_t groupAI)
{
    FormationInfo member;
    member.LeaderSpawnId = leaderSpawnId;
    member.FollowDist    = followDist;
    member.FollowAngle   = followAng;
    member.GroupAI       = groupAI;
    for (uint8_t i = 0; i < 2; ++i)
        member.LeaderWaypointIDs[i] = 0;

    _creatureGroupMap.emplace(spawnId, std::move(member));
}

CreatureGroup::CreatureGroup(uint32_t leaderSpawnId) : _leader(nullptr), _members(), _leaderSpawnId(leaderSpawnId), _formed(false), _engaging(false)
{
}

CreatureGroup::~CreatureGroup()
{
}

void CreatureGroup::addMember(Creature* member)
{
    sLogger.debug("FormationMgr : CreatureGroup::AddMember: Adding unit %s.", member->getGuid());

    //Check if it is a leader
    if (member->getSpawnId() == _leaderSpawnId)
    {
        sLogger.debug("FormationMgr : Unit %u is formation leader. Adding group.", member->getGuid());
        _leader = member;
    }

    // formation must be registered at this point
    FormationInfo* formationInfo = sFormationMgr->getFormationInfo(member->getSpawnId());
    _members.emplace(member, formationInfo);
    member->setFormation(this);
}

void CreatureGroup::removeMember(Creature* member)
{
    if (_leader == member)
        _leader = nullptr;

    _members.erase(member);
    member->setFormation(nullptr);
}

void CreatureGroup::memberEngagingTarget(Creature* member, Unit* target)
{
    // used to prevent recursive calls
    if (_engaging)
        return;

    const auto groupAI = sFormationMgr->getFormationInfo(member->getSpawnId())->GroupAI;
    if (!groupAI)
        return;

    if (member == _leader)
    {
        if (!(groupAI & FLAG_MEMBERS_ASSIST_LEADER))
            return;
    }
    else if (!(groupAI & FLAG_LEADER_ASSISTS_MEMBER))
        return;

    _engaging = true;

    for (auto const& pair : _members)
    {
        Creature* other = pair.first;
        if (other == member)
            continue;

        if (!other->isAlive())
            continue;

        if (((other != _leader && (groupAI & FLAG_MEMBERS_ASSIST_LEADER)) || (other == _leader && (groupAI & FLAG_LEADER_ASSISTS_MEMBER))))
            other->GetAIInterface()->onHostileAction(target);
    }

    _engaging = false;
}

void CreatureGroup::formationReset(bool dismiss)
{
    for (auto const& pair : _members)
    {
        if (pair.first != _leader && pair.first->isAlive())
        {
            if (dismiss)
                pair.first->getMovementManager()->initialize();
            else
                pair.first->getMovementManager()->moveIdle();
            sLogger.debug("FormationMgr : CreatureGroup::FormationReset: Set %s movement for member %s", dismiss ? "default" : "idle", pair.first->getGuid());
        }
    }

    _formed = !dismiss;
}

void CreatureGroup::leaderStartedMoving()
{
    if (!_leader)
        return;

    for (auto const& pair : _members)
    {
        Creature* member = pair.first;
        if (member == _leader || !member->isAlive() || member->GetAIInterface()->isEngaged() || !(pair.second->GroupAI & FLAG_IDLE_IN_FORMATION))
            continue;

        float angle = pair.second->FollowAngle + float(M_PI); // for some reason, someone thought it was a great idea to invert relativ angles...
        float dist = pair.second->FollowDist;

        if (!member->hasUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION))
            member->getMovementManager()->moveFormation(_leader, dist, angle, pair.second->LeaderWaypointIDs[0], pair.second->LeaderWaypointIDs[1]);
    }
}

bool CreatureGroup::canLeaderStartMoving() const
{
    for (std::unordered_map<Creature*, FormationInfo*>::value_type const& pair : _members)
    {
        if (pair.first != _leader && pair.first->isAlive())
        {
            if (pair.first->GetAIInterface()->isEngaged() || pair.first->isReturningHome())
                return false;
        }
    }

    return true;
}
