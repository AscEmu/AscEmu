/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementTrees.hpp"

#include "AchievementConditions.hpp"
#include "AchievementEvents.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Server/ClientProtocol.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Management/Conditions/PlayerConditions.hpp"

#include <algorithm>
#include <map>
#include <set>

namespace Achievements
{
    namespace
    {
        uint64_t eventKey(uint8_t event, uint32_t asset)
        {
            return (static_cast<uint64_t>(event) << 32) | asset;
        }

        // the timer start events of the criteria rows before 5.4, an unknown one never starts
        uint8_t startTriggerOfTimer(uint32_t timerEvent)
        {
            switch (timerEvent)
            {
                case 1: return StartSendEvent;
                case 2: return StartAcceptQuest;
                case 5: return StartCastSpell;
                case 6: return WoW::getServerDataExpansion() == WoW::Expansion::_WotLK ? StartBeSpellTarget : StartGainAura;
                case 7: return StartKillCreature;
                case 9: return StartUseItem;
                default: return StartUnknown;
            }
        }
    }

    uint32_t AchievementTrees::treeIdOf(WDB::Structures::AchievementEntry const* achievement) const
    {
        return m_fromCriteriaRows ? (LegacyRootNode | achievement->ID) : achievement->criteriaTreeID;
    }

    bool AchievementTrees::isBuiltFromCriteriaRows() const
    {
        return m_fromCriteriaRows;
    }

    AchievementTrees& AchievementTrees::getInstance()
    {
        static AchievementTrees mInstance;
        return mInstance;
    }

    void AchievementTrees::addCriteria(CriteriaRecord record)
    {
        m_criteriaRecords.push_back(std::move(record));
    }

    void AchievementTrees::addProgressNode(ProgressNodeRecord record)
    {
        m_progressRecords.push_back(std::move(record));
    }

    void AchievementTrees::addCondition(ConditionRecord record)
    {
        m_conditionRecords.push_back(std::move(record));
    }

    void AchievementTrees::readTrees()
    {
        for (auto const& [criteriaId, criteria] : sCriteriaStore)
        {
            auto const* entry = &criteria;

            CriteriaRecord record;
            record.id = entry->id;
            record.type = static_cast<uint16_t>(entry->type);
            record.asset = entry->asset;
            record.conditionTreeId = entry->modifierTreeId;
            record.startEvent = static_cast<uint8_t>(entry->startEvent);
            record.startAsset = entry->startAsset;
            record.timeLimit = entry->startTimer;
            record.failEvent = static_cast<uint8_t>(entry->failEvent);
            record.failAsset = entry->failAsset;
            record.flags = entry->flags;
            record.worldStateId = entry->eligibilityWorldStateId;
            record.worldStateValue = entry->eligibilityWorldStateValue;
            m_criteriaRecords.push_back(record);
        }

        for (auto const& [nodeId, node] : sCriteriaTreeStore)
        {
            auto const* entry = &node;

            ProgressNodeRecord record;
            record.id = entry->id;
            record.parentId = entry->parent;
            record.criteriaId = entry->criteriaId;
            record.amount = entry->amount;
            record.rule = static_cast<uint8_t>(entry->operatorType);
            record.flags = entry->flags;
            m_progressRecords.push_back(std::move(record));
        }

        for (auto const& [nodeId, node] : sModifierTreeStore)
        {
            auto const* entry = &node;

            ConditionRecord record;
            record.id = entry->id;
            record.parentId = entry->parent;
            record.type = entry->type;
            record.asset = entry->asset;
            record.secondaryAsset = entry->secondaryAsset;
            record.rule = static_cast<uint8_t>(entry->operatorType);
            record.amount = static_cast<int32_t>(entry->amount);
            m_conditionRecords.push_back(record);
        }
    }

