/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
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

#include "Group.h"

#include "ItemInterface.h"
#include "Management/Loot/LootDefines.hpp"
#include "Management/Loot/LootItem.hpp"
#include "Management/Loot/LootRoll.hpp"
#include "Battleground/BattlegroundMgr.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/LFG/LFGMgr.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/SpellAura.hpp"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/InstanceMap.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Item.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Objects/Units/Creatures/Vehicle.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Server/Packets/SmsgPartyCommandResult.h"
#include "Server/Packets/SmsgGroupSetLeader.h"
#include "Server/Packets/SmsgGroupDestroyed.h"
#include "Server/Packets/SmsgGroupList.h"
#include "Server/Packets/SmsgLootList.h"
#include "Server/Packets/SmsgLootStartRoll.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Server/Packets/SmsgInstanceReset.h"
#include "Server/Packets/SmsgPartyMemberStats.h"
#include "Server/PacketBroadcast.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

Group::Group(bool Assign)
{
    m_GroupType = GROUP_TYPE_PARTY; // Always init as party

    // Create initial subgroup
    std::fill(m_SubGroups.begin(), m_SubGroups.end(), nullptr);
    m_SubGroups[0] = std::make_unique<SubGroup>(this, 0);

    m_Leader = NULL;
    m_Looter = NULL;
    m_LootMethod = PARTY_LOOT_GROUP;
    m_LootThreshold = 2;
    m_SubGroupCount = 1;
    m_MemberCount = 0;

    if (Assign)
    {
        m_Id = sObjectMgr.generateGroupId();
        m_guid = WoWGuid(m_Id, 0, HIGHGUID_TYPE_GROUP).getRawGuid();
    }
    else
    {
        m_Id = 0;
        m_guid = 0;
    }

    m_dirty = false;
    m_updateblock = false;
    m_disbandOnNoMembers = true;
    memset(m_targetIcons, 0, sizeof(uint64_t) * 8);
    m_isqueued = false;
    m_difficulty = 0;
    m_raiddifficulty = 0;
    m_assistantLeader = m_mainAssist = m_mainTank = NULL;
    updatecounter = 0;
}

Group::~Group() = default;

void SubGroup::RemovePlayer(CachedCharacterInfo* info)
{
    m_GroupMembers.erase(info);
    info->subGroup = -1;
}

bool SubGroup::AddPlayer(CachedCharacterInfo* info)
{
    if (IsFull())
        return false;

    info->subGroup = (int8_t)GetID();
    m_GroupMembers.insert(info);
    return true;
}

bool SubGroup::HasMember(uint32_t guid)
{
    for (const auto itr : m_GroupMembers)
        if (itr != nullptr)
            if (itr->guid == guid)
                return true;

    return false;
}

SubGroup* Group::FindFreeSubGroup()
{
    for (uint32_t i = 0; i < m_SubGroupCount; i++)
        if (!m_SubGroups[i]->IsFull())
            return m_SubGroups[i].get();

    return NULL;
}

