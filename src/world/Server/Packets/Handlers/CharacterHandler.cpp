/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Common.hpp"
#include "git_version.hpp"
#include "Chat/ChatDefines.hpp"
#include "Server/WorldSession.h"
#include "Server/Packets/CmsgSetFactionAtWar.h"
#include "Server/Packets/CmsgSetFactionInactive.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/Packets/CmsgCharDelete.h"
#include "Server/Packets/SmsgCharDelete.h"
#include "Server/Packets/CmsgCharFactionChange.h"
#include "Server/Packets/SmsgCharFactionChange.h"
#include "Server/Packets/SmsgCharacterLoginFailed.h"
#include "Server/Packets/CmsgPlayerLogin.h"
#include "Server/Packets/CmsgCharRename.h"
#include "Server/Packets/SmsgCharRename.h"
#include "Management/ObjectMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Server/Packets/SmsgCharCreate.h"
#include "Server/Packets/CmsgCharCreate.h"
#include "Server/Packets/CmsgCharCustomize.h"
#include "Server/Packets/SmsgCharCustomize.h"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/Packets/SmsgLearnedDanceMoves.h"
#include "Server/Packets/SmsgFeatureSystemStatus.h"
#include "Server/Packets/CmsgSetPlayerDeclinedNames.h"
#include "Server/Packets/SmsgSetPlayerDeclinedNamesResult.h"
#include "Server/Packets/SmsgEnumCharactersResult.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Server/CharacterErrors.h"
#include "Cryptography/AuthCodes.h"
#include "Logging/Logger.hpp"
#include "Management/ArenaTeam.hpp"
#include "Management/Charter.hpp"
#include "Objects/Units/Creatures/Corpse.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/OpcodeTable.hpp"
#include "Server/World.h"
#include "Server/WorldSessionLog.hpp"
#include "Server/Script/HookInterface.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Server/Script/ScriptMgr.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"

using namespace AscEmu::Packets;

CharacterErrorCodes VerifyName(utf8_string name)
{
    const std::size_t MIN_NAME_LENGTH = 2;
    const std::size_t MAX_NAME_LENGTH = 12;

    if(!AscEmu::Util::utf8::is_valid(name))
        return E_CHAR_NAME_INVALID_CHARACTER;

    if (worldConfig.server.enableLimitedNames)
    {
        if(!AscEmu::Util::utf8::is_valid(name))
            return E_CHAR_NAME_INVALID_CHARACTER;

        const std::size_t length = AscEmu::Util::utf8::length(name);

        if (length == 0)
            return E_CHAR_NAME_NO_NAME;

        if (length < MIN_NAME_LENGTH)
            return E_CHAR_NAME_TOO_SHORT;

        if (length > MAX_NAME_LENGTH)
            return E_CHAR_NAME_TOO_LONG;
    }
    return E_CHAR_NAME_SUCCESS;
}

void WorldSession::handleSetFactionAtWarOpcode(WorldPacket& recvPacket)
{
    CmsgSetFactionAtWar srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->setFactionAtWar(srlPacket.id, srlPacket.state == 1);
}

void WorldSession::handleSetFactionInactiveOpcode(WorldPacket& recvPacket)
{
    CmsgSetFactionInactive srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    _player->setFactionInactive(srlPacket.id, srlPacket.state == 1);
}

void WorldSession::handleCharDeleteOpcode(WorldPacket& recvPacket)
{
    CmsgCharDelete srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const uint8_t deleteResult = deleteCharacter(srlPacket.guid);
    SendPacket(SmsgCharDelete(deleteResult).serialise().get());
}


