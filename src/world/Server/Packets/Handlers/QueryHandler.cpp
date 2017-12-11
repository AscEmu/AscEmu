/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
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
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"

#if VERSION_STRING != Cata
//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_NAME_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleNameQueryOpcode(WorldPacket& recv_data)
{
    CHECK_PACKET_SIZE(recv_data, 8);
    uint64 guid;
    recv_data >> guid;

    PlayerInfo* pn = objmgr.GetPlayerInfo((uint32)guid);

    if (!pn)
        return;

    LOG_DEBUG("Received CMSG_NAME_QUERY for: %s", pn->name);

    WoWGuid pguid((uint64)pn->guid); //VLack: The usual new style guid handling on 3.1.2
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

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_QUERY_TIME:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleQueryTimeOpcode(WorldPacket& /*recvData*/)
{

    WorldPacket data(SMSG_QUERY_TIME_RESPONSE, 4 + 4);
    data << (uint32)UNIXTIME;
    data << (uint32)0; //VLack: 3.1; thanks Stewart for reminding me to have the correct structure even if it seems the old one still works.
    SendPacket(&data);

}

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_CREATURE_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING != Cata
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 12);

    uint32 entry;
    uint64 guid;

    recv_data >> entry;
    recv_data >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 250); //VLack: thanks Aspire, this was 146 before

    if (entry == 300000)
    {
        data << entry;
        data << "WayPoint";
        data << uint8(0);
        data << uint8(0);
        data << uint8(0);
        data << "Level is WayPoint ID";
        for (uint8 i = 0; i < 8; i++)
        {
            data << uint32(0);
        }

        data << uint8(0);
    }
    else
    {
        CreatureProperties const* ci = sMySQLStore.getCreatureProperties(entry);
        if (ci == nullptr)
        {
            return;
        }

        MySQLStructure::LocalesCreature const* lcn = (language > 0) ? sMySQLStore.getLocalizedCreature(entry, language) : nullptr;
        if (lcn == nullptr)
        {
            data << entry;
            data << ci->Name;       // name of the creature
            data << uint8(0);       // name2, always seems to be empty
            data << uint8(0);       // name3, always seems to be empty
            data << uint8(0);       // name4, always seems to be empty
            data << ci->SubName;    // this is the title/guild of the creature
        }
        else
        {
            data << entry;
            data << lcn->name;
            data << uint8(0);
            data << uint8(0);
            data << uint8(0);
            data << lcn->subName;
        }

        data << ci->info_str;       // this is a string in 2.3.0 Example: stormwind guard has : "Direction"
        data << ci->typeFlags;      // flags like skinnable
        data << ci->Type;           // humanoid, beast, etc
        data << ci->Family;         // petfamily
        data << ci->Rank;           // normal, elite, etc
        data << ci->killcredit[0];  // quest kill credit 1
        data << ci->killcredit[1];  // quest kill credit 2
        data << ci->Male_DisplayID;
        data << ci->Female_DisplayID;
        data << ci->Male_DisplayID2;
        data << ci->Female_DisplayID2;
        data << ci->baseAttackMod;
        data << ci->rangeAttackMod;
        data << ci->Leader;         // faction leader

        // these are the 6 seperate quest items a creature can drop
        for (uint8 i = 0; i < 6; ++i)
        {
            data << uint32(ci->QuestItems[i]);
        }

        data << ci->waypointid;
    }

    SendPacket(&data);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_GAMEOBJECT_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleGameObjectQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 12);
    WorldPacket data(SMSG_GAMEOBJECT_QUERY_RESPONSE, 900);

    uint32 entryID;
    uint64 guid;

    recv_data >> entryID;
    recv_data >> guid;

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

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles MSG_CORPSE_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
#if VERSION_STRING != Cata
void WorldSession::HandleCorpseQueryOpcode(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN

    LOG_DETAIL("WORLD: Received MSG_CORPSE_QUERY");

    Corpse* pCorpse;
    WorldPacket data(MSG_CORPSE_QUERY, 25);
    MySQLStructure::MapInfo const* pMapinfo;

    pCorpse = objmgr.GetCorpseByOwner(GetPlayer()->GetLowGUID());
    if (pCorpse)
    {
        pMapinfo = sMySQLStore.getWorldMapInfo(pCorpse->GetMapId());
        if (pMapinfo)
        {
            if (pMapinfo->type == INSTANCE_NULL || pMapinfo->type == INSTANCE_BATTLEGROUND)
            {
                data << uint8(0x01);            //show ?
                data << pCorpse->GetMapId();    // mapid (that tombstones shown on)
                data << pCorpse->GetPositionX();
                data << pCorpse->GetPositionY();
                data << pCorpse->GetPositionZ();
                data << pCorpse->GetMapId();    //instance mapid (needs to be same as mapid to be able to recover corpse)
                data << uint32(0);
                SendPacket(&data);
            }
            else
            {
                data << uint8(0x01);            //show ?
                data << pMapinfo->repopmapid;   // mapid (that tombstones shown on)
                data << pMapinfo->repopx;
                data << pMapinfo->repopy;
                data << pMapinfo->repopz;
                data << pCorpse->GetMapId();    //instance mapid (needs to be same as mapid to be able to recover corpse)
                data << uint32(0);
                SendPacket(&data);
            }
        }
        else
        {
            data << uint8(0x01);                //show ?
            data << pCorpse->GetMapId();        // mapid (that tombstones shown on)
            data << pCorpse->GetPositionX();
            data << pCorpse->GetPositionY();
            data << pCorpse->GetPositionZ();
            data << pCorpse->GetMapId();        //instance mapid (needs to be same as mapid to be able to recover corpse)
            data << uint32(0);
            SendPacket(&data);
        }
    }
}
#endif

