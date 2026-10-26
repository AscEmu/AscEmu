/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementMgr.h"

#include "Achievements/AccountAchievements.hpp"
#include "Achievements/AchievementConditions.hpp"
#include "Achievements/AchievementEvents.hpp"
#include "Achievements/AchievementTrees.hpp"
#include "Logging/Log.hpp"
#include "Logging/Logger.hpp"
#include "Management/ObjectMgr.hpp"
#include "Management/WorldStatesHandler.hpp"
#include "Map/Maps/WorldMap.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/Packets/SmsgAccountCriteriaUpdate.h"
#include "Server/Packets/SmsgAchievementDeleted.h"
#include "Server/Packets/SmsgAllAccountCriteria.h"
#include "Server/Packets/SmsgAllAchievementData.h"
#include "Server/Packets/SmsgAllAchievementDataLegacy.h"
#include "Server/Packets/SmsgCriteriaDeleted.h"
#include "Server/Packets/SmsgCriteriaUpdate.h"
#include "Server/Packets/SmsgRespondInspectAchievements.h"
#include "Server/Script/ScriptMgr.hpp"
#include "Server/World.h"
#include "Server/WorldConfig.h"
#include "Server/WorldSession.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>

using namespace AscEmu::Packets;
using namespace Achievements;

namespace
{
    // the faction of an achievement in the client data, -1 for both
    constexpr int32_t AchievementFactionHorde = 0;
    constexpr int32_t AchievementFactionAlliance = 1;

    constexpr uint16_t TypeWinArena = 32;
    constexpr uint16_t TypeSkillStep = 40;
    constexpr uint16_t TypeLogin = 74;

    constexpr uint16_t TypeLootAnyEpicItem = 90;
    constexpr uint16_t TypeAchievementPoints = 115;

    // the points the achievement of all points asks for, the criteria rows carry no amount
    constexpr uint64_t AchievementPointsGoal = 9000;

    // the criteria rows before 5.4: types any progress completes
    std::unordered_set<uint16_t> const rowFlagTypes = { 8, 27, 34, 43, 133 };

    // the criteria rows before 5.4: types that complete with their amount, every other type only counts
    std::unordered_set<uint16_t> const rowAmountTypes =
    {
        0, 1, 3, 5, 7, 9, 10, 11, 12, 13, 14, 24, 28, 29, 30, 31, 35, 36, 37, 39, 41, 42, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
        55, 56, 57, 62, 67, 68, 69, 70, 72, 75, 76, 109, 110, 112, 113, 119, 124, 125, 126, 127, 128, 129, 134, 135, 136, 138, 139
    };

    // the criteria trees from 5.4 on: types any progress completes
    std::unordered_set<uint16_t> const treeFlagTypes = { 8, 27, 34, 43, 48, 49, 57, 153, 154, 155, 163, 164 };

    // the criteria trees from 5.4 on: types of statistics, they count but complete nothing
    std::unordered_set<uint16_t> const treeStatisticTypes =
    {
        15, 16, 17, 18, 20, 23, 26, 33, 38, 59, 60, 61, 65, 66, 77, 78, 80, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 93, 94, 107, 108, 114
    };

    bool isAccountAchievement(WDB::Structures::AchievementEntry const* achievement)
    {
        return (achievement->flags & AchievementFlagAccount) != 0;
    }

    void runQuery(QueryBuffer* buffer, std::ostringstream const& query)
    {
        if (buffer == nullptr)
            CharacterDatabase.executeNA(query.str().c_str());
        else
            buffer->addQueryNA(query.str().c_str());
    }

    // the criteria of an achievement the client lists, hidden achievements stay out
    bool isVisibleCriteria(uint32_t criteriaId)
    {
        for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(criteriaId))
            if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                if ((achievement->flags & ACHIEVEMENT_FLAG_HIDDEN) == 0)
                    return true;
        return false;
    }
}