bool Group::AddMember(CachedCharacterInfo* info, int32_t subgroupid/* =-1 */)
{
    if (info)
    {
        std::lock_guard lock(m_groupLock);

        Player* pPlayer = sObjectMgr.getPlayer(info->guid);

        if (m_isqueued)
        {
            m_isqueued = false;
            sBattlegroundManager.removeGroupFromQueues(m_Id);
        }

        if (!IsFull())
        {
            SubGroup* subgroup = (subgroupid > 0) ? m_SubGroups[subgroupid].get() : FindFreeSubGroup();
            if (subgroup == NULL)
            {
                return false;
            }

            if (subgroup->AddPlayer(info))
            {
                if (pPlayer != NULL)
                    sEventMgr.AddEvent(pPlayer, &Player::eventGroupFullUpdate, EVENT_PLAYER_UPDATE, 1500, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

                if (info->m_Group && info->m_Group->GetID() != m_Id)
                    info->m_Group->RemovePlayer(info);

                if (m_Leader == NULL && pPlayer)
                    m_Leader = info;

                info->m_Group = this;
                info->subGroup = (int8_t)subgroup->GetID();

                ++m_MemberCount;
                m_dirty = true;
                Update(); // Send group update

                return true;
            }

            info->m_Group = NULL;
            info->subGroup = -1;
            return false;
        }

        info->m_Group = NULL;
        info->subGroup = -1;
        return false;
    }

    return false;
}

//\TODO bool silent is not used - remove it!
void Group::SetLeader(Player* pPlayer, bool silent)
{
    if (pPlayer != nullptr)
    {
        m_Leader = pPlayer->getPlayerInfo();
        m_dirty = true;

        if (silent == false)
            SendPacketToAll(SmsgGroupSetLeader(pPlayer->getName()).serialise().get());
    }

    Update();
}

void Group::Update()
{
    if (m_updateblock)
        return;

    Player* pNewLeader = nullptr;

    if (!m_Leader || (m_Leader && sObjectMgr.getPlayer(m_Leader->guid)))
    {
        pNewLeader = FindFirstPlayer();
        if (pNewLeader)
            m_Leader = pNewLeader->getPlayerInfo();
    }

    if (m_Looter && !sObjectMgr.getPlayer(m_Looter->guid))
    {
        if (!pNewLeader)
            pNewLeader = FindFirstPlayer();

        if (pNewLeader)
            m_Looter = pNewLeader->getPlayerInfo();
    }

    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        if (SubGroup* sg1 = m_SubGroups[i].get())
        {
            uint32_t subGroupPosition = 0;
            for (const auto characterInfo : sg1->getGroupMembers())
            {
                // skip offline players
                if (!sObjectMgr.getPlayer(characterInfo->guid))
                {
                    ++subGroupPosition;
                    continue;
                }

                SmsgGroupList managedPacket(false);
                managedPacket.groupType = uint8_t(m_GroupType);
                managedPacket.requesterSubGroup = uint8_t(characterInfo->subGroup);
                managedPacket.groupPosition = subGroupPosition++;

                uint8_t flags = 0;
                if (characterInfo == m_assistantLeader)
                    flags |= 1;
                if (characterInfo == m_mainTank)
                    flags |= 2;
                if (characterInfo == m_mainAssist)
                    flags |= 4;
                managedPacket.requesterFlags = flags;

                //if the leader is in a BG, then the group is a BG group
                managedPacket.isBattlegroundGroup = (m_Leader && sObjectMgr.getPlayer(m_Leader->guid) && sObjectMgr.getPlayer(m_Leader->guid)->IsInBg());

                managedPacket.isLfgGroup = (m_GroupType & GROUP_TYPE_LFD) != 0;
                if (managedPacket.isLfgGroup)
                {
                    managedPacket.lfgState = uint8_t(sLfgMgr.GetState(GetID()) == LFG_STATE_FINISHED_DUNGEON ? 2 : 0);
                    managedPacket.lfgDungeon = uint32_t(sLfgMgr.GetDungeon(GetID()));
                }

                managedPacket.groupGuid = uint64_t(GetID());            // Group guid
                managedPacket.updateCounter = uint32_t(updatecounter++); // 3.3 - increments every time a group list update is being sent to client
                managedPacket.memberCount = uint32_t(m_MemberCount - 1); // we don't include self

                for (uint8_t j = 0; j < m_SubGroupCount; j++)
                {
                    if (SubGroup* sg2 = m_SubGroups[j].get())
                    {
                        for (const auto characterInfo2 : sg2->getGroupMembers())
                        {
                            if (characterInfo == characterInfo2)   // skip self
                                continue;

                            // should never happen but just in case
                            if (characterInfo2 == nullptr)
                                continue;

                            Player* plr = sObjectMgr.getPlayer(characterInfo2->guid);

                            SmsgGroupListMember member;
                            member.name = plr ? plr->getName() : characterInfo2->name;
                            member.guid = plr ? plr->getGuid() : uint64_t(characterInfo2->guid); // highguid 0
                            member.isOnline = sObjectMgr.getPlayer(characterInfo2->guid) != nullptr;
                            member.subGroup = uint8_t(characterInfo2->subGroup);

                            uint8_t memberFlags = 0;
                            if (characterInfo2 == m_assistantLeader)
                                memberFlags |= 1;
                            if (characterInfo2 == m_mainTank)
                                memberFlags |= 2;
                            if (characterInfo2 == m_mainAssist)
                                memberFlags |= 4;

                            member.flags = memberFlags;
                            member.roles = uint8_t(plr ? plr->retRoles() : 0);   // Player roles

                            managedPacket.members.push_back(std::move(member));
                        }
                    }
                }

                managedPacket.hasLeader = (m_Leader != nullptr);
                if (m_Leader)
                    managedPacket.leaderGuid = uint64_t(m_Leader->guid);

                managedPacket.lootMethod = uint8_t(m_LootMethod);

                managedPacket.hasLooter = (m_Looter != nullptr);
                if (m_Looter)
                    managedPacket.looterGuid = uint64_t(m_Looter->guid);

                managedPacket.lootThreshold = uint8_t(m_LootThreshold);
                managedPacket.difficulty = uint8_t(m_difficulty);
                managedPacket.raidDifficulty = uint8_t(m_raiddifficulty);

                if (Player* loggedInPlayer = sObjectMgr.getPlayer(characterInfo->guid))
                {
                    if (!loggedInPlayer->IsInWorld())
                    {
                        if (auto packet = loggedInPlayer->getSession()->buildPacket(managedPacket))
                            loggedInPlayer->copyAndSendDelayedPacket(packet.get());
                    }
                    else
                    {
                        loggedInPlayer->getSession()->sendManagedPacket(managedPacket);
                    }
                }
            }
        }
    }

    if (m_dirty)
    {
        m_dirty = false;
        SaveToDB();
    }
}

void Group::Disband()
{
    m_groupLock.lock();

    m_updateblock = true;

    if (m_isqueued)
    {
        m_isqueued = false;

        SmsgMessageChat packet(SystemMessagePacket("A change was made to your group. Removing the arena queue."));

        std::lock_guard lock(m_groupLock);

        for (uint8_t i = 0; i < m_SubGroupCount; i++)
        {
            for (auto groupMember : m_SubGroups[i]->getGroupMembers())
            {
                if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMember->guid))
                    if (loggedInPlayer->getSession())
                        loggedInPlayer->getSession()->sendManagedPacket(packet);
            }
        }

        sBattlegroundManager.removeGroupFromQueues(m_Id);
    }

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        SubGroup* sg = m_SubGroups[i].get();
        sg->Disband();
    }

    m_groupLock.unlock();

    CharacterDatabase.execute("DELETE FROM `groups` WHERE `group_id` = %u", m_Id);
    sObjectMgr.removeGroup(m_Id);    // destroy ourselves
}

