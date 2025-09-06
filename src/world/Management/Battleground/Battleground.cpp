/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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

#include "Management/Battleground/Battleground.hpp"
#include "Objects/Units/Players/PlayerDefines.hpp"
#include "Management/HonorHandler.h"
#include "BattlegroundMgr.hpp"
#include "Management/Arenas.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Spell/Definitions/AuraInterruptFlags.hpp"
#include "Logging/Logger.hpp"
#include "Management/Group.h"
#include "Management/ObjectMgr.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/GameObject.h"
#include "Objects/Units/Creatures/Creature.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/EventMgr.h"
#include "Server/WorldSession.h"
#include "Spell/Definitions/PowerType.hpp"
#include "Server/Packets/SmsgPlaySound.h"
#include "Server/Packets/SmsgBattlegroundPlayerLeft.h"
#include "Server/Packets/SmsgBattlegroundPlayerJoined.h"
#include "Server/Packets/SmsgMessageChat.h"
#include "Storage/WorldStrings.h"
#include <cstdarg>

Battleground::Battleground(WorldMap* worldMap, uint32_t id, uint32_t levelGroup, uint32_t type) : m_mapMgr(worldMap), m_id(id), m_type(type), m_levelGroup(levelGroup)
{
    sEventMgr.AddEvent(this, &Battleground::eventResurrectPlayers, EVENT_BATTLEGROUND_QUEUE_UPDATE, 30000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);

    for (auto& group : m_groups)
    {
        group = sObjectMgr.createGroup();
        group->m_disbandOnNoMembers = false;
        group->ExpandToRaid();
    }

    m_honorPerKill = HonorHandler::CalculateHonorPointsForKill(m_levelGroup * 10, m_levelGroup * 10);
}

uint32_t Battleground::getId()
{
    return m_id;
}

uint32_t Battleground::getLevelGroup()
{
    return m_levelGroup;
}

WorldMap* Battleground::getWorldMap()
{
    return m_mapMgr;
}

Battleground::~Battleground()
{
    sEventMgr.RemoveEvents(this);
    for (auto& m_group : m_groups)
    {
        m_group->Disband();
        m_group = nullptr;
    }

    m_resurrectMap.clear();
    m_players[0].clear();
    m_players[1].clear();
}

void Battleground::updatePvPData()
{
    if (isTypeArena(m_type))
        if (!m_hasEnded)
            return;

    if (UNIXTIME >= m_nextPvPUpdateTime)
    {
        std::lock_guard lock(m_mutex);

        WorldPacket data(10 * (m_players[0].size() + m_players[1].size()) + 50);
        buildPvPUpdateDataPacket(&data);
        distributePacketToAll(&data);

        m_nextPvPUpdateTime = UNIXTIME + 2;
    }
}

uint32_t Battleground::getStartTime()
{
    return m_startTime;
}

uint32_t Battleground::getType()
{
    return m_type;
}