Achievements::AccountAchievements* AchievementMgr::getAccount() const
{
    WorldSession* session = m_player->getSession();
    return session != nullptr ? &session->getAccountAchievements() : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// storage

void AchievementMgr::moveAccountData()
{
    AccountAchievements* account = getAccount();
    if (account == nullptr)
        return;

    for (auto itr = m_completedAchievements.begin(); itr != m_completedAchievements.end();)
    {
        auto const* achievement = sAchievementStore.lookupEntry(itr->first);
        if (achievement != nullptr && isAccountAchievement(achievement))
        {
            account->complete(itr->first, itr->second);
            itr = m_completedAchievements.erase(itr);
        }
        else
            ++itr;
    }

    for (auto itr = m_criteriaProgress.begin(); itr != m_criteriaProgress.end();)
    {
        Criteria const* criteria = sAchievementTrees.getCriteria(itr->first);
        if (criteria != nullptr && criteria->accountScope)
        {
            AccountProgress stored;
            if (!account->getProgress(itr->first, stored) || itr->second->counter > stored.counter)
                account->setProgress(itr->first, itr->second->counter, itr->second->date);
        }

        // a criteria only account wide achievements count leaves the character
        if (criteria != nullptr && criteria->accountScope && !criteria->characterScope)
            itr = m_criteriaProgress.erase(itr);
        else
            ++itr;
    }
}

void AchievementMgr::saveToDb(QueryBuffer* _buffer)
{
    std::lock_guard lock(m_lock);
    uint32_t const guid = m_player->getGuidLow();

    std::ostringstream query;
    query << "DELETE FROM character_achievement WHERE guid = " << guid << ";";
    runQuery(_buffer, query);

    query.str("");
    bool first = true;
    for (auto const& [achievementId, date] : m_completedAchievements)
    {
        if (query.str().length() >= 16000)
        {
            runQuery(_buffer, query);
            query.str("");
            first = true;
        }

        query << (first ? "INSERT INTO character_achievement VALUES " : ", ") << "(" << guid << ", " << achievementId << ", " << date << ")";
        first = false;
    }
    if (!first)
        runQuery(_buffer, query);

    query.str("");
    query << "DELETE FROM character_achievement_progress WHERE guid = " << guid << ";";
    runQuery(_buffer, query);

    // the derived criteria follow the player, the counted ones are stored
    query.str("");
    first = true;
    for (auto const& [criteriaId, progress] : m_criteriaProgress)
    {
        Criteria const* criteria = sAchievementTrees.getCriteria(criteriaId);
        if (criteria == nullptr || progress->counter == 0 || isDerivedType(criteria->record.type) || criteria->record.startEvent != 0)
            continue;

        if (query.str().length() >= 16000)
        {
            runQuery(_buffer, query);
            query.str("");
            first = true;
        }

        query << (first ? "INSERT INTO character_achievement_progress VALUES " : ", ")
            << "(" << guid << ", " << criteriaId << ", " << progress->counter << ", " << progress->date << ")";
        first = false;
    }
    if (!first)
        runQuery(_buffer, query);

    if (AccountAchievements* account = getAccount())
        account->save();
}

//////////////////////////////////////////////////////////////////////////////////////////
// progress

bool AchievementMgr::canEarn(WDB::Structures::AchievementEntry const* _achievement) const
{
    if (_achievement->factionFlag == AchievementFactionHorde && !m_player->isTeamHorde())
        return false;
    if (_achievement->factionFlag == AchievementFactionAlliance && !m_player->isTeamAlliance())
        return false;

    // an achievement of one map counts there only
    if (_achievement->mapID != -1 && m_player->GetMapId() != static_cast<uint32_t>(_achievement->mapID))
        return false;

    if (_achievement->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL))
        if (sObjectMgr.isInCompletedAchievements(_achievement->ID))
            return false;

    return true;
}

bool AchievementMgr::isOpenCriteria(Achievements::Criteria const* _criteria) const
{
    for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(_criteria->record.id))
    {
        auto const* achievement = sAchievementStore.lookupEntry(achievementId);
        if (achievement == nullptr || !canEarn(achievement))
            continue;

        // statistics keep counting
        if ((achievement->flags & ACHIEVEMENT_FLAG_COUNTER) || !hasCompleted(achievementId))
            return true;
    }
    return false;
}

uint32_t AchievementMgr::getProgress(Achievements::Criteria const* _criteria, bool _accountWide) const
{
    if (_accountWide)
    {
        AccountProgress progress;
        AccountAchievements* account = getAccount();
        return account != nullptr && account->getProgress(_criteria->record.id, progress) ? progress.counter : 0;
    }

    auto const itr = m_criteriaProgress.find(_criteria->record.id);
    return itr != m_criteriaProgress.end() ? itr->second->counter : 0;
}