// \todo port player to a main city of his new faction
void WorldSession::handleCharFactionOrRaceChange(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgCharFactionChange srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerInfoPacket = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());
    if (playerInfoPacket == nullptr)
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
        return;
    }

    const auto opcode = sOpcodeTables.getInternalIdForHex(recvPacket.GetOpcode());
    const uint32_t used_loginFlag = ((opcode == CMSG_CHAR_RACE_CHANGE) ? LOGIN_CUSTOMIZE_RACE : LOGIN_CUSTOMIZE_FACTION);
    uint32_t newflags = 0;

    const auto loginFlagsQuery = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u", srlPacket.guid.getGuidLow());
    if (loginFlagsQuery)
    {
        uint16_t loginFlags = loginFlagsQuery->Fetch()[0].asUint16();
        if (!(loginFlags & used_loginFlag))
        {
            SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
            return;
        }
        newflags = loginFlags - used_loginFlag;
    }

    if (!sMySQLStore.getPlayerCreateInfo(srlPacket.charCreate._race, playerInfoPacket->cl))
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_ERROR).serialise().get());
        return;
    }

    const auto loginErrorCode = VerifyName(srlPacket.charCreate.name);
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharFactionChange(loginErrorCode).serialise().get());
        return;
    }

    if (!HasGMPermissions())
    {
        const auto bannedNamesQuery = CharacterDatabase.Query("SELECT COUNT(*) FROM `banned_names` WHERE name = '%s'",
            CharacterDatabase.EscapeString(srlPacket.charCreate.name).c_str());
        if (bannedNamesQuery)
        {
            if (bannedNamesQuery->Fetch()[0].asUint32() > 0)
            {
                SendPacket(SmsgCharFactionChange(E_CHAR_NAME_RESERVED).serialise().get());
                return;
            }
        }
    }

    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.charCreate.name);
    if (playerInfo != nullptr && playerInfo->guid != srlPacket.guid.getGuidLow())
    {
        SendPacket(SmsgCharFactionChange(E_CHAR_CREATE_NAME_IN_USE).serialise().get());
        return;
    }

    Player::changeLooks(srlPacket.guid, srlPacket.charCreate.gender, srlPacket.charCreate.skin,
        srlPacket.charCreate.face, srlPacket.charCreate.hairStyle, srlPacket.charCreate.hairColor, srlPacket.charCreate.facialHair);

    std::string newname = srlPacket.charCreate.name;
    AscEmu::Util::Strings::capitalize(newname);
    std::string oldName = playerInfoPacket->name;

    sObjectMgr.updateCachedCharacterInfoName(playerInfoPacket, newname);

    _player->setName(newname);

    CharacterDatabase.Execute("UPDATE `characters` set name = '%s', login_flags = %u, race = %u WHERE guid = %u",
        newname.c_str(), newflags, static_cast<uint32_t>(srlPacket.charCreate._race), srlPacket.guid.getGuidLow());

    SendPacket(SmsgCharFactionChange(0, srlPacket.guid, srlPacket.charCreate).serialise().get());
#endif
}

void WorldSession::handlePlayerLoginOpcode(WorldPacket& recvPacket)
{
    CmsgPlayerLogin srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    sLogger.debugFlag(AscEmu::Logging::LF_OPCODE, "Received CMSG_PLAYER_LOGIN {} (guidLow)", srlPacket.guid.getGuidLow());

    if (sObjectMgr.getPlayer(srlPacket.guid.getGuidLow()) != nullptr || m_loggingInPlayer || _player)
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_DUPLICATE_CHARACTER).serialise().get());
        return;
    }

    auto query = std::make_unique<AsyncQuery>(std::make_unique<SQLClassCallbackP0<WorldSession>>(this, &WorldSession::loadPlayerFromDBProc));
    query->AddQuery("SELECT guid,class FROM characters WHERE guid = %u AND login_flags = %u",
        srlPacket.guid.getGuidLow(), static_cast<uint32_t>(LOGIN_NO_FLAG));
    CharacterDatabase.QueueAsyncQuery(std::move(query));
}

void WorldSession::handleCharRenameOpcode(WorldPacket& recvPacket)
{
    CmsgCharRename srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto playerInfo = sObjectMgr.getCachedCharacterInfo(srlPacket.guid.getGuidLow());
    if (playerInfo == nullptr)
        return;

    auto result = CharacterDatabase.Query("SELECT login_flags FROM characters WHERE guid = %u AND acct = %u",
        srlPacket.guid.getGuidLow(), _accountId);
    if (result == nullptr)
        return;

    const auto loginErrorCode = VerifyName(srlPacket.name);
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharRename(srlPacket.size, loginErrorCode, srlPacket.guid, srlPacket.name).serialise().get());
        return;
    }

    auto result2 = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'",
        CharacterDatabase.EscapeString(srlPacket.name).c_str());
    if (result2)
    {
        if (result2->Fetch()[0].asUint32() > 0)
        {
            SendPacket(SmsgCharRename(srlPacket.size, E_CHAR_NAME_PROFANE, srlPacket.guid, srlPacket.name).serialise().get());
            return;
        }
    }

    if (sObjectMgr.getCachedCharacterInfoByName(srlPacket.name) != nullptr)
    {
        SendPacket(SmsgCharRename(srlPacket.size, E_CHAR_CREATE_NAME_IN_USE, srlPacket.guid, srlPacket.name).serialise().get());
        return;
    }

    std::string newName = srlPacket.name;
    AscEmu::Util::Strings::capitalize(newName);
    std::string oldName = playerInfo->name;
    sObjectMgr.updateCachedCharacterInfoName(playerInfo, newName);

    _player->setName(newName);

    sPlrLog.writefromsession(this, "renamed character %s, %u (guid), to %s.", oldName.c_str(), playerInfo->guid, newName.c_str());

    CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s' WHERE guid = %u",
        newName.c_str(), srlPacket.guid.getGuidLow());
    CharacterDatabase.WaitExecute("UPDATE characters SET login_flags = %u WHERE guid = %u",
        static_cast<uint32_t>(LOGIN_NO_FLAG), srlPacket.guid.getGuidLow());

    SendPacket(SmsgCharRename(srlPacket.size, E_RESPONSE_SUCCESS, srlPacket.guid, newName).serialise().get());
}

