/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Management/Battleground/Battleground.hpp"
#include "Management/Battleground/BattlegroundMgr.hpp"
#include "Management/Arenas.hpp"
#include "Management/ArenaTeam.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/BattleGroundMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Packets/SmsgArenaError.h"
#include "Server/Packets/CmsgBattlemasterJoin.h"
#include "Server/Packets/SmsgGroupJoinedBattleground.h"
#include "Server/Packets/SmsgBattlefieldStatus.h"
#include "Storage/WorldStrings.h"
#include "Utilities/Random.hpp"

using namespace AscEmu::Packets;

BattlegroundManager& BattlegroundManager::getInstance()
{
    static BattlegroundManager mInstance;
    return mInstance;
}

void BattlegroundManager::initialize()
{
    m_holder = sEventMgr.GetEventHolder(WORLD_INSTANCE);

    sEventMgr.AddEvent(this, &BattlegroundManager::eventQueueUpdate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 15000, 0, 0);

    for (auto& m_instance : m_instances)
        m_instance.clear();
}

void BattlegroundManager::registerBgFactory(uint32_t map, BattlegroundFactoryMethod method)
{
    const auto bgFactory = m_bgFactories.find(map);
    if (bgFactory != m_bgFactories.end())
        return;

    m_bgFactories[map] = method;
}

void BattlegroundManager::registerArenaFactory(uint32_t map, ArenaFactoryMethod method)
{
    const auto itr = std::find(m_arenaMaps.begin(), m_arenaMaps.end(), map);
    if (itr != m_arenaMaps.end())
        return;

    m_arenaMaps.push_back(map);
    m_arenaFactories.push_back(method);
}

void BattlegroundManager::registerMapForBgType(uint32_t type, uint32_t map)
{
    const auto itr = m_bgMaps.find(type);
    if (itr != m_bgMaps.end())
        return;

    m_bgMaps[type] = map;
}

#if VERSION_STRING <= WotLK
void BattlegroundManager::handleBattlegroundListPacket(WorldSession* session, uint32_t battlegroundType, uint8_t from)
{

    WorldPacket data(SMSG_BATTLEFIELD_LIST, 18);

#if VERSION_STRING == WotLK
    // Send 0 instead of GUID when using the BG UI instead of Battlemaster
    if (from == 0)
        data << uint64_t(session->GetPlayer()->getGuid());
    else
        data << uint64_t(0);

    data << from;
    data << uint32_t(battlegroundType);                                     // typeid

    data << uint8_t(0);                                                     // unk
    data << uint8_t(0);                                                     // unk

    // Rewards
    data << uint8_t(0);                                                     // 3.3.3 hasWin
    data << uint32_t(0);                                                    // 3.3.3 winHonor
    data << uint32_t(0);                                                    // 3.3.3 winArena
    data << uint32_t(0);                                                    // 3.3.3 lossHonor

    uint8_t isRandom = battlegroundType == BattlegroundDef::TYPE_RANDOM;
    data << uint8_t(isRandom);                                              // 3.3.3 isRandom

    // Random bgs
    if (isRandom == 1)
    {
        auto hasWonRbgToday = session->GetPlayer()->hasWonRbgToday();
        uint32_t honorPointsForWinning, honorPointsForLosing, arenaPointsForWinning, arenaPointsForLosing;

        session->GetPlayer()->fillRandomBattlegroundReward(true, honorPointsForWinning, arenaPointsForWinning);
        session->GetPlayer()->fillRandomBattlegroundReward(false, honorPointsForLosing, arenaPointsForLosing);

        // rewards
        data << uint8_t(hasWonRbgToday);
        data << uint32_t(honorPointsForWinning);
        data << uint32_t(arenaPointsForWinning);
        data << uint32_t(honorPointsForLosing);
    }

    if (Battleground::isTypeArena(battlegroundType))
    {
        data << uint32_t(0);
        session->SendPacket(&data);
        return;
    }

    if (battlegroundType >= BATTLEGROUND_NUM_TYPES) // VLack: Nasty hackers might try to abuse this packet to crash us...
        return;

    uint32_t Count = 0;
    const size_t pos = data.wpos();

    data << uint32_t(0); // Count

    // Append the battlegrounds
    std::lock_guard instanceLock(m_instanceLock);
    for (auto itr : m_instances[battlegroundType])
    {
        if (itr.second->CanPlayerJoin(session->GetPlayer(), battlegroundType) && !itr.second->hasEnded())
        {
            data << uint32_t(itr.first);
            ++Count;
        }
    }

    data.put<uint32_t>(pos, Count);
#elif VERSION_STRING <= TBC

    data << uint64_t(session->GetPlayer()->getGuid());
    data << uint32_t(battlegroundType);

    if (Battleground::isTypeArena(battlegroundType))
    {
        data << uint8_t(5);
        data << uint32_t(0);
    }
    else
    {
        data << uint8_t(0);

        if (battlegroundType >= BATTLEGROUND_NUM_TYPES) // VLack: Nasty hackers might try to abuse this packet to crash us...
            return;

        uint32_t Count = 0;
        const size_t pos = data.wpos();

        data << uint32_t(0); // Count

        // Append the battlegrounds
        std::lock_guard instanceLock(m_instanceLock);
        for (auto itr : m_instances[battlegroundType])
        {
            if (itr.second->CanPlayerJoin(session->GetPlayer(), battlegroundType) && !itr.second->hasEnded())
            {
                data << uint32_t(itr.first);
                ++Count;
            }
        }

        data.put<uint32_t>(pos, Count);
    }
#endif

    session->SendPacket(&data);
}
#else
void BattlegroundManager::handleBattlegroundListPacket(WoWGuid& wowGuid, WorldSession* session, uint32_t battlegroundType)
{
    // Zyres: For some reason client requests bg list on login after reaching level 10
    // patiently wait 5 seconds after login. This issue is located in the client standard addons.
    if (session->m_currMsTime - session->m_loginTime < 5 * 1000)
        return;

    std::vector<uint32_t> _bgList;

    std::lock_guard instanceLock(m_instanceLock);
    for (auto itr : m_instances[battlegroundType])
    {
        if (itr.second->CanPlayerJoin(session->GetPlayer(), battlegroundType) && !itr.second->hasEnded())
        {
            if (session->GetPlayer()->getLevelGrouping() != itr.second->getLevelGroup())
                continue;
            _bgList.push_back(itr.second->getId());
        }
    }

    WorldPacket data(SMSG_BATTLEFIELD_LIST, 38 + _bgList.size());

    data << int32_t(0);
    data << int32_t(0);
    data << int32_t(0);
    data << int32_t(battlegroundType);
    data << int32_t(0);
    data << int32_t(0);
    data << int32_t(0);
    data << uint8_t(80);
    data << uint8_t(10);

    data.writeBit(wowGuid[0]);
    data.writeBit(wowGuid[1]);
    data.writeBit(wowGuid[7]);
    data.writeBit(0);
    data.writeBit(0);
    data.writeBits(_bgList.size(), 24);
    data.writeBit(wowGuid[6]);
    data.writeBit(wowGuid[4]);
    data.writeBit(wowGuid[2]);
    data.writeBit(wowGuid[3]);
    data.writeBit(1);
    data.writeBit(wowGuid[5]);
    data.writeBit(0);
    data.flushBits();

    data.WriteByteSeq(wowGuid[6]);
    data.WriteByteSeq(wowGuid[1]);
    data.WriteByteSeq(wowGuid[7]);
    data.WriteByteSeq(wowGuid[5]);

    for (int32_t bgId : _bgList)
        data << int32_t(bgId);

    data.WriteByteSeq(wowGuid[0]);
    data.WriteByteSeq(wowGuid[2]);
    data.WriteByteSeq(wowGuid[4]);
    data.WriteByteSeq(wowGuid[3]);

    session->SendPacket(&data);
}
#endif

