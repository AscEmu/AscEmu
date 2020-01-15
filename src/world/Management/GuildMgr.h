/*
Copyright (c) 2014-2020 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Guild.h"

class SERVER_DECL GuildMgr
{
    private:

        GuildMgr() = default;
        ~GuildMgr() = default;

    public:

        static GuildMgr& getInstance();
        void finalize();

        GuildMgr(GuildMgr&&) = delete;
        GuildMgr(GuildMgr const&) = delete;
        GuildMgr& operator=(GuildMgr&&) = delete;
        GuildMgr& operator=(GuildMgr const&) = delete;

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

#if VERSION_STRING >= Cata
        void resetTimes(bool week);
#endif
        uint32_t lastSave = 0;
        bool firstSave = false;

    protected:

        typedef std::unordered_map<uint32_t, Guild*> GuildContainer;

        GuildContainer GuildStore;
        std::vector<uint64_t> GuildXPperLevel;
        std::vector<GuildReward> GuildRewards;
};

#define sGuildMgr GuildMgr::getInstance()