void WorldSession::loadPlayerFromDBProc(QueryResultVector& results)
{
    if (results.empty())
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    QueryResult* result = results[0].result.get();
    if (result == nullptr)
    {
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    Field* fields = result->Fetch();
    const uint32_t playerGuid = fields[0].asUint32();
    const uint8_t _class = fields[1].asUint8();

    Player* player = sObjectMgr.createPlayerByGuid(_class, playerGuid);

    if (player == nullptr)
    {
        sLogger.failure("Unknown class {}!", _class);
        SendPacket(SmsgCharacterLoginFailed(E_CHAR_LOGIN_NO_CHARACTER).serialise().get());
        return;
    }

    player->setSession(this);
    m_bIsWLevelSet = false;

    sLogger.debug("Async loading player {}", static_cast<uint32_t>(playerGuid));
    m_loggingInPlayer = player;
    player->loadFromDB(playerGuid);
}

uint8_t WorldSession::deleteCharacter(WoWGuid guid)
{
    const auto playerInfo = sObjectMgr.getCachedCharacterInfo(guid.getGuidLow());
    if (playerInfo != nullptr && sObjectMgr.getPlayer(playerInfo->guid) == nullptr)
    {
        auto result = CharacterDatabase.Query("SELECT name FROM characters WHERE guid = %u AND acct = %u", guid.getGuidLow(), _accountId);
        if (!result)
            return E_CHAR_DELETE_FAILED;

        std::string name = result->Fetch()[0].asCString();

        if (playerInfo->m_guild)
        {
            const auto guild = sGuildMgr.getGuildById(playerInfo->m_guild);
            if (guild != nullptr && guild->getLeaderGUID() == playerInfo->guid)
                return E_CHAR_DELETE_FAILED_GUILD_LEADER;

            if (guild != nullptr)
                guild->handleRemoveMember(this, playerInfo->guid);
        }

        for (uint8_t i = 0; i < NUM_CHARTER_TYPES; ++i)
        {
            if (const auto charter = sObjectMgr.getCharterByGuid(guid, static_cast<CharterTypes>(i)))
                charter->removeSignature(guid.getGuidLow());
        }


        for (uint8_t i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
        {
            const auto arenaTeam = sObjectMgr.getArenaTeamByGuid(guid.getGuidLow(), i);
            if (arenaTeam != nullptr && arenaTeam->m_leader == guid.getGuidLow())
                return E_CHAR_DELETE_FAILED_ARENA_CAPTAIN;

            if (arenaTeam != nullptr)
                arenaTeam->removeMember(playerInfo);
        }

        sPlrLog.writefromsession(this, "deleted character %s %u (guidLow))", name.c_str(), guid.getGuidLow());

        CharacterDatabase.WaitExecute("DELETE FROM characters WHERE guid = %u", guid.getGuidLow());

        const auto corpse = sObjectMgr.getCorpseByOwner(guid.getGuidLow());
        if (corpse)
            CharacterDatabase.Execute("DELETE FROM corpses WHERE guid = %u", corpse->getGuidLow());

        CharacterDatabase.Execute("DELETE FROM playeritems WHERE ownerguid=%u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE playerguid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerpets WHERE ownerguid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerpetspells WHERE ownerguid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM tutorials WHERE playerId = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM questlog WHERE player_guid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playercooldowns WHERE player_guid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM mailbox WHERE player_guid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM social_friends WHERE character_guid = %u OR friend_guid = %u",
            guid.getGuidLow(), guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM social_ignores WHERE character_guid = %u OR ignore_guid = %u",
            guid.getGuidLow(), guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM character_achievement WHERE guid = %u AND achievement NOT IN "
            "(457, 467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, "
            "1413, 1415, 1414, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1463, 1400, 456, 1402)",
            guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM character_achievement_progress WHERE guid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerspells WHERE GUID = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerdeletedspells WHERE GUID = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerreputations WHERE guid = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playerskills WHERE GUID = %u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playersummons WHERE ownerguid=%u", guid.getGuidLow());
        CharacterDatabase.Execute("DELETE FROM playersummonspells WHERE ownerguid=%u", guid.getGuidLow());

        sObjectMgr.deleteCachedCharacterInfo(guid.getGuidLow());
        return E_CHAR_DELETE_SUCCESS;
    }
    return E_CHAR_DELETE_FAILED;
}

void WorldSession::handleCharCreateOpcode(WorldPacket& recvPacket)
{
    CmsgCharCreate srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto loginErrorCode = VerifyName(srlPacket.createStruct.name);
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharCreate(loginErrorCode).serialise().get());
        return;
    }

    const auto isAllowed = sMySQLStore.isCharacterNameAllowed(srlPacket.createStruct.name);
    if (!isAllowed)
    {
        SendPacket(SmsgCharCreate(E_CHAR_NAME_PROFANE).serialise().get());
        return;
    }

    if (sObjectMgr.getCachedCharacterInfoByName(srlPacket.createStruct.name) != nullptr)
    {
        SendPacket(SmsgCharCreate(E_CHAR_CREATE_NAME_IN_USE).serialise().get());
        return;
    }

    const auto isValid = sHookInterface.OnNewCharacter(srlPacket.createStruct._race, srlPacket.createStruct._class,
        this, srlPacket.createStruct.name.c_str());
    if (!isValid)
    {
        SendPacket(SmsgCharCreate(E_CHAR_CREATE_ERROR).serialise().get());
        return;
    }

    const auto bannedNamesQuery = CharacterDatabase.Query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'",
        CharacterDatabase.EscapeString(srlPacket.createStruct.name).c_str());
    if (bannedNamesQuery)
    {
        if (bannedNamesQuery->Fetch()[0].asUint32() > 0)
        {
            SendPacket(SmsgCharCreate(E_CHAR_NAME_PROFANE).serialise().get());
            return;
        }
    }

#if VERSION_STRING > TBC
    if (worldConfig.player.deathKnightLimit && has_dk && srlPacket.createStruct._class == DEATHKNIGHT)
    {
        SendPacket(SmsgCharCreate(E_CHAR_CREATE_UNIQUE_CLASS_LIMIT).serialise().get());
        return;
    }
#endif

    const auto charactersQuery = CharacterDatabase.Query("SELECT COUNT(*) FROM characters WHERE acct = %u", GetAccountId());
    if (charactersQuery)
    {
        if (charactersQuery->Fetch()[0].asUint32() >= 10)
        {
            SendPacket(SmsgCharCreate(E_CHAR_CREATE_SERVER_LIMIT).serialise().get());
            return;
        }
    }

    const auto newPlayer = sObjectMgr.createPlayer(srlPacket.createStruct._class);
    newPlayer->setSession(this);

    if (!newPlayer->create(srlPacket.createStruct))
    {
        newPlayer->m_isReadyToBeRemoved = true;
        delete newPlayer;

        SendPacket(SmsgCharCreate(E_CHAR_CREATE_FAILED).serialise().get());
        return;
    }

    const auto realmType = sLogonCommHandler.getRealmType();
    if (!HasGMPermissions() && realmType == REALMTYPE_PVP && _side != 255 && !worldConfig.player.isCrossoverCharsCreationEnabled)
    {
        if ((newPlayer->isTeamAlliance() && _side == 1) || (newPlayer->isTeamHorde() && _side == 0))
        {
            newPlayer->m_isReadyToBeRemoved = true;
            delete newPlayer;

            SendPacket(SmsgCharCreate(E_CHAR_CREATE_PVP_TEAMS_VIOLATION).serialise().get());
            return;
        }
    }

#if VERSION_STRING > TBC
    if (worldConfig.player.deathKnightPreReq && !has_level_55_char && srlPacket.createStruct._class == DEATHKNIGHT)
    {
        newPlayer->m_isReadyToBeRemoved = true;
        delete newPlayer;

        SendPacket(SmsgCharCreate(E_CHAR_CREATE_LEVEL_REQUIREMENT).serialise().get());
        return;
    }
#endif

    newPlayer->unsetBanned();

    if (newPlayer->getClass() == WARLOCK)
    {
        newPlayer->addSummonSpell(416, 3110);
        newPlayer->addSummonSpell(417, 19505);
        newPlayer->addSummonSpell(1860, 3716);
        newPlayer->addSummonSpell(1863, 7814);
    }

    newPlayer->saveToDB(true);

    auto playerInfo = std::make_unique<CachedCharacterInfo>();
    playerInfo->guid = newPlayer->getGuidLow();
    utf8_string name = newPlayer->getName();
    AscEmu::Util::Strings::capitalize(name);
    playerInfo->name = name;
    playerInfo->cl = newPlayer->getClass();
    playerInfo->race = newPlayer->getRace();
    playerInfo->gender = newPlayer->getGender();
    playerInfo->acct = GetAccountId();
    playerInfo->m_Group = nullptr;
    playerInfo->subGroup = 0;
    playerInfo->team = newPlayer->getTeam();
    playerInfo->m_guild = 0;
    playerInfo->guildRank = GUILD_RANK_NONE;
    playerInfo->lastOnline = UNIXTIME;

    sObjectMgr.addCachedCharacterInfo(std::move(playerInfo));

    newPlayer->m_isReadyToBeRemoved = true;
    delete newPlayer;

    SendPacket(SmsgCharCreate(E_CHAR_CREATE_SUCCESS).serialise().get());

    sLogonCommHandler.updateAccountCount(GetAccountId(), 1);
}