void Battleground::buildPvPUpdateDataPacket(WorldPacket* data)
{
    data->Initialize(MSG_PVP_LOG_DATA);
    data->reserve(10 * (m_players[0].size() + m_players[1].size()) + 50);

    BGScore* bs;
    if (isTypeArena(m_type))
    {
        if (!m_hasEnded)
            return;

        *data << uint8_t(1);

        if (!Rated())
        {
            *data << uint32_t(0); //uint32_t(negative rating)
            *data << uint32_t(0); //uint32_t(positive rating)
            *data << uint32_t(0); //uint32_t(0)[<-this is the new field in 3.1]
            *data << uint8_t(0);  //name if available / which is a null-terminated string, and we send an uint8_t(0), so we provide a zero length name string
            *data << uint32_t(0);
            *data << uint32_t(0);
            *data << uint32_t(0);
            *data << uint8_t(0);
        }
        else
        {
            /* Grab some arena teams */
            auto** teams = dynamic_cast< Arena* >(this)->GetTeams();

            if (teams[0])
            {
                *data << uint32_t(0);
                *data << uint32_t(3000 + m_deltaRating[0]);
                *data << uint32_t(0);
                *data << uint8_t(0);
            }
            else
            {
                *data << uint32_t(0);
                *data << uint32_t(0);
                *data << uint32_t(0);
                *data << uint8_t(0);
            }

            if (teams[1])
            {
                *data << uint32_t(0);
                *data << uint32_t(3000 + m_deltaRating[1]);
                *data << uint32_t(0);
                *data << uint8_t(0);
            }
            else
            {
                *data << uint32_t(0);
                *data << uint32_t(0);
                *data << uint32_t(0);
                *data << uint8_t(0);
            }
        }

        *data << uint8_t(1);
        *data << uint8_t(m_winningTeam);

        *data << uint32_t((m_players[0].size() + m_players[1].size()) - m_invisGMs);

        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
            {
                if (itr->m_isGmInvisible)
                    continue;

                *data << itr->getGuid();
                bs = &itr->m_bgScore;
                *data << bs->KillingBlows;

                *data << uint8_t(itr->getBgTeam());

                *data << bs->DamageDone;
                *data << bs->HealingDone;
                *data << uint32_t(0);
            }
        }
    }
    else
    {
        *data << uint8_t(0);
        if (m_hasEnded)
        {
            *data << uint8_t(1);
            *data << uint8_t(m_winningTeam ? 0 : 1);
        }
        else
        {
            *data << uint8_t(0);      // If the game has ended - this will be 1
        }

        *data << uint32_t((m_players[0].size() + m_players[1].size()) - m_invisGMs);

        const uint32_t FieldCount = getFieldCount(getType());
        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
            {
                if (itr != nullptr)
                {
                    if (itr->m_isGmInvisible)
                        continue;

                    *data << itr->getGuid();
                    bs = &itr->m_bgScore;

                    *data << bs->KillingBlows;
                    *data << bs->HonorableKills;
                    *data << bs->Deaths;
                    *data << bs->BonusHonor;
                    *data << bs->DamageDone;
                    *data << bs->HealingDone;

                    *data << FieldCount;
                    for (uint32_t x = 0; x < FieldCount; ++x)
                        *data << bs->MiscData[x];
                }
            }
        }
    }
}

uint8_t Battleground::Rated()
{
    return 0;
}

void Battleground::addPlayer(Player* plr, uint32_t team)
{
    std::lock_guard lock(m_mutex);

    // This is called when the player is added, not when they port. So, they're essentially still queued, but not inside the bg yet
    m_pendPlayers[team].insert(plr->getGuidLow());

    // Send a packet telling them that they can enter
    plr->setPendingBattleground(this);
    sBattlegroundManager.sendBattlefieldStatus(plr, BattlegroundDef::STATUS_READY, m_type, m_id, 80000, m_mapMgr->getBaseMap()->getMapId(), Rated());        // You will be removed from the queue in 2 minutes.

    // Add an event to remove them in 1 minute 20 seconds time
    sEventMgr.AddEvent(plr, &Player::removeFromBgQueue, EVENT_BATTLEGROUND_QUEUE_UPDATE, 80000, 1, 0);
}

