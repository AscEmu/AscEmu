/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

bool ChatHandler::HandleGMAllowWhispersCommand(const char* args, WorldSession* m_session)
{
    if (args == 0)
    {
        RedSystemMessage(m_session, "No playername set.");
        RedSystemMessage(m_session, "Use .gm allowwhispers <playername>");
        return true;
    }

    auto player_cache = objmgr.GetPlayerCache(args, false);
    if (player_cache == nullptr)
    {
        RedSystemMessage(m_session, "Player %s not found.", args);
        return true;
    }

    m_session->GetPlayer()->m_cache->InsertValue64(CACHE_GM_TARGETS, player_cache->GetUInt32Value(CACHE_PLAYER_LOWGUID));
    std::string name;
    player_cache->GetStringValue(CACHE_PLAYER_NAME, name);
    BlueSystemMessage(m_session, "Now accepting whispers from %s.", name.c_str());
    player_cache->DecRef();

    return true;
}

bool ChatHandler::HandleGMBlockWhispersCommand(const char* args, WorldSession* m_session)
{
    if (args == 0)
    {
        RedSystemMessage(m_session, "No playername set.");
        RedSystemMessage(m_session, "Use .gm blockwhispers <playername>");
        return true;
    }

    auto player_cache = objmgr.GetPlayerCache(args, false);
    if (player_cache == nullptr)
    {
        RedSystemMessage(m_session, "Player %s not found.", args);
        return true;
    }

    m_session->GetPlayer()->m_cache->RemoveValue64(CACHE_GM_TARGETS, player_cache->GetUInt32Value(CACHE_PLAYER_LOWGUID));
    std::string name;
    player_cache->GetStringValue(CACHE_PLAYER_NAME, name);
    BlueSystemMessage(m_session, "Now blocking whispers from %s.", name.c_str());
    player_cache->DecRef();

    return true;
}