void WorldSession::handleCharCustomizeLooksOpcode(WorldPacket& recvPacket)
{
#if VERSION_STRING > TBC
    CmsgCharCustomize srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    const auto loginErrorCode = VerifyName(srlPacket.createStruct.name);
    if (loginErrorCode != E_CHAR_NAME_SUCCESS)
    {
        SendPacket(SmsgCharCustomize(loginErrorCode).serialise().get());
        return;
    }

    auto result = CharacterDatabase.Query("SELECT COUNT(*) FROM `banned_names` WHERE name = '%s'",
        CharacterDatabase.EscapeString(srlPacket.createStruct.name).c_str());
    if (result)
    {
        if (result->Fetch()[0].asUint32() > 0)
        {
            SendPacket(SmsgCharCustomize(E_CHAR_NAME_PROFANE).serialise().get());
            return;
        }
    }

    const auto playerInfo = sObjectMgr.getCachedCharacterInfoByName(srlPacket.createStruct.name);
    if (playerInfo != nullptr && playerInfo->guid != srlPacket.guid.getGuidLow())
    {
        SendPacket(SmsgCharCustomize(E_CHAR_CREATE_NAME_IN_USE).serialise().get());
        return;
    }

    AscEmu::Util::Strings::capitalize(srlPacket.createStruct.name);

    CharacterDatabase.WaitExecute("UPDATE `characters` set name = '%s' WHERE guid = %u",
        srlPacket.createStruct.name.c_str(), srlPacket.guid.getGuidLow());
    CharacterDatabase.WaitExecute("UPDATE `characters` SET login_flags = %u WHERE guid = %u",
        static_cast<uint32_t>(LOGIN_NO_FLAG), srlPacket.guid.getGuidLow());

    Player::changeLooks(srlPacket.guid, srlPacket.createStruct.gender, srlPacket.createStruct.skin,
        srlPacket.createStruct.face, srlPacket.createStruct.hairStyle, srlPacket.createStruct.hairColor,
        srlPacket.createStruct.facialHair);

    SendPacket(SmsgCharCustomize(E_RESPONSE_SUCCESS, srlPacket.guid, srlPacket.createStruct).serialise().get());
#endif
}