    void AchievementTrees::readCriteriaRows()
    {
        m_fromCriteriaRows = true;

        // the highest amount of the criteria of an achievement, the goal of a summing achievement
        std::unordered_map<uint32_t, uint64_t> highestAmount;
        uint32_t nextConditionId = 1;

        for (auto const& [criteriaId, criteria] : sAchievementCriteriaStore)
        {
            auto const* entry = &criteria;

            CriteriaRecord record;
            record.id = entry->ID;
            record.type = static_cast<uint16_t>(entry->requiredType);
            record.asset = static_cast<int32_t>(entry->asset);
            record.flags = entry->flags;
            record.failEvent = static_cast<uint8_t>(entry->failEvent);
            record.failAsset = static_cast<int32_t>(entry->failAsset);
            if (entry->timerStartEvent != 0)
            {
                record.startEvent = startTriggerOfTimer(entry->timerStartEvent);
                record.startAsset = static_cast<int32_t>(entry->timerAsset);
                record.timeLimit = entry->timerTime;
            }
            else
            {
                record.startEvent = static_cast<uint8_t>(entry->startEvent);
                record.startAsset = static_cast<int32_t>(entry->startAsset);
            }

            record.worldStateId = entry->worldStateId;
            record.worldStateValue = entry->worldStateValue;

            // the conditions of the criteria row become a tree where all of them have to hold, the rows of 3.x have none
            for (uint8_t slot = 0; slot < 3; ++slot)
            {
                if (entry->conditionType[slot] == 0)
                    continue;

                if (record.conditionTreeId == 0)
                {
                    record.conditionTreeId = nextConditionId++;
                    m_conditionRecords.push_back({ record.conditionTreeId, 0, 0, 0, 0, 0, static_cast<uint8_t>(ConditionRule::AllChildren), 0 });
                }

                m_conditionRecords.push_back({ nextConditionId++, record.conditionTreeId, entry->conditionType[slot], static_cast<int32_t>(entry->conditionValue[slot]), 0, 0, static_cast<uint8_t>(ConditionRule::Single), 0 });
            }
            m_criteriaRecords.push_back(record);

            ProgressNodeRecord leaf;
            leaf.id = entry->ID;
            leaf.parentId = LegacyRootNode | entry->referredAchievement;
            leaf.criteriaId = entry->ID;
            leaf.amount = entry->quantity;
            leaf.rule = static_cast<uint8_t>(ProgressRule::Complete);
            leaf.order = static_cast<int32_t>(entry->order);
            m_progressRecords.push_back(std::move(leaf));

            uint64_t& highest = highestAmount[entry->referredAchievement];
            highest = std::max<uint64_t>(highest, entry->quantity);
        }

        for (auto const& [achievementId, achievementEntry] : sAchievementStore)
        {
            auto const* achievement = &achievementEntry;

            // an achievement sharing the criteria of another one counts them with its own rule
            uint32_t const owner = achievement->refAchievement != 0 ? achievement->refAchievement : achievement->ID;
            auto const amount = highestAmount.find(owner);
            if (amount == highestAmount.end())
                continue;

            ProgressNodeRecord root;
            root.id = treeIdOf(achievement);
            if (achievement->flags & ACHIEVEMENT_FLAG_SUMM)
            {
                root.rule = static_cast<uint8_t>(ProgressRule::SumOfChildren);
                root.amount = amount->second;
            }
            else if (achievement->count != 0)
            {
                root.rule = static_cast<uint8_t>(ProgressRule::CompletedChildren);
                root.amount = achievement->count;
            }
            else
            {
                root.rule = static_cast<uint8_t>(ProgressRule::AllChildren);
            }
            m_progressRecords.push_back(std::move(root));
        }
    }

    void AchievementTrees::loadRequirements()
    {
        uint32_t const build = WoW::getConfigBuild();
        auto result = WorldDatabase.query("SELECT criteria_id, requirement, param1, param2 FROM achievement_criteria_requirement WHERE min_build <= %u AND max_build >= %u", build, build);
        if (result == nullptr)
            return;

        uint32_t count = 0;
        uint32_t unknown = 0;
        do
        {
            Field* fields = result->fetch();
            auto const criteria = m_criteria.find(fields[0].asUint32());
            if (criteria == m_criteria.end())
            {
                ++unknown;
                continue;
            }

            criteria->second.requirements.push_back({ fields[1].asUint8(), fields[2].asUint32(), fields[3].asUint32() });
            ++count;
        } while (result->nextRow());

        sLogger.info("AchievementTrees : {} criteria requirements loaded, {} name criteria the client data does not know", count, unknown);
    }

