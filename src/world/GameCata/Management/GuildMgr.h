/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Singleton.h"
#include "Guild.h"

class SERVER_DECL GuildMgr : public Singleton <GuildMgr>
{
    public:

        GuildMgr();
        ~GuildMgr();

    public:

        Guild* getGuildByLeader(uint64_t guid) const;
        Guild* getGuildById(uint32_t guildId) const;
        Guild* getGuildByName(std::string const& guildName) const;
        std::string getGuildNameById(uint32_t guildId) const;

        void loadGuildXpForLevelFromDB();
        void loadGuildRewardsFromDB();

        void loadGuildDataFromDB();
        void addGuild(Guild* guild);
        void removeGuild(uint32_t guildId);

        void update(uint32_t diff);
        void saveGuilds();

        uint32_t getNextGuildId();

        uint32_t getXPForGuildLevel(uint8_t level) const;
        std::vector<GuildReward> const& getGuildRewards() const { return GuildRewards; }

        void resetTimes(bool week);

        uint32_t lastSave;
        bool firstSave;

    protected:

        typedef std::unordered_map<uint32_t, Guild*> GuildContainer;

        GuildContainer GuildStore;
        std::vector<uint64_t> GuildXPperLevel;
        std::vector<GuildReward> GuildRewards;
};

#define sGuildMgr GuildMgr::getSingleton()
