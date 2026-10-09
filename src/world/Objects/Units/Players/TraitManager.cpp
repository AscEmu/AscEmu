/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "TraitManager.hpp"

#if defined(AE_FOREVER)

#include "Player.hpp"
#include "Management/AchievementMgr.h"
#include "Database/Database.hpp"
#include "Database/Field.hpp"
#include "Logging/Logger.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/ForeverRuleset.hpp"
#include "Storage/WDB/WDBStores.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <ranges>
#include <unordered_set>

namespace
{
    constexpr std::size_t MaxTraitEntries = 100;
    constexpr std::size_t MaxTraitSubTrees = 10;

    uint64_t makeEntryKey(AscEmu::Traits::Entry const& entry)
    {
        return (uint64_t(uint32_t(entry.traitNodeId)) << 32) | uint32_t(entry.traitNodeEntryId);
    }

    constexpr int32_t TraitCombatConfigFlagActiveForSpec = 0x1;

    enum class TraitCurrencyType : int32_t
    {
        Gold = 0,
        CurrencyTypesBased = 1,
        TraitSourced = 2,
        TraitSourcedPlayerDataElement = 3
    };
}

int32_t TraitManager::getAvailableCurrency(int32_t traitCurrencyId, AscEmu::Traits::Config const* config) const
{
    if (!m_owner || traitCurrencyId <= 0)
        return 0;

    auto const* currency = sTraitCurrencyStore.lookupEntry(static_cast<uint32_t>(traitCurrencyId));
    if (!currency)
        return 0;

    switch (static_cast<TraitCurrencyType>(currency->type))
    {
        case TraitCurrencyType::Gold:
            return static_cast<int32_t>(std::min<uint64_t>(m_owner->getCoinage(), static_cast<uint64_t>(std::numeric_limits<int32_t>::max())));
        case TraitCurrencyType::CurrencyTypesBased:
            return currency->currencyTypesId > 0 ? static_cast<int32_t>(m_owner->getCurrency(static_cast<uint32_t>(currency->currencyTypesId))) : 0;
        case TraitCurrencyType::TraitSourced:
        {
            int64_t total = 0;
            auto hasTraitNodeEntry = [&](int32_t traitNodeEntryId)
            {
                if (!config || traitNodeEntryId <= 0)
                    return false;

                auto contains = [traitNodeEntryId](std::vector<AscEmu::Traits::Entry> const& entries)
                {
                    return std::ranges::any_of(entries, [traitNodeEntryId](AscEmu::Traits::Entry const& entry)
                    {
                        return entry.traitNodeEntryId == traitNodeEntryId && (entry.rank > 0 || entry.grantedRanks > 0);
                    });
                };

                if (contains(config->entries))
                    return true;
                return std::ranges::any_of(config->subTrees, [&](AscEmu::Traits::SubTree const& subTree) { return contains(subTree.entries); });
            };

            for (auto const& [id, source] : sTraitCurrencySourceStore)
            {
                (void)id;
                if (source.traitCurrencyId != static_cast<uint32_t>(traitCurrencyId))
                    continue;

                if (!AscEmu::Version::Forever::isSuperDistrictSetActiveForRealm(source.superDistrictSetId))
                    continue;
                if (source.questId > 0 && !m_owner->hasQuestFinished(static_cast<uint32_t>(source.questId)))
                    continue;
                if (source.achievementId > 0 && (!m_owner->getAchievementMgr() || !m_owner->getAchievementMgr()->hasCompleted(static_cast<uint32_t>(source.achievementId))))
                    continue;
                if (source.playerLevel > 0 && m_owner->getLevel() < static_cast<uint32_t>(source.playerLevel))
                    continue;
                if (source.traitNodeEntryId > 0 && !hasTraitNodeEntry(source.traitNodeEntryId))
                    continue;

                total += source.amount;
                if (total >= std::numeric_limits<int32_t>::max())
                    return std::numeric_limits<int32_t>::max();
            }

            return static_cast<int32_t>(std::max<int64_t>(0, total));
        }
        case TraitCurrencyType::TraitSourcedPlayerDataElement:
            sLogger.debug("[ForeverDebug][Traits] currency id={} uses unimplemented player-data-element source account={} character={}", traitCurrencyId, currency->playerDataElementAccountId, currency->playerDataElementCharacterId);
            return 0;
        default:
            sLogger.debug("[ForeverDebug][Traits] currency id={} has unknown type={}", traitCurrencyId, currency->type);
            return 0;
    }
}

