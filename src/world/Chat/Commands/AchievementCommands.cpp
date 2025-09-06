/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Management/AchievementMgr.h"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSessionLog.hpp"
#include "Utilities/Strings.hpp"


#if VERSION_STRING > TBC
//.achieve complete
bool ChatCommandHandler::handleAchievementCompleteCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (AscEmu::Util::Strings::isEqual(args, "all"))
    {
        selected_player->getAchievementMgr()->gmCompleteAchievement(m_session, 0, true);
        systemMessage(m_session, "All achievements have now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed all achievements for player %s", selected_player->getName().c_str());
        return true;
    }

    uint32_t achievement_id = std::stoul(args);
    if (achievement_id == 0)
        return false;

    if (selected_player->getAchievementMgr()->gmCompleteAchievement(m_session, achievement_id))
    {
        systemMessage(m_session, "The achievement has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement %u for player %s", achievement_id, selected_player->getName().c_str());
    }

    return true;
}

//.achieve criteria
bool ChatCommandHandler::handleAchievementCriteriaCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32_t criteria_id = std::stoul(args);
    if (criteria_id == 0)
    {
        if (AscEmu::Util::Strings::isEqual(args, "all"))
        {
            selected_player->getAchievementMgr()->gmCompleteCriteria(m_session, 0, true);
            systemMessage(m_session, "All achievement criteria have now been completed for that player.");
            sGMLog.writefromsession(m_session, "completed all achievement criteria for player %s", selected_player->getName().c_str());
            return true;
        }
        return false;
    }
    
    if (selected_player->getAchievementMgr()->gmCompleteCriteria(m_session, criteria_id))
    {
        systemMessage(m_session, "The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement criteria %u for player %s", criteria_id, selected_player->getName().c_str());
    }

    return true;
}

//.achieve reset
bool ChatCommandHandler::handleAchievementResetCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    bool reset_achievement = true;
    bool reset_criteria = false;
    bool resetAll = false;
    uint32_t achievement_id = 0;

    std::string criteria(args, 9);

    if (AscEmu::Util::Strings::isEqual(criteria, "criteria "))
    {
        achievement_id = std::stoul(args + 9);
        if (achievement_id == 0)
        {
            if (!AscEmu::Util::Strings::isEqual(args + 9, "all"))
                return false;

            resetAll = true;
        }
        reset_criteria = true;
        reset_achievement = false;
    }
    else if (AscEmu::Util::Strings::isEqual(args, "all"))
    {
        resetAll = true;
        reset_criteria = true;
    }
    else
    {
        achievement_id = std::stoul(args);
        if (achievement_id == 0)
        {
            if (args == nullptr)
                return false;

            const char* ptr = strstr(args, "|Hachievement:");
            if (ptr == nullptr)
                return false;

            // achievement id is just past "|Hachievement:" (14 bytes)
            achievement_id = std::stoul(ptr + 14);

            if (achievement_id == 0)
                return false;
        }
    }

    if (reset_achievement)
        selected_player->getAchievementMgr()->gmResetAchievement(achievement_id, resetAll);

    if (reset_criteria)
        selected_player->getAchievementMgr()->gmResetCriteria(achievement_id, resetAll);

    return true;
}
#endif