void SubGroup::Disband()
{
    for (auto itr = m_GroupMembers.begin(); itr != m_GroupMembers.end();)
    {
        if (*itr)
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer((*itr)->guid))
            {
                if (loggedInPlayer->getSession() != nullptr)
                {
                    SmsgPartyCommandResult partyPacket(2, "", loggedInPlayer->getDungeonDifficulty());
                    loggedInPlayer->getSession()->sendManagedPacket(partyPacket);

                    SmsgGroupDestroyed managedPacket;
                    loggedInPlayer->getSession()->sendManagedPacket(managedPacket);
#if VERSION_STRING >= Cata
                    loggedInPlayer->getSession()->sendEmptyGroupList(loggedInPlayer);
#else
                    (*itr)->m_Group->SendNullUpdate(loggedInPlayer);   // cebernic: panel refresh.
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
}

Player* Group::FindFirstPlayer()
{
    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i])
        {
            for (const auto itr : m_SubGroups[i]->getGroupMembers())
            {
                if (itr)
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
                    {
                        return loggedInPlayer;
                    }
                }
            }
        }
    }

    return nullptr;
}

void Group::RemovePlayer(CachedCharacterInfo* info)
{
    if (info == nullptr)
        return;

    Player* pPlayer = sObjectMgr.getPlayer(info->guid);

    m_groupLock.lock();

    if (m_isqueued)
    {
        m_isqueued = false;
        sBattlegroundManager.removeGroupFromQueues(m_Id);
    }

    SubGroup* sg = nullptr;
    if (info->subGroup >= 0 && info->subGroup < 8)
        sg = m_SubGroups[info->subGroup].get();

    if (!sg || sg->m_GroupMembers.find(info) == sg->m_GroupMembers.end())
    {
        for (uint8_t i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i])
            {
                if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
                {
                    sg = m_SubGroups[i].get();
                    break;
                }
            }
        }
    }

    info->m_Group = nullptr;
    info->subGroup = -1;

    if (!sg)
    {
        m_groupLock.unlock();
        return;
    }

    m_dirty = true;
    sg->RemovePlayer(info);
    --m_MemberCount;

    m_groupLock.unlock();

    // remove team member from the instance
    if (pPlayer)
    {
        if (pPlayer->getSession())
        {
#if VERSION_STRING < Cata
            SendNullUpdate(pPlayer);
#endif

            SmsgGroupDestroyed managedPacket;
            pPlayer->getSession()->sendManagedPacket(managedPacket);

            SmsgPartyCommandResult partyPacket(2, pPlayer->getName(), ERR_PARTY_NO_ERROR);
            pPlayer->getSession()->sendManagedPacket(partyPacket);

                    
#if VERSION_STRING >= Cata
            pPlayer->getSession()->sendEmptyGroupList(pPlayer);
#endif
        }

        //Remove some party auras.
        for (const auto& aur : pPlayer->getPositiveAuraRange())
        {
            if (aur && aur->m_areaAura)
            {
                Object* caster = aur->getCaster();
                if (caster && pPlayer->getGuid() != caster->getGuid())
                    aur->removeAura();
            }
        }
    }

    if (m_MemberCount < 2)
    {
        if (m_disbandOnNoMembers)
        {
            Disband();
            return;
        }
    }

    m_groupLock.lock();

    /* eek! ;P */
    Player* newPlayer = nullptr;
    if (m_Looter == info)
    {
        newPlayer = FindFirstPlayer();
        if (newPlayer)
            m_Looter = newPlayer->getPlayerInfo();
        else
            m_Looter = nullptr;
    }

    if (m_Leader == info)
    {
        if (newPlayer == nullptr)
            newPlayer = FindFirstPlayer();

        if (newPlayer)
            SetLeader(newPlayer, false);
        else
            m_Leader = nullptr;
    }

    m_groupLock.unlock();

    Update();
}

void Group::ExpandToRaid()
{
    if (m_isqueued)
    {
        m_isqueued = false;

        SmsgMessageChat packet(SystemMessagePacket("A change was made to your group. Removing the arena queue."));

        std::lock_guard lock(m_groupLock);

        for (uint8_t i = 0; i < m_SubGroupCount; i++)
        {
            for (auto groupMember : m_SubGroups[i]->getGroupMembers())
            {
                if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMember->guid))
                    if (loggedInPlayer->getSession())
                        loggedInPlayer->getSession()->sendManagedPacket(packet);
            }
        }

        sBattlegroundManager.removeGroupFromQueues(m_Id);
    }

    // Very simple ;)
    std::lock_guard lock(m_groupLock);

    m_SubGroupCount = 8;

    for (uint8_t i = 1; i < m_SubGroupCount; ++i)
        m_SubGroups[i] = std::make_unique<SubGroup>(this, i);

    m_GroupType = GROUP_TYPE_RAID;
    m_dirty = true;
    Update();
}

void Group::SetLooter(Player* pPlayer, uint8_t method, uint16_t threshold)
{
    if (pPlayer)
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
    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        for (auto groupMember : m_SubGroups[i]->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMember->guid))
                if (loggedInPlayer != pSkipTarget && loggedInPlayer->getSession())
                    loggedInPlayer->getSession()->SendPacket(packet);
        }
    }
}

void Group::OutPacketToAllButOne(uint16_t op, uint16_t len, const void* data, Player* pSkipTarget)
{
    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        for (auto groupMember : m_SubGroups[i]->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(groupMember->guid))
                if (loggedInPlayer != pSkipTarget)
                    loggedInPlayer->getSession()->OutPacket(op, len, data);
        }
    }
}

bool Group::HasMember(Player* pPlayer)
{
    if (!pPlayer)
        return false;

    std::set<CachedCharacterInfo*>::iterator itr;

    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i] != NULL)
        {
            if (m_SubGroups[i]->m_GroupMembers.find(pPlayer->getPlayerInfo()) != m_SubGroups[i]->m_GroupMembers.end())
            {
                return true;
            }
        }
    }

    return false;
}

