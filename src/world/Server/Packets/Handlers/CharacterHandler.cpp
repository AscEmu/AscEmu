/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/WorldSession.h"
#include "Server/Packets/CmsgSetFactionAtWar.h"
#include "Server/Packets/CmsgSetFactionInactive.h"
#include "Units/Players/Player.h"
#include "Server/Packets/CmsgCharDelete.h"
#include "Server/Packets/SmsgCharDelete.h"
#include "Server/Packets/CmsgCharFactionChange.h"
#include "Server/Packets/SmsgCharFactionChange.h"
#include "Server/Packets/SmsgCharacterLoginFailed.h"
#include "Server/Packets/CmsgPlayerLogin.h"
#include "Server/Packets/CmsgCharRename.h"
#include "Server/Packets/SmsgCharRename.h"
#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Storage/MySQLDataStore.hpp"

using namespace AscEmu::Packets;

void WorldSession::handleSetFactionAtWarOpcode(WorldPacket& recvPacket)
{
    CmsgSetFactionAtWar recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    GetPlayer()->SetAtWar(recv_packet.id, recv_packet.state == 1);
}

void WorldSession::handleSetFactionInactiveOpcode(WorldPacket& recvPacket)
{
    CmsgSetFactionInactive recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    GetPlayer()->SetFactionInactive(recv_packet.id, recv_packet.state == 1);
}

void WorldSession::handleCharDeleteOpcode(WorldPacket& recvPacket)
{
    CmsgCharDelete recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const uint8_t deleteResult = DeleteCharacter(recv_packet.guid.getGuidLow());
    SendPacket(SmsgCharDelete(deleteResult).serialise().get());
}

#if VERSION_STRING > TBC
// \todo port player to a main city of his new faction
void WorldSession::handleCharFactionOrRaceChange(WorldPacket& recvPacket)
{
    CmsgCharFactionChange recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto playerInfoPacket = objmgr.GetPlayerInfo(recv_packet.guid.getGuidLow());
    if (playerInfoPacket == nullptr)
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
        return;
    }

    const uint32_t used_loginFlag = ((recvPacket.GetOpcode() == CMSG_CHAR_RACE_CHANGE) ? LOGIN_CUSTOMIZE_RACE : LOGIN_CUSTOMIZE_FACTION);
    uint32_t newflags = 0;

    const auto loginFlagsQuery = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u", recv_packet.guid.getGuidLow());
    if (loginFlagsQuery)
    {
        uint16_t loginFlags = loginFlagsQuery->Fetch()[0].GetUInt16();
        if (!(loginFlags & used_loginFlag))
        {
            SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
            return;
        }
        newflags = loginFlags - used_loginFlag;
    }

    if (!sMySQLStore.getPlayerCreateInfo(recv_packet.charCreate._race, playerInfoPacket->cl))
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
        return;
    }

    const auto loginErrorCode = VerifyName(recv_packet.charCreate.name.c_str(), recv_packet.charCreate.name.length());
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharFactionChange(loginErrorCode).serialise().get());
        return;
    }

    if (!HasGMPermissions())
    {
        const auto bannedNamesQuery = CharacterDatabase.Query("SELECT COUNT(*) FROM `banned_names` WHERE name = '%s'",
            CharacterDatabase.EscapeString(recv_packet.charCreate.name).c_str());
        if (bannedNamesQuery)
        {
            if (bannedNamesQuery->Fetch()[0].GetUInt32() > 0)
            {
                SendPacket(SmsgCharFactionChange(E_CHAR_NAME_RESERVED).serialise().get());
                return;
            }
        }
    }

    const auto playerInfo = objmgr.GetPlayerInfoByName(recv_packet.charCreate.name.c_str());
    if (playerInfo != nullptr && playerInfo->guid != recv_packet.guid.getGuidLow())
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_NAME_IN_USE).serialise().get());
        return;
    }

    Player::CharChange_Looks(recv_packet.guid, recv_packet.charCreate.gender, recv_packet.charCreate.skin,
        recv_packet.charCreate.face, recv_packet.charCreate.hairStyle, recv_packet.charCreate.hairColor, recv_packet.charCreate.facialHair);

    std::string newname = recv_packet.charCreate.name;
    Util::CapitalizeString(newname);

    objmgr.RenamePlayerInfo(playerInfoPacket, playerInfoPacket->name, newname.c_str());

    CharacterDatabase.Execute("UPDATE `characters` set name = '%s', login_flags = %u, race = %u WHERE guid = '%u'",
        newname.c_str(), newflags, static_cast<uint32_t>(recv_packet.charCreate._race), recv_packet.guid.getGuidLow());

    SendPacket(SmsgCharFactionChange(0, recv_packet.guid, recv_packet.charCreate).serialise().get());
}
#endif

