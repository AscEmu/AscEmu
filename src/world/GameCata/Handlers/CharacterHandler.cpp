/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "git_version.h"
#include "AuthCodes.h"
#include "Management/WordFilter.h"
#include "Management/ArenaTeam.h"
#include "Management/Battleground/Battleground.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/MySQLDataStore.hpp"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Map/WorldCreator.h"
#include "Spell/Definitions/PowerType.h"
#include "GameCata/Management/GuildMgr.h"

void WorldSession::HandlePlayerLoginOpcode(WorldPacket& recv_data)
{
    ObjectGuid playerGuid;

    playerGuid[2] = recv_data.readBit();
    playerGuid[3] = recv_data.readBit();
    playerGuid[0] = recv_data.readBit();
    playerGuid[6] = recv_data.readBit();
    playerGuid[4] = recv_data.readBit();
    playerGuid[5] = recv_data.readBit();
    playerGuid[1] = recv_data.readBit();
    playerGuid[7] = recv_data.readBit();

    recv_data.ReadByteSeq(playerGuid[2]);
    recv_data.ReadByteSeq(playerGuid[7]);
    recv_data.ReadByteSeq(playerGuid[0]);
    recv_data.ReadByteSeq(playerGuid[3]);
    recv_data.ReadByteSeq(playerGuid[5]);
    recv_data.ReadByteSeq(playerGuid[6]);
    recv_data.ReadByteSeq(playerGuid[1]);
    recv_data.ReadByteSeq(playerGuid[4]);

    if (objmgr.GetPlayer((uint32_t)playerGuid) != nullptr || m_loggingInPlayer || _player)
    {
        uint8_t errorCode = E_CHAR_LOGIN_DUPLICATE_CHARACTER;
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &errorCode);
        return;
    }

    AsyncQuery* asyncQuery = new AsyncQuery(new SQLClassCallbackP0<WorldSession>(this, &WorldSession::LoadPlayerFromDBProc));
    asyncQuery->AddQuery("SELECT guid,class FROM characters WHERE guid = %u AND login_flags = %u", (uint32_t)playerGuid, (uint32_t)LOGIN_NO_FLAG);
    CharacterDatabase.QueueAsyncQuery(asyncQuery);
}

void WorldSession::HandleCharRenameOpcode(WorldPacket& recv_data)
{
    WorldPacket data(SMSG_CHAR_RENAME, recv_data.size() + 1);

    uint64_t guid;
    std::string name;
    recv_data >> guid;
    recv_data >> name;

    PlayerInfo* player_info = objmgr.GetPlayerInfo((uint32_t)guid);
    if (player_info == nullptr)
        return;

    QueryResult* result = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u AND acct = %u", (uint32_t)guid, _accountId);
    if (result == nullptr)
        return;

    delete result;

    // Check name for rule violation.
    LoginErrorCode err = VerifyName(name.c_str(), name.length());
    if (err != E_CHAR_NAME_SUCCESS)
    {
        data << uint8_t(err);
        data << guid;
        data << name;
        SendPacket(&data);
        return;
    }

    QueryResult* result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
    if (result2)
    {
        if (result2->Fetch()[0].GetUInt32() > 0)
        {
            // That name is banned!
            data << uint8_t(E_CHAR_NAME_PROFANE);
            data << guid;
            data << name;
            SendPacket(&data);
        }
        delete result2;
    }

    if (objmgr.GetPlayerInfoByName(name.c_str()) != NULL)
    {
        data << uint8_t(E_CHAR_CREATE_NAME_IN_USE);
        data << guid;
        data << name;
        SendPacket(&data);
        return;
    }

    Util::CapitalizeString(name);
    objmgr.RenamePlayerInfo(player_info, player_info->name, name.c_str());

    sPlrLog.writefromsession(this, "a rename was pending. renamed character %s (GUID: %u) to %s.", player_info->name, player_info->guid, name.c_str());

    free(player_info->name);

    player_info->name = strdup(name.c_str());
    CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s' WHERE guid = %u", name.c_str(), (uint32_t)guid);
    CharacterDatabase.WaitExecute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32_t)LOGIN_NO_FLAG, (uint32_t)guid);

    data << uint8_t(E_RESPONSE_SUCCESS);
    data << guid;
    data << name;

    SendPacket(&data);
}