void AchievementMgr::storeProgress(Achievements::Criteria const* _criteria, uint32_t _counter, bool _accountWide)
{
    uint32_t const criteriaId = _criteria->record.id;
    time_t const now = UNIXTIME;

    if (_accountWide)
    {
        if (AccountAchievements* account = getAccount())
            account->setProgress(criteriaId, _counter, now, _criteria->record.startEvent == 0);
    }
    else
    {
        auto& progress = m_criteriaProgress[criteriaId];
        if (progress == nullptr)
            progress = std::make_unique<CriteriaProgress>(criteriaId, _counter, now);
        else
        {
            progress->counter = _counter;
            progress->date = now;
        }
    }

    if (isVisibleCriteria(criteriaId))
        sendCriteriaUpdate(criteriaId, _counter, now, _accountWide);
}

void AchievementMgr::applyProgress(Achievements::Criteria const* _criteria, Achievements::ProgressUpdate const& _update)
{
    for (bool const accountWide : { false, true })
    {
        if ((accountWide && !_criteria->accountScope) || (!accountWide && !_criteria->characterScope))
            continue;

        // the account part waits for the data of the logonserver
        if (accountWide && (getAccount() == nullptr || !getAccount()->isLoaded()))
            continue;

        uint64_t const current = getProgress(_criteria, accountWide);
        uint64_t next = current;
        switch (_update.change)
        {
            case ProgressChange::Add:
                next = current + _update.value;
                break;
            case ProgressChange::Set:
                next = _update.value;
                break;
            case ProgressChange::Highest:
                next = std::max(current, _update.value);
                break;
        }

        next = std::min<uint64_t>(next, UINT32_MAX);
        if (next != current)
            storeProgress(_criteria, static_cast<uint32_t>(next), accountWide);
    }

    checkAchievementsOf(_criteria);
}

void AchievementMgr::updateAchievementCriteria(AchievementCriteriaTypes _type, int32_t _miscvalue1, int32_t _miscvalue2, uint32_t /*_time*/, Object* _reference)
{
    if (m_player->getSession() == nullptr || (m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements))
        return;

    std::lock_guard lock(m_lock);
    for (uint16_t const criteriaType : getCriteriaTypesOfEvent(_type))
    {
        for (Criteria const* criteria : sAchievementTrees.getCriteriaOfType(criteriaType))
        {
            // a criteria with a start event counts while its timer runs, one with an unsent fail event can not be reset
            if (criteria->record.startEvent != 0 && !m_criteriaTimers.contains(criteria->record.id))
                continue;
            if (criteria->record.failEvent != 0 && !isFailEventHooked(criteria->record.failEvent))
                continue;

            if (!isOpenCriteria(criteria))
                continue;

            ProgressUpdate update;
            if (!resolveEvent(m_player, *criteria, _miscvalue1, _miscvalue2, _reference, update))
                continue;

            if (!areChecksMet(criteria, update.subject))
                continue;

            if (!sScriptMgr.callScriptedAchievementCriteriaCanComplete(criteria->record.id, m_player, _reference))
                continue;

            applyProgress(criteria, update);
        }
    }
}

void AchievementMgr::updateAchievementCriteria(AchievementCriteriaTypes _type)
{
    updateAchievementCriteria(_type, 0, 0, 0, nullptr);
}

bool AchievementMgr::areChecksMet(Achievements::Criteria const* _criteria, Achievements::EventSubject const& _subject) const
{
    if (_criteria->record.conditionTreeId != 0 && !isConditionTreeMet(_criteria->conditions, m_player, _subject))
        return false;

    if (!_criteria->requirements.empty() && !areRequirementsMet(*_criteria, m_player, _subject))
        return false;

    if (_criteria->record.worldStateId != 0)
    {
        WorldMap* map = m_player->getWorldMap();
        if (map == nullptr || static_cast<int32_t>(map->getWorldStatesHandler().GetWorldStateForZone(m_player->getZoneId(), m_player->getAreaId(), _criteria->record.worldStateId)) != _criteria->record.worldStateValue)
            return false;
    }

    return true;
}

void AchievementMgr::deriveAll()
{
    for (uint16_t type = 0; type < 512; ++type)
    {
        if (!isDerivedType(type))
            continue;

        for (Criteria const* criteria : sAchievementTrees.getCriteriaOfType(type))
        {
            if (!isOpenCriteria(criteria))
                continue;

            ProgressUpdate update;
            if (!deriveProgress(m_player, *criteria, update.value))
                continue;

            update.change = ProgressChange::Set;
            if (!areChecksMet(criteria, update.subject))
                continue;

            applyProgress(criteria, update);
        }
    }
}

void AchievementMgr::updateAllAchievementCriteria()
{
    if (m_player->getSession() != nullptr && m_player->getSession()->HasGMPermissions() && worldConfig.gm.disableAchievements)
        return;

    std::lock_guard lock(m_lock);
    deriveAll();
}

