/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "GuildMgr.h"
#include "Guild.h"
#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"

initialiseSingleton(GuildMgr);

GuildMgr::GuildMgr()
{ }

GuildMgr::~GuildMgr()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        delete itr->second;
    }
}

void GuildMgr::addGuild(Guild* guild)
{
    GuildStore[guild->getId()] = guild;
}

void GuildMgr::removeGuild(uint32_t guildId)
{
    GuildStore.erase(guildId);
}

void GuildMgr::saveGuilds()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        itr->second->saveGuildToDB();
    }
}

uint32_t GuildMgr::getNextGuildId()
{
    return objmgr.GenerateGuildId();
}

// Guild collection
Guild* GuildMgr::getGuildById(uint32_t guildId) const
{
    GuildContainer::const_iterator itr = GuildStore.find(guildId);
    if (itr != GuildStore.end())
    {
        return itr->second;
    }

    return nullptr;
}

Guild* GuildMgr::getGuildByName(const std::string& guildName) const
{
    std::string search = guildName;
    Util::StringToUpperCase(search);
    // std::transform(search.begin(), search.end(), search.begin(), ::toupper);
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        std::string gname = itr->second->getName();
        Util::StringToUpperCase(gname);
        // std::transform(gname.begin(), gname.end(), gname.begin(), ::toupper);
        if (search == gname)
        {
            return itr->second;
        }
    }
    return nullptr;
}

std::string GuildMgr::getGuildNameById(uint32_t guildId) const
{
    if (Guild* guild = getGuildById(guildId))
    {
        return guild->getName();
    }

    return "";
}

Guild* GuildMgr::getGuildByLeader(uint64_t guid) const
{
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        if (itr->second->getLeaderGUID() == guid)
        {
            return itr->second;
        }
    }

    return nullptr;
}

uint32_t GuildMgr::getXPForGuildLevel(uint8_t level) const
{
    if (level < GuildXPperLevel.size())
    {
        return static_cast<uint32_t>(GuildXPperLevel[level]);
    }

    return 0;
}

