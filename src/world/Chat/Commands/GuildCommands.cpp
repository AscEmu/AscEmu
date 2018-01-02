/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"
#include "Server/MainServerDefines.h"
#include "Chat/ChatHandler.hpp"
#include "Objects/ObjectMgr.h"

#if VERSION_STRING == Cata
#include "GameCata/Management/Guild.h"
#include "GameCata/Management/GuildMgr.h"
#else
#include "Management/Guild.h"
#endif

//.guild create
bool ChatHandler::HandleGuildCreateCommand(const char* args, WorldSession* m_session)
{
#if VERSION_STRING != Cata
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->GetName());
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

    Guild* pGuild = NULL;
    pGuild = objmgr.GetGuildByGuildName(std::string(args));

    if (pGuild)
    {
        RedSystemMessage(m_session, "Guild name is already taken.");
        return true;
    }

    Charter tempCharter(0, selected_player->GetLowGUID(), CHARTER_TYPE_GUILD);
    tempCharter.SignatureCount = 0;
    tempCharter.GuildName = std::string(args);

    pGuild = Guild::Create();
    pGuild->CreateFromCharter(&tempCharter, selected_player->GetSession());
    GreenSystemMessage(m_session, "Guild created");
    sGMLog.writefromsession(m_session, "Created guild '%s'", args);
    return true;
#else
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!*args)
        return false;

    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->GetName());
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

    Charter tempCharter(0, selected_player->GetLowGUID(), CHARTER_TYPE_GUILD);
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

    //\todo danko
    //pGuild->CreateFromCharter(&tempCharter, selected_player->GetSession());
    GreenSystemMessage(m_session, "Guild created");
    sGMLog.writefromsession(m_session, "Created guild '%s'", args);
    return true;
#endif
}

//.guild disband
bool ChatHandler::HandleGuildDisbandCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING != Cata
    if (!selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is not in a guild.", selected_player->GetName());
        return true;
    }
#else
    if (!selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is not in a guild.", selected_player->GetName());
        return true;
    }
#endif

#if VERSION_STRING != Cata
    GreenSystemMessage(m_session, "Disbanded Guild: %s", selected_player->GetGuild()->getGuildName());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", selected_player->GetGuild()->getGuildName());
#else
    GreenSystemMessage(m_session, "Disbanded Guild: %s", selected_player->GetGuild()->getName().c_str());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", selected_player->GetGuild()->getName().c_str());
#endif
    selected_player->GetGuild()->disband();
    return true;
}

#if VERSION_STRING == Cata
bool ChatHandler::HandleGuildInfoCommand(const char* /*args*/, WorldSession* session)
{
    Player* selected_player = GetSelectedPlayer(session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!selected_player->GetGuild())
    {
        RedSystemMessage(session, "%s is not in a guild.", selected_player->GetName());
        return true;
    }

    GreenSystemMessage(session, "Player Guild Information:");
    GreenSystemMessage(session, "  GuildName: %s", selected_player->GetGuild()->getName().c_str());
    GreenSystemMessage(session, "  GuildId: %u", selected_player->GetGuild()->getId());
    GreenSystemMessage(session, "  Player rank: %u", selected_player->GetRankFromDB(selected_player->GetGUID()));

    return true;
}
#endif

//.guild join
bool ChatHandler::HandleGuildJoinCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING != Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->GetName());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->GetName());
        return true;
    }
#endif

    if (!*args)
        return false;

#if VERSION_STRING != Cata
    Guild* guild = objmgr.GetGuildByGuildName(std::string(args));
    if (guild != nullptr)
    {
        guild->getLock().Acquire();
        uint32 memberCount = static_cast<uint32>(guild->GetNumMembers());
        guild->getLock().Release();

        if (memberCount >= MAX_GUILD_MEMBERS)
        {
            m_session->SystemMessage("That guild is full.");
            return true;
        }

        guild->AddGuildMember(selected_player->getPlayerInfo(), m_session, -2);
        GreenSystemMessage(m_session, "You have joined the guild '%s'", guild->getGuildName());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", guild->getGuildName());
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Guild %s is not a valid guildname!", args);
    }
