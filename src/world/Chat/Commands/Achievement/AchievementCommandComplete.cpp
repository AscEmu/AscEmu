/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementCommandComplete.hpp"
#include "Server/WorldSessionLog.hpp"

#if VERSION_STRING > TBC
#include "Utilities/Narrow.hpp"
#include "Management/AchievementMgr.h"
#endif

bool AchievementCommandComplete::execute(const std::vector<std::string>& args, WorldSession* session)
{
#if VERSION_STRING > TBC
    if (args.size() < getArgumentCount())
    {
        session->systemMessage("Usage: .achieve complete <all> or <achievement id>");
        return false;
    }

    const std::string& firstArgument = args[0];

    Player* selected_player = ChatHandler::GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    if (firstArgument == "all")
    {
        selected_player->getAchievementMgr()->gmCompleteAchievement(session, 0, true);
        session->systemMessage("All achievements have now been completed for that player.");
        sGMLog.writefromsession(session, "completed all achievements for player {}", selected_player->getName().data());
        return true;
    }

    uint32_t achievement_id = Util::stringToUint32(firstArgument, true);
    if (achievement_id == 0)
        return false;

    if (selected_player->getAchievementMgr()->gmCompleteAchievement(session, achievement_id))
    {
        session->systemMessage("The achievement has now been completed for that player.");
        sGMLog.write(session, "completed achievement {} for player {}", achievement_id, selected_player->getName().data());
    }

    return true;
#else
    return false;
#endif
}

std::string AchievementCommandComplete::getHelp() const
{
    return "Completes the specified achievement.";
}

const char* AchievementCommandComplete::getRequiredPermission() const
{
    return "m";
}

size_t AchievementCommandComplete::getArgumentCount() const
{
    return 1;
}