int32_t TraitManager::getSpentCurrency(int32_t traitCurrencyId, AscEmu::Traits::Config const& config) const
{
    if (traitCurrencyId <= 0)
        return 0;

    int64_t spent = 0;
    auto addEntryCost = [&](AscEmu::Traits::Entry const& entry)
    {
        if (entry.rank <= 0)
            return;

        for (auto const& [id, relation] : sTraitNodeEntryXTraitCostStore)
        {
            (void)id;
            if (relation.traitNodeEntryId != static_cast<uint32_t>(entry.traitNodeEntryId))
                continue;

            auto const* cost = sTraitCostStore.lookupEntry(static_cast<uint32_t>(relation.traitCostId));
            if (!cost || cost->traitCurrencyId != traitCurrencyId || cost->amount <= 0)
                continue;

            spent += static_cast<int64_t>(cost->amount) * entry.rank;
            if (spent >= std::numeric_limits<int32_t>::max())
            {
                spent = std::numeric_limits<int32_t>::max();
                return;
            }
        }
    };

    for (auto const& entry : config.entries)
        addEntryCost(entry);
    for (auto const& subTree : config.subTrees)
        for (auto const& entry : subTree.entries)
            addEntryCost(entry);

    return static_cast<int32_t>(spent);
}

bool TraitManager::validateCurrencyBudget(AscEmu::Traits::Config const& config) const
{
    std::map<int32_t, int64_t> spent;

    auto addEntryCost = [&](AscEmu::Traits::Entry const& entry)
    {
        if (entry.rank <= 0)
            return true;

        for (auto const& [id, relation] : sTraitNodeEntryXTraitCostStore)
        {
            (void)id;
            if (relation.traitNodeEntryId != static_cast<uint32_t>(entry.traitNodeEntryId))
                continue;

            auto const* cost = sTraitCostStore.lookupEntry(static_cast<uint32_t>(relation.traitCostId));
            if (!cost || cost->traitCurrencyId <= 0 || cost->amount < 0)
                return false;

            spent[cost->traitCurrencyId] += static_cast<int64_t>(cost->amount) * entry.rank;
        }
        return true;
    };

    for (auto const& entry : config.entries)
        if (!addEntryCost(entry))
            return false;
    for (auto const& subTree : config.subTrees)
        for (auto const& entry : subTree.entries)
            if (!addEntryCost(entry))
                return false;

    for (auto const& [traitCurrencyId, amount] : spent)
    {
        const int32_t available = getAvailableCurrency(traitCurrencyId, &config);
        if (amount > available)
        {
            sLogger.debug("[ForeverDebug][Traits] reject config id={} currency={} spent={} available={}", config.id, traitCurrencyId, amount, available);
            return false;
        }
    }

    return true;
}

bool TraitManager::isTreeAllowedForConfig(AscEmu::Traits::Config const& config, uint32_t traitTreeId) const
{
    auto const* tree = sTraitTreeStore.lookupEntry(traitTreeId);
    if (!tree)
        return false;

    switch (config.type)
    {
        case AscEmu::Traits::ConfigType::Combat:
            return m_owner && config.specializationId == static_cast<int32_t>(m_owner->getCurrentSpecId())
                && std::ranges::any_of(sSkillLineXTraitTreeStore, [traitTreeId](auto const& pair)
                {
                    return pair.second.traitTreeId == static_cast<int32_t>(traitTreeId);
                });
        case AscEmu::Traits::ConfigType::Generic:
            return static_cast<int32_t>(tree->traitSystemId) == config.traitSystemId;
        case AscEmu::Traits::ConfigType::Profession:
            // Profession -> trait-tree ownership is carried by profession-specific
            // DB2 tables, not TraitTreeLoadout. The shared Trait graph can still
            // validate node/entry/rank/subtree relations here.
            return true;
        default:
            return false;
    }
}