bool Group::HasMember(CachedCharacterInfo* info)
{
    std::set<CachedCharacterInfo*>::iterator itr;
    uint8_t i = 0;

    std::lock_guard lock(m_groupLock);

    for (; i < m_SubGroupCount; i++)
    {
        if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
        {
            return true;
        }
    }

    return false;
}

void Group::MovePlayer(CachedCharacterInfo* info, uint8_t subgroup)
{
    if (subgroup >= m_SubGroupCount)
        return;

    if (m_SubGroups[subgroup]->IsFull())
        return;

    std::lock_guard lock(m_groupLock);

    SubGroup* sg = NULL;

    if (info->subGroup > 0 && info->subGroup < 8)
        sg = m_SubGroups[info->subGroup].get();

    if (sg == NULL || sg->m_GroupMembers.find(info) == sg->m_GroupMembers.end())
    {
        for (uint8_t i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] != NULL)
            {
                if (m_SubGroups[i]->m_GroupMembers.find(info) != m_SubGroups[i]->m_GroupMembers.end())
                {
                    sg = m_SubGroups[i].get();
                    break;
                }
            }
        }
    }

    if (sg == NULL)
    {
        return;
    }

    sg->RemovePlayer(info);

    // Grab the new group, and insert
    sg = m_SubGroups[subgroup].get();
    if (!sg->AddPlayer(info))
        RemovePlayer(info);
    else
        info->subGroup = (int8_t)sg->GetID();

    Update();
}

void Group::SendNullUpdate(Player* pPlayer)
{
    SmsgGroupList managedPacket;
    pPlayer->getSession()->sendManagedPacket(managedPacket);
}

void Group::LoadFromDB(Field* fields)
{
#define LOAD_ASSISTANT(__i, __d) g = fields[__i].asUint32(); if (g != 0) { __d = sObjectMgr.getCachedCharacterInfo(g); }

    std::lock_guard lock(m_groupLock);

    uint32_t g;
    m_updateblock = true;

    m_Id = fields[0].asUint32();

    m_GroupType = fields[1].asUint8();
    m_SubGroupCount = fields[2].asUint8();
    m_LootMethod = fields[3].asUint8();
    m_LootThreshold = fields[4].asUint8();
    m_difficulty = fields[5].asUint8();
    m_raiddifficulty = fields[6].asUint8();

    LOAD_ASSISTANT(7, m_assistantLeader);
    LOAD_ASSISTANT(8, m_mainTank);
    LOAD_ASSISTANT(9, m_mainAssist);

    // create groups
    for (uint8_t i = 1; i < m_SubGroupCount; ++i)
        m_SubGroups[i] = std::make_unique<SubGroup>(this, i);

    // assign players into groups
    for (uint8_t i = 0; i < m_SubGroupCount; ++i)
    {
        for (uint8_t j = 0; j < 5; ++j)
        {
            uint32_t guid = fields[10 + (i * 5) + j].asUint32();
            if (guid == 0)
                continue;

            CachedCharacterInfo* inf = sObjectMgr.getCachedCharacterInfo(guid);
            if (inf == NULL)
                continue;

            AddMember(inf);
        }
    }

    m_updateblock = false;
}

void Group::SaveToDB()
{
    if (!m_disbandOnNoMembers) /* don't save bg groups */
        return;

    std::stringstream ss;
    //uint32_t i = 0;
    //uint32_t fillers = 8 - m_SubGroupCount;


    ss << "DELETE FROM `groups` WHERE `group_id` = ";
    ss << m_Id;
    ss << ";";

    CharacterDatabase.execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO `groups` (group_id, group_type, subgroup_count, loot_method, loot_threshold, difficulty, raiddifficulty, assistant_leader, main_tank, main_assist,\
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
        << uint32_t(m_GroupType) << "," // group_type (2/52)
        << uint32_t(m_SubGroupCount) << "," // subgroup_count (3/52)
        << uint32_t(m_LootMethod) << "," // loot_method (4/52)
        << uint32_t(m_LootThreshold) << "," // loot_threshold (5/52)
        << uint32_t(m_difficulty) << "," // difficulty (6/52)
        << uint32_t(m_raiddifficulty) << ","; // raiddifficulty (7/52)

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
    for (uint8_t i = 0; i < m_SubGroupCount; ++i)
    {
        uint8_t j = 0;

        // For each member in the group, while membercount is less than 5 (guard clause), add their ID to query
        for (const auto& itr : m_SubGroups[i]->getGroupMembers())
        {
            ss << itr->guid << ",";
            ++j;
        }

        // Add 0 to query until we reach 5 (fill empty slots)
        for (; j < 5; ++j)
            ss << "0,";

        membersNotFilled -= 5;
    }

    // Fill remaining empty slots
    while (membersNotFilled > 0)
    {
        ss << "0,";
        --membersNotFilled;
    }

    //for (uint32_t i = 0; i < fillers; ++i)
    //    ss << "0, 0, 0, 0, 0,";

    // timestamp (51/52)
    ss << (uint32_t)UNIXTIME << ",'";

    // instanceids (52/52) // unused 03.02.22 pending delete
    ss << 0 << ":" << 0 << ":" << 0 << " ";

    ss << "')";
    /*printf("==%s==\n", ss.str().c_str());*/
    CharacterDatabase.execute(ss.str().c_str());
}

void Group::UpdateOutOfRangePlayer(Player* pPlayer, bool Distribute, Player* singleTarget)
{
    if (pPlayer == nullptr)
        return;

    uint32_t mask = pPlayer->getGroupUpdateFlags();
    if (mask & GROUP_UPDATE_FLAG_POWER_TYPE)                // if update power type, update current/max power also
        mask |= (GROUP_UPDATE_FLAG_CUR_POWER | GROUP_UPDATE_FLAG_MAX_POWER);

    if (mask & GROUP_UPDATE_FLAG_PET_POWER_TYPE)            // same for pets
        mask |= (GROUP_UPDATE_FLAG_PET_CUR_POWER | GROUP_UPDATE_FLAG_PET_MAX_POWER);

    if (pPlayer->m_isGmInvisible)
        mask = GROUP_UPDATE_FLAG_STATUS;

    uint32_t byteCount = 0;
    for (uint8_t i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];

    SmsgPartyMemberStats statsPacket(pPlayer->GetNewGUID(), mask);
    statsPacket.playerMember = pPlayer;

    if (!Distribute && singleTarget != nullptr)
    {
        singleTarget->getSession()->sendManagedPacket(statsPacket);
        return;
    }

    if (Distribute && pPlayer->IsInWorld())
    {
        float dist = pPlayer->getWorldMap()->getVisibilityRange();
        m_groupLock.lock();
        for (uint8_t i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] == nullptr)
                continue;

            for (const auto itr : m_SubGroups[i]->getGroupMembers())
            {
                Player* plr = sObjectMgr.getPlayer(itr->guid);
                if (plr && plr != pPlayer)
                {
                    if (plr->GetDistance2dSq(pPlayer) > dist)
                        plr->getSession()->sendManagedPacket(statsPacket);
                }
            }
        }
        m_groupLock.unlock();
    }
}