void WorldSession::initGMMyMaster()
{
#ifndef GM_TICKET_MY_MASTER_COMPATIBLE
    GM_Ticket* ticket = sTicketMgr.getGMTicketByPlayer(_player->getGuid());
    if (ticket)
    {
        //Send status change to gm_sync_channel
        const auto channel = sChannelMgr.getChannel(sWorld.getGmClientChannel(), _player);
        if (channel)
        {
            std::stringstream ss;
            ss << "GmTicket:" << GM_TICKET_CHAT_OPCODE_ONLINESTATE;
            ss << ":" << ticket->guid;
            ss << ":1";
            channel->Say(_player, ss.str().c_str(), nullptr, true);
        }
    }
#endif
}

void WorldSession::sendServerStats()
{
    if (worldConfig.server.sendStatsOnJoin)
    {
        _player->broadcastMessage("Info: %sAscEmu %s/%s-%s-%s %s(www.ascemu.org)", MSG_COLOR_WHITE, AE_BUILD_HASH, CONFIG, AE_PLATFORM, AE_ARCHITECTURE, MSG_COLOR_SEXBLUE);
        _player->broadcastMessage("Online Players: %s%u |rPeak: %s%u|r Accepted Connections: %s%u", MSG_COLOR_SEXBLUE, 
                                   static_cast<uint32_t>(sWorld.getSessionCount()), MSG_COLOR_SEXBLUE, sWorld.getPeakSessionCount(), MSG_COLOR_SEXBLUE, sWorld.getAcceptedConnections());

        _player->broadcastMessage("Uptime: |r%s", sWorld.getWorldUptimeString().c_str());
    }
}