bool AchievementMgr::updateAchievementCriteria(Player* /*_player*/, int32_t _criteriaId, uint32_t _count)
{
    Criteria const* criteria = sAchievementTrees.getCriteria(static_cast<uint32_t>(_criteriaId));
    if (criteria == nullptr)
    {
        sLogger.debug("AchievementMgr: criteria {} is unknown to the client data", _criteriaId);
        return false;
    }

    std::lock_guard lock(m_lock);
    if (!isOpenCriteria(criteria))
        return false;

    ProgressUpdate update;
    update.change = ProgressChange::Add;
    update.value = _count;
    applyProgress(criteria, update);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// timers

void AchievementMgr::removeProgress(Achievements::Criteria const* _criteria)
{
    uint32_t const criteriaId = _criteria->record.id;
    if (m_criteriaProgress.erase(criteriaId) != 0 && m_player->getSession() != nullptr)
    {
        SmsgCriteriaDeleted packet(criteriaId);
        m_player->getSession()->sendManagedPacket(packet);
    }

    // timed criteria of the account keep no progress either
    if (_criteria->accountScope)
        if (AccountAchievements* account = getAccount())
            account->setProgress(criteriaId, 0, UNIXTIME, false);
}

void AchievementMgr::startCriteria(uint8_t _startEvent, uint32_t _asset)
{
    std::lock_guard lock(m_lock);
    for (Criteria const* criteria : sAchievementTrees.getCriteriaOfStartEvent(_startEvent, _asset))
    {
        if (!isOpenCriteria(criteria))
            continue;

        // a start event without a time limit opens the criteria until a fail event
        int64_t const limit = criteria->record.timeLimit != 0 ? static_cast<int64_t>(criteria->record.timeLimit) * 1000 : INT64_MAX;
        auto [itr, started] = m_criteriaTimers.try_emplace(criteria->record.id, limit);
        if (!started)
        {
            if ((criteria->record.flags & CriteriaFlagResetOnStart) == 0)
                continue;
            itr->second = limit;
        }

        removeProgress(criteria);
    }
}

void AchievementMgr::failCriteria(uint8_t _failEvent, uint32_t _asset)
{
    std::lock_guard lock(m_lock);
    for (Criteria const* criteria : sAchievementTrees.getCriteriaOfFailEvent(_failEvent, _asset))
    {
        m_criteriaTimers.erase(criteria->record.id);

        // a criteria of achievements that are all done keeps its progress
        bool open = false;
        for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(criteria->record.id))
            if (!hasCompleted(achievementId))
                open = true;

        if (open)
            removeProgress(criteria);
    }
}

