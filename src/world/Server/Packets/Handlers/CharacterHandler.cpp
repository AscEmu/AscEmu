/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
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
#if VERSION_STRING == Cata
#include "GameCata/Management/GuildMgr.h"
#endif

LoginErrorCode VerifyName(const char* name, size_t nlen)
{
    const char* p;
    size_t i;

    static const char* bannedCharacters = "\t\v\b\f\a\n\r\\\"\'\? <>[](){}_=+-|/!@#$%^&*~`.,0123456789\0";
    static const char* allowedCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (worldConfig.server.enableLimitedNames)
    {
        if (nlen == 0)
            return E_CHAR_NAME_NO_NAME;
        else if (nlen < 2)
            return E_CHAR_NAME_TOO_SHORT;
        else if (nlen > 12)
            return E_CHAR_NAME_TOO_LONG;

        for (i = 0; i < nlen; ++i)
        {
            p = allowedCharacters;
            for (; *p != 0; ++p)
            {
                if (name[i] == *p)
                    goto cont;
            }
            return E_CHAR_NAME_INVALID_CHARACTER;
        cont:
            continue;
        }
    }
    else
    {
        for (i = 0; i < nlen; ++i)
        {
            p = bannedCharacters;
            while (*p != 0 && name[i] != *p && name[i] != 0)
                ++p;

            if (*p != 0)
                return E_CHAR_NAME_INVALID_CHARACTER;
        }
    }

    return E_CHAR_NAME_SUCCESS;
}

#if VERSION_STRING > TBC
void WorldSession::HandleCharCustomizeLooksOpcode(WorldPacket& recvData)
{
    uint64 guid;
    std::string newname;

    recvData >> guid;
    recvData >> newname;

    uint8 gender;
    uint8 skin;
    uint8 face;
    uint8 hairStyle;
    uint8 hairColor;
    uint8 facialHair;
#if VERSION_STRING != Cata
    uint8 race;
    uint8 faction;
#endif

    recvData >> gender;
    recvData >> skin;
    recvData >> hairColor;
    recvData >> hairStyle;
    recvData >> facialHair;
    recvData >> face;
#if VERSION_STRING != Cata
    recvData >> race;
    recvData >> faction;
#endif

    LoginErrorCode res = VerifyName(newname.c_str(), newname.length());
    if (res != E_CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1);
        data << uint8(E_CHAR_NAME_NO_NAME);
        SendPacket(&data);
        return;
    }

    QueryResult * result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM `banned_names` WHERE name = '%s'", CharacterDatabase.EscapeString(newname).c_str());
    if (result2)
    {
        if (result2->Fetch()[0].GetUInt32() > 0)
        {
            WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1);
            data << uint8(E_CHAR_NAME_PROFANE);
            SendPacket(&data);
        }
        delete result2;
    }

    PlayerInfo* info = objmgr.GetPlayerInfoByName(newname.c_str());
    if (info != NULL && info->guid != guid)
    {
        WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1);
        data << uint8(E_CHAR_CREATE_NAME_IN_USE);
        SendPacket(&data);
        return;
    }

    CharacterDatabase.EscapeString(newname).c_str();
    Util::CapitalizeString(newname);

    CharacterDatabase.WaitExecute("UPDATE `characters` set name = '%s' WHERE guid = '%u'", newname.c_str(), (uint32)guid);
    CharacterDatabase.WaitExecute("UPDATE `characters` SET login_flags = %u WHERE guid = '%u'", (uint32)LOGIN_NO_FLAG, (uint32)guid);

    Player::CharChange_Looks(guid, gender, skin, face, hairStyle, hairColor, facialHair);

    WorldPacket data(SMSG_CHAR_CUSTOMIZE, 1 + 8 + (newname.size() + 1) + 6);
    data << uint8(E_RESPONSE_SUCCESS);
    data << uint64(guid);
    data << newname;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(hairColor);
    data << uint8(facialHair);
    SendPacket(&data);
}
#endif