void WorldSession::handlePlayerLoginOpcode(WorldPacket& recvPacket)
{
    CmsgPlayerLogin recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    LOG_DEBUG("Received CMSG_PLAYER_LOGIN %u (guidLow)", recv_packet.guid.getGuidLow());

    if (objmgr.GetPlayer(recv_packet.guid.getGuidLow()) != nullptr || m_loggingInPlayer || _player)
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_DUPLICATE_CHARACTER).serialise().get());
        return;
    }

    const auto query = new AsyncQuery(new SQLClassCallbackP0<WorldSession>(this, &WorldSession::loadPlayerFromDBProc));
    query->AddQuery("SELECT guid,class FROM characters WHERE guid = %u AND login_flags = %u", recv_packet.guid.getGuidLow(), static_cast<uint32_t>(LOGIN_NO_FLAG));
    CharacterDatabase.QueueAsyncQuery(query);
}

void WorldSession::handleCharRenameOpcode(WorldPacket& recvPacket)
{
    CmsgCharRename recv_packet;
    if (!recv_packet.deserialise(recvPacket))
        return;

    const auto playerInfo = objmgr.GetPlayerInfo(recv_packet.guid.getGuidLow());
    if (playerInfo == nullptr)
        return;

    QueryResult* result = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u AND acct = %u", recv_packet.guid.getGuidLow(), _accountId);
    if (result == nullptr)
        return;

    const auto loginErrorCode = VerifyName(recv_packet.name.c_str(), recv_packet.name.length());
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharRename(recv_packet.size, loginErrorCode, recv_packet.guid, recv_packet.name).serialise().get());
        return;
    }

    QueryResult* result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.EscapeString(recv_packet.name).c_str());
    if (result2)
    {
        if (result2->Fetch()[0].GetUInt32() > 0)
        {
            SendPacket(SmsgCharRename(recv_packet.size, E_CHAR_NAME_PROFANE, recv_packet.guid, recv_packet.name).serialise().get());
            return;
        }
    }

    if (objmgr.GetPlayerInfoByName(recv_packet.name.c_str()) != nullptr)
    {
        SendPacket(SmsgCharRename(recv_packet.size, E_CHAR_CREATE_NAME_IN_USE, recv_packet.guid, recv_packet.name).serialise().get());
        return;
    }

    std::string newName = recv_packet.name;
    Util::CapitalizeString(newName);
    objmgr.RenamePlayerInfo(playerInfo, playerInfo->name, newName.c_str());

    sPlrLog.writefromsession(this, "renamed character %s, %u (guid), to %s.", playerInfo->name, playerInfo->guid, newName.c_str());

    free(playerInfo->name);

    playerInfo->name = strdup(newName.c_str());

    CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s' WHERE guid = %u", newName.c_str(), recv_packet.guid.getGuidLow());
    CharacterDatabase.WaitExecute("UPDATE characters SET login_flags = %u WHERE guid = %u", static_cast<uint32_t>(LOGIN_NO_FLAG), recv_packet.guid.getGuidLow());

    SendPacket(SmsgCharRename(recv_packet.size, E_RESPONSE_SUCCESS, recv_packet.guid, newName).serialise().get());
}

void WorldSession::loadPlayerFromDBProc(QueryResultVector& results)
{
    if (results.empty())
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    QueryResult* result = results[0].result;
    if (result == nullptr)
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    Field* fields = result->Fetch();

    const uint64_t playerGuid = fields[0].GetUInt64();
    const uint8_t _class = fields[1].GetUInt8();

    Player* player = nullptr;
    switch (_class)
    {
        case WARRIOR:
            player = new Warrior(static_cast<uint32_t>(playerGuid));
            break;
        case PALADIN:
            player = new Paladin(static_cast<uint32_t>(playerGuid));
            break;
        case HUNTER:
            player = new Hunter(static_cast<uint32_t>(playerGuid));
            break;
        case ROGUE:
            player = new Rogue(static_cast<uint32_t>(playerGuid));
            break;
        case PRIEST:
            player = new Priest(static_cast<uint32_t>(playerGuid));
            break;
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
            player = new DeathKnight(static_cast<uint32_t>(playerGuid));
            break;
#endif
        case SHAMAN:
            player = new Shaman(static_cast<uint32_t>(playerGuid));
            break;
        case MAGE:
            player = new Mage(static_cast<uint32_t>(playerGuid));
            break;
        case WARLOCK:
            player = new Warlock(static_cast<uint32_t>(playerGuid));
            break;
        case DRUID:
            player = new Druid(static_cast<uint32_t>(playerGuid));
            break;
    }

    if (player == nullptr)
    {
        LOG_ERROR("Unknown class %u!", _class);
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    player->SetSession(this);

    m_bIsWLevelSet = false;

    LOG_DEBUG("Async loading player %u", static_cast<uint32_t>(playerGuid));
    m_loggingInPlayer = player;
    player->LoadFromDB(static_cast<uint32_t>(playerGuid));
}