    void AchievementTrees::linkConditions()
    {
        for (auto& record : m_conditionRecords)
            m_conditions[record.id].record = std::move(record);

        for (auto& [id, node] : m_conditions)
        {
            if (node.record.parentId == 0 || node.record.parentId == id)
                continue;

            if (auto const parent = m_conditions.find(node.record.parentId); parent != m_conditions.end())
                parent->second.children.push_back(&node);
        }
    }

    void AchievementTrees::linkProgressNodes()
    {
        for (auto& record : m_criteriaRecords)
        {
            Criteria& criteria = m_criteria[record.id];
            criteria.record = std::move(record);
            criteria.conditions = getConditionTree(criteria.record.conditionTreeId);
        }

        for (auto& record : m_progressRecords)
            m_progressNodes[record.id].record = std::move(record);

        for (auto& [id, node] : m_progressNodes)
        {
            if (node.record.criteriaId != 0)
                node.criteria = getCriteria(node.record.criteriaId);

            if (node.record.parentId == 0 || node.record.parentId == id)
                continue;

            if (auto const parent = m_progressNodes.find(node.record.parentId); parent != m_progressNodes.end())
                parent->second.children.push_back(&node);
        }

        // the client lists the children in their order
        for (auto& [id, node] : m_progressNodes)
        {
            std::sort(node.children.begin(), node.children.end(), [](ProgressNode const* left, ProgressNode const* right)
            {
                return left->record.order != right->record.order ? left->record.order < right->record.order : left->record.id < right->record.id;
            });
        }
    }

    void AchievementTrees::forEachCriteria(ProgressNode const* node, std::vector<Criteria const*>& result)
    {
        if (node == nullptr)
            return;

        if (node->criteria != nullptr)
            result.push_back(node->criteria);

        for (ProgressNode const* child : node->children)
            forEachCriteria(child, result);
    }

    void AchievementTrees::linkAchievements()
    {
        for (auto const& [achievementId, achievementEntry] : sAchievementStore)
        {
            auto const* achievement = &achievementEntry;
            if (treeIdOf(achievement) == 0)
                continue;

            if (auto const node = m_progressNodes.find(treeIdOf(achievement)); node != m_progressNodes.end())
                m_treeByAchievement[achievement->ID] = &node->second;

            // the root of a sharing achievement gets the criteria of the achievement it shares
            if (m_fromCriteriaRows && achievement->refAchievement != 0)
            {
                auto const own = m_progressNodes.find(treeIdOf(achievement));
                auto const shared = m_progressNodes.find(LegacyRootNode | achievement->refAchievement);
                if (own != m_progressNodes.end() && shared != m_progressNodes.end())
                    own->second.children = shared->second.children;
            }
        }

        // an achievement without a tree of its own counts the tree of the achievement it shares
        for (auto const& [achievementId, achievementEntry] : sAchievementStore)
        {
            auto const* achievement = &achievementEntry;
            if (achievement->refAchievement == 0)
                continue;

            m_sharingAchievements[achievement->refAchievement].push_back(achievement->ID);
            if (!m_treeByAchievement.contains(achievement->ID))
                if (auto const shared = m_treeByAchievement.find(achievement->refAchievement); shared != m_treeByAchievement.end())
                    m_treeByAchievement[achievement->ID] = shared->second;
        }

        std::vector<Criteria const*> leaves;
        for (auto const& [achievementId, tree] : m_treeByAchievement)
        {
            auto const* achievement = sAchievementStore.lookupEntry(achievementId);
            if (achievement == nullptr)
                continue;

            leaves.clear();
            forEachCriteria(tree, leaves);
            for (Criteria const* leaf : leaves)
            {
                auto& achievements = m_achievementsByCriteria[leaf->record.id];
                if (std::find(achievements.begin(), achievements.end(), achievementId) == achievements.end())
                    achievements.push_back(achievementId);

                Criteria& criteria = m_criteria[leaf->record.id];
                if (achievement->flags & AchievementFlagAccount)
                    criteria.accountScope = true;
                else
                    criteria.characterScope = true;
            }
        }

        // only criteria of achievements are tracked, scenario and quest criteria have no owner yet
        for (auto const& [criteriaId, achievements] : m_achievementsByCriteria)
        {
            Criteria const* criteria = getCriteria(criteriaId);
            if (criteria == nullptr)
                continue;

            m_criteriaByType[criteria->record.type].push_back(criteria);
            if (criteria->record.startEvent != 0)
                m_criteriaByStartEvent[eventKey(criteria->record.startEvent, static_cast<uint32_t>(criteria->record.startAsset))].push_back(criteria);
            if (criteria->record.failEvent != 0)
                m_criteriaByFailEvent[eventKey(criteria->record.failEvent, static_cast<uint32_t>(criteria->record.failAsset))].push_back(criteria);
        }
    }

