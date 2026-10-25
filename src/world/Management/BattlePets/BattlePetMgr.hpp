/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetDefines.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace WDB::Structures
{
    struct BattlePetSpeciesEntry;
}

struct ItemProperties;

// The data every journal shares: the species with their creatures and summon spells, the stat states of breeds
// and species, the quality multipliers and the breeds and starting qualities of the world database.
class BattlePetMgr
{
    BattlePetMgr() = default;
    ~BattlePetMgr() = default;

public:
    static BattlePetMgr& getInstance();

    BattlePetMgr(BattlePetMgr&&) = delete;
    BattlePetMgr(BattlePetMgr const&) = delete;
    BattlePetMgr& operator=(BattlePetMgr&&) = delete;
    BattlePetMgr& operator=(BattlePetMgr const&) = delete;

    // after the client data, the spells, the creatures and the world database
    void initialize();

    WDB::Structures::BattlePetSpeciesEntry const* getSpecies(uint32_t speciesId) const;
    WDB::Structures::BattlePetSpeciesEntry const* getSpeciesBySpell(uint32_t spellId) const;
    WDB::Structures::BattlePetSpeciesEntry const* getSpeciesByCreature(uint32_t creatureId) const;

    // the species an item teaches: the 5.x item table, else the spells the item teaches
    WDB::Structures::BattlePetSpeciesEntry const* getSpeciesByItem(ItemProperties const* itemProperties) const;

    uint16_t rollBreed(uint32_t speciesId) const;
    BattlePets::Quality getDefaultQuality(uint32_t speciesId) const;
    uint32_t selectDisplay(WDB::Structures::BattlePetSpeciesEntry const* species) const;

    // the stored display when the creature of this realm has it, else a display of this realm
    uint32_t realmDisplay(WDB::Structures::BattlePetSpeciesEntry const* species, uint32_t storedDisplay) const;

    // power, health and speed from breed, species, quality and level
    void calculateStats(BattlePets::BattlePet& pet) const;

    // a pet as the journal keeps it, from its stored record
    BattlePets::BattlePet makePet(LogonComm::BattlePetRecord record) const;

private:
    struct StatStates
    {
        int32_t power = 0;
        int32_t stamina = 0;
        int32_t speed = 0;
    };

    static void addState(StatStates& stats, uint32_t stateId, int32_t value);

    std::unordered_map<uint32_t, StatStates> m_breedStates;
    std::unordered_map<uint32_t, StatStates> m_speciesStates;
    std::unordered_map<uint8_t, float> m_qualityMultipliers;

    std::unordered_map<uint32_t, WDB::Structures::BattlePetSpeciesEntry const*> m_speciesBySpell;
    std::unordered_map<uint32_t, WDB::Structures::BattlePetSpeciesEntry const*> m_speciesByCreature;
    std::unordered_map<uint32_t, WDB::Structures::BattlePetSpeciesEntry const*> m_speciesByItem;

    std::unordered_map<uint32_t, std::vector<uint16_t>> m_breedsBySpecies;
    std::unordered_map<uint32_t, uint8_t> m_defaultQualities;
};

#define sBattlePetMgr BattlePetMgr::getInstance()