void WorldSession::HandleCharEnumOpcode(WorldPacket& /*recvData*/)
{
    AsyncQuery* q = new AsyncQuery(new SQLClassCallbackP1<World, uint32>(World::getSingletonPtr(), &World::sendCharacterEnumToAccountSession, GetAccountId()));
#if VERSION_STRING != Cata
    q->AddQuery("SELECT guid, level, race, class, gender, bytes, bytes2, name, positionX, positionY, positionZ, mapId, zoneId, banned, restState, deathstate, login_flags, player_flags, guild_data.guildid FROM characters LEFT JOIN guild_data ON characters.guid = guild_data.playerid WHERE acct=%u ORDER BY guid LIMIT 10", GetAccountId());
#else
    q->AddQuery("SELECT guid, level, race, class, gender, bytes, bytes2, name, positionX, positionY, positionZ, mapId, zoneId, banned, restState, deathstate, login_flags, player_flags, guild_member.guildId FROM characters LEFT JOIN guild_member ON characters.guid = guild_member.playerGuid WHERE acct=%u ORDER BY guid LIMIT 10", GetAccountId());
#endif
    CharacterDatabase.QueueAsyncQuery(q);
}

void WorldSession::LoadAccountDataProc(QueryResult* result)
{
    size_t len;
    const char* data;
    char* d;

    if (!result)
    {
        CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", _accountId);
        return;
    }

    for (uint8 i = 0; i < 7; ++i)
    {
        data = result->Fetch()[1 + i].GetString();
        len = data ? strlen(data) : 0;
        if (len > 1)
        {
            d = new char[len + 1];
            memcpy(d, data, len + 1);
            SetAccountData(i, d, true, (uint32)len);
        }
    }
}

void WorldSession::HandleCharCreateOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 10);
    std::string name;
    uint8 race;
    uint8 class_;

    recv_data >> name;
    recv_data >> race;
    recv_data >> class_;
    recv_data.rpos(0);

    LoginErrorCode res = VerifyName(name.c_str(), name.length());
    if (res != E_CHAR_NAME_SUCCESS)
    {
        OutPacket(SMSG_CHAR_CREATE, 1, &res);
        return;
    }

    res = sMySQLStore.isCharacterNameAllowed(name) ? E_CHAR_NAME_PROFANE : E_CHAR_NAME_SUCCESS;
    if (res != E_CHAR_NAME_SUCCESS)
    {
        OutPacket(SMSG_CHAR_CREATE, 1, &res);
        return;
    }

    res = objmgr.GetPlayerInfoByName(name.c_str()) == NULL ? E_CHAR_CREATE_SUCCESS : E_CHAR_CREATE_NAME_IN_USE;
    if (res != E_CHAR_CREATE_SUCCESS)
    {
        OutPacket(SMSG_CHAR_CREATE, 1, &res);
        return;
    }

    res = sHookInterface.OnNewCharacter(race, class_, this, name.c_str()) ? E_CHAR_CREATE_SUCCESS : E_CHAR_CREATE_ERROR;
    if (res != E_CHAR_CREATE_SUCCESS)
    {
        OutPacket(SMSG_CHAR_CREATE, 1, &res);
        return;
    }

    QueryResult* result = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(name).c_str());
    if (result)
    {
        if (result->Fetch()[0].GetUInt32() > 0)
        {
            // That name is banned!
            LoginErrorCode login_error = E_CHAR_NAME_PROFANE;

            OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
            delete result;
            return;
        }
        delete result;
    }

    // Check if player got Death Knight already on this realm.
    if (worldConfig.player.deathKnightLimit && has_dk && (class_ == DEATHKNIGHT))
    {
        LoginErrorCode login_error = E_CHAR_CREATE_UNIQUE_CLASS_LIMIT;
        OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
        return;
    }

    // loading characters

    // Check the number of characters, so we can't make over 10.
    // They're able to manage to create >10 sometimes, not exactly sure how ..

    result = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE acct = %u", GetAccountId());
    if (result)
    {
        if (result->Fetch()[0].GetUInt32() >= 10)
        {
            // We can't make any more characters.
            LoginErrorCode login_error = E_CHAR_CREATE_SERVER_LIMIT;
            OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
            delete result;
            return;
        }
        delete result;
    }

    Player* pNewChar = objmgr.CreatePlayer(class_);
    pNewChar->SetSession(this);
    if (!pNewChar->Create(recv_data))
    {
        // failed.
        pNewChar->ok_to_remove = true;
        delete pNewChar;

        LoginErrorCode login_error = E_CHAR_CREATE_FAILED;
        OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
        return;
    }

    //Same Faction limitation only applies to PVP and RPPVP realms :)
    uint32 realmType = sLogonCommHandler.getRealmType();
    if (!HasGMPermissions() && realmType == REALMTYPE_PVP && _side >= 0 && !worldConfig.player.isCrossoverCharsCreationEnabled)  // ceberwow fixed bug
    {
        if ((pNewChar->IsTeamAlliance() && (_side == 1)) || (pNewChar->IsTeamHorde() && (_side == 0)))
        {
            pNewChar->ok_to_remove = true;
            delete pNewChar;

            LoginErrorCode login_error = E_CHAR_CREATE_PVP_TEAMS_VIOLATION;
            OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
            return;
        }
    }

    //Check if player has a level 55 or higher character on this realm and allow him to create DK.
    //This check can be turned off in world.conf
    if (worldConfig.player.deathKnightPreReq && !has_level_55_char && (class_ == DEATHKNIGHT))
    {
        pNewChar->ok_to_remove = true;
        delete pNewChar;

        LoginErrorCode login_error = E_CHAR_CREATE_LEVEL_REQUIREMENT;
        OutPacket(SMSG_CHAR_CREATE, 1, &login_error);
        return;
    }

    pNewChar->UnSetBanned();
    pNewChar->addSpell(22027);      // Remove Insignia

    if (pNewChar->getClass() == WARLOCK)
    {
        pNewChar->AddSummonSpell(416, 3110);        // imp fireball
        pNewChar->AddSummonSpell(417, 19505);
        pNewChar->AddSummonSpell(1860, 3716);
        pNewChar->AddSummonSpell(1863, 7814);
    }

    pNewChar->SaveToDB(true);

    PlayerInfo* pn = new PlayerInfo ;
    pn->guid = pNewChar->GetLowGUID();
    pn->name = strdup(pNewChar->GetName());
    pn->cl = pNewChar->getClass();
    pn->race = pNewChar->getRace();
    pn->gender = pNewChar->getGender();
    pn->acct = GetAccountId();
    pn->m_Group = 0;
    pn->subGroup = 0;
    pn->m_loggedInPlayer = NULL;
    pn->team = pNewChar->GetTeam();