void Battleground::removePendingPlayer(Player* plr)
{
    std::lock_guard lock(m_mutex);

    m_pendPlayers[plr->getBgTeam()].erase(plr->getGuidLow());

    // send a null bg update (so they don't join)
    sBattlegroundManager.sendBattlefieldStatus(plr, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
    plr->setPendingBattleground(nullptr);
    plr->setBgTeam(plr->getTeam());
}

void Battleground::onPlayerPushed(Player* plr)
{
    if (plr->getGroup() && !Rated())
        plr->getGroup()->RemovePlayer(plr->getPlayerInfo());

    plr->processPendingUpdates();

    if (plr->getGroup() == nullptr)
        if (plr->m_isGmInvisible == false)    //do not join invisible gm's into bg groups.
            m_groups[plr->getBgTeam()]->AddMember(plr->getPlayerInfo());
}

void Battleground::SetIsWeekend(bool /*isweekend*/)
{
}

void Battleground::portPlayer(Player* plr, bool skip_teleport)
{
    std::lock_guard lock(m_mutex);

    if (m_hasEnded)
    {
        plr->getSession()->systemMessage(plr->getSession()->LocalizedWorldSrv(ServerString::SS_YOU_CANNOT_JOIN_BG_AS_IT_HAS_ALREADY_ENDED));
        sBattlegroundManager.sendBattlefieldStatus(plr, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
        plr->setPendingBattleground(nullptr);
        return;
    }

    m_pendPlayers[plr->getBgTeam()].erase(plr->getGuidLow());
    if (m_players[plr->getBgTeam()].find(plr) != m_players[plr->getBgTeam()].end())
        return;

    plr->setFullHealthMana();
    plr->setTeam(plr->getBgTeam());

    // Do not let everyone know an invisible gm has joined.
    if (plr->m_isGmInvisible == false)
        distributePacketToTeam(AscEmu::Packets::SmsgBattlegroundPlayerJoined(plr->getGuid()).serialise().get(), plr->getBgTeam());
    else
        ++m_invisGMs;

    m_players[plr->getBgTeam()].insert(plr);

    // remove from any auto queue remove events
    sEventMgr.RemoveEvents(plr, EVENT_BATTLEGROUND_QUEUE_UPDATE);

    if (!skip_teleport)
        if (plr->IsInWorld())
            plr->removeFromWorld();

    plr->setPendingBattleground(nullptr);
    plr->setBattleground(this);
    plr->setLastBattlegroundPetId(0);
    plr->setLastBattlegroundPetSpell(0);

    if (!plr->isPvpFlagSet())
        plr->setPvpFlag();

    plr->removeAllAurasByAuraInterruptFlag(AURA_INTERRUPT_ON_PVP_ENTER);

    // Reset the score
    memset(&plr->m_bgScore, 0, sizeof(BGScore));

    // update pvp data
    updatePvPData();

    // add the player to the group
    if (plr->getGroup() && !Rated())
    {
        // remove them from their group
        plr->getGroup()->RemovePlayer(plr->getPlayerInfo());
    }

    if (!m_countdownStage)
    {
        m_countdownStage = 1;
        sEventMgr.AddEvent(this, &Battleground::eventCountdown, EVENT_BATTLEGROUND_COUNTDOWN, 30000, 0, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        sEventMgr.ModifyEventTimeLeft(this, EVENT_BATTLEGROUND_COUNTDOWN, 10000);
    }

    sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);

    if (!skip_teleport)
    {
        // This is where we actually teleport the player to the battleground
        plr->safeTeleport(m_mapMgr, GetStartingCoords(plr->getBgTeam()));
        sBattlegroundManager.sendBattlefieldStatus(plr, BattlegroundDef::STATUS_TIME, m_type, m_id, static_cast<uint32_t>(UNIXTIME) - m_startTime, m_mapMgr->getBaseMap()->getMapId(), Rated());     // Elapsed time is the last argument
    }
    else
    {
        // If we are not ported, call this immediatelly, otherwise its called after teleportation in Player::OnPushToWorld
        OnAddPlayer(plr);
    }
}

GameObject* Battleground::spawnGameObject(uint32_t entry, LocationVector const& v, uint32_t flags, uint32_t faction, float scale)
{
    if (GameObject* go = m_mapMgr->createGameObject(entry))
    {
        go->create(entry, m_mapMgr, 0, v, QuaternionData(), GO_STATE_CLOSED);

        go->SetFaction(faction);
        go->setScale(scale);
        go->setFlags(flags);
        go->SetPosition(v);
        go->SetInstanceID(m_mapMgr->getInstanceId());

        return go;
    }

    return nullptr;
}

Creature* Battleground::spawnCreature(uint32_t entry, float x, float y, float z, float o, uint32_t faction)
{
    CreatureProperties const* cp = sMySQLStore.getCreatureProperties(entry);
    if (cp == nullptr)
    {
        sLogger.failure("tried to push a invalid creature with entry {}!", entry);
        return nullptr;
    }

    if (Creature* c = m_mapMgr->createCreature(entry))
    {
        c->Load(cp, x, y, z, o);

        if (faction != 0)
            c->setFaction(faction);

        c->PushToWorld(m_mapMgr);
        return c;
    }

    return nullptr;
}

Creature* Battleground::spawnCreature(uint32_t entry, LocationVector &v, uint32_t faction)
{
    return spawnCreature(entry, v.x, v.y, v.z, v.o, faction);
}

void Battleground::addInvisGM()
{
    ++m_invisGMs;
}

void Battleground::removeInvisGM()
{
    --m_invisGMs;
}

std::recursive_mutex& Battleground::GetMutex()
{
    return m_mutex;
}

void Battleground::startBattleground()
{
    this->OnStart();
}

//todo Move reward calculations to seperate functions
void Battleground::endBattleground(PlayerTeam winningTeam)
{
    std::lock_guard lock(m_mutex);

    this->m_hasEnded = true;
    this->m_winningTeam = winningTeam;
    this->m_nextPvPUpdateTime = 0;

    const auto losingTeam = winningTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE;

    /* If we still need to handle reward calculations */
    if (this->HandleFinishBattlegroundRewardCalculation(winningTeam))
    {
        for (const auto winningPlr : m_players[winningTeam])
        {
            if (winningPlr->isQueuedForRbg())
            {
                winningPlr->applyRandomBattlegroundReward(true);
                winningPlr->setHasWonRbgToday(true);
            }

            winningPlr->saveToDB(false);
        }
        for (const auto losingPlr : m_players[losingTeam])
        {
            if (losingPlr->isQueuedForRbg())
            {
                losingPlr->applyRandomBattlegroundReward(false);
            }

            losingPlr->saveToDB(false);
        }

        this->addHonorToTeam(winningTeam, 3 * 185);
        this->addHonorToTeam(losingTeam, 1 * 185);
    }

    this->playSoundToAll(winningTeam == TEAM_ALLIANCE ? BattlegroundDef::ALLIANCEWINS : BattlegroundDef::HORDEWINS);

    this->updatePvPData();
}

bool Battleground::hasStarted()
{
    return this->m_hasStarted;
}

bool Battleground::hasEnded()
{
    return this->m_hasEnded;
}

void Battleground::addHonorToTeam(uint32_t team, uint32_t amount)
{
    std::lock_guard lock(m_mutex);

    for (const auto p : m_players[team])
        HonorHandler::AddHonorPointsToPlayer(p, amount);
}

void Battleground::castSpellOnTeam(uint32_t team, uint32_t spell)
{
    std::lock_guard lock(m_mutex);

    for (const auto p : m_players[team])
        p->castSpell(p, spell, false);
}

void Battleground::removeAuraFromTeam(uint32_t team, uint32_t aura)
{
    std::lock_guard lock(m_mutex);

    for (const auto p : m_players[team])
        p->removeAllAurasById(aura);
}

void Battleground::sendChatMessage(uint8_t Type, uint64_t Guid, const char* Format, ...)
{
    char msg[500];
    va_list ap;
    va_start(ap, Format);
    vsnprintf(msg, 500, Format, ap);
    va_end(ap);

    distributePacketToAll(AscEmu::Packets::SmsgMessageChat(Type, 0, 0, msg, Guid).serialise().get());
}

bool Battleground::HandleFinishBattlegroundRewardCalculation(PlayerTeam /*winningTeam*/)
{
    return true;
}

void Battleground::HookOnPlayerResurrect(Player* /*player*/)
{
}

void Battleground::HookOnUnitDied(Unit* /*victim*/)
{
}

void Battleground::distributePacketToAll(WorldPacket* packet)
{
    std::lock_guard lock(m_mutex);

    for (auto& m_player : m_players)
    {
        for (const auto itr : m_player)
            if (itr && itr->getSession())
                itr->getSession()->SendPacket(packet);
    }
}

void Battleground::distributePacketToTeam(WorldPacket* packet, uint32_t Team)
{
    std::lock_guard lock(m_mutex);

    for (const auto itr : m_players[Team])
        if (itr && itr->getSession())
            itr->getSession()->SendPacket(packet);
}

void Battleground::playSoundToAll(uint32_t Sound)
{
    distributePacketToAll(AscEmu::Packets::SmsgPlaySound(Sound).serialise().get());
}

bool Battleground::isFull() { return !(hasFreeSlots(0, m_type) || hasFreeSlots(1, m_type)); }

void Battleground::playSoundToTeam(uint32_t Team, uint32_t Sound)
{
    distributePacketToTeam(AscEmu::Packets::SmsgPlaySound(Sound).serialise().get(), Team);
}

void Battleground::removePlayer(Player* plr, bool logout)
{
    std::lock_guard lock(m_mutex);

    // Don't show invisible gm's leaving the game.
    if (plr->m_isGmInvisible == false)
        distributePacketToAll(AscEmu::Packets::SmsgBattlegroundPlayerLeft(plr->getGuid()).serialise().get());
    else
        --m_invisGMs;

    // Call subclassed virtual method
    OnRemovePlayer(plr);

    // Clean-up
    plr->setBattleground(nullptr);
    plr->setFullHealthMana();
    plr->setLastBattlegroundPetId(0);
    plr->setLastBattlegroundPetSpell(0);
    m_players[plr->getBgTeam()].erase(plr);
    memset(&plr->m_bgScore, 0, sizeof(BGScore));

    // are we in the group?
    if (plr->getGroup() == m_groups[plr->getBgTeam()])
        plr->getGroup()->RemovePlayer(plr->getPlayerInfo());

    // reset team
    plr->resetTeam();

    // revive the player if he is dead
    if (!plr->isAlive())
    {
        plr->setHealth(plr->getMaxHealth());
        plr->resurrect();
    }

    plr->removeAllAurasById(32727); // Arena preparation
    plr->removeAllAurasById(44521); // BG preparation
    plr->removeAllAurasById(44535);
    plr->removeAllAurasById(21074);

    plr->setMoveRoot(false);

    // teleport out
    if (!logout)
    {
        if (!m_hasEnded)
            if(!plr->getSession()->HasGMPermissions())
                plr->castSpell(plr, BattlegroundDef::DESERTER, true);

        if (!IS_INSTANCE(plr->getBGEntryMapId()))
            plr->safeTeleport(plr->getBGEntryMapId(), plr->getBGEntryInstanceId(), plr->getBGEntryPosition());
        else
            plr->safeTeleport(plr->getBindMapId(), 0, plr->getBindPosition());

        sBattlegroundManager.sendBattlefieldStatus(plr, BattlegroundDef::STATUS_NOFLAGS, 0, 0, 0, 0, 0);
    }

    if (m_players[0].size() == 0 && m_players[1].size() == 0)
    {
        // create an inactive event
        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_CLOSE);
        this->close();
    }

    plr->setBgTeam(plr->getTeam());
}

void Battleground::sendPVPData(Player* plr)
{
    std::lock_guard lock(m_mutex);

    WorldPacket data(10 * (m_players[0].size() + m_players[1].size()) + 50);
    buildPvPUpdateDataPacket(&data);
    plr->getSession()->SendPacket(&data);
}

void Battleground::eventCreate()
{
    OnCreate();
}

uint32_t Battleground::GetNameID()
{
    return 34;
}

int32_t Battleground::event_GetInstanceID()
{
    return m_mapMgr->getInstanceId();
}

void Battleground::eventCountdown()
{
    if (m_countdownStage == 1)
    {
        std::lock_guard lock(m_mutex);
        m_countdownStage = 2;

        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
                if (itr && itr->getSession())
                    itr->getSession()->SystemMessage(itr->getSession()->LocalizedWorldSrv(ServerString::SS_BATTLE_BEGIN_ONE_MINUTE),
                                                     itr->getSession()->LocalizedWorldSrv(GetNameID()));
        }
    }
    else if (m_countdownStage == 2)
    {
        std::lock_guard lock(m_mutex);

        m_countdownStage = 3;

        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
                if (itr && itr->getSession())
                    itr->getSession()->SystemMessage(itr->getSession()->LocalizedWorldSrv(ServerString::SS_THIRTY_SECONDS_UNTIL_THE_BATTLE),
                                                     itr->getSession()->LocalizedWorldSrv(GetNameID()));
        }
    }
    else if (m_countdownStage == 3)
    {
        std::lock_guard lock(m_mutex);

        m_countdownStage = 4;

        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
                if (itr && itr->getSession())
                    itr->getSession()->SystemMessage(itr->getSession()->LocalizedWorldSrv(ServerString::SS_FIFTEEN_SECONDS_UNTIL_THE_BATTLE),
                                                     itr->getSession()->LocalizedWorldSrv(GetNameID()));
        }

        sEventMgr.ModifyEventTime(this, EVENT_BATTLEGROUND_COUNTDOWN, 150);
        sEventMgr.ModifyEventTimeLeft(this, EVENT_BATTLEGROUND_COUNTDOWN, 15000);
    }
    else
    {
        std::lock_guard lock(m_mutex);
        for (auto& m_player : m_players)
        {
            for (const auto itr : m_player)
                if (itr && itr->getSession())
                    itr->getSession()->SystemMessage(itr->getSession()->LocalizedWorldSrv(ServerString::SS_THE_BATTLE_FOR_HAS_BEGUN),
                                                     itr->getSession()->LocalizedWorldSrv(GetNameID()));
        }

        sEventMgr.RemoveEvents(this, EVENT_BATTLEGROUND_COUNTDOWN);
        this->startBattleground();
    }
}

