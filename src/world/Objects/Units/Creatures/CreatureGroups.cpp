/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "CreatureGroups.h"
#include "Creature.h"
#include "AIInterface.h"
#include "Logging/Logger.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Movement/MovementManager.h"
#include "Server/DatabaseDefinition.hpp"

#define MAX_DESYNC 5.0f

FormationMgr* FormationMgr::getInstance()
{
    static FormationMgr mInstance;
    return &mInstance;
}

void FormationMgr::addCreatureToGroup(uint32_t leaderSpawnId, Creature* creature)
{
    WorldMap* map = creature->getWorldMap();

    const auto [itr, inserted] = map->CreatureGroupHolder.try_emplace(leaderSpawnId, Util::LazyInstanceCreator([leaderSpawnId] {
        return std::make_unique<CreatureGroup>(leaderSpawnId);
    }));

    if (!inserted)
    {
        //Add member to an existing group
        sLogger.debug("FormationMgr : Group found: {}, inserting creature {}, Group InstanceID {}", leaderSpawnId, creature->getGuid(), creature->GetInstanceID());

        // With dynamic spawn the creature may have just respawned
        // we need to find previous instance of creature and delete it from the formation, as it'll be invalidated
        std::vector<Creature*> creatures;
        map->getRegistry().snapshotCreatures(creatures);

        for (Creature* other : creatures)
        {
            if (!other || other == creature)
                continue;

            if (other->getSpawnId() != creature->getSpawnId())
                continue;

            if (itr->second->hasMember(other))
                itr->second->removeMember(other);
        }
    }
    else
    {
        //Create new group
        sLogger.debug("FormationMgr : Group not found: {}. Creating new group.", leaderSpawnId);
    }

    itr->second->addMember(creature);
}

void FormationMgr::removeCreatureFromGroup(CreatureGroup* group, Creature* member)
{
    sLogger.debug("FormationMgr : Deleting member pointer to GUID: {} from group {}", group->getLeaderSpawnId(), member->getSpawnId());
    group->removeMember(member);

    if (group->isEmpty())
    {
        WorldMap* map = member->getWorldMap();

        sLogger.debug("FormationMgr : Deleting group with InstanceID {}", member->GetInstanceID());
        auto itr = map->CreatureGroupHolder.find(group->getLeaderSpawnId());
        ASSERT(itr != map->CreatureGroupHolder.end() && "Not registered group in map");
        map->CreatureGroupHolder.erase(itr);
    }
}

void FormationMgr::loadCreatureFormations()
{
    auto oldMSTime = Util::TimeNow();

    //Get group data
    auto result = WorldDatabase.query("SELECT leaderGUID, memberGUID, dist, angle, groupAI, point_1, point_2 FROM creature_formations ORDER BY leaderGUID");
    if (!result)
    {
        sLogger.debug("FormationMgr : Loaded 0 creatures in formations. DB table `creature_formations` is empty!");
        return;
    }

    uint32_t count = 0;
    std::unordered_set<uint32_t> leaderSpawnIds;
    do
    {
        Field* fields = result->fetch();

        //Load group member data
        FormationInfo member;
        member.LeaderSpawnId              = fields[0].asUint32();
        uint32_t memberSpawnId            = fields[1].asUint32();
        member.FollowDist                 = 0.f;
        member.FollowAngle                = 0.f;

        //If creature is group leader we may skip loading of dist/angle
        if (member.LeaderSpawnId != memberSpawnId)
        {
            member.FollowDist             = fields[2].asFloat();
            member.FollowAngle            = fields[3].asFloat() * AscEmu::Math::PiF / 180.0f;
        }

        member.GroupAI                    = fields[4].asUint32();
        for (uint8_t i = 0; i < 2; ++i)
            member.LeaderWaypointIDs[i]   = fields[5 + i].asUint16();

        // check data correctness
        {
            auto spawnResult = WorldDatabase.query("SELECT * FROM creature_spawns WHERE id = %u", member.LeaderSpawnId);
            if (spawnResult == nullptr)
            {
                sLogger.failure("FormationMgr : creature_formations table leader guid {} incorrect (not exist)", member.LeaderSpawnId);
                continue;
            }

            spawnResult = nullptr;
            spawnResult = WorldDatabase.query("SELECT * FROM creature_spawns WHERE id = %u", memberSpawnId);
            if (spawnResult == nullptr)
            {
                sLogger.failure("FormationMgr : creature_formations table member guid {} incorrect (not exist)", memberSpawnId);
                continue;
            }

            leaderSpawnIds.insert(member.LeaderSpawnId);
        }

        _creatureGroupMap.emplace(memberSpawnId, std::move(member));
        ++count;
    } while (result->nextRow());

    for (uint32_t leaderSpawnId : leaderSpawnIds)
    {
        if (!_creatureGroupMap.count(leaderSpawnId))
        {
            sLogger.failure("FormationMgr : creature_formation contains leader spawn {} which is not included on its formation, removing", leaderSpawnId);
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

    sLogger.info("FormationMgr : Loaded {} creatures in formations in {} ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
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

void CreatureGroup::addMember(Creature* member)
{
    sLogger.debug("FormationMgr : CreatureGroup::AddMember: Adding unit {}.", member->getGuid());

    //Check if it is a leader
    if (member->getSpawnId() == _leaderSpawnId)
    {
        sLogger.debug("FormationMgr : Unit {} is formation leader. Adding group.", member->getGuid());
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
        if (!other || other == member || !other->isAlive())
            continue;

        AIInterface* ai = other->getAIInterface();
        if (!ai)
            continue;

        if (((other != _leader && (groupAI & FLAG_MEMBERS_ASSIST_LEADER)) || (other == _leader && (groupAI & FLAG_LEADER_ASSISTS_MEMBER))))
            ai->onHostileAction(target);
    }

    _engaging = false;
}

void CreatureGroup::formationReset(bool dismiss)
{
    for (auto const& pair : _members)
    {
        Creature* member = pair.first;
        if (!member || member == _leader || !member->isAlive())
            continue;

        if (dismiss)
            member->getMovementManager()->initialize();
        else
            member->getMovementManager()->moveIdle();

        sLogger.debug("FormationMgr : CreatureGroup::FormationReset: Set {} movement for member {}", dismiss ? "default" : "idle", member->getGuid());
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
        FormationInfo* info = pair.second;
        if (!member || !info || member == _leader || !member->isAlive())
            continue;

        AIInterface* ai = member->getAIInterface();
        if (!ai || ai->isEngaged() || !(info->GroupAI & FLAG_IDLE_IN_FORMATION))
            continue;

        float angle = info->FollowAngle + AscEmu::Math::PiF; // for some reason, someone thought it was a great idea to invert relativ angles...
        float dist = info->FollowDist;

        if (!member->hasUnitStateFlag(UNIT_STATE_FOLLOW_FORMATION))
            member->getMovementManager()->moveFormation(_leader, dist, angle, info->LeaderWaypointIDs[0], info->LeaderWaypointIDs[1]);
    }
}

bool CreatureGroup::canLeaderStartMoving() const
{
    for (const auto& pair : _members)
    {
        Creature* creature = pair.first;

        if (!creature)
            continue;

        if (creature == _leader)
            continue;

        if (!creature->isAlive())
            continue;

        AIInterface* ai = creature->getAIInterface();
        if (!ai)
            continue;

        if (ai->isEngaged() || creature->isReturningHome())
            return false;
    }

    return true;
}
