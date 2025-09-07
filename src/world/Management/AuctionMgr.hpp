/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class AuctionHouse;

class AuctionMgr
{
    AuctionMgr() = default;
    ~AuctionMgr() = default;

public:
    static AuctionMgr& getInstance();
    void initialize();
    void finalize();

    AuctionMgr(AuctionMgr&&) = delete;
    AuctionMgr(AuctionMgr const&) = delete;
    AuctionMgr& operator=(AuctionMgr&&) = delete;
    AuctionMgr& operator=(AuctionMgr const&) = delete;

    void loadAuctionHouses();
    void update();

    AuctionHouse* getAuctionHouse(uint32_t _entry);

    uint32_t generateAuctionId();

private:
    std::unordered_map<uint32_t, AuctionHouse*> m_auctionHouseEntryMap;
    std::vector<std::unique_ptr<AuctionHouse>> m_auctionHouses;

    std::atomic<unsigned long> m_maxId = 1;

    uint32_t m_loopcount = 0;
};

#define sAuctionMgr AuctionMgr::getInstance()
