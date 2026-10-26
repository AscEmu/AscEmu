/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace WDB::Structures
{
    struct AchievementEntry;
}

namespace Achievements
{
    // the achievement flag of an achievement that belongs to the account (5.x and later)
    constexpr uint32_t AchievementFlagAccount = 0x00020000;

    // before 5.x the tree of an achievement is built from its criteria, the root carries this bit
    constexpr uint32_t LegacyRootNode = 0x80000000;

    // how a node of a criteria tree is complete, the values of the client data
    enum class ProgressRule : uint8_t
    {
        Complete            = 0,    // the criteria of the node reaches the amount
        NotComplete         = 1,    // the criteria of the node does not reach the amount
        AllChildren         = 4,
        SumOfChildren       = 5,    // the progress of every criteria below adds up to the amount
        HighestChild        = 6,    // one criteria below reaches the amount
        StartedChildren     = 7,    // the amount of direct children has progress
        CompletedChildren   = 8,    // the amount of direct children is complete
        WeightedChildren    = 9     // the progress below, each times the amount of its node
    };

    // how a node of a modifier tree is true, the values of the client data
    enum class ConditionRule : uint8_t
    {
        Single              = 2,    // the condition of the node holds
        SingleNot           = 3,    // the condition of the node does not hold
        AllChildren         = 4,
        SomeChildren        = 8     // the amount of children holds, at least one
    };

    struct CriteriaRecord
    {
        uint32_t id = 0;
        uint16_t type = 0;
        int32_t asset = 0;
        uint32_t conditionTreeId = 0;
        uint8_t startEvent = 0;
        int32_t startAsset = 0;
        uint32_t timeLimit = 0;
        uint8_t failEvent = 0;
        int32_t failAsset = 0;
        uint32_t flags = 0;
        uint32_t worldStateId = 0;
        int32_t worldStateValue = 0;
    };

    struct ProgressNodeRecord
    {
        uint32_t id = 0;
        uint32_t parentId = 0;
        uint32_t criteriaId = 0;
        uint64_t amount = 0;
        uint8_t rule = 0;
        uint32_t flags = 0;
        int32_t order = 0;
        std::string description;
    };

    struct ConditionRecord
    {
        uint32_t id = 0;
        uint32_t parentId = 0;
        uint32_t type = 0;
        int32_t asset = 0;
        int32_t secondaryAsset = 0;
        int32_t tertiaryAsset = 0;
        uint8_t rule = 0;
        int32_t amount = 0;
    };

    struct ConditionNode
    {
        ConditionRecord record;
        std::vector<ConditionNode const*> children;
    };

    // a requirement of the world database for a criteria the client data leaves open
    struct CriteriaRequirement
    {
        uint8_t kind = 0;
        uint32_t param1 = 0;
        uint32_t param2 = 0;
    };

    struct Criteria
    {
        CriteriaRecord record;
        ConditionNode const* conditions = nullptr;
        std::vector<CriteriaRequirement> requirements;
        bool characterScope = false;            // a character achievement counts it
        bool accountScope = false;              // an achievement of the account counts it
    };

    struct ProgressNode
    {
        ProgressNodeRecord record;
        Criteria const* criteria = nullptr;
        std::vector<ProgressNode const*> children;
    };

    // criteria, criteria trees and modifier trees of the client data, linked to the achievements
    class AchievementTrees
    {
        AchievementTrees() = default;
        ~AchievementTrees() = default;

    public:
        static AchievementTrees& getInstance();

        AchievementTrees(AchievementTrees&&) = delete;
        AchievementTrees(AchievementTrees const&) = delete;
        AchievementTrees& operator=(AchievementTrees&&) = delete;
        AchievementTrees& operator=(AchievementTrees const&) = delete;

        void addCriteria(CriteriaRecord record);
        void addProgressNode(ProgressNodeRecord record);
        void addCondition(ConditionRecord record);

        // after the client data, links the trees and reports what the core does not handle yet
        void initialize();

        Criteria const* getCriteria(uint32_t criteriaId) const;
        std::vector<Criteria const*> const& getCriteriaOfType(uint16_t type) const;
        ConditionNode const* getConditionTree(uint32_t conditionTreeId) const;

        // the tree of an achievement, a tree shared with another achievement included
        ProgressNode const* getAchievementTree(uint32_t achievementId) const;

        // every achievement whose tree counts the criteria
        std::vector<uint32_t> const& getAchievementsOfCriteria(uint32_t criteriaId) const;

        // achievements that share the tree of an achievement
        std::vector<uint32_t> const& getSharingAchievements(uint32_t achievementId) const;

        // criteria a start event starts the timer of, criteria a fail event resets
        std::vector<Criteria const*> const& getCriteriaOfStartEvent(uint8_t event, uint32_t asset) const;
        std::vector<Criteria const*> const& getCriteriaOfFailEvent(uint8_t event, uint32_t asset) const;

        // a node with every criteria below it
        static void forEachCriteria(ProgressNode const* node, std::vector<Criteria const*>& result);

        // the client data names criteria rows instead of criteria trees (before 5.4)
        bool isBuiltFromCriteriaRows() const;

    private:
        void readTrees();
        void readCriteriaRows();
        void loadRequirements();
        void linkConditions();
        void linkProgressNodes();
        void linkAchievements();
        void report() const;

        // the root of the tree of an achievement
        uint32_t treeIdOf(WDB::Structures::AchievementEntry const* achievement) const;

        bool m_fromCriteriaRows = false;

        std::vector<CriteriaRecord> m_criteriaRecords;
        std::vector<ProgressNodeRecord> m_progressRecords;
        std::vector<ConditionRecord> m_conditionRecords;

        std::unordered_map<uint32_t, Criteria> m_criteria;
        std::unordered_map<uint32_t, ProgressNode> m_progressNodes;
        std::unordered_map<uint32_t, ConditionNode> m_conditions;

        std::unordered_map<uint16_t, std::vector<Criteria const*>> m_criteriaByType;
        std::unordered_map<uint32_t, ProgressNode const*> m_treeByAchievement;
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_achievementsByCriteria;
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_sharingAchievements;
        std::unordered_map<uint64_t, std::vector<Criteria const*>> m_criteriaByStartEvent;
        std::unordered_map<uint64_t, std::vector<Criteria const*>> m_criteriaByFailEvent;
    };
}

#define sAchievementTrees Achievements::AchievementTrees::getInstance()