void Battleground::OnStart()
{
}

void Battleground::setWorldState(uint32_t Index, uint32_t Value)
{
    if (m_zoneId == 0)
        return;

    m_mapMgr->getWorldStatesHandler().SetWorldStateForZone(m_zoneId, 0, Index, Value);
}

void Battleground::close()
{
    std::lock_guard lock(m_mutex);

    // remove all players from the battleground
    m_hasEnded = true;
    for (uint8_t i = 0; i < 2; ++i)
    {
        std::set<Player*>::iterator itr;
        std::set<uint32_t>::iterator it2;

        uint32_t guid;
        Player* plr;

        for (itr = m_players[i].begin(); itr != m_players[i].end();)
        {
            plr = *itr;
            ++itr;
            removePlayer(plr, false);
        }

        for (it2 = m_pendPlayers[i].begin(); it2 != m_pendPlayers[i].end();)
        {
            guid = *it2;
            ++it2;
            plr = sObjectMgr.getPlayer(guid);

            if (plr)
                removePendingPlayer(plr);
            else
                m_pendPlayers[i].erase(guid);
        }
    }

    // call the virtual on close for cleanup etc
    OnClose();
}

void Battleground::OnClose()
{
}

Creature* Battleground::spawnSpiritGuide(float x, float y, float z, float o, uint32_t horde)
{
    if (horde > 1)
        horde = 1;

    CreatureProperties const* pInfo = sMySQLStore.getCreatureProperties(13116 + horde);
    if (pInfo == nullptr)
        return nullptr;

    Creature* pCreature = m_mapMgr->createCreature(pInfo->Id);

    pCreature->Create(m_mapMgr->getBaseMap()->getMapId(), x, y, z, o);

    pCreature->setEntry(13116 + horde);
    pCreature->setScale(1.0f);

    pCreature->setMaxHealth(10000);
    pCreature->setMaxPower(POWER_TYPE_MANA, 4868);
    pCreature->setMaxPower(POWER_TYPE_FOCUS, 200);
#if VERSION_STRING < Cata
    pCreature->setMaxPower(POWER_TYPE_HAPPINESS, 2000000);
#endif

    pCreature->setHealth(100000);
    pCreature->setPower(POWER_TYPE_MANA, 4868);
    pCreature->setPower(POWER_TYPE_FOCUS, 200);
#if VERSION_STRING < Cata
    pCreature->setPower(POWER_TYPE_HAPPINESS, 2000000);
#endif

    pCreature->setLevel(60);
    pCreature->setFaction(84 - horde);

    pCreature->setRace(0);
    pCreature->setClass(2);
    pCreature->setGender(1);
    pCreature->setPowerType(0);

    pCreature->setVirtualItemSlotId(MELEE, 22802);

    pCreature->setUnitFlags(UNIT_FLAG_PLUS_MOB | UNIT_FLAG_IGNORE_PLAYER_COMBAT | UNIT_FLAG_IGNORE_CREATURE_COMBAT); // 832
    pCreature->setPvpFlag();

    pCreature->setBaseAttackTime(MELEE, 2000);
    pCreature->setBaseAttackTime(OFFHAND, 2000);
    pCreature->setBoundingRadius(0.208f);
    pCreature->setCombatReach(1.5f);

    pCreature->setDisplayId(13337 + horde);
    pCreature->setNativeDisplayId(13337 + horde);

    pCreature->setChannelSpellId(22011);
    pCreature->setModCastSpeed(1.0f);

    pCreature->setNpcFlags(UNIT_NPC_FLAG_SPIRITGUIDE);
    pCreature->setSheathType(SHEATH_STATE_MELEE);
#if VERSION_STRING == TBC
    pCreature->setPositiveAuraLimit(POS_AURA_LIMIT_CREATURE);
#endif

    pCreature->setAItoUse(false);

    pCreature->SetCreatureProperties(sMySQLStore.getCreatureProperties(pInfo->Id));

    pCreature->PushToWorld(m_mapMgr);
    return pCreature;
}

