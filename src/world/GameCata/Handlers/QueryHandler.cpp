/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"

void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recvData*/)
{
    Corpse* pCorpse = objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID());
    if (pCorpse == nullptr)
    {
        WorldPacket data(MSG_CORPSE_QUERY, 1);
        data << uint8_t(0);                         // no coprse for player
        SendPacket(&data);
    }
    else
    {
        WorldPacket data(MSG_CORPSE_QUERY, 25);
        data << uint8_t(1);                         // corpse found

        uint32_t corpsemap;
        uint32_t repopmap;
        float x, y, z;

        MySQLStructure::MapInfo const* pMapinfo = sMySQLStore.getWorldMapInfo(pCorpse->GetMapId());
        if (pMapinfo)
        {
            if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
            {
                repopmap = pCorpse->GetMapId();     // mapid (that tombstones shown on)
                x = pCorpse->GetPositionX();
                y = pCorpse->GetPositionY();
                z = pCorpse->GetPositionZ();
                corpsemap = pCorpse->GetMapId();    // instance mapid (needs to be same as mapid to be able to recover corpse)
            }
            else
            {
                repopmap = pMapinfo->repopmapid;    // mapid (that tombstones shown on)
                x = pMapinfo->repopx;
                y = pMapinfo->repopy;
                z = pMapinfo->repopz;
                corpsemap = pCorpse->GetMapId();    // instance mapid (needs to be same as mapid to be able to recover corpse)
            }
        }
        else
        {
            repopmap = pCorpse->GetMapId();         // mapid (that tombstones shown on)
            x = pCorpse->GetPositionX();
            y = pCorpse->GetPositionY();
            z = pCorpse->GetPositionZ();
            corpsemap = pCorpse->GetMapId();        // instance mapid (needs to be same as mapid to be able to recover corpse)
        }

        data << uint32_t(repopmap);
        data << float(x);
        data << float(y);
        data << float(z);
        data << uint32_t(corpsemap);
        data << uint32_t(0);                        // unk
        SendPacket(&data);
    }
}

void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket& /*recvData*/)
{
    uint32_t count = 0;

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 4 + 8 + 4);
    data << uint32_t(count);

    for (const auto& itr : _player->getInRangeObjectsSet())
    {
        if (!itr || !itr->IsCreature())
            continue;

        Creature* pCreature = static_cast<Creature*>(itr);
        if (pCreature->isQuestGiver())
        {
            data << uint64_t(pCreature->GetGUID());
            data << uint32_t(sQuestMgr.CalcStatus(pCreature, _player));
            ++count;
        }
    }

    data.put<uint32_t>(0, count);
    SendPacket(&data);
}

void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recvData)
{
    uint32_t entry;
    uint64_t guid;

    recvData >> entry;
    recvData >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 250);
    CreatureProperties const* ci = sMySQLStore.getCreatureProperties(entry);
    if (ci != nullptr)
    {
        MySQLStructure::LocalesCreature const* lcn = (language > 0) ? sMySQLStore.getLocalizedCreature(entry, language) : nullptr;
        data << uint32_t(entry);
        data << (lcn ? lcn->name : ci->Name);

        for (int i = 0; i < 7; ++i)
        {
            data << uint8_t(0);       // unk
        }

        data << (lcn ? lcn->subName : ci->SubName);
        data << ci->info_str;
        data << uint32_t(ci->typeFlags);
        data << uint32_t(0);                  // unk set 4 times with 1
        data << uint32_t(ci->Type);
        data << uint32_t(ci->Family);
        data << uint32_t(ci->Rank);
        data << uint32_t(ci->killcredit[0]);
        data << uint32_t(ci->killcredit[1]);
        data << uint32_t(ci->Male_DisplayID);
        data << uint32_t(ci->Female_DisplayID);
        data << uint32_t(ci->Male_DisplayID2);
        data << uint32_t(ci->Female_DisplayID2);
        data << float(ci->baseAttackMod);
        data << float(ci->rangeAttackMod);
        data << uint8_t(ci->Leader);

        for (uint8_t i = 0; i < 6; ++i)
        {
            data << uint32_t(ci->QuestItems[i]);
        }

        data << uint32_t(ci->waypointid);
        data << uint32_t(0);                  // unk
    }
    else
    {
        WorldPacket worldPacket(SMSG_CREATURE_QUERY_RESPONSE, 4);
        worldPacket << uint32_t(entry | 0x80000000);
    }

    SendPacket(&data);
}

//\todo Rewrite for cata - after this all functions are copied from wotlk

void WorldSession::HandleNameQueryOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);
    uint64 guid;
    recv_data >> guid;

    PlayerInfo* pn = objmgr.GetPlayerInfo(static_cast<uint32>(guid));

    if (!pn)
        return;

    LOG_DEBUG("Received CMSG_NAME_QUERY for: %s", pn->name);

    WoWGuid pguid(static_cast<uint64>(pn->guid)); //VLack: The usual new style guid handling on 3.1.2
    WorldPacket data(SMSG_NAME_QUERY_RESPONSE, strlen(pn->name) + 35);
    //    data << pn->guid << uint32(0);    //highguid
    data << pguid;
    data << uint8(0); //VLack: usual, new-style guid with an uint8
    data << pn->name;
    data << uint8(0);       // this is a string showed besides players name (eg. in combat log), a custom title ?
    data << uint8(pn->race);
    data << uint8(pn->gender);
    data << uint8(pn->cl);
    //    data << uint8(0);            // 2.4.0, why do i get the feeling blizz is adding custom classes or custom titles? (same thing in who list)
    data << uint8(0); //VLack: tell the server this name is not declined... (3.1 fix?)
    SendPacket(&data);
}