void WorldSession::fullLogin(Player* player)
{
    sLogger.debug("WorldSession : Fully loading player {}", player->getGuidLow());

    //////////////////////////////////////////////////////////////////////////////////////////
    // basic setup
    SetPlayer(player);
    m_MoverWoWGuid.Init(player->getGuid());

    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // maybe you logged out inside a bg
    player->logIntoBattleground();
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // start on GM Island or normal position for first login. Check out the config.
    player->setLoginPosition();
    //////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING > TBC
    //////////////////////////////////////////////////////////////////////////////////////////
    // send feature packet... mostly unknown content.
    SendPacket(SmsgFeatureSystemStatus(2, 0).serialise().get());
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // dance moves - unknown 2x uint32_t(0)
#if VERSION_STRING != Mop
    SendPacket(SmsgLearnedDanceMoves(0, 0).serialise().get());
#endif
    //////////////////////////////////////////////////////////////////////////////////////////
#endif

    //////////////////////////////////////////////////////////////////////////////////////////
    // hotfix data for cata
#if VERSION_STRING >= Cata
    //\todo send Hotfixdata
#endif
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // update/set attack speed - mostly 0 on login
    player->updateAttackSpeed();
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // set playerinfo - should be already set, just in case.
    player->setPlayerInfoIfNeeded();
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // account data times - since we just logged in, it is 0
    sendAccountDataTimes(PER_CHARACTER_CACHE_MASK);
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // if we are on a transport we need a lot more checks, otherwise the mapmgr complains
    const bool canEnterWorld = player->logOntoTransport();
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // set db, time and count - our db now knows that we are online.
    CharacterDatabase.Execute("UPDATE characters SET online = 1 WHERE guid = %u", player->getGuidLow());
    sLogger.debug("Player {} logged in.", player->getName());
    sWorld.incrementPlayerCount(player->getTeam());

    player->m_playedTime[2] = uint32_t(UNIXTIME);
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // send cinematic on first login if we still allow it in the config
    player->sendCinematicOnFirstLogin();
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // send social packets and lists
    player->sendFriendStatus(true);
    player->sendFriendLists(7);

    //////////////////////////////////////////////////////////////////////////////////////////
    // dungeon and raid setup
#if VERSION_STRING > TBC
    player->sendDungeonDifficultyPacket();
    player->sendRaidDifficultyPacket();
#endif
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // Send Equipment set list - not sure what the intend was here.
#if VERSION_STRING < Cata
    player->sendEquipmentSetList();
#endif
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // GMMyMaster is a custom addon, we send special chat messages to trigger it.
    // send serverstats (uptime, playerpeak,..)
    // server Message of the day from config (Welcome to the world of warcraft)

    initGMMyMaster();

    sendMOTD();

    sendServerStats();

#if VERSION_STRING == Mop
    std::string timeZone = "Etc/UTC";

    WorldPacket data(SMSG_SET_TIME_ZONE_INFORMATION, 2 + timeZone.length() * 2);
    data.writeBits(timeZone.length(), 7);
    data.writeBits(timeZone.length(), 7);
    data.flushBits();
    data.WriteString(timeZone);
    data.WriteString(timeZone);
    SendPacket(&data);

    data.Initialize(SMSG_HOTFIX_NOTIFY_BLOB);
    data.writeBits(0, 20);
    data.flushBits();
    SendPacket(&data);
#endif

    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////
    // the restxp is calculated with our offline time
    if (player->m_isResting)
        player->applyPlayerRestState(true);

    if (player->m_timeLogoff > 0 && player->getLevel() < player->getMaxLevel())
    {
        const uint32_t currenttime = uint32_t(UNIXTIME);
        const uint32_t timediff = currenttime - player->m_timeLogoff;

        if (timediff > 0)
            player->addCalculatedRestXp(timediff);
    }
    //////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////

    // Make sure CompleteLoading is always called
    // Without this if player is entering to a faulty map it would not be ever called
    player->setEnteringToWorld();

    // add us to the world if we are not already added
    if (canEnterWorld && !player->getWorldMap())
        player->AddToWorld();
    //////////////////////////////////////////////////////////////////////////////////////////

#if VERSION_STRING == Mop
    {
        WorldPacket packet(SMSG_SETUP_CURRENCY, 4);
        packet.writeBits(0, 21);
        packet.flushBits();
        player->sendPacket(&packet);
    }
#endif

    sHookInterface.OnFullLogin(player);

    sObjectMgr.addPlayer(player);
}

