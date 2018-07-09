/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Singleton.h"

#if VERSION_STRING != Cata
#include "Guild.h"
#else
#include "GameCata/Management/Guild.h"
#endif

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


#if VERSION_STRING == Cata
        std::string getGuildNameById(uint32_t guildId) const;

        void loadGuildXpForLevelFromDB();
        void loadGuildRewardsFromDB();

        uint32_t getNextGuildId();

        uint32_t getXPForGuildLevel(uint8_t level) const;
        std::vector<GuildReward> const& getGuildRewards() const { return GuildRewards; }

        void resetTimes(bool week);
#endif
        uint32_t lastSave;
        bool firstSave;

    protected:

        typedef std::unordered_map<uint32_t, Guild*> GuildContainer;

        GuildContainer GuildStore;
#if VERSION_STRING == Cata
        std::vector<uint64_t> GuildXPperLevel;
        std::vector<GuildReward> GuildRewards;
#endif
};

#define sGuildMgr GuildMgr::getSingleton()