#else
    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
    if (guild != nullptr)
    {
        uint32 memberCount = guild->getMembersCount();

        if (memberCount >= worldConfig.guild.maxMembers)
        {
            m_session->SystemMessage("That guild is full.");
            return true;
        }

        guild->addMember(selected_player->GetGUID(), 4);
        GreenSystemMessage(m_session, "You have joined the guild '%s'", guild->getName().c_str());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", guild->getName().c_str());
        return true;
    }
    else
    {
        RedSystemMessage(m_session, "Guild %s is not a valid guildname!", args);
    }
#endif

    return false;
}

//.guild listmembers
bool ChatHandler::HandleGuildListMembersCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING != Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#endif

#if VERSION_STRING != Cata
    GreenSystemMessage(m_session, "Now showing guild members for %s", selected_player->GetGuild()->getGuildName());
#else
    GreenSystemMessage(m_session, "Now showing guild members for %s", selected_player->GetGuild()->getName().c_str());
#endif

#if VERSION_STRING != Cata
    selected_player->GetGuild()->Lock();
    for (GuildMemberMap::iterator itr = selected_player->GetGuild()->GetGuildMembersBegin(); itr != selected_player->GetGuild()->GetGuildMembersEnd(); ++itr)
    {
        GuildMember* member = itr->second;
        if (!member || !member->pPlayer)
            continue;

        BlueSystemMessage(m_session, "%s (Rank: %s)", member->pPlayer->name, member->pRank->szRankName);
    }
    selected_player->GetGuild()->Unlock();
#endif

    return true;
}

//.guild rename
bool ChatHandler::HandleRenameGuildCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING != Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#endif

    if (!*args)
        return false;

#if VERSION_STRING != Cata
    Guild* guild = objmgr.GetGuildByGuildName(std::string(args));
#else
    Guild* guild = sGuildMgr.getGuildByName(std::string(args));
#endif
    if (guild != nullptr)
    {
        RedSystemMessage(m_session, "Guild name %s is already taken.", args);
        return true;
    }
    else
    {
#if VERSION_STRING != Cata
        GreenSystemMessage(m_session, "Changed guild name of %s to %s. This will take effect next restart.", selected_player->GetGuild()->getGuildName(), args);
        CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), selected_player->GetGuild()->getGuildId());
        sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", selected_player->GetGuild()->getGuildName(), args);
#else
        GreenSystemMessage(m_session, "Changed guild name of %s to %s. This will take effect next restart.", selected_player->GetGuild()->getName().c_str(), args);
        CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), selected_player->GetGuild()->getId());
        sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", selected_player->GetGuild()->getName().c_str(), args);
#endif
    }

    return true;
}

//.guild removeplayer
bool ChatHandler::HandleGuildRemovePlayerCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

#if VERSION_STRING != Cata
    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#else
    if (selected_player->GetGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }
#endif

#if VERSION_STRING != Cata
    if (selected_player->GetGuild()->GetGuildLeader() != selected_player->GetLowGUID() || !m_session->GetPlayer()->isGMFlagSet())
    {
        RedSystemMessage(m_session, "Only guild leaders and gms can remove players from a guild!");
        return true;
    }

    GreenSystemMessage(m_session, "Kicked %s from Guild: %s", selected_player->GetName(), selected_player->GetGuild()->getGuildName());

    if (selected_player != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", selected_player->GetName(), selected_player->GetGuild()->getGuildName());

    selected_player->GetGuild()->RemoveGuildMember(selected_player->getPlayerInfo(), selected_player->GetSession());
#else
    if (selected_player->GetGuild()->getLeaderGUID() != selected_player->GetGUID() || !m_session->GetPlayer()->isGMFlagSet())
    {
        RedSystemMessage(m_session, "Only guild leaders and gms can remove players from a guild!");
        return true;
    }

    GreenSystemMessage(m_session, "Kicked %s from Guild: %s", selected_player->GetName(), selected_player->GetGuild()->getName().c_str());

    if (selected_player != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", selected_player->GetName(), selected_player->GetGuild()->getName().c_str());

    selected_player->GetGuild()->handleRemoveMember(selected_player->GetSession(), selected_player->GetGUID());
#endif
    return true;
}