bool TraitManager::validateEntry(AscEmu::Traits::Config const& config, AscEmu::Traits::Entry const& entry, int32_t subTreeId) const
{
    if (entry.traitNodeId <= 0 || entry.traitNodeEntryId <= 0 || entry.rank < 0 || entry.grantedRanks < 0 || entry.bonusRanks < 0)
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry invalid values node={} entry={} rank={} granted={} bonus={}", entry.traitNodeId, entry.traitNodeEntryId, entry.rank, entry.grantedRanks, entry.bonusRanks);
        return false;
    }

    auto const* node = sTraitNodeStore.lookupEntry(static_cast<uint32_t>(entry.traitNodeId));
    auto const* nodeEntry = sTraitNodeEntryStore.lookupEntry(static_cast<uint32_t>(entry.traitNodeEntryId));
    if (!node)
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry node={} missing", entry.traitNodeId);
        return false;
    }
    if (!nodeEntry)
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry nodeEntry={} missing", entry.traitNodeEntryId);
        return false;
    }
    if (!isTreeAllowedForConfig(config, node->traitTreeId))
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry node={} entry={} tree={} not allowed type={} spec={} currentSpec={}", entry.traitNodeId, entry.traitNodeEntryId, node->traitTreeId, static_cast<int32_t>(config.type), config.specializationId, m_owner ? m_owner->getCurrentSpecId() : 0);
        return false;
    }

    bool const linked = std::ranges::any_of(sTraitNodeXTraitNodeEntryStore, [&](auto const& pair)
    {
        auto const& relation = pair.second;
        return relation.traitNodeId == static_cast<uint32_t>(entry.traitNodeId) && relation.traitNodeEntryId == entry.traitNodeEntryId;
    });
    if (!linked)
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry node={} entry={} relation missing", entry.traitNodeId, entry.traitNodeEntryId);
        return false;
    }

    if (nodeEntry->maxRanks > 0 && entry.rank > nodeEntry->maxRanks)
    {
        sLogger.debug("[ForeverDebug][Traits] reject entry node={} entry={} rank={} maxRanks={}", entry.traitNodeId, entry.traitNodeEntryId, entry.rank, nodeEntry->maxRanks);
        return false;
    }

    if (subTreeId > 0)
    {
        auto const* subTree = sTraitSubTreeStore.lookupEntry(static_cast<uint32_t>(subTreeId));
        if (!subTree || subTree->traitTreeId != node->traitTreeId)
            return false;
        if (node->traitSubTreeId > 0 && node->traitSubTreeId != subTreeId)
            return false;
        if (nodeEntry->traitSubTreeId > 0 && nodeEntry->traitSubTreeId != subTreeId)
            return false;
    }
    else if (node->traitSubTreeId > 0 || nodeEntry->traitSubTreeId > 0)
        return false;

    if (nodeEntry->traitDefinitionId > 0 && !sTraitDefinitionStore.lookupEntry(static_cast<uint32_t>(nodeEntry->traitDefinitionId)))
        return false;

    for (auto const& [id, relation] : sTraitNodeEntryXTraitCostStore)
    {
        (void)id;
        if (relation.traitNodeEntryId != static_cast<uint32_t>(entry.traitNodeEntryId))
            continue;
        auto const* cost = sTraitCostStore.lookupEntry(static_cast<uint32_t>(relation.traitCostId));
        if (!cost || (cost->traitCurrencyId > 0 && !sTraitCurrencyStore.lookupEntry(static_cast<uint32_t>(cost->traitCurrencyId))))
            return false;
    }

    return true;
}