Creature* Battleground::spawnSpiritGuide(LocationVector &v, uint32_t faction)
{
    return spawnSpiritGuide(v.x, v.y, v.z, v.o, faction);
}

uint32_t Battleground::getLastResurrect()
{
    return m_lastResurrectTime;
}

void Battleground::queuePlayerForResurrect(Player* plr, Creature* spirit_healer)
{
    std::lock_guard lock(m_mutex);

    const auto itr = m_resurrectMap.find(spirit_healer);
    if (itr != m_resurrectMap.end())
        itr->second.insert(plr->getGuidLow());
    plr->setAreaSpiritHealerGuid(spirit_healer->getGuid());
}

void Battleground::removePlayerFromResurrect(Player* plr, Creature* spirit_healer)
{
    std::lock_guard lock(m_mutex);

    const auto itr = m_resurrectMap.find(spirit_healer);
    if (itr != m_resurrectMap.end())
        itr->second.erase(plr->getGuidLow());
    plr->setAreaSpiritHealerGuid(0);
}

void Battleground::addSpiritGuide(Creature* pCreature)
{
    std::lock_guard lock(m_mutex);

    const auto itr = m_resurrectMap.find(pCreature);
    if (itr == m_resurrectMap.end())
    {
        std::set<uint32_t> ti;
        m_resurrectMap.insert(make_pair(pCreature, ti));
    }
}

