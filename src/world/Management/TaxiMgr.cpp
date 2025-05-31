/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/WDB/WDBStores.hpp"
#include "Management/TaxiMgr.hpp"
#include "Server/Opcodes.hpp"
#include "Objects/Units/Players/Player.hpp"

#include <charconv>
#include <string_view>
#include <cstdint>

#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"

TaxiMask sTaxiNodesMask;
TaxiMask sOldContinentsNodesMask;
TaxiMask sHordeTaxiNodesMask;
TaxiMask sAllianceTaxiNodesMask;
TaxiMask sDeathKnightTaxiNodesMask;

static uint8_t getFieldForTaxiNode(uint32_t nodeId)
{
#if VERSION_STRING < Cata
    return static_cast<uint8_t>((nodeId - 1) / 32);
#else
    return static_cast<uint8_t>((nodeId - 1) / 8);
#endif
}

static uint8_t getSubmaskForTaxiNode(uint32_t nodeId)
{
#if VERSION_STRING < Cata
    return 1 << ((nodeId - 1) % 32);
#else
    return 1 << ((nodeId - 1) % 8);
#endif
}

TaxiPath::TaxiPath()
{ 
    m_taximask.fill(0);
}

void TaxiPath::initTaxiNodesForLevel(uint32_t race, uint32_t chrClass, uint8_t level)
{
#if VERSION_STRING >= WotLK
    // class specific initial known nodes
    switch (chrClass)
    {
        case DEATHKNIGHT:
        {
            for (uint8_t i = 0; i < DBC_TAXI_MASK_SIZE; ++i)
                m_taximask[i] |= sOldContinentsNodesMask[i];
            break;
        }
    }
#endif
    
    const auto team = getSideByRace(race);

#if VERSION_STRING < Cata
    // Add race specific initial nodes
    switch (race)
    {
        case RACE_HUMAN:
            // Stormwind
            setTaximaskNode(2);
            break;
        case RACE_ORC:
            // Orgrimmar
            setTaximaskNode(23);
            break;
        case RACE_DWARF:
            // Ironforge
            setTaximaskNode(6);
            break;
        case RACE_NIGHTELF:
            // Auberdine
            setTaximaskNode(26);
            // Rut'theran Village
            setTaximaskNode(27);
            break;
        case RACE_UNDEAD:
            // Undercity
            setTaximaskNode(11);
            break;
        case RACE_TAUREN:
            // Thunder Bluff
            setTaximaskNode(22);
            break;
        case RACE_GNOME:
            // Ironforge
            setTaximaskNode(6);
            break;
        case RACE_TROLL:
            // Orgrimmar
            setTaximaskNode(23);
            break;
#if VERSION_STRING >= TBC
        case RACE_BLOODELF:
            // Silvermoon City
            setTaximaskNode(82);
            break;
        case RACE_DRAENEI:
            // Exodar
            setTaximaskNode(94);
            break;
#endif
#if VERSION_STRING >= Cata
        case RACE_GOBLIN:
            setTaximaskNode(23);
            break;
        case RACE_WORGEN:
            setTaximaskNode(2);
            break;
#endif
#if VERSION_STRING >= Mop   
        case RACE_PANDAREN_ALLIANCE:
            setTaximaskNode(2);
            break;
        case RACE_PANDAREN_HORDE:
            setTaximaskNode(23);
            break;
#endif
        default:
            break;
    }

#if VERSION_STRING >= TBC
    // Isle of Queldanas
    if (level >= 68)
        setTaximaskNode(213);
#endif
#else
    // Patch 4.2: players will now unlock all taxi nodes within the recommended level range of the player
    for (uint32_t i = 0; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        if (const auto itr = sTaxiNodesStore.lookupEntry(i))
        {
            // Skip scripted and debug nodes
            if (itr->flags == TAXI_NODE_FLAG_SCRIPT)
                continue;

            // Skip nodes that are restricted the player's opposite faction
            if ((!(itr->flags & TAXI_NODE_FLAG_ALLIANCE_RESTRICTED) && team == TEAM_ALLIANCE)
                || (!(itr->flags & TAXI_NODE_FLAG_HORDE_RESTRICTED) && team == TEAM_HORDE))
                continue;

            if (sTaxiMgr.isTaxiNodeUnlockedFor(itr->id, level))
                setTaximaskNode(itr->id);
        }
    }
#endif

    // New continent starting masks (It will be accessible only at new map)
    switch (team)
    {
        case TEAM_ALLIANCE:
#if VERSION_STRING >= TBC
            // Honor Hold
            setTaximaskNode(100);
#if VERSION_STRING >= Cata
            // Valiance Keep
            setTaximaskNode(245);
#endif
#endif
            break;
        case TEAM_HORDE:
#if VERSION_STRING >= TBC
            // Thrallmar
            setTaximaskNode(99);
#if VERSION_STRING >= Cata
            // Warsong Hold
            setTaximaskNode(257);
#endif
#endif
            break;
        default:
            break;
    }
}