void BattlegroundManager::handleBattlegroundJoin(WorldSession* session, WorldPacket& packet)
{
    Player* plr = session->GetPlayer();
    const uint32_t pguid = plr->getGuidLow();
    const uint32_t lgroup = plr->getLevelGrouping();

    CmsgBattlemasterJoin srlPacket;
    if (!srlPacket.deserialise(packet))
        return;

    if (srlPacket.bgType == BattlegroundDef::TYPE_RANDOM)
        plr->setIsQueuedForRbg(true);
    else
        plr->setIsQueuedForRbg(false);

    if (srlPacket.bgType >= BATTLEGROUND_NUM_TYPES || srlPacket.bgType == 0 || m_bgMaps.find(srlPacket.bgType) == m_bgMaps.end() && srlPacket.bgType != BattlegroundDef::TYPE_RANDOM)
    {
        sCheatLog.writefromsession(session, "tried to crash the server by joining battleground that does not exist (0)");
        plr->softDisconnect();
        return;
    }

    if (srlPacket.instanceId)
    {
        // We haven't picked the first instance. This means we've specified an instance to join
        std::lock_guard instanceLock(m_instanceLock);

        const auto itr = m_instances[srlPacket.bgType].find(srlPacket.instanceId);
        if (itr == m_instances[srlPacket.bgType].end())
        {
            session->systemMessage(session->LocalizedWorldSrv(SS_JOIN_INVALID_INSTANCE));
            return;
        }
    }

    // Queue him!
    std::lock_guard queueLock(m_queueLock);
    m_queuedPlayers[srlPacket.bgType][lgroup].push_back(pguid);
    sLogger.info("BattlegroundManager : Player {} is now in battleground queue for instance {}", session->GetPlayer()->getGuidLow(), srlPacket.instanceId + 1);

    plr->setIsQueuedForBg(true);
    plr->setQueuedBgInstanceId(srlPacket.instanceId);
    plr->setBgQueueType(srlPacket.bgType);

    plr->setBGEntryPoint(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetOrientation(), plr->GetMapId(), plr->GetInstanceID());

    sendBattlefieldStatus(plr, BattlegroundDef::STATUS_INQUEUE, srlPacket.bgType, srlPacket.instanceId, 0, m_bgMaps[srlPacket.bgType], 0);
}

void ErasePlayerFromList(uint32_t guid, std::list<uint32_t>* l)
{
    for (auto itr = l->begin(); itr != l->end(); ++itr)
    {
        if (*itr == guid)
        {
            l->erase(itr);
            return;
        }
    }
}

uint8_t GetBattlegroundCaption(BattlegroundDef::Types bgType)
{
    switch (bgType)
    {
        case BattlegroundDef::TYPE_ALTERAC_VALLEY:
            return 38;
        case BattlegroundDef::TYPE_WARSONG_GULCH:
            return 39;
        case BattlegroundDef::TYPE_ARATHI_BASIN:
            return 40;
        case BattlegroundDef::TYPE_ARENA_2V2:
            return 41;
        case BattlegroundDef::TYPE_ARENA_3V3:
            return 42;
        case BattlegroundDef::TYPE_ARENA_5V5:
            return 43;
        case BattlegroundDef::TYPE_EYE_OF_THE_STORM:
            return 44;
        case BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT:
            return 34;
        default:
            return 45;
    }
}