void Group::UpdateAllOutOfRangePlayersFor(Player* pPlayer)
{
    if (m_SubGroupCount > 8)
        return;

    // tell the other players about us
    UpdateOutOfRangePlayer(pPlayer, true);

    UpdateMask myMask;
    myMask.SetCount(getSizeOfStructure(WoWPlayer));
    UpdateMask hisMask;
    hisMask.SetCount(getSizeOfStructure(WoWPlayer));

    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; ++i)
    {
        if (m_SubGroups[i] == nullptr)
            continue;

        for (const auto itr : m_SubGroups[i]->getGroupMembers())
        {
            Player* plr = sObjectMgr.getPlayer(itr->guid);
            if (!plr || plr == pPlayer)
                continue;

            if (!plr->isVisibleObject(pPlayer->getGuid()))
            {
                UpdateOutOfRangePlayer(plr, false, pPlayer);
            }
            else
            {
                if (pPlayer->getSubGroupSlot() == plr->getSubGroupSlot())
                {
                    WorldPacket data(150);

                    // distribute quest fields to other players
                    hisMask.Clear();
                    myMask.Clear();

                    bool u1 = false;
                    bool u2 = false;

#if VERSION_STRING == Classic
                    uint16_t questIdOffset = 3;
#elif VERSION_STRING == TBC
                    uint16_t questIdOffset = 4;
#else
                    uint16_t questIdOffset = 5;
#endif

                    const uint32_t startBit = getOffsetForStructuredField(WoWPlayer, quests);

                    for (uint8_t x = 0; x < WOWPLAYER_QUEST_COUNT; ++x)
                    {
                        if (plr->getQuestLogEntryForSlot(x))
                        {
                            for (uint16_t j = startBit * x; j < startBit * x + questIdOffset; ++j)
                                hisMask.SetBit(j);

                            u1 = true;
                        }

                        if (pPlayer->getQuestLogEntryForSlot(x))
                        {
                            u2 = true;

                            for (uint16_t j = startBit * x; j < startBit * x + questIdOffset; ++j)
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
}

bool Group::isRaid() const
{
    return getGroupType() == GROUP_TYPE_RAID;
}

void Group::SetMainAssist(CachedCharacterInfo* pMember)
{
    if (m_mainAssist == pMember)
        return;

    m_mainAssist = pMember;
    m_dirty = true;
    Update();
}

void Group::SetMainTank(CachedCharacterInfo* pMember)
{
    if (m_mainTank == pMember)
        return;

    m_mainTank = pMember;
    m_dirty = true;
    Update();
}

void Group::SetAssistantLeader(CachedCharacterInfo* pMember)
{
    if (m_assistantLeader == pMember)
        return;

    m_assistantLeader = pMember;
    m_dirty = true;
    Update();
}

void Group::readyCheckStart()
{
    m_readyCheckInProgress = true;
    m_readyCheckResponded.clear();
}

void Group::readyCheckStop()
{
    m_readyCheckInProgress = false;
    m_readyCheckResponded.clear();
}

void Group::readyCheckMemberResponded(uint32_t guidLow)
{
    m_readyCheckResponded.insert(guidLow);
}

bool Group::readyCheckAllResponded() const
{
    for (uint8_t i = 0; i < m_SubGroupCount; ++i)
    {
        for (auto* member : m_SubGroups[i]->getGroupMembers())
        {
            if (m_readyCheckResponded.find(member->guid) == m_readyCheckResponded.end())
                return false;
        }
    }

    return true;
}

void Group::resetInstances(uint8_t method, bool isRaid, Player* SendMsgTo)
{
    if (isBGGroup() /* || isBFGroup()*/)
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_DISBAND

    // we assume that when the difficulty changes, all instances that can be reset will be
    InstanceDifficulty::Difficulties diff = getDifficulty(isRaid);

    for (BoundInstancesMap::iterator itr = m_boundInstances[diff].begin(); itr != m_boundInstances[diff].end();)
    {
        InstanceSaved* instanceSave = itr->second.save;
        WDB::Structures::MapEntry const* entry = sMapStore.lookupEntry(itr->first);
        if (!entry || entry->isRaid() != isRaid || (!instanceSave->canReset() && method != INSTANCE_RESET_GROUP_DISBAND))
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->mapType == WDB::Structures::MAP_RAID || diff == InstanceDifficulty::Difficulties::DUNGEON_HEROIC)
            {
                ++itr;
                continue;
            }
        }

        bool isEmpty = true;
        // if the map is loaded, reset it
        WorldMap* map = sMapMgr.findWorldMap(instanceSave->getMapId(), instanceSave->getInstanceId());
        if (map && map->getBaseMap()->isInstanceMap() && !(method == INSTANCE_RESET_GROUP_DISBAND && !instanceSave->canReset()))
        {
            if (instanceSave->canReset())
                isEmpty = ((InstanceMap*)map)->reset(method);
            else
                isEmpty = !map->getPlayerCount();
        }

        if (SendMsgTo && SendMsgTo->getSession())
        {
            if (!isEmpty)
            {
                SendMsgTo->sendResetInstanceFailed(0, instanceSave->getMapId());
            }
            else
            {
                SmsgInstanceReset managedPacket(instanceSave->getMapId());
                SendMsgTo->getSession()->sendManagedPacket(managedPacket);
            }
        }

        if (isEmpty || method == INSTANCE_RESET_GROUP_DISBAND || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if (isEmpty && instanceSave->canReset())
            {
                instanceSave->deleteFromDB();
            }
            else
            {
                CharacterDatabase.execute("DELETE FROM group_instance WHERE instance = %u", instanceSave->getInstanceId());
            }

            // i don't know for sure if hash_map iterators
            m_boundInstances[diff].erase(itr);
            itr = m_boundInstances[diff].begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            instanceSave->removeGroup(this);
        }
        else
        {
            ++itr;
        }
    }
}

InstanceGroupBind* Group::getBoundInstance(Player* player)
{
    uint32_t mapid = player->GetMapId();
    WDB::Structures::MapEntry const* mapEntry = sMapStore.lookupEntry(mapid);
    return getBoundInstance(mapEntry);
}

InstanceGroupBind* Group::getBoundInstance(BaseMap* aMap)
{
    // Currently spawn numbering not different from map difficulty
    InstanceDifficulty::Difficulties difficulty = getDifficulty(aMap->isRaid());
    return getBoundInstance(difficulty, aMap->getMapId());
}

InstanceGroupBind* Group::getBoundInstance(WDB::Structures::MapEntry const* mapEntry)
{
    if (!mapEntry || !mapEntry->isInstanceMap())
        return nullptr;

    InstanceDifficulty::Difficulties difficulty = getDifficulty(mapEntry->isRaid());
    return getBoundInstance(difficulty, mapEntry->id);
}

InstanceGroupBind* Group::getBoundInstance(InstanceDifficulty::Difficulties difficulty, uint32_t mapId)
{
    // some instances only have one difficulty
#if VERSION_STRING > TBC
    getDownscaledMapDifficultyData(mapId, difficulty);
#endif

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapId);
    if (itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return nullptr;
}

Group::BoundInstancesMap& Group::getBoundInstances(InstanceDifficulty::Difficulties difficulty)
{
    return m_boundInstances[difficulty];
}

InstanceGroupBind* Group::bindToInstance(InstanceSaved* save, bool permanent, bool load)
{
    if (!save   || isBGGroup() /*|| isBFGroup()*/)
        return nullptr;

    InstanceGroupBind& bind = m_boundInstances[save->getDifficulty()][save->getMapId()];
    if (!load && (!bind.save || permanent != bind.perm || save != bind.save))
    {
        CharacterDatabase.execute("REPLACE INTO group_instance (guid, instance, permanent) VALUES (%u, %u, %u)", GetID(), save->getInstanceId(), permanent);
    }

    if (bind.save != save)
    {
        if (bind.save)
            bind.save->removeGroup(this);
        save->addGroup(this);
    }

    bind.save = save;
    bind.perm = permanent;

    return &bind;
}

void Group::unbindInstance(uint32_t mapid, uint8_t difficulty, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if (itr != m_boundInstances[difficulty].end())
    {
        if (!unload)
        {
            CharacterDatabase.execute("DELETE FROM group_instance WHERE guid = %u AND instance = %u", GetID(), itr->second.save->getInstanceId());
        }

        itr->second.save->removeGroup(this);
        m_boundInstances[difficulty].erase(itr);
    }
}

InstanceDifficulty::Difficulties Group::getDifficulty(bool isRaid) const
{
    return isRaid ? InstanceDifficulty::Difficulties(m_raiddifficulty) : InstanceDifficulty::Difficulties(m_difficulty);
}

void Group::SetDungeonDifficulty(uint8_t diff)
{
    m_difficulty = diff;

    Lock();
    for (uint32_t i = 0; i < GetSubGroupCount(); ++i)
    {
        for (const auto itr : GetSubGroup(i)->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
            {
                loggedInPlayer->setDungeonDifficulty(diff);
                loggedInPlayer->sendDungeonDifficultyPacket();
            }
        }
    }
    Unlock();
}

void Group::SetRaidDifficulty(uint8_t diff)
{
    m_raiddifficulty = diff;

    Lock();

    for (uint32_t i = 0; i < GetSubGroupCount(); ++i)
    {
        for (const auto itr : GetSubGroup(i)->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
            {
                loggedInPlayer->setRaidDifficulty(diff);
                loggedInPlayer->sendRaidDifficultyPacket();
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

        uint32_t Flags = dynamic_cast<Unit*>(o)->getDynamicFlags();

        Flags |= U_DYN_FLAG_LOOTABLE;
        Flags |= U_DYN_FLAG_TAPPED_BY_PLAYER;

#if VERSION_STRING < Mop
        o->BuildFieldUpdatePacket(&buf, getOffsetForStructuredField(WoWUnit, dynamic_flags), Flags);
#else
        o->BuildFieldUpdatePacket(&buf, getOffsetForStructuredField(WoWObject, dynamic_field), Flags);
#endif

        Lock();

        switch (m_LootMethod)
        {
            case PARTY_LOOT_ROUND_ROBIN:
            case PARTY_LOOT_FREE_FOR_ALL:
            case PARTY_LOOT_GROUP:
            case PARTY_LOOT_NEED_BEFORE_GREED:
            {
                for (uint32_t Index = 0; Index < GetSubGroupCount(); ++Index)
                {
                    SubGroup* sGrp = GetSubGroup(Index);
                    for (const auto itr2 : sGrp->getGroupMembers())
                    {
                        if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr2->guid))
                            if (loggedInPlayer->isVisibleObject(o->getGuid()))       // Save updates for non-existent creatures
                                loggedInPlayer->getUpdateMgr().pushUpdateData(&buf, 1);
                    }
                }

                break;
            }

            case PARTY_LOOT_MASTER_LOOTER:
            {
                Player* pLooter = GetLooter() ? sObjectMgr.getPlayer(GetLooter()->guid) : nullptr;
                if (pLooter == nullptr)
                    pLooter = sObjectMgr.getPlayer(GetLeader()->guid);

                if (pLooter->isVisibleObject(o->getGuid()))
                {
                    Unit* victim = dynamic_cast<Unit*>(o);

                    victim->setTaggerGuid(pLooter);
                    pLooter->getUpdateMgr().pushUpdateData(&buf, 1);
                }

                break;
            }
        }

        Unlock();
    }
}

void Group::sendGroupLoot(Loot* loot, Object* object, Player* /*plr*/, uint32_t mapId)
{
    std::vector<LootItem>::iterator item;
    uint8_t itemSlot = 0;

    for (item = loot->items.begin(); item != loot->items.end(); ++item, ++itemSlot)
    {
        if (item->is_passed)
            continue;

        if (item->is_blocked)
            continue;

        if (item->is_ffa)
            continue;

        if (!item->itemproto)
            continue;

        //roll for over-threshold item if it's one-player loot
        if (item->itemproto->Quality >= uint32_t(GetThreshold()))
        {
            int32_t ipid = 0;
            uint32_t factor = 0;

            if (item->iRandomProperty)
            {
                ipid = item->iRandomProperty->ID;
            }
            else if (item->iRandomSuffix)
            {
                ipid = -int32_t(item->iRandomSuffix->id);
                factor = Item::generateRandomSuffixFactor(item->itemproto);
            }

            // Block the Item
            loot->items[itemSlot].is_blocked = true;

            // Init Roll
            item->roll = std::make_unique<LootRoll>(60000, MemberCount(), object->getGuid(), itemSlot, item->itemproto->ItemId, factor, uint32_t(ipid), object->getWorldMap());

            // Send Roll
            SmsgLootStartRoll lootStartRollPacket(object->getGuid(), mapId, itemSlot, item->itemproto->ItemId,
                factor, uint32_t(ipid), item->count, 60000, 7 /* some sort of flags that require research */, item->itemproto->DisplayInfoID);

            Lock();
            for (uint32_t i = 0; i < GetSubGroupCount(); ++i)
            {
                for (const auto pinfo : GetSubGroup(i)->getGroupMembers())
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer(pinfo->guid))
                    {
                        if (loggedInPlayer->getItemInterface()->CanReceiveItem(item->itemproto, item->count) == 0)
                        {
                            if (loggedInPlayer->m_passOnLoot)
                                item->playerRolled(loggedInPlayer, ROLL_PASS);
                            else
                                loggedInPlayer->getSession()->sendManagedPacket(lootStartRollPacket);
                        }
                    }
                }
            }
            Unlock();
        }
        else
        {
            item->is_underthreshold = true;
        }
    }
}

Player* Group::GetRandomPlayerInRangeButSkip(Player* plr, float range, Player* plr_skip)
{
    std::vector<Player*> players;

    // We must iterate through all subgroups
    for (uint8_t i = 0; i < 8; ++i)
    {
        SubGroup* s_grp = GetSubGroup(i);

        if (s_grp == nullptr)
            continue;

        for (const auto itr : s_grp->getGroupMembers())
        {
            // Skip NULLs and not alive players
            Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid);
            if (!(loggedInPlayer && loggedInPlayer->isAlive()))
                continue;

            // Skip desired player
            if (loggedInPlayer == plr_skip)
                continue;

            // Skip player not in range
            if (!loggedInPlayer->isInRange(plr, range))
                continue;

            players.push_back(loggedInPlayer);
        }
    }

    Player* new_plr = nullptr;

    if (!players.empty())
    {
        // Get a random player in members subset
        uint32_t i = Util::getRandomUInt((uint32_t)players.size() - 1);
        new_plr = players[i];
        players.clear();
    }

    return new_plr;
}

