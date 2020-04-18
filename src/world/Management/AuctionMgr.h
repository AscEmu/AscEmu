/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
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

class AuctionMgr
{
    private:

        AuctionMgr() = default;
        ~AuctionMgr() = default;

    public:

        static AuctionMgr& getInstance();

        void initialize()
        {
            loopcount = 0;
            maxId = 1;
        }

        void finalize()
        {
            std::vector<AuctionHouse*>::iterator itr = auctionHouses.begin();
            for (; itr != auctionHouses.end(); ++itr)
                delete(*itr);
        }

        AuctionMgr(AuctionMgr&&) = delete;
        AuctionMgr(AuctionMgr const&) = delete;
        AuctionMgr& operator=(AuctionMgr&&) = delete;
        AuctionMgr& operator=(AuctionMgr const&) = delete;

        void LoadAuctionHouses();
        void Update();

        AuctionHouse* GetAuctionHouse(uint32_t Entry);

        uint32_t GenerateAuctionId()
        {
            uint32_t id = ++maxId;

            return id;
        }

    private:

        std::unordered_map<uint32_t, AuctionHouse*> auctionHouseEntryMap;
        std::vector<AuctionHouse*> auctionHouses;

        std::atomic<unsigned long> maxId;

        uint32_t loopcount;
};

#define sAuctionMgr AuctionMgr::getInstance()

#endif // AUCTIONMGR_H
