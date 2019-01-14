/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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

        void update(uint32_t diff);
        void saveGuilds();

        void addGuild(Guild* guild);
        void removeGuild(uint32_t guildId);

        Guild* getGuildById(uint32_t guildId) const;
        Guild* getGuildByLeader(uint64_t guid) const;
        Guild* getGuildByName(std::string const& guildName) const;

        void loadGuildDataFromDB();


        std::string getGuildNameById(uint32_t guildId) const;

        void loadGuildXpForLevelFromDB();
        void loadGuildRewardsFromDB();

        uint32_t getNextGuildId();

        uint32_t getXPForGuildLevel(uint8_t level) const;
        std::vector<GuildReward> const& getGuildRewards() const { return GuildRewards; }

        void resetTimes(bool week);
        uint32_t lastSave = 0;
        bool firstSave = false;

    protected:

        typedef std::unordered_map<uint32_t, Guild*> GuildContainer;

        GuildContainer GuildStore;
        std::vector<uint64_t> GuildXPperLevel;
        std::vector<GuildReward> GuildRewards;
};

#define sGuildMgr GuildMgr::getSingleton()