    void AchievementTrees::initialize()
    {
        // the trees of 5.4 and later, before 5.4 they are built from the criteria rows
        if (m_criteriaRecords.empty())
        {
            if (!sCriteriaStore.empty())
                readTrees();
            else
                readCriteriaRows();
        }

        linkConditions();
        linkProgressNodes();
        linkAchievements();
        loadRequirements();

        m_criteriaRecords.clear();
        m_progressRecords.clear();
        m_conditionRecords.clear();

        sLogger.info("AchievementTrees : {} criteria, {} tree nodes, {} modifier nodes, {} achievements with a tree",
            m_criteria.size(), m_progressNodes.size(), m_conditions.size(), m_treeByAchievement.size());

        report();
    }

    void AchievementTrees::report() const
    {
        // criteria types no game event updates yet
        std::map<uint16_t, uint32_t> untrackedTypes;
        for (auto const& [type, criteriaList] : m_criteriaByType)
            if (!isCriteriaTypeTracked(type))
                untrackedTypes[type] += static_cast<uint32_t>(criteriaList.size());

        // conditions the core can not evaluate yet, they keep their criteria from progressing
        std::map<uint32_t, uint32_t> unhandledConditions;
        std::set<uint32_t> playerConditions;
        std::set<uint32_t> visited;
        std::vector<ConditionNode const*> pending;
        for (auto const& [criteriaId, achievements] : m_achievementsByCriteria)
        {
            Criteria const* criteria = getCriteria(criteriaId);
            if (criteria == nullptr || criteria->conditions == nullptr)
                continue;

            pending.push_back(criteria->conditions);
            while (!pending.empty())
            {
                ConditionNode const* node = pending.back();
                pending.pop_back();
                if (!visited.insert(node->record.id).second)
                    continue;

                if (node->record.type != 0 && !isConditionTypeHandled(node->record.type))
                    ++unhandledConditions[node->record.type];

                if (node->record.type == ConditionTypePlayerCondition || node->record.type == ConditionTypeTargetPlayerCondition)
                    playerConditions.insert(static_cast<uint32_t>(node->record.asset));

                for (ConditionNode const* child : node->children)
                    pending.push_back(child);

                // a condition naming another modifier tree
                if (node->record.type == ConditionTypeModifierTree)
                    if (ConditionNode const* linked = getConditionTree(static_cast<uint32_t>(node->record.asset)))
                        pending.push_back(linked);
            }
        }

        // requirements of the world database the core can not evaluate yet
        std::map<uint32_t, uint32_t> unhandledRequirements;
        for (auto const& [criteriaId, achievements] : m_achievementsByCriteria)
            if (Criteria const* criteria = getCriteria(criteriaId))
                for (CriteriaRequirement const& requirement : criteria->requirements)
                    if (!isRequirementKindHandled(requirement.kind))
                        ++unhandledRequirements[requirement.kind];

        auto format = [](auto const& counts)
        {
            std::string text;
            for (auto const& [type, count] : counts)
                text += (text.empty() ? "" : ", ") + std::to_string(type) + " (" + std::to_string(count) + ")";
            return text;
        };

        // start and fail events no game event sends yet keep their criteria from progressing
        std::map<uint32_t, uint32_t> unhookedStarts;
        std::map<uint32_t, uint32_t> unhookedFails;
        for (auto const& [criteriaId, achievements] : m_achievementsByCriteria)
        {
            Criteria const* criteria = getCriteria(criteriaId);
            if (criteria == nullptr)
                continue;
            if (criteria->record.startEvent != 0 && !isStartEventHooked(criteria->record.startEvent))
                ++unhookedStarts[criteria->record.startEvent];
            if (criteria->record.failEvent != 0 && !isFailEventHooked(criteria->record.failEvent))
                ++unhookedFails[criteria->record.failEvent];
        }

        if (!unhookedStarts.empty())
            sLogger.info("AchievementTrees : criteria start events without game events, event (criteria): {}", format(unhookedStarts));
        if (!unhookedFails.empty())
            sLogger.info("AchievementTrees : criteria fail events without game events, event (criteria): {}", format(unhookedFails));

        if (!untrackedTypes.empty())
            sLogger.info("AchievementTrees : criteria types without game events, type (criteria): {}", format(untrackedTypes));

        if (!unhandledConditions.empty())
            sLogger.info("AchievementTrees : modifier types not evaluated yet, type (nodes): {}", format(unhandledConditions));

        if (!unhandledRequirements.empty())
            sLogger.info("AchievementTrees : criteria requirements not evaluated yet, requirement (rows): {}", format(unhandledRequirements));

        Conditions::reportPlayerConditions(playerConditions);
    }