void Battleground::removeSpiritGuide(Creature* pCreature)
{
    std::lock_guard lock(m_mutex);

    m_resurrectMap.erase(pCreature);
}

void Battleground::eventResurrectPlayers()
{
    std::lock_guard lock(m_mutex);

    for (auto& i : m_resurrectMap)
    {
        for (unsigned int itr : i.second)
        {
            Player* plr = m_mapMgr->getPlayer(itr);
            if (plr && plr->isDead())
            {
                WorldPacket data(SMSG_SPELL_START, 50);
                data << plr->GetNewGUID();
                data << plr->GetNewGUID();
                data << uint32_t(BattlegroundDef::RESURRECT);
                data << uint8_t(0);
                data << uint16_t(0);
                data << uint32_t(0);
                data << uint16_t(2);
                data << plr->getGuid();
                plr->sendMessageToSet(&data, true);

                data.Initialize(SMSG_SPELL_GO);
                data << plr->GetNewGUID();
                data << plr->GetNewGUID();
                data << uint32_t(BattlegroundDef::RESURRECT);
                data << uint8_t(0);
                data << uint8_t(1);
                data << uint8_t(1);
                data << plr->getGuid();
                data << uint8_t(0);
                data << uint16_t(2);
                data << plr->getGuid();
                plr->sendMessageToSet(&data, true);

                plr->resurrect();
                plr->setHealth(plr->getMaxHealth());
                plr->setPower(POWER_TYPE_MANA, plr->getMaxPower(POWER_TYPE_MANA));
                plr->setPower(POWER_TYPE_ENERGY, plr->getMaxPower(POWER_TYPE_ENERGY));
                plr->castSpell(plr, BattlegroundDef::REVIVE_PREPARATION, true);

#if VERSION_STRING >= TBC
                // Spawn last active pet
                if (plr->getLastBattlegroundPetId() != 0)
                {
                    plr->spawnPet(plr->getLastBattlegroundPetId());
                }
                else if (plr->getLastBattlegroundPetSpell() != 0)
                {
                    // TODO: not correct, according to classic spell should not be casted
                    // instead the pet should just spawn
                    // Hackfixing for now
                    plr->addUnitFlags(UNIT_FLAG_NO_REAGANT_COST);
                    plr->castSpell(plr, plr->getLastBattlegroundPetSpell(), true);
                    plr->removeUnitFlags(UNIT_FLAG_NO_REAGANT_COST);
                }
#endif

                plr->setLastBattlegroundPetId(0);
                plr->setLastBattlegroundPetSpell(0);
            }
        }
        i.second.clear();
    }
    m_lastResurrectTime = static_cast<uint32_t>(UNIXTIME);
}