#if VERSION_STRING > TBC
void Group::UpdateAchievementCriteriaForInrange(Object* o, AchievementCriteriaTypes type, int32_t miscvalue1, int32_t miscvalue2, uint32_t time)
{
    Lock();

    for (uint32_t Index = 0; Index < GetSubGroupCount(); ++Index)
    {
        SubGroup* sGrp = GetSubGroup(Index);
        for (const auto itr2 : sGrp->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr2->guid))
                if (loggedInPlayer->isVisibleObject(o->getGuid()))
                    loggedInPlayer->updateAchievementCriteria(type, miscvalue1, miscvalue2, time);
        }
    }

    Unlock();
}
#endif

void Group::teleport(WorldSession* m_session)
{
    std::lock_guard lock(m_groupLock);

    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        if (SubGroup* sg1 = m_SubGroups[i].get())
        {
            for (const auto itr1 : sg1->getGroupMembers())
            {
                if (itr1 == nullptr)
                    continue;

                Player* member = sObjectMgr.getPlayer(itr1->guid);
                if (member == nullptr || !member->IsInWorld())
                    continue;

                sChatHandler.HandleSummonCommand(member->getName().c_str(), m_session);
            }
        }
    }
}

void Group::ExpandToLFG()
{
    m_GroupType = GroupTypes(m_GroupType | GROUP_TYPE_LFD | GROUP_TYPE_UNK1);
    SaveToDB();
    Update();
}

