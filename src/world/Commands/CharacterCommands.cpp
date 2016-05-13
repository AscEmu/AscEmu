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