void WorldSession::FullLogin(Player* plr)
{
    LogDebug("WorldSession : Fully loading player %u", plr->getGuidLow());

    SetPlayer(plr);

    m_MoverGuid = plr->getGuid();
    m_MoverWoWGuid.Init(plr->getGuid());

    MapMgr* mapMgr = sInstanceMgr.GetInstance(plr);
    if (mapMgr && mapMgr->m_battleground)
    {
        if (mapMgr->m_battleground->HasEnded() == true ||
            mapMgr->m_battleground->HasFreeSlots(plr->GetTeamInitial(), mapMgr->m_battleground->GetType() == false))
        {
            mapMgr = nullptr;
        }
    }

    if (!mapMgr)
    {
        if (!IS_INSTANCE(plr->m_bgEntryPointMap))
        {
            plr->m_position.x = plr->m_bgEntryPointX;
            plr->m_position.y = plr->m_bgEntryPointY;
            plr->m_position.z = plr->m_bgEntryPointZ;
            plr->m_position.o = plr->m_bgEntryPointO;
            plr->m_mapId = plr->m_bgEntryPointMap;
        }
        else
        {
            plr->m_position.x = plr->GetBindPositionX();
            plr->m_position.y = plr->GetBindPositionY();
            plr->m_position.z = plr->GetBindPositionZ();
            plr->m_position.o = 0;
            plr->m_mapId = plr->GetBindMapId();
        }
    }

    uint32 VMapId;
    float VO;
    float VX;
    float VY;
    float VZ;

    if (HasGMPermissions() && plr->m_FirstLogin && sWorld.settings.gm.isStartOnGmIslandEnabled)
    {
        VMapId = 1;
        VO = 0;
        VX = 16222.6f;
        VY = 16265.9f;
        VZ = 14.2085f;

        plr->m_position.x = VX;
        plr->m_position.y = VY;
        plr->m_position.z = VZ;
        plr->m_position.o = VO;
        plr->m_mapId = VMapId;

        plr->SetBindPoint(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), plr->GetMapId(), plr->GetZoneId());
    }
    else
    {
        VMapId = plr->GetMapId();
        VO = plr->GetOrientation();
        VX = plr->GetPositionX();
        VY = plr->GetPositionY();
        VZ = plr->GetPositionZ();
    }

    plr->SendLoginVerifyWorld(VMapId, VX, VY, VZ, VO);

    WorldPacket datab(SMSG_FEATURE_SYSTEM_STATUS, 7);
    datab << uint8(2);
    datab << uint32(1);
    datab << uint32(1);
    datab << uint32(2);
    datab << uint32(0);
    datab.writeBit(true);
    datab.writeBit(true);
    datab.writeBit(false);
    datab.writeBit(true);
    datab.writeBit(false);
    datab.writeBit(false);
    datab.flushBits();
    datab << uint32(1);
    datab << uint32(0);
    datab << uint32(10);
    datab << uint32(60);
    SendPacket(&datab);

    WorldPacket dataldm(SMSG_LEARNED_DANCE_MOVES, 4 + 4);
    dataldm << uint64(0);
    SendPacket(&dataldm);

    plr->UpdateAttackSpeed();

    PlayerInfo* info = objmgr.GetPlayerInfo(plr->getGuidLow());
    if (info == nullptr)
    {
        info = new PlayerInfo;
        info->cl = plr->getClass();
        info->gender = plr->getGender();
        info->guid = plr->getGuidLow();
        info->name = strdup(plr->getName().c_str());
        info->lastLevel = plr->getLevel();
        info->lastOnline = UNIXTIME;
        info->lastZone = plr->GetZoneId();
        info->race = plr->getRace();
        info->team = plr->GetTeam();
        info->guildRank = GUILD_RANK_NONE;
        info->m_Group = nullptr;
        info->subGroup = 0;
        objmgr.AddPlayerInfo(info);
    }
    plr->m_playerInfo = info;

    info->m_loggedInPlayer = plr;

    SendAccountDataTimes(PER_CHARACTER_CACHE_MASK);

    CharacterDatabase.Execute("UPDATE characters SET online = 1 WHERE guid = %u", plr->getGuidLow());

    bool enter_world = true;

#if VERSION_STRING != Cata
    if (plr->obj_movement_info.transporter_info.guid != 0)
#else
    if (!plr->obj_movement_info.getTransportGuid().IsEmpty())
#endif
    {
#if VERSION_STRING != Cata
        Transporter* pTrans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(plr->obj_movement_info.transporter_info.guid));
#else
        Transporter* pTrans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(static_cast<uint32>(plr->obj_movement_info.getTransportGuid())));
