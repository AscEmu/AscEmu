/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Management/LFG/LFGMgr.h"
#include "Server/MainServerDefines.h"
#include "Map/MapMgr.h"
#include "Spell/SpellAuras.h"
#include "Map/WorldCreator.h"
#include "Objects/ObjectMgr.h"
#include "Units/Creatures/Pet.h"
#include "Server/Packets/SmsgPartyCommandResult.h"

using namespace AscEmu::Packets;

Group::Group(bool Assign)
{
    m_GroupType = GROUP_TYPE_PARTY;	 // Always init as party

    // Create initial subgroup
    memset(m_SubGroups, 0, sizeof(SubGroup*) * 8);
    m_SubGroups[0] = new SubGroup(this, 0);

    memset(m_instanceIds, 0, sizeof(uint32) * MAX_NUM_MAPS * NUM_INSTANCE_MODES);

    m_Leader = NULL;
    m_Looter = NULL;
    m_LootMethod = PARTY_LOOT_GROUP;
    m_LootThreshold = 2;
    m_SubGroupCount = 1;
    m_MemberCount = 0;

    if (Assign)
    {
        m_Id = objmgr.GenerateGroupId();
        ObjectMgr::getSingleton().AddGroup(this);
        m_guid = MAKE_NEW_GUID(m_Id, 0, HIGHGUID_TYPE_GROUP);
    }

    m_dirty = false;
    m_updateblock = false;
    m_disbandOnNoMembers = true;
    memset(m_targetIcons, 0, sizeof(uint64) * 8);
    m_isqueued = false;
    m_difficulty = 0;
    m_raiddifficulty = 0;
    m_assistantLeader = m_mainAssist = m_mainTank = NULL;
    updatecounter = 0;
    m_Id = 0;
    m_guid = 0;
}

Group::~Group()
{
    for (uint32 j = 0; j < m_SubGroupCount; ++j)
    {
        SubGroup* sub = GetSubGroup(j);
        if (sub)
            delete sub;
    }

    ObjectMgr::getSingleton().RemoveGroup(this);
}

SubGroup::~SubGroup()
{

}

void SubGroup::RemovePlayer(PlayerInfo* info)
{
    m_GroupMembers.erase(info);
    info->subGroup = -1;
}

bool SubGroup::AddPlayer(PlayerInfo* info)
{
    if (IsFull())
        return false;

    info->subGroup = (int8)GetID();
    m_GroupMembers.insert(info);
    return true;
}

bool SubGroup::HasMember(uint32 guid)
{
    for (GroupMembersSet::iterator itr = m_GroupMembers.begin(); itr != m_GroupMembers.end(); ++itr)
        if ((*itr) != NULL)
            if ((*itr)->guid == guid)
                return true;

    return false;
}

GroupMembersSet& SubGroup::getGroupMembers()
{
    return m_GroupMembers;
}

SubGroup* Group::FindFreeSubGroup()
{
    for (uint32 i = 0; i < m_SubGroupCount; i++)
        if (!m_SubGroups[i]->IsFull())
            return m_SubGroups[i];

    return NULL;
}

bool Group::AddMember(PlayerInfo* info, int32 subgroupid/* =-1 */)
{
    m_groupLock.Acquire();
    Player* pPlayer = info->m_loggedInPlayer;

    if (m_isqueued)
    {
        m_isqueued = false;
        BattlegroundManager.RemoveGroupFromQueues(this);
    }

    if (!IsFull())
    {
        SubGroup* subgroup = (subgroupid > 0) ? m_SubGroups[subgroupid] : FindFreeSubGroup();
        if (subgroup == NULL)
        {
            m_groupLock.Release();
            return false;
        }

        if (subgroup->AddPlayer(info))
        {
            if (pPlayer != NULL)
                sEventMgr.AddEvent(pPlayer, &Player::EventGroupFullUpdate, EVENT_PLAYER_UPDATE, 1500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

            if (info->m_Group && info->m_Group != this)
                info->m_Group->RemovePlayer(info);

            if (m_Leader == NULL && info->m_loggedInPlayer)
                m_Leader = info;

            info->m_Group = this;
            info->subGroup = (int8)subgroup->GetID();

            ++m_MemberCount;
            m_dirty = true;
            Update();	// Send group update

            m_groupLock.Release();
            return true;
        }
        else
        {
            m_groupLock.Release();
            info->m_Group = NULL;
            info->subGroup = -1;
            return false;
        }

    }
    else
    {
        info->m_Group = NULL;
        info->subGroup = -1;
        m_groupLock.Release();
        return false;
    }
}

//\TODO bool silent is not used - remove it!
void Group::SetLeader(Player* pPlayer, bool silent)
{
    if (pPlayer != nullptr)
    {
        m_Leader = pPlayer->getPlayerInfo();
        m_dirty = true;

        if (silent == false)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, pPlayer->getName().size() + 1);
            data << pPlayer->getName().c_str();
            SendPacketToAll(&data);
        }
    }
    Update();
}

