/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstddef>
#include <map>
#include <unordered_map>
#include "CommonTypes.hpp"
#include "Utilities/Util.hpp"
#include <ByteBuffer.h>
#include <deque>

struct TaxiPathBySourceAndDestination;

#define TaxiMaskSize 114
typedef uint8_t TaxiMask[TaxiMaskSize];
typedef std::unordered_map<uint32_t, uint8_t> TaxiNodeLevelDataContainer;
typedef std::map<uint32_t, TaxiPathBySourceAndDestination> TaxiPathSetForSource;
typedef std::map<uint32_t, TaxiPathSetForSource> TaxiPathSetBySource;

extern SERVER_DECL TaxiMask            sTaxiNodesMask;
extern SERVER_DECL TaxiMask            sOldContinentsNodesMask;
extern SERVER_DECL TaxiMask            sHordeTaxiNodesMask;
extern SERVER_DECL TaxiMask            sAllianceTaxiNodesMask;
extern SERVER_DECL TaxiMask            sDeathKnightTaxiNodesMask;

enum TaxiNodeFlags
{
    TAXI_NODE_FLAG_SCRIPT               = 0x0,
    TAXI_NODE_FLAG_ALLIANCE_RESTRICTED  = 0x1,
    TAXI_NODE_FLAG_HORDE_RESTRICTED     = 0x2,
    TAXI_NODE_FLAG_UNK                  = 0x4
};

struct TaxiPathBySourceAndDestination
{
    TaxiPathBySourceAndDestination() : ID(0), price(0) { }
    TaxiPathBySourceAndDestination(uint32_t _id, uint32_t _price) : ID(_id), price(_price) { }

    uint32_t ID;
    uint32_t price;
};

namespace TaxiNodeError
{
    enum
    {
        ERR_Ok                          = 0,
        ERR_UnspecificError             = 1,
        ERR_NoDirectPath                = 2,
        ERR_NotEnoughMoney              = 3,
        ERR_TaxiTooFarAway              = 4,
        ERR_TaxiNoVendorNearby          = 5,
        ERR_TaxiNotVisited              = 6,
        ERR_TaxiPlayerBusy              = 7,
        ERR_TaxiPlayerAlreadyMounted    = 8,
        ERR_TaxiPlayerShapeshifted      = 9,
        ERR_TaxiPlayerMoving            = 10,
        ERR_TaxiSameNode                = 11,
        ERR_TaxiNotStanding             = 12
    };
};

class Player;

struct TaxiNode
{
    uint32_t id;
    float x, y, z;
    uint32_t mapid;
    uint32_t hordeMount;
    uint32_t allianceMount;
};

struct TaxiPathNode
{
    float x, y, z;
    uint32_t mapid;
};

class SERVER_DECL TaxiPath
{
public:
    TaxiPath();
    ~TaxiPath() { }

    // Nodes
#if VERSION_STRING > WotLK
    void initTaxiNodesForLevel(uint32_t race, uint32_t chrClass, uint8_t level);
#endif
    void loadTaxiMask(std::string const& data);

    bool isTaximaskNodeKnown(uint32_t nodeidx) const
    {
        uint8_t  field = uint8_t((nodeidx - 1) / 8);
        uint32_t submask = 1 << ((nodeidx - 1) % 8);
        return (m_taximask[field] & submask) == submask;
    }
    bool setTaximaskNode(uint32_t nodeidx)
    {
        uint8_t  field = uint8_t((nodeidx - 1) / 8);
        uint32_t submask = 1 << ((nodeidx - 1) % 8);
        if ((m_taximask[field] & submask) != submask)
        {
            m_taximask[field] |= submask;
            return true;
        }
        else
            return false;
    }
    void appendTaximaskTo(ByteBuffer& data, bool all);

    // Destinations
    bool loadTaxiDestinationsFromString(std::string const& values, uint32_t team);
    std::string saveTaxiDestinationsToString();

    void clearTaxiDestinations() { m_TaxiDestinations.clear(); }
    void addTaxiDestination(uint32_t dest) { m_TaxiDestinations.push_back(dest); }

    uint32_t getTaxiSource() const { return m_TaxiDestinations.empty() ? 0 : m_TaxiDestinations.front(); }
    uint32_t getTaxiDestination() const { return m_TaxiDestinations.size() < 2 ? 0 : m_TaxiDestinations[1]; }
    uint32_t getCurrentTaxiPath() const;
    void setNodeAfterTeleport(uint32_t nodeId) { nodeAfterTeleport = nodeId; }

    uint32_t nextTaxiDestination()
    {
        m_TaxiDestinations.pop_front();
        return getTaxiDestination();
    }

    std::deque<uint32_t> const& getPath() const { return m_TaxiDestinations; }
    bool empty() const { return m_TaxiDestinations.empty(); }

    uint32_t nodeAfterTeleport = 0;
    friend std::ostringstream& operator<<(std::ostringstream& ss, TaxiPath const& taxi);

private:
    TaxiMask m_taximask;
    std::deque<uint32_t> m_TaxiDestinations;
};

class SERVER_DECL TaxiMgr
{
private:
    TaxiMgr() = default;
    ~TaxiMgr() = default;

public:
    static TaxiMgr& getInstance()
    {
        static TaxiMgr mInstance;
        return mInstance;
    }

    void initialize();
    void loadTaxiNodeLevelData();

    uint32_t getNearestTaxiNode(float x, float y, float z, uint32_t mapid, uint32_t team);
    void getTaxiPath(uint32_t source, uint32_t destination, uint32_t& path, uint32_t& cost);
    uint32_t getTaxiMountDisplayId(uint32_t id, uint32_t team, bool allowed_alt_team = false);
    bool isTaxiNodeUnlockedFor(uint32_t taxiNodeId, uint8_t level) const;

    TaxiMgr(TaxiMgr&&) = delete;
    TaxiMgr(TaxiMgr const&) = delete;
    TaxiMgr& operator=(TaxiMgr&&) = delete;
    TaxiMgr& operator=(TaxiMgr const&) = delete;

private:
    TaxiNodeLevelDataContainer _taxiNodeLevelDataStore;
};

#define sTaxiMgr TaxiMgr::getInstance()