void BattlegroundManager::handleGetBattlegroundQueueCommand(WorldSession* session)
{
    std::stringstream ss;

    std::lock_guard queueLock(m_queueLock);

    bool foundSomething = false;

    for (uint32_t _bgType = 0; _bgType < BATTLEGROUND_NUM_TYPES; ++_bgType)
    {
        for (uint8_t _levelGroup = 0; _levelGroup < BattlegroundDef::MAX_LEVEL_GROUP; ++_levelGroup)
        {
            if (m_queuedPlayers[_bgType][_levelGroup].empty())
                continue;

            foundSomething = true;

            ss << session->LocalizedWorldSrv(GetBattlegroundCaption(static_cast<BattlegroundDef::Types>(_bgType)));

            switch (_levelGroup)
            {
                case 0:
                    ss << " (<10)";
                    break;
                case 1:
                    ss << " (<20)";
                    break;
                case 2:
                    ss << " (<30)";
                    break;
                case 3:
                    ss << " (<40)";
                    break;
                case 4:
                    ss << " (<50)";
                    break;
                case 5:
                    ss << " (<60)";
                    break;
                case 6:
                    ss << " (<70)";
                    break;
                case 7:
                    ss << " (<80)";
                    break;
            }

            ss << ": ";

            ss << static_cast<uint32_t>(m_queuedPlayers[_bgType][_levelGroup].size()) << " players queued";

            if (!Battleground::isTypeArena(_bgType))
            {
                int ally = 0, horde = 0;

                for (auto it3 = m_queuedPlayers[_bgType][_levelGroup].begin(); it3 != m_queuedPlayers[_bgType][_levelGroup].end();)
                {
                    auto it4 = it3++;
                    Player* plr = sObjectMgr.getPlayer(*it4);
                    if (!plr || plr->getLevelGrouping() != _levelGroup)
                        continue;

                    if (plr->isTeamAlliance())
                        ally++;
                    else
                        horde++;
                }

                ss << " (Alliance: " << ally << " Horde: " << horde;

                if (static_cast<int>(m_queuedPlayers[_bgType][_levelGroup].size()) > ally + horde)
                    ss << " Unknown: " << static_cast<int>(m_queuedPlayers[_bgType][_levelGroup].size()) - ally - horde;

                ss << ")";
            }

            session->SystemMessage(ss.str().c_str());
            ss.rdbuf()->str("");
        }

        if (Battleground::isTypeArena(_bgType))
        {
            if (!m_queuedGroups[_bgType].empty())
            {
                foundSomething = true;

                ss << session->LocalizedWorldSrv(GetBattlegroundCaption(static_cast<BattlegroundDef::Types>(_bgType))) << " (rated): ";
                ss << static_cast<uint32_t>(m_queuedGroups[_bgType].size()) << " groups queued";

                session->SystemMessage(ss.str().c_str());
                ss.rdbuf()->str("");
            }
        }
    }

    if (!foundSomething)
        session->SystemMessage("There's nobody queued.");
}

void BattlegroundManager::eventQueueUpdate()
{
    this->eventQueueUpdate(false);
}

uint32_t BattlegroundManager::getArenaGroupQInfo(Group* group, uint8_t type, uint32_t* averageRating)
{
    uint32_t count = 0;
    uint32_t rating = 0;

    if (group == nullptr || group->GetLeader() == nullptr)
        return 0;

    Player* leader = sObjectMgr.getPlayer(group->GetLeader()->guid);
    if (leader == nullptr)
        return 0;

    const auto arenaTeam = leader->getArenaTeam(type - BattlegroundDef::TYPE_ARENA_2V2);
    if (arenaTeam == nullptr)
        return 0;

    for (const auto groupMember : group->GetSubGroup(0)->getGroupMembers())
    {
        if (Player* member = sObjectMgr.getPlayer(groupMember->guid))
        {
            if (arenaTeam == member->getArenaTeam(type - BattlegroundDef::TYPE_ARENA_2V2))
            {
                if (const auto arenaTeamMember = arenaTeam->getMemberByGuid(member->getGuidLow()))
                {
                    rating += arenaTeamMember->PersonalRating;
                    count++;
                }
            }
        }
    }

    *averageRating = count > 0 ? rating / count : 0;

    return arenaTeam->m_id;
}

void BattlegroundManager::addGroupToArena(Battleground* battleground, Group* group, uint32_t team)
{
    if (group == nullptr || group->GetLeader() == nullptr)
        return;

    Player* playerLeader = sObjectMgr.getPlayer(group->GetLeader()->guid);
    if (playerLeader == nullptr)
        return;

    const auto arenaTeam = playerLeader->getArenaTeam(static_cast<uint8_t>(battleground->getType() - BattlegroundDef::TYPE_ARENA_2V2));
    if (arenaTeam == nullptr)
        return;

    for (const auto groupMember : group->GetSubGroup(0)->getGroupMembers())
    {
        playerLeader = sObjectMgr.getPlayer(groupMember->guid);
        if (playerLeader && arenaTeam == playerLeader->getArenaTeam(static_cast<uint8_t>(battleground->getType() - BattlegroundDef::TYPE_ARENA_2V2)))
        {
            if (battleground->hasFreeSlots(team, battleground->getType()))
            {
                battleground->addPlayer(playerLeader, team);
                playerLeader->setTeam(team);
            }
        }
    }
}

int BattlegroundManager::createArenaType(uint8_t type, Group* group1, Group* group2)
{
    const auto arena = dynamic_cast<Arena*>(createInstance(type, BattlegroundDef::LEVEL_GROUP_70));
    if (arena == nullptr)
    {
        sLogger.failure("{} ({}): Couldn't create Arena Instance", __FILE__, __LINE__);
        return -1;
    }

    arena->rated_match = true;

    addGroupToArena(arena, group1, TEAM_ALLIANCE);
    addGroupToArena(arena, group2, TEAM_HORDE);

    return 0;
}

void BattlegroundManager::addPlayerToBg(Battleground* battleground, std::deque<uint32_t> *playerVec, uint32_t type, uint32_t levelGroup)
{
    const uint32_t playerGuid = *playerVec->begin();
    playerVec->pop_front();

    if (Player* player = sObjectMgr.getPlayer(playerGuid))
    {
        if (battleground->CanPlayerJoin(player, battleground->getType()))
        {
            battleground->addPlayer(player, player->getTeam());
            ErasePlayerFromList(player->getGuidLow(), &m_queuedPlayers[type][levelGroup]);
        }
        else
        {
            // Put again the player in the queue
            playerVec->push_back(playerGuid);
        }
    }
    else
    {
        ErasePlayerFromList(playerGuid, &m_queuedPlayers[type][levelGroup]);
    }
}

void BattlegroundManager::addPlayerToBgTeam(Battleground* battleground, std::deque<uint32_t> *playerVec, uint32_t type, uint32_t levelGroup, uint32_t team)
{
    if (battleground->hasFreeSlots(team, battleground->getType()))
    {
        const uint32_t playerGuid = *playerVec->begin();
        playerVec->pop_front();

        if (Player* player = sObjectMgr.getPlayer(playerGuid))
        {
            player->setBgTeam(team);
            battleground->addPlayer(player, team);
        }
        ErasePlayerFromList(playerGuid, &m_queuedPlayers[type][levelGroup]);
    }
}

