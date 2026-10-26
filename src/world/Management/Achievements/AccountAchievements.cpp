/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AccountAchievements.hpp"

#include "LogonCommDefines.hpp"
#include "Logging/Logger.hpp"
#include "Management/AchievementMgr.h"
#include "Network/WorldPacket.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/LogonCommClient/LogonCommHandler.h"
#include "Server/WorldSession.h"

namespace Achievements
{
    AccountAchievements::AccountAchievements(WorldSession& session) : m_session(session)
    {
    }

    void AccountAchievements::requestLoad()
    {
        if (LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket())
        {
            WorldPacket data(LRCMSG_ACHIEVEMENT_REQUEST, 4);
            data << m_session.GetAccountId();
            socket->SendPacket(&data, false);
        }
    }

    void AccountAchievements::onLoaded(std::vector<LogonComm::AccountAchievementRecord> achievements, std::vector<LogonComm::AccountCriteriaRecord> criteria)
    {
        std::lock_guard lock(m_mutex);

        // a change of this session before the answer stays
        for (auto const& achievement : achievements)
            m_completed.try_emplace(achievement.achievementId, static_cast<time_t>(achievement.date));

        for (auto const& progress : criteria)
        {
            auto& stored = m_progress[progress.criteriaId];
            if (progress.counter > stored.counter)
                stored = { progress.counter, static_cast<time_t>(progress.date) };
        }

        m_loaded = true;
    }

    bool AccountAchievements::isLoaded() const
    {
        std::lock_guard lock(m_mutex);
        return m_loaded;
    }

    void AccountAchievements::update()
    {
        {
            std::lock_guard lock(m_mutex);
            if (!m_loaded || m_announced)
                return;
        }

        Player* player = m_session.GetPlayer();
        if (player == nullptr || !player->IsInWorld())
            return;

        {
            std::lock_guard lock(m_mutex);
            m_announced = true;
        }
        player->getAchievementMgr()->onAccountDataLoaded();
    }

    bool AccountAchievements::hasCompleted(uint32_t achievementId) const
    {
        std::lock_guard lock(m_mutex);
        return m_completed.contains(achievementId);
    }

    time_t AccountAchievements::getCompletedTime(uint32_t achievementId) const
    {
        std::lock_guard lock(m_mutex);
        auto const itr = m_completed.find(achievementId);
        return itr != m_completed.end() ? itr->second : 0;
    }

    std::unordered_map<uint32_t, time_t> AccountAchievements::getCompleted() const
    {
        std::lock_guard lock(m_mutex);
        return m_completed;
    }

    void AccountAchievements::complete(uint32_t achievementId, time_t date)
    {
        std::lock_guard lock(m_mutex);
        if (m_completed.try_emplace(achievementId, date).second)
            m_changedAchievements.insert(achievementId);
    }

    void AccountAchievements::reset(uint32_t achievementId)
    {
        std::lock_guard lock(m_mutex);
        m_completed.erase(achievementId);
        m_changedAchievements.erase(achievementId);

        if (LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket())
        {
            WorldPacket data(LRCMSG_ACHIEVEMENT_RESET, 12);
            data << m_session.GetAccountId() << achievementId << uint32_t(0);
            socket->SendPacket(&data, false);
        }
    }

    void AccountAchievements::resetCriteria(uint32_t criteriaId)
    {
        std::lock_guard lock(m_mutex);
        m_progress.erase(criteriaId);
        m_changedCriteria.erase(criteriaId);

        if (LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket())
        {
            WorldPacket data(LRCMSG_ACHIEVEMENT_RESET, 12);
            data << m_session.GetAccountId() << uint32_t(0) << criteriaId;
            socket->SendPacket(&data, false);
        }
    }

    bool AccountAchievements::getProgress(uint32_t criteriaId, AccountProgress& progress) const
    {
        std::lock_guard lock(m_mutex);
        auto const itr = m_progress.find(criteriaId);
        if (itr == m_progress.end())
            return false;

        progress = itr->second;
        return true;
    }

    std::unordered_map<uint32_t, AccountProgress> AccountAchievements::getAllProgress() const
    {
        std::lock_guard lock(m_mutex);
        return m_progress;
    }

    void AccountAchievements::setProgress(uint32_t criteriaId, uint32_t counter, time_t date, bool persist)
    {
        std::lock_guard lock(m_mutex);
        auto& stored = m_progress[criteriaId];
        if (stored.counter == counter)
            return;

        stored = { counter, date };
        if (persist)
            m_changedCriteria.insert(criteriaId);
    }

    void AccountAchievements::save()
    {
        std::vector<LogonComm::AccountAchievementRecord> achievements;
        std::vector<LogonComm::AccountCriteriaRecord> criteria;
        {
            std::lock_guard lock(m_mutex);
            if (m_changedAchievements.empty() && m_changedCriteria.empty())
                return;

            for (uint32_t const achievementId : m_changedAchievements)
                if (auto const itr = m_completed.find(achievementId); itr != m_completed.end())
                    achievements.push_back({ achievementId, static_cast<uint64_t>(itr->second) });

            for (uint32_t const criteriaId : m_changedCriteria)
                if (auto const itr = m_progress.find(criteriaId); itr != m_progress.end())
                    criteria.push_back({ criteriaId, itr->second.counter, static_cast<uint64_t>(itr->second.date) });

            m_changedAchievements.clear();
            m_changedCriteria.clear();
        }

        LogonCommClientSocket* socket = sLogonCommHandler.getLogonServerSocket();
        if (socket == nullptr)
        {
            sLogger.failure("AccountAchievements::save: no logonserver, the achievements of account {} are not saved", m_session.GetAccountId());
            return;
        }

        WorldPacket data(LRCMSG_ACHIEVEMENT_SAVE, 16 + achievements.size() * 12 + criteria.size() * 16);
        data << m_session.GetAccountId();
        LogonComm::writeAccountAchievements(data, achievements, criteria);
        socket->SendPacket(&data, false);
    }
}
