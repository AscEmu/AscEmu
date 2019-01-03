/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Server/WorldSession.h"
#include "Objects/ObjectMgr.h"

//.gm active
bool ChatHandler::HandleGMActiveCommand(const char* args, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    bool toggle_no_notice = std::string(args) == "no_notice" ? true : false;
    if (player->isGMFlagSet())
    {
        if (!toggle_no_notice)
        {
            SystemMessage(m_session, "GM Flag removed.");
            BlueSystemMessage(m_session, "<GM> Will no longer show in chat messages or above your name until you use this command again.");
        }
        player->removePlayerFlags(PLAYER_FLAG_GM);
        player->SetFaction(player->GetInitialFactionId());
        player->UpdatePvPArea();
        player->UpdateVisibility();
    }
    else
    {
        if (player->hasPlayerFlags(PLAYER_FLAG_DEVELOPER))
            HandleGMDevTagCommand("no_notice", m_session);

        SystemMessage(m_session, "GM Flag set.");
        BlueSystemMessage(m_session, "<GM> will now appear above your name and in chat messages until you use this command again.");
        player->addPlayerFlags(PLAYER_FLAG_GM);
        player->SetFaction(35);
        player->removePvpFlag();
        player->UpdateVisibility();
    }
    return true;
}

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

//.gm announce
bool ChatHandler::HandleGMAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
    {
        RedSystemMessage(m_session, "No announce set.");
        RedSystemMessage(m_session, "Use .gm announce <your text>");
        return true;
    }

    std::stringstream teamAnnounce;
    teamAnnounce << MSG_COLOR_RED << "[Team]" << MSG_COLOR_GREEN << " |Hplayer:" << m_session->GetPlayer()->getName().c_str();
    teamAnnounce << "|h[" << m_session->GetPlayer()->getName().c_str() << "]|h:" << MSG_COLOR_YELLOW << " " << args;

    sWorld.sendMessageToOnlineGms(teamAnnounce.str());

    sGMLog.writefromsession(m_session, "used .gm annouince command: [%s]", args);

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

//.gm devtag
bool ChatHandler::HandleGMDevTagCommand(const char* args, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    bool toggle_no_notice = std::string(args) == "no_notice" ? true : false;

    if (player->hasPlayerFlags(PLAYER_FLAG_DEVELOPER))
    {
        if (!toggle_no_notice)
        {
            SystemMessage(m_session, "DEV Flag removed.");
            BlueSystemMessage(m_session, "<DEV> Will no longer show in chat messages or above your name until you use this command again.");
        }
        player->removePlayerFlags(PLAYER_FLAG_DEVELOPER);
    }
    else
    {
        if (player->isGMFlagSet())
            HandleGMActiveCommand("no_notice", m_session);

        SystemMessage(m_session, "DEV Flag set.");
        BlueSystemMessage(m_session, "<DEV> will now appear above your name and in chat messages until you use this command again.");
        player->addPlayerFlags(PLAYER_FLAG_DEVELOPER);
    }

    return true;
}

//.gm list
bool ChatHandler::HandleGMListCommand(const char* /*args*/, WorldSession* m_session)
{
    bool print_headline = true;

    bool is_gamemaster = m_session->GetPermissionCount() != 0;

    objmgr._playerslock.AcquireReadLock();
    for (PlayerStorageMap::const_iterator itr = objmgr._players.begin(); itr != objmgr._players.end(); ++itr)
    {
        if (itr->second->GetSession()->GetPermissionCount())
        {
            if (!worldConfig.gm.listOnlyActiveGms)
            {
                if (print_headline)
                    GreenSystemMessage(m_session, "The following GMs are on this server:");

                if (worldConfig.gm.hidePermissions && !is_gamemaster)
                    SystemMessage(m_session, " - %s", itr->second->getName().c_str());
                else
                    SystemMessage(m_session, " - %s [%s]", itr->second->getName().c_str(), itr->second->GetSession()->GetPermissions());

                print_headline = false;
            }
            else if (worldConfig.gm.listOnlyActiveGms && itr->second->isGMFlagSet())
            {
                if (itr->second->isGMFlagSet())
                {
                    if (print_headline)
                        GreenSystemMessage(m_session, "The following GMs are active on this server:");

                    if (worldConfig.gm.hidePermissions && !is_gamemaster)
                        SystemMessage(m_session, " - %s", itr->second->getName().c_str());
                    else
                        SystemMessage(m_session, " - %s [%s]", itr->second->getName().c_str(), itr->second->GetSession()->GetPermissions());

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
        if (!worldConfig.gm.listOnlyActiveGms)
            SystemMessage(m_session, "No GMs are currently logged in on this server.");
        else
            SystemMessage(m_session, "No GMs are currently active on this server.");
    }

    return true;
}

//.gm logcomment
bool ChatHandler::HandleGMLogCommentCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        RedSystemMessage(m_session, "No logcomment set.");
        RedSystemMessage(m_session, "Use .gm logcomment <your comment message>");
        return true;
    }

    BlueSystemMessage(m_session, "Added Logcomment: %s", args);
    sGMLog.writefromsession(m_session, "Comment: %s", args);
    return true;
}
