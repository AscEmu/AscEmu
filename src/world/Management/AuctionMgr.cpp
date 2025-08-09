/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
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
    m_auctionHouseEntryMap.clear();
    m_auctionHouses.clear();
}

void AuctionMgr::loadAuctionHouses()
{
    sLogger.info("AuctionMgr : Loading Auction Houses...");

    auto res = CharacterDatabase.Query("SELECT MAX(auctionId) FROM auctions");
    if (res)
    {
        m_maxId = res->Fetch()[0].asUint32();
    }

    res = WorldDatabase.Query("SELECT DISTINCT ahgroup FROM auctionhouse");
    std::map<uint32_t, AuctionHouse*> tempmap;
    if (res)
    {
        const uint32_t period = (res->GetRowCount() / 20) + 1;
        uint32_t c = 0;
        do
        {
            const auto& ah = m_auctionHouses.emplace_back(std::make_unique<AuctionHouse>(res->Fetch()[0].asUint32()));
            ah->loadAuctionsFromDB();
            tempmap.try_emplace(res->Fetch()[0].asUint32(), ah.get());
            if (!((++c) % period))
                sLogger.info("AuctionHouse : Done {}/{}, {}% complete.", c, res->GetRowCount(), c * 100 / res->GetRowCount());

        }
        while (res->NextRow());
    }

    res = WorldDatabase.Query("SELECT creature_entry, ahgroup FROM auctionhouse");
    if (res)
    {
        do
        {
            m_auctionHouseEntryMap.try_emplace(res->Fetch()[0].asUint32(), tempmap[res->Fetch()[1].asUint32()]);
        }
        while (res->NextRow());
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