void BattlegroundManager::eventQueueUpdate(bool forceStart)
{
    std::deque<uint32_t> tempPlayerVec[2];

    Player* player;
    Battleground* battleground;

    std::list<uint32_t>::iterator it3, it4;
    std::map<uint32_t, Battleground*>::iterator iitr;

    Arena* arena;

    int32_t team;
    uint32_t playerGuid;
    uint32_t factionMap[MAX_PLAYER_TEAMS];
    uint32_t count;

    std::queue<uint32_t> teams[MAX_PLAYER_TEAMS];

    std::lock_guard queueLock(m_queueLock);
    std::lock_guard instanceLock(m_instanceLock);

    for (uint32_t _bgType = 0; _bgType < BATTLEGROUND_NUM_TYPES; ++_bgType)
    {
        for (uint8_t _levelGroup = 0; _levelGroup < BattlegroundDef::MAX_LEVEL_GROUP; ++_levelGroup)
        {
            if (m_queuedPlayers[_bgType][_levelGroup].empty())
                continue;

            tempPlayerVec[0].clear();
            tempPlayerVec[1].clear();

            // We try to add the players who queued for a specific Bg/Arena instance to
            // the Bg/Arena where they queued to, and add the rest to another list
            for (it3 = m_queuedPlayers[_bgType][_levelGroup].begin(); it3 != m_queuedPlayers[_bgType][_levelGroup].end();)
            {
                it4 = it3++;
                playerGuid = *it4;
                player = sObjectMgr.getPlayer(playerGuid);

                // Player has left the game or switched level group since queuing (by leveling for example)
                if (!player || player->getLevelGrouping() != _levelGroup)
                {
                    m_queuedPlayers[_bgType][_levelGroup].erase(it4);
                    continue;
                }

                // queued to a specific instance id?
                if (player->hasQueuedBgInstanceId())
                {
                    iitr = m_instances[_bgType].find(player->getQueuedBgInstanceId());
                    if (iitr == m_instances[_bgType].end())
                    {
                        // queue no longer valid, since instance has closed since queuing
                        player->getSession()->SystemMessage(player->getSession()->LocalizedWorldSrv(SS_QUEUE_BG_INSTANCE_ID_NO_VALID_DELETED), player->getQueuedBgInstanceId());
                        player->setIsQueuedForBg(false);
                        player->setBgQueueType(0);
                        player->setQueuedBgInstanceId(0);
                        m_queuedPlayers[_bgType][_levelGroup].erase(it4);
                        continue;
                    }

                    // can we join the specified Bg instance?
                    battleground = iitr->second;
                    if (battleground->CanPlayerJoin(player, battleground->getType()))
                    {
                        battleground->addPlayer(player, player->getTeam());
                        m_queuedPlayers[_bgType][_levelGroup].erase(it4);
                    }
                }
                else
                {
                    if (Battleground::isTypeArena(_bgType))
                        tempPlayerVec[player->getTeam()].push_back(playerGuid);
                    else if (!player->hasAurasWithId(BattlegroundDef::DESERTER))
                        tempPlayerVec[player->getTeam()].push_back(playerGuid);
                }
            }

            /// Now that we have a list of players who didn't queue for a specific instance
            /// try to add them to a Bg/Arena that is already under way
            std::vector<uint32_t> tryJoinVec;
            if (_bgType == BattlegroundDef::TYPE_RANDOM)
                tryJoinVec = m_avalibleInRandom;
            else
                tryJoinVec.push_back(_bgType);

            for (unsigned int tmpJoinBgType : tryJoinVec)
            {
                for (iitr = m_instances[tmpJoinBgType].begin(); iitr != m_instances[tmpJoinBgType].end(); ++iitr)
                {
                    if (iitr->second->hasEnded() || iitr->second->getLevelGroup() != _levelGroup)
                        continue;

                    if (Battleground::isTypeArena(_bgType))
                    {
                        arena = dynamic_cast<Arena*>(iitr->second);
                        if (arena->Rated())
                            continue;

                        factionMap[0] = arena->GetTeamFaction(0);
                        factionMap[1] = arena->GetTeamFaction(1);

                        team = arena->GetFreeTeam();
                        while (team >= 0 && !tempPlayerVec[factionMap[team]].empty())
                        {
                            playerGuid = *tempPlayerVec[factionMap[team]].begin();
                            tempPlayerVec[factionMap[team]].pop_front();
                            player = sObjectMgr.getPlayer(playerGuid);
                            if (player)
                            {
                                player->setBgTeam(team);
                                arena->addPlayer(player, team);
                                team = arena->GetFreeTeam();
                            }
                            ErasePlayerFromList(playerGuid, &m_queuedPlayers[_bgType][_levelGroup]);
                        }
                    }
                    else
                    {
                        battleground = iitr->second;
                        int size = static_cast<int>(std::min(tempPlayerVec[0].size(), tempPlayerVec[1].size()));
                        for (int counter = 0; counter < size && battleground->isFull() == false; counter++)
                        {
                            addPlayerToBgTeam(battleground, &tempPlayerVec[0], _bgType, _levelGroup, 0);
                            addPlayerToBgTeam(battleground, &tempPlayerVec[1], _bgType, _levelGroup, 1);
                        }

                        while (!tempPlayerVec[0].empty() && battleground->hasFreeSlots(0, battleground->getType()))
                            addPlayerToBgTeam(battleground, &tempPlayerVec[0], _bgType, _levelGroup, 0);

                        while (!tempPlayerVec[1].empty() && battleground->hasFreeSlots(1, battleground->getType()))
                            addPlayerToBgTeam(battleground, &tempPlayerVec[1], _bgType, _levelGroup, 1);
                    }
                }
            }

            // Now that that we added everyone we could to a running Bg/Arena
            // We shall see if we can start a new one!
            if (Battleground::isTypeArena(_bgType))
            {
                // enough players to start a round?
                uint32_t minPlayers = sBattlegroundManager.getMinimumPlayers(_bgType);
                if (!forceStart && tempPlayerVec[0].size() + tempPlayerVec[1].size() < minPlayers * 2)
                    continue;

                if (canCreateInstance(_bgType, _levelGroup))
                {
                    arena = dynamic_cast<Arena*>(createInstance(_bgType, _levelGroup));
                    if (arena == nullptr)
                    {
                        sLogger.failure("{} ({}): Couldn't create Arena Instance", __FILE__, __LINE__);
                        return;
                    } // No alliance in the queue

                    if (tempPlayerVec[0].empty())
                    {
                        count = getMaximumPlayers(_bgType) * 2;
                        while (count > 0 && !tempPlayerVec[1].empty())
                        {
                            if (teams[0].size() > teams[1].size())
                                teams[1].push(tempPlayerVec[1].front());
                            else
                                teams[0].push(tempPlayerVec[1].front());

                            tempPlayerVec[1].pop_front();
                            count--;
                        }
                    }
                    else // No horde in the queue
                    {
                        if (tempPlayerVec[1].empty())
                        {
                            count = getMaximumPlayers(_bgType) * 2;
                            while (count > 0 && !tempPlayerVec[0].empty())
                            {
                                if (teams[0].size() > teams[1].size())
                                    teams[1].push(tempPlayerVec[0].front());
                                else
                                    teams[0].push(tempPlayerVec[0].front());

                                tempPlayerVec[0].pop_front();
                                count--;
                            }
                        }
                        else // There are both alliance and horde players in the queue
                        {
                            count = getMaximumPlayers(_bgType);
                            while (count > 0 && !tempPlayerVec[0].empty() && !tempPlayerVec[1].empty())
                            {
                                teams[0].push(tempPlayerVec[0].front());
                                teams[1].push(tempPlayerVec[1].front());
                                tempPlayerVec[0].pop_front();
                                tempPlayerVec[1].pop_front();
                                count--;
                            }
                        }
                    }

                    // Now we just need to add the players to the Arena instance
                    while (!teams[0].empty())
                    {
                        for (uint32_t localeTeam = 0; localeTeam < 2; localeTeam++)
                        {
                            playerGuid = teams[localeTeam].front();
                            teams[localeTeam].pop();

                            player = sObjectMgr.getPlayer(playerGuid);
                            if (player == nullptr)
                                continue;

                            player->setBgTeam(localeTeam);
                            arena->addPlayer(player, player->getBgTeam());
                            // remove from the main queue (painful!)
                            ErasePlayerFromList(player->getGuidLow(), &m_queuedPlayers[_bgType][_levelGroup]);
                        }
                    }
                }
            }
            else
            {
                uint32_t bgToStart = _bgType;
                if (_bgType == BattlegroundDef::TYPE_RANDOM)
                {
                    if (!forceStart)
                    {
                        std::vector<uint32_t> bgPossible;
                        for (unsigned int tmpJoinBgType : m_avalibleInRandom)
                        {
                            uint32_t minPlayers = sBattlegroundManager.getMinimumPlayers(tmpJoinBgType);
                            if (tempPlayerVec[0].size() >= minPlayers && tempPlayerVec[1].size() >= minPlayers)
                                bgPossible.push_back(tmpJoinBgType);
                        }

                        if (!bgPossible.empty())
                        {
                            uint32_t num = Util::getRandomUInt(0, static_cast<uint32_t>(bgPossible.size() - 1));
                            bgToStart = bgPossible[num];
                        }
                    }
                    else
                    {
                        uint32_t num = Util::getRandomUInt(0, static_cast<uint32_t>(m_avalibleInRandom.size() - 1));
                        bgToStart = m_avalibleInRandom[num];
                    }
                }

                uint32_t minPlayers = sBattlegroundManager.getMinimumPlayers(bgToStart);
                if (forceStart || tempPlayerVec[0].size() >= minPlayers && tempPlayerVec[1].size() >= minPlayers && bgToStart != BattlegroundDef::TYPE_RANDOM)
                {
                    if (canCreateInstance(bgToStart, _levelGroup))
                    {
                        battleground = createInstance(bgToStart, _levelGroup);
                        if (battleground == nullptr)
                            return;

                        // push as many as possible in
                        if (forceStart)
                        {
                            for (uint8_t k = 0; k < 2; ++k)
                            {
                                while (!tempPlayerVec[k].empty() && battleground->hasFreeSlots(k, battleground->getType()))
                                    addPlayerToBgTeam(battleground, &tempPlayerVec[k], _bgType, _levelGroup, k);
                            }
                        }
                        else
                        {
                            int size = static_cast<int>(std::min(tempPlayerVec[0].size(), tempPlayerVec[1].size()));
                            for (int counter = 0; counter < size && battleground->isFull() == false; counter++)
                            {
                                addPlayerToBgTeam(battleground, &tempPlayerVec[0], _bgType, _levelGroup, 0);
                                addPlayerToBgTeam(battleground, &tempPlayerVec[1], _bgType, _levelGroup, 1);
                            }
                        }
                    }
                }
            }
        }
    }

    // Handle paired arena team joining
    Group* group1, *group2;
    uint32_t teamids[2] = { 0, 0 };
    uint32_t avgRating[2] = { 0, 0 };
    uint32_t n;
    std::list<uint32_t>::iterator itz;
    for (uint8_t i = BattlegroundDef::TYPE_ARENA_2V2; i <= BattlegroundDef::TYPE_ARENA_5V5; ++i)
    {
        if (!forceStart && m_queuedGroups[i].size() < 2) // got enough to have an arena battle ;P
            continue;

        for (uint32_t j = 0; j < static_cast<uint32_t>(m_queuedGroups[i].size()); j++)
        {
            group1 = group2 = nullptr;
            n = Util::getRandomUInt(static_cast<uint32_t>(m_queuedGroups[i].size())) - 1;
            for (itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end() && n > 0; ++itz)
                --n;

            if (itz == m_queuedGroups[i].end())
                itz = m_queuedGroups[i].begin();

            if (itz == m_queuedGroups[i].end())
            {
                sLogger.failure("Internal error at {}:{}", __FILE__, __LINE__);
                return;
            }

            group1 = sObjectMgr.getGroupById(*itz);
            if (group1 == nullptr)
                continue;

            if (forceStart && m_queuedGroups[i].size() == 1)
            {
                if (createArenaType(i, group1, nullptr) == -1)
                    return;

                m_queuedGroups[i].remove(group1->GetID());
                continue;
            }

            teamids[0] = getArenaGroupQInfo(group1, i, &avgRating[0]);

            std::list<uint32_t> possibleGroups;
            for (itz = m_queuedGroups[i].begin(); itz != m_queuedGroups[i].end(); ++itz)
            {
                group2 = sObjectMgr.getGroupById(*itz);
                if (group2)
                {
                    teamids[1] = getArenaGroupQInfo(group2, i, &avgRating[1]);
                    uint32_t delta = abs(static_cast<int32_t>(avgRating[0]) - static_cast<int32_t>(avgRating[1]));
                    if (teamids[0] != teamids[1] && delta <= worldConfig.rate.arenaQueueDiff)
                        possibleGroups.push_back(group2->GetID());
                }
            }

            if (!possibleGroups.empty())
            {
                n = Util::getRandomUInt(static_cast<uint32_t>(possibleGroups.size())) - 1;
                for (itz = possibleGroups.begin(); itz != possibleGroups.end() && n > 0; ++itz)
                    --n;

                if (itz == possibleGroups.end())
                    itz = possibleGroups.begin();

                if (itz == possibleGroups.end())
                {
                    sLogger.failure("Internal error at {}:{}", __FILE__, __LINE__);
                    return;
                }

                group2 = sObjectMgr.getGroupById(*itz);
                if (group2)
                {
                    if (createArenaType(i, group1, group2) == -1) return;
                    m_queuedGroups[i].remove(group1->GetID());
                    m_queuedGroups[i].remove(group2->GetID());
                }
            }
        }
    }
}

