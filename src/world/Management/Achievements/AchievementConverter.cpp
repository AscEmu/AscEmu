/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementConverter.hpp"

#include "AchievementConditions.hpp"
#include "AchievementEvents.hpp"
#include "AchievementTrees.hpp"
#include "LogonCommAchievements.hpp"
#include "LogonCommDefines.hpp"
#include "Logging/Logger.hpp"
#include "Network/WorldPacket.hpp"
#include "Server/ClientProtocol.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <algorithm>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace Achievements
{
    namespace
    {
        // the first build whose criteria the trees name
        constexpr uint32_t TreeCriteriaBuild = WoW::Build::MOP_BUILD;

        struct PendingAccount
        {
            std::map<uint32_t, uint64_t> achievements;
            std::map<uint32_t, std::pair<uint32_t, uint64_t>> criteria;
        };

        std::mutex pendingLock;
        std::map<uint32_t, PendingAccount> pendingAccounts;

        // a condition as modifier type and value, the shape the trees of 5.4 use
        using Condition = std::pair<uint32_t, uint32_t>;

        // one condition of an old criteria, met by any of its forms in the trees
        using ConditionForms = std::vector<Condition>;

        struct OldCriteria
        {
            uint32_t achievementId = 0;
            uint32_t type = 0;
            int32_t asset = 0;
            std::vector<ConditionForms> conditions;
        };

        // the criteria of the builds before 5.4: the old client data, else the world table made from it
        std::unordered_map<uint32_t, OldCriteria> oldCriteria;

        // the modifier types the requirements of the world database correspond to
        void addRequirement(OldCriteria& old, uint8_t kind, uint32_t param1, uint32_t param2)
        {
            switch (kind)
            {
                case 1:     // target creature
                    old.conditions.push_back({ { 4, param1 } });
                    break;
                case 2:     // target class and race
                    if (param1 != 0)
                        old.conditions.push_back({ { 28, param1 } });
                    if (param2 != 0)
                        old.conditions.push_back({ { 27, param2 } });
                    break;
                case 5:     // target aura
                    old.conditions.push_back({ { 10, param1 } });
                    break;
                case 6:     // target level
                    old.conditions.push_back({ { 70, param1 } });
                    break;
                case 10:    // player aura
                    old.conditions.push_back({ { 8, param1 } });
                    break;
                case 11:    // zone or area
                    old.conditions.push_back({ { 17, param1 }, { 41, param1 } });
                    break;
                case 12:    // map
                    old.conditions.push_back({ { 32, param1 } });
                    break;
                case 13:    // player class and race
                    if (param1 != 0)
                        old.conditions.push_back({ { 26, param1 } });
                    if (param2 != 0)
                        old.conditions.push_back({ { 25, param2 } });
                    break;
                case 15:    // known title
                    old.conditions.push_back({ { 38, param1 } });
                    break;
                case 32:    // item quality
                    old.conditions.push_back({ { 15, param1 } });
                    break;
                default:
                    break;
            }
        }

        void loadOldCriteria()
        {
            for (auto const& [criteriaId, criteria] : sAchievementCriteriaStore)
            {
                auto const* entry = &criteria;

                OldCriteria& old = oldCriteria[entry->ID];
                old = { entry->referredAchievement, entry->requiredType, static_cast<int32_t>(entry->asset), {} };

                // the conditions of the 4.x rows carry the modifier types already, the 3.x rows have none
                for (uint8_t slot = 0; slot < 3; ++slot)
                    if (entry->conditionType[slot] != 0)
                        old.conditions.push_back({ { entry->conditionType[slot], entry->conditionValue[slot] } });
            }

            if (oldCriteria.empty())
            {
                if (auto result = WorldDatabase.query("SELECT criteria_id, achievement_id, type, asset FROM achievement_criteria_legacy"))
                {
                    do
                    {
                        Field* fields = result->fetch();
                        oldCriteria[fields[0].asUint32()] = { fields[1].asUint32(), fields[2].asUint32(), fields[3].asInt32(), {} };
                    }
                    while (result->nextRow());
                }
            }

            // the requirements the world database gave the old criteria, the rows of the latest build first
            std::set<uint32_t> withRequirements;
            if (auto result = WorldDatabase.query("SELECT criteria_id, requirement, param1, param2, min_build FROM achievement_criteria_requirement WHERE min_build < %u ORDER BY criteria_id, min_build DESC", TreeCriteriaBuild))
            {
                uint32_t currentId = 0;
                uint32_t currentBuild = 0;
                do
                {
                    Field* fields = result->fetch();
                    uint32_t const criteriaId = fields[0].asUint32();
                    uint32_t const minBuild = fields[4].asUint32();
                    if (criteriaId != currentId)
                    {
                        currentId = criteriaId;
                        currentBuild = minBuild;
                    }

                    // only the rows of one build
                    if (minBuild != currentBuild)
                        continue;

                    auto const old = oldCriteria.find(criteriaId);
                    if (old != oldCriteria.end())
                        addRequirement(old->second, fields[1].asUint8(), fields[2].asUint32(), fields[3].asUint32());
                }
                while (result->nextRow());
            }
        }

        // the conditions of a criteria of the trees, linked trees included
        void collectConditions(ConditionNode const* node, std::set<Condition>& conditions, uint8_t depth = 0)
        {
            if (node == nullptr || depth > 16)
                return;

            if (node->record.rule == static_cast<uint8_t>(ConditionRule::Single) && node->record.type != 0)
            {
                conditions.insert({ node->record.type, static_cast<uint32_t>(node->record.asset) });
                if (node->record.type == ConditionTypeModifierTree)
                    collectConditions(sAchievementTrees.getConditionTree(static_cast<uint32_t>(node->record.asset)), conditions, depth + 1);
            }

            for (ConditionNode const* child : node->children)
                collectConditions(child, conditions, depth + 1);
        }

        bool meetsOldConditions(Criteria const* criteria, OldCriteria const& old)
        {
            std::set<Condition> conditions;
            collectConditions(criteria->conditions, conditions);

            for (ConditionForms const& forms : old.conditions)
                if (std::ranges::none_of(forms, [&conditions](Condition const& form) { return conditions.contains(form); }))
                    return false;
            return true;
        }

        // the criteria of the trees that counts what an old criteria counted: same achievement, type, asset and conditions
        Criteria const* mapOldCriteria(uint32_t oldCriteriaId)
        {
            auto const itr = oldCriteria.find(oldCriteriaId);
            if (itr == oldCriteria.end())
                return nullptr;

            OldCriteria const& old = itr->second;
            std::vector<Criteria const*> leaves;
            AchievementTrees::forEachCriteria(sAchievementTrees.getAchievementTree(old.achievementId), leaves);

            std::vector<Criteria const*> sameAsset;
            std::vector<Criteria const*> sameType;
            for (Criteria const* leaf : leaves)
            {
                if (leaf->record.type != old.type || std::ranges::find(sameType, leaf) != sameType.end())
                    continue;

                sameType.push_back(leaf);
                if (leaf->record.asset == old.asset)
                    sameAsset.push_back(leaf);
            }

            // an asset that moved into the tree: the criteria of its type
            std::vector<Criteria const*>& candidates = sameAsset.empty() ? sameType : sameAsset;

            // the conditions of the old criteria have to hold for the new one
            std::erase_if(candidates, [&old](Criteria const* candidate) { return !meetsOldConditions(candidate, old); });
            if (candidates.size() == 1)
                return candidates.front();

            // several fit: the criteria that kept its id, else none, a wrong guess would complete other criteria
            auto const kept = std::ranges::find_if(candidates, [oldCriteriaId](Criteria const* candidate) { return candidate->record.id == oldCriteriaId; });
            return kept != candidates.end() ? *kept : nullptr;
        }

        bool isAccountAchievement(uint32_t achievementId)
        {
            auto const* achievement = sAchievementStore.lookupEntry(achievementId);
            return achievement != nullptr && (achievement->flags & AchievementFlagAccount) != 0;
        }
    }

    void convertCharacters(std::vector<std::pair<uint32_t, uint32_t>> const& characters)
    {
        if (characters.empty())
            return;

        loadOldCriteria();

        uint32_t converted = 0;
        uint32_t mapped = 0;
        uint32_t dropped = 0;
        uint32_t accountAchievements = 0;

        for (auto const& [guid, accountId] : characters)
        {
            std::lock_guard lock(pendingLock);
            PendingAccount& account = pendingAccounts[accountId];

            if (auto completed = CharacterDatabase.query("SELECT achievement, date FROM character_achievement WHERE guid = %u", guid))
            {
                do
                {
                    Field* row = completed->fetch();
                    uint32_t const achievementId = row[0].asUint32();
                    uint64_t const date = row[1].asUint64();
                    if (!isAccountAchievement(achievementId))
                        continue;

                    auto [itr, added] = account.achievements.try_emplace(achievementId, date);
                    if (!added)
                        itr->second = std::min(itr->second, date);
                    ++accountAchievements;
                }
                while (completed->nextRow());
            }

            // the progress under the criteria ids of the trees, the highest counter of merged criteria
            std::map<uint32_t, std::pair<uint32_t, uint64_t>> progress;
            if (auto rows = CharacterDatabase.query("SELECT criteria, counter, date FROM character_achievement_progress WHERE guid = %u", guid))
            {
                do
                {
                    Field* row = rows->fetch();
                    Criteria const* criteria = mapOldCriteria(row[0].asUint32());
                    if (criteria == nullptr)
                    {
                        ++dropped;
                        continue;
                    }

                    // derived criteria follow the player at login
                    if (isDerivedType(criteria->record.type))
                        continue;

                    uint32_t const counter = row[1].asUint32();
                    uint64_t const date = row[2].asUint64();
                    auto merge = [counter, date](std::pair<uint32_t, uint64_t>& stored)
                    {
                        if (counter > stored.first)
                            stored = { counter, date };
                    };

                    if (criteria->characterScope)
                        merge(progress[criteria->record.id]);
                    if (criteria->accountScope)
                        merge(account.criteria[criteria->record.id]);
                    ++mapped;
                }
                while (rows->nextRow());
            }

            CharacterDatabase.waitExecute("DELETE FROM character_achievement_progress WHERE guid = %u", guid);
            if (!progress.empty())
            {
                std::ostringstream query;
                query << "INSERT INTO character_achievement_progress VALUES ";
                bool first = true;
                for (auto const& [criteriaId, value] : progress)
                {
                    query << (first ? "" : ", ") << "(" << guid << ", " << criteriaId << ", " << value.first << ", " << value.second << ")";
                    first = false;
                }
                CharacterDatabase.waitExecuteNA(query.str().c_str());
            }

            ++converted;
        }

        if (oldCriteria.empty())
            sLogger.info("AchievementConverter : neither the client data nor achievement_criteria_legacy name the old criteria, their progress can not be mapped");
        oldCriteria.clear();

        sLogger.info("AchievementConverter : {} characters converted, {} progress rows mapped, {} dropped, {} account wide achievements for {} accounts",
            converted, mapped, dropped, accountAchievements, pendingAccounts.size());
    }

    void sendPendingConversions()
    {
        LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket();
        if (socket == nullptr)
            return;

        std::lock_guard lock(pendingLock);
        for (auto const& [accountId, account] : pendingAccounts)
        {
            if (account.achievements.empty() && account.criteria.empty())
                continue;

            std::vector<LogonComm::AccountAchievementRecord> achievements;
            for (auto const& [achievementId, date] : account.achievements)
                achievements.push_back({ achievementId, date });

            std::vector<LogonComm::AccountCriteriaRecord> criteria;
            for (auto const& [criteriaId, value] : account.criteria)
                criteria.push_back({ criteriaId, value.first, value.second });

            WorldPacket data(LRCMSG_ACHIEVEMENT_SAVE, 16 + achievements.size() * 12 + criteria.size() * 16);
            data << accountId;
            LogonComm::writeAccountAchievements(data, achievements, criteria);
            socket->SendPacket(&data, false);
        }

        if (!pendingAccounts.empty())
            sLogger.info("AchievementConverter : the account wide achievements of {} accounts went to the logonserver", pendingAccounts.size());
        pendingAccounts.clear();
    }
}
