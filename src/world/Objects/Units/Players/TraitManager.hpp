/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"

#if defined(AE_FOREVER)

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Player;
class QueryBuffer;
class QueryResult;

namespace AscEmu::Traits
{
    enum class ConfigType : int32_t
    {
        Invalid = 0,
        Combat = 1,
        Profession = 2,
        Generic = 3
    };

    struct Entry
    {
        int32_t traitNodeId = 0;
        int32_t traitNodeEntryId = 0;
        int32_t rank = 0;
        int32_t grantedRanks = 0;
        int32_t bonusRanks = 0;
    };

    struct SubTree
    {
        int32_t traitSubTreeId = 0;
        bool active = false;
        std::vector<Entry> entries;
    };

    struct Config
    {
        int32_t id = 0;
        ConfigType type = ConfigType::Invalid;
        int32_t specializationId = 0;
        int32_t combatConfigFlags = 0;
        int32_t localIdentifier = 0;
        int32_t skillLineId = 0;
        int32_t traitSystemId = 0;
        int32_t variationId = 0;
        std::string name;
        int32_t savedConfigId = 0;
        int32_t savedLocalIdentifier = 0;
        std::vector<Entry> entries;
        std::vector<SubTree> subTrees;
    };
}

class SERVER_DECL TraitManager
{
public:
    explicit TraitManager(Player* owner) : m_owner(owner) { }

    bool commitConfig(AscEmu::Traits::Config config);
    bool commitConfigUpdate(AscEmu::Traits::Config const& update, int32_t savedConfigId = 0, int32_t savedLocalIdentifier = 0);
    AscEmu::Traits::Config* createGenericConfig(int32_t traitSystemId, int32_t variationId = 0);
    AscEmu::Traits::Config* createGenericConfigForTree(uint32_t traitTreeId);
    AscEmu::Traits::Config const* getConfig(int32_t configId) const;
    AscEmu::Traits::Config const* getGenericConfigBySystem(int32_t traitSystemId) const;
    bool hasGenericConfigForSystem(int32_t traitSystemId) const { return getGenericConfigBySystem(traitSystemId) != nullptr; }

    void loadFromDB(QueryResult* configs, QueryResult* entries, QueryResult* subTrees);
    void saveToDB(QueryBuffer* buf) const;
    void clear();
    void syncActivePlayerData(bool notifyClient = true);
    AscEmu::Traits::Config* ensureCombatConfigForCurrentSpec();
    int32_t getAvailableCurrency(int32_t traitCurrencyId, AscEmu::Traits::Config const* config = nullptr) const;
    int32_t getSpentCurrency(int32_t traitCurrencyId, AscEmu::Traits::Config const& config) const;

private:
    bool validateConfig(AscEmu::Traits::Config const& config) const;
    bool validateEntry(AscEmu::Traits::Config const& config, AscEmu::Traits::Entry const& entry, int32_t subTreeId) const;
    bool isTreeAllowedForConfig(AscEmu::Traits::Config const& config, uint32_t traitTreeId) const;
    bool validateCurrencyBudget(AscEmu::Traits::Config const& config) const;
    void applyTraitSpells();
    int32_t allocateConfigId() const;

    Player* m_owner = nullptr;
    std::unordered_map<int32_t, AscEmu::Traits::Config> m_configs;
    std::unordered_set<uint32_t> m_traitSpells;
};

#endif