void BattlegroundManager::removePlayerFromQueues(Player* player)
{
    if (player->getBgQueueType() >= BATTLEGROUND_NUM_TYPES)
    {
        sLogger.failure("BattlegroundManager::removePlayerFromQueues queueType {} is not valid!", BATTLEGROUND_NUM_TYPES);
        return;
    }

    std::lock_guard queueLock(m_queueLock);

    sEventMgr.RemoveEvents(player, EVENT_BATTLEGROUND_QUEUE_UPDATE);

    uint32_t lgroup = player->getLevelGrouping();

    std::list<uint32_t>::iterator itr = m_queuedPlayers[player->getBgQueueType()][lgroup].begin();
    while (itr != m_queuedPlayers[player->getBgQueueType()][lgroup].end())
    {
        if (*itr == player->getGuidLow())
        {
            sLogger.debug("Removing player {} from queue instance {} type {}", player->getGuidLow(), player->getQueuedBgInstanceId(), player->getBgQueueType());
            m_queuedPlayers[player->getBgQueueType()][lgroup].erase(itr);
            break;
        }

        ++itr;
    }

    player->setIsQueuedForBg(false);
    player->setBgTeam(player->getTeam());
    player->setPendingBattleground(nullptr);

    sendBattlefieldStatus(player, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);

    if (auto group = player->getGroup())
    {
        sLogger.debug("Player {} removed whilst in a group. Removing players group {} from queue", player->getGuidLow(), group->GetID());
        removeGroupFromQueues(group->GetID());
    }
}

