/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.gm allowwhispers
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

//.gm blockwhispers
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

//.gm list
bool ChatHandler::HandleGMListCommand(const char* /*args*/, WorldSession* m_session)
{
    bool print_headline = true;

    bool is_gamemaster = m_session->GetPermissionCount() != 0;

    PlayerStorageMap::const_iterator itr;
    objmgr._playerslock.AcquireReadLock();
    for (itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession()->GetPermissionCount())
        {
            if (!sWorld.gamemaster_listOnlyActiveGMs)
            {
                if (print_headline)
                    GreenSystemMessage(m_session, "The following GMs are on this server:");

                if (sWorld.gamemaster_hidePermissions && !is_gamemaster)
                    SystemMessage(m_session, " - %s", itr->second->GetName());
                else
                    SystemMessage(m_session, " - %s [%s]", itr->second->GetName(), itr->second->GetSession()->GetPermissions());

                print_headline = false;
            }
            else if (sWorld.gamemaster_listOnlyActiveGMs && itr->second->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
            {
                if (itr->second->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
                {
                    if (print_headline)
                        GreenSystemMessage(m_session, "The following GMs are active on this server:");

                    if (sWorld.gamemaster_hidePermissions && !is_gamemaster)
                        SystemMessage(m_session, " - %s", itr->second->GetName());
                    else
                        SystemMessage(m_session, " - %s [%s]", itr->second->GetName(), itr->second->GetSession()->GetPermissions());

                    print_headline = false;
                }
                else
                {
                    SystemMessage(m_session, "No GMs are currently logged in on this server.");
                    print_headline = false;
                }
            }
        }
    }
    objmgr._playerslock.ReleaseReadLock();

    if (print_headline)
    {
        if (!sWorld.gamemaster_listOnlyActiveGMs)
            SystemMessage(m_session, "No GMs are currently logged in on this server.");
        else
            SystemMessage(m_session, "No GMs are currently active on this server.");
    }

    return true;
}

//.gm active
bool ChatHandler::HandleGMActiveCommand(const char* /*args*/, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
    {
        SystemMessage(m_session, "GM Flag removed.");
        BlueSystemMessage(m_session, "<GM> Will no longer show in chat messages or above your name until you use this command again.");
        player->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
        player->SetFaction(player->GetInitialFactionId());
        player->UpdatePvPArea();
        player->UpdateVisibility();
    }
    else
    {
        SystemMessage(m_session, "GM Flag set.");
        BlueSystemMessage(m_session, "<GM> will now appear above your name and in chat messages until you use this command again.");
        player->SetFlag(PLAYER_FLAGS, PLAYER_FLAG_GM);
        player->SetFaction(35);
        player->RemovePvPFlag();
        player->UpdateVisibility();
    }
    return true;
}