void Group::GoOffline(Player* p)
{
    uint32_t mask = GROUP_UPDATE_FLAG_STATUS;
    uint32_t byteCount = 0;

    for (uint8_t i = 1; i < GROUP_UPDATE_FLAGS_COUNT; ++i)
        if (mask & (1 << i))
            byteCount += GroupUpdateLength[i];

    SmsgPartyMemberStats statsPacket(p->GetNewGUID(), mask);
    statsPacket.playerMember = p;

    if (p->IsInWorld())
    {
        std::lock_guard lock(m_groupLock);

        for (uint8_t i = 0; i < m_SubGroupCount; ++i)
        {
            if (m_SubGroups[i] == nullptr)
                continue;

            for (const auto itr : m_SubGroups[i]->getGroupMembers())
            {
                Player* plr = sObjectMgr.getPlayer(itr->guid);
                if (plr && plr != p && plr->getSession())
                    plr->getSession()->sendManagedPacket(statsPacket);
            }
        }
    }
}

void Group::sendLooter(Creature* creature, Player* groupLooter)
{
    SmsgLootList managedPacket{ creature->GetNewGUID() };

    if (groupLooter)
    {
        managedPacket.hasGroupLooter = true;
        managedPacket.groupLooterGuid = groupLooter->GetNewGUID();
        managedPacket.looterGuid = GetLooter()->guid;
    }

    PacketBroadcast::sendFromGroup(*this, managedPacket);
}

