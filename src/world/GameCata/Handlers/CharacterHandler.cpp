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

    uint32_t numchar = 0;

    uint32_t start_time = getMSTime();

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

            ObjectGuid guid = MAKE_NEW_GUID(fields[0].GetUInt32(), 0, 0x000);
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
            ObjectGuid guildGuid = MAKE_NEW_GUID(charEnum.guildId, 0, charEnum.guildId ? uint32_t(0x1FF) : 0);

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
            uint32_t petLevel;
            uint32_t petFamily;

            if (charEnum.Class == WARLOCK || charEnum.Class == HUNTER)
            {
                QueryResult* player_pet_db_result = CharacterDatabase.Query("SELECT entry, level FROM playerpets WHERE ownerguid = %u AND MOD(active, 10) = 1 AND alive = TRUE;", Arcemu::Util::GUID_LOPART(charEnum.guid));
                if (player_pet_db_result)
                {
                    petLevel = player_pet_db_result->Fetch()[1].GetUInt32();
                    petInfo = sMySQLStore.GetCreatureProperties(player_pet_db_result->Fetch()[0].GetUInt32());
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

            QueryResult* item_db_result = CharacterDatabase.Query("SELECT slot, entry, enchantments FROM playeritems WHERE ownerguid=%u AND containerslot = '-1' AND slot BETWEEN '0' AND '20'", Arcemu::Util::GUID_LOPART(charEnum.guid));
            memset(player_items, 0, sizeof(PlayerItem) * INVENTORY_SLOT_BAG_END);

            if (item_db_result)
            {
                do
                {
                    uint32_t enchantid;

                    int8_t item_slot = item_db_result->Fetch()[0].GetInt8();
                    ItemProperties const* itemProperties = sMySQLStore.GetItemProperties(item_db_result->Fetch()[1].GetUInt32());
                    if (itemProperties)
                    {
                        player_items[item_slot].displayId = itemProperties->DisplayInfoID;
                        player_items[item_slot].inventoryType = static_cast<uint8>(itemProperties->InventoryType);

                        if (item_slot == EQUIPMENT_SLOT_MAINHAND || item_slot == EQUIPMENT_SLOT_OFFHAND)
                        {
                            const char* enchant_field = item_db_result->Fetch()[2].GetString();
                            if (sscanf(enchant_field, "%u,0,0;", (unsigned int*)&enchantid) == 1 && enchantid > 0)
                            {
                                DBC::Structures::SpellItemEnchantmentEntry const* spell_item_enchant = sSpellItemEnchantmentStore.LookupEntry(enchantid);
                                if (spell_item_enchant != nullptr)
                                    player_items[item_slot].enchantmentId = spell_item_enchant->visual;
                            }
                        }
                    }
                } while (item_db_result->NextRow());
                delete item_db_result;
            }

            for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
            {
                buffer << uint32_t(player_items[i].displayId);
                buffer << uint8_t(player_items[i].inventoryType);
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

    LogDebugFlag(LF_OPCODE, "Character Enum Built in %u ms.", getMSTime() - start_time);
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