bool Battleground::CanPlayerJoin(Player* plr, uint32_t type)
{
    return hasFreeSlots(plr->getBgTeam(), type) && (plr->getLevelGrouping() == getLevelGroup()) && (!plr->hasAurasWithId(BattlegroundDef::DESERTER));
}

bool Battleground::CreateCorpse(Player* /*plr*/)
{
    return true;
}

bool Battleground::HookSlowLockOpen(GameObject* /*pGo*/, Player* /*pPlayer*/, Spell* /*pSpell*/)
{
    return false;
}

bool Battleground::HookQuickLockOpen(GameObject* /*go*/, Player* /*player*/, Spell* /*spell*/)
{
    return false;
}

void Battleground::queueAtNearestSpiritGuide(Player* plr, Creature* old)
{
    float dist = 999999.0f;
    const Creature* cl = nullptr;
    std::set<uint32_t> *closest = nullptr;

    std::lock_guard lock(m_lock);

    for (auto& itr : m_resurrectMap)
    {
        if (itr.first == old)
            continue;

        const float dd = plr->GetDistance2dSq(itr.first);
        if (dd < dist)
        {
            cl = itr.first;
            closest = &itr.second;
            dist = dd;
        }
    }

    if (closest != nullptr)
    {
        closest->insert(plr->getGuidLow());
        plr->setAreaSpiritHealerGuid(cl->getGuid());
        plr->castSpell(plr, 2584, true);
    }
}