void TaxiPath::loadTaxiMask(std::string const& data)
{
    const auto tokens = AscEmu::Util::Strings::split(data, " ");
    auto iter = tokens.cbegin();
    uint8_t index = 0;
    for (; index < DBC_TAXI_MASK_SIZE && iter != tokens.cend(); ++iter, ++index)
    {
        if (const uint32_t mask = std::stoul((*iter).c_str()))
        {
            // load and set bits only for existing taxi nodes
            m_taximask[index] = sTaxiNodesMask[index] & mask;
        }
        else
        {
            m_taximask[index] = 0;
        }
    }
}

bool TaxiPath::isTaximaskNodeKnown(uint32_t nodeidx) const
{
    const auto field = getFieldForTaxiNode(nodeidx);
    const auto submask = getSubmaskForTaxiNode(nodeidx);
    return (m_taximask[field] & submask) == submask;
}

bool TaxiPath::setTaximaskNode(uint32_t nodeidx)
{
    const auto field = getFieldForTaxiNode(nodeidx);
    const auto submask = getSubmaskForTaxiNode(nodeidx);
    if ((m_taximask[field] & submask) != submask)
    {
        m_taximask[field] |= submask;
        return true;
    }
    return false;
}

std::string TaxiPath::saveTaximaskNodeToString() const
{
    std::ostringstream ss;
    for (uint8_t i = 0; i < DBC_TAXI_MASK_SIZE; ++i)
        ss << uint32_t(m_taximask[i]) << " ";
    return ss.str();
}

bool TaxiPath::loadTaxiDestinationsFromString(std::string const& values, uint32_t team)
{
    clearTaxiDestinations();

    const auto tokens = AscEmu::Util::Strings::split(values, " ");
    for (const auto& itr : tokens)
    {
        if (const uint32_t node = std::stoul((itr).c_str()))
            addTaxiDestination(node);
        else
            return false;
    }

    if (m_TaxiDestinations.empty())
        return true;

    // Check integrity
    if (m_TaxiDestinations.size() < 2)
        return false;

    for (size_t i = 1; i < m_TaxiDestinations.size(); ++i)
    {
        uint32_t cost;
        uint32_t path;
        sTaxiMgr.getTaxiPath(m_TaxiDestinations[i - 1], m_TaxiDestinations[i], path, cost);
        if (!path)
            return false;
    }

    // can't load taxi path without mount set (quest taxi path?)
    if (!sTaxiMgr.getTaxiMountDisplayId(getTaxiSource(), team, true))
        return false;

    return true;
}

std::string TaxiPath::saveTaxiDestinationsToString() const
{
    if (m_TaxiDestinations.empty())
        return "";

    std::ostringstream ss;

    for (size_t i = 0; i < m_TaxiDestinations.size(); ++i)
    {
        ss << m_TaxiDestinations[i] << ' ';
    }

    return ss.str();
}