void WorldSession::handleSetPlayerDeclinedNamesOpcode(WorldPacket& recvPacket)
{
    CmsgSetPlayerDeclinedNames srlPacket;
    if (!srlPacket.deserialise(recvPacket))
        return;

    //\todo check utf8 and cyrillic chars
    const uint32_t error = 0;     // 0 = success, 1 = error

    SendPacket(SmsgSetPlayerDeclinedNamesResult(error, srlPacket.guid).serialise().get());
}

void WorldSession::characterEnumProc(QueryResult* result)
{
    std::vector<CharEnumData> enumData;

#if VERSION_STRING > TBC
    has_dk = false;
#endif
    _side = 255;

    uint8_t charRealCount = 0;

    const auto startTime = Util::TimeNow();

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            CharEnumData charEnum;

            charEnum.guid = fields[0].asUint64();
            charEnum.level = fields[1].asUint8();
            charEnum.race = fields[2].asUint8();
            charEnum.Class = fields[3].asUint8();

            if (!isClassRaceCombinationPossible(charEnum.Class, charEnum.race))
            {
                sLogger.debug("Class {} and race {} is not a valid combination for Version {} - skipped",
                    charEnum.Class, charEnum.race, VERSION_STRING);
                continue;
            }

            charEnum.gender = fields[4].asUint8();
            charEnum.bytes = fields[5].asUint32();
            charEnum.bytes2 = fields[6].asUint32();
            charEnum.name = fields[7].asCString();
            charEnum.x = fields[8].asFloat();
            charEnum.y = fields[9].asFloat();
            charEnum.z = fields[10].asFloat();
            charEnum.mapId = fields[11].asUint32();
            charEnum.zoneId = fields[12].asUint32();
            charEnum.banned = fields[13].asUint32();

            charEnum.deathState = fields[15].asUint32();
            charEnum.loginFlags = fields[16].asUint32();
            charEnum.flags = fields[17].asUint32();
            charEnum.guildId = fields[18].asUint32();

            if (_side == 255)
                _side = getSideByRace(charEnum.race);

#if VERSION_STRING >= WotLK
            has_level_55_char = has_level_55_char || (charEnum.level >= 55);
            has_dk = has_dk || (charEnum.Class == DEATHKNIGHT);
#endif

            charEnum.char_flags = 0;

            if (charEnum.banned && (charEnum.banned < 10 || charEnum.banned >static_cast<uint32_t>(UNIXTIME)))
                charEnum.char_flags |= CHARACTER_SCREEN_FLAG_BANNED;
            if (charEnum.deathState != 0)
                charEnum.char_flags |= CHARACTER_SCREEN_FLAG_DEAD;
            if (charEnum.flags & PLAYER_FLAG_NOHELM)
                charEnum.char_flags |= CHARACTER_SCREEN_FLAG_HIDE_HELM;
            if (charEnum.flags & PLAYER_FLAG_NOCLOAK)
                charEnum.char_flags |= CHARACTER_SCREEN_FLAG_HIDE_CLOAK;
            if (charEnum.loginFlags == 1)
                charEnum.char_flags |= CHARACTER_SCREEN_FLAG_FORCED_RENAME;

#if VERSION_STRING >= WotLK
            switch (charEnum.loginFlags)
            {
                case LOGIN_CUSTOMIZE_LOOKS:
                    charEnum.customization_flag = CHAR_CUSTOMIZE_FLAG_CUSTOMIZE;
                    break;
                case LOGIN_CUSTOMIZE_RACE:
                    charEnum.customization_flag = CHAR_CUSTOMIZE_FLAG_RACE;
                    break;
                case LOGIN_CUSTOMIZE_FACTION:
                    charEnum.customization_flag = CHAR_CUSTOMIZE_FLAG_FACTION;
                    break;
                default:
                    charEnum.customization_flag = CHAR_CUSTOMIZE_FLAG_NONE;
            }
#endif

            charEnum.pet_data.display_id = 0;
            charEnum.pet_data.level = 0;
            charEnum.pet_data.family = 0;

            if (charEnum.Class == WARLOCK || charEnum.Class == HUNTER
#if VERSION_STRING >= WotLK
                || charEnum.Class == DEATHKNIGHT || charEnum.Class == MAGE
#endif
                )
            {
                auto player_pet_db_result = CharacterDatabase.Query("SELECT entry, model, level FROM playerpets WHERE ownerguid = %u "
                    "AND active = TRUE AND alive = TRUE LIMIT 1;", WoWGuid::getGuidLowPartFromUInt64(charEnum.guid));
                if (player_pet_db_result)
                {
                    if (const auto petInfo = sMySQLStore.getCreatureProperties(player_pet_db_result->Fetch()[0].asUint32()))
                    {
                        charEnum.pet_data.display_id = player_pet_db_result->Fetch()[1].asUint32();
                        charEnum.pet_data.level = player_pet_db_result->Fetch()[2].asUint32();
                        charEnum.pet_data.family = petInfo->Family;
                    }
                }
            }

            auto item_db_result = CharacterDatabase.Query("SELECT slot, entry, enchantments FROM playeritems "
                "WHERE ownerguid=%u AND containerslot = '-1' AND slot BETWEEN '0' AND '22'",
                WoWGuid::getGuidLowPartFromUInt64(charEnum.guid));

            memset(charEnum.player_items, 0, sizeof(PlayerItem) * INVENTORY_SLOT_BAG_END);

            if (item_db_result)
            {
                do
                {
                    int8_t item_slot = item_db_result->Fetch()[0].asInt8();
                    const auto itemProperties = sMySQLStore.getItemProperties(item_db_result->Fetch()[1].asUint32());
                    if (itemProperties)
                    {
                        charEnum.player_items[item_slot].displayId = itemProperties->DisplayInfoID;
                        charEnum.player_items[item_slot].inventoryType = static_cast<uint8_t>(itemProperties->InventoryType);

                        std::string enchant_field = item_db_result->Fetch()[2].asCString();
                        if (!enchant_field.empty())
                        {
                            std::vector<std::string> enchants = AscEmu::Util::Strings::split(enchant_field, ";");
                            uint32_t enchant_id;
                            uint32_t enchslot;

                            for (auto& enchant : enchants)
                            {
                                if (sscanf(enchant.c_str(), "%u,0,%u", &enchant_id, &enchslot) == 2)
                                {
#if VERSION_STRING == Cata
                                    if (enchslot == TRANSMOGRIFY_ENCHANTMENT_SLOT)
                                    {
                                        const auto itemProperties = sMySQLStore.getItemProperties(enchant_id);
                                        if (itemProperties)
                                            charEnum.player_items[item_slot].displayId = itemProperties->DisplayInfoID;
                                    }
#endif
                                    // Only Display Perm Enchants on Weapons
                                    if ((item_slot == EQUIPMENT_SLOT_MAINHAND || item_slot == EQUIPMENT_SLOT_OFFHAND) && enchslot == PERM_ENCHANTMENT_SLOT)
                                    {
                                        const auto spellItemEnchantmentEntry = sSpellItemEnchantmentStore.lookupEntry(enchant_id);
                                        if (spellItemEnchantmentEntry != nullptr)
                                            charEnum.player_items[item_slot].enchantmentId = spellItemEnchantmentEntry->visual;
                                    }
                                }
                            }
                        }
                    }
                } while (item_db_result->NextRow());
            }

            // save data to serialize it in packet serialisation SmsgEnumCharactersResult.
            enumData.push_back(charEnum);

            ++charRealCount;
        } while (result->NextRow());
    }

    sLogger.debug("Character Enum Built in {} ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    SendPacket(SmsgEnumCharactersResult(charRealCount, enumData).serialise().get());
}