#if VERSION_STRING != Cata
    pn->guild = NULL;
    pn->guildRank = NULL;
    pn->guildMember = NULL;
#else
    pn->m_guild = 0;
    pn->guildRank = GUILD_RANK_NONE;
#endif
    pn->lastOnline = UNIXTIME;
    objmgr.AddPlayerInfo(pn);

    pNewChar->ok_to_remove = true;
    delete  pNewChar;

    LoginErrorCode login_error = E_CHAR_CREATE_SUCCESS;
    OutPacket(SMSG_CHAR_CREATE, 1, &login_error);

    sLogonCommHandler.updateAccountCount(GetAccountId(), 1);
}

void WorldSession::HandleCharDeleteOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);
    uint8 fail = E_CHAR_DELETE_SUCCESS;

    uint64 guid;
    recv_data >> guid;

    if (objmgr.GetPlayer((uint32)guid) != NULL)
    {
        // "Char deletion failed"
        fail = E_CHAR_DELETE_FAILED;
    }
    else
    {
        fail = DeleteCharacter((uint32)guid);
    }
    OutPacket(SMSG_CHAR_DELETE, 1, &fail);
}

uint8 WorldSession::DeleteCharacter(uint32 guid)
{
    PlayerInfo* inf = objmgr.GetPlayerInfo(guid);
    if (inf != NULL && inf->m_loggedInPlayer == NULL)
    {
        QueryResult* result = CharacterDatabase.Query("SELECT name FROM characters WHERE guid = %u AND acct = %u", (uint32)guid, _accountId);
        if (!result)
            return E_CHAR_DELETE_FAILED;

        std::string name = result->Fetch()[0].GetString();
        delete result;

#if VERSION_STRING != Cata
        if (inf->guild)
        {
            if (inf->guild->GetGuildLeader() == inf->guid)
                return E_CHAR_DELETE_FAILED_GUILD_LEADER;
            else
                inf->guild->RemoveGuildMember(inf, NULL);
        }
#else
        if (inf->m_guild)
        {
            Guild* guild = sGuildMgr.getGuildById(inf->m_guild);
            if (guild->getLeaderGUID() == inf->guid)
                return E_CHAR_DELETE_FAILED_GUILD_LEADER;
            else
                guild->handleRemoveMember(this, Arcemu::Util::GUID_HIPART(inf->guid));
        }
#endif

        for (uint8 i = 0; i < NUM_CHARTER_TYPES; ++i)
        {
            Charter* c = objmgr.GetCharterByGuid(guid, (CharterTypes)i);
            if (c != nullptr)
                c->RemoveSignature((uint32)guid);
        }


        for (uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
        {
            ArenaTeam* t = objmgr.GetArenaTeamByGuid((uint32)guid, i);
            if (t != NULL && t->m_leader == guid)
                return E_CHAR_DELETE_FAILED_ARENA_CAPTAIN;
            if (t != NULL)
                t->RemoveMember(inf);
        }

        /*if (_socket != NULL)
            sPlrLog.write("Account: %s | IP: %s >> Deleted player %s", GetAccountName().c_str(), GetSocket()->GetRemoteIP().c_str(), name.c_str());*/

        sPlrLog.writefromsession(this, "deleted character %s (GUID: %u)", name.c_str(), (uint32)guid);

        CharacterDatabase.WaitExecute("DELETE FROM characters WHERE guid = %u", (uint32)guid);

        Corpse* c = objmgr.GetCorpseByOwner((uint32)guid);
        if (c)
            CharacterDatabase.Execute("DELETE FROM corpses WHERE guid = %u", c->GetLowGUID());

        CharacterDatabase.Execute("DELETE FROM playeritems WHERE ownerguid=%u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE playerguid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerId = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM mailbox WHERE player_guid = %u", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u OR friend_guid = %u", (uint32)guid, (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u OR ignore_guid = %u", (uint32)guid, (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = '%u' AND achievement NOT IN (457, 467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, 1413, 1415, 1414, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1463, 1400, 456, 1402)", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = '%u'", (uint32)guid);
        CharacterDatabase.Execute("DELETE FROM playerspells WHERE GUID = '%u'", guid);
        CharacterDatabase.Execute("DELETE FROM playerdeletedspells WHERE GUID = '%u'", guid);
        CharacterDatabase.Execute("DELETE FROM playerreputations WHERE guid = '%u'", guid);
        CharacterDatabase.Execute("DELETE FROM playerskills WHERE GUID = '%u'", guid);

        // remove player info
        objmgr.DeletePlayerInfo((uint32)guid);
        return E_CHAR_DELETE_SUCCESS;
    }
    return E_CHAR_DELETE_FAILED;
}

//\todo move this to all other versions
#if VERSION_STRING != Cata
void WorldSession::HandleCharRenameOpcode(WorldPacket& recv_data)
{
    WorldPacket data(SMSG_CHAR_RENAME, recv_data.size() + 1);

    uint64 guid;
    std::string name;
    recv_data >> guid;
    recv_data >> name;

    PlayerInfo* pi = objmgr.GetPlayerInfo((uint32)guid);
    if (pi == 0) return;

    QueryResult* result = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u AND acct = %u", (uint32)guid, _accountId);
    if (result == 0)
    {
        delete result;
        return;
    }
    delete result;

    // Check name for rule violation.

    LoginErrorCode err = VerifyName(name.c_str(), name.length());
    if (err != E_CHAR_NAME_SUCCESS)
    {
        data << uint8(err);
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
            data << uint8(E_CHAR_NAME_PROFANE);
            data << guid;
            data << name;
            SendPacket(&data);
        }
        delete result2;
    }

    // Check if name is in use.
    if (objmgr.GetPlayerInfoByName(name.c_str()) != NULL)
    {
        data << uint8(E_CHAR_CREATE_NAME_IN_USE);
        data << guid;
        data << name;
        SendPacket(&data);
        return;
    }

    // correct capitalization
    Util::CapitalizeString(name);
    objmgr.RenamePlayerInfo(pi, pi->name, name.c_str());

    sPlrLog.writefromsession(this, "a rename was pending. renamed character %s (GUID: %u) to %s.", pi->name, pi->guid, name.c_str());

    // If we're here, the name is okay.
    free(pi->name);
    pi->name = strdup(name.c_str());
    CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s' WHERE guid = %u", name.c_str(), (uint32)guid);
    CharacterDatabase.WaitExecute("UPDATE characters SET login_flags = %u WHERE guid = %u", (uint32)LOGIN_NO_FLAG, (uint32)guid);

    data << uint8(E_RESPONSE_SUCCESS);
    data << guid;
    data << name;
    SendPacket(&data);
}
#endif

#if VERSION_STRING != Cata
void WorldSession::HandlePlayerLoginOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);
    uint64_t playerGuid = 0;

    LOG_DEBUG("WORLD: Recvd Player Logon Message");

    recv_data >> playerGuid;

    if (objmgr.GetPlayer((uint32_t)playerGuid) != nullptr || m_loggingInPlayer || _player)
    {
        uint8 respons = E_CHAR_LOGIN_DUPLICATE_CHARACTER;
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &respons);
        return;
    }

    AsyncQuery* asyncQuery = new AsyncQuery(new SQLClassCallbackP0<WorldSession>(this, &WorldSession::LoadPlayerFromDBProc));
    asyncQuery->AddQuery("SELECT guid,class FROM characters WHERE guid = %u AND login_flags = %u", (uint32_t)playerGuid, (uint32_t)LOGIN_NO_FLAG);
    CharacterDatabase.QueueAsyncQuery(asyncQuery);
}
#endif