uint32_t TaxiPath::getCurrentTaxiPath() const
{
    if (m_TaxiDestinations.size() < 2)
        return 0;

    uint32_t path;
    uint32_t cost;

    sTaxiMgr.getTaxiPath(m_TaxiDestinations[0], m_TaxiDestinations[1], path, cost);

    return path;
}

uint32_t TaxiPath::nextTaxiDestination()
{
    m_TaxiDestinations.pop_front();
    return getTaxiDestination();
}

TaxiMask const& TaxiPath::getTaxiMask(bool all) const
{
    return all ? sTaxiNodesMask : m_taximask;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Taxi Mgr
uint32_t TaxiMgr::getNearestTaxiNode(LocationVector const& pos, uint32_t mapid, uint32_t team) const
{
    bool found = false;
    float dist = 10000;
    uint32_t id = 0;

    for (uint32_t i = 1; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        const auto node = sTaxiNodesStore.lookupEntry(i);
        if (node == nullptr || node->mapid != mapid || (!node->mountCreatureID[team == TEAM_ALLIANCE ? 1 : 0] && node->mountCreatureID[0] != 32981)) // dk flight
            continue;

        const auto field = getFieldForTaxiNode(i);
        const auto submask = getSubmaskForTaxiNode(i);

        // skip not taxi network nodes
        if ((sTaxiNodesMask[field] & submask) == 0)
            continue;

        float dist2 = (node->x - pos.x) * (node->x - pos.x) + (node->y - pos.y) * (node->y - pos.y) + (node->z - pos.z) * (node->z - pos.z);
        if (found)
        {
            if (dist2 < dist)
            {
                dist = dist2;
                id = i;
            }
        }
        else
        {
            found = true;
            dist = dist2;
            id = i;
        }
    }

    return id;
}

void TaxiMgr::getTaxiPath(uint32_t source, uint32_t destination, uint32_t& path, uint32_t& cost) const
{
    const auto src_i = sTaxiPathSetBySource.find(source);
    if (src_i == sTaxiPathSetBySource.cend())
    {
        path = 0;
        cost = 0;
        return;
    }

    const auto& pathSet = src_i->second;

    const auto dest_i = pathSet.find(destination);
    if (dest_i == pathSet.cend())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint32_t TaxiMgr::getTaxiMountDisplayId(uint32_t id, uint32_t team, bool allowed_alt_team /* = false */) const
{
    uint32_t mount_id = 0;

    // select mount creature id
    if (const auto node = sTaxiNodesStore.lookupEntry(id))
    {
        uint32_t mount_entry = 0;
        if (team == TEAM_ALLIANCE)
            mount_entry = node->mountCreatureID[1];
        else
            mount_entry = node->mountCreatureID[0];

        // Fix for Alliance not being able to use Acherus taxi
        // only one mount type for both sides
        if (mount_entry == 0 && allowed_alt_team)
        {
            // Simply reverse the selection. At least one team in theory should have a valid mount ID to choose.
            mount_entry = team == TEAM_ALLIANCE ? node->mountCreatureID[0] : node->mountCreatureID[1];
        }

        if (const auto mount_info = sMySQLStore.getCreatureProperties(mount_entry))
        {
            mount_id = mount_info->getRandomModelId();
            if (mount_id == 0)
            {
                sLogger.failure("TaxiMgr:::No displayid found for the taxi mount with the entry {}! Can't load it!", mount_entry);
                return 0;
            }
        }
    }

    return mount_id;
}

void TaxiMgr::loadTaxiNodeLevelData()
{
    auto oldMSTime = Util::TimeNow();

    //                                               0            1
    auto result = WorldDatabase.Query("SELECT TaxiNodeId, `Level` FROM taxi_level_data ORDER BY TaxiNodeId ASC");

    if (!result)
    {
        sLogger.info("TaxiMgr:: Loaded 0 taxi node level definitions. DB table `taxi_level_data` is empty.");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t taxiNodeId = fields[0].asUint16();
        uint8_t level = fields[1].asUint8();

        const auto node = sTaxiNodesStore.lookupEntry(taxiNodeId);
        if (node == nullptr)
        {
            sLogger.failure("TaxiMgr:: Table `taxi_level_data` has data for nonexistent taxi node (ID: {}), skipped", taxiNodeId);
            continue;
        };

        _taxiNodeLevelDataStore.emplace(taxiNodeId, level);

        ++count;
    } while (result->NextRow());

    sLogger.info("TaxiMgr:: Loaded {} taxi node level definitions in {} ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

bool TaxiMgr::isTaxiNodeUnlockedFor(uint32_t taxiNodeId, uint8_t level) const
{
    const auto itr = _taxiNodeLevelDataStore.find(taxiNodeId);
    if (itr != _taxiNodeLevelDataStore.cend())
        return itr->second <= level;

    return false;
}

void TaxiMgr::initialize()
{
#if VERSION_STRING > WotLK
    loadTaxiNodeLevelData();
#endif

    // Initialize global taxinodes mask
    // include existed nodes that have at least single not spell base (scripted) path
    std::set<uint32_t> spellPaths;

#if VERSION_STRING > WotLK
    for (uint32_t i = 0; i < sSpellEffectStore.getNumRows(); ++i)
    {
        if (const auto sInfo = sSpellEffectStore.lookupEntry(i))
        {
            if (sInfo->Effect == SPELL_EFFECT_START_TAXI)
                spellPaths.insert(sInfo->EffectMiscValue);
        }
    }
#else
    for (uint32_t i = 0; i < sSpellStore.getNumRows(); ++i)
    {
        if (const auto sInfo = sSpellStore.lookupEntry(i))
        {
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (sInfo->Effect[j] == SPELL_EFFECT_START_TAXI)
                    spellPaths.insert(sInfo->EffectMiscValue[j]);
            }
        }
    }
#endif

    sTaxiNodesMask.fill(0);
    sOldContinentsNodesMask.fill(0);
    sHordeTaxiNodesMask.fill(0);
    sAllianceTaxiNodesMask.fill(0);
    sDeathKnightTaxiNodesMask.fill(0);
    for (uint32_t i = 1; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        const auto node = sTaxiNodesStore.lookupEntry(i);
        if (node == nullptr)
            continue;

        const auto src_i = std::as_const(sTaxiPathSetBySource).find(i);
        if (src_i != sTaxiPathSetBySource.cend() && !src_i->second.empty())
        {
            bool ok = false;
            for (auto dest_i = src_i->second.begin(); dest_i != src_i->second.end(); ++dest_i)
            {
                // not spell path
                if (spellPaths.find(dest_i->second.ID) == spellPaths.end())
                {
                    ok = true;
                    break;
                }
            }

            if (!ok)
                continue;
        }

        // valid taxi network node
        const auto field = getFieldForTaxiNode(i);
        const auto submask = getSubmaskForTaxiNode(i);

        sTaxiNodesMask[field] |= submask;
        if (node->mountCreatureID[0] && node->mountCreatureID[0] != 32981)
            sHordeTaxiNodesMask[field] |= submask;
        if (node->mountCreatureID[1] && node->mountCreatureID[1] != 32981)
            sAllianceTaxiNodesMask[field] |= submask;
        if (node->mountCreatureID[0] == 32981 || node->mountCreatureID[1] == 32981)
            sDeathKnightTaxiNodesMask[field] |= submask;

        // old continent node (+ nodes virtually at old continents, check explicitly to avoid loading map files for zone info)
        if (node->mapid < 2 || i == 82 || i == 83 || i == 93 || i == 94)
            sOldContinentsNodesMask[field] |= submask;

        // fix DK node at Ebon Hold and Shadow Vault flight master
        if (i == 315 || i == 333)
            ((WDB::Structures::TaxiNodesEntry*)node)->mountCreatureID[1] = 32981;
    }
}