uint64_t Battleground::GetFlagHolderGUID(uint32_t /*faction*/) const
{
    return 0;
}

uint32_t Battleground::getFreeSlots(uint32_t t, uint32_t type)
{
    std::lock_guard lock(m_mutex);

    const uint32_t maxPlayers = sBattlegroundManager.getMaximumPlayers(type);

    const size_t s = maxPlayers - m_players[t].size() - m_pendPlayers[t].size();
    return static_cast<uint32_t>(s);
}

bool Battleground::hasFreeSlots(uint32_t Team, uint32_t type)
{
    std::lock_guard lock(m_mutex);

    const uint32_t maxPlayers = sBattlegroundManager.getMaximumPlayers(type);

    if (isTypeArena(type))
        return m_players[Team].size() + m_pendPlayers[Team].size() < maxPlayers;

        uint32_t size[2];
        size[0] = uint32_t(m_players[0].size() + m_pendPlayers[0].size());
        size[1] = uint32_t(m_players[1].size() + m_pendPlayers[1].size());
        return (size[Team] < maxPlayers) && ((static_cast<int>(size[Team]) - static_cast<int>(size[1 - Team])) <= 0);
}

bool Battleground::isTypeArena(uint32_t x)
{
    return (x >= BattlegroundDef::TYPE_ARENA_2V2 && x <= BattlegroundDef::TYPE_ARENA_5V5);
}

bool Battleground::isArena()
{
    return m_type >= BattlegroundDef::TYPE_ARENA_2V2 && m_type <= BattlegroundDef::TYPE_ARENA_5V5;
}

uint32_t Battleground::getFieldCount(uint32_t BGType)
{
    switch (BGType)
    {
        case BattlegroundDef::TYPE_ALTERAC_VALLEY:
            return 5;
        case BattlegroundDef::TYPE_ARATHI_BASIN:
        case BattlegroundDef::TYPE_WARSONG_GULCH:
        case BattlegroundDef::TYPE_STRAND_OF_THE_ANCIENT:
        case BattlegroundDef::TYPE_ISLE_OF_CONQUEST:
            return 2;
        case BattlegroundDef::TYPE_EYE_OF_THE_STORM:
            return 1;
        default:
            return 0;
    }
}
