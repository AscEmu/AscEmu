/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"

#include "Management/GuildMgr.h"
#include "Management/Guild.h"

//.guild create
bool ChatHandler::HandleGuildCreateCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->getName().c_str());
        return true;
    }

    if (strlen((char*)args) > 75)
    {
        // send message to user
        char buf[256];
        snprintf((char*)buf, 256, "The name was too long by %u", (uint32)strlen(args) - 75);
        SystemMessage(m_session, buf);
        return true;
    }

    for (uint32 i = 0; i < strlen(args); i++)
    {
        if (!isalpha(args[i]) && args[i] != ' ')
        {
            SystemMessage(m_session, "Error, name can only contain chars A-Z and a-z.");
            return true;
        }
    }

    Charter tempCharter(0, selected_player->getGuidLow(), CHARTER_TYPE_GUILD);
    tempCharter.SignatureCount = 0;
    tempCharter.GuildName = std::string(args);

    Guild* guild = new Guild;
    if (!guild->create(selected_player, std::string(args)))
    {
        delete guild;
        SystemMessage(m_session, "Guild not created");
        return true;
    }

    sGuildMgr.addGuild(guild);

    GreenSystemMessage(m_session, "Guild created");
    sGMLog.writefromsession(m_session, "Created guild '%s'", args);
    return true;
}

//.guild disband
bool ChatHandler::HandleGuildDisbandCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING < Cata
    if (!selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is not in a guild.", selected_player->getName().c_str());
        return true;
    }
#else
    if (!selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is not in a guild.", selected_player->getName().c_str());
        return true;
    }
#endif

    GreenSystemMessage(m_session, "Disbanded Guild: %s", selected_player->GetGuild()->getName().c_str());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", selected_player->GetGuild()->getName().c_str());
    selected_player->GetGuild()->disband();
    return true;
}

#if VERSION_STRING >= Cata
bool ChatHandler::HandleGuildInfoCommand(const char* /*args*/, WorldSession* session)
{
    Player* selected_player = GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!selected_player->GetGuild())
    {
        RedSystemMessage(session, "%s is not in a guild.", selected_player->getName().c_str());
        return true;
    }

    GreenSystemMessage(session, "Player Guild Information:");
    GreenSystemMessage(session, "  GuildName: %s", selected_player->GetGuild()->getName().c_str());
    GreenSystemMessage(session, "  GuildId: %u", selected_player->GetGuild()->getId());
    GreenSystemMessage(session, "  Player rank: %u", selected_player->GetRankFromDB(selected_player->getGuid()));

    return true;
}
#endif

//.guild join
bool ChatHandler::HandleGuildJoinCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING < Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->getName().c_str());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->getName().c_str());
        return true;
    }
#endif

    if (!*args)
        return false;

    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
    if (guild != nullptr)
    {
        uint32 memberCount = guild->getMembersCount();

        if (worldConfig.guild.maxMembers > 0 && memberCount >= worldConfig.guild.maxMembers)
        {
            m_session->SystemMessage("That guild is full.");
            return true;
        }

        guild->addMember(selected_player->getGuid(), 4);
        GreenSystemMessage(m_session, "You have joined the guild '%s'", guild->getName().c_str());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", guild->getName().c_str());
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Guild %s is not a valid guildname!", args);
    }

    return false;
}

//.guild listmembers
bool ChatHandler::HandleGuildListMembersCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->getName().c_str());
        return true;
    }

    GreenSystemMessage(m_session, "Now showing guild members for %s", selected_player->GetGuild()->getName().c_str());
    return true;
}

//.guild rename
bool ChatHandler::HandleRenameGuildCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING < Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->getName().c_str());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->getName().c_str());
        return true;
    }
#endif

    if (!*args)
        return false;

    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
    if (guild != nullptr)
    {
        RedSystemMessage(m_session, "Guild name %s is already taken.", args);
        return true;
    }
    else
    {
        GreenSystemMessage(m_session, "Changed guild name of %s to %s. This will take effect next restart.", selected_player->GetGuild()->getName().c_str(), args);
        CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), selected_player->GetGuild()->getId());
        sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", selected_player->GetGuild()->getName().c_str(), args);
    }

    return true;
}

//.guild removeplayer
bool ChatHandler::HandleGuildRemovePlayerCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->getName().c_str());
        return true;
    }

    if (selected_player->GetGuild()->getLeaderGUID() != selected_player->getGuid() || !m_session->GetPlayer()->isGMFlagSet())
    {
        RedSystemMessage(m_session, "Only guild leaders and gms can remove players from a guild!");
        return true;
    }

    GreenSystemMessage(m_session, "Kicked %s from Guild: %s", selected_player->getName().c_str(), selected_player->GetGuild()->getName().c_str());

    if (selected_player != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", selected_player->getName().c_str(), selected_player->GetGuild()->getName().c_str());

    selected_player->GetGuild()->handleRemoveMember(selected_player->GetSession(), selected_player->getGuid());

    return true;
}