void WorldSession::LoadPlayerFromDBProc(QueryResultVector& results)
{
    if (results.size() < 1)
    {
        uint8 respons = E_CHAR_LOGIN_NO_CHARACTER;
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &respons);
        return;
    }

    QueryResult* result = results[0].result;
    if (! result)
    {
        LOG_ERROR("Player login query failed!");
        uint8 respons = E_CHAR_LOGIN_NO_CHARACTER;
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &respons);
        return;
    }

    Field* fields = result->Fetch();

    uint64 playerGuid = fields[0].GetUInt64();
    uint8 _class = fields[1].GetUInt8();

    Player* plr = nullptr;

    switch(_class)
    {
        case WARRIOR:
            plr = new Warrior(static_cast<uint32>(playerGuid));
            break;
        case PALADIN:
            plr = new Paladin(static_cast<uint32>(playerGuid));
            break;
        case HUNTER:
            plr = new Hunter(static_cast<uint32>(playerGuid));
            break;
        case ROGUE:
            plr = new Rogue(static_cast<uint32>(playerGuid));
            break;
        case PRIEST:
            plr = new Priest(static_cast<uint32>(playerGuid));
            break;
        case DEATHKNIGHT:
            plr = new DeathKnight(static_cast<uint32>(playerGuid));
            break;
        case SHAMAN:
            plr = new Shaman(static_cast<uint32>(playerGuid));
            break;
        case MAGE:
            plr = new Mage(static_cast<uint32>(playerGuid));
            break;
        case WARLOCK:
            plr = new Warlock(static_cast<uint32>(playerGuid));
            break;
        case DRUID:
            plr = new Druid(static_cast<uint32>(playerGuid));
            break;
    }

    if (plr == NULL)
    {
        LOG_ERROR("Class %u unknown!", _class);
        uint8 respons = E_CHAR_LOGIN_NO_CHARACTER;
        OutPacket(SMSG_CHARACTER_LOGIN_FAILED, 1, &respons);
        return;
    }

    plr->SetSession(this);

    m_bIsWLevelSet = false;

    LOG_DEBUG("Async loading player %u", (uint32)playerGuid);
    m_loggingInPlayer = plr;
    plr->LoadFromDB((uint32)playerGuid);
}