bool TraitManager::validateConfig(AscEmu::Traits::Config const& config) const
{
    if (config.id <= 0 || config.type == AscEmu::Traits::ConfigType::Invalid || config.type > AscEmu::Traits::ConfigType::Generic)
        return false;

    if (config.entries.size() > MaxTraitEntries || config.subTrees.size() > MaxTraitSubTrees)
        return false;

    if (config.type == AscEmu::Traits::ConfigType::Combat && config.specializationId <= 0)
        return false;
    if (config.type == AscEmu::Traits::ConfigType::Profession && config.skillLineId <= 0)
        return false;
    if (config.type == AscEmu::Traits::ConfigType::Generic && (config.traitSystemId <= 0 || !sTraitSystemStore.lookupEntry(static_cast<uint32_t>(config.traitSystemId))))
        return false;

    std::unordered_set<uint64_t> entries;
    auto validateUniqueEntry = [&](AscEmu::Traits::Entry const& entry, int32_t subTreeId)
    {
        if (!entries.insert(makeEntryKey(entry)).second)
            return false;
        return validateEntry(config, entry, subTreeId);
    };

    for (auto const& entry : config.entries)
        if (!validateUniqueEntry(entry, 0))
            return false;

    std::unordered_set<int32_t> subtreeIds;
    for (auto const& subTree : config.subTrees)
    {
        if (subTree.traitSubTreeId <= 0 || subTree.entries.size() > MaxTraitEntries || !subtreeIds.insert(subTree.traitSubTreeId).second)
            return false;
        for (auto const& entry : subTree.entries)
            if (!validateUniqueEntry(entry, subTree.traitSubTreeId))
                return false;
    }

    return validateCurrencyBudget(config);
}

void TraitManager::applyTraitSpells()
{
    if (!m_owner)
        return;

    std::unordered_set<uint32_t> desired;
    auto collect = [&](AscEmu::Traits::Entry const& entry)
    {
        if (entry.rank + entry.grantedRanks + entry.bonusRanks <= 0)
            return;
        auto const* nodeEntry = sTraitNodeEntryStore.lookupEntry(static_cast<uint32_t>(entry.traitNodeEntryId));
        if (!nodeEntry || nodeEntry->traitDefinitionId <= 0)
            return;
        auto const* definition = sTraitDefinitionStore.lookupEntry(static_cast<uint32_t>(nodeEntry->traitDefinitionId));
        if (definition && definition->spellId > 0 && sSpellStore.lookupEntry(static_cast<uint32_t>(definition->spellId)))
            desired.insert(static_cast<uint32_t>(definition->spellId));
    };

    const uint32_t currentSpecId = m_owner->getCurrentSpecId();
    for (auto const& [id, config] : m_configs)
    {
        (void)id;
        // Only the active combat configuration contributes combat trait spells.
        // Generic configs (Legacy) remain active, while profession configs are only
        // applied once their dedicated profession ownership path is implemented.
        if (config.type == AscEmu::Traits::ConfigType::Combat)
        {
            if (config.specializationId != static_cast<int32_t>(currentSpecId) || (config.combatConfigFlags & TraitCombatConfigFlagActiveForSpec) == 0)
                continue;
        }
        else if (config.type == AscEmu::Traits::ConfigType::Profession)
            continue;

        for (auto const& entry : config.entries)
            collect(entry);
        for (auto const& subTree : config.subTrees)
            if (subTree.active)
                for (auto const& entry : subTree.entries)
                    collect(entry);
    }

    for (uint32_t spellId : m_traitSpells)
        if (!desired.contains(spellId))
            m_owner->removeSpell(spellId, false);

    for (uint32_t spellId : desired)
        if (!m_owner->hasSpell(spellId))
            m_owner->addSpell(spellId);

    m_traitSpells = std::move(desired);
}

