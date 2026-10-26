/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "LogonCommAchievements.hpp"

#include <cstdint>
#include <ctime>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

class WorldSession;

namespace Achievements
{
    struct AccountProgress
    {
        uint32_t counter = 0;
        time_t date = 0;
    };

    // the account wide achievements and their criteria, kept by the logonserver for every realm of the account
    class AccountAchievements
    {
    public:
        explicit AccountAchievements(WorldSession& session);

        void requestLoad();
        void onLoaded(std::vector<LogonComm::AccountAchievementRecord> achievements, std::vector<LogonComm::AccountCriteriaRecord> criteria);
        bool isLoaded() const;

        // the session update hands the loaded data to the player once
        void update();

        bool hasCompleted(uint32_t achievementId) const;
        time_t getCompletedTime(uint32_t achievementId) const;
        std::unordered_map<uint32_t, time_t> getCompleted() const;
        void complete(uint32_t achievementId, time_t date);
        void reset(uint32_t achievementId);
        void resetCriteria(uint32_t criteriaId);

        bool getProgress(uint32_t criteriaId, AccountProgress& progress) const;
        std::unordered_map<uint32_t, AccountProgress> getAllProgress() const;
        // timed criteria change the progress of the session only
        void setProgress(uint32_t criteriaId, uint32_t counter, time_t date, bool persist = true);

        // writes what changed since the last save to the logonserver
        void save();

    private:
        WorldSession& m_session;

        mutable std::mutex m_mutex;
        bool m_loaded = false;
        bool m_announced = false;
        std::unordered_map<uint32_t, time_t> m_completed;
        std::unordered_map<uint32_t, AccountProgress> m_progress;
        std::set<uint32_t> m_changedAchievements;
        std::set<uint32_t> m_changedCriteria;
    };
}