#if VERSION_STRING != Cata
void WorldSession::FullLogin(Player* plr)
{
    LOG_DEBUG("Fully loading player %u", plr->GetLowGUID());

    SetPlayer(plr);
    m_MoverWoWGuid.Init(plr->GetGUID());

    MapMgr* mgr = sInstanceMgr.GetInstance(plr);
    if (mgr && mgr->m_battleground)
    {
        // Don't allow player to login into a bg that has ended or is full
        if (mgr->m_battleground->HasEnded() == true ||
                mgr->m_battleground->HasFreeSlots(plr->GetTeamInitial(), mgr->m_battleground->GetType() == false))
        {
            mgr = NULL;
        }
    }

    // Trying to log to an instance that doesn't exists anymore?
    if (!mgr)
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

    // copy to movement array
    movement_packet[0] = m_MoverWoWGuid.GetNewGuidMask();
    memcpy(&movement_packet[1], m_MoverWoWGuid.GetNewGuid(), m_MoverWoWGuid.GetNewGuidLen());

    // world preload
    uint32 VMapId;
    float VO;
    float VX;
    float VY;
    float VZ;

    // GMs should start on GM Island and be bound there
    if (HasGMPermissions() && plr->m_FirstLogin && worldConfig.gm.isStartOnGmIslandEnabled)
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // send voicechat state - active/inactive
    //
    // {SERVER} Packet: (0x03C7) UNKNOWN PacketSize = 2
    // |------------------------------------------------|----------------|
    // |00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |0123456789ABCDEF|
    // |------------------------------------------------|----------------|
    // |02 01                                            |..              |
    // -------------------------------------------------------------------
    //
    //
    // Old packetdump is OLD. This is probably from 2.2.0 (that was the patch when it was added to wow)!
    //
    //////////////////////////////////////////////////////////////////////////////////////////////////////


    StackWorldPacket<20> datab(SMSG_FEATURE_SYSTEM_STATUS);


    datab.Initialize(SMSG_FEATURE_SYSTEM_STATUS);

    datab << uint8(2);
    datab << uint8(0);

    SendPacket(&datab);

#if VERSION_STRING > TBC
    WorldPacket dataldm(SMSG_LEARNED_DANCE_MOVES, 4 + 4);

    dataldm << uint32(0);
    dataldm << uint32(0);

    SendPacket(&dataldm);
#endif
    plr->UpdateAttackSpeed();

    // Make sure our name exists (for premade system)
    PlayerInfo* info = objmgr.GetPlayerInfo(plr->GetLowGUID());

    if (info == NULL)
    {
        info = new PlayerInfo;
        info->cl = plr->getClass();
        info->gender = plr->getGender();
        info->guid = plr->GetLowGUID();
        info->name = strdup(plr->GetName());
        info->lastLevel = plr->getLevel();
        info->lastOnline = UNIXTIME;
        info->lastZone = plr->GetZoneId();
        info->race = plr->getRace();
        info->team = plr->GetTeam();
        info->guild = NULL;
        info->guildRank = NULL;
        info->guildMember = NULL;
        info->m_Group = NULL;
        info->subGroup = 0;
        objmgr.AddPlayerInfo(info);
    }
    plr->m_playerInfo = info;
    if (plr->m_playerInfo->guild)
    {
        plr->SetGuildId(plr->m_playerInfo->guild->getGuildId());
        plr->SetGuildRank(plr->m_playerInfo->guildRank->iId);
    }

    info->m_loggedInPlayer = plr;

    // account data == UI config
    SendAccountDataTimes(PER_CHARACTER_CACHE_MASK);

    // Set TIME OF LOGIN
    CharacterDatabase.Execute("UPDATE characters SET online = 1 WHERE guid = %u" , plr->GetLowGUID());

    bool enter_world = true;

    // Find our transporter and add us if we're on one.
    if (plr->obj_movement_info.transporter_info.guid != 0)
    {
        Transporter* pTrans = objmgr.GetTransporter(Arcemu::Util::GUID_LOPART(plr->obj_movement_info.transporter_info.guid));
        if (pTrans)
        {
            if (plr->IsDead())
            {
                plr->ResurrectPlayer();
                plr->SetHealth(plr->GetMaxHealth());
                plr->SetPower(POWER_TYPE_MANA, plr->GetMaxPower(POWER_TYPE_MANA));
            }

            float c_tposx = pTrans->GetPositionX() + plr->GetTransPositionX();
            float c_tposy = pTrans->GetPositionY() + plr->GetTransPositionY();
            float c_tposz = pTrans->GetPositionZ() + plr->GetTransPositionZ();

            if (plr->GetMapId() != pTrans->GetMapId())       // loaded wrong map
            {
                plr->SetMapId(pTrans->GetMapId());

                StackWorldPacket<20> dataw(SMSG_NEW_WORLD);

                dataw << pTrans->GetMapId();
                dataw << c_tposx;
                dataw << c_tposy;
                dataw << c_tposz;
                dataw << plr->GetOrientation();

                SendPacket(&dataw);

                // shit is sent in worldport ack.
                enter_world = false;
            }

            plr->SetPosition(c_tposx, c_tposy, c_tposz, plr->GetOrientation(), false);
            pTrans->AddPassenger(plr);
        }
    }

    LOG_DEBUG("Player %s logged in.", plr->GetName());

    sWorld.incrementPlayerCount(plr->GetTeam());

    if (plr->m_FirstLogin && !worldConfig.player.skipCinematics)
    {
        uint32 introid = plr->info->introid;

        OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &introid);
    }

    LOG_DETAIL("WORLD: Created new player for existing players (%s)", plr->GetName());

    // Login time, will be used for played time calc
    plr->m_playedtime[2] = uint32(UNIXTIME);

    //Issue a message telling all guild members that this player has signed on
    if (plr->IsInGuild())
    {
        plr->SendGuildMOTD();
        plr->m_playerInfo->guild->LogGuildEvent(GE_SIGNED_ON, 1, plr->GetName());
    }

    // Send online status to people having this char in friendlist
    _player->Social_TellFriendsOnline();
    // send friend list (for ignores)
    _player->Social_SendFriendList(7);