void TraitManager::syncActivePlayerData(bool notifyClient)
{
    if (!m_owner)
        return;

    auto& activePlayer = m_owner->foreverActivePlayerFields();
    auto previousConfigs = std::move(activePlayer.traitConfigs);
    const uint32_t previousActiveCombatConfigId = activePlayer.activeCombatTraitConfigId;

    activePlayer.traitConfigs.clear();
    activePlayer.traitConfigUpdateStates.clear();

    uint32_t activeCombatConfigId = 0;
    for (auto const& [id, config] : m_configs)
    {
        AscEmu::Version::Forever::Fields::TraitConfig wireConfig;
        wireConfig.id = config.id;
        wireConfig.name = config.name;
        wireConfig.type = static_cast<int32_t>(config.type);
        wireConfig.skillLineId = config.skillLineId;
        wireConfig.chrSpecializationId = config.specializationId;
        wireConfig.combatConfigFlags = config.combatConfigFlags;
        wireConfig.localIdentifier = config.localIdentifier;
        wireConfig.traitSystemId = config.traitSystemId;
        wireConfig.variationId = config.variationId;

        auto convertEntry = [](AscEmu::Traits::Entry const& entry)
        {
            AscEmu::Version::Forever::Fields::TraitEntry wireEntry;
            wireEntry.traitNodeId = entry.traitNodeId;
            wireEntry.traitNodeEntryId = entry.traitNodeEntryId;
            wireEntry.rank = entry.rank;
            wireEntry.grantedRanks = entry.grantedRanks;
            wireEntry.bonusRanks = entry.bonusRanks;
            return wireEntry;
        };

        wireConfig.entries.reserve(config.entries.size());
        for (auto const& entry : config.entries)
            wireConfig.entries.push_back(convertEntry(entry));

        wireConfig.subTrees.reserve(config.subTrees.size());
        for (auto const& subTree : config.subTrees)
        {
            AscEmu::Version::Forever::Fields::TraitSubTreeCache wireSubTree;
            wireSubTree.traitSubTreeId = subTree.traitSubTreeId;
            wireSubTree.active = subTree.active ? 1u : 0u;
            wireSubTree.entries.reserve(subTree.entries.size());
            for (auto const& entry : subTree.entries)
                wireSubTree.entries.push_back(convertEntry(entry));
            wireConfig.subTrees.push_back(std::move(wireSubTree));
        }

        activePlayer.traitConfigs.emplace(id, std::move(wireConfig));
        if (config.type == AscEmu::Traits::ConfigType::Combat &&
            config.specializationId == static_cast<int32_t>(m_owner->getCurrentSpecId()) &&
            (config.combatConfigFlags & TraitCombatConfigFlagActiveForSpec) != 0)
            activeCombatConfigId = static_cast<uint32_t>(id);
    }

    activePlayer.activeCombatTraitConfigId = activeCombatConfigId;

    if (!notifyClient)
        return;

    // Diff-map sync: mark every current config changed and every removed config deleted.
    // Changed entries send a complete nested TraitConfig update.
    for (auto const& [id, config] : activePlayer.traitConfigs)
        activePlayer.traitConfigUpdateStates[id] = AscEmu::Version::Forever::Fields::TraitConfigMapState::Changed;
    for (auto const& [id, config] : previousConfigs)
        if (!activePlayer.traitConfigs.contains(id))
            activePlayer.traitConfigUpdateStates[id] = AscEmu::Version::Forever::Fields::TraitConfigMapState::Deleted;

    if (!activePlayer.traitConfigUpdateStates.empty())
    {
        activePlayer.changes.set(AscEmu::Version::Forever::Fields::ActivePlayerData::TraitDataParentBit);
        activePlayer.changes.set(AscEmu::Version::Forever::Fields::ActivePlayerData::TraitConfigsBit);
    }

    if (previousActiveCombatConfigId != activePlayer.activeCombatTraitConfigId || activePlayer.activeCombatTraitConfigId != 0)
    {
        activePlayer.changes.set(AscEmu::Version::Forever::Fields::ActivePlayerData::TraitDataParentBit);
        activePlayer.changes.set(AscEmu::Version::Forever::Fields::ActivePlayerData::ActiveCombatTraitConfigIdBit);
    }

    if (m_owner->IsInWorld() && activePlayer.hasChanges())
        m_owner->updateObject();
}

bool TraitManager::commitConfig(AscEmu::Traits::Config config)
{
    if (!validateConfig(config))
    {
        sLogger.debug("[ForeverDebug][Traits] rejected trait config id={} type={} entries={} subtrees={}", config.id, static_cast<int32_t>(config.type), config.entries.size(), config.subTrees.size());
        return false;
    }

    m_configs[config.id] = std::move(config);
    applyTraitSpells();
    syncActivePlayerData(m_owner && m_owner->IsInWorld());
    auto const& stored = m_configs.at(config.id);
    sLogger.debug("[ForeverDebug][Traits] committed trait config id={} type={} entries={} subtrees={} spec={} skillLine={} traitSystem={}", stored.id, static_cast<int32_t>(stored.type), stored.entries.size(), stored.subTrees.size(), stored.specializationId, stored.skillLineId, stored.traitSystemId);
    return true;
}

