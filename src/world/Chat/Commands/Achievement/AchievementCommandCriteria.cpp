/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "AchievementCommandCriteria.hpp"
#include "Server/WorldSessionLog.hpp"

#if VERSION_STRING > TBC
#include "Utilities/Narrow.hpp"
#include "Management/AchievementMgr.h"
#endif

bool AchievementCommandCriteria::execute(const std::vector<std::string>& args, WorldSession* session)
{
#if VERSION_STRING > TBC
    if (args.size() < getArgumentCount())
    {
        session->systemMessage("Usage: .achieve criteria <all> or <achievement id>");
        return false;
    }

    const std::string& firstArgument = args[0];

    Player* selected_player = ChatHandler::GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    if (firstArgument == "all")
    {
        selected_player->getAchievementMgr()->gmCompleteCriteria(session, 0, true);
        session->systemMessage("All achievement criteria have now been completed for that player.");
        sGMLog.writefromsession(session, "completed all achievement criteria for player %s", selected_player->getName().c_str());
        return true;
    }

    uint32_t criteria_id = Util::stringToUint32(firstArgument, true);
    if (criteria_id == 0)
        return false;
    
    if (selected_player->getAchievementMgr()->gmCompleteCriteria(session, criteria_id))
    {
        session->systemMessage("The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(session, "completed achievement criteria %u for player %s", criteria_id, selected_player->getName().c_str());
    }

    return true;
#else
    return false;
#endif
}

std::string AchievementCommandCriteria::getHelp() const
{
    return "Completes the specified achievement criteria.";
}

const char* AchievementCommandCriteria::getRequiredPermission() const
{
    return "m";
}

size_t AchievementCommandCriteria::getArgumentCount() const
{
    return 1;
}