void WorldSession::HandleQueryTimeOpcode(WorldPacket& /*recvData*/)
{
    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4 + 4);
    data << static_cast<uint32>(UNIXTIME);
    data << static_cast<uint32>(0); //VLack: 3.1; thanks Stewart for reminding me to have the correct structure even if it seems the old one still works.
    SendPacket(&data);

}

void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 12);
    WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 900);

    uint32 entryID;
    uint64 guid;

    recvData >> entryID;
    recvData >> guid;

    LOG_DETAIL("WORLD: CMSG_GAMEOBJECT_QUERY '%u'", entryID);

    auto gameobject_info = sMySQLStore.getGameObjectProperties(entryID);
    if (gameobject_info == nullptr)
    {
        return;
    }

    MySQLStructure::LocalesGameobject const* lgn = (language > 0) ? sMySQLStore.getLocalizedGameobject(entryID, language) : nullptr;

    data << entryID;                        // unique identifier of the GO template
    data << gameobject_info->type;          // type of the gameobject
    data << gameobject_info->display_id;    // displayid/modelid of the gameobject

                                            // Name of the gameobject
    if (lgn != nullptr)
    {
        data << lgn->name;
    }
    else
    {
        data << gameobject_info->name;
    }

    data << uint8(0);               // name2, always seems to be empty
    data << uint8(0);               // name3, always seems to be empty
    data << uint8(0);               // name4, always seems to be empty
    data << gameobject_info->category_name;  // Category string of the GO, like "attack", "pvp", "point", etc
    data << gameobject_info->cast_bar_text;  // text displayed when using the go, like "collecting", "summoning" etc
    data << gameobject_info->Unkstr;
    data << gameobject_info->raw.parameter_0;     // spellfocus id, ex.: spell casted when interacting with the GO
    data << gameobject_info->raw.parameter_1;
    data << gameobject_info->raw.parameter_2;
    data << gameobject_info->raw.parameter_3;
    data << gameobject_info->raw.parameter_4;
    data << gameobject_info->raw.parameter_5;
    data << gameobject_info->raw.parameter_6;
    data << gameobject_info->raw.parameter_7;
    data << gameobject_info->raw.parameter_8;
    data << gameobject_info->raw.parameter_9;
    data << gameobject_info->raw.parameter_10;
    data << gameobject_info->raw.parameter_11;
    data << gameobject_info->raw.parameter_12;
    data << gameobject_info->raw.parameter_13;
    data << gameobject_info->raw.parameter_14;
    data << gameobject_info->raw.parameter_15;
    data << gameobject_info->raw.parameter_16;
    data << gameobject_info->raw.parameter_17;
    data << gameobject_info->raw.parameter_18;
    data << gameobject_info->raw.parameter_19;
    data << gameobject_info->raw.parameter_20;
    data << gameobject_info->raw.parameter_21;
    data << gameobject_info->raw.parameter_22;
    data << gameobject_info->raw.parameter_23;
    data << float(gameobject_info->size);       // scaling of the GO

                                                // questitems that the go can contain
    for (uint8 i = 0; i < 6; ++i)
    {
        data << uint32(gameobject_info->QuestItems[i]);
    }

    SendPacket(&data);
}

void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 4);
    uint32 pageid = 0;
    recvData >> pageid;

    while (pageid)
    {
        MySQLStructure::ItemPage const* page = sMySQLStore.getItemPage(pageid);
        if (page == nullptr)
        {
            return;
        }

        MySQLStructure::LocalesItemPages const* lpi = (language > 0) ? sMySQLStore.getLocalizedItemPages(pageid, language) : nullptr;

        WorldPacket data(SMSG_PAGE_TEXT_QUERY_RESPONSE, 1000);
        data << pageid;

        if (lpi != nullptr)
        {
            data << lpi->text;
        }
        else
        {
            data << page->text;
        }

        data << page->nextPage;
        pageid = page->nextPage;
        SendPacket(&data);
    }
}

void WorldSession::HandleItemNameQueryOpcode(WorldPacket& recvData)
{
    CHECK_PACKET_SIZE(recvData, 4);

    uint32 itemid;

    recvData >> itemid;
    recvData.read_skip<uint64>();

    WorldPacket reply(SMSG_ITEM_NAME_QUERY_RESPONSE, 100);
    reply << itemid;

    std::string Name = ("Unknown Item");

    ItemProperties const* proto = sMySQLStore.getItemProperties(itemid);
    if (proto != nullptr)
    {
        MySQLStructure::LocalesItem const* li = (language > 0) ? sMySQLStore.getLocalizedItem(itemid, language) : nullptr;
        if (li != nullptr)
        {
            Name = li->name;
        }
        else
        {
            Name = proto->Name;
        }

        reply << Name;
        reply << proto->InventoryType;
    }
    else
    {
        reply << Name;
    }

    SendPacket(&reply);
}

void WorldSession::HandleAchievmentQueryOpcode(WorldPacket& recvData)
{
    uint64 guid = recvData.unpackGUID();               // Get the inspectee's GUID
    Player* pTarget = objmgr.GetPlayer(static_cast<uint32>(guid));
    if (!pTarget)
    {
        return;
    }

    pTarget->GetAchievementMgr().SendAllAchievementData(GetPlayer());
}