bool TraitManager::commitConfigUpdate(AscEmu::Traits::Config const& update, int32_t savedConfigId, int32_t savedLocalIdentifier)
{
    auto itr = m_configs.find(update.id);
    if (itr == m_configs.end())
    {
        sLogger.debug("[ForeverDebug][Traits] rejected update for unknown config id={} type={}", update.id, static_cast<int32_t>(update.type));
        return false;
    }

    AscEmu::Traits::Config merged = itr->second;
    if (update.type != merged.type)
    {
        sLogger.debug("[ForeverDebug][Traits] rejected config id={} type mismatch current={} wire={}", update.id, static_cast<int32_t>(merged.type), static_cast<int32_t>(update.type));
        return false;
    }

    auto findEntry = [](std::vector<AscEmu::Traits::Entry>& entries, int32_t nodeId, int32_t nodeEntryId)
    {
        return std::find_if(entries.begin(), entries.end(), [=](AscEmu::Traits::Entry const& entry)
        {
            return entry.traitNodeId == nodeId && entry.traitNodeEntryId == nodeEntryId;
        });
    };

    for (auto const& changed : update.entries)
    {
        auto entry = findEntry(merged.entries, changed.traitNodeId, changed.traitNodeEntryId);
        if (changed.rank <= 0 && changed.grantedRanks <= 0 && changed.bonusRanks <= 0)
        {
            if (entry != merged.entries.end())
                merged.entries.erase(entry);
            continue;
        }

        if (entry != merged.entries.end())
            *entry = changed;
        else
            merged.entries.push_back(changed);
    }

    // Forever sends the config header with every commit. Keep ownership fields tied
    // to the server-side config and accept only mutable client-facing metadata here.
    if (!update.name.empty())
        merged.name = update.name;
    if (savedLocalIdentifier > 0)
        merged.localIdentifier = savedLocalIdentifier;
    merged.savedConfigId = savedConfigId;
    merged.savedLocalIdentifier = savedLocalIdentifier;

    if (!validateConfig(merged))
    {
        sLogger.debug("[ForeverDebug][Traits] rejected merged trait config id={} entries={} subtrees={}", merged.id, merged.entries.size(), merged.subTrees.size());
        return false;
    }

    m_configs[merged.id] = std::move(merged);
    applyTraitSpells();
    syncActivePlayerData(m_owner && m_owner->IsInWorld());
    auto const& stored = m_configs.at(update.id);
    sLogger.debug("[ForeverDebug][Traits] updated trait config id={} type={} entries={} spec={} flags=0x{:X}", stored.id, static_cast<int32_t>(stored.type), stored.entries.size(), stored.specializationId, stored.combatConfigFlags);
    return true;
}

AscEmu::Traits::Config* TraitManager::ensureCombatConfigForCurrentSpec()
{
    if (!m_owner)
        return nullptr;

    int32_t specId = static_cast<int32_t>(m_owner->getCurrentSpecId());
    if (specId <= 0)
    {
        const uint32_t* classSpecializations = getClassSpecializations(static_cast<uint8_t>(m_owner->getClass()));
        if (classSpecializations)
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                if (classSpecializations[index] == 0)
                    continue;

                specId = static_cast<int32_t>(classSpecializations[index]);
                m_owner->getActiveSpec().setSpecializationId(static_cast<uint32_t>(specId));
                m_owner->setCurrentSpecId(static_cast<uint32_t>(specId));
                sLogger.debug("[ForeverDebug][Traits] initialized default Classic 1.60 specialization spec={} class={} index={}", specId, m_owner->getClass(), index);
                break;
            }
        }
    }

    if (specId <= 0)
    {
        sLogger.debug("[ForeverDebug][Traits] cannot create active combat config: no specialization for class={}", m_owner->getClass());
        return nullptr;
    }

    for (auto& [id, config] : m_configs)
    {
        if (config.type == AscEmu::Traits::ConfigType::Combat && config.specializationId == specId && (config.combatConfigFlags & TraitCombatConfigFlagActiveForSpec) != 0)
            return &config;
    }

    AscEmu::Traits::Config config;
    config.id = allocateConfigId();
    config.type = AscEmu::Traits::ConfigType::Combat;
    config.specializationId = specId;
    config.combatConfigFlags = TraitCombatConfigFlagActiveForSpec;
    config.localIdentifier = 1;

    const int32_t id = config.id;
    if (!commitConfig(std::move(config)))
        return nullptr;

    sLogger.debug("[ForeverDebug][Traits] created active combat config id={} spec={}", id, specId);
    return &m_configs.at(id);
}