void Group::Update()
{
    if (m_updateblock)
        return;

    Player* pNewLeader = NULL;

    if (m_Leader == NULL || (m_Leader != NULL && m_Leader->m_loggedInPlayer == NULL))
    {
        pNewLeader = FindFirstPlayer();
        if (pNewLeader != NULL)
            m_Leader = pNewLeader->getPlayerInfo();
    }

    if (m_Looter != NULL && m_Looter->m_loggedInPlayer == NULL)
    {
        if (pNewLeader == NULL)
            pNewLeader = FindFirstPlayer();
        if (pNewLeader != NULL)
            m_Looter = pNewLeader->getPlayerInfo();
    }

    GroupMembersSet::iterator itr1, itr2;

    uint8 i = 0, j = 0;
    uint8 flags;
    SubGroup* sg1 = NULL;
    SubGroup* sg2 = NULL;
    m_groupLock.Acquire();

    for (i = 0; i < m_SubGroupCount; i++)
    {
        sg1 = m_SubGroups[i];

        if (sg1 != NULL)
        {
            for (itr1 = sg1->GetGroupMembersBegin(); itr1 != sg1->GetGroupMembersEnd(); ++itr1)
            {
                // should never happen but just in case
                if ((*itr1) == NULL)
                    continue;

                /* skip offline players */
                if ((*itr1)->m_loggedInPlayer == NULL)
                {
                    continue;
                }

                WorldPacket data(SMSG_GROUP_LIST, (50 + (m_MemberCount * 20)));
                data << uint8(m_GroupType);
                data << uint8((*itr1)->subGroup);

                flags = 0;
                if ((*itr1) == m_assistantLeader)
                    flags |= 1;
                if ((*itr1) == m_mainTank)
                    flags |= 2;
                if ((*itr1) == m_mainAssist)
                    flags |= 4;
                data << uint8(flags);

                if (m_Leader != NULL && m_Leader->m_loggedInPlayer != NULL && m_Leader->m_loggedInPlayer->IsInBg())
                    data << uint8(1);   //if the leader is in a BG, then the group is a BG group
                else
                    data << uint8(0);

                if (m_GroupType & GROUP_TYPE_LFD)
                {
                    data << uint8(sLfgMgr.GetState(GetID()) == LFG_STATE_FINISHED_DUNGEON ? 2 : 0);
					data << uint32(sLfgMgr.GetDungeon(GetID()));
#if VERSION_STRING >= Cata
                    data << uint8(0);   //unk
#endif
                }

                data << uint64(GetID());            // Group guid
                data << uint32(updatecounter++);    // 3.3 - increments every time a group list update is being sent to client
                data << uint32(m_MemberCount - 1);	// we don't include self

                for (j = 0; j < m_SubGroupCount; j++)
                {
                    sg2 = m_SubGroups[j];

                    if (sg2 != NULL)
                    {
                        for (itr2 = sg2->GetGroupMembersBegin(); itr2 != sg2->GetGroupMembersEnd(); ++itr2)
                        {
                            if ((*itr1) == (*itr2))   // skip self
                                continue;

                            // should never happen but just in case
                            if ((*itr2) == NULL)
                                continue;

                            Player* plr = (*itr2)->m_loggedInPlayer;
                            data << (plr ? plr->getName().c_str() : (*itr2)->name);
							if(plr)
								data << plr->getGuid();
							else
                                data << (*itr2)->guid << uint32(0);	// highguid

                            if ((*itr2)->m_loggedInPlayer != NULL)
                                data << uint8(1);
                            else
                                data << uint8(0);

                            data << uint8((*itr2)->subGroup);

                            flags = 0;

                            if ((*itr2) == m_assistantLeader)
                                flags |= 1;
                            if ((*itr2) == m_mainTank)
                                flags |= 2;
                            if ((*itr2) == m_mainAssist)
                                flags |= 4;

                            data << uint8(flags);
                            data << uint8(plr ? plr->GetRoles() : 0);   // Player roles
                        }
                    }
                }

                if (m_Leader != NULL)
                    data << m_Leader->guid << uint32(0);
                else
                    data << uint64(0);

                data << uint8(m_LootMethod);

                if (m_Looter != NULL)
                    data << m_Looter->guid << uint32(0);
                else
                    data << uint64(0);

                data << uint8(m_LootThreshold);
                data << uint8(m_difficulty);
                data << uint8(m_raiddifficulty);
                data << uint8(0);   // 3.3 - unk

                if (!(*itr1)->m_loggedInPlayer->IsInWorld())
                    (*itr1)->m_loggedInPlayer->CopyAndSendDelayedPacket(&data);
                else
                    (*itr1)->m_loggedInPlayer->GetSession()->SendPacket(&data);
            }
        }
    }

    if (m_dirty)
    {
        m_dirty = false;
        SaveToDB();
    }

    m_groupLock.Release();
}

void Group::Disband()
{
    m_groupLock.Acquire();
    m_updateblock = true;

    if (m_isqueued)
    {
        m_isqueued = false;
        WorldPacket* data = sChatHandler.FillSystemMessageData("A change was made to your group. Removing the arena queue.");
        SendPacketToAll(data);
        delete data;

        BattlegroundManager.RemoveGroupFromQueues(this);
    }

    uint8 i = 0;
    for (i = 0; i < m_SubGroupCount; i++)
    {
        SubGroup* sg = m_SubGroups[i];
        sg->Disband();
    }

    m_groupLock.Release();
    CharacterDatabase.Execute("DELETE FROM `groups` WHERE `group_id` = %u", m_Id);
    sInstanceMgr.OnGroupDestruction(this);
    delete this;	// destroy ourselves, the destructor removes from eventmgr and objectmgr.
}

