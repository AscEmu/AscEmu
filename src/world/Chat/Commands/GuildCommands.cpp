/*
Copyright (c) 2014-2017 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.guild create
bool ChatHandler::HandleGuildCreateCommand(const char* args, WorldSession* m_session)
{
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
}

//.guild disband
bool ChatHandler::HandleGuildDisbandCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (!selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is not in a guild.", selected_player->GetName());
        return true;
    }

    GreenSystemMessage(m_session, "Disbanded Guild: %s", selected_player->GetGuild()->GetGuildName());
    sGMLog.writefromsession(m_session, "Disbanded Guild %s", selected_player->GetGuild()->GetGuildName());
    selected_player->GetGuild()->Disband();
    return true;
}

//.guild join
bool ChatHandler::HandleGuildJoinCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s is already in a guild.", selected_player->GetName());
        return true;
    }

    if (!*args)
        return false;

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
        GreenSystemMessage(m_session, "You have joined the guild '%s'", guild->GetGuildName());
        sGMLog.writefromsession(m_session, "Force joined guild '%s'", guild->GetGuildName());
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

    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }

    GreenSystemMessage(m_session, "Now showing guild members for %s", selected_player->GetGuild()->GetGuildName());

    selected_player->GetGuild()->Lock();
    for (GuildMemberMap::iterator itr = selected_player->GetGuild()->GetGuildMembersBegin(); itr != selected_player->GetGuild()->GetGuildMembersEnd(); ++itr)
    {
        GuildMember* member = itr->second;
        if (!member || !member->pPlayer)
            continue;

        BlueSystemMessage(m_session, "%s (Rank: %s)", member->pPlayer->name, member->pRank->szRankName);
    }
    selected_player->GetGuild()->Unlock();

    return true;
}

//.guild rename
bool ChatHandler::HandleRenameGuildCommand(const char* args, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }

    if (!*args)
        return false;

    Guild* guild = objmgr.GetGuildByGuildName(std::string(args));
    if (guild != nullptr)
    {
        RedSystemMessage(m_session, "Guild name %s is already taken.", args);
        return true;
    }
    else
    {
        GreenSystemMessage(m_session, "Changed guild name of %s to %s. This will take effect next restart.", selected_player->GetGuild()->GetGuildName(), args);
        CharacterDatabase.Execute("UPDATE guilds SET `guildName` = \'%s\' WHERE `guildId` = '%u'", CharacterDatabase.EscapeString(std::string(args)).c_str(), selected_player->GetGuild()->GetGuildId());
        sGMLog.writefromsession(m_session, "Changed guild name of '%s' to '%s'", selected_player->GetGuild()->GetGuildName(), args);
    }

    return true;
}

//.guild removeplayer
bool ChatHandler::HandleGuildRemovePlayerCommand(const char* /*args*/, WorldSession* m_session)
{
    Player* selected_player = GetSelectedPlayer(m_session, true, true);
    if (selected_player == nullptr)
        return true;

    if (selected_player->IsInGuild())
    {
        RedSystemMessage(m_session, "%s not in a guild.", selected_player->GetName());
        return true;
    }

    if (selected_player->GetGuild()->GetGuildLeader() != selected_player->GetLowGUID() || !m_session->GetPlayer()->HasFlag(PLAYER_FLAGS, PLAYER_FLAG_GM))
    {
        RedSystemMessage(m_session, "Only guild leaders and gms can remove players from a guild!");
        return true;
    }

    GreenSystemMessage(m_session, "Kicked %s from Guild: %s", selected_player->GetName(), selected_player->GetGuild()->GetGuildName());

    if (selected_player != m_session->GetPlayer())
        sGMLog.writefromsession(m_session, "Kicked %s from Guild %s", selected_player->GetName(), selected_player->GetGuild()->GetGuildName());

    selected_player->GetGuild()->RemoveGuildMember(selected_player->getPlayerInfo(), selected_player->GetSession());
    return true;
}