int32_t TraitManager::allocateConfigId() const
{
    int32_t id = 1;
    while (m_configs.contains(id))
        ++id;
    return id;
}

AscEmu::Traits::Config* TraitManager::createGenericConfigForTree(uint32_t traitTreeId)
{
    auto const* tree = sTraitTreeStore.lookupEntry(traitTreeId);
    if (!tree || tree->traitSystemId == 0)
        return nullptr;

    auto const* system = sTraitSystemStore.lookupEntry(tree->traitSystemId);
    if (!system)
        return nullptr;

    int32_t variationId = 0;
    switch (system->variationType)
    {
        case 0: // TraitSystemVariationType::None
            break;
        case 1: // TraitSystemVariationType::Spec
            variationId = m_owner ? static_cast<int32_t>(m_owner->getCurrentSpecId()) : 0;
            break;
        default:
            sLogger.debug("[ForeverDebug][Traits] unsupported variation type={} system={} tree={}", system->variationType, tree->traitSystemId, traitTreeId);
            return nullptr;
    }

    return createGenericConfig(static_cast<int32_t>(tree->traitSystemId), variationId);
}

AscEmu::Traits::Config* TraitManager::createGenericConfig(int32_t traitSystemId, int32_t variationId)
{
    if (traitSystemId <= 0)
        return nullptr;

    if (auto const* existing = getGenericConfigBySystem(traitSystemId))
        return &m_configs.at(existing->id);

    AscEmu::Traits::Config config;
    config.id = allocateConfigId();
    config.type = AscEmu::Traits::ConfigType::Generic;
    config.traitSystemId = traitSystemId;
    config.variationId = variationId;

    const int32_t id = config.id;
    if (!commitConfig(std::move(config)))
        return nullptr;
    return &m_configs.at(id);
}

AscEmu::Traits::Config const* TraitManager::getConfig(int32_t configId) const
{
    auto const itr = m_configs.find(configId);
    return itr != m_configs.end() ? &itr->second : nullptr;
}

AscEmu::Traits::Config const* TraitManager::getGenericConfigBySystem(int32_t traitSystemId) const
{
    auto const itr = std::find_if(m_configs.begin(), m_configs.end(), [traitSystemId](auto const& pair)
    {
        return pair.second.type == AscEmu::Traits::ConfigType::Generic && pair.second.traitSystemId == traitSystemId;
    });
    return itr != m_configs.end() ? &itr->second : nullptr;
}

void TraitManager::clear()
{
    m_configs.clear();
    m_traitSpells.clear();
}