void BattlegroundManager::removeGroupFromQueues(uint32_t groupId)
{
    if (auto group = sObjectMgr.getGroupById(groupId))
    {
        std::lock_guard queueLock(m_queueLock);
        for (uint32_t i = BattlegroundDef::TYPE_ARENA_2V2; i < BattlegroundDef::TYPE_ARENA_5V5 + 1; ++i)
        {
            for (std::list<uint32_t>::iterator itr = m_queuedGroups[i].begin(); itr != m_queuedGroups[i].end();)
            {
                if (*itr == groupId)
                    itr = m_queuedGroups[i].erase(itr);
                else
                    ++itr;
            }
        }

        for (const auto itr : group->GetSubGroup(0)->getGroupMembers())
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itr->guid))
                sendBattlefieldStatus(loggedInPlayer, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
    }
}

bool BattlegroundManager::canCreateInstance(uint32_t /*type*/, uint32_t /*levelGroup*/)
{
    return true;
}

// Returns the minimum number of players (Only valid for battlegrounds)
uint32_t BattlegroundManager::getMinimumPlayers(uint32_t dbcIndex)
{
    switch (dbcIndex)
    {
        case BattlegroundDef::TYPE_ALTERAC_VALLEY:
            return worldConfig.bg.minPlayerCountAlteracValley;
        case BattlegroundDef::TYPE_WARSONG_GULCH:
            return worldConfig.bg.minPlayerCountWarsongGulch;
        case BattlegroundDef::TYPE_ARATHI_BASIN:
            return worldConfig.bg.minPlayerCountArathiBasin;
        case BattlegroundDef::TYPE_EYE_OF_THE_STORM:
            return worldConfig.bg.minPlayerCountEyeOfTheStorm;
        case BattlegroundDef::TYPE_ARENA_2V2:
            return worldConfig.arena.minPlayerCount2V2;
        case BattlegroundDef::TYPE_ARENA_3V3:
            return worldConfig.arena.minPlayerCount3V3;
        case BattlegroundDef::TYPE_ARENA_5V5:
            return worldConfig.arena.minPlayerCount5V5;
        case BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT:
            return worldConfig.bg.minPlayerCountStrandOfTheAncients;
        case BattlegroundDef::TYPE_ISLE_OF_CONQUEST:
            return worldConfig.bg.minPlayerCountIsleOfConquest;
        default:
            return 1;
    }
}

// Returns the maximum number of players (Only valid for battlegrounds)
uint32_t BattlegroundManager::getMaximumPlayers(uint32_t dbcIndex)
{
    switch (dbcIndex)
    {
        case BattlegroundDef::TYPE_ALTERAC_VALLEY:
            return worldConfig.bg.maxPlayerCountAlteracValley;
        case BattlegroundDef::TYPE_WARSONG_GULCH:
            return worldConfig.bg.maxPlayerCountWarsongGulch;
        case BattlegroundDef::TYPE_ARATHI_BASIN:
            return worldConfig.bg.maxPlayerCountArathiBasin;
        case BattlegroundDef::TYPE_EYE_OF_THE_STORM:
            return worldConfig.bg.maxPlayerCountEyeOfTheStorm;
        case BattlegroundDef::TYPE_ARENA_2V2:
            return worldConfig.arena.maxPlayerCount2V2;
        case BattlegroundDef::TYPE_ARENA_3V3:
            return worldConfig.arena.maxPlayerCount3V3;
        case BattlegroundDef::TYPE_ARENA_5V5:
            return worldConfig.arena.maxPlayerCount5V5;
        case BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT:
            return worldConfig.bg.maxPlayerCountStrandOfTheAncients;
        case BattlegroundDef::TYPE_ISLE_OF_CONQUEST:
            return worldConfig.bg.maxPlayerCountIsleOfConquest;
        default:
            return 1;
    }
}