#if VERSION_STRING != TBC
    plr->SendDungeonDifficulty();
    plr->SendRaidDifficulty();

    plr->SendEquipmentSetList();
#endif

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

        // Revision
        _player->BroadcastMessage("Build hash: %s%s", MSG_COLOR_CYAN, BUILD_HASH_STR);
        // Shows Online players, and connection peak
        _player->BroadcastMessage("Online Players: %s%u |rPeak: %s%u|r Accepted Connections: %s%u",
            MSG_COLOR_SEXGREEN, sWorld.getSessionCount(), MSG_COLOR_SEXBLUE, sWorld.getPeakSessionCount(), MSG_COLOR_SEXBLUE, sWorld.getAcceptedConnections());

        // Shows Server uptime
        _player->BroadcastMessage("Server Uptime: |r%s", sWorld.getWorldUptimeString().c_str());
    }

    // server Message Of The Day
    SendMOTD();

    //Set current RestState
    if (plr->m_isResting)
        // We are resting at an inn , turn on Zzz
        plr->ApplyPlayerRestState(true);

    //Calculate rest bonus if there is time between lastlogoff and now
    if (plr->m_timeLogoff > 0 && plr->getLevel() < plr->GetMaxLevel())    // if timelogoff = 0 then it's the first login
    {
        uint32 currenttime = uint32(UNIXTIME);
        uint32 timediff = currenttime - plr->m_timeLogoff;

        //Calculate rest bonus
        if (timediff > 0)
            plr->AddCalculatedRestXP(timediff);
    }

    if (info->m_Group)
        info->m_Group->Update();

    if (enter_world && !_player->GetMapMgr())
        plr->AddToWorld();

    sHookInterface.OnFullLogin(_player);

    objmgr.AddPlayer(_player);
}
#endif