void TraitManager::loadFromDB(QueryResult* configs, QueryResult* entries, QueryResult* subTrees)
{
    clear();

    if (configs)
    {
        do
        {
            Field* fields = configs->fetch();
            AscEmu::Traits::Config config;
            config.id = fields[0].asInt32();
            config.type = static_cast<AscEmu::Traits::ConfigType>(fields[1].asInt32());
            config.specializationId = fields[2].asInt32();
            config.combatConfigFlags = fields[3].asInt32();
            config.localIdentifier = fields[4].asInt32();
            config.skillLineId = fields[5].asInt32();
            config.traitSystemId = fields[6].asInt32();
            config.variationId = fields[7].asInt32();
            config.name = fields[8].asCString();
            config.savedConfigId = fields[9].asInt32();
            config.savedLocalIdentifier = fields[10].asInt32();
            m_configs.emplace(config.id, std::move(config));
        } while (configs->nextRow());
    }

    if (subTrees)
    {
        do
        {
            Field* fields = subTrees->fetch();
            auto itr = m_configs.find(fields[0].asInt32());
            if (itr == m_configs.end())
                continue;
            AscEmu::Traits::SubTree subtree;
            subtree.traitSubTreeId = fields[1].asInt32();
            subtree.active = fields[2].asBool();
            itr->second.subTrees.push_back(std::move(subtree));
        } while (subTrees->nextRow());
    }

    if (entries)
    {
        do
        {
            Field* fields = entries->fetch();
            auto itr = m_configs.find(fields[0].asInt32());
            if (itr == m_configs.end())
                continue;

            AscEmu::Traits::Entry entry;
            const int32_t subtreeId = fields[1].asInt32();
            entry.traitNodeId = fields[2].asInt32();
            entry.traitNodeEntryId = fields[3].asInt32();
            entry.rank = fields[4].asInt32();
            entry.grantedRanks = fields[5].asInt32();
            entry.bonusRanks = fields[6].asInt32();

            if (!subtreeId)
                itr->second.entries.push_back(entry);
            else if (auto subtree = std::find_if(itr->second.subTrees.begin(), itr->second.subTrees.end(), [subtreeId](auto const& value) { return value.traitSubTreeId == subtreeId; }); subtree != itr->second.subTrees.end())
                subtree->entries.push_back(entry);
        } while (entries->nextRow());
    }

    for (auto itr = m_configs.begin(); itr != m_configs.end();)
    {
        if (!validateConfig(itr->second))
            itr = m_configs.erase(itr);
        else
            ++itr;
    }

    ensureCombatConfigForCurrentSpec();
    applyTraitSpells();
    syncActivePlayerData(false);
}

void TraitManager::saveToDB(QueryBuffer* buf) const
{
    if (!buf || !m_owner)
        return;

    const uint32_t guid = m_owner->getGuidLow();
    buf->addQuery("DELETE FROM character_trait_config_entry WHERE guid = %u", guid);
    buf->addQuery("DELETE FROM character_trait_config_subtree WHERE guid = %u", guid);
    buf->addQuery("DELETE FROM character_trait_config WHERE guid = %u", guid);

    for (auto const& [id, config] : m_configs)
    {
        const std::string escapedName = CharacterDatabase.escapeString(config.name);
        buf->addQuery("INSERT INTO character_trait_config (guid, config_id, config_type, specialization_id, combat_config_flags, local_identifier, skill_line_id, trait_system_id, variation_id, name, saved_config_id, saved_local_identifier) VALUES (%u, %d, %d, %d, %d, %d, %d, %d, %d, '%s', %d, %d)", guid, id, static_cast<int32_t>(config.type), config.specializationId, config.combatConfigFlags, config.localIdentifier, config.skillLineId, config.traitSystemId, config.variationId, escapedName.c_str(), config.savedConfigId, config.savedLocalIdentifier);

        for (auto const& entry : config.entries)
            buf->addQuery("INSERT INTO character_trait_config_entry (guid, config_id, subtree_id, trait_node_id, trait_node_entry_id, `rank`, granted_ranks, bonus_ranks) VALUES (%u, %d, 0, %d, %d, %d, %d, %d)", guid, id, entry.traitNodeId, entry.traitNodeEntryId, entry.rank, entry.grantedRanks, entry.bonusRanks);

        for (auto const& subtree : config.subTrees)
        {
            buf->addQuery("INSERT INTO character_trait_config_subtree (guid, config_id, subtree_id, active) VALUES (%u, %d, %d, %u)", guid, id, subtree.traitSubTreeId, subtree.active ? 1u : 0u);
            for (auto const& entry : subtree.entries)
                buf->addQuery("INSERT INTO character_trait_config_entry (guid, config_id, subtree_id, trait_node_id, trait_node_entry_id, `rank`, granted_ranks, bonus_ranks) VALUES (%u, %d, %d, %d, %d, %d, %d, %d)", guid, id, subtree.traitSubTreeId, entry.traitNodeId, entry.traitNodeEntryId, entry.rank, entry.grantedRanks, entry.bonusRanks);
        }
    }
}

#endif
