/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#if VERSION_STRING == Mop
#include "GuildFinderMgr.h"
#include "Management/GuildMgr.h"
#include "Management/Guild.h"
#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"

initialiseSingleton(GuildFinderMgr);

GuildFinderMgr::GuildFinderMgr()
{
}

GuildFinderMgr::~GuildFinderMgr()
{
}

void GuildFinderMgr::loadGuildFinderDataFromDB()
{
    loadGuildSettingsFromDB();
    loadMembershipRequestsFromDB();
}

void GuildFinderMgr::loadGuildSettingsFromDB()
{
    LogNotice("Loading guild finder guild-related settings...");

    CharacterDatabase.Execute("DELETE gfgs FROM guild_finder_guild_settings gfgs LEFT JOIN guilds g ON gfgs.guildId = g.guildId WHERE g.guildId IS NULL");

    //                                                          0                1               2                 3             4           5             6         7
    QueryResult* result = CharacterDatabase.Query("SELECT gfgs.guildId, gfgs.availability, gfgs.classRoles, gfgs.interests, gfgs.level, gfgs.listed, gfgs.comment, c.race "
                                                 "FROM guild_finder_guild_settings gfgs "
                                                 "LEFT JOIN guild_members gm ON gm.guildId = gfgs.guildId "
                                                 "LEFT JOIN characters c ON c.guid = gm.playerid LIMIT 1");

    if (result == nullptr)
    {
        LogNotice("Loaded 0 guild finder guild-related settings. Table `guild_finder_guild_settings` is empty.");
        return;
    }

    uint32_t count = 0;
    auto startTime = Util::TimeNow();
    do
    {
        Field* fields = result->Fetch();
        uint32_t guildId = fields[0].GetUInt32();
        uint8_t availability = fields[1].GetUInt8();
        uint8_t classRoles = fields[2].GetUInt8();
        uint8_t interests = fields[3].GetUInt8();
        uint8_t level = fields[4].GetUInt8();
        bool listed = (fields[5].GetUInt8() != 0);
        std::string comment = fields[6].GetString();

        PlayerTeam guildTeam = TEAM_ALLIANCE;
        if (auto raceEntry = sChrRacesStore.LookupEntry(fields[7].GetUInt8()))
        {
            if (raceEntry->team_id == 1)
            {
                guildTeam = TEAM_HORDE;
            }
        }

        LFGuildSettings settings(listed, guildTeam, guildId, classRoles, availability, interests, level, comment);
        _lfgGuildStore[guildId] = settings;

        ++count;
    } while(result->NextRow());

    LogNotice("Loaded %u guild finder guild-related settings in %u ms.", count, Util::GetTimeDifferenceToNow(startTime));
}

void GuildFinderMgr::loadMembershipRequestsFromDB()
{
    LogNotice("Loading guild finder membership requests...");

    CharacterDatabase.Execute("DELETE gfa FROM guild_finder_applicant gfa LEFT JOIN guilds g ON gfa.guildId = g.guildId WHERE g.guildId IS NULL");
    //                                                       0         1           2            3           4         5         6
    QueryResult* result = CharacterDatabase.Query("SELECT guildId, playerGuid, availability, classRole, interests, comment, submitTime FROM guild_finder_applicant");
    if (result == nullptr)
    {
        LogNotice("Loaded 0 guild finder membership requests. Table `guild_finder_applicant` is empty.");
        return;
    }

    uint32_t count = 0;
    auto startTime = Util::TimeNow();
    do
    {
        Field* fields = result->Fetch();
        uint32_t guildId = fields[0].GetUInt32();
        uint32_t playerId = fields[1].GetUInt32();
        uint8_t availability = fields[2].GetUInt8();
        uint8_t classRoles = fields[3].GetUInt8();
        uint8_t interests = fields[4].GetUInt8();
        std::string comment = fields[5].GetString();
        uint32_t submitTime = fields[6].GetUInt32();

        MembershipRequest request(playerId, guildId, availability, classRoles, interests, comment, time_t(submitTime));

        _membershipRequestStore[guildId].push_back(request);

        ++count;
    } while(result->NextRow());

    LogNotice("Loaded %u guild finder membership requests in %u ms.", count, Util::GetTimeDifferenceToNow(startTime));
}

