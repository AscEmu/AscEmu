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

void WorldSession::CharacterEnumProc(QueryResult* result)
{
    struct PlayerItem
    {
        uint32_t displayId;
        uint8_t inventoryType;
        uint32_t enchantmentId;
    };

    PlayerItem player_items[20];

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

    _side = -1;

    uint32_t numchar = 0;
    uint8_t char_real_count = 0;

    if (result)
        numchar = result->GetRowCount();

    uint32_t start_time = Util::getMSTime();

    WorldPacket data(SMSG_CHAR_ENUM, 1 + numchar * 200);
    data << uint8_t(char_real_count);

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            CharEnumData charEnum;

            charEnum.guid = fields[0].GetUInt64();
            charEnum.level = fields[1].GetUInt8();
            charEnum.race = fields[2].GetUInt8();
            charEnum.Class = fields[3].GetUInt8();
            if (charEnum.Class == DEATHKNIGHT)
            {
                LogDebugFlag(LF_OPCODE, "Your character Table includes DeathKnights! You are running AscEmu for TBC clients - skip!", Util::getMSTime() - start_time);
                continue;
            }
            charEnum.gender = fields[4].GetUInt8();
            charEnum.bytes = fields[5].GetUInt32();
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

            if (_side < 0)
            {
                static uint8_t sides[RACE_DRAENEI + 1] = { 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0 };
                _side = sides[charEnum.race];
            }

            data << uint64_t(charEnum.guid);
            data << charEnum.name;
            data << uint8_t(charEnum.race);
            data << uint8_t(charEnum.Class);
            data << uint8_t(charEnum.gender);
            data << uint32_t(charEnum.bytes);
            data << uint8_t(charEnum.bytes2 & 0xFF);
            data << uint8_t(charEnum.level);
            data << uint32_t(charEnum.zoneId);
            data << uint32_t(charEnum.mapId);
            data << float(charEnum.x);
            data << float(charEnum.y);
            data << float(charEnum.z);
            data << uint32_t(charEnum.guildId);

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

            data << uint32_t(char_flags);
            data << uint8_t(0);

            CreatureProperties const* petInfo = nullptr;
            uint32_t petLevel = 0;

            if (charEnum.Class == WARLOCK || charEnum.Class == HUNTER)
            {
                QueryResult* player_pet_db_result = CharacterDatabase.Query("SELECT entry, level FROM playerpets WHERE ownerguid = %u AND MOD(active, 10) = 1 AND alive = TRUE;", Arcemu::Util::GUID_LOPART(charEnum.guid));
                if (player_pet_db_result)
                {
                    petLevel = player_pet_db_result->Fetch()[1].GetUInt32();
                    petInfo = sMySQLStore.getCreatureProperties(player_pet_db_result->Fetch()[0].GetUInt32());
                    delete player_pet_db_result;
                }
            }

            if (petInfo != nullptr)
            {
                data << uint32_t(petInfo->Male_DisplayID);
                data << uint32_t(petLevel);
                data << uint32_t(petInfo->Family);
            }
            else
            {
                data << uint32_t(0);
                data << uint32_t(0);
                data << uint32_t(0);
            }

            QueryResult* item_db_result = CharacterDatabase.Query("SELECT slot, entry, enchantments FROM playeritems WHERE ownerguid=%u AND containerslot = '-1' AND slot BETWEEN '0' AND '20'", Arcemu::Util::GUID_LOPART(charEnum.guid));
            memset(player_items, 0, sizeof(PlayerItem) * 20);

            if (item_db_result)
            {
                do
                {
                    uint32_t enchantid;

                    int8_t item_slot = item_db_result->Fetch()[0].GetInt8();
                    ItemProperties const* itemProperties = sMySQLStore.getItemProperties(item_db_result->Fetch()[1].GetUInt32());
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

            for (uint8_t i = 0; i < /*INVENTORY_SLOT_BAG_END*/ 20; ++i)
            {
                data << uint32_t(player_items[i].displayId);
                data << uint8_t(player_items[i].inventoryType);
                data << uint32_t(player_items[i].enchantmentId);
            }

            ++char_real_count;
        } while (result->NextRow());
    }

    data.put<uint8_t>(0, char_real_count);

    LogDebugFlag(LF_OPCODE, "Character Enum Built in %u ms.", Util::getMSTime() - start_time);
    SendPacket(&data);
}
