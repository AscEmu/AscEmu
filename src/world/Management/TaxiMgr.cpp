/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Storage/WDB/WDBStores.hpp"
#include "Management/TaxiMgr.hpp"
#include "Server/Opcodes.hpp"
#include "Objects/Units/Players/Player.hpp"

#include <charconv>
#include <string_view>
#include <cstdint>

#include "Server/DatabaseDefinition.hpp"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Utilities/Strings.hpp"

TaxiMask sTaxiNodesMask;
TaxiMask sOldContinentsNodesMask;
TaxiMask sHordeTaxiNodesMask;
TaxiMask sAllianceTaxiNodesMask;
TaxiMask sDeathKnightTaxiNodesMask;

uint32_t TeamForRace(uint8_t race)
{
    if (WDB::Structures::ChrRacesEntry const* rEntry = sChrRacesStore.lookupEntry(race))
    {
        switch (rEntry->team_id)
        {
            case 1: return TEAM_HORDE;
            case 7: return TEAM_ALLIANCE;
        }
    }

    return TEAM_ALLIANCE;
}

TaxiPath::TaxiPath()
{ 
    memset(m_taximask, 0, sizeof(m_taximask));
}

#if VERSION_STRING > WotLK
void TaxiPath::initTaxiNodesForLevel(uint32_t race, uint32_t chrClass, uint8_t level)
{
    // class specific initial known nodes
    switch (chrClass)
    {
        case DEATHKNIGHT:
        {
            for (uint8_t i = 0; i < TaxiMaskSize; ++i)
                m_taximask[i] |= sOldContinentsNodesMask[i];
            break;
        }
    }
    
    uint32_t team = TeamForRace(race);

    // Patch 4.2: players will now unlock all taxi nodes within the recommended level range of the player
    for (uint32_t i = 0; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        if (WDB::Structures::TaxiNodesEntry const* itr = sTaxiNodesStore.lookupEntry(i))
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

    // New continent starting masks (It will be accessible only at new map)
    switch (team)
    {
        case TEAM_ALLIANCE:
            setTaximaskNode(100); // Honor Hold
            setTaximaskNode(245); // Valiance Keep
            break;
        case TEAM_HORDE:
            setTaximaskNode(99); // Thrallmar
            setTaximaskNode(257); // Warsong Hold
            break;
        default:
            break;
    }
}
#endif

std::vector<std::string_view> tokenize(std::string_view str, char sep, bool keepEmpty)
{
    std::vector<std::string_view> tokens;

    size_t start = 0;
    for (size_t end = str.find(sep); end != std::string_view::npos; end = str.find(sep, start))
    {
        if (keepEmpty || (start < end))
            tokens.push_back(str.substr(start, end - start));
        start = end + 1;
    }

    if (keepEmpty || (start < str.length()))
        tokens.push_back(str.substr(start));

    return tokens;
}

std::optional<uint32_t> to_uint(const std::string_view& input)
{
    int out;
    const std::from_chars_result result = std::from_chars(input.data(), input.data() + input.size(), out);
    if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range)
    {
        return std::nullopt;
    }
    return out;
}

void TaxiPath::loadTaxiMask(std::string const& data)
{
    std::vector<std::string_view> tokens = tokenize(data, ' ', false);
    for (uint8_t index = 0; (index < TaxiMaskSize) && (index < tokens.size()); ++index)
    {
        if (Optional<uint32_t> mask = to_uint(tokens[index]))
        {
            // load and set bits only for existing taxi nodes
            m_taximask[index] = sTaxiNodesMask[index] & *mask;
        }
        else
        {
            m_taximask[index] = 0;
        }
    }
}

void TaxiPath::appendTaximaskTo(ByteBuffer& data, bool all)
{

#if VERSION_STRING > WotLK
    data << uint32_t(TaxiMaskSize);
#endif

    if (all)
    {
        for (uint8_t i = 0; i < TaxiMaskSize; ++i)
            data << uint8_t(sTaxiNodesMask[i]);              // all existing nodes
    }
    else
    {
        for (uint8_t i = 0; i < TaxiMaskSize; ++i)
            data << uint8_t(m_taximask[i]);                  // known nodes
    }
}