void Group::updateLooterGuid(Object* pLootedObject)
{
    switch (GetMethod())
    {
    case PARTY_LOOT_MASTER_LOOTER:
    case PARTY_LOOT_FREE_FOR_ALL:
        return;
    default:
        // round robin style looting applies for all low
        // quality items in each loot method except free for all and master loot
        break;
    }

    CachedCharacterInfo* oldLooter = GetLooter();
    if (!oldLooter)
        oldLooter = GetLeader();

    CachedCharacterInfo* pNewLooter = nullptr;

    m_groupLock.lock();
    for (uint8_t i = 0; i < m_SubGroupCount; i++)
    {
        auto start = m_SubGroups[i]->m_GroupMembers.begin();
        auto member = m_SubGroups[i]->m_GroupMembers.find(oldLooter);

        if (m_SubGroups[i]->m_GroupMembers.find(oldLooter) != m_SubGroups[i]->m_GroupMembers.end())
        {
            auto nextFromMember = std::next(member);

            // try to get next member
            if (nextFromMember != m_SubGroups[i]->m_GroupMembers.end())
            {
                if ((*nextFromMember))
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer((*nextFromMember)->guid))
                    {
                        if (loggedInPlayer->isAtGroupRewardDistance(pLootedObject))
                        {
                            pNewLooter = (*nextFromMember);
                            break;
                        }
                    }
                }
            }
            else // get member from start
            {
                if ((*start))
                {
                    if (Player* loggedInPlayer = sObjectMgr.getPlayer((*start)->guid))
                    {
                        if (loggedInPlayer->isAtGroupRewardDistance(pLootedObject))
                        {
                            if ((*start) != oldLooter)
                            {
                                pNewLooter = (*start);
                                break;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if (i < 7)
            {
                const auto nextSubGroup = m_SubGroups[i + 1].get();
                if (nextSubGroup && nextSubGroup->m_GroupMembers.begin() != nextSubGroup->m_GroupMembers.end())
                {
                    continue;
                }

                if (m_SubGroups[i]->m_GroupMembers.begin() != m_SubGroups[i]->m_GroupMembers.end())
                {
                    member = m_SubGroups[i]->m_GroupMembers.begin();
                    if ((*member))
                        if (sObjectMgr.getPlayer((*member)->guid))
                            pNewLooter = (*member);
                }
            }
        }

        // Get First member on a subGroup it coult be possible that group 1 is empty
        if (!pNewLooter)
        {
            for (uint8_t x = 0; x < m_SubGroupCount; x++)
            {
                if (m_SubGroups[x]->m_GroupMembers.begin() != m_SubGroups[x]->m_GroupMembers.end())
                {
                    member = m_SubGroups[x]->m_GroupMembers.begin();
                    if ((*member))
                    {
                        if (sObjectMgr.getPlayer((*member)->guid))
                        {
                            if ((*member) != oldLooter)
                            {
                                pNewLooter = (*member);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    m_groupLock.unlock();

    if (pNewLooter)
    {
        if (oldLooter != pNewLooter)
            m_Looter = pNewLooter;
    }

    // Update Group
    Update();
}
