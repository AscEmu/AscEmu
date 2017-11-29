/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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

void WorldSession::CharacterEnumProc(QueryResult* result)
{
    struct PlayerItem
    {
        uint32_t displayId;
        uint8_t inventoryType;
        uint32_t enchantmentId;
    };

    PlayerItem player_items[INVENTORY_SLOT_BAG_END];

    struct CharEnumData
    {
        uint64_t guid;
        uint8_t level;
        uint8_t race;
        uint8_t Class;
        uint8_t gender;
        uint32_t bytes;
        uint32_t bytes2;
        std::string name;
        float x;
        float y;
        float z;
        uint32_t mapId;
        uint32_t zoneId;
        uint32_t banned;

        uint32_t deathState;
        uint32_t loginFlags;
        uint32_t flags;
        uint32_t guildId;

    };

    has_dk = false;
    _side = -1;

    auto startTime = Util::TimeNow();

    WorldPacket data(SMSG_CHAR_ENUM, 270);
    ByteBuffer buffer;

    data.writeBits(0, 23);
    data.writeBit(1);
    data.writeBits(result ? result->GetRowCount() : 0, 17);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            CharEnumData charEnum;

            uint32_t dbGuid = fields[0].GetUInt32();
            ObjectGuid guid = MAKE_NEW_GUID(dbGuid, 0, 0x000);
            charEnum.level = fields[1].GetUInt8();
            charEnum.race = fields[2].GetUInt8();
            charEnum.Class = fields[3].GetUInt8();
            charEnum.gender = fields[4].GetUInt8();

            charEnum.bytes = fields[5].GetUInt32();
            uint8_t skin = uint8_t(charEnum.bytes & 0xFF);
            uint8_t face = uint8_t((charEnum.bytes >> 8) & 0xFF);
            uint8_t hairStyle = uint8_t((charEnum.bytes >> 16) & 0xFF);
            uint8_t hairColor = uint8_t((charEnum.bytes >> 24) & 0xFF);
            uint8_t facialHair = uint8_t(charEnum.bytes & 0xFF);

            charEnum.bytes2 = fields[6].GetUInt32();
            charEnum.name = fields[7].GetString();
            charEnum.x = fields[8].GetFloat();
            charEnum.y = fields[9].GetFloat();
            charEnum.z = fields[10].GetFloat();
            charEnum.mapId = fields[11].GetUInt32();
            charEnum.zoneId = fields[12].GetUInt32();
            charEnum.banned = fields[13].GetUInt32();

            charEnum.deathState = fields[15].GetUInt32();
            charEnum.loginFlags = fields[16].GetUInt32();
            charEnum.flags = fields[17].GetUInt32();
            charEnum.guildId = fields[18].GetUInt32();
            ObjectGuid guildGuid = MAKE_NEW_GUID(charEnum.guildId, 0, HIGHGUID_TYPE_GUILD);

            if (_side < 0)
            {
                static uint8_t sides[NUM_RACES] = { 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                _side = sides[charEnum.race];
            }

            has_level_55_char = has_level_55_char || (charEnum.level >= 55);
            has_dk = has_dk || (charEnum.Class == DEATHKNIGHT);

            uint32_t char_flags = 0;

            if (charEnum.banned && (charEnum.banned < 10 || charEnum.banned > (uint32_t)UNIXTIME))
                char_flags |= PLAYER_FLAG_IS_BANNED;
            if (charEnum.deathState != 0)
                char_flags |= PLAYER_FLAG_IS_DEAD;
            if (charEnum.flags & PLAYER_FLAG_NOHELM)
                char_flags |= PLAYER_FLAG_NOHELM;
            if (charEnum.flags & PLAYER_FLAG_NOCLOAK)
                char_flags |= PLAYER_FLAG_NOCLOAK;
            if (charEnum.loginFlags == 1)
                char_flags |= PLAYER_FLAGS_RENAME_FIRST;

            data.writeBit(guid[3]);
            data.writeBit(guildGuid[1]);
            data.writeBit(guildGuid[7]);
            data.writeBit(guildGuid[2]);
            data.writeBits(uint32_t(charEnum.name.length()), 7);
            data.writeBit(guid[4]);
            data.writeBit(guid[7]);
            data.writeBit(guildGuid[3]);
            data.writeBit(guid[5]);
            data.writeBit(guildGuid[6]);
            data.writeBit(guid[1]);
            data.writeBit(guildGuid[5]);
            data.writeBit(guildGuid[4]);
            data.writeBit(charEnum.loginFlags & 0x20); // 0x20 = AT_LOGIN_FIRST
            data.writeBit(guid[0]);
            data.writeBit(guid[2]);
            data.writeBit(guid[6]);
            data.writeBit(guildGuid[0]);


            CreatureProperties const* petInfo = nullptr;
            uint32_t petDisplayId;
            uint32_t petLevel = 0;
            uint32_t petFamily;

            if (charEnum.Class == WARLOCK || charEnum.Class == HUNTER)
            {
                QueryResult* player_pet_db_result = CharacterDatabase.Query("SELECT entry, level FROM playerpets WHERE ownerguid = %u AND MOD(active, 10) = 1 AND alive = TRUE;", dbGuid);
                if (player_pet_db_result)
                {
                    petLevel = player_pet_db_result->Fetch()[1].GetUInt32();
                    petInfo = sMySQLStore.getCreatureProperties(player_pet_db_result->Fetch()[0].GetUInt32());
                    delete player_pet_db_result;
                }
            }

            if (petInfo != nullptr)
            {
                petDisplayId = petInfo->Male_DisplayID;
                petLevel = petLevel;
                petFamily = petInfo->Family;
            }
            else
            {
                petDisplayId = 0;
                petLevel = 0;
                petFamily = 0;
            }

            buffer << uint8_t(charEnum.Class);

            QueryResult* item_db_result = CharacterDatabase.Query("SELECT containerslot, slot, entry, enchantments FROM playeritems WHERE ownerguid=%u AND containerslot = '-1' AND slot < 23", dbGuid);

            memset(player_items, 0, sizeof(player_items));

            if (item_db_result)
            {
                do
                {
                    uint32_t enchantid;
                    uint32_t container_slot = item_db_result->Fetch()[0].GetInt32();
                    int8_t item_slot = item_db_result->Fetch()[1].GetInt8();
                    if (container_slot == -1 && item_slot < INVENTORY_SLOT_BAG_END && item_slot >= EQUIPMENT_SLOT_START)
                    {
                        ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_db_result->Fetch()[2].GetUInt32());
                        if (itemProperties)
                        {
                            if (!(item_slot == EQUIPMENT_SLOT_HEAD && (charEnum.flags & (uint32_t)PLAYER_FLAG_NOHELM) != 0) &&
                                !(item_slot == EQUIPMENT_SLOT_BACK && (charEnum.flags & (uint32_t)PLAYER_FLAG_NOCLOAK) != 0))
                            {
                                player_items[item_slot].displayId = itemProperties->DisplayInfoID;
                                player_items[item_slot].inventoryType = static_cast<uint8>(itemProperties->InventoryType);

                                if (item_slot == EQUIPMENT_SLOT_MAINHAND || item_slot == EQUIPMENT_SLOT_OFFHAND)
                                {
                                    const char* enchant_field = item_db_result->Fetch()[3].GetString();
                                    if (sscanf(enchant_field, "%u,0,0;", (unsigned int*)&enchantid) == 1 && enchantid > 0)
                                    {
                                        DBC::Structures::SpellItemEnchantmentEntry const* spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(enchantid);
                                        if (spell_item_enchant != nullptr)
                                            player_items[item_slot].enchantmentId = spell_item_enchant->visual;
                                        else
                                            player_items[item_slot].enchantmentId = 0;
                                    }
                                }
                            }
                        }
                        else
                        {
                            LOG_ERROR("Table player_items includes invalid item entry %u!", item_db_result->Fetch()[1].GetUInt32());
                        }
                    }
                } while (item_db_result->NextRow());
                delete item_db_result;
            }

            for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                buffer << uint8_t(player_items[i].inventoryType);
                buffer << uint32_t(player_items[i].displayId);
                buffer << uint32_t(player_items[i].enchantmentId);
            }

            buffer << uint32_t(petFamily);
            buffer.WriteByteSeq(guildGuid[2]);
            buffer << uint8_t(0);
            buffer << uint8_t(hairStyle);
            buffer.WriteByteSeq(guildGuid[3]);
            buffer << uint32_t(petDisplayId);
            buffer << uint32_t(char_flags);
            buffer << uint8_t(hairColor);
            buffer.WriteByteSeq(guid[4]);
            buffer << uint32_t(charEnum.mapId);
            buffer.WriteByteSeq(guildGuid[5]);
            buffer << float(charEnum.z);
            buffer.WriteByteSeq(guildGuid[6]);
            buffer << uint32_t(petLevel);
            buffer.WriteByteSeq(guid[3]);
            buffer << float(charEnum.y);

            switch (charEnum.loginFlags)
            {
                case LOGIN_CUSTOMIZE_LOOKS:
                    buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_CUSTOMIZE);    //Character recustomization flag
                    break;
                case LOGIN_CUSTOMIZE_RACE:
                    buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_RACE);         //Character recustomization + race flag
                    break;
                case LOGIN_CUSTOMIZE_FACTION:
                    buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_FACTION);      //Character recustomization + race + faction flag
                    break;
                default:
                    buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_NONE);         //Character recustomization no flag set
            }

            buffer << uint8_t(facialHair);
            buffer.WriteByteSeq(guid[7]);
            buffer << uint8_t(charEnum.gender);
            buffer.append(charEnum.name.c_str(), charEnum.name.length());
            buffer << uint8_t(face);
            buffer.WriteByteSeq(guid[0]);
            buffer.WriteByteSeq(guid[2]);
            buffer.WriteByteSeq(guildGuid[1]);
            buffer.WriteByteSeq(guildGuid[7]);
            buffer << float(charEnum.x);
            buffer << uint8_t(skin);
            buffer << uint8_t(charEnum.race);
            buffer << uint8_t(charEnum.level);
            buffer.WriteByteSeq(guid[6]);
            buffer.WriteByteSeq(guildGuid[4]);
            buffer.WriteByteSeq(guildGuid[0]);
            buffer.WriteByteSeq(guid[5]);
            buffer.WriteByteSeq(guid[1]);
            buffer << uint32_t(charEnum.zoneId);

        } while (result->NextRow());

        data.flushBits();
        data.append(buffer);
    }

    LogDebugFlag(LF_OPCODE, "Character Enum Built in %u ms.", Util::GetTimeDifferenceToNow(startTime));
    SendPacket(&data);
}

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
    LogDebug("WorldSession : Fully loading player %u", plr->GetLowGUID());

    SetPlayer(plr);

    m_MoverGuid = plr->GetGUID();
    m_MoverWoWGuid.Init(plr->GetGUID());

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

    PlayerInfo* info = objmgr.GetPlayerInfo(plr->GetLowGUID());
    if (info == nullptr)
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
        info->guildRank = GUILD_RANK_NONE;
        info->m_Group = nullptr;
        info->subGroup = 0;
        objmgr.AddPlayerInfo(info);
    }
    plr->m_playerInfo = info;

    info->m_loggedInPlayer = plr;

    SendAccountDataTimes(PER_CHARACTER_CACHE_MASK);

    CharacterDatabase.Execute("UPDATE characters SET online = 1 WHERE guid = %u", plr->GetLowGUID());

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
                plr->SetHealth(plr->GetMaxHealth());
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

    LOG_DEBUG("Player %s logged in.", plr->GetName());

    sWorld.incrementPlayerCount(plr->GetTeam());

    if (plr->m_FirstLogin && !sWorld.settings.player.skipCinematics)
    {
        uint32 introid = plr->info->introid;
        OutPacket(SMSG_TRIGGER_CINEMATIC, 4, &introid);
    }

    LOG_DETAIL("Created new player for existing players (%s)", plr->GetName());

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

    if (plr->m_timeLogoff > 0 && plr->getLevel() < plr->GetMaxLevel())
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