void WorldSession::HandlePageTextQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

    CHECK_PACKET_SIZE(recv_data, 4);
    uint32 pageid = 0;
    recv_data >> pageid;

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

//////////////////////////////////////////////////////////////////////////////////////////
/// This function handles CMSG_ITEM_NAME_QUERY:
//////////////////////////////////////////////////////////////////////////////////////////
void WorldSession::HandleItemNameQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN
    CHECK_PACKET_SIZE(recv_data, 4);

    uint32 itemid;

    recv_data >> itemid;
    recv_data.read_skip<uint64>();

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

#if VERSION_STRING != Cata
void WorldSession::HandleInrangeQuestgiverQuery(WorldPacket& /*recv_data*/)
{
    CHECK_INWORLD_RETURN;

    WorldPacket data(SMSG_QUESTGIVER_STATUS_MULTIPLE, 1000);
    Object::InRangeSet::iterator itr;
    Creature* pCreature;
    uint32 count = 0;
    data << count;

    // 32 count
    // <foreach count>
    //    64 guid
    //    8 status

    for (itr = _player->m_objectsInRange.begin(); itr != _player->m_objectsInRange.end(); ++itr)
    {
        if (!(*itr)->IsCreature())
            continue;

        pCreature = static_cast<Creature*>(*itr);

        if (pCreature->isQuestGiver())
        {
            data << pCreature->GetGUID();
            data << uint8(sQuestMgr.CalcStatus(pCreature, _player));
            ++count;
        }
    }

    *(uint32*)(data.contents()) = count;
    SendPacket(&data);
}
#endif

void WorldSession::HandleAchievmentQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN;

    uint64 guid = recv_data.unpackGUID();               // Get the inspectee's GUID
    Player* pTarget = objmgr.GetPlayer((uint32)guid);
    if (!pTarget)
    {
        return;
    }
#if VERSION_STRING > TBC
    pTarget->GetAchievementMgr().SendAllAchievementData(GetPlayer());
#endif
}
#endif
