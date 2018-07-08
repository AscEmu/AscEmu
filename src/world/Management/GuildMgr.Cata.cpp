/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

#include "GuildMgr.h"

#if VERSION_STRING == Cata
#include "GameCata/Management/Guild.h"

#include "Objects/ObjectMgr.h"
#include "Server/MainServerDefines.h"


uint32_t GuildMgr::getNextGuildId()
{
    return objmgr.GenerateGuildId();
}

// Guild collection


std::string GuildMgr::getGuildNameById(uint32_t guildId) const
{
    if (Guild* guild = getGuildById(guildId))
    {
        return guild->getName();
    }

    return "";
}

uint32_t GuildMgr::getXPForGuildLevel(uint8_t level) const
{
    if (level < GuildXPperLevel.size())
    {
        return static_cast<uint32_t>(GuildXPperLevel[level]);
    }

    return 0;
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
    for (uint8_t level = 1; level < worldConfig.guild.maxLevel; ++level)
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
#endif
