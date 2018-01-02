/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"

#if VERSION_STRING > TBC
//.achieve complete
bool ChatHandler::HandleAchievementCompleteCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32 achievement_id = atol(args);
    if (achievement_id == 0)
    {
        achievement_id = GetAchievementIDFromLink(args);
        if (achievement_id == 0)
        {
            if (stricmp(args, "all") == 0)
            {
                selected_player->GetAchievementMgr().GMCompleteAchievement(m_session, -1);
                SystemMessage(m_session, "All achievements have now been completed for that player.");
                sGMLog.writefromsession(m_session, "completed all achievements for player %s", selected_player->GetName());
                return true;
            }
            return false;
        }
    }
    else if (selected_player->GetAchievementMgr().GMCompleteAchievement(m_session, achievement_id))
    {
        SystemMessage(m_session, "The achievement has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement %u for player %s", achievement_id, selected_player->GetName());
    }
    return true;
}

//.achieve criteria
bool ChatHandler::HandleAchievementCriteriaCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    uint32 criteria_id = atol(args);
    if (criteria_id == 0)
    {
        if (stricmp(args, "all") == 0)
        {
            selected_player->GetAchievementMgr().GMCompleteCriteria(m_session, -1);
            SystemMessage(m_session, "All achievement criteria have now been completed for that player.");
            sGMLog.writefromsession(m_session, "completed all achievement criteria for player %s", selected_player->GetName());
            return true;
        }
        return false;
    }
    else if (selected_player->GetAchievementMgr().GMCompleteCriteria(m_session, criteria_id))
    {
        SystemMessage(m_session, "The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement criteria %u for player %s", criteria_id, selected_player->GetName());
    }
    return true;
}

//.achieve reset
bool ChatHandler::HandleAchievementResetCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    bool reset_achievement = true;
    bool reset_criteria = false;
    int32 achievement_id;

    if (strnicmp(args, "criteria ", 9) == 0)
    {
        achievement_id = atol(args + 9);
        if (achievement_id == 0)
        {
            if (stricmp(args + 9, "all") != 0)
            {
                return false;
            }
            achievement_id = -1;
        }
        reset_criteria = true;
        reset_achievement = false;
    }
    else if (stricmp(args, "all") == 0)
    {
        achievement_id = -1;
        reset_criteria = true;
    }
    else
    {
        achievement_id = atol(args);
        if (achievement_id == 0)
        {
            achievement_id = GetAchievementIDFromLink(args);
            if (achievement_id == 0)
                return false;
        }
    }

    if (reset_achievement)
        selected_player->GetAchievementMgr().GMResetAchievement(achievement_id);

    if (reset_criteria)
        selected_player->GetAchievementMgr().GMResetCriteria(achievement_id);

    return true;
}
#endif