void SubGroup::Disband()
{
    WorldPacket data(SMSG_GROUP_DESTROYED, 1);
    WorldPacket data2(SMSG_PARTY_COMMAND_RESULT, 12);
    data2 << uint32(2);
    data2 << uint8(0);
    data2 << uint32(m_Parent->m_difficulty);	// you leave the group

    for (GroupMembersSet::iterator itr = m_GroupMembers.begin(); itr != m_GroupMembers.end();)
    {
        if ((*itr) != nullptr)
        {
            if ((*itr)->m_loggedInPlayer)
            {
                if ((*itr)->m_loggedInPlayer->GetSession() != nullptr)
                {
                    data2.put(5, uint32((*itr)->m_loggedInPlayer->iInstanceType));
                    (*itr)->m_loggedInPlayer->GetSession()->SendPacket(&data2);
                    (*itr)->m_loggedInPlayer->GetSession()->SendPacket(&data);
#if VERSION_STRING >= Cata
                    (*itr)->m_loggedInPlayer->GetSession()->sendEmptyGroupList((*itr)->m_loggedInPlayer);
#else
                    (*itr)->m_Group->SendNullUpdate((*itr)->m_loggedInPlayer);   // cebernic: panel refresh.
#endif
                }
            }

            (*itr)->m_Group = nullptr;
            (*itr)->subGroup = -1;
        }

        --m_Parent->m_MemberCount;
        itr = m_GroupMembers.erase(itr);
    }

    m_Parent->m_SubGroups[m_Id] = nullptr;
    delete this;
}

Player* Group::FindFirstPlayer()
{
    GroupMembersSet::iterator itr;
    m_groupLock.Acquire();

    for (uint8 i = 0; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i] != NULL)
        {
            for (itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd(); ++itr)
            {
                if ((*itr) != NULL)
                {
                    if ((*itr)->m_loggedInPlayer != NULL)
                    {
                        m_groupLock.Release();
                        return (*itr)->m_loggedInPlayer;
                    }
                }
            }
        }
    }

    m_groupLock.Release();
    return NULL;
}

void Group::RemovePlayer(PlayerInfo* info)
{
    if (info == nullptr)
        return;

    WorldPacket data(50);
    Player* pPlayer = info->m_loggedInPlayer;

    m_groupLock.Acquire();
    if (m_isqueued)
    {
        m_isqueued = false;
        BattlegroundManager.RemoveGroupFromQueues(this);
    }

    SubGroup* sg = NULL;
    if (info->subGroup >= 0 && info->subGroup < 8)
        sg = m_SubGroups[info->subGroup];

    if (sg == NULL || sg->m_GroupMembers.find(info) == sg->m_GroupMembers.end())
    {
        for (uint8 i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] != NULL)
            {
                if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
                {
                    sg = m_SubGroups[i];
                    break;
                }
            }
        }
    }

    info->m_Group = NULL;
    info->subGroup = -1;

    if (sg == NULL)
    {
        m_groupLock.Release();
        return;
    }

    m_dirty = true;
    sg->RemovePlayer(info);
    --m_MemberCount;

    // remove team member from the instance
    if (info->m_loggedInPlayer != NULL)
    {
        sInstanceMgr.PlayerLeftGroup(this, info->m_loggedInPlayer);
    }

    if (pPlayer != NULL)
    {
        if (pPlayer->GetSession() != NULL)
        {
#if VERSION_STRING < Cata
            SendNullUpdate(pPlayer);
#endif

            data.SetOpcode(SMSG_GROUP_DESTROYED);
            pPlayer->GetSession()->SendPacket(&data);

#if VERSION_STRING >= Cata
            pPlayer->GetSession()->SendPacket(SmsgPartyCommandResult(2, pPlayer->getName().c_str(), ERR_PARTY_NO_ERROR).serialise().get());
            pPlayer->GetSession()->sendEmptyGroupList(pPlayer);
#else
            data.Initialize(SMSG_PARTY_COMMAND_RESULT);
            data << uint32(2);
            data << uint8(0);
            data << uint32(0);  // you leave the group
            pPlayer->GetSession()->SendPacket(&data);
#endif
        }

        //Remove some party auras.
        for (uint32 i = MAX_POSITIVE_AURAS_EXTEDED_START; i < MAX_POSITIVE_AURAS_EXTEDED_END; i++)
        {
            if (pPlayer->m_auras[i] && pPlayer->m_auras[i]->m_areaAura)
            {
                Object* caster = pPlayer->m_auras[i]->GetCaster();
                if ((caster != NULL) && (pPlayer->getGuid() != caster->getGuid()))
                    pPlayer->m_auras[i]->Remove();
            }
        }
    }

    if (m_MemberCount < 2)
    {
        if (m_disbandOnNoMembers)
        {
            m_groupLock.Release();
            Disband();
            return;
        }
    }

    /* eek! ;P */
    Player* newPlayer = NULL;
    if (m_Looter == info)
    {
        newPlayer = FindFirstPlayer();
        if (newPlayer != NULL)
            m_Looter = newPlayer->getPlayerInfo();
        else
            m_Looter = NULL;
    }

    if (m_Leader == info)
    {
        if (newPlayer == NULL)
            newPlayer = FindFirstPlayer();

        if (newPlayer != NULL)
            SetLeader(newPlayer, false);
        else
            m_Leader = NULL;
    }

    Update();
    m_groupLock.Release();
}