void AchievementMgr::updateCriteriaTimers(uint32_t _diffMs)
{
    if (m_criteriaTimers.empty())
        return;

    std::lock_guard lock(m_lock);
    for (auto itr = m_criteriaTimers.begin(); itr != m_criteriaTimers.end();)
    {
        if (itr->second == INT64_MAX)
        {
            ++itr;
            continue;
        }

        if (itr->second <= static_cast<int64_t>(_diffMs))
        {
            // the time ran out, the criteria starts over with the next start event
            if (Criteria const* criteria = sAchievementTrees.getCriteria(itr->first))
                removeProgress(criteria);
            itr = m_criteriaTimers.erase(itr);
        }
        else
        {
            itr->second -= _diffMs;
            ++itr;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// completion

bool AchievementMgr::isCriteriaComplete(Achievements::Criteria const* _criteria, uint64_t _amount, bool _accountWide) const
{
    uint16_t const type = _criteria->record.type;
    uint64_t const counter = getProgress(_criteria, _accountWide);
    if (counter == 0)
        return false;

    bool const fromRows = sAchievementTrees.isBuiltFromCriteriaRows();
    if (type == TypeLogin)
        return true;
    if ((fromRows ? rowFlagTypes : treeFlagTypes).contains(type))
        return counter >= 1;
    if (type == TypeSkillStep)
        return counter >= _amount * 75;
    if (type == TypeWinArena)
        return _amount != 0 && counter >= _amount;

    if (fromRows)
    {
        if (type == TypeLootAnyEpicItem)
            return _amount != 0 && counter >= _amount;
        if (type == TypeAchievementPoints)
            return counter >= AchievementPointsGoal;

        return rowAmountTypes.contains(type) && counter >= _amount;
    }

    if (treeStatisticTypes.contains(type))
        return false;

    return counter >= _amount;
}

uint64_t AchievementMgr::sumOfTree(Achievements::ProgressNode const* _node, bool _accountWide, bool _weighted) const
{
    uint64_t sum = 0;
    if (_node->criteria != nullptr)
        sum += getProgress(_node->criteria, _accountWide) * (_weighted ? _node->record.amount : 1);

    for (ProgressNode const* child : _node->children)
        sum += sumOfTree(child, _accountWide, _weighted);
    return sum;
}

uint64_t AchievementMgr::highestOfTree(Achievements::ProgressNode const* _node, bool _accountWide) const
{
    uint64_t highest = _node->criteria != nullptr ? getProgress(_node->criteria, _accountWide) : 0;
    for (ProgressNode const* child : _node->children)
        highest = std::max(highest, highestOfTree(child, _accountWide));
    return highest;
}

bool AchievementMgr::isTreeComplete(Achievements::ProgressNode const* _node, bool _accountWide) const
{
    if (_node == nullptr)
        return false;

    uint64_t const amount = _node->record.amount;
    switch (static_cast<ProgressRule>(_node->record.rule))
    {
        case ProgressRule::Complete:
            return _node->criteria != nullptr && isCriteriaComplete(_node->criteria, amount, _accountWide);
        case ProgressRule::NotComplete:
            return _node->criteria == nullptr || !isCriteriaComplete(_node->criteria, amount, _accountWide);
        case ProgressRule::AllChildren:
            for (ProgressNode const* child : _node->children)
                if (!isTreeComplete(child, _accountWide))
                    return false;
            return true;
        case ProgressRule::SumOfChildren:
            return sumOfTree(_node, _accountWide, false) >= amount;
        case ProgressRule::HighestChild:
            return highestOfTree(_node, _accountWide) >= amount;
        case ProgressRule::StartedChildren:
        {
            uint64_t started = 0;
            for (ProgressNode const* child : _node->children)
                if (child->criteria != nullptr && getProgress(child->criteria, _accountWide) >= 1 && ++started >= amount)
                    return true;
            return false;
        }
        case ProgressRule::CompletedChildren:
        {
            uint64_t completed = 0;
            for (ProgressNode const* child : _node->children)
                if (isTreeComplete(child, _accountWide) && ++completed >= amount)
                    return true;
            return false;
        }
        case ProgressRule::WeightedChildren:
            return sumOfTree(_node, _accountWide, true) >= amount;
        default:
            break;
    }

    return false;
}

void AchievementMgr::checkAchievementsOf(Achievements::Criteria const* _criteria)
{
    for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(_criteria->record.id))
        checkAchievement(achievementId);
}

void AchievementMgr::checkAchievement(uint32_t _achievementId)
{
    auto const* achievement = sAchievementStore.lookupEntry(_achievementId);
    if (achievement == nullptr || (achievement->flags & ACHIEVEMENT_FLAG_COUNTER) || hasCompleted(_achievementId) || !canEarn(achievement))
        return;

    bool const accountWide = isAccountAchievement(achievement);
    if (accountWide && (getAccount() == nullptr || !getAccount()->isLoaded()))
        return;

    if (isTreeComplete(sAchievementTrees.getAchievementTree(_achievementId), accountWide))
        completedAchievement(achievement);
}

void AchievementMgr::completedAchievement(WDB::Structures::AchievementEntry const* _achievement)
{
    if ((_achievement->flags & ACHIEVEMENT_FLAG_COUNTER) || hasCompleted(_achievement->ID))
        return;

    if (showCompletedAchievement(_achievement->ID, m_player))
        sendAchievementEarned(_achievement);

    if (isAccountAchievement(_achievement))
    {
        if (AccountAchievements* account = getAccount())
            account->complete(_achievement->ID, UNIXTIME);
    }
    else
    {
        m_completedAchievements[_achievement->ID] = UNIXTIME;
    }

    sObjectMgr.addCompletedAchievement(_achievement->ID);
    giveAchievementReward(_achievement);

    // achievements counting this one, then those sharing its tree
    for (uint16_t const type : getCriteriaTypesOfEvent(ACHIEVEMENT_CRITERIA_TYPE_COMPLETE_ACHIEVEMENT))
    {
        for (Criteria const* criteria : sAchievementTrees.getCriteriaOfType(type))
        {
            if (!isOpenCriteria(criteria))
                continue;

            ProgressUpdate update;
            if (deriveProgress(m_player, *criteria, update.value) && areChecksMet(criteria, update.subject))
            {
                update.change = ProgressChange::Set;
                applyProgress(criteria, update);
            }
        }
    }

    for (uint32_t const sharingId : sAchievementTrees.getSharingAchievements(_achievement->ID))
        checkAchievement(sharingId);
}

//////////////////////////////////////////////////////////////////////////////////////////
// queries

bool AchievementMgr::hasCompletedOnCharacter(uint32_t _achievementId) const
{
    return m_completedAchievements.contains(_achievementId);
}

bool AchievementMgr::hasCompletedOnAccount(uint32_t _achievementId) const
{
    AccountAchievements* account = getAccount();
    return account != nullptr && account->hasCompleted(_achievementId);
}

bool AchievementMgr::hasCompleted(uint32_t _achievementId) const
{
    return hasCompletedOnCharacter(_achievementId) || hasCompletedOnAccount(_achievementId);
}

time_t AchievementMgr::getCompletedTime(WDB::Structures::AchievementEntry const* _achievement)
{
    if (auto const itr = m_completedAchievements.find(_achievement->ID); itr != m_completedAchievements.end())
        return itr->second;

    AccountAchievements* account = getAccount();
    return account != nullptr ? account->getCompletedTime(_achievement->ID) : 0;
}

std::vector<uint32_t> AchievementMgr::getCompletedAchievementIds() const
{
    std::vector<uint32_t> ids;
    ids.reserve(m_completedAchievements.size());
    for (auto const& [achievementId, date] : m_completedAchievements)
        ids.push_back(achievementId);

    if (AccountAchievements* account = getAccount())
        for (auto const& [achievementId, date] : account->getCompleted())
            if (!m_completedAchievements.contains(achievementId))
                ids.push_back(achievementId);
    return ids;
}

uint32_t AchievementMgr::getCompletedAchievementsCount() const
{
    return static_cast<uint32_t>(getCompletedAchievementIds().size());
}

uint32_t AchievementMgr::getAchievementPoints() const
{
    uint32_t points = 0;
    for (uint32_t const achievementId : getCompletedAchievementIds())
        if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
            points += achievement->points;
    return points;
}

uint32_t AchievementMgr::getCriteriaProgressCount()
{
    uint32_t count = 0;
    for (auto const& [criteriaId, progress] : m_criteriaProgress)
        if (progress->counter > 0 && isVisibleCriteria(criteriaId))
            ++count;
    return count;
}

//////////////////////////////////////////////////////////////////////////////////////////
// packets

void AchievementMgr::sendCriteriaUpdate(uint32_t _criteriaId, uint32_t _counter, time_t _date, bool _accountWide)
{
    if (isCharacterLoading || m_player->getSession() == nullptr)
        return;

    // clients before 9.x show the account progress as their own
    if (_accountWide && m_player->getSession()->getClientProtocol().expansion >= WoW::Expansion::_Shadowlands)
    {
        SmsgAccountCriteriaUpdate packet({ _criteriaId, _counter, _date }, m_player->getSession()->GetAccountId());
        m_player->getSession()->sendManagedPacket(packet);
        return;
    }

    SmsgCriteriaUpdate packet(_criteriaId, _counter, m_player->GetNewGUID(), secsToTimeBitFields(_date), _date);
    m_player->getSession()->sendManagedPacket(packet);
}

void AchievementMgr::sendAllAchievementDataInChunks(Player* _player)
{
    bool const isSelf = _player == m_player;

    AchievementDataChunkInput input;
    input.isSelf = isSelf;
    input.achievingPlayerGuid = m_player->getGuid();
    {
        std::lock_guard lock(m_lock);

        for (auto const& [achievementId, date] : m_completedAchievements)
            if (showCompletedAchievement(achievementId, m_player))
                input.completed.push_back({ achievementId, secsToTimeBitFields(date) });

        for (auto const& [criteriaId, criteriaProgress] : m_criteriaProgress)
        {
            if (criteriaProgress->counter == 0 || !isVisibleCriteria(criteriaId))
                continue;

            // others see the statistics only, the client shows the rest as done or not done
            bool statistic = false;
            for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(criteriaId))
                if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                    statistic = statistic || (achievement->flags & ACHIEVEMENT_FLAG_COUNTER) != 0;

            if (isSelf || statistic)
                input.progress.push_back({ criteriaId, criteriaProgress->counter, secsToTimeBitFields(criteriaProgress->date) });
        }
    }

    SmsgAllAchievementDataLegacy packetBuilder(std::move(input));
    for (auto& chunk : packetBuilder.buildChunks())
        _player->getSession()->SendPacket(chunk.get());

    if (isSelf)
        isCharacterLoading = false;
}
void AchievementMgr::sendAllAchievementData(Player* _player)
{
    // the clients up to 3.x take the data in chunks of their own layout
    if (_player->getSession()->getClientProtocol().expansion <= WoW::Expansion::_WotLK)
    {
        sendAllAchievementDataInChunks(_player);
        return;
    }

    std::vector<CompletedAchievementEntry> completed;
    std::vector<CriteriaProgressEntry> progress;
    std::vector<CriteriaProgressEntry> accountProgress;
    {
        std::lock_guard lock(m_lock);

        for (auto const& [achievementId, date] : m_completedAchievements)
            if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                if ((achievement->flags & ACHIEVEMENT_FLAG_HIDDEN) == 0)
                    completed.push_back({ achievementId, date, false });

        for (auto const& [criteriaId, criteriaProgress] : m_criteriaProgress)
            if (criteriaProgress->counter > 0 && isVisibleCriteria(criteriaId))
                progress.push_back({ criteriaId, criteriaProgress->counter, criteriaProgress->date });

        // the achievements of the account, only those this realm knows
        if (AccountAchievements* account = getAccount())
        {
            for (auto const& [achievementId, date] : account->getCompleted())
                if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                    if ((achievement->flags & ACHIEVEMENT_FLAG_HIDDEN) == 0 && !m_completedAchievements.contains(achievementId))
                        completed.push_back({ achievementId, date, true });

            for (auto const& [criteriaId, accountCriteria] : account->getAllProgress())
                if (accountCriteria.counter > 0 && sAchievementTrees.getCriteria(criteriaId) != nullptr && isVisibleCriteria(criteriaId))
                    accountProgress.push_back({ criteriaId, accountCriteria.counter, accountCriteria.date });
        }
    }

    // clients before 9.x list the account progress with their own
    if (_player->getSession()->getClientProtocol().expansion < WoW::Expansion::_Shadowlands)
    {
        for (auto const& entry : accountProgress)
            if (std::ranges::none_of(progress, [&entry](CriteriaProgressEntry const& own) { return own.criteriaId == entry.criteriaId; }))
                progress.push_back(entry);
    }
    else if (!accountProgress.empty() && _player == m_player)
    {
        SmsgAllAccountCriteria accountPacket(std::move(accountProgress), _player->getSession()->GetAccountId());
        _player->getSession()->sendManagedPacket(accountPacket);
    }

    SmsgAllAchievementData packet{ m_player->getGuid(), std::move(progress), std::move(completed) };
    _player->getSession()->sendManagedPacket(packet);

    if (_player == m_player)
    {
        isCharacterLoading = false;
        m_sentWithAccountData = getAccount() != nullptr && getAccount()->isLoaded();
    }
}

void AchievementMgr::sendRespondInspectAchievements(Player* _player)
{
    std::vector<CompletedAchievementEntry> completed;
    std::vector<CriteriaProgressEntry> progress;
    {
        std::lock_guard lock(m_lock);
        for (uint32_t const achievementId : getCompletedAchievementIds())
            if (auto const* achievement = sAchievementStore.lookupEntry(achievementId))
                if ((achievement->flags & ACHIEVEMENT_FLAG_HIDDEN) == 0)
                    completed.push_back({ achievementId, getCompletedTime(achievement), isAccountAchievement(achievement) });

        // other players see the statistics only
        for (auto const& [criteriaId, criteriaProgress] : m_criteriaProgress)
            if (criteriaProgress->counter > 0 && isVisibleCriteria(criteriaId))
                progress.push_back({ criteriaId, criteriaProgress->counter, criteriaProgress->date });
    }

    SmsgRespondInspectAchievements packet{ m_player->getGuid(), std::move(progress), std::move(completed) };
    _player->getSession()->sendManagedPacket(packet);
}

void AchievementMgr::onAccountDataLoaded()
{
    // the login already listed the achievements of the account
    if (m_sentWithAccountData)
        return;

    {
        std::lock_guard lock(m_lock);
        deriveAll();
    }

    sendAllAchievementData(m_player);
}

//////////////////////////////////////////////////////////////////////////////////////////
// game master

bool AchievementMgr::gmCompleteAchievement(WorldSession* _gmSession, uint32_t _achievementId, bool _finishAll)
{
    std::lock_guard lock(m_lock);
    if (_finishAll)
    {
        for (auto const& [achievementId, achievement] : sAchievementStore)
            if ((achievement.flags & ACHIEVEMENT_FLAG_COUNTER) == 0 && canEarn(&achievement))
                completedAchievement(&achievement);

        m_player->getSession()->systemMessage("All achievements completed.");
        return true;
    }

    if (hasCompleted(_achievementId))
    {
        _gmSession->systemMessage("Player has already completed that achievement.");
        return false;
    }

    auto const* achievement = sAchievementStore.lookupEntry(_achievementId);
    if (achievement == nullptr)
    {
        _gmSession->systemMessage("Achievement {} entry not found.", _achievementId);
        return false;
    }

    if (achievement->flags & ACHIEVEMENT_FLAG_COUNTER)
    {
        _gmSession->systemMessage("Achievement {} cannot be completed because it is a counter.", achievement->ID);
        return false;
    }

    completedAchievement(achievement);
    return true;
}

void AchievementMgr::gmResetAchievement(uint32_t _achievementId, bool _finishAll)
{
    std::lock_guard lock(m_lock);

    std::vector<uint32_t> achievements;
    if (_finishAll)
        achievements = getCompletedAchievementIds();
    else
        achievements.push_back(_achievementId);

    for (uint32_t const achievementId : achievements)
    {
        SmsgAchievementDeleted packet(achievementId);
        m_player->getSession()->sendManagedPacket(packet);

        m_completedAchievements.erase(achievementId);
        if (AccountAchievements* account = getAccount(); account != nullptr && account->hasCompleted(achievementId))
            account->reset(achievementId);
    }

    if (_finishAll)
        CharacterDatabase.execute("DELETE FROM character_achievement WHERE guid = %u", m_player->getGuidLow());
    else
        CharacterDatabase.execute("DELETE FROM character_achievement WHERE guid = %u AND achievement = %u", m_player->getGuidLow(), _achievementId);
}

bool AchievementMgr::gmCompleteCriteria(WorldSession* _gmSession, uint32_t _criteriaId, bool _finishAll)
{
    std::lock_guard lock(m_lock);
    if (_finishAll)
    {
        _gmSession->systemMessage("Completing every criteria is not supported, complete the achievements instead.");
        return false;
    }

    Criteria const* criteria = sAchievementTrees.getCriteria(_criteriaId);
    if (criteria == nullptr)
    {
        _gmSession->systemMessage("Achievement criteria {} not found.", _criteriaId);
        return false;
    }

    // the amount of the first node that counts the criteria
    uint64_t amount = 1;
    for (uint32_t const achievementId : sAchievementTrees.getAchievementsOfCriteria(_criteriaId))
    {
        ProgressNode const* tree = sAchievementTrees.getAchievementTree(achievementId);
        std::vector<ProgressNode const*> pending{ tree };
        while (!pending.empty() && amount == 1)
        {
            ProgressNode const* node = pending.back();
            pending.pop_back();
            if (node == nullptr)
                continue;
            if (node->criteria == criteria && node->record.amount > 1)
                amount = node->record.amount;
            for (ProgressNode const* child : node->children)
                pending.push_back(child);
        }
    }

    ProgressUpdate update;
    update.change = ProgressChange::Highest;
    update.value = criteria->record.type == TypeSkillStep ? amount * 75 : amount;
    applyProgress(criteria, update);
    return true;
}

void AchievementMgr::gmResetCriteria(uint32_t _criteriaId, bool _finishAll)
{
    {
        std::lock_guard lock(m_lock);
        AccountAchievements* account = getAccount();

        std::set<uint32_t> criteriaIds;
        if (_finishAll)
        {
            for (auto const& [criteriaId, progress] : m_criteriaProgress)
                criteriaIds.insert(criteriaId);
            if (account != nullptr)
                for (auto const& [criteriaId, progress] : account->getAllProgress())
                    criteriaIds.insert(criteriaId);
        }
        else
        {
            criteriaIds.insert(_criteriaId);
        }

        for (uint32_t const criteriaId : criteriaIds)
        {
            SmsgCriteriaDeleted packet(criteriaId);
            m_player->getSession()->sendManagedPacket(packet);
            m_criteriaProgress.erase(criteriaId);

            AccountProgress stored;
            if (account != nullptr && account->getProgress(criteriaId, stored))
                account->resetCriteria(criteriaId);
        }
    }

    if (_finishAll)
        CharacterDatabase.execute("DELETE FROM character_achievement_progress WHERE guid = %u", m_player->getGuidLow());
    else
        CharacterDatabase.execute("DELETE FROM character_achievement_progress WHERE guid = %u AND criteria = %u", m_player->getGuidLow(), _criteriaId);

    updateAllAchievementCriteria();
}