#endif
        if (pTrans)
        {
            if (plr->IsDead())
            {
                plr->ResurrectPlayer();
                plr->setHealth(plr->getMaxHealth());
                plr->SetPower(POWER_TYPE_MANA, plr->GetMaxPower(POWER_TYPE_MANA));
            }

            float c_tposx = pTrans->GetPositionX() + plr->GetTransPositionX();
            float c_tposy = pTrans->GetPositionY() + plr->GetTransPositionY();
            float c_tposz = pTrans->GetPositionZ() + plr->GetTransPositionZ();

            if (plr->GetMapId() != pTrans->GetMapId())       // loaded wrong map
            {
                plr->SetMapId(pTrans->GetMapId());

                WorldPacket dataw(SMSG_NEW_WORLD, 4 + 4 + 4 + 4 + 4);
                dataw << c_tposx;
                dataw << plr->GetOrientation();
                dataw << c_tposz;
                dataw << pTrans->GetMapId();
                dataw << c_tposy;
                SendPacket(&dataw);

                enter_world = false;
            }

            plr->SetPosition(c_tposx, c_tposy, c_tposz, plr->GetOrientation(), false);
            pTrans->AddPassenger(plr);
        }
    }

    LOG_DEBUG("Player %s logged in.", plr->getName().c_str());

    sWorld.incrementPlayerCount(plr->GetTeam());

    if (plr->m_FirstLogin && !sWorld.settings.player.skipCinematics)
    {
        if (const auto charEntry = sChrClassesStore.LookupEntry(plr->getClass()))
        {
            if (charEntry->cinematic_id != 0)
                OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &charEntry->cinematic_id);
            else if (const auto raceEntry = sChrRacesStore.LookupEntry(plr->getRace()))
                OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &raceEntry->cinematic_id);
        }
    }

    LOG_DETAIL("Created new player for existing players (%s)", plr->getName().c_str());

    // Login time, will be used for played time calc
    plr->m_playedtime[2] = uint32(UNIXTIME);

    // Send online status to people having this char in friendlist
    _player->Social_TellFriendsOnline();
    // send friend list (for ignores)
    _player->Social_SendFriendList(7);

    plr->SendDungeonDifficulty();
    plr->SendRaidDifficulty();

    //plr->SendEquipmentSetList();

    //\todo danko
#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    GM_Ticket* ticket = objmgr.GetGMTicketByPlayer(_player->GetGUID());
    if (ticket != NULL)
    {
        //Send status change to gm_sync_channel
        Channel* chn = channelmgr.GetChannel(sWorld.getGmClientChannel().c_str(), _player);
        if (chn)
        {
            std::stringstream ss;
            ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ONLINESTATE;
            ss << ":" << ticket->guid;
            ss << ":1";
            chn->Say(_player, ss.str().c_str(), NULL, true);
        }
    }
#endif

    if (Config.MainConfig.getBoolDefault("Server", "SendStatsOnJoin", false))
    {
#ifdef WIN32
        _player->BroadcastMessage("Server: %sAscEmu - %s-Windows-%s", MSG_COLOR_WHITE, CONFIG, ARCH);
#else
        _player->BroadcastMessage("Server: %sAscEmu - %s-%s", MSG_COLOR_WHITE, PLATFORM_TEXT, ARCH);
#endif

        _player->BroadcastMessage("Build hash: %s%s", MSG_COLOR_CYAN, BUILD_HASH_STR);
        _player->BroadcastMessage("Online Players: %s%u |rPeak: %s%u|r Accepted Connections: %s%u",
                                  MSG_COLOR_SEXGREEN, sWorld.getSessionCount(), MSG_COLOR_SEXBLUE, sWorld.getPeakSessionCount(), MSG_COLOR_SEXBLUE, sWorld.getAcceptedConnections());

        _player->BroadcastMessage("Server Uptime: |r%s", sWorld.getWorldUptimeString().c_str());
    }

    SendMOTD();

    if (plr->m_isResting)
        plr->ApplyPlayerRestState(true);

    if (plr->m_timeLogoff > 0 && plr->getLevel() < plr->getMaxLevel())
    {
        uint32 currenttime = uint32(UNIXTIME);
        uint32 timediff = currenttime - plr->m_timeLogoff;

        if (timediff > 0)
            plr->AddCalculatedRestXP(timediff);
    }

    if (info->m_Group)
        info->m_Group->Update();

    if (enter_world && !_player->GetMapMgr())
        plr->AddToWorld();

    sHookInterface.OnFullLogin(_player);

    // Set our Guild Infos   
    if (plr->getPlayerInfo()->m_guild && sGuildMgr.getGuildById(plr->getPlayerInfo()->m_guild != NULL))
    {
        plr->SetInGuild(plr->getPlayerInfo()->m_guild);
        plr->SetRank(static_cast<uint8_t>(plr->getPlayerInfo()->guildRank));
        plr->GetGuild()->sendLoginInfo(plr->GetSession());
        if (Guild* guild = sGuildMgr.getGuildById(plr->GetGuildId()))
            plr->SetGuildLevel(guild->getLevel());
    }
    else
    {
        plr->SetInGuild(0);
        plr->SetRank(GUILD_RANK_NONE);
    }

    objmgr.AddPlayer(_player);
}
