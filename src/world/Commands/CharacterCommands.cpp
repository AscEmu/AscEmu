/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.character addhonorpoints
bool ChatHandler::HandleCharAddHonorPointsCommand(const char* args, WorldSession* m_session)
{
    uint32 honor_amount = args ? atol(args) : 1;

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor points added to Player %s.", honor_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor points to your character.", m_session->GetPlayer()->GetName(),  honor_amount);
    sGMLog.writefromsession(m_session, "added %u honor points to character %s", honor_amount, player_target->GetName());

    HonorHandler::AddHonorPointsToPlayer(player_target, honor_amount);

    return true;
}

//.character addhonorkill
bool ChatHandler::HandleCharAddHonorKillCommand(const char* args, WorldSession* m_session)
{
    uint32 kill_amount = args ? atol(args) : 1;
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    BlueSystemMessage(m_session, "%u honor kill points added to Player %s.", kill_amount, player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s added %u honor kill points to your character.", m_session->GetPlayer()->GetName(), kill_amount);
    sGMLog.writefromsession(m_session, "added %u honor kill points to character %s", kill_amount, player_target->GetName());

    player_target->m_killsToday += kill_amount;
    player_target->m_killsLifetime += kill_amount;
    player_target->SetUInt32Value(PLAYER_FIELD_KILLS, uint16(player_target->m_killsToday) | (player_target->m_killsYesterday << 16));
    player_target->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORBALE_KILLS, player_target->m_killsLifetime);

    return true;
}

//.character demorph
bool ChatHandler::HandleCharDeMorphCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->DeMorph();

    return true;
}

//.character levelup
bool ChatHandler::HandleCharLevelUpCommand(const char* args, WorldSession* m_session)
{
    uint32 levels = 0;
    levels = atoi(args);

    if (levels = 0 || levels < 0)
    {
        RedSystemMessage(m_session, "Command must be in format: .character levelup <level>.");
        RedSystemMessage(m_session, "A negative level is not allowed.");
        return true;
    }

    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    sGMLog.writefromsession(m_session, "used level up command on %s, with %u levels", player_target->GetName(), levels);

    levels += player_target->getLevel();

    if (levels > PLAYER_LEVEL_CAP)
        levels = PLAYER_LEVEL_CAP;

    auto level_info = objmgr.GetLevelInfo(player_target->getRace(), player_target->getClass(), levels);
    if (level_info == nullptr)
    {
        RedSystemMessage(m_session, "No LevelInfo for Leve: %u, Race: %u, Class: %u", levels, player_target->getRace(), player_target->getClass());
        return true;
    }

    player_target->ApplyLevelInfo(level_info, levels);

    if (player_target->getClass() == WARLOCK)
    {
        std::list<Pet*> summons = player_target->GetSummons();
        for (std::list<Pet*>::iterator itr = summons.begin(); itr != summons.end(); ++itr)
        {
            if ((*itr)->IsInWorld() && (*itr)->isAlive())
            {
                (*itr)->setLevel(levels);
                (*itr)->ApplyStatsForLevel();
                (*itr)->UpdateSpellList();
            }
        }
    }

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "%s leveled up to level: %u", player_target->GetName(), levels);
        BlueSystemMessage(player_target->GetSession(), "%s leveled you up to %u.", m_session->GetPlayer()->GetName(), levels);
        sGMLog.writefromsession(m_session, "leveled player %s to level %u", player_target->GetName(), levels);
    }
    else
    {
        BlueSystemMessage(m_session, "You leveled yourself to %u", levels);
    }

    player_target->Social_TellFriendsOnline();

    return true;
}

//.character removesickness
bool ChatHandler::HandleCharRemoveSickessCommand(const char* args, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    player_target->RemoveAura(15007);

    if (player_target != m_session->GetPlayer())
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from %s", player_target->GetName());
        BlueSystemMessage(player_target->GetSession(), "%s removed your resurection sickness.", m_session->GetPlayer()->GetName());
        sGMLog.writefromsession(m_session, "removed resurrection sickness from player %s", player_target->GetName());
    }
    else
    {
        BlueSystemMessage(m_session, "Removed resurrection sickness from you");
    }

    return true;
}

//.character setallexplored
bool ChatHandler::HandleCharSetAllExploredCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player_target = GetSelectedPlayer(m_session, true, true);
    if (player_target == nullptr)
        return true;

    SystemMessage(m_session, "%s has explored all zones now.", player_target->GetName());
    GreenSystemMessage(player_target->GetSession(), "%s sets all areas as explored for you.", m_session->GetPlayer()->GetName());
    sGMLog.writefromsession(m_session, "sets all areas as explored for player %s", player_target->GetName());

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_LENGTH; ++i)
    {
        player_target->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0xFFFFFFFF);
    }

#ifdef ENABLE_ACHIEVEMENTS
    player_target->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EXPLORE_AREA); // update
#endif
    return true;
}