void Group::ExpandToRaid()
{
    if (m_isqueued)
    {
        m_isqueued = false;
        WorldPacket* data = sChatHandler.FillSystemMessageData("A change was made to your group. Removing the arena queue.");
        SendPacketToAll(data);
        delete data;

        BattlegroundManager.RemoveGroupFromQueues(this);
    }

    // Very simple ;)
    m_groupLock.Acquire();
    m_SubGroupCount = 8;

    for (uint8 i = 1; i < m_SubGroupCount; ++i)
        m_SubGroups[i] = new SubGroup(this, i);

    m_GroupType = GROUP_TYPE_RAID;
    m_dirty = true;
    Update();
    m_groupLock.Release();
}

void Group::SetLooter(Player* pPlayer, uint8 method, uint16 threshold)
{
    if (pPlayer != NULL)
    {
        m_LootMethod = method;
        m_Looter = pPlayer->getPlayerInfo();
        m_LootThreshold = threshold;
        m_dirty = true;
    }
    Update();
}

void Group::SendPacketToAllButOne(WorldPacket* packet, Player* pSkipTarget)
{
    GroupMembersSet::iterator itr;
    uint8 i = 0;
    m_groupLock.Acquire();
    for (; i < m_SubGroupCount; i++)
    {
        for (itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd(); ++itr)
        {
            if ((*itr)->m_loggedInPlayer != NULL && (*itr)->m_loggedInPlayer != pSkipTarget && (*itr)->m_loggedInPlayer->GetSession())
                (*itr)->m_loggedInPlayer->GetSession()->SendPacket(packet);
        }
    }

    m_groupLock.Release();
}

void Group::OutPacketToAllButOne(uint16 op, uint16 len, const void* data, Player* pSkipTarget)
{
    GroupMembersSet::iterator itr;
    uint8 i = 0;
    m_groupLock.Acquire();
    for (; i < m_SubGroupCount; i++)
    {
        for (itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd(); ++itr)
        {
            if ((*itr)->m_loggedInPlayer != NULL && (*itr)->m_loggedInPlayer != pSkipTarget)
                (*itr)->m_loggedInPlayer->GetSession()->OutPacket(op, len, data);
        }
    }

    m_groupLock.Release();
}

bool Group::HasMember(Player* pPlayer)
{
    if (!pPlayer)
        return false;

    GroupMembersSet::iterator itr;
    m_groupLock.Acquire();

    for (uint8 i = 0; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i] != NULL)
        {
            if (m_SubGroups[i]->m_GroupMembers.find(pPlayer->getPlayerInfo()) != m_SubGroups[i]->m_GroupMembers.end())
            {
                m_groupLock.Release();
                return true;
            }
        }
    }

    m_groupLock.Release();
    return false;
}

bool Group::HasMember(PlayerInfo* info)
{
    GroupMembersSet::iterator itr;
    uint8 i = 0;

    m_groupLock.Acquire();

    for (; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
        {
            m_groupLock.Release();
            return true;
        }
    }

    m_groupLock.Release();
    return false;
}

void Group::MovePlayer(PlayerInfo* info, uint8 subgroup)
{
    if (subgroup >= m_SubGroupCount)
        return;

    if (m_SubGroups[subgroup]->IsFull())
        return;

    m_groupLock.Acquire();
    SubGroup* sg = NULL;

    if (info->subGroup > 0 && info->subGroup < 8)
        sg = m_SubGroups[info->subGroup];

    if (sg == NULL || sg->m_GroupMembers.find(info) == sg->m_GroupMembers.end())
    {
        for (uint8 i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] != NULL)
            {
                if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
                {
                    sg = m_SubGroups[i];
                    break;
                }
            }
        }
    }

    if (sg == NULL)
    {
        m_groupLock.Release();
        return;
    }

    sg->RemovePlayer(info);

    // Grab the new group, and insert
    sg = m_SubGroups[subgroup];
    if (!sg->AddPlayer(info))
    {
        RemovePlayer(info);
    }
    else
    {
        info->subGroup = (int8)sg->GetID();
    }

    Update();
    m_groupLock.Release();
}

void Group::SendNullUpdate(Player* pPlayer)
{
    // this packet is 28 bytes long.		// AS OF 3.3
    uint8 buffer[28];
    memset(buffer, 0, 28);
    pPlayer->GetSession()->OutPacket(SMSG_GROUP_LIST, 28, buffer);
}