Battleground* BattlegroundManager::createInstance(uint32_t type, uint32_t levelGroup)
{
    if (!m_bgMaps.contains(type))
    {
        if (!Battleground::isTypeArena(type))
        {
            sLogger.failure("BattlegroundManager", "No map Id is registered for Battleground type {}", type);
            return nullptr;
        }
    }

    BattlegroundFactoryMethod cfunc = nullptr;

    if (!Battleground::isTypeArena(type))
        if (m_bgFactories.contains(m_bgMaps[type]))
            cfunc = m_bgFactories[m_bgMaps[type]];

    BattlegroundMap* mgr = nullptr;
    Battleground* bg;
    bool isWeekend = false;
    struct tm tm;
    uint32_t iid;
    time_t t;
    int n;

    if (Battleground::isTypeArena(type))
    {
        // arenas follow a different procedure.
        const auto arenaMapCount = static_cast<uint32_t>(m_arenaMaps.size());
        if (arenaMapCount == 0)
        {
            sLogger.failure("BattlegroundManager", "There are no Arenas registered. Cannot create Arena.");
            return nullptr;
        }

        uint32_t index = Util::getRandomUInt(arenaMapCount - 1);
        uint32_t mapid = m_arenaMaps[index];
        const ArenaFactoryMethod arenaFactory = m_arenaFactories[index];

        mgr = sMapMgr.createBattleground(mapid);
        if (mgr == nullptr)
        {
            sLogger.failure("call failed for map {}, type {}, level group {}", mapid, type, levelGroup);
            return nullptr; // Shouldn't happen
        }

        const uint32_t players_per_side = getMaximumPlayers(type);

        iid = ++m_maxBattlegroundId[type];
        bg = arenaFactory(mgr, iid, levelGroup, type, players_per_side);
        mgr->setBattleground(bg);
        sLogger.info("BattlegroundManager : Created arena battleground type {} for level group {} on map {}.", type, levelGroup, mapid);
        sEventMgr.AddEvent(bg, &Battleground::eventCreate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 1, 1, 0);
        std::lock_guard instanceLock(m_instanceLock);
        m_instances[type].insert(std::make_pair(iid, bg));
        
        return bg;
    }

    if (cfunc == nullptr)
    {
        sLogger.failure("Could not find CreateBattlegroundFunc pointer for type {} level group {}", type, levelGroup);
        return nullptr;
    }

    t = time(nullptr);
#ifdef WIN32
    // localtime_s(&tm, &t);
    //zack : some luv for vs2k3 compiler
    tm = *localtime(&t);
#else
    localtime_r(&t, &tm);
#endif

    switch (type)
    {
        case BattlegroundDef::TYPE_WARSONG_GULCH:
            n = 0;
            break;
        case BattlegroundDef::TYPE_ARATHI_BASIN:
            n = 1;
            break;
        case BattlegroundDef::TYPE_EYE_OF_THE_STORM:
            n = 2;
            break;
        case BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT:
            n = 3;
            break;
        case BattlegroundDef::TYPE_ISLE_OF_CONQUEST:
            n = 4;
            break;
        default:
            n = 0;
            break;
    }
    if (tm.tm_yday / 7 % 4 == n)
    {
        // Set weekend from Thursday night at midnight until Tuesday morning
        isWeekend = tm.tm_wday >= 5 || tm.tm_wday < 2;
    }

    // Create Map Manager
    mgr = sMapMgr.createBattleground(m_bgMaps[type]);
    if (mgr == nullptr)
    {
        sLogger.failure("call failed for map {}, type {}, level group {}", m_bgMaps[type], type, levelGroup);
        return nullptr; // Shouldn't happen
    }

    // Call the create function
    iid = ++m_maxBattlegroundId[type];
    bg = cfunc(mgr, iid, levelGroup, type);
    bg->SetIsWeekend(isWeekend);
    mgr->setBattleground(bg);

    sEventMgr.AddEvent(bg, &Battleground::eventCreate, EVENT_BATTLEGROUND_QUEUE_UPDATE, 1, 1, 0);
    sLogger.info("BattlegroundManager : Created battleground type {} for level group {}.", type, levelGroup);

    std::lock_guard instanceLock(m_instanceLock);
    m_instances[type].insert(std::make_pair(iid, bg));

    return bg;
}

