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
 *
 */

#include "StdAfx.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Map/WorldCreatorDefines.hpp"

#if VERSION_STRING == TBC
//MIT
void WorldSession::HandleCreatureQueryOpcode(WorldPacket& recv_data)
{
    CHECK_INWORLD_RETURN

        CHECK_PACKET_SIZE(recv_data, 12);

    uint32 entry;
    uint64 guid;

    recv_data >> entry;
    recv_data >> guid;

    WorldPacket data(SMSG_CREATURE_QUERY_RESPONSE, 150);

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
            return;

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
    }

    SendPacket(&data);
}
#endif
//MIT end

#if VERSION_STRING == WotLK

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

    pCorpse = objmgr.GetCorpseByOwner(GetPlayer()->getGuidLow());
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
    uint32 count = 0;
    data << count;

    // 32 count
    // <foreach count>
    //    64 guid
    //    8 status

    for (const auto& itr : _player->getInRangeObjectsSet())
    {
        if (!itr || !itr->isCreature())
            continue;

        Creature* pCreature = static_cast<Creature*>(itr);
        if (pCreature->isQuestGiver())
        {
            data << pCreature->getGuid();
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