void GuildFinderMgr::addMembershipRequest(uint32_t guildGuid, MembershipRequest const& request)
{
    _membershipRequestStore[guildGuid].push_back(request);

    CharacterDatabase.Execute("REPLACE INTO guild_finder_applicant VALUES(%u, %u, %u, %u, %u, '%s', %u)",
        request.getGuildId(), request.getPlayerGUID(), request.getAvailability(), request.getClassRoles(),
        request.getInterests(), request.getComment().c_str(),request.getSubmitTime());
  
    if (Player* player = objmgr.GetPlayer(request.getPlayerGUID()))
    {
        sendMembershipRequestListUpdate(*player);
    }

    if (Guild* guild = sGuildMgr.getGuildById(guildGuid))
    {
        sendApplicantListUpdate(*guild);
    }
}

void GuildFinderMgr::removeAllMembershipRequestsFromPlayer(uint32_t playerId)
{
    for (MembershipRequestStore::iterator itr = _membershipRequestStore.begin(); itr != _membershipRequestStore.end(); ++itr)
    {
        std::vector<MembershipRequest>::iterator itr2 = itr->second.begin();
        for (; itr2 != itr->second.end(); ++itr2)
        {
            if (itr2->getPlayerGUID() == playerId)
            {
                break;
            }
        }

        if (itr2 == itr->second.end())
        {
            continue;
        }

        CharacterDatabase.Execute("DELETE FROM guild_finder_applicant WHERE guildId = %u AND playerGuid = %u", itr2->getGuildId(), itr2->getPlayerGUID());

        itr->second.erase(itr2);

        if (Guild* guild = sGuildMgr.getGuildById(itr->first))
        {
            sendApplicantListUpdate(*guild);
        }
    }
}

void GuildFinderMgr::removeMembershipRequest(uint32_t playerId, uint32_t guildId)
{
    std::vector<MembershipRequest>::iterator itr = _membershipRequestStore[guildId].begin();
    for (; itr != _membershipRequestStore[guildId].end(); ++itr)
    {
        if (itr->getPlayerGUID() == playerId)
        {
            break;
        }
    }

    if (itr == _membershipRequestStore[guildId].end())
    {
        return;
    }

    CharacterDatabase.Execute("DELETE FROM guild_finder_applicant WHERE guildId = %u AND playerGuid = %u", itr->getGuildId(), itr->getPlayerGUID());

    _membershipRequestStore[guildId].erase(itr);

    if (Player* player = objmgr.GetPlayer(playerId))
    {
        sendMembershipRequestListUpdate(*player);
    }

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        sendApplicantListUpdate(*guild);
    }
}

std::list<MembershipRequest> GuildFinderMgr::getAllMembershipRequestsForPlayer(uint32_t playerGuid)
{
    std::list<MembershipRequest> resultSet;
    for (MembershipRequestStore::const_iterator itr = _membershipRequestStore.begin(); itr != _membershipRequestStore.end(); ++itr)
    {
        std::vector<MembershipRequest> const& guildReqs = itr->second;
        for (std::vector<MembershipRequest>::const_iterator itr2 = guildReqs.begin(); itr2 != guildReqs.end(); ++itr2)
        {
            if (itr2->getPlayerGUID() == playerGuid)
            {
                resultSet.push_back(*itr2);
                break;
            }
        }
    }
    return resultSet;
}

