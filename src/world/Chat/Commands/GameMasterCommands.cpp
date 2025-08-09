/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

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
        player->setFaction(player->getInitialFactionId());
        player->updatePvPArea();
        player->updateVisibility();
    }
    else
    {
#if VERSION_STRING >= WotLK
        if (player->hasPlayerFlags(PLAYER_FLAG_DEVELOPER))
            HandleGMDevTagCommand("no_notice", m_session);
#endif

        SystemMessage(m_session, "GM Flag set.");
        BlueSystemMessage(m_session, "<GM> will now appear above your name and in chat messages until you use this command again.");
        player->addPlayerFlags(PLAYER_FLAG_GM);
        player->setFaction(35);
        player->removePvpFlag();
        player->updateVisibility();
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

    const auto playerTarget = sObjectMgr.getPlayer(args, false);
    if (playerTarget == nullptr)
    {
        RedSystemMessage(m_session, "Player %s not found.", args);
        return true;
    }

    m_session->GetPlayer()->addToGMTargetList(playerTarget->getGuidLow());
    std::string name = playerTarget->getName();
    BlueSystemMessage(m_session, "Now accepting whispers from %s.", name.c_str());

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

    auto playerTarget = sObjectMgr.getPlayer(args, false);
    if (playerTarget == nullptr)
    {
        RedSystemMessage(m_session, "Player %s not found.", args);
        return true;
    }

    m_session->GetPlayer()->removeFromGMTargetList(playerTarget->getGuidLow());
    std::string name = playerTarget->getName();
    BlueSystemMessage(m_session, "Now blocking whispers from %s.", name.c_str());

    return true;
}

//.gm devtag
bool ChatHandler::HandleGMDevTagCommand(const char* args, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    bool toggle_no_notice = std::string(args) == "no_notice" ? true : false;

#if VERSION_STRING >= WotLK
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
#endif

    return true;
}

//.gm list
bool ChatHandler::HandleGMListCommand(const char* /*args*/, WorldSession* m_session)
{
    bool print_headline = true;

    bool is_gamemaster = m_session->hasPermissions();

    std::lock_guard guard(sObjectMgr.m_playerLock);
    for (const auto playerPair : sObjectMgr.getPlayerStorage())
    {
        Player* player = playerPair.second;
        if (player->getSession()->hasPermissions())
        {
            if (!worldConfig.gm.listOnlyActiveGms)
            {
                if (print_headline)
                    GreenSystemMessage(m_session, "The following GMs are on this server:");

                if (worldConfig.gm.hidePermissions && !is_gamemaster)
                    SystemMessage(m_session, " - %s", player->getName().c_str());
                else
                    SystemMessage(m_session, " - %s [%s]", player->getName().c_str(), player->getSession()->GetPermissions().get());

                print_headline = false;
            }
            else if (worldConfig.gm.listOnlyActiveGms && player->isGMFlagSet())
            {
                if (player->isGMFlagSet())
                {
                    if (print_headline)
                        GreenSystemMessage(m_session, "The following GMs are active on this server:");

                    if (worldConfig.gm.hidePermissions && !is_gamemaster)
                        SystemMessage(m_session, " - %s", player->getName().c_str());
                    else
                        SystemMessage(m_session, " - %s [%s]", player->getName().c_str(), player->getSession()->GetPermissions().get());

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