    Criteria const* AchievementTrees::getCriteria(uint32_t criteriaId) const
    {
        auto const itr = m_criteria.find(criteriaId);
        return itr != m_criteria.end() ? &itr->second : nullptr;
    }

    std::vector<Criteria const*> const& AchievementTrees::getCriteriaOfType(uint16_t type) const
    {
        static std::vector<Criteria const*> const empty;
        auto const itr = m_criteriaByType.find(type);
        return itr != m_criteriaByType.end() ? itr->second : empty;
    }

    ConditionNode const* AchievementTrees::getConditionTree(uint32_t conditionTreeId) const
    {
        if (conditionTreeId == 0)
            return nullptr;

        auto const itr = m_conditions.find(conditionTreeId);
        return itr != m_conditions.end() ? &itr->second : nullptr;
    }

    ProgressNode const* AchievementTrees::getAchievementTree(uint32_t achievementId) const
    {
        auto const itr = m_treeByAchievement.find(achievementId);
        return itr != m_treeByAchievement.end() ? itr->second : nullptr;
    }

    std::vector<uint32_t> const& AchievementTrees::getAchievementsOfCriteria(uint32_t criteriaId) const
    {
        static std::vector<uint32_t> const empty;
        auto const itr = m_achievementsByCriteria.find(criteriaId);
        return itr != m_achievementsByCriteria.end() ? itr->second : empty;
    }

    std::vector<Criteria const*> const& AchievementTrees::getCriteriaOfStartEvent(uint8_t event, uint32_t asset) const
    {
        static std::vector<Criteria const*> const empty;
        auto const itr = m_criteriaByStartEvent.find(eventKey(event, asset));
        return itr != m_criteriaByStartEvent.end() ? itr->second : empty;
    }

    std::vector<Criteria const*> const& AchievementTrees::getCriteriaOfFailEvent(uint8_t event, uint32_t asset) const
    {
        static std::vector<Criteria const*> const empty;
        auto const itr = m_criteriaByFailEvent.find(eventKey(event, asset));
        return itr != m_criteriaByFailEvent.end() ? itr->second : empty;
    }

    std::vector<uint32_t> const& AchievementTrees::getSharingAchievements(uint32_t achievementId) const
    {
        static std::vector<uint32_t> const empty;
        auto const itr = m_sharingAchievements.find(achievementId);
        return itr != m_sharingAchievements.end() ? itr->second : empty;
    }
}
