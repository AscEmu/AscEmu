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


#if VERSION_STRING == WotLK

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