bool TaxiPath::loadTaxiDestinationsFromString(std::string const& values, uint32_t team)
{
    clearTaxiDestinations();

    std::vector<std::string_view> tokens = tokenize(values, ' ', false);
    auto itr = tokens.begin();
    for (auto itr : tokens)
    {
        if (Optional<uint32_t> node = to_uint(itr))
            addTaxiDestination(*node);
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

std::string TaxiPath::saveTaxiDestinationsToString()
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

std::ostringstream& operator<<(std::ostringstream& ss, TaxiPath const& taxi)
{
    for (uint8_t i = 0; i < TaxiMaskSize; ++i)
        ss << uint32_t(taxi.m_taximask[i]) << ' ';
    return ss;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Taxi Mgr
uint32_t TaxiMgr::getNearestTaxiNode(float x, float y, float z, uint32_t mapid, uint32_t team)
{
    bool found = false;
    float dist = 10000;
    uint32_t id = 0;

    for (uint32_t i = 1; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        WDB::Structures::TaxiNodesEntry const* node = sTaxiNodesStore.lookupEntry(i);

        if (!node || node->mapid != mapid || (!node->mountCreatureID[team == TEAM_ALLIANCE ? 1 : 0] && node->mountCreatureID[0] != 32981)) // dk flight
            continue;

        uint8_t  field = (uint8_t)((i - 1) / 8);
        uint32_t submask = 1 << ((i - 1) % 8);

        // skip not taxi network nodes
        if ((sTaxiNodesMask[field] & submask) == 0)
            continue;

        float dist2 = (node->x - x) * (node->x - x) + (node->y - y) * (node->y - y) + (node->z - z) * (node->z - z);
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

void TaxiMgr::getTaxiPath(uint32_t source, uint32_t destination, uint32_t& path, uint32_t& cost)
{
    TaxiPathSetBySource::iterator src_i = sTaxiPathSetBySource.find(source);
    if (src_i == sTaxiPathSetBySource.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    TaxiPathSetForSource& pathSet = src_i->second;

    TaxiPathSetForSource::iterator dest_i = pathSet.find(destination);
    if (dest_i == pathSet.end())
    {
        path = 0;
        cost = 0;
        return;
    }

    cost = dest_i->second.price;
    path = dest_i->second.ID;
}

uint32_t TaxiMgr::getTaxiMountDisplayId(uint32_t id, uint32_t team, bool allowed_alt_team /* = false */)
{
    uint32_t mount_id = 0;

    // select mount creature id
    WDB::Structures::TaxiNodesEntry const* node = sTaxiNodesStore.lookupEntry(id);
    if (node)
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

        CreatureProperties const* mount_info = sMySQLStore.getCreatureProperties(mount_entry);
        if (mount_info)
        {
            mount_id = mount_info->getRandomModelId();
            if (!mount_id)
            {
                sLogger.failure("TaxiMgr:::No displayid found for the taxi mount with the entry %u! Can't load it!", mount_entry);
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
    QueryResult* result = WorldDatabase.Query("SELECT TaxiNodeId, `Level` FROM taxi_level_data ORDER BY TaxiNodeId ASC");

    if (!result)
    {
        sLogger.info("TaxiMgr:: Loaded 0 taxi node level definitions. DB table `taxi_level_data` is empty.");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t taxiNodeId = fields[0].GetUInt16();
        uint8_t level = fields[1].GetUInt8();

        WDB::Structures::TaxiNodesEntry const* node = sTaxiNodesStore.lookupEntry(taxiNodeId);

        if (!node)
        {
            sLogger.failure("TaxiMgr:: Table `taxi_level_data` has data for nonexistent taxi node (ID: %u), skipped", taxiNodeId);
            continue;
        };

        _taxiNodeLevelDataStore.emplace(taxiNodeId, level);

        ++count;
    } while (result->NextRow());

    sLogger.info("TaxiMgr:: Loaded %u taxi node level definitions in %u ms", count, Util::GetTimeDifferenceToNow(oldMSTime));
}

bool TaxiMgr::isTaxiNodeUnlockedFor(uint32_t taxiNodeId, uint8_t level) const
{
    TaxiNodeLevelDataContainer::const_iterator itr = _taxiNodeLevelDataStore.find(taxiNodeId);
    if (itr != _taxiNodeLevelDataStore.end())
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
    std::set<uint32> spellPaths;

#if VERSION_STRING > WotLK
    for (uint32_t i = 0; i < sSpellEffectStore.getNumRows(); ++i)
    {
        if (WDB::Structures::SpellEffectEntry const* sInfo = sSpellEffectStore.lookupEntry(i))
        {
            if (sInfo->Effect == SPELL_EFFECT_START_TAXI)
                spellPaths.insert(sInfo->EffectMiscValue);
        }
    }
#else
    for (uint32_t i = 0; i < sSpellStore.getNumRows(); ++i)
    {
        if (WDB::Structures::SpellEntry const* sInfo = sSpellStore.lookupEntry(i))
        {
            for (uint8_t j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                if (sInfo->Effect[j] == SPELL_EFFECT_START_TAXI)
                    spellPaths.insert(sInfo->EffectMiscValue[j]);
            }
        }
    }
#endif

    memset(sTaxiNodesMask, 0, sizeof(sTaxiNodesMask));
    memset(sOldContinentsNodesMask, 0, sizeof(sOldContinentsNodesMask));
    memset(sHordeTaxiNodesMask, 0, sizeof(sHordeTaxiNodesMask));
    memset(sAllianceTaxiNodesMask, 0, sizeof(sAllianceTaxiNodesMask));
    memset(sDeathKnightTaxiNodesMask, 0, sizeof(sDeathKnightTaxiNodesMask));
    for (uint32_t i = 1; i < sTaxiNodesStore.getNumRows(); ++i)
    {
        WDB::Structures::TaxiNodesEntry const* node = sTaxiNodesStore.lookupEntry(i);
        if (!node)
            continue;

        TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(i);
        if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
        {
            bool ok = false;
            for (TaxiPathSetForSource::const_iterator dest_i = src_i->second.begin(); dest_i != src_i->second.end(); ++dest_i)
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
        uint8_t  field = (uint8_t)((i - 1) / 8);
        uint32_t submask = 1 << ((i - 1) % 8);

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