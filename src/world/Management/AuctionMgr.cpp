/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#include "Management/AuctionHouse.h"
#include "Management/AuctionMgr.h"
#include "Server/MainServerDefines.h"
#include "Log.hpp"

initialiseSingleton(AuctionMgr);

void AuctionMgr::LoadAuctionHouses()
{
    LogNotice("AuctionMgr : Loading Auction Houses...");

    QueryResult* res = CharacterDatabase.Query("SELECT MAX(auctionId) FROM auctions");
    if (res)
    {
        maxId = res->Fetch()[0].GetUInt32();
        delete res;
    }

    res = WorldDatabase.Query("SELECT DISTINCT ahgroup FROM auctionhouse");
    AuctionHouse* ah;
    std::map<uint32, AuctionHouse*> tempmap;
    if (res)
    {
        uint32 period = (res->GetRowCount() / 20) + 1;
        uint32 c = 0;
        do
        {
            ah = new AuctionHouse(res->Fetch()[0].GetUInt32());
            ah->LoadAuctions();
            auctionHouses.push_back(ah);
            tempmap.insert(std::make_pair(res->Fetch()[0].GetUInt32(), ah));
            if (!((++c) % period))
                LogNotice("AuctionHouse : Done %u/%u, %u%% complete.", c, res->GetRowCount(), c * 100 / res->GetRowCount());

        }
        while (res->NextRow());
        delete res;
    }

    res = WorldDatabase.Query("SELECT creature_entry, ahgroup FROM auctionhouse");
    if (res)
    {
        do
        {
            auctionHouseEntryMap.insert(std::make_pair(res->Fetch()[0].GetUInt32(), tempmap[res->Fetch()[1].GetUInt32()]));
        }
        while (res->NextRow());
        delete res;
    }
}

AuctionHouse* AuctionMgr::GetAuctionHouse(uint32 Entry)
{
    std::unordered_map<uint32, AuctionHouse*>::iterator itr = auctionHouseEntryMap.find(Entry);
    if (itr == auctionHouseEntryMap.end()) return NULL;
    return itr->second;
}

void AuctionMgr::Update()
{
    if ((++loopcount % 100))
        return;

    std::vector<AuctionHouse*>::iterator itr = auctionHouses.begin();
    for (; itr != auctionHouses.end(); ++itr)
    {
        (*itr)->UpdateDeletionQueue();

        // Actual auction loop is on a separate timer.
        if (!(loopcount % 1200))
            (*itr)->UpdateAuctions();
    }
}