void Group::LoadFromDB(Field* fields)
{
#define LOAD_ASSISTANT(__i, __d) g = fields[__i].GetUInt32(); if (g != 0) { __d = objmgr.GetPlayerInfo(g); }

    m_groupLock.Acquire();

    uint32 g;
    m_updateblock = true;

    m_Id = fields[0].GetUInt32();

    ObjectMgr::getSingleton().AddGroup(this);

    m_GroupType = fields[1].GetUInt8();
    m_SubGroupCount = fields[2].GetUInt8();
    m_LootMethod = fields[3].GetUInt8();
    m_LootThreshold = fields[4].GetUInt8();
    m_difficulty = fields[5].GetUInt8();
    m_raiddifficulty = fields[6].GetUInt8();

    LOAD_ASSISTANT(7, m_assistantLeader);
    LOAD_ASSISTANT(8, m_mainTank);
    LOAD_ASSISTANT(9, m_mainAssist);

    // create groups
    for (uint8 i = 1; i < m_SubGroupCount; ++i)
        m_SubGroups[i] = new SubGroup(this, i);

    // assign players into groups
    for (uint8 i = 0; i < m_SubGroupCount; ++i)
    {
        for (uint8 j = 0; j < 5; ++j)
        {
            uint32 guid = fields[10 + (i * 5) + j].GetUInt32();
            if (guid == 0)
                continue;

            PlayerInfo* inf = objmgr.GetPlayerInfo(guid);
            if (inf == NULL)
                continue;

            AddMember(inf);
        }
    }

    char* ids = strdup(fields[50].GetString());
    char* q = ids;
    char* p = strchr(q, ' ');
    while (p)
    {
        char* r = strchr(q, ':');
        if (r == NULL || r > p)
            continue;
        *p = 0;
        *r = 0;
        char* s = strchr(r + 1, ':');
        if (s == NULL || s > p)
            continue;
        *s = 0;
        uint32 mapId = atoi(q);
        uint32 mode = atoi(r + 1);
        uint32 instanceId = atoi(s + 1);

        if (mapId >= MAX_NUM_MAPS)
            continue;

        m_instanceIds[mapId][mode] = instanceId;

        q = p + 1;
        p = strchr(q, ' ');
    }
    free(ids);

    m_updateblock = false;

    m_groupLock.Release();
}