#if VERSION_STRING > TBC
/// \todo port player to a main city of his new faction
void WorldSession::HandleCharFactionOrRaceChange(WorldPacket& recv_data)
{
    uint64 guid;
    std::string newname;
    uint8 gender;
    uint8 skin;
    uint8 face;
    uint8 hairStyle;
    uint8 hairColor;
    uint8 facialHair;
    uint8 race;

    recv_data >> guid;
    recv_data >> newname;
    recv_data >> gender;
    recv_data >> skin;
    recv_data >> hairColor;
    recv_data >> hairStyle;
    recv_data >> facialHair;
    recv_data >> face;
    recv_data >> race;

    uint8 _class = 0;
    PlayerInfo* info = objmgr.GetPlayerInfo(static_cast<uint32>(guid));

    if (info)
        _class = info->cl;
    else
    {
        WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
        data << uint8(E_CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    uint32 used_loginFlag = ((recv_data.GetOpcode() == CMSG_CHAR_RACE_CHANGE) ? LOGIN_CUSTOMIZE_RACE : LOGIN_CUSTOMIZE_FACTION);
    uint32 newflags = 0;

    QueryResult* query = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u", guid);
    if (query)
    {
        uint16 lflag = query->Fetch()[0].GetUInt16();
        if (!(lflag & used_loginFlag))
        {
            WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
            data << uint8(E_CHAR_CREATE_ERROR);
            SendPacket(&data);
            return;
        }
        newflags = lflag - used_loginFlag;
    }
    delete query;
    if (!sMySQLStore.getPlayerCreateInfo(race, info->cl))
    {
        WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
        data << uint8(E_CHAR_CREATE_ERROR);
        SendPacket(&data);
        return;
    }

    LoginErrorCode res = VerifyName(newname.c_str(), newname.length());
    if (res != E_CHAR_NAME_SUCCESS)
    {
        WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
        data << uint8(res);
        SendPacket(&data);
        return;
    }

    if (!HasGMPermissions())
    {
        QueryResult * result = CharacterDatabase.Query("SELECT COUNT(*) FROM `banned_names` WHERE name = '%s'", CharacterDatabase.EscapeString(newname).c_str());
        if (result)
        {
            if (result->Fetch()[0].GetUInt32() > 0)
            {
                WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
                data << uint8(E_CHAR_NAME_RESERVED);
                SendPacket(&data);
                return;
            }
            delete result;
        }
    }

    PlayerInfo* newinfo = objmgr.GetPlayerInfoByName(newname.c_str());
    if (newinfo != NULL && newinfo->guid != guid)
    {
        WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1);
        data << uint8(E_CHAR_CREATE_NAME_IN_USE);
        SendPacket(&data);
        return;
    }

    Player::CharChange_Looks(guid, gender, skin, face, hairStyle, hairColor, facialHair);
    //Player::CharChange_Language(guid, race);

    Util::CapitalizeString(newname);
    objmgr.RenamePlayerInfo(info, info->name, newname.c_str());
    CharacterDatabase.Execute("UPDATE `characters` set name = '%s', login_flags = %u, race = %u WHERE guid = '%u'", newname.c_str(), newflags, (uint32)race, (uint32)guid);

    //CharacterDatabase.WaitExecute("UPDATE `characters` SET login_flags = %u WHERE guid = '%u'", (uint32)LOGIN_NO_FLAG, (uint32)guid);
    WorldPacket data(SMSG_CHAR_FACTION_CHANGE, 1 + 8 + (newname.size() + 1) + 1 + 1 + 1 + 1 + 1 + 1 + 1);
    data << uint8(0);
    data << uint64(guid);
    data << newname;
    data << uint8(gender);
    data << uint8(skin);
    data << uint8(face);
    data << uint8(hairStyle);
    data << uint8(hairColor);
    data << uint8(facialHair);
    data << uint8(race);
    SendPacket(&data);
}
#endif

void WorldSession::HandleDeclinedPlayerNameOpcode(WorldPacket& recv_data)
{
    uint32_t error = 0;     // 0 = success, 1 = error

    uint64_t guid;
    std::string name;

    recv_data >> guid;
    recv_data >> name;

    //\todo check utf8 and cyrillic chars
    // check declined names

    WorldPacket data(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 4 + 8);
    data << uint32_t(error);
    data << uint64_t(guid);
    SendPacket(&data);
}
