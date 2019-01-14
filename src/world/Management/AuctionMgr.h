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
 *
 */

#ifndef AUCTIONMGR_H
#define AUCTIONMGR_H

#include "AuctionHouse.h"

class AuctionMgr : public Singleton <AuctionMgr>
{
    public:

        AuctionMgr()
        {
            loopcount = 0;
            maxId = 1;
        }

        ~AuctionMgr()
        {
            std::vector<AuctionHouse*>::iterator itr = auctionHouses.begin();
            for (; itr != auctionHouses.end(); ++itr)
                delete(*itr);
        }

        void LoadAuctionHouses();
        void Update();

        AuctionHouse* GetAuctionHouse(uint32 Entry);

        uint32 GenerateAuctionId()
        {
            uint32 id = ++maxId;

            return id;
        }

    private:

        std::unordered_map<uint32, AuctionHouse*> auctionHouseEntryMap;
        std::vector<AuctionHouse*> auctionHouses;

        std::atomic<unsigned long> maxId;

        uint32 loopcount;
};

#define sAuctionMgr AuctionMgr::getSingleton()

#endif // AUCTIONMGR_H