void Group::SaveToDB()
{
    if (!m_disbandOnNoMembers)	/* don't save bg groups */
        return;

    std::stringstream ss;
    //uint32 i = 0;
    //uint32 fillers = 8 - m_SubGroupCount;


    ss << "DELETE FROM `groups` WHERE `group_id` = ";
    ss << m_Id;
    ss << ";";

    CharacterDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO groups (group_id, group_type, subgroup_count, loot_method, loot_threshold, difficulty, raiddifficulty, assistant_leader, main_tank, main_assist,\
          group1member1, group1member2, group1member3, group1member4, group1member5,\
          group2member1, group2member2, group2member3, group2member4, group2member5,\
          group3member1, group3member2, group3member3, group3member4, group3member5,\
          group4member1, group4member2, group4member3, group4member4, group4member5,\
          group5member1, group5member2, group5member3, group5member4, group5member5,\
          group6member1, group6member2, group6member3, group6member4, group6member5,\
          group7member1, group7member2, group7member3, group7member4, group7member5,\
          group8member1, group8member2, group8member3, group8member4, group8member5,\
                    timestamp, instanceids) VALUES("
        << m_Id << "," // group_id (1/52)
        << uint32(m_GroupType) << "," // group_type (2/52)
        << uint32(m_SubGroupCount) << "," // subgroup_count (3/52)
        << uint32(m_LootMethod) << "," // loot_method (4/52)
        << uint32(m_LootThreshold) << "," // loot_threshold (5/52)
        << uint32(m_difficulty) << "," // difficulty (6/52)
        << uint32(m_raiddifficulty) << ","; // raiddifficulty (7/52)

    // assistant_leader (8/52)
    if (m_assistantLeader)
        ss << m_assistantLeader->guid << ",";
    else
        ss << "0,";

    // main_tank (9/52)
    if (m_mainTank)
        ss << m_mainTank->guid << ",";
    else
        ss << "0,";

    // main_assist (10/52)
    if (m_mainAssist)
        ss << m_mainAssist->guid << ",";
    else
        ss << "0,";

    auto membersNotFilled = 40;

    // For each subgroup
    for (uint8 i = 0; i < m_SubGroupCount; ++i)
    {
        uint8 j = 0;

        // For each member in the group, while membercount is less than 5 (guard clause), add their ID to query
        for (GroupMembersSet::iterator itr = m_SubGroups[i]->GetGroupMembersBegin(); j < 5 && itr != m_SubGroups[i]->GetGroupMembersEnd(); ++j, ++itr)
        {
            ss << (*itr)->guid << ",";
        }

        // Add 0 to query until we reach 5 (fill empty slots)
        for (; j < 5; ++j)
            ss << "0,";

        membersNotFilled -= 5;
    }

    // Fill remaining empty slots
    for (membersNotFilled; membersNotFilled > 0; --membersNotFilled)
        ss << "0,";

    //for (uint32 i = 0; i < fillers; ++i)
    //    ss << "0,0,0,0,0,";

    // timestamp (51/52)
    ss << (uint32)UNIXTIME << ",'";

    // instanceids (52/52)
    for (uint32 i = 0; i < MAX_NUM_MAPS; i++)
    {
        for (uint32 j = 0; j < NUM_INSTANCE_MODES; j++)
        {
            if (m_instanceIds[i][j] > 0)
            {
                ss << i << ":" << j << ":" << m_instanceIds[i][j] << " ";
            }
        }
    }
    ss << "')";
    /*printf("==%s==\n", ss.str().c_str());*/
    CharacterDatabase.Execute(ss.str().c_str());
}

void Group::UpdateOutOfRangePlayer(Player* pPlayer, bool Distribute, WorldPacket* Packet)
{
    if (pPlayer == nullptr)
        return;

    uint32 mask = pPlayer->GetGroupUpdateFlags();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)                // if update power type, update current/max power also
        mask |= (GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER);

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)            // same for pets
        mask |= (GROUP_UPDATE_FLAG_PET_CUR_POWER | GROUP_UPDATE_FLAG_PET_MAX_POWER);
    WorldPacket* data = Packet;
    if (!Packet)
        data = new WorldPacket(SMSG_PARTY_MEMBER_STATS, 500);
    if (pPlayer->m_isGmInvisible)
        mask = GROUP_UPDATE_FLAG_STATUS;
    uint32 byteCount = 0;
    for (uint8 i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];
    data->Initialize(SMSG_PARTY_MEMBER_STATS, 8 + 4 + byteCount);
    *data << pPlayer->GetNewGUID();
    *data << mask;

    if (mask & GROUP_UPDATE_FLAG_STATUS)
    {
        if (pPlayer && !pPlayer->m_isGmInvisible)
            *data << uint16(pPlayer->GetGroupStatus());
        else
            *data << uint16(MEMBER_STATUS_OFFLINE);
    }

    if (mask & GROUP_UPDATE_FLAG_CUR_HP)
        *data << uint32(pPlayer->getHealth());

    if (mask & GROUP_UPDATE_FLAG_MAX_HP)
        *data << uint32(pPlayer->getMaxHealth());

    uint8 powerType = pPlayer->getPowerType();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)
        *data << uint8(powerType);

    if (mask & GROUP_UPDATE_FLAG_CUR_POWER)
        *data << uint16(pPlayer->getPower(powerType));

    if (mask & GROUP_UPDATE_FLAG_MAX_POWER)
        *data << uint16(pPlayer->getMaxPower(powerType));

    if (mask & GROUP_UPDATE_FLAG_LEVEL)
        *data << uint16(pPlayer->getLevel());

    if (mask & GROUP_UPDATE_FLAG_ZONE)
        *data << uint16(pPlayer->GetZoneId());

    if (mask & GROUP_UPDATE_FLAG_POSITION)
    {
        *data << uint16(pPlayer->GetPositionX());
        *data << uint16(pPlayer->GetPositionY());
    }

    if (mask & GROUP_UPDATE_FLAG_AURAS)
    {
        uint64 auramask = pPlayer->GetAuraUpdateMaskForRaid();
        *data << uint64(auramask);
        for (uint32 i = 0; i < 64; ++i)
        {
            if (auramask & (uint64(1) << i))
            {
                Aura * aurApp = pPlayer->GetAuraWithSlot(i);
                *data << uint32(aurApp ? aurApp->GetSpellId() : 0);
                *data << uint8(1);
            }
        }
    }

    Pet* pet = pPlayer->GetSummon();
    if (mask & GROUP_UPDATE_FLAG_PET_GUID)
    {
        if (pet)
            *data << (uint64)pet->getGuid();
        else
            *data << (uint64)0;
    }

    if (mask & GROUP_UPDATE_FLAG_PET_NAME)
    {
        if (pet)
            *data << pet->GetName().c_str();
        else
            *data << uint8(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MODEL_ID)
    {
        if (pet)
            *data << uint16(pet->getDisplayId());
        else
            *data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_HP)
    {
        if (pet)
            *data << uint32(pet->getHealth());
        else
            *data << uint32(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_HP)
    {
        if (pet)
            *data << uint32(pet->getMaxHealth());
        else
            *data << uint32(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)
    {
        if (pet)
            *data << uint8(pet->getPowerType());
        else
            *data << uint8(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_CUR_POWER)
    {
        if (pet)
            *data << uint16(pet->getPower(pet->getPowerType()));
        else
            *data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_PET_MAX_POWER)
    {
        if (pet)
            *data << uint16(pet->getMaxPower(pet->getPowerType()));
        else
            *data << uint16(0);
    }

    if (mask & GROUP_UPDATE_FLAG_VEHICLE_SEAT)
    {
#if VERSION_STRING < Cata
#ifdef FT_VEHICLES
        if (Vehicle* veh = pPlayer->getCurrentVehicle())
            *data << uint32(veh->GetVehicleInfo()->seatID[pPlayer->getMovementInfo()->transport_seat]);
#endif
#endif
    }

    if (mask & GROUP_UPDATE_FLAG_PET_AURAS)
    {
        if (pet)
        {
            uint64 auramask = pet->GetAuraUpdateMaskForRaid();
            *data << uint64(auramask);
            for (uint32 i = 0; i < 64; ++i)
            {
                if (auramask & (uint64(1) << i))
                {
                    Aura * aurApp = pet->GetAuraWithSlot(i);
                    *data << uint32(aurApp ? aurApp->GetSpellId() : 0);
                    *data << uint8(1);
                }
            }
        }
        else
            *data << uint64(0);
    }
    if (Distribute && pPlayer->IsInWorld())
    {
        Player* plr;
        float dist = pPlayer->GetMapMgr()->m_UpdateDistance;
        m_groupLock.Acquire();
        for (uint8 i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] == NULL)
                continue;

            for (GroupMembersSet::iterator itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd();)
            {
                plr = (*itr)->m_loggedInPlayer;
                ++itr;

                if (plr && plr != pPlayer)
                {
                    if (plr->GetDistance2dSq(pPlayer) > dist)
                        plr->GetSession()->SendPacket(data);
                }
            }
        }
        m_groupLock.Release();
    }

    if (!Packet)
        delete data;
}

void Group::UpdateAllOutOfRangePlayersFor(Player* pPlayer)
{
    WorldPacket data(150);
    WorldPacket data2(150);

    if (m_SubGroupCount > 8)
        return;

    // tell the other players about us
    UpdateOutOfRangePlayer(pPlayer, true, &data2);

    // tell us any other players we don't know about
    Player* plr;
    bool u1, u2;
    UpdateMask myMask;
    myMask.SetCount(PLAYER_END);
    UpdateMask hisMask;
    hisMask.SetCount(PLAYER_END);

    m_groupLock.Acquire();
    for (uint8 i = 0; i < m_SubGroupCount; ++i)
    {
        if (m_SubGroups[i] == NULL)
            continue;

        for (GroupMembersSet::iterator itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd(); ++itr)
        {
            plr = (*itr)->m_loggedInPlayer;
            if (!plr || plr == pPlayer) continue;

            if (!plr->IsVisible(pPlayer->getGuid()))
            {
                UpdateOutOfRangePlayer(plr, false, &data);
                pPlayer->GetSession()->SendPacket(&data);
            }
            else
            {
                if (pPlayer->GetSubGroup() == plr->GetSubGroup())
                {
                    // distribute quest fields to other players
                    hisMask.Clear();
                    myMask.Clear();
                    u1 = u2 = false;
#if VERSION_STRING == TBC
                    for (uint16 j = PLAYER_QUEST_LOG_1_1; j <= PLAYER_QUEST_LOG_25_1; ++j)
#elif VERSION_STRING == Classic
                    for (uint16 j = PLAYER_QUEST_LOG_1_1; j <= PLAYER_QUEST_LOG_15_4; ++j)
#else
                    for (uint16 j = PLAYER_QUEST_LOG_1_1; j <= PLAYER_QUEST_LOG_25_5; ++j)
#endif
                    {
                        if (plr->getUInt32Value(j))
                        {
                            hisMask.SetBit(j);
                            u1 = true;
                        }

                        if (pPlayer->getUInt32Value(j))
                        {
                            u2 = true;
                            myMask.SetBit(j);
                        }
                    }

                    if (u1)
                    {
                        data.clear();
                        plr->BuildValuesUpdateBlockForPlayer(&data, &hisMask);
                        pPlayer->getUpdateMgr().pushUpdateData(&data, 1);
                    }

                    if (u2)
                    {
                        data.clear();
                        pPlayer->BuildValuesUpdateBlockForPlayer(&data, &myMask);
                        plr->getUpdateMgr().pushUpdateData(&data, 1);
                    }
                }
            }
        }
    }

    m_groupLock.Release();
}

bool Group::isRaid() const
{
    return getGroupType() == GROUP_TYPE_RAID;
}

Group* Group::Create()
{
    return new Group(true);
}

void Group::SetMainAssist(PlayerInfo* pMember)
{
    if (m_mainAssist == pMember)
        return;

    m_mainAssist = pMember;
    m_dirty = true;
    Update();
}

void Group::SetMainTank(PlayerInfo* pMember)
{
    if (m_mainTank == pMember)
        return;

    m_mainTank = pMember;
    m_dirty = true;
    Update();
}

void Group::SetAssistantLeader(PlayerInfo* pMember)
{
    if (m_assistantLeader == pMember)
        return;

    m_assistantLeader = pMember;
    m_dirty = true;
    Update();
}

void Group::SetDungeonDifficulty(uint8 diff)
{
    m_difficulty = diff;

    Lock();
    for (uint8 i = 0; i < GetSubGroupCount(); ++i)
    {
        for (GroupMembersSet::iterator itr = GetSubGroup(i)->GetGroupMembersBegin(); itr != GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
        {
            if ((*itr)->m_loggedInPlayer)
            {
                (*itr)->m_loggedInPlayer->SetDungeonDifficulty(diff);
                (*itr)->m_loggedInPlayer->sendDungeonDifficultyPacket();
            }
        }
    }
    Unlock();
}

void Group::SetRaidDifficulty(uint8 diff)
{
    m_raiddifficulty = diff;

    Lock();

    for (uint8 i = 0; i < GetSubGroupCount(); ++i)
    {
        for (GroupMembersSet::iterator itr = GetSubGroup(i)->GetGroupMembersBegin(); itr != GetSubGroup(i)->GetGroupMembersEnd(); ++itr)
        {
            if ((*itr)->m_loggedInPlayer)
            {
                (*itr)->m_loggedInPlayer->SetRaidDifficulty(diff);
                (*itr)->m_loggedInPlayer->sendRaidDifficultyPacket();
            }
        }
    }

    Unlock();
}

void Group::SendLootUpdates(Object* o)
{
    if (o->isCreatureOrPlayer())
    {
        // Build the actual update.
        ByteBuffer buf(500);

        uint32 Flags = static_cast<Unit*>(o)->getDynamicFlags();

        Flags |= U_DYN_FLAG_LOOTABLE;
        Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

        o->BuildFieldUpdatePacket(&buf, UNIT_DYNAMIC_FLAGS, Flags);

        Lock();

        switch (m_LootMethod)
        {
            case PARTY_LOOT_RR:
            case PARTY_LOOT_FFA:
            case PARTY_LOOT_GROUP:
            case PARTY_LOOT_NBG:
            {

                SubGroup* sGrp = NULL;
                GroupMembersSet::iterator itr2;

                for (uint32 Index = 0; Index < GetSubGroupCount(); ++Index)
                {
                    sGrp = GetSubGroup(Index);
                    itr2 = sGrp->GetGroupMembersBegin();

                    for (; itr2 != sGrp->GetGroupMembersEnd(); ++itr2)
                    {
                        PlayerInfo* p = *itr2;

                        if (p->m_loggedInPlayer != NULL && p->m_loggedInPlayer->IsVisible(o->getGuid()))       // Save updates for non-existent creatures
                            p->m_loggedInPlayer->getUpdateMgr().pushUpdateData(&buf, 1);
                    }
                }

                break;
            }

            case PARTY_LOOT_MASTER:
            {
                Player* pLooter = GetLooter() ? GetLooter()->m_loggedInPlayer : NULL;
                if (pLooter == NULL)
                    pLooter = GetLeader()->m_loggedInPlayer;

                if (pLooter->IsVisible(o->getGuid()))
                {
                    Unit* victim = static_cast<Unit*>(o);

                    victim->Tag(pLooter->getGuid());
                    pLooter->getUpdateMgr().pushUpdateData(&buf, 1);
                }

                break;
            }
        }

        Unlock();
    }
}

Player* Group::GetRandomPlayerInRangeButSkip(Player* plr, float range, Player* plr_skip)
{
    std::vector<Player*> players;

    // We must iterate through all subgroups
    for (uint8 i = 0; i < 8; ++i)
    {
        SubGroup* s_grp = GetSubGroup(i);

        if (s_grp == NULL)
            continue;

        for (GroupMembersSet::iterator itr = s_grp->GetGroupMembersBegin(); itr != s_grp->GetGroupMembersEnd(); ++itr)
        {
            // Skip NULLs and not alive players
            if (!((*itr)->m_loggedInPlayer != NULL && (*itr)->m_loggedInPlayer->isAlive()))
                continue;

            // Skip desired player
            if ((*itr)->m_loggedInPlayer == plr_skip)
                continue;

            // Skip player not in range
            if (!(*itr)->m_loggedInPlayer->isInRange(plr, range))
                continue;

            players.push_back((*itr)->m_loggedInPlayer);
        }
    }

    Player* new_plr = NULL;

    if (players.size() > 0)
    {
        // Get a random player in members subset
        uint32 i = Util::getRandomUInt((uint32)players.size() - 1);
        new_plr = players[i];
        players.clear();
    }

    return new_plr;
}

#if VERSION_STRING > TBC
void Group::UpdateAchievementCriteriaForInrange(Object* o, AchievementCriteriaTypes type, int32 miscvalue1, int32 miscvalue2, uint32 time)
{
    Lock();

    SubGroup* sGrp = NULL;
    GroupMembersSet::iterator itr2;

    for (uint32 Index = 0; Index < GetSubGroupCount(); ++Index)
    {
        sGrp = GetSubGroup(Index);
        itr2 = sGrp->GetGroupMembersBegin();

        for (; itr2 != sGrp->GetGroupMembersEnd(); ++itr2)
        {
            PlayerInfo* p = *itr2;

            if (p->m_loggedInPlayer != NULL && p->m_loggedInPlayer->IsVisible(o->getGuid()))
                p->m_loggedInPlayer->GetAchievementMgr().UpdateAchievementCriteria(type, miscvalue1, miscvalue2, time);
        }
    }

    Unlock();
}
#endif

void Group::Teleport(WorldSession* m_session)
{
	GroupMembersSet::iterator itr1, itr2;
	uint8 i = 0;
	SubGroup* sg1 = NULL;
	Player * member = NULL;
	m_groupLock.Acquire();
	for(i = 0; i < m_SubGroupCount; i++)
	{
		sg1 = m_SubGroups[i];

		if(sg1 != NULL)
		{
			for(itr1 = sg1->GetGroupMembersBegin(); itr1 != sg1->GetGroupMembersEnd(); ++itr1)
			{
				if((*itr1) == NULL)
					continue;
				member = (*itr1)->m_loggedInPlayer;
				// skip offline players and not in world players
				if(member == NULL || !member->IsInWorld())
					continue;
				sChatHandler.HandleSummonCommand(member->getName().c_str(), m_session);
				//member->SafeTeleport(map, instanceid, x, y, z, o);
			}
		}
	}
	m_groupLock.Release();
}

void Group::ExpandToLFG()
{
    m_GroupType = GroupTypes(m_GroupType | GROUP_TYPE_LFD | GROUP_TYPE_UNK1);
    SaveToDB();
    Update();
}

void Group::GoOffline(Player* p)
{
    uint32 mask = GROUP_UPDATE_FLAG_STATUS;
    uint32 byteCount = 0;

    for (uint8 i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];

    WorldPacket data(SMSG_PARTY_MEMBER_STATS, 8 + 4 + byteCount);
    data << p->GetNewGUID();
    data << mask;
    data << uint16(MEMBER_STATUS_OFFLINE);

    if (p->IsInWorld())
    {
        m_groupLock.Acquire();
        for (uint8 i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] == NULL)
                continue;

            for (GroupMembersSet::iterator itr = m_SubGroups[i]->GetGroupMembersBegin(); itr != m_SubGroups[i]->GetGroupMembersEnd();)
            {
                Player* plr = (*itr)->m_loggedInPlayer;
                ++itr;

                if (plr && plr != p)
                    plr->SendPacket(&data);
            }
        }
        m_groupLock.Release();
    }
}