void WorldSession::handleCharEnumOpcode(WorldPacket& /*recvPacket*/)
{
    auto asyncQuery = std::make_unique<AsyncQuery>(std::make_unique<SQLClassCallbackP1<World, uint32_t>>(&sWorld,
        &World::sendCharacterEnumToAccountSession, GetAccountId()));

    asyncQuery->AddQuery("SELECT guid, level, race, class, gender, bytes, bytes2, name, positionX, positionY, "
        "positionZ, mapId, zoneId, banned, restState, deathstate, login_flags, player_flags, guild_members.guildId "
        "FROM characters LEFT JOIN guild_members ON characters.guid = guild_members.playerid WHERE acct=%u ORDER BY guid LIMIT 10",
        GetAccountId());

    CharacterDatabase.QueueAsyncQuery(std::move(asyncQuery));
}

void WorldSession::loadAccountDataProc(QueryResult* result)
{
    if (result == nullptr)
    {
        CharacterDatabase.Execute("INSERT INTO account_data VALUES(%u, '', '', '', '', '', '', '', '', '')", _accountId);
        return;
    }

    for (uint8_t i = 0; i < 7; ++i)
    {
        const char* accountData = result->Fetch()[1 + i].asCString();
        const size_t length = accountData ? strlen(accountData) : 0;
        if (length > 1)
        {
            auto d = std::make_unique<char[]>(length + 1);
            memcpy(d.get(), accountData, length + 1);
            SetAccountData(i, std::move(d), true, static_cast<uint32_t>(length));
        }
    }
}
