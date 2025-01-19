/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementCommandReset.hpp"
#include "Server/WorldSessionLog.hpp"

#if VERSION_STRING > TBC
#include "Utilities/Narrow.hpp"
#include "Management/AchievementMgr.h"
#endif

bool AchievementCommandReset::execute(const std::vector<std::string>& args, WorldSession* session)
{
#if VERSION_STRING > TBC
    if (args.size() < getArgumentCount())
    {
        session->systemMessage("Usage: .achieve (<criteria> <all>/<achievement id>) | (<all>) | (<achievement id>)");
        return false;
    }

    const std::string& firstArgument = args[0];
    const std::string& secondArgument = (args.size() >= 2) ? args[1] : "0";

    Player* selected_player = ChatHandler::GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    bool reset_achievement = true;
    bool reset_criteria = false;
    bool resetAll = false;
    uint32_t achievement_id = 0;

    if (firstArgument == "criteria ")
    {
        achievement_id = Util::stringToUint32(secondArgument, true);
        if (achievement_id == 0)
        {
            if (secondArgument != "all")
                return false;

            resetAll = true;
        }
        reset_criteria = true;
        reset_achievement = false;
    }
    else if (firstArgument == "all")
    {
        resetAll = true;
        reset_criteria = true;
    }
    else
    {
        achievement_id = Util::stringToUint32(firstArgument, true);
        if (achievement_id == 0)
        {
            if (firstArgument == "|Hachievement:")
                return false;

            // achievement id is just past "|Hachievement:" (14 bytes)
            achievement_id = Util::stringToUint32(secondArgument, true);

            if (achievement_id == 0)
                return false;
        }
    }

    if (reset_achievement)
        selected_player->getAchievementMgr()->gmResetAchievement(achievement_id, resetAll);

    if (reset_criteria)
        selected_player->getAchievementMgr()->gmResetCriteria(achievement_id, resetAll);

    return true;
#else
    return false;
#endif
}

std::string AchievementCommandReset::getHelp() const
{
    return "Resets achievement data from the target.";
}

const char* AchievementCommandReset::getRequiredPermission() const
{
    return "m";
}

size_t AchievementCommandReset::getArgumentCount() const
{
    return 1;
}
