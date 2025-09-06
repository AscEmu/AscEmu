/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <sstream>

#include "Chat/ChatDefines.hpp"
#include "Chat/ChatCommandHandler.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

//.gm active
bool ChatCommandHandler::HandleGMActiveCommand(const char* args, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    bool toggle_no_notice = std::string(args) == "no_notice" ? true : false;
    if (player->isGMFlagSet())
    {
        if (!toggle_no_notice)
        {
            systemMessage(m_session, "GM Flag removed.");
            blueSystemMessage(m_session, "<GM> Will no longer show in chat messages or above your name until you use this command again.");
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

        systemMessage(m_session, "GM Flag set.");
        blueSystemMessage(m_session, "<GM> will now appear above your name and in chat messages until you use this command again.");
        player->addPlayerFlags(PLAYER_FLAG_GM);
        player->setFaction(35);
        player->removePvpFlag();
        player->updateVisibility();
    }
    return true;
}

//.gm allowwhispers
bool ChatCommandHandler::HandleGMAllowWhispersCommand(const char* args, WorldSession* m_session)
{
    if (args == 0)
    {
        redSystemMessage(m_session, "No playername set.");
        redSystemMessage(m_session, "Use .gm allowwhispers <playername>");
        return true;
    }

    const auto playerTarget = sObjectMgr.getPlayer(args, false);
    if (playerTarget == nullptr)
    {
        redSystemMessage(m_session, "Player %s not found.", args);
        return true;
    }

    m_session->GetPlayer()->addToGMTargetList(playerTarget->getGuidLow());
    std::string name = playerTarget->getName();
    blueSystemMessage(m_session, "Now accepting whispers from {}.", name);

    return true;
}

//.gm announce
bool ChatCommandHandler::HandleGMAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
    {
        redSystemMessage(m_session, "No announce set.");
        redSystemMessage(m_session, "Use .gm announce <your text>");
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
bool ChatCommandHandler::HandleGMBlockWhispersCommand(const char* args, WorldSession* m_session)
{
    if (args == 0)
    {
        redSystemMessage(m_session, "No playername set.");
        redSystemMessage(m_session, "Use .gm blockwhispers <playername>");
        return true;
    }

    auto playerTarget = sObjectMgr.getPlayer(args, false);
    if (playerTarget == nullptr)
    {
        redSystemMessage(m_session, "Player {} not found.", args);
        return true;
    }

    m_session->GetPlayer()->removeFromGMTargetList(playerTarget->getGuidLow());
    std::string name = playerTarget->getName();
    blueSystemMessage(m_session, "Now blocking whispers from {}.", name);

    return true;
}

//.gm devtag
bool ChatCommandHandler::HandleGMDevTagCommand(const char* args, WorldSession* m_session)
{
    auto player = m_session->GetPlayer();
    bool toggle_no_notice = std::string(args) == "no_notice" ? true : false;

#if VERSION_STRING >= WotLK
    if (player->hasPlayerFlags(PLAYER_FLAG_DEVELOPER))
    {
        if (!toggle_no_notice)
        {
            systemMessage(m_session, "DEV Flag removed.");
            blueSystemMessage(m_session, "<DEV> Will no longer show in chat messages or above your name until you use this command again.");
        }
        player->removePlayerFlags(PLAYER_FLAG_DEVELOPER);
    }
    else
    {
        if (player->isGMFlagSet())
            HandleGMActiveCommand("no_notice", m_session);

        systemMessage(m_session, "DEV Flag set.");
        blueSystemMessage(m_session, "<DEV> will now appear above your name and in chat messages until you use this command again.");
        player->addPlayerFlags(PLAYER_FLAG_DEVELOPER);
    }
#endif

    return true;
}

//.gm list
bool ChatCommandHandler::HandleGMListCommand(const char* /*args*/, WorldSession* m_session)
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
                    greenSystemMessage(m_session, "The following GMs are on this server:");

                if (worldConfig.gm.hidePermissions && !is_gamemaster)
                    systemMessage(m_session, " - {}", player->getName());
                else
                    systemMessage(m_session, " - {} [{}]", player->getName(), player->getSession()->GetPermissions().get());

                print_headline = false;
            }
            else if (worldConfig.gm.listOnlyActiveGms && player->isGMFlagSet())
            {
                if (player->isGMFlagSet())
                {
                    if (print_headline)
                        greenSystemMessage(m_session, "The following GMs are active on this server:");

                    if (worldConfig.gm.hidePermissions && !is_gamemaster)
                        systemMessage(m_session, " - {}", player->getName());
                    else
                        systemMessage(m_session, " - {} [{}]", player->getName(), player->getSession()->GetPermissions().get());

                    print_headline = false;
                }
                else
                {
                    systemMessage(m_session, "No GMs are currently logged in on this server.");
                    print_headline = false;
                }
            }
        }
    }

    if (print_headline)
    {
        if (!worldConfig.gm.listOnlyActiveGms)
            systemMessage(m_session, "No GMs are currently logged in on this server.");
        else
            systemMessage(m_session, "No GMs are currently active on this server.");
    }

    return true;
}

//.gm logcomment
bool ChatCommandHandler::HandleGMLogCommentCommand(const char* args, WorldSession* m_session)
{
    if (!args)
    {
        redSystemMessage(m_session, "No logcomment set.");
        redSystemMessage(m_session, "Use .gm logcomment <your comment message>");
        return true;
    }

    blueSystemMessage(m_session, "Added Logcomment: {}", args);
    sGMLog.writefromsession(m_session, "Comment: %s", args);
    return true;
}
