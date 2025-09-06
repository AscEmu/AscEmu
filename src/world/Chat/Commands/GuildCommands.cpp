/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatCommandHandler.hpp"
#include "Management/Charter.hpp"
#include "Management/Guild/Guild.hpp"
#include "Management/Guild/GuildMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Server/World.h"
#include "Server/WorldSession.h"
#include "Server/WorldSessionLog.hpp"

//.guild create
bool ChatCommandHandler::HandleGuildCreateCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (selected_player->getGuild())
    {
        redSystemMessage(m_session, "{} is already in a guild.", selected_player->getName());
        return true;
    }

    if (strlen(args) > 75)
    {
        systemMessage(m_session, "The name was too long by {}", uint32_t(strlen(args) - 75));
        return true;
    }

    for (uint32_t i = 0; i < strlen(args); i++)
    {
        if (!isalpha(args[i]) && args[i] != ' ')
        {
            systemMessage(m_session, "Error, name can only contain chars A-Z and a-z.");
            return true;
        }
    }

    Charter tempCharter(0, selected_player->getGuidLow(), CHARTER_TYPE_GUILD);
    tempCharter.setGuildName(std::string(args));

    auto* guild = sGuildMgr.createGuild(selected_player, std::string(args));
    if (guild == nullptr)
    {
        systemMessage(m_session, "Guild not created");
        return true;
    }

    greenSystemMessage(m_session, "Guild created");
    sGMLog.writefromsession(m_session, "Created guild '%s'", args);
    return true;
}

//.guild disband
bool ChatCommandHandler::HandleGuildDisbandCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!selected_player->isInGuild())
    {
        redSystemMessage(m_session, "{} is not in a guild.", selected_player->getName());
        return true;
    }

    greenSystemMessage(m_session, "Disbanded Guild: {}", selected_player->getGuild()->getName());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", selected_player->getGuild()->getName().c_str());
    selected_player->getGuild()->disband();
    return true;
}

#if VERSION_STRING >= Cata
bool ChatCommandHandler::HandleGuildInfoCommand(const char* /*args*/, WorldSession* session)
{
    Player* selected_player = GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!selected_player->getGuild())
    {
        redSystemMessage(session, "{} is not in a guild.", selected_player->getName());
        return true;
    }

    greenSystemMessage(session, "Player Guild Information:");
    greenSystemMessage(session, "  GuildName: {}", selected_player->getGuild()->getName());
    greenSystemMessage(session, "  GuildId: {}", selected_player->getGuild()->getId());
    greenSystemMessage(session, "  Player rank: {}", selected_player->getGuildRankFromDB());

    return true;
}
#endif

//.guild join
bool ChatCommandHandler::HandleGuildJoinCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->isInGuild())
    {
        redSystemMessage(m_session, "{} is already in a guild.", selected_player->getName());
        return true;
    }

    if (!*args)
        return false;

    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
    if (guild != nullptr)
    {
        uint32_t memberCount = guild->getMembersCount();

        if (worldConfig.guild.maxMembers > 0 && memberCount >= worldConfig.guild.maxMembers)
        {
            m_session->SystemMessage("That guild is full.");
            return true;
        }

        guild->addMember(selected_player->getGuid(), 4);
        greenSystemMessage(m_session, "You have joined the guild '{}'", guild->getName());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", guild->getName().c_str());
        return true;
    }
    redSystemMessage(m_session, "Guild {} is not a valid guildname!", args);

    return false;
}

//.guild listmembers
bool ChatCommandHandler::HandleGuildListMembersCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->getGuild())
    {
        redSystemMessage(m_session, "{} not in a guild.", selected_player->getName());
        return true;
    }

    greenSystemMessage(m_session, "Now showing guild members for {}", selected_player->getGuild()->getName());
    return true;
}

//.guild rename
bool ChatCommandHandler::HandleRenameGuildCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->isInGuild())
    {
        redSystemMessage(m_session, "{} not in a guild.", selected_player->getName());
        return true;
    }

    if (!*args)
        return false;

    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
    if (guild != nullptr)
    {
        redSystemMessage(m_session, "Guild name {} is already taken.", args);
        return true;
    }
    greenSystemMessage(m_session, "Changed guild name of {} to {}. This will take effect next restart.", selected_player->getGuild()->getName(), args);
    CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), selected_player->getGuild()->getId());
    sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", selected_player->getGuild()->getName().c_str(), args);

    return true;
}

//.guild removeplayer
bool ChatCommandHandler::HandleGuildRemovePlayerCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->getGuild())
    {
        redSystemMessage(m_session, "{} not in a guild.", selected_player->getName());
        return true;
    }

    if (selected_player->getGuild()->getLeaderGUID() != selected_player->getGuid() || !m_session->GetPlayer()->isGMFlagSet())
    {
        redSystemMessage(m_session, "Only guild leaders and gms can remove players from a guild!");
        return true;
    }

    greenSystemMessage(m_session, "Kicked {} from Guild: {}", selected_player->getName(), selected_player->getGuild()->getName());

    if (selected_player != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", selected_player->getName().c_str(), selected_player->getGuild()->getName().c_str());

    selected_player->getGuild()->handleRemoveMember(selected_player->getSession(), selected_player->getGuid());

    return true;
}