void BattlegroundManager::deleteBattleground(Battleground* battleground)
{
    uint32_t type = battleground->getType();
    uint32_t levelGroup = battleground->getLevelGroup();

    std::lock_guard instanceLock(m_instanceLock);
    std::lock_guard queueLock(m_queueLock);

    m_instances[type].erase(battleground->getId());

    // erase any queued players
    std::list<uint32_t>::iterator itr = m_queuedPlayers[type][levelGroup].begin();
    for (; itr != m_queuedPlayers[type][levelGroup].end();)
    {
        std::list<uint32_t>::iterator it2 = itr++;

        if (Player* plr = sObjectMgr.getPlayer(*it2))
        {
            if (plr->getQueuedBgInstanceId() == battleground->getId())
            {
                plr->getSession()->systemMessage(plr->getSession()->LocalizedWorldSrv(SS_QUEUE_BG_INSTANCE_ID_NO_VALID_LONGER_EXISTS), battleground->getId());
                sendBattlefieldStatus(plr, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
                plr->setIsQueuedForBg(false);
                m_queuedPlayers[type][levelGroup].erase(it2);
            }
        }
        else
        {
            m_queuedPlayers[type][levelGroup].erase(it2);
        }
    }

    //sLogger.info("Deleting battleground from queue {}, instance {}", bg->GetType(), bg->GetId());
    delete battleground;
}

void BattlegroundManager::sendBattlefieldStatus(Player* player, BattlegroundDef::Status status, uint32_t type, uint32_t instanceId, uint32_t time, uint32_t mapId, uint8_t ratedMatch)
{
    player->sendPacket(SmsgBattlefieldStatus(player->GetNewGUID(), status, type, instanceId, time, mapId, ratedMatch, Battleground::isTypeArena(type)).serialise().get());
}

void BattlegroundManager::handleArenaJoin(WorldSession* session, uint32_t battlegroundType, uint8_t asGroup, uint8_t ratedMatch)
{
    uint32_t pguid = session->GetPlayer()->getGuidLow();
    uint32_t lgroup = session->GetPlayer()->getLevelGrouping();

    if (asGroup && session->GetPlayer()->getGroup() == nullptr)
        return;

    const auto group = session->GetPlayer()->getGroup();
    if (asGroup)
    {
        if (group->GetSubGroupCount() != 1)
        {
            session->SystemMessage(session->LocalizedWorldSrv(SS_SORRY_RAID_GROUPS_JOINING_BG_ARE_UNSUPPORTED));
            return;
        }
        if (group->GetLeader() != session->GetPlayer()->getPlayerInfo())
        {
            session->SystemMessage(session->LocalizedWorldSrv(SS_MUST_BE_PARTY_LEADER_TO_ADD_GROUP_AN_ARENA));
            return;
        }

        if (!ratedMatch)
        {
            // add all players normally.. bleh ;P
            group->Lock();
            for (const auto itx : group->GetSubGroup(0)->getGroupMembers())
            {
                if (Player* loggedInPlayer = sObjectMgr.getPlayer(itx->guid))
                    if (!loggedInPlayer->isQueuedForBg() && !loggedInPlayer->getBattleground())
                        handleArenaJoin(loggedInPlayer->getSession(), battlegroundType, 0, 0);
            }

            group->Unlock();
            return;
        }

        if (battlegroundType > BattlegroundDef::TYPE_ARENA_5V5)
            return;

        // make sure all players are 70
        uint32_t maxplayers;
        const auto type = static_cast<uint8_t>(battlegroundType - BattlegroundDef::TYPE_ARENA_2V2);
        switch (battlegroundType)
        {
            case BattlegroundDef::TYPE_ARENA_3V3:
                maxplayers = 3;
                break;

            case BattlegroundDef::TYPE_ARENA_5V5:
                maxplayers = 5;
                break;

            case BattlegroundDef::TYPE_ARENA_2V2:
            default:
                maxplayers = 2;
                break;
        }

        Player* loggedInLeader = sObjectMgr.getPlayer(group->GetLeader()->guid);
        if (loggedInLeader && loggedInLeader->getArenaTeam(type) == nullptr)
        {
            session->SendPacket(SmsgArenaError(0, static_cast<uint8_t>(maxplayers)).serialise().get());
            return;
        }

        group->Lock();
        for (const auto itx : group->GetSubGroup(0)->getGroupMembers())
        {
            if (maxplayers == 0)
            {
                session->SystemMessage(session->LocalizedWorldSrv(SS_TOO_MANY_PLAYERS_PARTY_TO_JOIN_OF_ARENA));
                group->Unlock();
                return;
            }

            if (itx->lastLevel < PLAYER_ARENA_MIN_LEVEL)
            {
                session->SystemMessage(session->LocalizedWorldSrv(SS_SORRY_SOME_OF_PARTY_MEMBERS_ARE_NOT_LVL_70));
                group->Unlock();
                return;
            }

            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itx->guid))
            {
                if (loggedInPlayer->getBattleground() || loggedInPlayer->isQueuedForBg())
                {
                    session->SystemMessage(session->LocalizedWorldSrv(
                        SS_ONE_OR_MORE_OF_PARTY_MEMBERS_ARE_ALREADY_QUEUED_OR_INSIDE_BG));
                    group->Unlock();
                    return;
                }

                if (loggedInPlayer->getArenaTeam(type) != loggedInLeader->getArenaTeam(type))
                {
                    session->SystemMessage(session->LocalizedWorldSrv(
                        SS_ONE_OR_MORE_OF_YOUR_PARTY_MEMBERS_ARE_NOT_MEMBERS_OF_YOUR_TEAM));
                    group->Unlock();
                    return;
                }

                --maxplayers;
            }
        }

        for (const auto itx : group->GetSubGroup(0)->getGroupMembers())
        {
            if (Player* loggedInPlayer = sObjectMgr.getPlayer(itx->guid))
            {
                sendBattlefieldStatus(loggedInPlayer, BattlegroundDef::STATUS_INQUEUE, battlegroundType, 0, 0, 0, 1);
                loggedInPlayer->setIsQueuedForBg(true);
                loggedInPlayer->setQueuedBgInstanceId(0);
                loggedInPlayer->setBgQueueType(battlegroundType);
                //\todo error/bgtype missing, always send all arenas (from legacy)
                loggedInPlayer->getSession()->SendPacket(SmsgGroupJoinedBattleground(6).serialise().get());
                loggedInPlayer->setBGEntryPoint(loggedInPlayer->GetPositionX(), loggedInPlayer->GetPositionY(), loggedInPlayer->GetPositionZ(),
                    loggedInPlayer->GetOrientation(), loggedInPlayer->GetMapId(), loggedInPlayer->GetInstanceID());
            }
        }

        group->Unlock();

        std::lock_guard queueLock(m_queueLock);
        m_queuedGroups[battlegroundType].push_back(group->GetID());
        sLogger.info("BattlegroundMgr : Group {} is now in battleground queue for arena type {}", group->GetID(), battlegroundType);

        // send the battleground status packet

        return;
    }

    // Queue him!
    std::lock_guard queueLock(m_queueLock);
    m_queuedPlayers[battlegroundType][lgroup].push_back(pguid);

    sLogger.info("BattlegroundMgr : Player {} is now in battleground queue for (Arena {})", session->GetPlayer()->getGuidLow(), battlegroundType);

    // send the battleground status packet
    sendBattlefieldStatus(session->GetPlayer(), BattlegroundDef::STATUS_INQUEUE, battlegroundType, 0, 0, 0, 0);
    session->GetPlayer()->setIsQueuedForBg(true);
    session->GetPlayer()->setQueuedBgInstanceId(0);
    session->GetPlayer()->setBgQueueType(battlegroundType);

    session->GetPlayer()->setBGEntryPoint(session->GetPlayer()->GetPositionX(), session->GetPlayer()->GetPositionY(), session->GetPlayer()->GetPositionZ(), session->GetPlayer()->GetOrientation(),
        session->GetPlayer()->GetMapId(), session->GetPlayer()->GetInstanceID());
}
