/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <unordered_map>
#include <cstdint>
#include "CommonTypes.hpp"

enum GroupAIFlags
{
    FLAG_AGGRO_NONE            = 0,                                                         // No creature group behavior
    FLAG_MEMBERS_ASSIST_LEADER = 0x00000001,                                                // The member aggroes if the leader aggroes
    FLAG_LEADER_ASSISTS_MEMBER = 0x00000002,                                                // The leader aggroes if the member aggroes
    FLAG_MEMBERS_ASSIST_MEMBER = (FLAG_MEMBERS_ASSIST_LEADER | FLAG_LEADER_ASSISTS_MEMBER), // every member will assist if any member is attacked
    FLAG_IDLE_IN_FORMATION     = 0x00000200,                                                // The member will follow the leader when pathing idly
};

class Creature;
class CreatureGroup;
class Unit;
struct Position;

struct FormationInfo
{
    uint32_t LeaderSpawnId;
    float FollowDist;
    float FollowAngle;
    uint32_t GroupAI;
    uint32_t LeaderWaypointIDs[2];
};

class SERVER_DECL FormationMgr
{
private:
    FormationMgr() = default;
    ~FormationMgr() = default;

    std::unordered_map<uint32_t /*spawnID*/, FormationInfo> _creatureGroupMap;

public:
    static FormationMgr* getInstance();

    void addCreatureToGroup(uint32_t leaderSpawnId, Creature* creature);
    void removeCreatureFromGroup(CreatureGroup* group, Creature* creature);

    void loadCreatureFormations();
    FormationInfo* getFormationInfo(uint32_t spawnId);

    void addFormationMember(uint32_t spawnId, float followAng, float followDist, uint32_t leaderSpawnId, uint32_t groupAI);
};

class SERVER_DECL CreatureGroup
{
private:
    Creature* _leader; // Important do not forget sometimes to work with pointers instead synonims :D:D
    std::unordered_map<Creature*, FormationInfo*> _members;

    uint32_t _leaderSpawnId;
    bool _formed;
    bool _engaging;

public:
    // Group cannot be created empty
    explicit CreatureGroup(uint32_t leaderSpawnId);
    ~CreatureGroup() = default;

    Creature* getLeader() const { return _leader; }
    uint32_t getLeaderSpawnId() const { return _leaderSpawnId; }
    bool isEmpty() const { return _members.empty(); }
    bool isFormed() const { return _formed; }
    bool isLeader(Creature const* creature) const { return _leader == creature; }

    bool hasMember(Creature* member) const { return _members.count(member) > 0; }
    void addMember(Creature* member);
    void removeMember(Creature* member);
    void formationReset(bool dismiss);

    void leaderStartedMoving();
    void memberEngagingTarget(Creature* member, Unit* target);
    bool canLeaderStartMoving() const;
};

#define sFormationMgr FormationMgr::getInstance()