uint8_t GuildFinderMgr::countRequestsFromPlayer(uint32_t playerId)
{
    uint8_t result = 0;
    for (MembershipRequestStore::const_iterator itr = _membershipRequestStore.begin(); itr != _membershipRequestStore.end(); ++itr)
    {
        for (std::vector<MembershipRequest>::const_iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2)
        {
            if (itr2->getPlayerGUID() != playerId)
            {
                continue;
            }

            ++result;
            break;
        }
    }
    return result;
}

LFGuildStore GuildFinderMgr::getGuildsMatchingSetting(LFGuildPlayer& settings, PlayerTeam faction)
{
    LFGuildStore resultSet;
    for (LFGuildStore::const_iterator itr = _lfgGuildStore.begin(); itr != _lfgGuildStore.end(); ++itr)
    {
        LFGuildSettings const& guildSettings = itr->second;

        if (guildSettings.getTeam() != faction)
        {
            continue;
        }

        if (!(guildSettings.getAvailability() & settings.getAvailability()))
        {
            continue;
        }

        if (!(guildSettings.getClassRoles() & settings.getClassRoles()))
        {
            continue;
        }

        if (!(guildSettings.getInterests() & settings.getInterests()))
        {
            continue;
        }

        if (!(guildSettings.getLevel() & settings.getLevel()))
        {
            continue;
        }

        resultSet.insert(std::make_pair(itr->first, guildSettings));
    }

    return resultSet;
}

bool GuildFinderMgr::hasRequest(uint32_t playerId, uint32_t guildId)
{
    for (std::vector<MembershipRequest>::const_iterator itr = _membershipRequestStore[guildId].begin(); itr != _membershipRequestStore[guildId].end(); ++itr)
    {
        if (itr->getPlayerGUID() == playerId)
        {
            return true;
        }
    }

    return false;
}

void GuildFinderMgr::setGuildSettings(uint32_t guildGuid, LFGuildSettings const& settings)
{
    _lfgGuildStore[guildGuid] = settings;
    
    CharacterDatabase.Execute("REPLACE INTO guild_finder_guild_settings VALUES(%u, %u, %u, %u, %u, %u, '%s')", 
        settings.getGUID(), settings.getAvailability(), settings.getClassRoles(), settings.getInterests(),
        settings.getLevel(), settings.isListed(), settings.getComment().c_str());
}

void GuildFinderMgr::deleteGuild(uint32_t guildId)
{
    std::vector<MembershipRequest>::iterator itr = _membershipRequestStore[guildId].begin();
    while (itr != _membershipRequestStore[guildId].end())
    {
        uint32_t applicant = itr->getPlayerGUID();

        CharacterDatabase.Execute("DELETE FROM guild_finder_applicant WHERE guildId = %u AND playerGuid = %u", itr->getGuildId(), applicant);

        CharacterDatabase.Execute("DELETE FROM guild_finder_guild_settings WHERE guildId = %u", itr->getGuildId());

        if (Player* player = objmgr.GetPlayer(applicant))
        {
            sendMembershipRequestListUpdate(*player);
        }

        ++itr;
    }

    _membershipRequestStore.erase(guildId);
    _lfgGuildStore.erase(guildId);

    if (Guild* guild = sGuildMgr.getGuildById(guildId))
    {
        sendApplicantListUpdate(*guild);
    }
}

void GuildFinderMgr::sendApplicantListUpdate(Guild& guild)
{
    WorldPacket data(SMSG_LF_GUILD_APPLICANT_LIST_UPDATED, 0);
    if (Player* player = objmgr.GetPlayer(WoWGuid::getGuidLowPartFromUInt64(guild.getLeaderGUID())))
    {
        player->SendMessageToSet(&data, false);
    }

    guild.broadcastPacketToRank(&data, GR_OFFICER);
}

void GuildFinderMgr::sendMembershipRequestListUpdate(Player& player)
{
    WorldPacket data(SMSG_LF_GUILD_APPLICATIONS_LIST_CHANGED, 0);
    player.SendMessageToSet(&data, false);
}

#endif