void GuildMgr::loadGuildDataFromDB()
{
    // 1. Load all guilds
    LogDebug("Loading guilds definitions...");
    {
        auto startTime = Util::TimeNow();

        //                                                         0          1            2             3              4              5              6
        QueryResult* result = CharacterDatabase.Query("SELECT g.guildId, g.guildName, g.leaderGuid, g.emblemStyle, g.emblemColor, g.borderStyle, g.borderColor, "
        //           7               8          9          10            11            12              13                 4                   15
            "g.backgroundColor, g.guildInfo, g.motd, g.createdate, g.bankBalance, g.guildLevel, g.guildExperience, g.todayExperience, COUNT(gbt.guildId) "
            "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildId = gbt.guildId GROUP BY g.guildId ORDER BY g.guildId ASC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild definitions. DB table `guild` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                Guild* guild = new Guild();

                if (!guild->loadGuildFromDB(fields))
                {
                    delete guild;
                    continue;
                }

                addGuild(guild);

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild definitions in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 2. Load all guild ranks
    LogDebug("Loading guild ranks...");
    {
        auto startTime = Util::TimeNow();

        // Delete orphaned guild rank entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gr FROM guild_rank gr LEFT JOIN guild g ON gr.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0      1      2      3           4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, rid, rname, rights, bankMoneyPerDay FROM guild_rank ORDER BY guildid ASC, rid ASC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild ranks. DB table `guild_rank` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadRankFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild ranks in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 3. Load all guild members
    LogDebug("Loading guild members...");
    {
        auto startTime = Util::TimeNow();

        CharacterDatabase.Execute("DELETE gm FROM guild_member gm LEFT JOIN guild g ON gm.guildId = g.guildId WHERE g.guildId IS NULL");

        QueryResult* result = CharacterDatabase.Query("SELECT guildId, playerGuid, rank, pnote, offnote FROM guild_member");
        QueryResult* result2 = CharacterDatabase.Query("SELECT guid, tab0, tab1, tab2, tab3, tab4, tab5, tab6, tab7, money FROM guild_member_withdraw");

        if (result == nullptr || result2 == nullptr)
        {
            LogDebug("Loaded 0 guild members. DB table `guild_member` OR `guild_member_withdraw` is empty.");
        }
        else
        {
            uint32_t count = 0;

            do
            {
                Field* fields = result->Fetch();
                Field* fields2 = result2->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadMemberFromDB(fields, fields2);
                }

                ++count;
            } while(result->NextRow() && result2->NextRow());

            LogDebug("Loaded %u guild members int %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 4. Load all guild bank tab rights
    LogDebug("Loading bank tab rights...");
    {
        auto startTime = Util::TimeNow();

        // Delete orphaned guild bank right entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gbr FROM guild_bank_right gbr LEFT JOIN guild g ON gbr.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0        1    2      3        4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, rid, gbright, slotPerDay FROM guild_bank_right ORDER BY guildId ASC, tabId ASC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild bank tab rights. DB table `guild_bank_right` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadBankRightFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u bank tab rights in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 5. Load all event logs
    LogDebug("Loading guild event logs...");
    {
        auto startTime = Util::TimeNow();

        CharacterDatabase.Execute("DELETE FROM guild_eventlog WHERE logGuid > %u", 100);
        CharacterDatabase.Execute("DELETE ge FROM guild_eventlog ge LEFT JOIN guild g ON ge.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                      0         1        2            3            4          5        6
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, logGuid, eventType, playerGuid1, playerGuid2, newRank, timeStamp FROM guild_eventlog ORDER BY timeStamp DESC, logGuid DESC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild event logs. DB table `guild_eventlog` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadEventLogFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild event logs in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 6. Load all bank event logs
    LogDebug("Loading guild bank event logs...");
    {
        auto startTime = Util::TimeNow();

        // Remove log entries that exceed the number of allowed entries per guild
        CharacterDatabase.Execute("DELETE FROM guild_bank_eventlog WHERE logGuid > %u", 25);
        CharacterDatabase.Execute("DELETE ge FROM guild_bank_eventlog ge LEFT JOIN guild g ON ge.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0       1       2         3           4           5             6             7          8
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, logGuid, eventType, playerGuid, itemOrMoney, itemStackCount, destTabId, timeStamp FROM guild_bank_eventlog ORDER BY timeStamp DESC, logGuid DESC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild bank event logs. DB table `guild_bank_eventlog` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadBankEventLogFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild bank event logs in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 7. Load all news event logs
    LogDebug("Loading Guild News...");
    {
        auto startTime = Util::TimeNow();

        CharacterDatabase.Execute("DELETE FROM guild_newslog WHERE logGuid > %u", 250);
        CharacterDatabase.Execute("DELETE gn FROM guild_newslog gn LEFT JOIN guild g ON gn.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0        1         2          3         4      5        6
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, logGuid, eventType, playerGuid, flags, value, timeStamp FROM guild_newslog ORDER BY timeStamp DESC, logGuid DESC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild event logs. DB table `guild_newslog` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadGuildNewsLogFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild new logs in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }


    // 8. Load all guild bank tabs
    LogDebug("Loading guild bank tabs...");
    {
        auto startTime = Util::TimeNow();

        // Delete orphaned guild bank tab entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gbt FROM guild_bank_tab gbt LEFT JOIN guild g ON gbt.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0       1       2        3        4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, tabName, tabIcon, tabText FROM guild_bank_tab ORDER BY guildId ASC, tabId ASC");

        if (result == nullptr)
        {
            LogDebug("Loaded 0 guild bank tabs. DB table `guild_bank_tab` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadBankTabFromDB(fields);
                }

                ++count;
            } while(result->NextRow());

            LogDebug("Loaded %u guild bank tabs in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 9. Fill all guild bank tabs
    LogDebug("Filling bank tabs with items...");
    {
        auto startTime = Util::TimeNow();

        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, slotId, itemGuid FROM guild_bank_item");

        if (result == nullptr)
        {
            LogError("Loaded 0 guild bank tab items. DB table `guild_bank_item` or `item_instance` is empty.");
        }
        else
        {
            uint32_t count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32_t guildId = fields[0].GetUInt32();

                if (Guild* guild = getGuildById(guildId))
                {
                    guild->loadBankItemFromDB(fields);
                }

                ++count;
            } while (result->NextRow());

            LogNotice("Loaded %u guild bank tab items in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
        }
    }

    // 10. Load guild achievements TODO
    {
        //TODO
    }
}

void GuildMgr::loadGuildXpForLevelFromDB()
{
    auto startTime = Util::TimeNow();

    GuildXPperLevel.resize(worldConfig.guild.maxLevel);
    for (uint8_t level = 0; level < worldConfig.guild.maxLevel; ++level)
    {
        GuildXPperLevel[level] = 0;
    }

    //                                                 0         1
    QueryResult* result = WorldDatabase.Query("SELECT lvl, xp_for_next_level FROM guild_xp_for_level");
    if (result == nullptr)
    {
        LogDebug("Loaded 0 xp for guild level definitions. DB table `guild_xp_for_level` is empty.");
        return;
    }

    uint32_t count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32_t level = fields[0].GetUInt8();
        uint32_t requiredXP = static_cast<uint32_t>(fields[1].GetUInt64());

        if (level >= worldConfig.guild.maxLevel)
        {
            LogDebug("Table `guild_xp_for_level` includes invalid xp definitions for level %u which is higher than the defined levelcap in your config file! <skipped>", level);
            continue;
        }

        GuildXPperLevel[level] = requiredXP;
        ++count;

    } while (result->NextRow());

    // fill level gaps
    for (uint8_t level = 1; level < worldConfig.guild.maxLevel ; ++level)
    {
        if (!GuildXPperLevel[level])
        {
            LogError("Level %i does not have XP for guild level data. Using data of level [%i] + 1660000.", level + 1, level);
            GuildXPperLevel[level] = GuildXPperLevel[level - 1] + 1660000;
        }
    }

    LogDebug("Loaded %u xp for guild level definitions in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
}

void GuildMgr::loadGuildRewardsFromDB()
{
    auto startTime = Util::TimeNow();

    //                                                  0       1         2        3         4
    QueryResult* result = WorldDatabase.Query("SELECT entry, standing, racemask, price, achievement FROM guild_rewards");
    if (result == nullptr)
    {
        LogDebug("Loaded 0 guild reward definitions. DB table `guild_rewards` is empty.");
        return;
    }

    uint32_t count = 0;

    do
    {
        GuildReward reward;
        Field* fields = result->Fetch();
        reward.entry = fields[0].GetUInt32();
        reward.standing = fields[1].GetUInt8();
        reward.racemask = fields[2].GetInt32();
        reward.price = fields[3].GetUInt64();
        reward.achievementId = fields[4].GetUInt32();

        if (!sItemStore.LookupEntry(reward.entry))
        {
            LogError("Guild rewards constains not existing item entry %u", reward.entry);
            continue;
        }

        if (reward.standing >= 8)
        {
            LogError("Guild rewards contains wrong reputation standing %u, max is %u", uint32_t(reward.standing), 8 - 1);
            continue;
        }

        GuildRewards.push_back(reward);
        ++count;
    } while(result->NextRow());

    LogDebug("Loaded %u guild reward definitions in %u ms", count, Util::GetTimeDifferenceToNow(startTime));
}

void GuildMgr::resetTimes(bool week)
{
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        if (Guild* guild = itr->second)
        {
            guild->resetTimes(week);
        }
    }
}

void GuildMgr::update(uint32_t /*diff*/)
{
    if (!firstSave)
    {
        lastSave = static_cast<uint32_t>(time(nullptr));
        firstSave = true;
    }

    if (time(nullptr) >= lastSave)
    {
        lastSave = static_cast<uint32_t>(time(nullptr)) + worldConfig.guild.saveInterval;
        sGuildMgr.saveGuilds();
    }
}
