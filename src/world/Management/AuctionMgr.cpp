/*
Copyright (c) 2014-2024 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AuctionMgr.hpp"
#include "Management/AuctionHouse.h"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"

AuctionMgr& AuctionMgr::getInstance()
{
    static AuctionMgr mInstance;
    return mInstance;
}

void AuctionMgr::initialize()
{
}

void AuctionMgr::finalize()
{
    auto itr = m_auctionHouses.begin();
    for (; itr != m_auctionHouses.end(); ++itr)
        delete(*itr);
}

void AuctionMgr::loadAuctionHouses()
{
    sLogger.info("AuctionMgr : Loading Auction Houses...");

    QueryResult* res = CharacterDatabase.Query("SELECT MAX(auctionId) FROM auctions");
    if (res)
    {
        m_maxId = res->Fetch()[0].GetUInt32();
        delete res;
    }

    res = WorldDatabase.Query("SELECT DISTINCT ahgroup FROM auctionhouse");
    std::map<uint32_t, AuctionHouse*> tempmap;
    if (res)
    {
        const uint32_t period = (res->GetRowCount() / 20) + 1;
        uint32_t c = 0;
        do
        {
            auto ah = new AuctionHouse(res->Fetch()[0].GetUInt32());
            ah->loadAuctionsFromDB();
            m_auctionHouses.push_back(ah);
            tempmap.insert(std::make_pair(res->Fetch()[0].GetUInt32(), ah));
            if (!((++c) % period))
                sLogger.info("AuctionHouse : Done {}/{}, {}% complete.", c, res->GetRowCount(), c * 100 / res->GetRowCount());

        }
        while (res->NextRow());
        delete res;
    }

    res = WorldDatabase.Query("SELECT creature_entry, ahgroup FROM auctionhouse");
    if (res)
    {
        do
        {
            m_auctionHouseEntryMap.insert(std::make_pair(res->Fetch()[0].GetUInt32(), tempmap[res->Fetch()[1].GetUInt32()]));
        }
        while (res->NextRow());
        delete res;
    }
}

AuctionHouse* AuctionMgr::getAuctionHouse(uint32_t _entry)
{
    const auto itr = m_auctionHouseEntryMap.find(_entry);
    if (itr == m_auctionHouseEntryMap.end())
        return nullptr;

    return itr->second;
}

uint32_t AuctionMgr::generateAuctionId()
{
    uint32_t id = ++m_maxId;

    return id;
}

void AuctionMgr::update()
{
    if (++m_loopcount % 100)
        return;

    for (auto itr = m_auctionHouses.begin(); itr != m_auctionHouses.end(); ++itr)
    {
        (*itr)->updateDeletionQueue();

        // Actual auction loop is on a separate timer.
        if (!(m_loopcount % 1200))
            (*itr)->updateAuctions();
    }
}
